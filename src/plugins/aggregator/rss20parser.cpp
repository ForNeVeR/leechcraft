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

#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QtDebug>
#include "rss20parser.h"

namespace LeechCraft
{
namespace Aggregator
{
	RSS20Parser::RSS20Parser ()
	{
	}
	
	RSS20Parser::~RSS20Parser ()
	{
	}
	
	RSS20Parser& RSS20Parser::Instance ()
	{
		static RSS20Parser inst;
		return inst;
	}
	
	bool RSS20Parser::CouldParse (const QDomDocument& doc) const
	{
		QDomElement root = doc.documentElement ();
		return root.tagName () == "rss" &&
			root.attribute ("version") == "2.0";
	}
	
	channels_container_t RSS20Parser::Parse (const QDomDocument& doc,
			const IDType_t& feedId) const
	{
		channels_container_t channels;
		QDomElement root = doc.documentElement ();
		QDomElement channel = root.firstChildElement ("channel");
		while (!channel.isNull ())
		{
			Channel_ptr chan (new Channel (feedId));
			chan->Title_ = channel.firstChildElement ("title").text ().trimmed ();
			chan->Description_ = channel.firstChildElement ("description").text ();
			chan->Link_ = GetLink (channel);
			chan->LastBuild_ = RFC822TimeToQDateTime (channel.firstChildElement ("lastBuildDate").text ());
			chan->Language_ = channel.firstChildElement ("language").text ();
			chan->Author_ = GetAuthor (channel);
			if (chan->Author_.isEmpty ())
				chan->Author_ = channel.firstChildElement ("managingEditor").text ();
			if (chan->Author_.isEmpty ())
				chan->Author_ = channel.firstChildElement ("webMaster").text ();
			chan->PixmapURL_ = channel.firstChildElement ("image").attribute ("url");
	
			QDomElement item = channel.firstChildElement ("item");
			while (!item.isNull ())
			{
				chan->Items_.push_back (Item_ptr (ParseItem (item, chan->ChannelID_)));
				item = item.nextSiblingElement ("item");
			}
			if (!chan->LastBuild_.isValid () || chan->LastBuild_.isNull ())
			{
				if (chan->Items_.size ())
					chan->LastBuild_ = chan->Items_.at (0)->PubDate_;
				else
					chan->LastBuild_ = QDateTime::currentDateTime ();
			}
			channels.push_back (chan);
			channel = channel.nextSiblingElement ("channel");
		}
		return channels;
	}
	
	Item* RSS20Parser::ParseItem (const QDomElement& item,
			const IDType_t& channelId) const
	{
		Item *result = new Item (channelId);
		result->Title_ = UnescapeHTML (item.firstChildElement ("title").text ());
		if (result->Title_.isEmpty ())
			result->Title_ = "<>";
		result->Link_ = item.firstChildElement ("link").text ();

		result->Description_ = item.firstChildElement ("description").text ();
		GetDescription (item, result->Description_);

		QDomNodeList duration = item.elementsByTagNameNS (ITunes_, "duration");
		if (duration.size ())
		{
			if (!result->Description_.isEmpty ())
				result->Description_ += "<br /><br />";
			result->Description_ += QObject::tr ("Duration: %1")
				.arg (duration.at (0).toElement ().text ());
		}

		QString pubDateText = item.firstChildElement ("pubDate").text ();
		if (pubDateText.size ())
		{
			result->PubDate_ = RFC822TimeToQDateTime (pubDateText);
			if (!result->PubDate_.isValid () || result->PubDate_.isNull ())
				result->PubDate_ = QDateTime::currentDateTime ();
		}

		result->Guid_ = item.firstChildElement ("guid").text ();
		if (result->Guid_.isEmpty ())
			result->Guid_ = "empty";
		result->Categories_ = GetAllCategories (item);
		result->Unread_ = true;
		result->Author_ = GetAuthor (item);
		result->NumComments_ = GetNumComments (item);
		result->CommentsLink_ = GetCommentsRSS (item);
		result->CommentsPageLink_ = GetCommentsLink (item);
		result->Enclosures_ = GetEnclosures (item, result->ItemID_);
		result->Enclosures_ += GetEncEnclosures (item, result->ItemID_);
		QPair<double, double> point = GetGeoPoint (item);
		result->Latitude_ = point.first;
		result->Longitude_ = point.second;
		result->MRSSEntries_ = GetMediaRSS (item, result->ItemID_);
		return result;
	}
}
}
