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

#ifndef PLUGINS_AZOTH_SORTFILTERPROXYMODEL_H
#define PLUGINS_AZOTH_SORTFILTERPROXYMODEL_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
namespace Azoth
{
	class SortFilterProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT

		bool ShowOffline_;
		bool MUCMode_;
		bool OrderByStatus_;
		bool HideMUCParts_;
		QObject *MUCEntry_;
	public:
		SortFilterProxyModel (QObject* = 0);

		void SetMUCMode (bool);
		bool IsMUCMode () const;
		void SetMUC (QObject*);
	public slots:
		void showOfflineContacts (bool);
	private slots:
		void handleStatusOrderingChanged ();
		void handleHideMUCPartsChanged ();
		void handleMUCDestroyed ();
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const;
		bool lessThan (const QModelIndex&, const QModelIndex&) const;
	signals:
		void mucMode ();
		void wholeMode ();
	};
}
}

#endif
