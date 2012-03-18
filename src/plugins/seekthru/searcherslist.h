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

#ifndef PLUGINS_SEEKTHRU_SEARCHERSLIST_H
#define PLUGINS_SEEKTHRU_SEARCHERSLIST_H
#include <QWidget>
#include "ui_searcherslist.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SearchersList : public QWidget
			{
				Q_OBJECT

				Ui::SearchersList Ui_;
				QModelIndex Current_;
			public:
				SearchersList (QWidget* = 0);
			private slots:
				void handleCurrentChanged (const QModelIndex&);
				void on_ButtonAdd__released ();
				void on_ButtonRemove__released ();
				void on_Tags__editingFinished ();
			};
		};
	};
};

#endif

