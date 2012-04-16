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

#include "tagsmanager.h"
#include <stdexcept>
#include <algorithm>
#include <QStringList>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <util/util.h>
#include <util/tags/tagscompleter.h>

using namespace LeechCraft;

TagsManager::TagsManager ()
{
	ReadSettings ();
	GetID (tr ("untagged"));
	Util::TagsCompleter::SetModel (GetModel ());
}

TagsManager& TagsManager::Instance ()
{
	static TagsManager tm;
	return tm;
}

TagsManager::~TagsManager ()
{
}

int TagsManager::columnCount (const QModelIndex&) const
{
	return 1;
}

QVariant TagsManager::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () ||
			role != Qt::DisplayRole)
		return QVariant ();

	TagsDictionary_t::const_iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	return *pos;
}

QModelIndex TagsManager::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex TagsManager::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int TagsManager::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Tags_.size ();
}

ITagsManager::tag_id TagsManager::GetID (const QString& tag)
{
	const auto& keys = Tags_.keys (tag);
	if (keys.isEmpty ())
		return InsertTag (tag);
	else if (keys.size () > 1)
		throw std::runtime_error (qPrintable (QString ("More than one key for %1").arg (tag)));
	else
		return keys.at (0).toString ();
}

QString TagsManager::GetTag (ITagsManager::tag_id id) const
{
	return Tags_ [id];
}

QStringList TagsManager::GetAllTags () const
{
	return Tags_.values ();
}

QStringList TagsManager::Split (const QString& string) const
{
	const auto& splitted = string.split (";", QString::SkipEmptyParts);
	QStringList result;
	std::transform (splitted.begin (), splitted.end (), std::back_inserter (result),
			[] (const QString& s) { return s.trimmed (); });
	return result;
}

QStringList TagsManager::SplitToIDs (const QString& string)
{
	const auto& tags = Split (string);
	QStringList result;
	std::transform (tags.begin (), tags.end (), std::back_inserter (result),
			[this] (const QString& tag) { return GetID (tag.simplified ()); });
	return result;
}

QString TagsManager::Join (const QStringList& tags) const
{
	return tags.join ("; ");
}

QString TagsManager::JoinIDs (const QStringList& tagIDs) const
{
	QStringList hr;
	std::transform (tagIDs.begin (), tagIDs.end (), std::back_inserter (hr),
			[this] (const QString& id) { return GetTag (id); });
	return Join (hr);
}

ITagsManager::tag_id TagsManager::InsertTag (const QString& tag)
{
	beginInsertRows (QModelIndex (), Tags_.size (), Tags_.size ());
	QUuid uuid = QUuid::createUuid ();
	Tags_ [uuid] = tag;
	endInsertRows ();
	WriteSettings ();
	emit tagsUpdated (GetAllTags ());
	return uuid.toString ();
}

void TagsManager::RemoveTag (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	TagsDictionary_t::iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Tags_.erase (pos);
	endRemoveRows ();
	WriteSettings ();
	emit tagsUpdated (GetAllTags ());
}

void TagsManager::SetTag (const QModelIndex& index, const QString& newTag)
{
	if (!index.isValid ())
		return;

	TagsDictionary_t::iterator pos = Tags_.begin ();
	std::advance (pos, index.row ());
	*pos = newTag;

	emit dataChanged (index, index);

	WriteSettings ();

	emit tagsUpdated (GetAllTags ());
}

QAbstractItemModel* TagsManager::GetModel ()
{
	return this;
}

QObject* TagsManager::GetObject ()
{
	return this;
}

void TagsManager::ReadSettings ()
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("Tags");
	Tags_ = settings.value ("Dict").value<TagsDictionary_t> ();
	if (!Tags_.isEmpty ())
	{
		beginInsertRows (QModelIndex (), 0, Tags_.size () - 1);
		endInsertRows ();
	}
	settings.endGroup ();
}

void TagsManager::WriteSettings () const
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("Tags");
	settings.setValue ("Dict", QVariant::fromValue<TagsDictionary_t> (Tags_));
	settings.endGroup ();
}

