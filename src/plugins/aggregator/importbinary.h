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

#ifndef PLUGINS_AGGREGATOR_IMPORTBINARY_H
#define PLUGINS_AGGREGATOR_IMPORTBINARY_H
#include <QDialog>
#include "ui_importbinary.h"
#include "feed.h"
#include "channel.h"

namespace LeechCraft
{
namespace Aggregator
{
	class ImportBinary : public QDialog
	{
		Q_OBJECT

		Ui::ImportBinary Ui_;
		channels_container_t Channels_;
	public:
		ImportBinary (QWidget* = 0);
		virtual ~ImportBinary ();
		QString GetFilename () const;
		QString GetTags () const;
		feeds_container_t GetSelectedFeeds () const;
	private slots:
		void on_File__textEdited (const QString&);
		void on_Browse__released ();
	private:
		bool HandleFile (const QString&);
		void Reset ();
	};
}
}

#endif
