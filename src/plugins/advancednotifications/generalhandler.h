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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_GENERALHANDLER_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_GENERALHANDLER_H
#include <QObject>
#include <QList>
#include <QIcon>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include "concretehandlerbase.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class GeneralHandler : public QObject
	{
		Q_OBJECT

		QList<ConcreteHandlerBase_ptr> Handlers_;

		ICoreProxy_ptr Proxy_;
		QMap<QString, QString> Cat2IconName_;
	public:
		GeneralHandler (ICoreProxy_ptr);

		void Handle (const Entity&);

		ICoreProxy_ptr GetProxy () const;

		QIcon GetIconForCategory (const QString&) const;
	signals:
		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

#endif
