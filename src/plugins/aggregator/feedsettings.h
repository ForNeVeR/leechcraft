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

#ifndef PLUGINS_AGGREGATOR_FEEDSETTINGS_H
#define PLUGINS_AGGREGATOR_FEEDSETTINGS_H
#include <memory>
#include <QDialog>
#include <QModelIndex>
#include <util/tags/tagscompleter.h>
#include "ui_feedsettings.h"
#include "common.h"

namespace LeechCraft
{
namespace Aggregator
{
	class FeedSettings : public QDialog
	{
		Q_OBJECT

		Ui::FeedSettings Ui_;
		std::auto_ptr<LeechCraft::Util::TagsCompleter> ChannelTagsCompleter_;
		QModelIndex Index_;
		IDType_t SettingsID_;
	public:
		FeedSettings (const QModelIndex&, QWidget* = 0);
	public slots:
		virtual void accept ();
	private slots:
		void on_UpdateFavicon__released ();
	};
}
}

#endif
