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

#include "metaaccount.h"
#include <QtDebug>
#include "core.h"
#include "metaprotocol.h"
#include "metaentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	MetaAccount::MetaAccount (MetaProtocol *parent)
	: QObject (parent)
	, Parent_ (parent)
	{
		Core::Instance ().SetMetaAccount (this);
	}

	QObject* MetaAccount::GetQObject ()
	{
		return this;
	}

	QObject* MetaAccount::GetParentProtocol () const
	{
		return Parent_;
	}

	IAccount::AccountFeatures MetaAccount::GetAccountFeatures () const
	{
		return FMUCsSupportFileTransfers;
	}

	QList<QObject*> MetaAccount::GetCLEntries ()
	{
		return Core::Instance ().GetEntries ();
	}

	QString MetaAccount::GetAccountName () const
	{
		return tr ("Metacontacts");
	}

	QString MetaAccount::GetOurNick () const
	{
		return "R";
	}

	void MetaAccount::RenameAccount (const QString&)
	{
	}

	QByteArray MetaAccount::GetAccountID () const
	{
		return "org.LeechCraft.Azoth.Accounts.MetaAccount";
	}

	QList<QAction*> MetaAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void MetaAccount::QueryInfo (const QString&)
	{
	}

	void MetaAccount::OpenConfigurationDialog ()
	{
	}

	EntryStatus MetaAccount::GetState () const
	{
		return EntryStatus (SOnline, QString ());
	}

	void MetaAccount::ChangeState (const EntryStatus&)
	{
	}

	void MetaAccount::Authorize (QObject*)
	{
	}

	void MetaAccount::DenyAuth (QObject*)
	{
	}

	void MetaAccount::RequestAuth (const QString&, const QString&,
			const QString&, const QStringList&)
	{
	}

	void MetaAccount::RemoveEntry (QObject *entryObj)
	{
		MetaEntry *entry = qobject_cast<MetaEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< entryObj
					<< "to MetaEntry";
			return;
		}

		Core::Instance ().RemoveEntry (entry);
	}

	QObject* MetaAccount::GetTransferManager () const
	{
		return 0;
	}
}
}
}
