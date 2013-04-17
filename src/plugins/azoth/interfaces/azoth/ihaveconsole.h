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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#define PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#include <QMetaType>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for accounts that support protocol consoles.
	 * 
	 * An example of a protocol console may be an XML console for XMPP
	 * protocol or just raw text console for IRC.
	 * 
	 * The account supporting console notifies about new packets (both
	 * incoming and outgoing) by the gotConsolePacket() signal.
	 * 
	 * Azoth core and other plugins may toggle the status of the console
	 * by the means of SetConsoleEnabled() function. By default, the
	 * console for each account should be disabled.
	 * 
	 * @sa IAccount
	 */
	class IHaveConsole
	{
	public:
		virtual ~IHaveConsole () {}
		
		/** @brief Defines the format of the packets in this protocol.
		 */
		enum PacketFormat
		{
			/** @brief XML packets (like in XMPP).
			 * 
			 * The packets would be represented as formatted XML text.
			 */
			PFXML,
			
			/** @brief Plain text packets (like in IRC).
			 * 
			 * The packets would be represented as unformatted plain
			 * text.
			 */
			PFPlainText,
			
			/** @brief Binary packets (like in Oscar).
			 * 
			 * The packets would be converted to Base64 or Hex-encoding.
			 */
			PFBinary
		};
		
		/** @brief Defines the direction of a packet.
		 */
		enum PacketDirection
		{
			/** @brief Incoming packet.
			 */
			PDIn,
			
			/** @brief Outgoing packet.
			 */
			PDOut
		};

		/** @brief Returns the packet format used in this account.
		 * 
		 * @return The packet format.
		 */
		virtual PacketFormat GetPacketFormat () const = 0;
		
		/** @brief Enables or disables the console.
		 * 
		 * This function toggles the status of the console for the
		 * corresponding account.
		 * 
		 * If the console is enabled, only this account's packets should
		 * be emitted by the gotConsolePacket() signal. If the console
		 * is disabled, gotConsolePacket() signal shouldn't be emitted
		 * at all.
		 * 
		 * By default, console for each account should be in disabled
		 * state, unless explicitly enabled by calling this function.
		 * 
		 * @param[in] enabled Whether the console should be enabled.
		 */
		virtual void SetConsoleEnabled (bool enabled) = 0;
		
		/** @brief Notifies about new packet.
		 * 
		 * This signal is used by the console-supporting account to
		 * notify about new packets, both incoming and outgoing.
		 * 
		 * This signal should be emitted if and only if the console has
		 * been explicitly enabled for this account by calling the
		 * SetConsoleEnabled() function.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] packet The packet data.
		 * @param[out] direction The direction of the packet, member of
		 * the PacketDirection enum.
		 * @param[out] hrEntryId The human-readable ID of the related
		 * entry, or null string if not applicable.
		 */
		virtual void gotConsolePacket (const QByteArray& packet,
				int direction, const QString& hrEntryId) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveConsole,
		"org.Deviant.LeechCraft.Azoth.IHaveConsole/1.0");

#endif
