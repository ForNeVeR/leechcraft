/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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

#pragma once

#include <memory>
#include <QObject>
#include <QUrl>
#include <interfaces/netstoremanager/istorageaccount.h>
#include <interfaces/netstoremanager/isupportfilelistings.h>
#include "drivemanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace DBox
{
	class Account;
	class Syncer;

	typedef std::shared_ptr<Account> Account_ptr;

	class Account : public QObject
					, public IStorageAccount
					, public ISupportFileListings
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::NetStoreManager::IStorageAccount
				LeechCraft::NetStoreManager::ISupportFileListings)


		QObject *ParentPlugin_;
		QString Name_;
		QString AccessToken_;
		QString UserID_;

		bool Trusted_;

		DriveManager *DriveManager_;
		QHash<QString, DBoxItem> Items_;

	public:
		Account (const QString& name, QObject *parentPlugin = 0);

		QObject* GetQObject ();
		QObject* GetParentPlugin () const;
		QByteArray GetUniqueID () const;
		AccountFeatures GetAccountFeatures () const;
		QString GetAccountName () const;
		void Upload (const QString& filepath,
				const QByteArray& parentId = QByteArray (),
				UploadType ut = UploadType::Upload,
				const QByteArray& id = QByteArray ());
		void Download (const QByteArray& id, const QString& filepath,
				TaskParameters tp, bool silent = false, bool open = false);

		ListingOps GetListingOps () const;
		HashAlgorithm GetCheckSumAlgorithm () const;

		void RefreshListing ();
		void RefreshChildren (const QByteArray& parentId);
		void RequestUrl (const QByteArray& id);
		void CreateDirectory (const QString& name, const QByteArray& parentId);
		void Delete (const QList<QByteArray>& ids, bool ask = true);
		void Copy (const QList<QByteArray>& ids, const QByteArray& newParentId);
		void Move (const QList<QByteArray>& ids, const QByteArray& newParentId);

		void MoveToTrash (const QList<QByteArray>& ids);
		void RestoreFromTrash (const QList<QByteArray>& ids);

		void Rename (const QByteArray& id, const QString& newName);
		void RequestChanges ();

		QByteArray Serialize ();
		static Account_ptr Deserialize (const QByteArray& data, QObject *parentPlugin);

		bool IsTrusted () const;
		void SetTrusted (bool trust);

		void SetAccessToken (const QString& token);
		QString GetAccessToken () const;
		void SetUserID (const QString& uid);

		DriveManager* GetDriveManager () const;
	private slots:
		void handleFileList (const QList<DBoxItem>& items);
		void handleSharedFileUrl (const QUrl& url, const QDateTime& expiredDate);
		void handleGotNewItem (const DBoxItem& item);

	signals:
		void upError (const QString& error, const QString& filepath);
		void upFinished (const QByteArray& id, const QString& filepath);
		void upProgress (quint64 done, quint64 total, const QString& filepath);
		void upStatusChanged (const QString& status, const QString& filepath);

		void gotListing (const QList<StorageItem>& items);
		void listingUpdated ();
		void gotFileUrl (const QUrl& url, const QByteArray& id);

		void gotChanges (const QList<Change>& changes);

		void gotNewItem (const StorageItem& item, const QByteArray& parentId);
	};
}
}
}

