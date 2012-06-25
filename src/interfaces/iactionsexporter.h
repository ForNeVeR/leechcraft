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

#ifndef INTERFACES_IACTIONSEXPORTER_H
#define INTERFACES_IACTIONSEXPORTER_H
#include <QList>
#include <QMap>
#include <QtPlugin>

class QAction;

namespace LeechCraft
{
	enum class ActionsEmbedPlace
	{
		ToolsMenu,
		CommonContextMenu,
		QuickLaunch,
		TrayMenu,
		LCTray
	};
}

/** @brief Interface for embedding actions and menus into various
 * places.
 */
class IActionsExporter
{
public:
	virtual ~IActionsExporter () {}

	/** @brief Returns the actions to embed.
	 *
	 * Returns the list of actions that will be inserted into the Tools
	 * menu.
	 *
	 * @param[in] area The area where the actions should be placed.
	 *
	 * @return The list of actions for the given area.
	 */
	virtual QList<QAction*> GetActions (LeechCraft::ActionsEmbedPlace area) const = 0;

	/** @brief Returns the actions to embed into the menu.
	 *
	 * For each string key found in the returned map, the corresponding
	 * list of QActions would be added to the submenu under that name in
	 * the main menu. That allows several different plugins to insert
	 * actions into one menu easily.
	 *
	 * @return The map of menu name -> list of its actions.
	 */
	virtual QMap<QString, QList<QAction*>> GetMenuActions () const
	{
		return QMap<QString, QList<QAction*>> ();
	}
protected:
	/** @brief Notifies about new actions for the given area.
	 *
	 * The sender of this signal remains the owner of actions, and it
	 * may delete them at any given time.
	 *
	 * @param[out] actions The list of new actions for the given area.
	 * @param[out] area The area where these actions should be placed.
	 */
	virtual void gotActions (QList<QAction*> actions, LeechCraft::ActionsEmbedPlace area) = 0;
};

Q_DECLARE_INTERFACE (IActionsExporter, "org.Deviant.LeechCraft.IActionsExporter/1.0");

#endif
