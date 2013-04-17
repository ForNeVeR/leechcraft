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

#include "fdopropsadaptor.h"
#include <QVariantMap>
#include <QStringList>
#include <QMetaMethod>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusVariant>
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
namespace MPRIS
{
	FDOPropsAdaptor::FDOPropsAdaptor (QObject *parent)
	: QDBusAbstractAdaptor (parent)
	{
	}

	void FDOPropsAdaptor::Notify (const QString& iface, const QString& prop, const QVariant& val)
	{
		QVariantMap changes;
		changes [prop] = val;
		emit PropertiesChanged (iface, changes, QStringList ());
	}

	QDBusVariant FDOPropsAdaptor::Get (const QString& iface, const QString& propName)
	{
		QObject *child = 0;
		QMetaProperty prop;
		if (!GetProperty (iface, propName, &prop, &child))
			return QDBusVariant (QVariant ());

		return QDBusVariant (prop.read (child));
	}

	void FDOPropsAdaptor::Set (const QString& iface, const QString& propName, const QDBusVariant& value)
	{
		QObject *child = 0;
		QMetaProperty prop;
		if (!GetProperty (iface, propName, &prop, &child))
			return;

		if (!prop.isWritable ())
		{
			auto context = dynamic_cast<QDBusContext*> (parent ());
			if (context->calledFromDBus ())
				context->sendErrorReply (QDBusError::AccessDenied, propName + " isn't writable");
			return;
		}

		prop.write (child, value.variant ());
	}

	bool FDOPropsAdaptor::GetProperty (const QString& iface, const QString& prop, QMetaProperty *propObj, QObject **childObject) const
	{
		auto adaptors = parent ()->findChildren<QDBusAbstractAdaptor*> ();
		for (const auto& child : adaptors)
		{
			const auto mo = child->metaObject ();

			if (!iface.isEmpty ())
			{
				const auto idx = mo->indexOfClassInfo ("D-Bus Interface");
				if (idx == -1)
					continue;

				const auto& info = mo->classInfo (idx);
				if (iface != info.value ())
					continue;
			}

			const auto idx = mo->indexOfProperty (prop.toUtf8 ().constData ());
			if (idx != -1)
			{
				*propObj = mo->property (idx);
				*childObject = child;
				return true;
			}
		}

		auto context = dynamic_cast<QDBusContext*> (parent ());
		if (context->calledFromDBus ())
			context->sendErrorReply (QDBusError::InvalidMember, "no such property " + prop);

		return false;
	}
}
}
}
