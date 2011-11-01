/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_MAINWIDGET_H
#define PLUGINS_AZOTH_MAINWIDGET_H
#include <QWidget>
#include "interfaces/iaccount.h"
#include "ui_mainwidget.h"

class QToolButton;
class QMenu;

namespace LeechCraft
{
namespace Azoth
{
	class SortFilterProxyModel;
	class ConsoleWidget;

	class MainWidget : public QWidget
	{
		Q_OBJECT

		Ui::MainWidget Ui_;

		QMenu *MainMenu_;
		QToolButton *MenuButton_;
		SortFilterProxyModel *ProxyModel_;

		QHash<IAccount*, ConsoleWidget*> Account2CW_;

		QMenu *MenuChangeStatus_;
		QMenu *TrayChangeStatus_;

		QAction *AccountJoinConference_;
		QAction *AccountManageBookmarks_;
		QAction *AccountAddContact_;
		QAction *AccountSetActivity_;
		QAction *AccountSetMood_;
		QAction *AccountSetLocation_;
		QAction *AccountConsole_;

		QMap<QString, bool> FstLevelExpands_;
		QMap<QString, QMap<QString, bool> > SndLevelExpands_;
	public:
		MainWidget (QWidget* = 0);

		QList<QAction*> GetMenuActions ();
		QMenu* GetChangeStatusMenu () const;
	private:
		void CreateMenu ();
		QMenu* CreateStatusChangeMenu (const char*, bool withCustom = false);
		void UpdateFastStatusButton (State);
		IAccount* GetAccountFromSender (const char*);
	private slots:
		void on_CLTree__activated (const QModelIndex&);
		void on_CLTree__customContextMenuRequested (const QPoint&);
		void handleChangeStatusRequested ();
		void fastStateChangeRequested ();
		void applyFastStatus ();

		void handleCatRenameTriggered ();
		void joinAccountConference ();
		void joinAccountConfFromBM ();
		void manageAccountBookmarks ();
		void addAccountContact ();
		void handleAccountSetActivity ();
		void handleAccountSetMood ();
		void handleAccountSetLocation ();
		void handleAccountConsole ();

		void handleManageBookmarks ();
		void handleAddAccountRequested ();
		void handleAddContactRequested ();

		void handleShowOffline (bool);
		void clearFilter ();

		void handleEntryMadeCurrent (QObject*);
		void on_RosterMode__currentIndexChanged (int);
		void menuBarVisibilityToggled ();

		void handleRowsInserted (const QModelIndex&, int, int);
		void rebuildTreeExpansions ();
		void expandIndex (const QPersistentModelIndex&);
		void on_CLTree__expanded (const QModelIndex&);
		void on_CLTree__collapsed (const QModelIndex&);

		void consoleRemoved (QWidget*);
	signals:
		void gotConsoleWidget (ConsoleWidget*);
	};
}
}

#endif
