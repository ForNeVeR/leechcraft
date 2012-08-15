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

#include <boost/optional.hpp>
#include <QObject>
#include <QMap>
#include <interfaces/media/audiostructs.h>

class QStandardItem;
class QNetworkAccessManager;

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	class AuthManager;
	class AccountsManager;

	class PlaylistManager : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;

		AuthManager *AuthMgr_;
		AccountsManager *AccMgr_;
		QStandardItem *Root_;

		QMap<QString, QStandardItem*> AccItems_;
		QMap<QString, QMap<QString, QStandardItem*>> AccPlaylists_;

		QHash<QUrl, Media::AudioInfo> Infos_;
	public:
		PlaylistManager (QNetworkAccessManager*, AuthManager*, AccountsManager*, QObject* = 0);

		QStandardItem* GetRoot () const;
		void Update ();

		boost::optional<Media::AudioInfo> GetMediaInfo (const QUrl&) const;
	private slots:
		void requestPlaylists (const QString&);
		void handleGotPlaylists ();
		void handleGotPlaylistContents ();
		void handleAccountsChanged ();
	};
}
}
}
