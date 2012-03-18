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

#include "notificationactionhandler.h"

namespace LeechCraft
{
namespace Util
{
	NotificationActionHandler::NotificationActionHandler (Entity& e, QObject *parent)
	: Entity_ (e)
	{
		Entity_.Additional_ ["HandlingObject"] = QVariant::fromValue<QObject_ptr> (QObject_ptr (this));
	}

	void NotificationActionHandler::AddFunction (const QString& name, Callback_t callback)
	{
		ActionName2Callback_ << qMakePair (name, callback);
		const QStringList& sl = Entity_.Additional_ ["NotificationActions"].toStringList ();
		Entity_.Additional_ ["NotificationActions"] = sl + QStringList (name);
	}

	void NotificationActionHandler::AddDependentObject (QObject *obj)
	{
		DependentObjects_ << QPointer<QObject> (obj);
	}

	void NotificationActionHandler::notificationActionTriggered (int idx)
	{
		Q_FOREACH (const QPointer<QObject>& obj, DependentObjects_)
			if (obj.isNull ())
				return;

		ActionName2Callback_.at (idx).second ();
	}
}
}
