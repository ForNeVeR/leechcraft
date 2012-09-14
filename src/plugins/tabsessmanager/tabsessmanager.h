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

#ifndef PLUGINS_TABSESSMANAGER_TABSESSMANAGER_H
#define PLUGINS_TABSESSMANAGER_TABSESSMANAGER_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihaverecoverabletabs.h>

namespace LeechCraft
{
namespace TabSessManager
{
	class Plugin : public QObject
				 , public IInfo
				 , public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter)

		ICoreProxy_ptr Proxy_;
		QList<QObject*> Tabs_;
		bool IsRecovering_;

		bool IsScheduled_;

		QMenu *SessMgrMenu_;
		struct TabUncloseInfo
		{
			TabRecoverInfo RecInfo_;
			IHaveRecoverableTabs *Plugin_;
		};
		QHash<QAction*, TabUncloseInfo> UncloseAct2Data_;

		QMenu *UncloseMenu_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		QByteArray GetCurrentSession () const;
		void AddCustomSession (const QString&);
	private slots:
		void handleNewTab (const QString&, QWidget*);
		void handleRemoveTab (QWidget*);
		void handleTabMoved (int, int);
		void handleUnclose ();
		void recover ();
		void handleTabRecoverDataChanged ();
		void saveDefaultSession ();
		void saveCustomSession ();
		void loadCustomSession ();
	signals:
		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

#endif
