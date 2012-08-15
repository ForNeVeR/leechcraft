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

#include <boost/optional.hpp>
#include <QObject>
#include <phonon/mediasource.h>
#include "interfaces/lmp/iplaylistprovider.h"

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace LeechCraft
{
namespace LMP
{
	class MediaInfo;
	class StaticPlaylistManager;

	class PlaylistManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		QStandardItem *StaticRoot_;

		StaticPlaylistManager *Static_;

		enum PlaylistTypes
		{
			Other,
			Static,
			Random50
		};
		enum Roles
		{
			PlaylistType = IPlaylistProvider::ItemRoles::Max + 1
		};

		QObjectList PlaylistProviders_;
	public:
		PlaylistManager (QObject* = 0);

		QAbstractItemModel* GetPlaylistsModel () const;
		StaticPlaylistManager* GetStaticManager () const;

		void AddProvider (QObject*);

		QList<Phonon::MediaSource> GetSources (const QModelIndex&) const;

		boost::optional<MediaInfo> TryResolveMediaInfo (const QUrl&) const;
	private slots:
		void handleStaticPlaylistsChanged ();
	};
}
}
