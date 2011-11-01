/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "skinengine.h"
#include <algorithm>
#include <QAction>
#include <QTabWidget>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QApplication>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include <QTimer>

using namespace LeechCraft;

const int MaxIconSize = 32;

SkinEngine::SkinEngine ()
{
	QTimer *timer = new QTimer (this);
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (flushCaches ()));
	timer->start (60000);

	FindIconSets ();
}

SkinEngine& SkinEngine::Instance ()
{
	static SkinEngine e;
	return e;
}

SkinEngine::~SkinEngine ()
{
}

QMap<int, QString> SkinEngine::GetIconPath (const QString& actionIcon) const
{
	return IconName2Path_ [GetIconName (actionIcon)];
}

QIcon SkinEngine::GetIcon (const QString& actionIcon, const QString& actionIconOff) const
{
	const QPair<QString, QString>& namePair = qMakePair (actionIcon, actionIconOff);
	if (IconCache_.contains (namePair))
		return IconCache_ [namePair];

	QString icon = GetIconName (actionIcon);

	QIcon iconEntity;
	sizef_t files = IconName2Path_ [icon];
	for (sizef_t::const_iterator sizePair = files.begin ();
			sizePair != files.end (); ++sizePair)
		iconEntity.addFile (sizePair.value (),
				QSize (sizePair.key (), sizePair.key ()),
				QIcon::Normal,
				QIcon::On);

	if (actionIconOff.size ())
	{
		QString offIcon = GetIconName (actionIconOff);

		sizef_t offFiles = IconName2Path_ [offIcon];
		for (sizef_t::const_iterator sizePair = offFiles.begin ();
				sizePair != offFiles.end (); ++sizePair)
			iconEntity.addFile (sizePair.value (),
					QSize (sizePair.key (), sizePair.key ()),
					QIcon::Normal,
					QIcon::Off);
	}

	IconCache_ [namePair] = iconEntity;

	return iconEntity;
}

void SkinEngine::UpdateIconSet (const QList<QAction*>& actions)
{
	FindIcons ();

	for (QList<QAction*>::const_iterator i = actions.begin (),
			end = actions.end (); i != end; ++i)
	{
		if (!(*i)->property ("ActionIcon").isValid ())
			continue;

		SetIcon (*i);

		if ((*i)->property ("WatchActionIconChange").toBool ())
			(*i)->installEventFilter (this);
	}
}

void SkinEngine::UpdateIconSet (const QList<QTabWidget*>& tabs)
{
	FindIcons ();

	for (QList<QTabWidget*>::const_iterator i = tabs.begin (),
			end = tabs.end (); i != end; ++i)
	{
		QStringList icons = (*i)->property ("TabIcons").toString ()
			.split (" ", QString::SkipEmptyParts);

		int tab = 0;
		for (QStringList::const_iterator name = icons.begin ();
				name != icons.end (); ++name, ++tab)
		{
			QString icon = GetIconName (*name);;

			QIcon iconEntity;
			sizef_t files = IconName2Path_ [icon];
			for (sizef_t::const_iterator sizePair = files.begin ();
					sizePair != files.end (); ++sizePair)
				iconEntity.addFile (sizePair.value (),
						QSize (sizePair.key (), sizePair.key ()),
						QIcon::Normal,
						QIcon::On);

			(*i)->setTabIcon (tab, iconEntity);
		}
	}
}

QStringList SkinEngine::ListIcons () const
{
	return IconSets_;
}

bool SkinEngine::eventFilter (QObject *obj, QEvent *e)
{
	if (e->type () != QEvent::DynamicPropertyChange)
		return QObject::eventFilter (obj, e);

	QAction *act = qobject_cast<QAction*> (obj);
	if (!act)
		return QObject::eventFilter (obj, e);

	SetIcon (act);

	return QObject::eventFilter (obj, e);
}

QString SkinEngine::GetIconName (const QString& actionIcon) const
{
	QString icon;
	if (IconName2FileName_.contains (actionIcon))
		icon = IconName2FileName_ [actionIcon];
	else
		icon = QString ("lc_") + actionIcon;
	return icon;
}

void SkinEngine::SetIcon (QAction *act)
{
	QString actionIcon = act->property ("ActionIcon").toString ();
	QString actionIconOff = act->property ("ActionIconOff").toString ();

	act->setIcon (GetIcon (actionIcon, actionIconOff));
}

void SkinEngine::FindIconSets ()
{
	IconSets_.clear ();

	/** Platform-specific aths for *.mapping files are as follows:
	 *
	 * Unix:
	 * /usr/share/leechcraft/icons
	 * /usr/local/share/leechcraft/icons
	 *
	 * Mac OS X:
	 * %APP_DIR%/../Resources/icons
	 *
	 * Win32:
	 * %APP_DIR%/icons
	 */
#if defined (Q_WS_MAC)
	QDir dir (QApplication::applicationDirPath () + "/../Resources/icons");
	IconSets_ << dir.entryList (QStringList ("*.mapping"));
#elif defined (Q_WS_X11)
	#if defined (INSTALL_PREFIX)
	QDir dir = QDir (INSTALL_PREFIX "/share/leechcraft/icons");
	IconSets_ << dir.entryList (QStringList ("*.mapping"));
	#else
	QDir dir = QDir ("/usr/share/leechcraft/icons");
	IconSets_ << dir.entryList (QStringList ("*.mapping"));
	dir = QDir ("/usr/local/share/leechcraft/icons");
	IconSets_ << dir.entryList (QStringList ("*.mapping"));
	#endif
#elif defined (Q_WS_WIN)
	QDir dir = QDir::current ();
	if (dir.cd (QCoreApplication::applicationDirPath () + "/icons"))
		IconSets_ << dir.entryList (QStringList ("*.mapping"));
#endif

	dir = QDir::home ();
	if (dir.cd (".icons"))
		IconSets_ << dir.entryList (QStringList ("*.mapping"));
	dir = QDir::home ();
	if (dir.cd (".leechcraft") && dir.cd ("icons"))
		IconSets_ << dir.entryList (QStringList ("*.mapping"));

	for (QStringList::iterator i = IconSets_.begin (),
			end = IconSets_.end (); i != end; ++i)
		*i = i->left (i->size () - 8);

#ifndef Q_NO_DEBUG
	qDebug () << Q_FUNC_INFO
		<< "found"
		<< IconSets_;
#endif
}

void SkinEngine::FindIcons ()
{
	QString iconSet = XmlSettingsManager::Instance ()->
		property ("IconSet").toString ();

	if (iconSet != OldIconSet_)
	{
		IconName2Path_.clear ();
		IconName2FileName_.clear ();
		IconDirs_.clear ();
		IconCache_.clear ();

		OldIconSet_ = iconSet;

		FillMapping (QDir::homePath () + "/.icons", iconSet);
		FillMapping (QDir::homePath () + "/.leechcraft/icons", iconSet);
		CollectDir (QDir::homePath () + "/.icons", iconSet);
		CollectDir (QDir::homePath () + "/.leechcraft/icons", iconSet);

#if defined (Q_WS_MAC)
		FillMapping (QApplication::applicationDirPath () + "/../Resources/icons", iconSet);
		CollectDir (QApplication::applicationDirPath () + "/../Resources/icons", iconSet);
#elif defined (Q_WS_X11)
	#if defined (INSTALL_PREFIX)
		FillMapping (INSTALL_PREFIX "/share/leechcraft/icons", iconSet);
		CollectDir (INSTALL_PREFIX "/share/icons", iconSet);
	#else
		FillMapping ("/usr/share/leechcraft/icons", iconSet);
		FillMapping ("/usr/local/share/leechcraft/icons", iconSet);
		CollectDir ("/usr/share/icons", iconSet);
		CollectDir ("/usr/local/share/icons", iconSet);
	#endif
#elif defined (Q_WS_WIN)
		FillMapping (QApplication::applicationDirPath () + "/icons", iconSet);
		CollectDir (QApplication::applicationDirPath () + "/icons", iconSet);
#endif
	}
}

void SkinEngine::FillMapping (const QString& folder, const QString& iconSet)
{
	QDir dir (folder);

	if (dir.exists (iconSet + ".mapping"))
	{
		QFile mappingFile (dir.filePath (iconSet + ".mapping"));
		if (mappingFile.open (QIODevice::ReadOnly))
			ParseMapping (mappingFile);
		else
			qWarning () << Q_FUNC_INFO
				<< "failed to open mapping file"
				<< iconSet
				<< folder
				<< mappingFile.errorString ();
	}

	if (QFileInfo (dir.filePath (iconSet + ".mapping.d")).isDir ())
	{
		dir.cd (iconSet + ".mapping.d");
		Q_FOREACH (QString entry, dir.entryList (QDir::Files))
		{
			QFile mappingFile (dir.filePath (entry));
			if (mappingFile.open (QIODevice::ReadOnly))
				ParseMapping (mappingFile);
			else
				qWarning () << Q_FUNC_INFO
					<< "failed to open mapping file"
					<< folder
					<< iconSet
					<< "for mapping.d:"
					<< dir.filePath (entry)
					<< mappingFile.errorString ();
		}
	}
}

void SkinEngine::ParseMapping (QFile& mappingFile)
{
	QByteArray lineData = mappingFile.readLine ();
	while (!lineData.isEmpty ())
	{
		QStringList pair = QString::fromUtf8 (lineData)
			.split (' ', QString::SkipEmptyParts);
		if (pair.size () == 2)
		{
			// Thread mapping declarations in a special way.
			// /dirs contains a ;-separated list of directories with icons.
			if (pair.at (0).startsWith ('/'))
			{
				QString name = pair.at (0).mid (1);
				if (name == "dirs")
					IconDirs_ << pair.at (1).simplified ().split (';',
							QString::SkipEmptyParts);
			}
			else
				IconName2FileName_ [pair.at (0).simplified ()] = pair.at (1).simplified ();
		}

		lineData = mappingFile.readLine ();
	}
}

void SkinEngine::CollectDir (const QString& folder, const QString& iconSet)
{
	QList<int> numbers = GetDirForBase (folder, iconSet);
	QDir baseDir (folder);
	baseDir.cd (iconSet);
	for (QList<int>::const_iterator i = numbers.begin (),
			end = numbers.end (); i != end; ++i)
	{
		QDir current = baseDir;
		if (*i == 0)
		{
			current.cd ("scalable");

			QStringList subdirs = current.entryList (QStringList (),
					QDir::Dirs | QDir::NoDotAndDotDot);

			for (QStringList::const_iterator j = subdirs.begin (),
					subdirsEnd = subdirs.end (); j != subdirsEnd; ++j)
				if (!IconDirs_.size () || IconDirs_.contains (*j))
					CollectSubdir (current, *j, *i);
		}
		else if (*i >= 16 && *i <= 32)
		{
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);

			QStringList subdirs = current.entryList (QStringList (),
					QDir::Dirs | QDir::NoDotAndDotDot);

			for (QStringList::const_iterator j = subdirs.begin (),
					subdirsEnd = subdirs.end (); j != subdirsEnd; ++j)
				if (!IconDirs_.size () || IconDirs_.contains (*j))
					CollectSubdir (current, *j, *i);
		}
	}
}

void SkinEngine::CollectSubdir (QDir current, const QString& dir, int size)
{
	current.cd (dir);
	QFileInfoList infos =
		current.entryInfoList (QStringList ("*.png") << "*.svg",
				QDir::Files | QDir::Readable);

	QStringList values = IconName2FileName_.values ();
	for (QFileInfoList::const_iterator i = infos.begin (),
			infoEnd = infos.end (); i != infoEnd; ++i)
		if (values.contains (i->baseName ()) ||
				i->baseName ().startsWith ("lc_"))
			IconName2Path_ [i->baseName ()] [size] = i->absoluteFilePath ();
}

QList<int> SkinEngine::GetDirForBase (const QString& base,
		const QString& iconSet)
{
	QDir baseDir (base);
	baseDir.cd (iconSet);
	QList<int> numbers;
	QStringList entries = baseDir.entryList ();
	for (QStringList::const_iterator i = entries.begin (),
			end = entries.end (); i != end; ++i)
	{
		if (*i == "scalable")
		{
			numbers.push_back (0);
			continue;
		}

		QStringList splitted = i->split ('x');
		if (splitted.size () != 2)
			continue;

		int size = splitted.at (0).toInt ();
		if (size > MaxIconSize)
			continue;
		numbers.push_back (size);
	}

	std::sort (numbers.begin (), numbers.end ());
	return numbers;
}

void SkinEngine::flushCaches ()
{
	IconCache_.clear ();
}
