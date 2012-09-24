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

#pragma once

#include <QAbstractItemModel>
#include <util/utilconfig.h>

namespace LeechCraft
{
namespace Util
{
	class UTIL_API FlattenFilterModel : public QAbstractItemModel
	{
		Q_OBJECT
	protected:
		QAbstractItemModel *Source_;
		QList<QPersistentModelIndex> SourceIndexes_;
	public:
		FlattenFilterModel (QObject* = 0);

		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& parent = QModelIndex()) const;
		int columnCount (const QModelIndex& parent = QModelIndex()) const;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;

		void SetSource (QAbstractItemModel*);
	protected:
		virtual bool IsIndexAccepted (const QModelIndex&) const;
	private slots:
		void handleDataChanged (const QModelIndex&, const QModelIndex&);
		void handleRowsInserted (const QModelIndex&, int, int);
		void handleRowsAboutRemoved (const QModelIndex&, int, int);
	};
}
}
