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

#ifndef SKINENGINE_H
#define SKINENGINE_H
#include <QObject>
#include <QMap>
#include <QString>
#include <QDir>
#include <QHash>

class QIcon;
class QAction;
class QTabWidget;
class QFile;

namespace LeechCraft
{
	class SkinEngine : public QObject
	{
		Q_OBJECT

		QString OldIconSet_;
		typedef QMap<int, QString> sizef_t;
		QMap<QString, sizef_t> IconName2Path_;
		QMap<QString, QString> IconName2FileName_;
		QStringList IconSets_;
		QStringList IconDirs_;

		mutable QHash<QPair<QString, QString>, QIcon> IconCache_;

		SkinEngine ();
	public:
		static SkinEngine& Instance ();
		virtual ~SkinEngine ();

		QMap<int, QString> GetIconPath (const QString&) const;
		QIcon GetIcon (const QString&, const QString&) const;
		void UpdateIconSet (const QList<QAction*>&);
		void UpdateIconSet (const QList<QTabWidget*>&);
		QStringList ListIcons () const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		QString GetIconName (const QString&) const;
		void SetIcon (QAction*);
		void FindIconSets ();
		void FindIcons ();
		void FillMapping (const QString&, const QString&);
		void ParseMapping (QFile&);
		void CollectDir (const QString&, const QString&);
		void CollectSubdir (QDir, const QString&, int);
		QList<int> GetDirForBase (const QString&, const QString&);
	private slots:
		void flushCaches ();
	};
};

#endif

