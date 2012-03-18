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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_CALLBACKS_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_CALLBACKS_H
#include <QObject>
#include <QHash>
#include <QSet>
#include <QMap>
#include <QFile>
#include <msn/connection.h>
#include <msn/notificationserver.h>
#include <msn/externals.h>
#include <interfaces/azothcommon.h>

class QTcpSocket;

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNAccount;
	class MSNBuddyEntry;

	class Callbacks : public QObject
					, public MSN::Callbacks
	{
		Q_OBJECT

		MSNAccount *Account_;

		QHash<void*, QTcpSocket*> Sockets_;

		MSN::NotificationServerConnection *Conn_;
		QFile LogFile_;
	public:
		Callbacks (MSNAccount*);

		void SetNotificationServerConnection (MSN::NotificationServerConnection*);

		void registerSocket (void *sock, int read, int write, bool isSSL);
		void unregisterSocket (void *sock);
		void closeSocket (void *sock);

		void showError (MSN::Connection *conn, std::string msg);

		void buddyChangedStatus (MSN::NotificationServerConnection *conn, MSN::Passport buddy,
				std::string friendlyname, MSN::BuddyStatus state, unsigned int clientID, std::string msnobject);
		void buddyOffline (MSN::NotificationServerConnection *conn, MSN::Passport buddy);
		void log (int writing, const char *buf);
		void gotFriendlyName (MSN::NotificationServerConnection *conn, std::string friendlyname);
		void gotBuddyListInfo (MSN::NotificationServerConnection *conn, MSN::ListSyncInfo *data);
		void buddyChangedPersonalInfo (MSN::NotificationServerConnection *conn, MSN::Passport fromPassport, MSN::personalInfo pInfo);
		void gotLatestListSerial (MSN::NotificationServerConnection *conn, std::string lastChange);
		void gotGTC (MSN::NotificationServerConnection *conn, char c);
		void gotBLP (MSN::NotificationServerConnection *conn, char c);

		void addedListEntry (MSN::NotificationServerConnection *conn, MSN::ContactList list, MSN::Passport buddy, std::string friendlyname);
		void removedListEntry (MSN::NotificationServerConnection *conn, MSN::ContactList list, MSN::Passport buddy);
		void addedGroup (MSN::NotificationServerConnection *conn, bool added, std::string groupName, std::string groupId);
		void removedGroup (MSN::NotificationServerConnection *conn, bool removed, std::string groupId);
		void renamedGroup (MSN::NotificationServerConnection *conn, bool renamed, std::string newGroupName, std::string groupId);
		void addedContactToGroup (MSN::NotificationServerConnection *conn, bool added, std::string groupId, std::string contactId);
		void removedContactFromGroup (MSN::NotificationServerConnection *conn, bool removed, std::string groupId, std::string contactId);
		void addedContactToAddressBook (MSN::NotificationServerConnection *conn, bool added, std::string passport, std::string displayName, std::string guid);
		void removedContactFromAddressBook (MSN::NotificationServerConnection *conn, bool removed, std::string contactId, std::string passport);
		void enabledContactOnAddressBook (MSN::NotificationServerConnection *conn, bool enabled, std::string contactId, std::string passport);
		void disabledContactOnAddressBook (MSN::NotificationServerConnection *conn, bool disabled, std::string contactId);

		void gotSwitchboard (MSN::SwitchboardServerConnection *conn, const void *tag);
		void buddyJoinedConversation (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname, int is_initial);
		void buddyLeftConversation (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy);
		void gotInstantMessage (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname, MSN::Message *msg);
		void gotMessageSentACK (MSN::SwitchboardServerConnection *conn, int trID);
		void gotEmoticonNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string alias, std::string msnobject);
		void failedSendingMessage (MSN::Connection *conn);
		void gotNudge (MSN::SwitchboardServerConnection *conn, MSN::Passport username);
		void gotVoiceClipNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string msnobject);
		void gotWinkNotification (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string msnobject);
		void gotInk (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string image);
		void gotActionMessage (MSN::SwitchboardServerConnection *conn, MSN::Passport username, std::string message);
		void buddyTyping (MSN::SwitchboardServerConnection *conn, MSN::Passport buddy, std::string friendlyname);

		void gotInitialEmailNotification (MSN::NotificationServerConnection *conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders);
		void gotNewEmailNotification (MSN::NotificationServerConnection *conn, std::string from, std::string subject);

		void fileTransferProgress (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, long long unsigned int transferred, long long unsigned int total);
		void fileTransferFailed (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, MSN::fileTransferError error);
		void fileTransferSucceeded (MSN::SwitchboardServerConnection *conn, unsigned int sessionID);
		void fileTransferInviteResponse (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, bool response);
		void gotVoiceClipFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string file);
		void gotEmoticonFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string alias, std::string file);
		void gotWinkFile (MSN::SwitchboardServerConnection *conn, unsigned int sessionID, std::string file);
		void gotNewConnection (MSN::Connection *conn);

		void gotOIMList (MSN::NotificationServerConnection *conn, std::vector<MSN::eachOIM> OIMs);
		void gotOIM (MSN::NotificationServerConnection *conn, bool success, std::string id, std::string message);
		void gotOIMSendConfirmation (MSN::NotificationServerConnection *conn, bool success, int id);
		void gotOIMDeleteConfirmation (MSN::NotificationServerConnection *conn, bool success, std::string id);
		void gotContactDisplayPicture (MSN::SwitchboardServerConnection *conn, MSN::Passport passport, std::string filename);

		void connectionReady (MSN::Connection *conn);
		void closingConnection (MSN::Connection *conn);

		void changedStatus (MSN::NotificationServerConnection *conn, MSN::BuddyStatus state);

		void* connectToServer (std::string server, int port, bool *connected, bool isSSL = false);

		void askFileTransfer (MSN::SwitchboardServerConnection* conn, MSN::fileTransferInvite ft);

		int listenOnPort (int port);
		std::string getOurIP();
		std::string getSecureHTTPProxy();

		int getSocketFileDescriptor (void *sock);
		size_t getDataFromSocket (void *sock, char *data, size_t size);
		size_t writeDataToSocket (void *sock, char *data, size_t size);

		void gotInboxUrl (MSN::NotificationServerConnection*, MSN::hotmailInfo);
	private slots:
		void handleSocketRead ();
		void handleSocketWrite ();
		void handleSocketConnected ();
	signals:
		void finishedConnecting ();
		void gotBuddies (const QList<MSN::Buddy*>&);
		void removedBuddy (const QString& cid, const QString& pass);
		void removedBuddy (MSN::ContactList, const QString& pass);
		void gotGroups (const QList<MSN::Group>&);
		void removedGroup (const QString&);
		void renamedGroup (const QString& id, const QString& newName);
		void gotOurFriendlyName (const QString&);

		void buddyAddedToGroup (const QString& cid, const QString& group);
		void buddyRemovedFromGroup (const QString& cid, const QString& group);

		void weChangedState (State);
		void buddyChangedStatus (const QString&, State);
		void buddyChangedStatusText (const QString&, const QString&);
		void buddyUpdatedName (const QString&, const QString&);

		void gotMessage (const QString&, MSN::Message*);
		void gotNudge (const QString&);
		void messageDelivered (int);

		void gotSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*);
		void buddyJoinedSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*);
		void buddyLeftSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*);

		void initialEmailNotification (int total, int unread);
		void newEmailNotification (const QString& from, const QString& subj);
		
		void fileTransferProgress (uint sess, quint64 done, quint64 total);
		void fileTransferFailed (uint sess);
		void fileTransferFinished (uint sess);
		void fileTransferGotResponse (uint sess, bool resp);
		void fileTransferSuggested (MSN::fileTransferInvite);
	};
}
}
}

#endif
