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

#include "readitlateraccount.h"
#include <QDataStream>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/ibookmarksservice.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterAccount::ReadItLaterAccount (const QString& login, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, ParentService_ (parent)
	, IsSyncing_ (false)
	, LastUpload_ (QDateTime::fromString ("01.01.1980", "ddMMyyyy"))
	, LastDownload_ (QDateTime::fromString ("01.01.1980", "ddMMyyyy"))
	{
	}

	QObject* ReadItLaterAccount::GetQObject ()
	{
		return this;
	}

	QObject* ReadItLaterAccount::GetParentService () const
	{
		return ParentService_;
	}

	QByteArray ReadItLaterAccount::GetAccountID () const
	{
		return QString ("org.LeechCraft.Poshuku.OnlineBookmarks.ReadItLater.%1")
				.arg (Login_).toUtf8 ();
	}

	QString ReadItLaterAccount::GetLogin () const
	{
		return Login_;
	}

	QString ReadItLaterAccount::GetPassword () const
	{
		return Password_;
	}

	void ReadItLaterAccount::SetPassword (const QString& pass)
	{
		Password_ = pass;
	}

	bool ReadItLaterAccount::IsSyncing () const
	{
		return IsSyncing_;
	}

	void ReadItLaterAccount::SetSyncing (bool sync)
	{
		IsSyncing_ = sync;
	}

	QDateTime ReadItLaterAccount::GetLastDownloadDateTime () const
	{
		return LastDownload_;
	}

	QDateTime ReadItLaterAccount::GetLastUploadDateTime () const
	{
		return LastUpload_;
	}

	void ReadItLaterAccount::SetLastDownloadDateTime(const QDateTime& date)
	{
		LastDownload_ = date;
	}

	void ReadItLaterAccount::AppendDownloadedBookmarks (const QVariantList& bookmarks)
	{
		Q_FOREACH (const QVariant& var, bookmarks)
			if (!DownloadedBookmarks_.contains (var))
				DownloadedBookmarks_ << var;
	}

	QVariantList ReadItLaterAccount::GetBookmarksDiff (const QVariantList& list)
	{
		QVariantList diff;
		Q_FOREACH (const QVariant& var, list)
			if (!DownloadedBookmarks_.contains (var))
				diff << var;

		return diff;
	}

	void ReadItLaterAccount::SetLastUploadDateTime(const QDateTime& date)
	{
		LastUpload_ = date;
	}

	QByteArray ReadItLaterAccount::Serialize () const
	{
		quint16 version = 1;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
					<< Login_
					<< IsSyncing_
					<< LastUpload_
					<< LastDownload_
					<< DownloadedBookmarks_;
		}

		return result;
	}

	ReadItLaterAccount* ReadItLaterAccount::Deserialize (const QByteArray& data,
			QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString login;
		in >> login;
		ReadItLaterAccount *acc = new ReadItLaterAccount (login, parent);
		in >> acc->IsSyncing_
			>> acc->LastUpload_
			>> acc->LastDownload_
			>> acc->DownloadedBookmarks_;
		return acc;
	}

}
}
}
}
