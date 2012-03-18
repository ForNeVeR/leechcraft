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

#include "itemhandlercustomwidget.h"
#include <QGridLayout>
#include <QLabel>

namespace LeechCraft
{
	ItemHandlerCustomWidget::ItemHandlerCustomWidget ()
	{
	}

	ItemHandlerCustomWidget::~ItemHandlerCustomWidget ()
	{
	}

	bool ItemHandlerCustomWidget::CanHandle (const QDomElement& element) const
	{
		return element.attribute ("type") == "customwidget";
	}

	void ItemHandlerCustomWidget::Handle (const QDomElement& item, QWidget *pwidget)
	{
		QGridLayout *lay = qobject_cast<QGridLayout*> (pwidget->layout ());
		QWidget *widget = new QWidget (XSD_);
		widget->setObjectName (item.attribute ("name"));
		QVBoxLayout *layout = new QVBoxLayout ();
		layout->setContentsMargins (0, 0, 0, 0);
		widget->setLayout (layout);
		widget->setSizePolicy (QSizePolicy::Expanding,
				QSizePolicy::Expanding);

		if (item.attribute ("label") == "own")
		{
			lay->setRowStretch (0, 1);
			lay->addWidget (widget, 0, 0, 1, -1);
		}
	}
};
