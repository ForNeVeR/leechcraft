/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "playlistmanager.h"
#include <memory>
#include <QStandardItem>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/lmp/iplaylistprovider.h>
#include "authmanager.h"
#include "accountsmanager.h"
#include "consts.h"

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	PlaylistManager::PlaylistManager (QNetworkAccessManager *nam,
			AuthManager *authMgr, AccountsManager *accMgr, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, AuthMgr_ (authMgr)
	, AccMgr_ (accMgr)
	, Root_ (new QStandardItem ("mp3tunes.com"))
	{
		Root_->setEditable (false);
		connect (AuthMgr_,
				SIGNAL (sidReady (QString)),
				this,
				SLOT (requestPlaylists (QString)));

		connect (AccMgr_,
				SIGNAL (accountsChanged ()),
				this,
				SLOT (handleAccountsChanged ()),
				Qt::QueuedConnection);
	}

	QStandardItem* PlaylistManager::GetRoot () const
	{
		return Root_;
	}

	void PlaylistManager::Update ()
	{
		while (Root_->rowCount ())
			Root_->removeRow (0);
		AccItems_.clear ();
		AccPlaylists_.clear ();
		Infos_.clear ();

		const auto& accs = AccMgr_->GetAccounts ();
		Q_FOREACH (const auto& acc, accs)
		{
			auto item = new QStandardItem (acc);
			item->setEditable (false);
			AccItems_ [acc] = item;
			Root_->appendRow (item);

			requestPlaylists (acc);
		}
	}

	boost::optional<Media::AudioInfo> PlaylistManager::GetMediaInfo (const QUrl& url) const
	{
		return Infos_.contains (url) ?
				boost::make_optional (Infos_ [url]) :
				boost::optional<Media::AudioInfo> ();
	}

	void PlaylistManager::requestPlaylists (const QString& accName)
	{
		const auto& sid = AuthMgr_->GetSID (accName);
		if (sid.isEmpty ())
			return;

		const auto& url = QString ("http://ws.mp3tunes.com/api/v1/lockerData?output=xml"
					"&sid=%1&partner_token=%2&type=playlist")
				.arg (sid)
				.arg (Consts::PartnerId);
		auto reply = NAM_->get (QNetworkRequest (url));
		reply->setProperty ("AccountName", accName);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotPlaylists ()));
	}

	void PlaylistManager::handleGotPlaylists ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& accName = reply->property ("AccountName").toString ();
		const auto& data = reply->readAll ();

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			return;
		}

		auto playlistsRoot = AccItems_ [accName];
		auto& playlists = AccPlaylists_ [accName];

		auto plItem = doc.documentElement ()
				.firstChildElement ("playlistList")
				.firstChildElement ("item");
		while (!plItem.isNull ())
		{
			std::shared_ptr<void> (static_cast<void*> (0),
					[&plItem] (void*) { plItem = plItem.nextSiblingElement ("item"); });
			const auto& id = plItem.firstChildElement ("playlistId").text ();
			if (id == "INBOX" || id.isEmpty ())
				continue;

			const auto& title = plItem.firstChildElement ("playlistTitle").text ();
			auto item = new QStandardItem (title);
			item->setEditable (false);
			playlists [id] = item;
			playlistsRoot->appendRow (item);

			const auto& sid = AuthMgr_->GetSID (accName);
			const auto& url = QString ("http://ws.mp3tunes.com/api/v1/lockerData?output=xml"
					"&sid=%1&partner_token=%2&type=track&playlist_id=%3")
					.arg (sid)
					.arg (Consts::PartnerId)
					.arg (id);
			auto contentsReply = NAM_->get (QNetworkRequest (url));
			contentsReply->setProperty ("AccountName", accName);
			contentsReply->setProperty ("PlaylistID", id);
			connect (contentsReply,
					SIGNAL (finished ()),
					this,
					SLOT (handleGotPlaylistContents ()));
		}
	}

	void PlaylistManager::handleGotPlaylistContents ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& accName = reply->property ("AccountName").toString ();
		const auto& id = reply->property ("PlaylistID").toString ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			return;
		}

		QList<QUrl> urls;
		QStringList compositions;

		auto trackItem = doc.documentElement ()
				.firstChildElement ("trackList")
				.firstChildElement ("item");
		while (!trackItem.isNull ())
		{
			auto getText = [&trackItem] (const QString& elem) -> QString
			{
				const auto& text = trackItem.firstChildElement (elem).text ();
				return text.isEmpty () ? "unknown" : text;
			};

			const auto& urlStr = getText ("playURL").toUtf8 ();
			const auto& url = QUrl::fromEncoded (urlStr);
			urls << url;

			Media::AudioInfo info;
			info.Artist_ = getText ("artistName");
			info.Album_ = getText ("albumTitle");
			info.Title_ = getText ("trackTitle");
			info.Length_ = getText ("trackLength").toInt () / 1000;

			compositions << QString::fromUtf8 ("%1 — %2 — %3")
					.arg (info.Artist_)
					.arg (info.Album_)
					.arg (info.Title_);

			Infos_ [url] = info;

			trackItem = trackItem.nextSiblingElement ("item");
		}

		auto plItem = AccPlaylists_ [accName] [id];

		if (urls.isEmpty ())
		{
			plItem->parent ()->removeRow (plItem->row ());
			AccPlaylists_ [accName].remove (id);
			return;
		}

		plItem->setData (QVariant::fromValue (urls), IPlaylistProvider::ItemRoles::SourceURLs);
		plItem->setToolTip ("<ul><li>" + compositions.join ("</li><li>") + "</li></ul>");
	}

	void PlaylistManager::handleAccountsChanged ()
	{
		Update ();
	}
}
}
}
