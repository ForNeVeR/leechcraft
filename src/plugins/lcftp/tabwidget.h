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

#ifndef PLUGINS_LCFTP_TABWIDGET_H
#define PLUGINS_LCFTP_TABWIDGET_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_tabwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class TabWidget : public QWidget
							, public ITabWidget
			{
				Q_OBJECT
				Q_INTERFACES (ITabWidget)

				Ui::TabWidget Ui_;
				static QObject *S_ParentMultiTabs_;
			public:
				TabWidget (const QUrl& url, const QString& str, QWidget* = 0);
				virtual ~TabWidget ();
				static void SetParentMultiTabs (QObject*);

				void Remove ();
				QToolBar* GetToolBar () const;
				QList<QAction*> GetTabBarContextMenuActions () const;
				QObject *ParentMultiTabs ();
				TabClassInfo GetTabClassInfo () const;
			private:
				void Setup (Pane*);
				Pane* Other (Pane*);
			private slots:
				void handleDownloadRequested (const QUrl&);
				void handleUploadRequested (const QString&);
			};

			typedef TabWidget *TabWidget_ptr;
		};
	};
};

#endif

