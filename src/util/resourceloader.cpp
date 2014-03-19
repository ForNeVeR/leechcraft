/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "resourceloader.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QtDebug>
#include <QBuffer>

namespace LeechCraft
{
	namespace Util
	{
		ResourceLoader::ResourceLoader (const QString& relPath, QObject* parent)
		: QObject (parent)
		, RelativePath_ (relPath)
		, SubElemModel_ (new QStandardItemModel (this))
		, AttrFilters_ (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable)
		, SortModel_ (new QSortFilterProxyModel (this))
		, Watcher_ (new QFileSystemWatcher (this))
		, CacheFlushTimer_ (new QTimer (this))
		, CachePathContents_ (0)
		, CachePixmaps_ (0)
		{
			if (RelativePath_.startsWith ('/'))
				RelativePath_ = RelativePath_.mid (1);
			if (!RelativePath_.endsWith ('/'))
				RelativePath_.append ('/');

			SortModel_->setDynamicSortFilter (true);
			SortModel_->setSourceModel (SubElemModel_);
			SortModel_->sort (0);

			connect (Watcher_,
					SIGNAL (directoryChanged (const QString&)),
					this,
					SLOT (handleDirectoryChanged (const QString&)));

			connect (CacheFlushTimer_,
					SIGNAL (timeout ()),
					this,
					SLOT (handleFlushCaches ()));
		}

		void ResourceLoader::AddLocalPrefix (QString prefix)
		{
			if (!prefix.isEmpty () &&
					!prefix.endsWith ('/'))
				prefix.append ('/');
			QString result = QDir::homePath () + "/.leechcraft/data/" + prefix;
			LocalPrefixesChain_ << result;

			QDir testDir = QDir::home ();
			if (!testDir.exists (".leechcraft/data/" + prefix + RelativePath_))
			{
				qDebug () << Q_FUNC_INFO
						<< ".leechcraft/data/" + prefix + RelativePath_
						<< "doesn't exist, trying to create it...";

				if (!testDir.mkpath (".leechcraft/data/" + prefix + RelativePath_))
				{
					qWarning () << Q_FUNC_INFO
							<< "failed to create"
							<< ".leechcraft/data/" + prefix + RelativePath_;
				}
			}

			ScanPath (result + RelativePath_);

			Watcher_->addPath (result + RelativePath_);
		}

		void ResourceLoader::AddGlobalPrefix ()
		{
#ifdef Q_OS_MAC
			QStringList prefixes = QApplication::arguments ().contains ("-nobundle") ?
					QStringList ("/usr/local/share/leechcraft/") :
					QStringList (QApplication::applicationDirPath () + "/../Resources/share/");
#elif defined (Q_OS_WIN32)
			QStringList prefixes = QStringList (QApplication::applicationDirPath () + "/share/");
#elif defined (INSTALL_PREFIX)
			QStringList prefixes = QStringList (INSTALL_PREFIX "/share/leechcraft/");
#else
			QStringList prefixes = QStringList ("/usr/local/share/leechcraft/")
					<< "/usr/share/leechcraft/";
#endif
			bool hasBeenAdded = false;
			for (const QString& prefix : prefixes)
			{
				GlobalPrefixesChain_ << prefix;
				ScanPath (prefix + RelativePath_);

				if (QFile::exists (prefix + RelativePath_))
				{
					Watcher_->addPath (prefix + RelativePath_);
					hasBeenAdded = true;
				}
			}

			if (!hasBeenAdded)
				qWarning () << Q_FUNC_INFO
						<< "no prefixes have been added:"
						<< prefixes
						<< "; rel path:"
						<< RelativePath_;
		}

		void ResourceLoader::SetCacheParams (int size, int timeout)
		{
			if (qApp->property ("no-resource-caching").toBool ())
				return;

			if (size <= 0)
			{
				CacheFlushTimer_->stop ();

				handleFlushCaches ();
			}
			else
			{
				if (timeout > 0)
					CacheFlushTimer_->start (timeout);

				CachePathContents_.setMaxCost (size * 1024);
				CachePixmaps_.setMaxCost (size * 1024);
			}
		}

		void ResourceLoader::FlushCache ()
		{
			handleFlushCaches ();
		}

		QFileInfoList ResourceLoader::List (const QString& option,
				const QStringList& nameFilters, QDir::Filters filters) const
		{
			QSet<QString> alreadyListed;
			QFileInfoList result;
			for (const auto& prefix : LocalPrefixesChain_ + GlobalPrefixesChain_)
			{
				const QString& path = prefix + RelativePath_ + option;
				QDir dir (path);
				const QFileInfoList& list =
						dir.entryInfoList (nameFilters, filters);
				for (const auto& info : list)
				{
					const QString& fname = info.fileName ();
					if (alreadyListed.contains (fname))
						continue;

					alreadyListed << fname;
					result << info;
				}
			}

			return result;
		}

		QString ResourceLoader::GetPath (const QStringList& pathVariants) const
		{
			for (const auto& prefix : LocalPrefixesChain_ + GlobalPrefixesChain_)
				for (const auto& path : pathVariants)
				{
					const QString& can = QFileInfo (prefix + RelativePath_ + path).absoluteFilePath ();
					if (QFile::exists (can))
						return can;
				}

			return QString ();
		}

		namespace
		{
			QStringList IconizeBasename (const QString& basename)
			{
				QStringList variants;
				variants << basename + ".svg"
						<< basename + ".png"
						<< basename + ".jpg"
						<< basename + ".gif";
				return variants;
			}
		}

		QString ResourceLoader::GetIconPath (const QString& basename) const
		{
			return GetPath (IconizeBasename (basename));
		}

		QIODevice_ptr ResourceLoader::Load (const QStringList& pathVariants, bool open) const
		{
			QString path = GetPath (pathVariants);
			if (path.isNull ())
				return QIODevice_ptr ();

			if (CachePathContents_.contains (path))
			{
				std::shared_ptr<QBuffer> result (new QBuffer ());
				result->setData (*CachePathContents_ [path]);
				if (open)
					result->open (QIODevice::ReadOnly);
				return result;
			}

			std::shared_ptr<QFile> result (new QFile (path));

			if (!result->isSequential () &&
					result->size () < CachePathContents_.maxCost () / 2)
			{
				if (result->open (QIODevice::ReadOnly))
				{
					const QByteArray& data = result->readAll ();
					CachePathContents_.insert (path, new QByteArray (data), data.size ());
					result->close ();
				}
			}

			if (open)
				result->open (QIODevice::ReadOnly);

			return result;
		}

		QIODevice_ptr ResourceLoader::Load (const QString& pathVariant, bool open) const
		{
			return Load (QStringList (pathVariant), open);
		}

		QIODevice_ptr ResourceLoader::GetIconDevice (const QString& basename, bool open) const
		{
			return Load (IconizeBasename (basename), open);
		}

		QPixmap ResourceLoader::LoadPixmap (const QString& basename) const
		{
			if (CachePixmaps_.contains (basename))
				return *CachePixmaps_ [basename];

			auto dev = GetIconDevice (basename, true);
			if (!dev)
				return QPixmap ();

			const auto& data = dev->readAll ();

			QPixmap result;
			result.loadFromData (data);
			CachePixmaps_.insert (basename, new QPixmap (result), data.size ());
			return result;
		}

		QAbstractItemModel* ResourceLoader::GetSubElemModel () const
		{
			return SortModel_;
		}

		void ResourceLoader::SetAttrFilters (QDir::Filters filters)
		{
			AttrFilters_ = filters;
		}

		void ResourceLoader::SetNameFilters (const QStringList& filters)
		{
			NameFilters_ = filters;
		}

		void ResourceLoader::ScanPath (const QString& path)
		{
			for (const auto& entry : QDir (path).entryList (NameFilters_, AttrFilters_))
			{
				Entry2Paths_ [entry] << path;
				if (SubElemModel_->findItems (entry).size ())
					continue;

				SubElemModel_->appendRow (new QStandardItem (entry));
			}
		}

		void ResourceLoader::handleDirectoryChanged (const QString& path)
		{
			emit watchedDirectoriesChanged ();

			for (auto i = Entry2Paths_.begin (), end = Entry2Paths_.end (); i != end; ++i)
				i->removeAll (path);

			QFileInfo fi (path);
			if (fi.exists () &&
					fi.isDir () &&
					fi.isReadable ())
				ScanPath (path);

			QStringList toRemove;
			for (auto i = Entry2Paths_.begin (), end = Entry2Paths_.end (); i != end; ++i)
				if (i->isEmpty ())
					toRemove << i.key ();

			for (const auto& entry : toRemove)
			{
				Entry2Paths_.remove (entry);

				auto items = SubElemModel_->findItems (entry);
				for (auto item : SubElemModel_->findItems (entry))
					SubElemModel_->removeRow (item->row ());
			}
		}

		void ResourceLoader::handleFlushCaches ()
		{
			CachePathContents_.clear ();
			CachePixmaps_.clear ();
		}
	}
}
