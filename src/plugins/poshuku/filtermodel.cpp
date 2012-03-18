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

#include "filtermodel.h"
#include <QStringList>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "favoritesmodel.h"
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	FilterModel::FilterModel (QObject *parent)
	: LeechCraft::Util::TagsFilterModel (parent)
	{
	}

	FilterModel::~FilterModel ()
	{
	}

	QStringList FilterModel::GetTagsForIndex (int row) const
	{
		QStringList ids = sourceModel ()->data (sourceModel ()->index (row, 0),
				LeechCraft::RoleTags).toStringList ();
		QStringList tags;
		Q_FOREACH (QString id, ids)
			tags.append (Core::Instance ().GetProxy ()->
					GetTagsManager ()->GetTag (id));
		return tags;
	}
}
}
