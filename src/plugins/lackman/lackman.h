/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_LACKMAN_H
#define PLUGINS_LACKMAN_LACKMAN_H
#include <QWidget>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_lackman.h"

class QSortFilterProxyModel;
class QStringListModel;

namespace LeechCraft
{
namespace Util
{
	class ShortcutManager;
}
namespace LackMan
{
	class TypeFilterProxyModel;
	class StringFilterModel;

	class Plugin : public QWidget
				 , public IInfo
				 , public IHaveTabs
				 , public ITabWidget
				 , public IHaveSettings
				 , public IActionsExporter
				 , public IEntityHandler
				 , public IHaveShortcuts
				 , public IHaveRecoverableTabs
				 , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs ITabWidget IHaveSettings IActionsExporter
				IEntityHandler IHaveShortcuts IHaveRecoverableTabs IRecoverableTab)

		Ui::LackMan Ui_;
		std::auto_ptr<QTranslator> Translator_;
		StringFilterModel *FilterString_;
		TypeFilterProxyModel *TypeFilter_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		QStringListModel *TagsModel_;
		Util::ShortcutManager *ShortcutMgr_;

		QAction *UpdateAll_;
		QAction *UpgradeAll_;
		QAction *Apply_;
		QAction *Cancel_;
		QToolBar *Toolbar_;

		TabClassInfo TabClass_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		void SetShortcut (const QString&, const QKeySequences_t&);
		QMap<QString, ActionInfo> GetActionInfo () const;

		void RecoverTabs (const QList<TabRecoverInfo>& infos);

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;
	private slots:
		void handleTagsUpdated (const QStringList&);
		void on_PackageStatus__currentIndexChanged (int);
		void handlePackageSelected (const QModelIndex&);
		void handleFetchListUpdated (const QList<int>&);
	private:
		void BuildActions ();
	signals:
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);

		void tabRecoverDataChanged ();
	};
}
}

#endif
