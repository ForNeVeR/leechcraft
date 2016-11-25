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

#pragma once

#include <QString>
#include <QXmppMucIq.h>
#include <QXmppDiscoveryIq.h>
#include <interfaces/azoth/azothcommon.h>

class QDomElement;
class QWidget;
class QDataStream;

class QXmppMessage;
class QXmppDataForm;
class QXmppPresence;

QDataStream& operator<< (QDataStream&, const QXmppDiscoveryIq::Identity&);
QDataStream& operator>> (QDataStream&, QXmppDiscoveryIq::Identity&);

namespace LeechCraft
{
namespace Azoth
{
struct EntryStatus;

namespace Xoox
{
class EntryBase;
class CapsDatabase;

namespace XooxUtil
{
	extern const QString NsRegister;

	QString RoleToString (const QXmppMucItem::Role&);
	QString AffiliationToString (const QXmppMucItem::Affiliation&);

	struct StaticClientInfo
	{
		QString ID_;
		QString HumanReadableName_;

		bool IsEmpty () const
		{
			return ID_.isEmpty () || HumanReadableName_.isEmpty ();
		}
	};
	StaticClientInfo GetStaticClientInfo (const QString&);
	QString GetClientIDName (const QString&);
	QString GetClientHRName (const QString&);

	QDomElement XmppElem2DomElem (const QXmppElement&);
	QXmppElement Form2XmppElem (const QXmppDataForm&);

	bool RunFormDialog (QWidget*);

	bool CheckUserFeature (EntryBase *entry,
			const QString& variant, const QString& feature, const CapsDatabase *capsDB);

	QXmppMessage Forwarded2Message (const QXmppElement& wrapper);

	EntryStatus PresenceToStatus (const QXmppPresence& pres);

	QXmppPresence StatusToPresence (State, const QString&, int);
}
}
}
}
