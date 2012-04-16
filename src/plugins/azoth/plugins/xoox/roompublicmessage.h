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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPUBLICMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPUBLICMESSAGE_H
#include <QObject>
#include <QPointer>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/irichtextmessage.h>
#include "roomparticipantentry.h"

class QXmppMessage;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class RoomParticipantEntry;

	class RoomPublicMessage : public QObject
							, public IMessage
							, public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage
				LeechCraft::Azoth::IRichTextMessage);

		QPointer<RoomCLEntry> ParentEntry_;
		RoomParticipantEntry_ptr ParticipantEntry_;
		QString Message_;
		QDateTime Datetime_;
		Direction Direction_;
		QString FromJID_;
		QString FromVariant_;
		MessageType Type_;
		MessageSubType SubType_;

		QString XHTML_;
	public:
		RoomPublicMessage (const QString&, RoomCLEntry*);
		RoomPublicMessage (const QString&, Direction,
				RoomCLEntry*,
				MessageType,
				MessageSubType,
				RoomParticipantEntry_ptr = RoomParticipantEntry_ptr ());
		RoomPublicMessage (const QXmppMessage&, RoomCLEntry*,
				RoomParticipantEntry_ptr = RoomParticipantEntry_ptr ());

		QObject* GetObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;

		/** Since it's outgoing message, the other part
		 * always equals to the room entry.
		 */
		QObject* OtherPart () const;
		QObject* ParentCLEntry () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);

		QString GetRichBody () const;
		void SetRichBody (const QString&);
	};
}
}
}

#endif
