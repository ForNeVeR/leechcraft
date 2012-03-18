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

#ifndef PLUGINS_POSHUKU_ADDTOFAVORITESDIALOG_H
#define PLUGINS_POSHUKU_ADDTOFAVORITESDIALOG_H
#include <memory>
#include <QDialog>
#include <util/tagscompleter.h>
#include "ui_addtofavoritesdialog.h"

namespace LeechCraft
{
namespace Util
{
	class TagsCompletionModel;
};

namespace Poshuku
{
	class AddToFavoritesDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddToFavoritesDialog Ui_;

		std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsCompleter_;
	public:
		AddToFavoritesDialog (const QString&,
				const QString&,
				QWidget* = 0);
		virtual ~AddToFavoritesDialog ();

		QString GetTitle () const;
		QStringList GetTags () const;
	};
}
}

#endif
