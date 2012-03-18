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

#include "dir.h"

using namespace LeechCraft;

Dir::Dir (QObject *parent)
: QObject (parent)
, Imp_ (new QDir)
{
}

Dir::Dir (const Dir& obj)
: QObject (obj.parent ())
, Imp_ (new QDir (*obj.Imp_))
{
}

Dir::~Dir ()
{
}

QString Dir::absoluteFilePath (const QString& file) const
{
	return Imp_->absoluteFilePath (file);
}

QString Dir::absolutePath () const
{
	return Imp_->absolutePath ();
}

QString Dir::canonicalPath () const
{
	return Imp_->canonicalPath ();
}

bool Dir::cd (const QString& dir)
{
	return Imp_->cd (dir);
}

bool Dir::cdUp ()
{
	return Imp_->cdUp ();
}

uint Dir::count () const
{
	return Imp_->count ();
}

QString Dir::dirName () const
{
	return Imp_->dirName ();
}

QFileInfoList Dir::entryInfoList (const QStringList& nameFilters,
		QDir::Filters filters, QDir::SortFlags sort) const
{
	return Imp_->entryInfoList (nameFilters, filters, sort);
}

QFileInfoList Dir::entryInfoList (QDir::Filters filters, QDir::SortFlags sort) const
{
	return Imp_->entryInfoList (filters, sort);
}

QStringList Dir::entryList (const QStringList& nameFilters,
		QDir::Filters filters, QDir::SortFlags sort) const
{
	return Imp_->entryList (nameFilters, filters, sort);
}

QStringList Dir::entryList (QDir::Filters filters, QDir::SortFlags sort) const
{
	return Imp_->entryList (filters, sort);
}

bool Dir::exists (const QString& dir) const
{
	return Imp_->exists (dir);
}

bool Dir::exists () const
{
	return Imp_->exists ();
}

QString Dir::filePath (const QString& path) const
{
	return Imp_->filePath (path);
}

QDir::Filters Dir::filter () const
{
	return Imp_->filter ();
}

bool Dir::isAbsolute () const
{
	return Imp_->isAbsolute ();
}

bool Dir::isReadable () const
{
	return Imp_->isReadable ();
}

bool Dir::isRelative () const
{
	return Imp_->isRelative ();
}

bool Dir::isRoot () const
{
	return Imp_->isRoot ();
}

bool Dir::makeAbsolute ()
{
	return Imp_->makeAbsolute ();
}

bool Dir::mkdir (const QString& path) const
{
	return Imp_->mkdir (path);
}

bool Dir::mkpath (const QString& path) const
{
	return Imp_->mkpath (path);
}

QStringList Dir::nameFilters () const
{
	return Imp_->nameFilters ();
}

QString Dir::path () const
{
	return Imp_->path ();
}

void Dir::refresh () const
{
	Imp_->refresh ();
}

QString Dir::relativeFilePath (const QString& path) const
{
	return Imp_->relativeFilePath (path);
}

bool Dir::remove (const QString& file)
{
	return Imp_->remove (file);
}

bool Dir::rename (const QString& old, const QString& newName)
{
	return Imp_->rename (old, newName);
}

bool Dir::rmdir (const QString& dir) const
{
	return Imp_->rmdir (dir);
}

bool Dir::rmpath (const QString& path) const
{
	return Imp_->rmpath (path);
}

void Dir::setFilter (QDir::Filters filters)
{
	Imp_->setFilter (filters);
}

void Dir::setNameFilters (const QStringList& filters)
{
	Imp_->setNameFilters (filters);
}

void Dir::setPath (const QString& path)
{
	Imp_->setPath (path);
}

void Dir::setSorting (QDir::SortFlags sorts)
{
	Imp_->setSorting (sorts);
}

QDir::SortFlags Dir::sorting () const
{
	return Imp_->sorting ();
}

bool Dir::operator!= (const Dir& other) const
{
	return *Imp_ != *other.Imp_;
}

Dir& Dir::operator= (const Dir& other)
{
	*Imp_ = *other.Imp_;
	return *this;
}

bool Dir::operator== (const Dir& other) const
{
	return *Imp_ == *other.Imp_;
}

QString Dir::operator[] (int pos) const
{
	return (*Imp_) [pos];
}

