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

#ifndef PLUGINS_AZOTH_INTERFACES_IAUTHABLE_H
#define PLUGINS_AZOTH_INTERFACES_IAUTHABLE_H
#include <QString>
#include "azothcommon.h"

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Represents an entry that supports authorizations.
	 */
	class IAuthable
	{
	public:
		virtual ~IAuthable () {}

		/** @brief Returns the AuthStatus between our user and this
		 * remote.
		 *
		 * @return Authorization status of this entry.
		 */
		virtual AuthStatus GetAuthStatus () const = 0;

		/** @brief Resends authorization to the entry.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void ResendAuth (const QString& reason = QString ()) = 0;

		/** @brief Revokes authorization from the entry.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void RevokeAuth (const QString& reason = QString ()) = 0;

		/** @brief Unsubscribes ourselves from the contact.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void Unsubscribe (const QString& reason = QString ()) = 0;

		/** @brief Rerequest authorization.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void RerequestAuth (const QString& reason = QString ()) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IAuthable,
		"org.Deviant.LeechCraft.Azoth.IAuthable/1.0");

#endif
