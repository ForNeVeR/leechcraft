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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVESERVICEDISCOVERY_H
#define PLUGINS_AZOTH_INTERFACES_IHAVESERVICEDISCOVERY_H
#include <QMetaType>

class QAbstractItemModel;
class QModelIndex;
class QString;

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for service discovery sessions.
	 *
	 * This interface is expected to be implemented by the objects
	 * returned from IHaveServiceDiscovery::CreateSDSession()
	 * representing service discovery sessions.
	 *
	 * The service discovery query string is set via SetQuery() method,
	 * and the model representing service discovery results is obtained
	 * via GetRepresentationModel() method. The list of possible actions
	 * for a given index in that model is obtained via GetActionsFor(),
	 * and if the user chooses to execute an action, ExecuteAction() is
	 * called.
	 *
	 * @sa IHaveServiceDiscovery
	 */
	class ISDSession
	{
	public:
		virtual ~ISDSession () {}

		/** @brief Sets the service discovery query.
		 *
		 * The contents and semantics of the query string vary depending
		 * on the protocol. For example, for XMPP protocol it could be
		 * "neko.im" for querying an XMPP server or something like
		 * "c_plus_plus@conference.jabber.ru" for querying a MUC room.
		 *
		 * @param[in] query The query string.
		 *
		 * @sa GetRepresentationModel, GetQuery()
		 */
		virtual void SetQuery (const QString& query) = 0;

		/** @brief Returns the query of this SD session.
		 *
		 * This function should return the query string of this service
		 * discovery session, or an empty string if no query has been
		 * set.
		 *
		 * @return The query string.
		 *
		 * @sa SetQuery()
		 */
		virtual QString GetQuery () const = 0;

		/** @brief Returns the model representing discovery results.
		 *
		 * Of course, this model can be changed dynamically by the
		 * account in accordance with the query set by the SetQuery()
		 * method.
		 *
		 * If the protocol (and account) supports recursive and
		 * hierarchical service discovery results, it is recommended
		 * that the model fetches data lazily, as long as fetchMore()
		 * and such is called, to minimize network traffic and load.
		 *
		 * The model returned by this function must be the same during
		 * the whole lifetime of this service discovery session.
		 *
		 * @return The model with service discovery results.
		 *
		 * @sa SetQuery, GetActionsFor, ExecuteAction
		 */
		virtual QAbstractItemModel* GetRepresentationModel () const = 0;

		/** @brief Returns the list of actions for the given index.
		 *
		 * This function should return the list of actions for the given
		 * model index, which belongs to the model returned by the
		 * GetRepresentationModel(). If the protocol (or account)
		 * doesn't support the concept of actions, or there are no
		 * actions for the index, an empty list should be returned.
		 *
		 * The actual return value is the list of pairs. In each pair,
		 * the first element is the ID of the action (defined by the
		 * underlying implementation, of course), and the other one is
		 * the human-readable name of the action. If the user selects an
		 * action to be executed, its ID is passed to the
		 * ExecuteAction() function.
		 *
		 * @param[in] index The model index for which to return the list
		 * of actions.
		 * @return The list of actions for the given index.
		 *
		 * @sa GetRepresentationModel, ExecuteAction
		 */
		virtual QList<QPair<QByteArray, QString>> GetActionsFor (const QModelIndex& index) = 0;

		/** @brief Executes the action with the given id.
		 *
		 * This function is called when the user selects an action from
		 * the list returned by GetActionsFor().
		 *
		 * The id is the same ID that was returned from the
		 * GetActionsFor().
		 *
		 * @param[in] index The model index for which to execute the
		 * action.
		 * @param[in] id The ID of the action to be executed.
		 *
		 * @sa GetActionsFor
		 */
		virtual void ExecuteAction (const QModelIndex& index, const QByteArray& id) = 0;
	};

	/** @brief Interface for accounts supporting service discovery.
	 *
	 * Service discovery is a mechanism that allows one to discover the
	 * capabilities and items of remote contacts and other entities. For
	 * example, through service discovery it may be possible to fetch
	 * the list of MUC participants without joining the MUC, to register
	 * on a gateway into another protocol, and such.
	 *
	 * The list of features, capabilities and such is represented via a
	 * model (as by Qt's MVC), and each capability (an index in that
	 * model) may have an action, like viewing information, registering
	 * on a gateway, and such.
	 *
	 * Since there may be several service discovery sessions active at a
	 * time, the account implementing IHaveServiceDiscovery serves
	 * merely as service discovery session factory.
	 *
	 * Whenever a service discovery session is needed by the core or
	 * other plugins, CreateSDSession() is called and it is expected to
	 * return an object representing a service discovery session and
	 * implementing ISDSession.
	 *
	 * Service discovery session may also be created by the plugin
	 * itself. For that, the plugin should create an object representing
	 * a service discovery session and emit the gotSDSession() signal.
	 *
	 * Azoth architecture and implementation is done with the XMPP
	 * protocol in mind.
	 *
	 * @sa IAccount, ISDSession
	 */
	class IHaveServiceDiscovery
	{
	public:
		virtual ~IHaveServiceDiscovery () {}

		/** @brief Creates a new service discovery session.
		 *
		 * This function is called by Azoth core or other plugins
		 * whenever a new service discovery session is required. This
		 * function is expected to return an object implementing
		 * ISDSession.
		 *
		 * The ownership of the returned object is passed to the caller.
		 *
		 * @return An object implementing ISDSession.
		 *
		 * @sa ISDSession, gotSDSession()
		 */
		virtual QObject* CreateSDSession () = 0;

		/** @brief Returns the default query for this account.
		 *
		 * The returned query (if non-empty) will be used to initiate
		 * service discovery upon opening the service discovery tab for
		 * this account.
		 *
		 * For example, an XMPP account would wish to return the user's
		 * server as default query.
		 *
		 * @return The default query.
		 */
		virtual QString GetDefaultQuery () const = 0;
	protected:
		/** @brief Notifies about a new service discovery session.
		 *
		 * This signal should be emitted when the account itself has
		 * created a service discovery session and wants to notify
		 * the rest of Azoth about it.
		 *
		 * The ownership of session is passed to the Azoth core.
		 *
		 * @param[out] session The newly created SD session implementing
		 * ISDSession.
		 *
		 * @sa CreateSDSession()
		 */
		virtual void gotSDSession (QObject *session) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISDSession,
		"org.Deviant.LeechCraft.Azoth.ISDSession/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveServiceDiscovery,
		"org.Deviant.LeechCraft.Azoth.IHaveServiceDiscovery/1.0");

#endif
