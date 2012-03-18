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

#ifndef PLUGINS_LMP_LMP_H
#define PLUGINS_LMP_LMP_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/imediaplayer.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/iactionsexporter.h>

class QToolBar;

namespace LeechCraft
{
namespace LMP
{
	class LMP : public QObject
			  , public IInfo
			  , public IMediaPlayer
			  , public IHaveSettings
			  , public IEntityHandler
			  , public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IMediaPlayer IHaveSettings IEntityHandler IActionsExporter)

		std::auto_ptr<QTranslator> Translator_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);
		QIcon GetIcon () const;

		IVideoWidget* CreateWidget () const;
		IVideoWidget* GetDefaultWidget () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	signals:
		void bringToFront ();
		void gotEntity (const LeechCraft::Entity&);

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

#endif
