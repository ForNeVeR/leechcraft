/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "account.h"
#include <stdexcept>
#include <QUuid>
#include <QDataStream>
#include <QInputDialog>
#include <QMutex>
#include <QStandardItemModel>
#include <util/xpc/util.h>
#include "core.h"
#include "accountconfigdialog.h"
#include "accountthread.h"
#include "accountthreadworker.h"
#include "storage.h"
#include "accountfoldermanager.h"
#include "mailmodel.h"
#include "taskqueuemanager.h"

Q_DECLARE_METATYPE (QList<QStringList>)
Q_DECLARE_METATYPE (QList<QByteArray>)

namespace LeechCraft
{
namespace Snails
{
	Account::Account (QObject *parent)
	: QObject (parent)
	, Thread_ (new AccountThread (true, this))
	, MessageFetchThread_ (new AccountThread (false, this))
	, AccMutex_ (new QMutex (QMutex::Recursive))
	, ID_ (QUuid::createUuid ().toByteArray ())
	, UseSASL_ (false)
	, SASLRequired_ (false)
	, UseTLS_ (true)
	, UseSSL_ (false)
	, InSecurityRequired_ (false)
	, OutSecurity_ (SecurityType::SSL)
	, OutSecurityRequired_ (false)
	, SMTPNeedsAuth_ (true)
	, APOP_ (false)
	, APOPFail_ (false)
	, FolderManager_ (new AccountFolderManager (this))
	, FoldersModel_ (new QStandardItemModel (this))
	, MailModel_ (new MailModel (this))
	{
		Thread_->start (QThread::IdlePriority);
		MessageFetchThread_->start (QThread::LowPriority);

		connect (FolderManager_,
				SIGNAL (foldersUpdated ()),
				this,
				SLOT (handleFoldersUpdated ()));
	}

	QByteArray Account::GetID () const
	{
		return ID_;
	}

	QString Account::GetName () const
	{
		return AccName_;
	}

	QString Account::GetServer () const
	{
		return InType_ == InType::Maildir ?
			QString () :
			InHost_ + ':' + QString::number (InPort_);
	}

	QString Account::GetType () const
	{
		QMutexLocker l (GetMutex ());
		switch (InType_)
		{
		case InType::IMAP:
			return "IMAP";
		case InType::POP3:
			return "POP3";
		case InType::Maildir:
			return "Maildir";
		default:
			return "<unknown>";
		}
	}

	AccountFolderManager* Account::GetFolderManager () const
	{
		return FolderManager_;
	}

	MailModel* Account::GetMailModel () const
	{
		return MailModel_;
	}

	QAbstractItemModel* Account::GetFoldersModel () const
	{
		return FoldersModel_;
	}

	void Account::ShowFolder (const QModelIndex& idx)
	{
		MailModel_->Clear ();

		const auto& path = idx.data (FoldersRole::Path).toStringList ();
		qDebug () << Q_FUNC_INFO << path;
		if (path.isEmpty ())
			return;

		MailModel_->SetFolder (path);
		MailModel_->Clear ();

		QList<Message_ptr> messages;
		const auto& ids = Core::Instance ().GetStorage ()->LoadIDs (this, path);
		for (const auto& id : ids)
			messages << Core::Instance ().GetStorage ()->LoadMessage (this, path, id);

		MailModel_->Append (messages);

		Synchronize (path, ids.isEmpty () ? QByteArray {} : ids.last ());
	}

	void Account::Synchronize (Account::FetchFlags flags)
	{
		MailModel_->Clear ();
		MailModel_->SetFolder ({ "INBOX" });

		auto folders = FolderManager_->GetSyncFolders ();
		if (folders.isEmpty ())
			folders << QStringList ("INBOX");

		Thread_->GetTaskManager ()->AddTask ({
				"synchronize",
				{
					{ flags },
					{ folders },
					QByteArray {}
				}
			});
	}

	void Account::Synchronize (const QStringList& path, const QByteArray& last)
	{
		Thread_->GetTaskManager ()->AddTask ({
				"synchronize",
				{
					Account::FetchFlags { FetchFlag::FetchAll },
					QList<QStringList> { path },
					last
				},
				"syncFolder"
			});
	}

	void Account::FetchWholeMessage (Message_ptr msg)
	{
		MessageFetchThread_->GetTaskManager ()->AddTask ({
				"fetchWholeMessage",
				{ msg }
			});
	}

	void Account::SendMessage (Message_ptr msg)
	{
		auto pair = msg->GetAddress (Message::Address::From);
		if (pair.first.isEmpty ())
			pair.first = UserName_;
		if (pair.second.isEmpty ())
			pair.second = UserEmail_;
		msg->SetAddress (Message::Address::From, pair);

		MessageFetchThread_->GetTaskManager ()->AddTask ({
				"sendMessage",
				{ msg }
			});
	}

	void Account::FetchAttachment (Message_ptr msg,
			const QString& attName, const QString& path)
	{
		MessageFetchThread_->GetTaskManager ()->AddTask ({
				"fetchAttachment",
				{
					msg,
					attName,
					path
				}
			});
	}

	void Account::SetReadStatus (bool read, const QList<QByteArray>& ids, const QStringList& folder)
	{
		MessageFetchThread_->GetTaskManager ()->AddTask ({
				"setReadStatus",
				{
					read,
					ids,
					folder
				}
			});
	}

	void Account::Update (const Message_ptr& message)
	{
		MailModel_->Update (message);
	}

	QByteArray Account::Serialize () const
	{
		QMutexLocker l (GetMutex ());

		QByteArray result;

		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (5);
		out << ID_
			<< AccName_
			<< Login_
			<< UseSASL_
			<< SASLRequired_
			<< UseTLS_
			<< UseSSL_
			<< InSecurityRequired_
			<< static_cast<qint8> (OutSecurity_)
			<< OutSecurityRequired_
			<< SMTPNeedsAuth_
			<< APOP_
			<< APOPFail_
			<< InHost_
			<< InPort_
			<< OutHost_
			<< OutPort_
			<< OutLogin_
			<< static_cast<quint8> (InType_)
			<< static_cast<quint8> (OutType_)
			<< UserName_
			<< UserEmail_
			<< FolderManager_->Serialize ();

		return result;
	}

	void Account::Deserialize (const QByteArray& arr)
	{
		QDataStream in (arr);
		quint8 version = 0;
		in >> version;

		if (version < 1 || version > 5)
			throw std::runtime_error (qPrintable ("Unknown version " + QString::number (version)));

		quint8 inType = 0, outType = 0;

		{
			QMutexLocker l (GetMutex ());
			in >> ID_
				>> AccName_
				>> Login_
				>> UseSASL_
				>> SASLRequired_
				>> UseTLS_;

			if (version >= 4)
				in >> UseSSL_;
			else
				UseSSL_ = !UseTLS_;

			in >> InSecurityRequired_;

			if (version >= 5)
			{
				qint8 type = 0;
				in >> type
					>> OutSecurityRequired_;
				OutSecurity_ = static_cast<SecurityType> (type);
			}

			in >> SMTPNeedsAuth_
				>> APOP_
				>> APOPFail_
				>> InHost_
				>> InPort_
				>> OutHost_
				>> OutPort_
				>> OutLogin_
				>> inType
				>> outType;

			InType_ = static_cast<InType> (inType);
			OutType_ = static_cast<OutType> (outType);

			if (version >= 2)
				in >> UserName_
					>> UserEmail_;

			if (version >= 3)
			{
				QByteArray fstate;
				in >> fstate;
				FolderManager_->Deserialize (fstate);

				handleFoldersUpdated ();
			}
		}
	}

	void Account::OpenConfigDialog ()
	{
		std::unique_ptr<AccountConfigDialog> dia (new AccountConfigDialog);

		{
			QMutexLocker l (GetMutex ());
			dia->SetName (AccName_);
			dia->SetUserName (UserName_);
			dia->SetUserEmail (UserEmail_);
			dia->SetLogin (Login_);
			dia->SetUseSASL (UseSASL_);
			dia->SetSASLRequired (SASLRequired_);

			if (UseSSL_)
				dia->SetInSecurity (SecurityType::SSL);
			else if (UseTLS_)
				dia->SetInSecurity (SecurityType::TLS);
			else
				dia->SetInSecurity (SecurityType::No);

			dia->SetInSecurityRequired (InSecurityRequired_);

			dia->SetOutSecurity (OutSecurity_);
			dia->SetOutSecurityRequired (OutSecurityRequired_);

			dia->SetSMTPAuth (SMTPNeedsAuth_);
			dia->SetAPOP (APOP_);
			dia->SetAPOPRequired (APOPFail_);
			dia->SetInHost (InHost_);
			dia->SetInPort (InPort_);
			dia->SetOutHost (OutHost_);
			dia->SetOutPort (OutPort_);
			dia->SetOutLogin (OutLogin_);
			dia->SetInType (InType_);
			dia->SetOutType (OutType_);

			const auto& folders = FolderManager_->GetFolders ();
			dia->SetAllFolders (folders);
			const auto& toSync = FolderManager_->GetSyncFolders ();
			Q_FOREACH (const auto& folder, folders)
			{
				const auto flags = FolderManager_->GetFolderFlags (folder);
				if (flags & AccountFolderManager::FolderOutgoing)
					dia->SetOutFolder (folder);
			}
			dia->SetFoldersToSync (toSync);
		}

		if (dia->exec () != QDialog::Accepted)
			return;

		{
			QMutexLocker l (GetMutex ());
			AccName_ = dia->GetName ();
			UserName_ = dia->GetUserName ();
			UserEmail_ = dia->GetUserEmail ();
			Login_ = dia->GetLogin ();
			UseSASL_ = dia->GetUseSASL ();
			SASLRequired_ = dia->GetSASLRequired ();

			UseSSL_ = false;
			UseTLS_ = false;
			switch (dia->GetInSecurity ())
			{
			case SecurityType::SSL:
				UseSSL_ = true;
				break;
			case SecurityType::TLS:
				UseTLS_ = true;
				break;
			case SecurityType::No:
				break;
			}

			InSecurityRequired_ = dia->GetInSecurityRequired ();
			SMTPNeedsAuth_ = dia->GetSMTPAuth ();
			APOP_ = dia->GetAPOP ();
			APOPFail_ = dia->GetAPOPRequired ();
			InHost_ = dia->GetInHost ();
			InPort_ = dia->GetInPort ();
			OutHost_ = dia->GetOutHost ();
			OutPort_ = dia->GetOutPort ();
			OutLogin_ = dia->GetOutLogin ();
			InType_ = dia->GetInType ();
			OutType_ = dia->GetOutType ();

			FolderManager_->ClearFolderFlags ();
			const auto& out = dia->GetOutFolder ();
			if (!out.isEmpty ())
				FolderManager_->AppendFolderFlags (out, AccountFolderManager::FolderOutgoing);

			Q_FOREACH (const auto& sync, dia->GetFoldersToSync ())
				FolderManager_->AppendFolderFlags (sync, AccountFolderManager::FolderSyncable);
		}

		emit accountChanged ();
	}

	bool Account::IsNull () const
	{
		return AccName_.isEmpty () ||
			Login_.isEmpty ();
	}

	QString Account::GetInUsername ()
	{
		return Login_;
	}

	QString Account::GetOutUsername ()
	{
		return OutLogin_;
	}

	QMutex* Account::GetMutex () const
	{
		return AccMutex_;
	}

	QString Account::BuildInURL ()
	{
		QMutexLocker l (GetMutex ());

		QString result;

		switch (InType_)
		{
		case InType::IMAP:
			result = UseSSL_ ? "imaps://" : "imap://";
			break;
		case InType::POP3:
			result = "pop3://";
			break;
		case InType::Maildir:
			result = "maildir://localhost";
			break;
		}

		if (InType_ != InType::Maildir)
		{
			result += Login_;
			result += ":";
			result.replace ('@', "%40");

			QString pass;
			getPassword (&pass);

			result += pass + '@';
		}

		result += InHost_;

		qDebug () << Q_FUNC_INFO << result;

		return result;
	}

	QString Account::BuildOutURL ()
	{
		QMutexLocker l (GetMutex ());

		if (OutType_ == OutType::Sendmail)
			return "sendmail://localhost";

		QString result = OutSecurity_ == SecurityType::SSL ? "smtps://" : "smtp://";

		if (SMTPNeedsAuth_)
		{
			QString pass;
			if (OutLogin_.isEmpty ())
			{
				result += Login_;
				getPassword (&pass);
			}
			else
			{
				result += OutLogin_;
				getPassword (&pass, Direction::Out);
			}
			result += ":" + pass;

			result.replace ('@', "%40");
			result += '@';
		}

		result += OutHost_;

		qDebug () << Q_FUNC_INFO << result;

		return result;
	}

	QString Account::GetPassImpl (Direction dir)
	{
		QList<QVariant> keys;
		keys << GetStoreID (dir);
		const QVariantList& result =
			Util::GetPersistentData (keys, &Core::Instance ());
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}

	QByteArray Account::GetStoreID (Account::Direction dir) const
	{
		QByteArray result = GetID ();
		if (dir == Direction::Out)
			result += "/out";
		return result;
	}

	void Account::buildInURL (QString *res)
	{
		*res = BuildInURL ();
	}

	void Account::buildOutURL (QString *res)
	{
		*res = BuildOutURL ();
	}

	void Account::getPassword (QString *outPass, Direction dir)
	{
		QString pass = GetPassImpl (dir);
		if (!pass.isEmpty ())
		{
			*outPass = pass;
			return;
		}

		pass = QInputDialog::getText (0,
				"LeechCraft",
				Account::tr ("Enter password for account %1:")
						.arg (GetName ()),
				QLineEdit::Password);
		*outPass = pass;
		if (pass.isEmpty ())
			return;

		QList<QVariant> keys;
		keys << GetStoreID (dir);

		QList<QVariant> passwordVar;
		passwordVar << pass;
		QList<QVariant> values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		Core::Instance ().SendEntity (e);
	}

	void Account::handleMsgHeaders (const QList<Message_ptr>& messages, const QStringList& folder)
	{
		qDebug () << Q_FUNC_INFO << messages.size ();
		Core::Instance ().GetStorage ()->SaveMessages (this, folder, messages);
		emit mailChanged ();

		MailModel_->Append (messages);
	}

	void Account::handleGotUpdatedMessages (const QList<Message_ptr>& messages, const QStringList& folder)
	{
		qDebug () << Q_FUNC_INFO << messages.size ();
		Core::Instance ().GetStorage ()->SaveMessages (this, folder, messages);
		emit mailChanged ();

		for (const auto& message : messages)
			MailModel_->Update (message);
	}

	void Account::handleGotOtherMessages (const QList<QByteArray>& ids, const QStringList& folder)
	{
		qDebug () << Q_FUNC_INFO << ids.size () << folder;
		QList<Message_ptr> msgs;
		for (const auto& id : ids)
			msgs << Core::Instance ().GetStorage ()->LoadMessage (this, folder, id);

		MailModel_->Append (msgs);
	}

	void Account::handleMessagesRemoved (const QList<QByteArray>& ids, const QStringList& folder)
	{
		qDebug () << Q_FUNC_INFO << ids.size () << folder;
		for (const auto& id : ids)
		{
			Core::Instance ().GetStorage ()->RemoveMessage (this, folder, id);
			MailModel_->Remove (id);
		}
	}

	void Account::handleFolderSyncFinished (const QStringList& folder, const QByteArray& lastRequestedId)
	{
		if (lastRequestedId.isEmpty ())
			return;

		Thread_->GetTaskManager ()->AddTask ({
				"getMessageCount",
				{
					folder,
					static_cast<QObject*> (this),
					QByteArray { "handleMessageCountFetched" }
				}
			});
	}

	void Account::handleMessageCountFetched (int count, const QStringList& folder)
	{
		const auto storedCount = Core::Instance ().GetStorage ()->GetNumMessages (this, folder);
		if (count != storedCount)
			Synchronize (folder, {});
	}

	namespace
	{
		QStandardItem* BuildFolderItem (QStringList folder, QStandardItem *root)
		{
			if (folder.isEmpty ())
				return root;

			const QString name = folder.takeFirst ();
			for (int i = 0; i < root->rowCount (); ++i)
				if (root->child (i)->text () == name)
					return BuildFolderItem (folder, root->child (i));

			QStandardItem *item = new QStandardItem (name);
			root->appendRow (item);
			return BuildFolderItem (folder, item);
		}
	}

	void Account::handleGotFolders (QList<QStringList> folders)
	{
		FolderManager_->SetFolders (folders);
	}

	void Account::handleFoldersUpdated ()
	{
		FoldersModel_->clear ();
		for (const auto& folder : FolderManager_->GetFolders ())
		{
			auto item = BuildFolderItem (folder, FoldersModel_->invisibleRootItem ());
			item->setData (folder, FoldersRole::Path);
			item->setEditable (false);
		}
	}

	void Account::handleMessageBodyFetched (Message_ptr msg)
	{
		for (const auto& folder : msg->GetFolders ())
			Core::Instance ().GetStorage ()->SaveMessages (this, folder, { msg });
		emit messageBodyFetched (msg);
	}
}
}
