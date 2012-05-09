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

#pragma once

#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/media/ilyricsfinder.h>
#include "searcher.h"

namespace LeechCraft
{
namespace DeadLyrics
{
	class DeadLyRicS : public QObject
						, public IInfo
						, public IHaveSettings
						, public Media::ILyricsFinder
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings Media::ILyricsFinder)

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		ICoreProxy_ptr Proxy_;
		Searchers_t Searchers_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		void RequestLyrics (const Media::LyricsQuery&, Media::QueryOptions);
	signals:
		void gotLyrics (const Media::LyricsQuery&, const QStringList&);
	};
}
}
