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

#ifndef PLUGINS_AZOTH_INTERFACES_AZOTHCOMMON_H
#define PLUGINS_AZOTH_INTERFACES_AZOTHCOMMON_H
#include <QMetaType>
#include <interfaces/iactionsexporter.h>

namespace LeechCraft
{
namespace Azoth
{
	enum State
	{
		SOffline,
		SOnline,
		SAway,
		SXA,
		SDND,
		SChat,
		SInvisible,
		SProbe,
		SError,
		SInvalid,
		/** Makes sense only for account state. This state is used when
		 * account is connecting and the moment and hasn't connected
		 * successfully and neither has failed yet.
		 */
		SConnecting
	};

	inline bool IsLess (State s1, State s2)
	{
		static int order [] = { 7, 3, 4, 5, 6, 1, 2, 8, 9, 10 };
		return order [s1] < order [s2];
	}

	/** Represents possible state of authorizations between two
	 * entities: our user and a remote contact.
	 *
	 * Modelled after RFC 3921, Section 9.
	 */
	enum AuthStatus
	{
		/** Contact and user are not subscribed to each other, and
		 * neither has requested a subscription from the other.
		 */
		ASNone = 0x00,

		/** Contact is subscribed to user (one-way).
		 */
		ASFrom = 0x01,

		/** User is subscribed to contact (one-way).
		 */
		ASTo = 0x02,

		/** User and contact are subscribed to each other (two-way).
		 */
		ASBoth = 0x03,

		/** Contact has requested our subscription.
		 */
		ASContactRequested = 0x08
	};

	/** Represents possible chat states.
	 *
	 * Modelled after XMPP XEP-085.
	 */
	enum ChatPartState
	{
		/** Unknown chat state.
		 */
		CPSNone,

		/** User is actively participating in the chat session.
		 */
		CPSActive,

		/** User has not been actively participating in the chat
		 * session.
		 */
		CPSInactive,

		/** User has effectively ended their participation in the chat
		 * session.
		 */
		CPSGone,

		/** User is composing a message.
		 */
		CPSComposing,

		/** User had been composing but now has stopped.
		 */
		CPSPaused
	};
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::State);
Q_DECLARE_METATYPE (LeechCraft::Azoth::ChatPartState);

#endif
