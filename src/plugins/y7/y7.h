/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy, Von Never
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

#pragma once

#include <QList>
#include <QObject>

#include <ShObjIdl.h>
#include <Windows.h>

#include <interfaces/iinfo.h>

class QAbstractItemModel;
class QModelIndex;
class IJobHolder;

namespace LeechCraft
{
namespace Y7
{
	class Plugin : public QObject
					, public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		ICoreProxy_ptr Proxy_;
		ITaskbarList3 *Taskbar_;
		QList<IJobHolder*> JobHolders_;
		QAbstractItemModel *ProgressModel_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
	private:
		void InitProgressModel ();
		void SetProgress ();
	private slots:		
		void initButtons ();

		void initProgress ();
		void progressRowsInserted (const QModelIndex &parent, int start, int end);
		void progressDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);

		void initTabs ();
	};
}
}
