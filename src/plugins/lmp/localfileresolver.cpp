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

#include "localfileresolver.h"
#include <QtDebug>
#include <QFileInfo>
#include <taglib/fileref.h>
#include <taglib/tag.h>

namespace LeechCraft
{
namespace LMP
{
	ResolveError::ResolveError (const QString& path, const std::string& what)
	: runtime_error (what)
	, Path_ (path)
	{
	}

	ResolveError::~ResolveError () throw ()
	{
	}

	QString ResolveError::GetPath () const
	{
		return Path_;
	}

	LocalFileResolver::LocalFileResolver (QObject *parent)
	: QObject (parent)
	{
	}

	TagLib::FileRef LocalFileResolver::GetFileRef (const QString& file) const
	{
#ifdef Q_OS_WIN32
		return TagLib::FileRef (reinterpret_cast<const wchar_t*> (file.utf16 ()));
#else
		return TagLib::FileRef (file.toUtf8 ().constData ());
#endif
	}

	MediaInfo LocalFileResolver::ResolveInfo (const QString& file)
	{
		const auto& modified = QFileInfo (file).lastModified ();
		{
			QReadLocker locker (&CacheLock_);
			if (Cache_.contains (file))
			{
				const auto& pair = Cache_ [file];
				if (pair.first == modified)
					return pair.second;
			}
		}

		QMutexLocker tlLocker (&TaglibMutex_);

		auto r = GetFileRef (file);
		auto tag = r.tag ();
		if (!tag)
			throw ResolveError (file, "failed to get file tags");

		auto audio = r.audioProperties ();

		auto ftl = [] (const TagLib::String& str) { return QString::fromUtf8 (str.toCString (true)); };
		auto genres = ftl (tag->genre ()).split ('/', QString::SkipEmptyParts);
		std::for_each (genres.begin (), genres.end (),
				[] (QString& genre) { genre = genre.trimmed (); });

		MediaInfo info =
		{
			file,
			ftl (tag->artist ()),
			ftl (tag->album ()),
			ftl (tag->title ()),
			genres,
			audio ? audio->length () : 0,
			static_cast<qint32> (tag->year ()),
			static_cast<qint32> (tag->track ())
		};
		{
			QWriteLocker locker (&CacheLock_);
			if (Cache_.size () > 200)
				Cache_.clear ();
			Cache_ [file] = qMakePair (modified, info);
		}
		return info;
	}

	QMutex& LocalFileResolver::GetMutex ()
	{
		return TaglibMutex_;
	}
}
}
