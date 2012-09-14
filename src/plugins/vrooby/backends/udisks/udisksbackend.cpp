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

#include "udisksbackend.h"
#include <memory>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QTimer>
#include <QtDebug>
#include <util/util.h>

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LeechCraft
{
namespace Vrooby
{
namespace UDisks
{
	Backend::Backend (QObject *parent)
	: DevBackend (parent)
	, Valid_ (false)
	, DevicesModel_ (new QStandardItemModel (this))
	, UDisksObj_ (0)
	{
		InitialEnumerate ();

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (updateDeviceSpaces ()));
		timer->start (10000);
	}

	bool Backend::IsValid () const
	{
		return Valid_;
	}

	QAbstractItemModel* Backend::GetDevicesModel () const
	{
		return DevicesModel_;
	}

	namespace
	{
		QDBusInterface_ptr GetDeviceInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks",
						path,
						"org.freedesktop.UDisks.Device",
						QDBusConnection::systemBus ()));
		}
	}

	void Backend::MountDevice (const QString& id)
	{
		auto iface = GetDeviceInterface (id);
		if (!iface)
			return;

		if (!iface->property ("DeviceIsMounted").toBool ())
		{
			auto async = iface->asyncCall ("FilesystemMount", QString (), QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (mountCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	void Backend::InitialEnumerate ()
	{
		auto sb = QDBusConnection::systemBus ();
		auto iface = sb.interface ();
		const auto& services = iface->registeredServiceNames ()
				.value ().filter ("org.freedesktop.UDisks");
		if (services.isEmpty ())
			return;

		UDisksObj_ = new QDBusInterface ("org.freedesktop.UDisks", "/org/freedesktop/UDisks", "org.freedesktop.UDisks", sb);
		auto async = UDisksObj_->asyncCall ("EnumerateDevices");
		auto watcher = new QDBusPendingCallWatcher (async, this);
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleEnumerationFinished (QDBusPendingCallWatcher*)));

		connect (UDisksObj_,
				SIGNAL (DeviceAdded (QDBusObjectPath)),
				this,
				SLOT (handleDeviceAdded (QDBusObjectPath)));
		connect (UDisksObj_,
				SIGNAL (DeviceRemoved (QDBusObjectPath)),
				this,
				SLOT (handleDeviceRemoved (QDBusObjectPath)));
		connect (UDisksObj_,
				SIGNAL (DeviceChanged (QDBusObjectPath)),
				this,
				SLOT (handleDeviceChanged (QDBusObjectPath)));
	}

	bool Backend::AddPath (const QDBusObjectPath& path)
	{
		const auto& str = path.path ();
		if (Object2Item_.contains (str))
			return true;

		auto iface = GetDeviceInterface (str);

		const auto& slaveTo = iface->property ("PartitionSlave").value<QDBusObjectPath> ();
		const bool isSlave = slaveTo.path () != "/";
		const bool isRemovable = iface->property ("DeviceIsRemovable").toBool ();
		qDebug () << str << slaveTo.path () << isSlave;
		if ((!isSlave && !isRemovable) || Unremovables_.contains (slaveTo.path ()))
		{
			Unremovables_ << str;
			return false;
		}

		auto item = new QStandardItem;
		Object2Item_ [str] = item;
		SetItemData (iface, item);
		if (!isSlave)
			DevicesModel_->appendRow (item);
		else
		{
			if (!Object2Item_.contains (slaveTo.path ()))
				if (!AddPath (slaveTo))
					return false;
			Object2Item_ [slaveTo.path ()]->appendRow (item);
		}
		return true;
	}

	void Backend::RemovePath (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		auto item = Object2Item_.take (path);
		if (!item)
			return;

		auto getChildren = [] (QStandardItem *item)
		{
			QList<QStandardItem*> result;
			for (int i = 0; i < item->rowCount (); ++i)
				result << item->child (i);
			return result;
		};

		QList<QStandardItem*> toRemove = getChildren (item);
		for (int i = 0; i < toRemove.size (); ++i)
			toRemove += getChildren (toRemove [i]);

		for (QStandardItem *item : toRemove)
			Object2Item_.remove (Object2Item_.key (item));

		if (item->parent ())
			item->parent ()->removeRow (item->row ());
		else
			DevicesModel_->removeRow (item->row ());
	}

	void Backend::SetItemData (QDBusInterface_ptr iface, QStandardItem *item)
	{
		if (!item)
			return;

		const bool isRemovable = iface->property ("DeviceIsRemovable").toBool ();
		const bool isPartition = iface->property ("DeviceIsPartition").toBool ();

		const auto& vendor = iface->property ("DriveVendor").toString () +
				" " +
				iface->property ("DriveModel").toString ();
		const auto& partLabel = iface->property ("PartitionLabel").toString ().trimmed ();
		const auto& partName = partLabel.isEmpty () ?
				tr ("Partition %1")
						.arg (iface->property ("PartitionNumber").toInt ()) :
				partLabel;
		const auto& name = isPartition ? partName : vendor;
		const auto& fullName = isPartition ?
				QString ("%1: %2").arg (vendor, partName) :
				vendor;

		auto parentIface = iface;
		bool hasRemovableParent = isRemovable;
		while (!hasRemovableParent)
		{
			const auto& slaveTo = parentIface->property ("PartitionSlave").value<QDBusObjectPath> ();
			if (slaveTo.path () == "/")
				break;

			parentIface = GetDeviceInterface (slaveTo.path ());
			hasRemovableParent = parentIface->property ("DeviceIsRemovable").toBool ();
		}

		DevicesModel_->blockSignals (true);

		const auto& mountPaths = iface->property ("DeviceMountPaths").toStringList ();
		if (!mountPaths.isEmpty ())
		{
			const auto& space = boost::filesystem::space (mountPaths.value (0).toStdWString ()).free;
			item->setData (static_cast<qint64> (space), DeviceRoles::AvailableSize);
		}
		else
			item->setData (-1, DeviceRoles::AvailableSize);

		item->setText (name);
		item->setData (DeviceType::GenericDevice, DeviceRoles::DevType);
		item->setData (iface->property ("DeviceFile").toString (), DeviceRoles::DevFile);
		item->setData (iface->property ("PartitionType").toInt (), DeviceRoles::PartType);
		item->setData (isRemovable, DeviceRoles::IsRemovable);
		item->setData (isPartition, DeviceRoles::IsPartition);
		item->setData (isPartition && hasRemovableParent, DeviceRoles::IsMountable);
		item->setData (iface->property ("DeviceIsMounted").toBool (), DeviceRoles::IsMounted);
		item->setData (iface->property ("DeviceIsMediaAvailable"), DeviceRoles::IsMediaAvailable);
		item->setData (iface->path (), DeviceRoles::DevID);
		item->setData (fullName, DeviceRoles::VisibleName);
		item->setData (iface->property ("PartitionSize").toLongLong (), DeviceRoles::TotalSize);
		DevicesModel_->blockSignals (false);
		item->setData (mountPaths, DeviceRoles::MountPoints);
	}

	void Backend::toggleMount (const QString& id)
	{
		auto iface = GetDeviceInterface (id);
		if (!iface)
			return;

		const bool isMounted = iface->property ("DeviceIsMounted").toBool ();
		if (isMounted)
		{
			auto async = iface->asyncCall ("FilesystemUnmount", QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (umountCallFinished (QDBusPendingCallWatcher*)));
		}
		else
		{
			auto async = iface->asyncCall ("FilesystemMount", QString (), QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (mountCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	namespace
	{
		QString GetErrorText (const QString& errorCode)
		{
			QMap<QString, QString> texts;
			texts ["org.freedesktop.UDisks.Error.PermissionDenied"] = Backend::tr ("permission denied");
			texts ["org.freedesktop.PolicyKit.Error.NotAuthorized"] = Backend::tr ("not authorized");
			texts ["org.freedesktop.PolicyKit.Error.Busy"] = Backend::tr ("the device is busy");
			texts ["org.freedesktop.PolicyKit.Error.Failed"] = Backend::tr ("the operation has failed");
			texts ["org.freedesktop.PolicyKit.Error.Cancelled"] = Backend::tr ("the operation has been cancelled");
			texts ["org.freedesktop.PolicyKit.Error.InvalidOption"] = Backend::tr ("invalid mount options were given");
			texts ["org.freedesktop.PolicyKit.Error.FilesystemDriverMissing"] = Backend::tr ("unsupported filesystem");
			return texts.value (errorCode, Backend::tr ("unknown error"));
		}
	}

	void Backend::mountCallFinished (QDBusPendingCallWatcher *watcher)
	{
		qDebug () << Q_FUNC_INFO;
		watcher->deleteLater ();
		QDBusPendingReply<QString> reply = *watcher;

		if (!reply.isError ())
		{
			emit gotEntity (Util::MakeNotification ("Vrooby",
						tr ("Device has been successfully mounted at %1.")
							.arg (reply.value ()),
						PInfo_));
			return;
		}

		const auto& error = reply.error ();
		qWarning () << Q_FUNC_INFO
				<< error.name ()
				<< error.message ();
		emit gotEntity (Util::MakeNotification ("Vrooby",
					tr ("Failed to mount the device: %1 (%2).")
						.arg (GetErrorText (error.name ()))
						.arg (error.message ()),
					PCritical_));
	}

	void Backend::umountCallFinished (QDBusPendingCallWatcher *watcher)
	{
		qDebug () << Q_FUNC_INFO;
		watcher->deleteLater ();
		QDBusPendingReply<void> reply = *watcher;

		if (!reply.isError ())
		{
			emit gotEntity (Util::MakeNotification ("Vrooby",
						tr ("Device has been successfully unmounted."),
						PInfo_));
			return;
		}

		const auto& error = reply.error ();
		qWarning () << Q_FUNC_INFO
				<< error.name ()
				<< error.message ();
		emit gotEntity (Util::MakeNotification ("Vrooby",
					tr ("Failed to unmount the device: %1 (%2).")
						.arg (GetErrorText (error.name ()))
						.arg (error.message ()),
					PCritical_));
	}

	void Backend::handleEnumerationFinished (QDBusPendingCallWatcher *watcher)
	{
		watcher->deleteLater ();
		QDBusPendingReply<QList<QDBusObjectPath>> reply = *watcher;
		if (reply.isError ())
		{
			Valid_ = false;
			qWarning () << reply.error ().message ();
			return;
		}

		for (const QDBusObjectPath& path : reply.value ())
			AddPath (path);
	}

	void Backend::handleDeviceAdded (const QDBusObjectPath& path)
	{
		AddPath (path);
	}

	void Backend::handleDeviceRemoved (const QDBusObjectPath& path)
	{
		RemovePath (path);
	}

	void Backend::handleDeviceChanged (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();

		auto item = Object2Item_.value (path);
		SetItemData (GetDeviceInterface (path), item);
	}

	void Backend::updateDeviceSpaces ()
	{
		for (QStandardItem *item : Object2Item_.values ())
		{
			const auto& mountPaths = item->data (DeviceRoles::MountPoints).toStringList ();
			if (mountPaths.isEmpty ())
				continue;

			const auto& space = boost::filesystem::space (mountPaths.value (0).toStdWString ());
			const auto free = static_cast<qint64> (space.free);
			if (free != item->data (DeviceRoles::AvailableSize).value<qint64> ())
				item->setData (static_cast<qint64> (free), DeviceRoles::AvailableSize);
		}
	}
}
}
}
