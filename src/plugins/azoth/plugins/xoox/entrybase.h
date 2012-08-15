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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ENTRYBASE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ENTRYBASE_H
#include <QObject>
#include <QImage>
#include <QMap>
#include <QVariant>
#include <QXmppMessage.h>
#include <QXmppVCardIq.h>
#include <QXmppVersionIq.h>
#include <QXmppDiscoveryIq.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iadvancedclentry.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/ihavedirectedstatus.h>
#include <interfaces/azoth/isupportgeolocation.h>
#include <interfaces/azoth/isupportmicroblogs.h>

class QXmppPresence;
class QXmppVersionIq;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class PEPEventBase;
	class GlooxMessage;
	class VCardDialog;
	class GlooxAccount;

	/** Common base class for GlooxCLEntry, which reprensents usual
	 * entries in the contact list, and RoomCLEntry, which represents
	 * participants in MUCs.
	 *
	 * This class tries to unify and provide a common implementation of
	 * what those classes, well, have in common.
	 */
	class EntryBase : public QObject
					, public ICLEntry
					, public IAdvancedCLEntry
					, public IMetaInfoEntry
					, public IHaveDirectedStatus
					, public ISupportMicroblogs
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry
				LeechCraft::Azoth::IAdvancedCLEntry
				LeechCraft::Azoth::IMetaInfoEntry
				LeechCraft::Azoth::IHaveDirectedStatus
				LeechCraft::Azoth::ISupportMicroblogs)
	protected:
		GlooxAccount *Account_;

		QList<QObject*> AllMessages_;
		QList<GlooxMessage*> UnreadMessages_;
		QMap<QString, EntryStatus> CurrentStatus_;
		QList<QAction*> Actions_;
		QAction *Commands_;
		QAction *DetectNick_;
		QAction *StdSep_;

		QMap<QString, GeolocationInfo_t> Location_;

		QImage Avatar_;
		QString RawInfo_;
		QXmppVCardIq VCardIq_;
		QPointer<VCardDialog> VCardDialog_;

		QByteArray VCardPhotoHash_;

		QMap<QString, QMap<QString, QVariant>> Variant2ClientInfo_;
		QMap<QString, QByteArray> Variant2VerString_;
		QMap<QString, QXmppVersionIq> Variant2Version_;
		QMap<QString, QList<QXmppDiscoveryIq::Identity>> Variant2Identities_;

		bool HasUnreadMsgs_;
		bool VersionReqsEnabled_;
		bool HasBlindlyRequestedVCard_;
	public:
		EntryBase (GlooxAccount* = 0);
		virtual ~EntryBase ();

		// ICLEntry
		QObject* GetObject ();
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		virtual QList<QAction*> GetActions () const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();

		// IAdvancedCLEntry
		AdvancedFeatures GetAdvancedFeatures () const;
		void DrawAttention (const QString&, const QString&);

		// IMetaInfoEntry
		QVariant GetMetaInfo (DataField) const;

		// IHaveDirectedStatus
		bool CanSendDirectedStatusNow (const QString&);
		void SendDirectedStatus (const EntryStatus&, const QString&);

		// ISupportMicroblogs
		void RequestLastPosts (int);

		virtual QString GetJID () const = 0;

		void HandlePresence (const QXmppPresence&, const QString&);
		void HandleMessage (GlooxMessage*);
		void HandlePEPEvent (QString, PEPEventBase*);
		void HandleAttentionMessage (const QXmppMessage&);
		void UpdateChatState (QXmppMessage::State, const QString&);
		void SetStatus (const EntryStatus&, const QString&);
		void SetAvatar (const QByteArray&);
		void SetAvatar (const QImage&);
		QXmppVCardIq GetVCard () const;
		void SetVCard (const QXmppVCardIq&, bool initial = false);
		void SetRawInfo (const QString&);

		bool HasUnreadMsgs () const;
		QList<GlooxMessage*> GetUnreadMessages () const;

		void SetClientInfo (const QString&, const QString&, const QByteArray&);
		void SetClientInfo (const QString&, const QXmppPresence&);
		void SetClientVersion (const QString&, const QXmppVersionIq&);
		void SetDiscoIdentities (const QString&, const QList<QXmppDiscoveryIq::Identity>&);

		GeolocationInfo_t GetGeolocationInfo (const QString&) const;

		void SetVersionReqsEnabled (bool);

		QByteArray GetVariantVerString (const QString&) const;
		QXmppVersionIq GetClientVersion (const QString&) const;
	private:
		void CheckVCardUpdate (const QXmppPresence&);
		QString FormatRawInfo (const QXmppVCardIq&);
		void SetNickFromVCard (const QXmppVCardIq&);
	private slots:
		void handleCommands ();
		void handleDetectNick ();
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();
		void messagesAreRead ();

		void attentionDrawn (const QString&, const QString&);
		void moodChanged (const QString&);
		void activityChanged (const QString&);
		void tuneChanged (const QString&);
		void locationChanged (const QString&);

		void gotRecentPosts (const QList<LeechCraft::Azoth::Post>&);
		void gotNewPost (const LeechCraft::Azoth::Post&);

		void locationChanged (const QString&, QObject*);

		void vcardUpdated ();
	};
}
}
}

#endif
