/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H

#include <QObject>
#include <QUrl>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/iaccount.h>
#include <interfaces/ibookmarksservice.h>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{

	class PluginManager;
	class AccountsSettings;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr CoreProxy_;
		QObject *PluginProxy_;
		std::shared_ptr<PluginManager> PluginManager_;
		AccountsSettings *AccountsSettings_;

		QObjectList ServicesPlugins_;
		QObjectList ActiveAccounts_;
		QHash<QString, IAccount*> Url2Account_;

		QHash<QStandardItem*, IAccount*> Item2Account_;
		QHash<QStandardItem*, IBookmarksService*> Item2Service_;

		QTimer *DownloadTimer_;
		QTimer *UploadTimer_;

		Core ();
	public:
		static Core& Instance ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		void SetPluginProxy (QObject*);

		AccountsSettings* GetAccountsSettingsWidget () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		void AddServicePlugin (QObject*);
		QObjectList GetServicePlugins () const;

		void AddActiveAccount (QObject*);
		void SetActiveAccounts (QObjectList);
		QObjectList GetActiveAccounts () const;

		void DeletePassword (QObject*);
		QString GetPassword (QObject*);
		void SavePassword (QObject*);

		QModelIndex GetServiceIndex (QObject*) const;
		QVariantList GetAllBookmarks () const;
	private:
		QObject* GetBookmarksModel () const;
		QVariantList GetUniqueBookmarks (IAccount*,
				const QVariantList&, bool byService = false);
	private slots:
		void handleGotBookmarks (QObject*, const QVariantList&);
		void handleBookmarksUploaded ();
	public slots:
		void syncBookmarks ();
		void uploadBookmarks ();
		void downloadBookmarks ();
		void downloadAllBookmarks ();

		void checkDownloadPeriod ();
		void checkUploadPeriod ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
