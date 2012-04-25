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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMHANDLER_H
#include <QObject>
#include <QHash>
#include <interfaces/azoth/imucentry.h>
#include "clientconnection.h"
#include "roomparticipantentry.h"

class QXmppVCardIq;
class QXmppMucManager;
class QXmppMucRoom;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class GlooxAccount;
	class RoomParticipantEntry;
	class FormBuilder;

	class RoomHandler : public QObject
	{
		Q_OBJECT

		GlooxAccount *Account_;
		QXmppMucManager *MUCManager_;
		const QString RoomJID_;
		QXmppMucRoom *Room_;
		RoomCLEntry *CLEntry_;
		QHash<QString, RoomParticipantEntry_ptr> Nick2Entry_;
		QString Subject_;
		// contains new nicks
		QSet<QString> PendingNickChanges_;
		bool HadRequestedPassword_;

		QXmppDiscoveryIq ServerDisco_;
	public:
		RoomHandler (const QString& roomJID, const QString& ourNick, GlooxAccount*);

		QString GetRoomJID () const;
		RoomCLEntry* GetCLEntry ();
		void HandleVCard (const QXmppVCardIq&, const QString&);

		void SetPresence (QXmppPresence);

		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetParticipants () const;
		QString GetSubject () const;
		void SetSubject (const QString&);
		void Join ();
		void Leave (const QString& msg, bool remove = true);
		RoomParticipantEntry* GetSelf ();
		QString GetOurNick () const;
		void SetOurNick (const QString&);

		void SetAffiliation (RoomParticipantEntry*,
				QXmppMucItem::Affiliation, const QString&);
		void SetRole (RoomParticipantEntry*,
				QXmppMucItem::Role, const QString&);

		QXmppMucRoom* GetRoom () const;

		void HandleErrorPresence (const QXmppPresence&, const QString&);
		void HandlePermsChanged (const QString&,
				QXmppMucItem::Affiliation,
				QXmppMucItem::Role,
				const QString&);
		void HandleMessage (const QXmppMessage&, const QString&);
		void UpdatePerms (const QList<QXmppMucItem>&);
		/** Creates a new entry for the given nick if it
		 * doesn't exist already (and does so by calling
		 * CreateParticipantEntry()) or just returns the
		 * already existing one.
		 */
		RoomParticipantEntry_ptr GetParticipantEntry (const QString& nick, bool announce = true);
	private slots:
		void handleParticipantAdded (const QString&);
		void handleParticipantChanged (const QString&);
		void handleParticipantRemoved (const QString&);

		void requestVoice ();

		void handleMessagesAreRead ();
	private:
		bool IsGateway () const;
		/** Creates a new entry for the given nick.
		 */
		RoomParticipantEntry_ptr CreateParticipantEntry (const QString& nick, bool announce);
		void MakeLeaveMessage (const QXmppPresence&, const QString&);
		void MakeJoinMessage (const QXmppPresence&, const QString&);
		void MakeStatusChangedMessage (const QXmppPresence&, const QString&);
		void MakeNickChangeMessage (const QString&, const QString&);
		void MakeKickMessage (const QString&, const QString&);
		void MakeBanMessage (const QString&, const QString&);
		void MakePermsChangedMessage (const QString&,
				QXmppMucItem::Affiliation,
				QXmppMucItem::Role,
				const QString&);
		void HandleNickConflict ();
		void HandlePasswordRequired ();
		QString GetPassKey () const;

		void RemoveEntry (RoomParticipantEntry*);

		void RemoveThis ();
	signals:
		void gotPendingForm (QXmppDataForm*, const QString&);
	};
}
}
}

#endif
