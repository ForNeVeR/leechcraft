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

#include "proxyobject.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "browserwidget.h"

namespace LeechCraft
{
namespace Poshuku
{
	QObject* ProxyObject::GetHistoryModel () const
	{
		return Core::Instance ().GetHistoryModel ();
	}

	QObject* ProxyObject::GetFavoritesModel () const
	{
		return Core::Instance ().GetFavoritesModel ();
	}
	
	QObject* ProxyObject::OpenInNewTab (const QUrl& url, bool inverted) const
	{
		bool raise = XmlSettingsManager::Instance ()->
				property ("BackgroundNewTabs").toBool ();
		if (inverted)
			raise = !raise;
		return Core::Instance ().NewURL (url, raise);
	}
}
}
