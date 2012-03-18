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

#ifndef PLUGINS_AGGREGATOR_SINGLEREGEXP_H
#define PLUGINS_AGGREGATOR_SINGLEREGEXP_H
#include <QDialog>
#include "ui_singleregexp.h"

class QString;

namespace LeechCraft
{
namespace Aggregator
{
	class SingleRegexp : public QDialog
	{
		Q_OBJECT

		Ui::SingleRegexp Ui_;
	public:
		SingleRegexp (const QString& = QString (), const QString& = QString (), bool = false, QWidget* = 0);
		QString GetTitle () const;
		QString GetBody () const;
	private slots:
		void lineEdited (const QString&, QWidget* = 0);
	};
}
}

#endif
