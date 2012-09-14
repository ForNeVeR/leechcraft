/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "streamlistfetcherbase.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <interfaces/media/iradiostationprovider.h>
#include "roles.h"

namespace LeechCraft
{
namespace HotStreams
{
	StreamListFetcherBase::StreamListFetcherBase (QStandardItem *root, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Root_ (root)
	, RadioIcon_ (":/hotstreams/resources/images/radio.png")
	{
	}

	void StreamListFetcherBase::Request (const QNetworkRequest& req)
	{
		auto reply = NAM_->get (req);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void StreamListFetcherBase::HandleData (const QByteArray& data)
	{
		auto result = Parse (data);
		std::sort (result.begin (), result.end (),
				[] (decltype (result.at (0)) left, decltype (result.at (0)) right)
					{ return QString::localeAwareCompare (left.Name_, right.Name_) < 0; });
		Q_FOREACH (const auto& stream, result)
		{
			auto name = stream.Name_;
			if (!stream.Genres_.isEmpty ())
				name += " (" + stream.Genres_.join ("; ") + ")";

			auto tooltip = "<span style=\"white-space: nowrap\">" + stream.Description_;
			if (!stream.DJ_.isEmpty ())
				tooltip += "<br /><em>DJ:</em> " + stream.DJ_;
			tooltip += "</span>";

			auto item = new QStandardItem (name);
			item->setToolTip (tooltip);
			item->setIcon (RadioIcon_);
			item->setData (stream.Name_, StreamItemRoles::PristineName);
			item->setData (Media::RadioType::Predefined, Media::RadioItemRole::ItemType);
			item->setData (stream.URL_, Media::RadioItemRole::RadioID);
			item->setData (stream.PlaylistFormat_, StreamItemRoles::PlaylistFormat);
			item->setEditable (false);
			Root_->appendRow (item);
		}

		deleteLater ();
	}

	void StreamListFetcherBase::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();
		HandleData (reply->readAll ());
	}
}
}
