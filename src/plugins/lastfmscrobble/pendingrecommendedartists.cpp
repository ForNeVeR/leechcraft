/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "pendingrecommendedartists.h"
#include <memory>
#include <QDomDocument>
#include <QNetworkReply>
#include <QtDebug>
#include "authenticator.h"
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingRecommendedArtists::PendingRecommendedArtists (Authenticator *auth,
			QNetworkAccessManager *nam, int num, QObject *obj)
	: BaseSimilarArtists (QString (), num, obj)
	, NAM_ (nam)
	{
		if (auth->IsAuthenticated ())
			request ();
		else
			connect (auth,
					SIGNAL (authenticated ()),
					this,
					SLOT (request ()));
	}

	void PendingRecommendedArtists::request ()
	{
		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("limit", QString::number (NumGet_));
		auto reply = Request ("user.getRecommendedArtists", NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleReplyError ()));
	}

	void PendingRecommendedArtists::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			emit error ();
			return;
		}

		auto artistElem = doc.documentElement ()
				.firstChildElement ("recommendations")
				.firstChildElement ("artist");
		auto elemGuard = [&artistElem] (void*) { artistElem = artistElem.nextSiblingElement ("artist"); };
		while (!artistElem.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0), elemGuard);

			const auto& name = artistElem.firstChildElement ("name").text ();
			if (name.isEmpty ())
				continue;

			QStringList similarTo;
			auto similarElem = artistElem.firstChildElement ("context").firstChildElement ("artist");
			while (!similarElem.isNull ())
			{
				similarTo << similarElem.firstChildElement ("name").text ();
				similarElem = similarElem.nextSiblingElement ("artist");
			}

			++InfosWaiting_;

			QMap<QString, QString> params;
			params ["artist"] = name;
			AddLanguageParam (params);
			auto infoReply = Request ("artist.getInfo", NAM_, params);

			infoReply->setProperty ("SimilarTo", similarTo);
			connect (infoReply,
					SIGNAL (finished ()),
					this,
					SLOT (handleInfoReplyFinished ()));
			connect (infoReply,
					SIGNAL (error (QNetworkReply::NetworkError)),
					this,
					SLOT (handleInfoReplyError ()));
		}
	}
}
}
