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

#include "roompublicmessage.h"
#include <QTextDocument>
#include <QtDebug>
#include <QXmppMessage.h>
#include <QXmppClient.h>
#include "roomclentry.h"
#include "roomparticipantentry.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "roomhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomPublicMessage::RoomPublicMessage (const QString& msg, RoomCLEntry *entry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (DOut)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	{
	}

	RoomPublicMessage::RoomPublicMessage (const QString& msg,
			IMessage::Direction direction,
			RoomCLEntry *entry,
			IMessage::MessageType type,
			IMessage::MessageSubType subType,
			RoomParticipantEntry_ptr part)
	: QObject (entry)
	, ParentEntry_ (entry)
	, ParticipantEntry_ (part)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (direction)
	, Type_ (type)
	, SubType_ (subType)
	{
	}

	RoomPublicMessage::RoomPublicMessage (const QXmppMessage& msg,
			RoomCLEntry *entry,
			RoomParticipantEntry_ptr partEntry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, ParticipantEntry_ (partEntry)
	, Message_ (msg.body ())
	, Datetime_ (msg.stamp ().isValid () ? msg.stamp ().toLocalTime () : QDateTime::currentDateTime ())
	, Direction_ (DIn)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	, XHTML_ (msg.getXhtml ())
	{
		ClientConnection::Split (msg.from (), &FromJID_, &FromVariant_);
	}

	QObject* RoomPublicMessage::GetObject ()
	{
		return this;
	}

	void RoomPublicMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		QXmppClient *client =
				qobject_cast<GlooxAccount*> (ParentEntry_->GetParentAccount ())->
						GetClientConnection ()->GetClient ();

		QXmppMessage msg;
		msg.setBody (Message_);
		msg.setTo (ParentEntry_->GetRoomHandler ()->GetRoomJID ());
		msg.setType (QXmppMessage::GroupChat);
		msg.setXhtml (XHTML_);
		client->sendPacket (msg);
	}

	void RoomPublicMessage::Store ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->HandleMessage (this);
	}

	IMessage::Direction RoomPublicMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType RoomPublicMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::MessageSubType RoomPublicMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	QObject* RoomPublicMessage::OtherPart () const
	{
		switch (Direction_)
		{
		case DIn:
			return ParticipantEntry_.get ();
		case DOut:
			return ParentEntry_;
		}
	}

	QObject* RoomPublicMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString RoomPublicMessage::GetOtherVariant() const
	{
		return FromVariant_;
	}

	QString RoomPublicMessage::GetBody () const
	{
		return Qt::escape (Message_);
	}

	void RoomPublicMessage::SetBody (const QString& msg)
	{
		Message_ = msg;
	}

	QDateTime RoomPublicMessage::GetDateTime () const
	{
		return Datetime_;
	}

	void RoomPublicMessage::SetDateTime (const QDateTime& dt)
	{
		Datetime_ = dt;
	}

	QString RoomPublicMessage::GetRichBody () const
	{
		return XHTML_;
	}

	void RoomPublicMessage::SetRichBody (const QString& xhtml)
	{
		XHTML_ = xhtml;
	}
}
}
}
