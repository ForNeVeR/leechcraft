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

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/isyncable.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
namespace Otlozhu
{
	class Plugin : public QObject
					, public IInfo
					, public IHaveTabs
					, public IEntityHandler
					, public ISyncable
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IEntityHandler ISyncable)

		TabClassInfo TCTodo_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		Sync::ChainIDs_t AvailableChains () const;
		Sync::Payloads_t GetAllDeltas (const Sync::ChainID_t&) const;
		Sync::Payloads_t GetNewDeltas (const Sync::ChainID_t&) const;
		void PurgeNewDeltas (const Sync::ChainID_t&, quint32);
		void ApplyDeltas (const Sync::Payloads_t&, const Sync::ChainID_t&);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void newDeltasAvailable (const LeechCraft::Sync::ChainID_t&);

		void gotEntity (const LeechCraft::Entity&);
	};
}
}
