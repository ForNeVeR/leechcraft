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

#ifndef NETWORKDISKCACHE_H
#define NETWORKDISKCACHE_H
#include <QNetworkDiskCache>
#include <QMutex>

namespace LeechCraft
{
	class NetworkDiskCache : public QNetworkDiskCache
	{
		Q_OBJECT

		bool IsCollectingGarbage_;
		qint64 PreviousSize_;

		QMutex InsertRemoveMutex_;
	public:
		NetworkDiskCache (QObject* = 0);

		virtual QIODevice* prepare (const QNetworkCacheMetaData&);

		virtual void insert (QIODevice *device);
		virtual bool remove (const QUrl &url);
	protected:
		virtual qint64 expire ();
	public slots:
		void handleCacheSize ();
		void collectGarbage ();
	private slots:
		void handleCollectorFinished ();
	};
};

#endif

