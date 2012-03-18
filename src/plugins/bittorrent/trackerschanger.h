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

#ifndef PLUGINS_BITTORRENT_TRACKERSCHANGER_H
#define PLUGINS_BITTORRENT_TRACKERSCHANGER_H
#include <QDialog>
#include <libtorrent/torrent_info.hpp>
#include "ui_trackerschanger.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class TrackersChanger : public QDialog
			{
				Q_OBJECT

				Ui::TrackersChanger Ui_;
			public:
				TrackersChanger (QWidget* = 0);
				void SetTrackers (const std::vector<libtorrent::announce_entry>&);
				std::vector<libtorrent::announce_entry> GetTrackers () const;
			private slots:
				void currentItemChanged (QTreeWidgetItem*);
				void on_ButtonAdd__released ();
				void on_ButtonModify__released ();
				void on_ButtonRemove__released ();
			};
		};
	};
};

#endif

