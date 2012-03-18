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
#include <interfaces/structures.h>
#include "filter.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	class RuleOptionDialog;

	class UserFiltersModel : public QAbstractItemModel
	{
		Q_OBJECT

		Filter Filter_;
		QStringList Headers_;
	public:
		UserFiltersModel (QObject* = 0);

		int columnCount (const QModelIndex& = QModelIndex ()) const;
		QVariant data (const QModelIndex&, int) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = QModelIndex ()) const;

		const Filter& GetFilter () const;
		bool InitiateAdd (const QString& = QString ());
		void Modify (int);
		void Remove (int);

		void AddMultiFilters (QStringList);
	private:
		bool Add (const RuleOptionDialog&);
		void SplitRow (int*, bool*) const;
		void ReadSettings ();
		void WriteSettings () const;
	private slots:
		void blockImage ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
