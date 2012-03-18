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

#ifndef PLUGINS_QROSP_WRAPPERS_TAGSMANAGERWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_TAGSMANAGERWRAPPER_H
#include <QObject>
#include <QStringList>

class ITagsManager;
class QAbstractItemModel;

namespace LeechCraft
{
namespace Qrosp
{
	class TagsManagerWrapper : public QObject
	{
		Q_OBJECT

		ITagsManager *Manager_;
	public:
		TagsManagerWrapper (ITagsManager*);
	public slots:
		QString GetID (const QString& tag);
		QString GetTag (const QString& id) const;
		QStringList GetAllTags () const;
		QStringList Split (const QString& string) const;
		QString Join (const QStringList& tags) const;
		QAbstractItemModel* GetModel ();
		QObject* GetObject ();
	signals:
		void tagsUpdated (const QStringList& tags);
	};
}
}

#endif
