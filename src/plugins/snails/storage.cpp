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

#include "storage.h"
#include <stdexcept>
#include <QFile>
#include <QApplication>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	Storage::Storage (QObject *parent)
	: QObject (parent)
	, Settings_ (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Snails_Storage")
	{
		SDir_ = Util::CreateIfNotExists ("snails/storage");
	}

	void Storage::SaveMessages (Account *acc, const QList<Message_ptr>& msgs)
	{
		const QDir& dir = DirForAccount (acc);

		Q_FOREACH (Message_ptr msg, msgs)
		{
			if (msg->GetID ().isEmpty ())
				continue;

			const QString dirName = msg->GetID ().toHex ().left (2);

			QDir msgDir = dir;
			if (!dir.exists (dirName))
				msgDir.mkdir (dirName);
			if (!msgDir.cd (dirName))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd into"
						<< msgDir.filePath (dirName);
				continue;
			}

			QFile file (msgDir.filePath (msg->GetID ().toHex ()));
			file.open (QIODevice::WriteOnly);
			file.write (qCompress (msg->Serialize (), 9));

			qApp->processEvents ();
		}
	}

	QList<Message_ptr> Storage::LoadMessages (Account *acc)
	{
		QList<Message_ptr> result;

		const QDir& dir = DirForAccount (acc);
		Q_FOREACH (auto str, dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
		{
			QDir subdir = dir;
			if (!subdir.cd (str))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< str;
				continue;
			}

			Q_FOREACH (auto str, subdir.entryList (QDir::NoDotAndDotDot | QDir::Files))
			{
				QFile file (subdir.filePath (str));
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to open"
							<< str
							<< file.errorString ();
					continue;
				}

				Message_ptr msg (new Message);
				try
				{
					msg->Deserialize (qUncompress (file.readAll ()));
					result << msg;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "error deserializing the message from"
							<< file.fileName ()
							<< e.what ();
					continue;
				}
			}
		}

		return result;
	}

	Message_ptr Storage::LoadMessage (Account *acc, const QByteArray& id)
	{
		QDir dir = DirForAccount (acc);
		if (!dir.cd (id.toHex ().left (2)))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd to"
					<< dir.filePath (id.toHex ().left (2));
			throw std::runtime_error ("Unable to cd to the directory");
		}

		QFile file (dir.filePath (id.toHex ()));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< file.fileName ()
					<< file.errorString ();
			throw std::runtime_error ("Unable to open the message file");
		}

		Message_ptr msg (new Message);
		try
		{
			msg->Deserialize (qUncompress (file.readAll ()));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error deserializing the message from"
					<< file.fileName ()
					<< e.what ();
			throw;
		}

		return msg;
	}

	int Storage::GetNumMessages (Account *acc) const
	{
		int result = 0;

		const QDir& dir = DirForAccount (acc);
		Q_FOREACH (auto str, dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
		{
			QDir subdir = dir;
			if (!subdir.cd (str))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< str;
				continue;
			}

			result += subdir.entryList (QDir::NoDotAndDotDot | QDir::Files).size ();
		}

		return result;
	}

	QDir Storage::DirForAccount (Account *acc) const
	{
		const QByteArray& id = acc->GetID ().toHex ();

		QDir dir = SDir_;
		if (!dir.exists (id))
			dir.mkdir (id);
		if (!dir.cd (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd into"
					<< dir.filePath (id);
			throw std::runtime_error ("Unable to cd to the dir");
		}

		return dir;
	}
}
}
