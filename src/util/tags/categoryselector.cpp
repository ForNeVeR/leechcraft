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

#include "categoryselector.h"
#include <algorithm>
#include <QStringList>
#include <QCheckBox>
#include <QVariant>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QtDebug>

using namespace LeechCraft::Util;
const int RoleTag = 52;

CategorySelector::CategorySelector (QWidget *parent)
: QTreeWidget (parent)
{
	setWindowTitle (tr ("Tags selector"));
	setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);
	setRootIsDecorated (false);
	setUniformRowHeights (true);

	QRect avail = QApplication::desktop ()->availableGeometry (this);
	setMinimumHeight (avail.height () / 3 * 2);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	QAction *all = new QAction (tr ("Select all"), this);
	connect (all,
			SIGNAL (triggered ()),
			this,
			SLOT (selectAll ()));

	QAction *none = new QAction (tr ("Select none"), this);
	connect (none,
			SIGNAL (triggered ()),
			this,
			SLOT (selectNone ()));

	addAction (all);
	addAction (none);

	setContextMenuPolicy (Qt::ActionsContextMenu);
}

CategorySelector::~CategorySelector ()
{
}

void CategorySelector::SetCaption (const QString& caption)
{
	setHeaderLabel (caption);
	Caption_ = caption;
}

void CategorySelector::setPossibleSelections (QStringList mytags)
{
	disconnect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	clear ();

	mytags.sort ();
	QList<QTreeWidgetItem*> items;
	for (auto i = mytags.begin (), end = mytags.end (); i != end; ++i)
	{
		if (i->isEmpty ())
			continue;

		auto item = new QTreeWidgetItem (QStringList (*i));
		item->setCheckState (0, Qt::Unchecked);
		item->setData (0, RoleTag, *i);
		items << item;
	}
	addTopLevelItems (items);

	setHeaderLabel (Caption_);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	emit tagsSelectionChanged (QStringList ());
}

QStringList CategorySelector::GetSelections ()
{
	QStringList tags;

	for (int i = 0, size = topLevelItemCount ();
			i < size; ++i)
	{
		QTreeWidgetItem *item = topLevelItem (i);
		if (item->checkState (0) == Qt::Checked)
			tags += item->data (0, RoleTag).toString ();
	}

	return tags;
}

void CategorySelector::SetSelections (const QStringList& tags)
{
	blockSignals (true);
	for (int i = 0; i < topLevelItemCount (); ++i)
	{
		Qt::CheckState state =
			tags.contains (topLevelItem (i)->data (0, RoleTag).toString ()) ?
				Qt::Checked :
				Qt::Unchecked;
		topLevelItem (i)->setCheckState (0, state);
	}
	blockSignals (false);
}

void CategorySelector::moveEvent (QMoveEvent *e)
{
	QWidget::moveEvent (e);
	QPoint pos = e->pos ();
	QRect avail = QApplication::desktop ()->availableGeometry (this);
	int dx = 0, dy = 0;
	if (pos.x () + width () > avail.width ())
		dx = width () + pos.x () - avail.width ();
	if (pos.y () + height () > avail.height () &&
			height () < avail.height ())
		dy = height () + pos.y () - avail.height ();

	if (dx || dy)
		move (pos - QPoint (dx, dy));
}

void CategorySelector::selectAll ()
{
	disconnect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	QStringList tags;

	for (int i = 0, size = topLevelItemCount (); i < size; ++i)
	{
		QTreeWidgetItem *item = topLevelItem (i);
		item->setCheckState (0, Qt::Checked);
		tags += item->data (0, RoleTag).toString ();
	}

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	emit tagsSelectionChanged (tags);
}

void CategorySelector::selectNone ()
{
	disconnect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	for (int i = 0; i < topLevelItemCount (); ++i)
		topLevelItem (i)->setCheckState (0, Qt::Unchecked);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	emit tagsSelectionChanged (QStringList ());
}

void CategorySelector::lineTextChanged (const QString& text)
{
	QStringList tags = text.split ("; ", QString::SkipEmptyParts);
	SetSelections (tags);
}

void CategorySelector::buttonToggled ()
{
	emit tagsSelectionChanged (GetSelections ());
}

