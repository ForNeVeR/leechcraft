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

#include "tagscompleter.h"
#include <algorithm>
#include <QtDebug>
#include <QWidget>
#include <QStringList>
#include <QLineEdit>
#include "tagslineedit.h"

using namespace LeechCraft::Util;

QAbstractItemModel *LeechCraft::Util::TagsCompleter::CompletionModel_ = 0;

TagsCompleter::TagsCompleter (TagsLineEdit *toComplete, QObject *parent)
: QCompleter (parent)
{
	setCompletionRole (Qt::DisplayRole);
	setModel (CompletionModel_);
	toComplete->SetCompleter (this);
}

void TagsCompleter::OverrideModel (QAbstractItemModel *model)
{
	setModel (model);
}

QStringList TagsCompleter::splitPath (const QString& string) const
{
	QStringList splitted = string.split (";", QString::SkipEmptyParts);
	QStringList result;
	Q_FOREACH (QString s, splitted)
		result << s.trimmed ();
	return result;
}
