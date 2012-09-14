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

#pragma once

#include <QObject>
#include <interfaces/media/ieventsprovider.h>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	class RecEventsFetcher;

	class EventsFetchAggregator : public QObject
	{
		Q_OBJECT

		QList<RecEventsFetcher*> PendingFetchers_;
		Media::EventInfos_t Aggregated_;
	public:
		EventsFetchAggregator (QObject* = 0);

		void AddFetcher (RecEventsFetcher*);
	private slots:
		void handleGot (const Media::EventInfos_t&);
	signals:
		void gotRecommendedEvents (const Media::EventInfos_t&);
	};
}
}
