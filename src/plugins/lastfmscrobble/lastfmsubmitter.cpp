/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "lastfmsubmitter.h"
#include "codes.h"
#include <QCryptographicHash>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QTimer>
#include <lastfm/Track>
#include <lastfm.h>
#include <interfaces/media/audiostructs.h>
#include <util/util.h>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	const QString ScrobblingSite = "http://ws.audioscrobbler.com/2.0/";

	MediaMeta::MediaMeta ()
	: TrackNumber_ (0)
	, Length_ (0)
	{
	}

	MediaMeta::MediaMeta (const QMap<QString, QVariant>& tagMap)
	: Artist_ (tagMap ["Artist"].toString ())
	, Album_ (tagMap ["Album"].toString ())
	, Title_ (tagMap ["Title"].toString ())
	, Genre_ (tagMap ["Genre"].toString ())
	, Date_ (tagMap ["Date"].toString ())
	, TrackNumber_ (tagMap ["TrackNumber"].toInt ())
	, Length_ (tagMap ["Length"].toInt ())
	{
	}

	MediaMeta::MediaMeta (const Media::AudioInfo& info)
	: Artist_ (info.Artist_)
	, Album_ (info.Album_)
	, Title_ (info.Title_)
	, Genre_ (info.Genres_.join (" / "))
	, Date_ (QString::number (info.Year_))
	, TrackNumber_ (info.TrackNumber_)
	, Length_ (info.Length_)
	{
	}

	namespace
	{
		QString AuthToken (const QString& username, const QString& password)
		{
			const QString& passHash = QCryptographicHash::hash (password.toAscii (),
					QCryptographicHash::Md5).toHex ();
			return QCryptographicHash::hash ((username + passHash).toAscii (),
					QCryptographicHash::Md5).toHex ();
		}

		QString ApiSig (const QString& api_key, const QString& authToken,
				const QString& method, const QString& username,
				const QString& secret)
		{
			const QString& str = QString ("api_key%1authToken%2method%3username%4%5")
					.arg (api_key)
					.arg (authToken)
					.arg (method)
					.arg (username)
					.arg (secret);
			return QCryptographicHash::hash (str.toAscii (),
					QCryptographicHash::Md5).toHex ();
		}

		QByteArray MakeCall (QList<QPair<QString, QString>> params)
		{
			std::sort (params.begin (), params.end (),
					[] (decltype (*params.constEnd ()) left, decltype (*params.constEnd ()) right)
						{ return left.first < right.first; });
			auto str = std::accumulate (params.begin (), params.end (), QString (),
					[] (const QString& str, decltype (params.front ()) pair)
						{ return str + pair.first + pair.second; });
			str += lastfm::ws::SharedSecret;
			const auto& sig = QCryptographicHash::hash (str.toUtf8 (), QCryptographicHash::Md5).toHex ();

			params << QPair<QString, QString> ("api_sig", sig);

			QUrl url;
			std::for_each (params.begin (), params.end (),
					[&url] (decltype (params.front ()) pair) { url.addQueryItem (pair.first, pair.second); });
			return url.encodedQuery ();
		}

		QString GetQueueFilename ()
		{
			return Util::CreateIfNotExists ("lastfmscrobble").absoluteFilePath ("queue.xml");
		}
	}

	LastFMSubmitter::LastFMSubmitter (QObject *parent)
	: QObject (parent)
	, SubmitTimer_ (new QTimer (this))
	{
		lastfm::ws::ApiKey = "a5ca8821e39cdb5efd2e5667070084b2";
		lastfm::ws::SharedSecret = "50fb8b6f35fc55b7ddf6bb033dfc6fbe";

		SubmitTimer_->setSingleShot (true);

		LoadQueue ();
	}

	void LastFMSubmitter::Init (QNetworkAccessManager *manager)
	{
		NAM_ = manager;
		const QString& authToken = AuthToken (lastfm::ws::Username,
				Password_);

		const QString& api_sig = ApiSig (lastfm::ws::ApiKey, authToken,
				"auth.getMobileSession", lastfm::ws::Username,
				lastfm::ws::SharedSecret);
		const QString& url = QString ("%1?method=%2&username=%3&authToken=%4&api_key=%5&api_sig=%6")
				.arg (ScrobblingSite)
				.arg ("auth.getMobileSession")
				.arg (lastfm::ws::Username)
				.arg (authToken)
				.arg (lastfm::ws::ApiKey)
				.arg (api_sig);

		QNetworkReply *reply = manager->get (QNetworkRequest (QUrl (url)));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (getSessionKey ()));
	}

	void LastFMSubmitter::SetUsername (const QString& username)
	{
		lastfm::ws::Username = username;
	}

	void LastFMSubmitter::SetPassword (const QString& password)
	{
		Password_ = password;
	}

	bool LastFMSubmitter::IsConnected () const
	{
		return Scrobbler_ ? true : false;
	}

	namespace
	{
		lastfm::MutableTrack ToLastFMTrack (const MediaMeta& info)
		{
			lastfm::MutableTrack mutableTrack;
			mutableTrack.setTitle (info.Title_);
			mutableTrack.setAlbum (info.Album_);
			mutableTrack.setArtist (info.Artist_);
			mutableTrack.stamp ();
			mutableTrack.setSource (lastfm::Track::Player);
			mutableTrack.setDuration (info.Length_);
			mutableTrack.setTrackNumber (info.TrackNumber_);
			return mutableTrack;
		}
	}

	void LastFMSubmitter::NowPlaying (const MediaMeta& info)
	{
		SubmitTimer_->stop ();

		NextSubmit_ = lastfm::Track ();

		const auto& lfmTrack = ToLastFMTrack (info);
		if (info.Length_ < 30)
			return;
		if (!Scrobbler_)
		{
			SubmitQueue_ << lfmTrack;
			return;
		}
		Scrobbler_->nowPlaying (lfmTrack);

		NextSubmit_ = lfmTrack;
		Scrobbler_->cache (lfmTrack);
		SubmitTimer_->start (std::min (info.Length_ / 2, 240) * 1000);
	}

	void LastFMSubmitter::Love ()
	{
		if (NextSubmit_.isNull ())
			return;

		QNetworkRequest req (QUrl ("http://ws.audioscrobbler.com/2.0/"));
		const QString method = "track.love";
		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("method", method);
		params << QPair<QString, QString> ("track", NextSubmit_.title ());
		params << QPair<QString, QString> ("artist", NextSubmit_.artist ());
		params << QPair<QString, QString> ("api_key", lastfm::ws::ApiKey);
		params << QPair<QString, QString> ("sk", lastfm::ws::SessionKey);
		const auto& data = MakeCall (params);
		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		QNetworkReply *reply = NAM_->post (req, data);
		connect (reply,
				SIGNAL (finished ()),
				reply,
				SLOT (deleteLater ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				reply,
				SLOT (deleteLater ()));
	}

	void LastFMSubmitter::Clear ()
	{
		NextSubmit_ = lastfm::MutableTrack ();
		SubmitTimer_->stop ();
	}

	void LastFMSubmitter::LoadQueue ()
	{
		QFile file (GetQueueFilename ());
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< file.errorString ();
			return;
		}

		QDomDocument doc;
		doc.setContent (file.readAll ());
		auto elem = doc.documentElement ().firstChildElement ();
		while (!elem.isNull ())
		{
			lastfm::Track track (elem);
			if (!track.isNull ())
				SubmitQueue_ << track;
			elem = elem.nextSiblingElement ();
		}
	}

	void LastFMSubmitter::SaveQueue () const
	{
		QDomDocument doc ("queue");
		auto root = doc.createElement ("queue");
		doc.appendChild (root);

		std::for_each (SubmitQueue_.begin (), SubmitQueue_.end (),
				[&doc, &root] (decltype (SubmitQueue_.front ()) item)
				{
					root.appendChild (item.toDomElement (doc));
				});

		QFile file (GetQueueFilename ());
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< file.errorString ();
			return;
		}

		file.write (doc.toByteArray ());
	}

	bool LastFMSubmitter::CheckError (const QDomDocument& doc)
	{
		auto sub = doc.documentElement ().firstChildElement ("error");
		if (sub.isNull ())
			return false;

		const int code = sub.attribute ("code").toInt ();
		switch (code)
		{
		case ErrorCodes::AuthError:
			emit authFailure ();
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown error code"
					<< code;
			break;
		}

		return true;
	}

	void LastFMSubmitter::checkFlushQueue (int code)
	{
		qDebug () << Q_FUNC_INFO << code;
		if (code == lastfm::Audioscrobbler::TracksScrobbled)
		{
			qDebug () << "tracks scrobbled, clearing queue";
			SubmitQueue_.clear ();
			SaveQueue ();
		}
	}

	void LastFMSubmitter::submit ()
	{
		SubmitQueue_ << NextSubmit_;
		SaveQueue ();

		if (!IsConnected ())
			return;

		Scrobbler_->submit ();
	}

	void LastFMSubmitter::getSessionKey ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();
		QDomDocument doc;
		doc.setContent (QString::fromUtf8 (reply->readAll ()));
		if (CheckError (doc))
			return;

		const auto& domList = doc.documentElement ().elementsByTagName ("key");
		if (!domList.size ())
			return;

		lastfm::ws::SessionKey = domList.at (0).toElement ().text ();

		Scrobbler_.reset (new lastfm::Audioscrobbler ("tst"));

		connect (Scrobbler_.get (),
				SIGNAL (status (int)),
				this,
				SIGNAL (status (int)));
		connect (Scrobbler_.get (),
				SIGNAL (status (int)),
				this,
				SLOT (checkFlushQueue (int)));

		connect (SubmitTimer_,
				SIGNAL (timeout ()),
				Scrobbler_.get (),
				SLOT (submit ()));

		if (!SubmitQueue_.isEmpty ())
		{
			Scrobbler_->cache (SubmitQueue_);
			submit ();
		}
	}
}
}
