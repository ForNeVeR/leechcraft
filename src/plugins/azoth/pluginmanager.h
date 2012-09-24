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

#ifndef PLUGINS_AZOTH_PLUGINMANAGER_H
#define PLUGINS_AZOTH_PLUGINMANAGER_H
#include <QString>
#include <QDateTime>
#include <util/basehookinterconnector.h>
#include <interfaces/core/ihookproxy.h>

class QDateTime;
class QObject;
class QWebView;

namespace LeechCraft
{
namespace Azoth
{
	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		PluginManager (QObject* = 0);
	signals:
		void hookAddingCLEntryBegin (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookAddingCLEntryEnd (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookChatTabCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebView *webView);

		void hookDnDEntry2Entry (LeechCraft::IHookProxy_ptr proxy,
				QObject *source,
				QObject *target);

		/** @brief Hook for adjusting where CL entry actions appear.
		 *
		 * This hook is called to determine where the given action for
		 * the given entry should be shown. By default, it is only shown
		 * in the contact list context menu. This hook is only called
		 * for actions that were created as a result of the
		 * hookEntryActionsRequested() hook.
		 *
		 * The handler of this hook should append the string IDs of the
		 * corresponding places to the return value of the proxy object,
		 * so an example implementation for inserting an action into
		 * contact list menu and tab's context menu would look like:
		 * @code
		 * QStringList ours;
		 * ours << "contactListContextMenu"
		 * 	<< "tabContextMenu";
		 * proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
		 * @endcode
		 *
		 * The following IDs are possible:
		 * - contactListContextMenu for contact list context menu (this
		 *   is the default option if the return value is unmodified).
		 * - tabContextMenu for tab's context menu.
		 * - applicationMenu for the menu item in the application's main
		 *   menu.
		 * - toolbar for the toolbar in the entry chat window.
		 *
		 * Please note that this hook would be called on each plugin
		 * that exposes it, so each plugin would have it called for each
		 * action created in hookEntryActionsRequested(), for all
		 * plugins, not only their own ones.
		 *
		 * @param[out] proxy The proxy object.
		 * @param[out] action The previously created action.
		 * @param[out] entry The entry for which action is queried.
		 *
		 * @sa hookEntryActionsRequested()
		 */
		void hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);

		void hookEntryActionsRemoved (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);

		/** @brief Hook for adding actions for contact list entries.
		 *
		 * This hook is called for adding new CL entry-related actions
		 * could be.
		 *
		 * The passed entry object implements ICLEntry and represents
		 * the entry for which the action could be created. Of course,
		 * it's OK to not create actions for various entries, for
		 * example, after querying their type, state, etc.
		 *
		 * The exact places where the action would appear is later
		 * adjusted inside the hookEntryActionAreasRequested() hook.
		 *
		 * The handler of this hook should append the new actions to the
		 * proxy's return value, which is actually a list of QObjects.
		 * So, a typical hook would look like:
		 * @code
		 * QAction *action = new QAction (tr ("Some action"), entry);
		 * QList<QVariant> list = proxy->GetReturnValue ().toList ();
		 * list << QVariant::fromValue<QObject*> (action);
		 * proxy->SetReturnValue (list);
		 * @endcode
		 *
		 * Please note that it's better to create actions as children of
		 * the entry (as in this example) so that they are automatically
		 * deleted when the entry is deleted.
		 *
		 * @param[out] proxy The proxy object.
		 * @param[out] entry The object implementing ICLEntry and
		 * representing the entry for which actions are to be created.
		 *
		 * @sa hookEntryActionAreasRequested()
		 */
		void hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);

		void hookEntryStatusChanged (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry,
				QString variant);

		void hookGonnaAppendMsg (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);

		void hookGonnaHandleSmiles (LeechCraft::IHookProxy_ptr proxy,
				QString body,
				QString pack);

		void hookGotAuthRequest (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);

		/** @brief Hook for handling incoming messages.
		 *
		 * This hook is called for handling incoming messages. The
		 * message object could be modified accordingly, if possible,
		 * and the result would be visible to the rest of Azoth.
		 *
		 * The message object, of course, implements IMessage.
		 *
		 * If the hook handler cancels default handler (by calling
		 * IHookProxy::CancelDefault on the proxy object), nothing would
		 * be done with the message: particularly, it won't be appended
		 * to the chat window and such.
		 *
		 * @param[out] proxy The proxy object.
		 * @param[out] message The message object implementing IMessage.
		 */
		void hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookGotMessage2 (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookFormatDateTime (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QDateTime dateTime,
				QObject *message);
		void hookFormatNickname (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString nick,
				QObject *message);
		void hookFormatBodyBegin (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookFormatBodyEnd (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookIsHighlightMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookMadeCurrent (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab);
		void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
		void hookMessageCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
		void hookShouldCountUnread (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookThemeReloaded (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QWebView *view,
				QObject *entry);

		/** @brief Hook for tooltip formatting.
		 *
		 * This hook is called while formatting tooltip for the contact
		 * list tree for the given entry, after general information
		 * about the entry has been formatted.
		 *
		 * The already-formatted string is contained as "tooltip" value
		 * in the proxy. The hook may change the string by updating this
		 * value.
		 *
		 * If the hook handler cancels default handler (by calling
		 * IHookProxy::CancelDefault on the proxy object), variants info
		 * won't be formatted, and the hook's "tooltip" value would be
		 * returned.
		 *
		 * @param[out] proxy Standard proxy object.
		 * @param[out] entry The entry for which the tooltip is being
		 * formatted.
		 * @param[in,out] proxy::"tooltip" The tooltip string.
		 *
		 * @sa IHookProxy
		 */
		void hookTooltipBeforeVariants (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
	};
}
}

#endif
