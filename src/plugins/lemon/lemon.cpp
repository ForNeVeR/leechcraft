/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "lemon.h"
#include <QIcon>
#include "core.h"
#include "actionsmanager.h"

namespace LeechCraft
{
namespace Lemon
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Core::Instance ().SetProxy (proxy);

		Manager_ = new ActionsManager (this);
		connect (Manager_,
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
				this,
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lemon";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Lemon";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Global network status monitor.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		return place == ActionsEmbedPlace::LCTray ?
				Manager_->GetActions () :
				QList<QAction*> ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lemon, LeechCraft::Lemon::Plugin);
