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

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#include <QFlags>
#include <QMetaType>
#include <QVariant>

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	/** @brief Represents a single MUC entry in the CL.
	 *
	 * This class extends ICLEntry by providing methods and data
	 * specific to MUCs. A well-written plugin should implement this
	 * interface along with ICLEntry for MUC entries.
	 *
	 * See IConfigurableMUC if the MUC supports being configured and
	 * IMUCPerms if the MUC supports adjusting permissions for its
	 * participants.
	 */
	class IMUCEntry
	{
		Q_GADGET
	public:
		virtual ~IMUCEntry () {}

		enum MUCFeature
		{
			/** This room has a configuration dialog and can be
			 * configured.
			 */
			MUCFCanBeConfigured = 0x0001,

			/** Room can have a (possibly empty) subject which may be
			 * retrieved by GetMUCSubject().
			 */
			MUCFCanHaveSubject = 0x0002,

			/** Room supports invitations.
			 */
			MUCFCanInvite = 0x0004
		};

		Q_DECLARE_FLAGS (MUCFeatures, MUCFeature);

		/** @brief The list of features of this MUC.
		 *
		 * Returns the list of features supported by this MUC.
		 */
		virtual MUCFeatures GetMUCFeatures () const = 0;

		/** @brief Returns subject of this MUC.
		 *
		 * Returns the subject/topic of this MUC room, possibly empty.
		 * If the protocol or smth doesn't support subjects for MUCs,
		 * this function should return an empty string.
		 *
		 * @return The subject of this MUC.
		 */
		virtual QString GetMUCSubject () const = 0;

		/** @brief Updates the subject of this MUC.
		 *
		 * Sets the subject of the conference. If it fails for some
		 * reason, for example, due to insufficient rights, this
		 * function should do nothing.
		 *
		 * @param[in] subject The new subject of this room to set.
		 */
		virtual void SetMUCSubject (const QString& subject) = 0;

		/** @brief The list of participants of this MUC.
		 *
		 * If the protocol plugin chooses to return info about
		 * participants via the IAccount interface, the ICLEntry objects
		 * returned from this function and from IAccount should be the
		 * same for the same participants.
		 *
		 * @return The list of participants of this MUC.
		 */
		virtual QList<QObject*> GetParticipants () = 0;

		/** @brief Requests to join the room.
		 *
		 * If the we aren't joined to this MUC (for example, there was a
		 * nick conflict, or this entry represents a bookmark), the room
		 * should be tried to be joined.
		 */
		virtual void Join () = 0;

		/** @brief Requests to leave the room.
		 *
		 * The protocol implementation is expected to leave the room
		 * with the given leave message. If leaving is impossible for
		 * some reason, it's ok to stay.
		 *
		 * If the room is successfully left, the parent account should
		 * take care of removing the contact list entries corresponding
		 * to its participants and the room itself.
		 *
		 * @param[in] msg The leave message (if applicable).
		 */
		virtual void Leave (const QString& msg = QString ()) = 0;

		/** @brief Returns the nick of our participant.
		 *
		 * @return The nickname or null string if not applicable.
		 */
		virtual QString GetNick () const = 0;

		/** @brief Changes the nick of our participant.
		 *
		 * If changing nicks is not allowed or is not supported, nothing
		 * should be done.
		 *
		 * @param[in] nick New nick for our participant in
		 * this room.
		 */
		virtual void SetNick (const QString& nick) = 0;

		/** @brief Returns human-readable name of participants' group.
		 *
		 * This function should return the human-readable name of the
		 * group which holds the participants of this room.
		 *
		 * @return The human-readable name.
		 */
		virtual QString GetGroupName () const = 0;

		/** @brief Returns the real ID of a participant.
		 *
		 * This function should return a real protocol ID of the given
		 * participant (JID for XMPP protocol, for example), or a null
		 * string if the ID is unknown, or the given participant doesn't
		 * belong to this room.
		 *
		 * @param[in] participant The participant for which to return
		 * the real JID.
		 * @return The real ID of a participant.
		 */
		virtual QString GetRealID (QObject *participant) const = 0;

		/** @brief Returns the data identifying this room.
		 *
		 * The returned variant map should have the same format as the
		 * one from IMUCJoinWidget::GetIdentifyingData().
		 *
		 * @return The identifying data.
		 *
		 * @sa IMUCJoinWidget::GetIdentifyingData()
		 */
		virtual QVariantMap GetIdentifyingData () const = 0;

		/** @brief Invites the user to this MUC.
		 *
		 * This function should invite the given user to this MUC by
		 * means of the protocol, if applicable, or by a plain message
		 * if not.
		 *
		 * User is identified by its protocol-specific ID, as returned
		 * by ICLEntry::GetHumanReadableID().
		 *
		 * The invitation may contain an optional message.
		 *
		 * @param[in] userId The protocol-specific ID of the user to
		 * invite.
		 * @param[in] msg The optional message to send along with the
		 * invite.
		 *
		 * @sa ICLEntry::GetHumanReadableID()
		 */
		virtual void InviteToMUC (const QString& userId, const QString& msg) = 0;

		/** @brief Notifies about new participants in the room.
		 *
		 * This signal should emitted when new participants join this
		 * room.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] parts The list of participants that joined.
		 */
		virtual void gotNewParticipants (const QList<QObject*>& parts) = 0;

		/** @brief Notifies about subject change.
		 *
		 * This signal should be emitted when room subject is changed
		 * to newSubj.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] newSubj The new subject of this room.
		 */
		virtual void mucSubjectChanged (const QString& newSubj) = 0;

		/** @brief Notifies about nick conflict.
		 *
		 * This signal should be emitted when room gets the error from
		 * the server that the nickname is already in use.
		 *
		 * The signal handler could either call SetNick() with some
		 * other nickname (in this case the room should automatically
		 * try to rejoin) or do nothing it all (in this case the room
		 * should, well, do nothing as well).
		 *
		 * This signal should be emitted only if the error arises while
		 * joining, not as result of SetNick().
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] usedNick The nickname that was used to join the
		 * room.
		 */
		virtual void nicknameConflict (const QString& usedNick) = 0;

		/** @brief Notifies about participant being kicked.
		 *
		 * This signal should be emitted whenever our user gets kicked
		 * from this room.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] reason The optional reason message.
		 */
		virtual void beenKicked (const QString& reason) = 0;

		/** @brief Notifies about participant being banned.
		 *
		 * This signal should be emitted whenever our user gets banned
		 * from this room.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] reason The optional reason message.
		 */
		virtual void beenBanned (const QString& reason) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IMUCEntry::MUCFeatures);
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMUCEntry,
		"org.Deviant.LeechCraft.Azoth.IMUCEntry/1.0");

#endif
