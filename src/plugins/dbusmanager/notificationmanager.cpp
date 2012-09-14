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

#include "notificationmanager.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include <QIcon>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace DBusManager
{
	NotificationManager::NotificationManager (QObject *parent)
	: QObject (parent)
	{
		if (!QDBusConnection::sessionBus ().interface ()->
				isServiceRegistered ("org.freedesktop.Notifications"))
		{
			qWarning () << Q_FUNC_INFO
				<< QDBusConnection::sessionBus ().interface ()->registeredServiceNames ().value ();
			return;
		}

		Connection_.reset (new QDBusInterface ("org.freedesktop.Notifications",
					"/org/freedesktop/Notifications"));
		if (!Connection_->isValid ())
		{
			qWarning () << Q_FUNC_INFO
				<< Connection_->lastError ();
		}

		connect (Connection_.get (),
				SIGNAL (ActionInvoked (uint, QString)),
				this,
				SLOT (handleActionInvoked (uint, QString)));
		connect (Connection_.get (),
				SIGNAL (NotificationClosed (uint, uint)),
				this,
				SLOT (handleNotificationClosed (uint)));
	}

	bool NotificationManager::CouldNotify (const Entity& e) const
	{
		return XmlSettingsManager::Instance ()->
				property ("UseNotifications").toBool () &&
			Connection_.get () &&
			Connection_->isValid () &&
			e.Mime_ == "x-leechcraft/notification" &&
			e.Additional_ ["Priority"].toInt () != PLog_ &&
			!e.Additional_ ["Text"].toString ().isEmpty ();
	}

	void NotificationManager::HandleNotification (const Entity& e)
	{
		if (!Connection_.get () ||
				!XmlSettingsManager::Instance ()->
					property ("UseNotifications").toBool ())
			return;

		QStringList actions = e.Additional_ ["NotificationActions"].toStringList ();
		if (actions.isEmpty ())
			DoNotify (e, false);
		else
		{
			CapCheckData cd =
			{
				e
			};

			QDBusPendingCall pending = Connection_->
					asyncCall ("GetCapabilities");
			QDBusPendingCallWatcher *watcher =
				new QDBusPendingCallWatcher (pending, this);
			Watcher2CapCheck_ [watcher] = cd;
			connect (watcher,
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (handleCapCheckCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	void NotificationManager::DoNotify (const Entity& e, bool hasActions)
	{
		Priority prio = static_cast<Priority> (e.Additional_ ["Priority"].toInt ());
		QString header = e.Entity_.toString ();
		QString text = e.Additional_ ["Text"].toString ();
		bool uus = e.Additional_ ["UntilUserSees"].toBool ();

		QStringList fmtActions;
		QStringList actions;
		if (hasActions)
		{
			actions = e.Additional_ ["NotificationActions"].toStringList ();
			int i = 0;
			Q_FOREACH (QString action, actions)
				fmtActions << QString::number (i++) << action;
		}

		if (prio == PLog_)
			return;

		int timeout = 0;
		if (!uus &&
				Core::Instance ().GetProxy ())
			timeout = Core::Instance ().GetProxy ()->GetSettingsManager ()->
					property ("FinishedDownloadMessageTimeout").toInt () * 1000;

		QList<QVariant> arguments;
		arguments << header
			<< uint (0)
			<< QString ("leechcraft_main")
			<< QString ()
			<< text
			<< fmtActions
			<< QVariantMap ()
			<< timeout;

		ActionData ad =
		{
			e,
			e.Additional_ ["HandlingObject"].value<QObject_ptr> (),
			actions
		};

		auto pending = Connection_->asyncCallWithArgumentList ("Notify", arguments);
		auto watcher = new QDBusPendingCallWatcher (pending, this);
		Watcher2AD_ [watcher] = ad;
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleNotificationCallFinished (QDBusPendingCallWatcher*)));
	}

	void NotificationManager::handleNotificationCallFinished (QDBusPendingCallWatcher *w)
	{
		QDBusPendingReply<uint> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
				<< reply.error ().name ()
				<< reply.error ().message ();
			return;
		}
		int id = reply.argumentAt<0> ();
		CallID2AD_ [id] = Watcher2AD_ [w];
		Watcher2AD_.remove (w);

		w->deleteLater ();
	}

	void NotificationManager::handleCapCheckCallFinished (QDBusPendingCallWatcher *w)
	{
		QDBusPendingReply<QStringList> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to handle notification, failed to query caps:"
				<< reply.error ().name ()
				<< reply.error ().message ();
			return;
		}
		QStringList caps = reply.argumentAt<0> ();
		bool hasActions = caps.contains ("actions");
		Entity e = Watcher2CapCheck_.take (w).Entity_;
		DoNotify (e, hasActions);
	}

	void NotificationManager::handleActionInvoked (uint id, QString action)
	{
		const ActionData& ad = CallID2AD_.take (id);
		if (!ad.Handler_)
		{
			qWarning () << Q_FUNC_INFO
					<< "handler already destroyed";
			return;
		}

		int idx = action.toInt ();

		QMetaObject::invokeMethod (ad.Handler_.get (),
				"notificationActionTriggered",
				Qt::QueuedConnection,
				Q_ARG (int, idx));
	}

	void NotificationManager::handleNotificationClosed (uint id)
	{
		CallID2AD_.remove (id);
	}
}
}
