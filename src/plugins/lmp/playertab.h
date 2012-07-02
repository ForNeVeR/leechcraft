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

#include <memory>
#include <QWidget>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "lmpsystemtrayicon.h"
#include "player.h"
#include "ui_playertab.h"

class QStandardItemModel;
class QSortFilterProxyModel;

namespace Media
{
	struct LyricsQuery;
}

namespace LeechCraft
{
struct Entity;

namespace LMP
{
	struct MediaInfo;
	class Player;

	class PlayerTab : public QWidget
					, public ITabWidget
					, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab);

		Ui::PlayerTab Ui_;

		QObject *Plugin_;
		const TabClassInfo TC_;

		QSortFilterProxyModel *CollectionFilterModel_;

		Player *Player_;
		QToolBar *TabToolbar_;

		QLabel *PlayedTime_;
		QLabel *RemainingTime_;

		QHash<QString, Media::SimilarityInfos_t> Similars_;
		QString LastSimilar_;

		LMPSystemTrayIcon *TrayIcon_;
		QAction *PlayPause_;
		QMenu *TrayMenu_;

		QAction *CollectionShowTrackProps_;
	public:
		PlayerTab (const TabClassInfo&, QObject*, QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		Player* GetPlayer () const;

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;

		void InitWithOtherPlugins ();
	private:
		void SetupToolbar ();
		void SetupCollection ();
		void SetupPlaylistsTab ();
		void SetNowPlaying (const MediaInfo&, const QPixmap&);
		void Scrobble (const MediaInfo&);
		void FillSimilar (const Media::SimilarityInfos_t&);
		void RequestLyrics (const MediaInfo&);
	private slots:
		void handleSongChanged (const MediaInfo&);
		void handleCurrentPlayTime (qint64);
		void handleLoveTrack ();

		void handleSimilarError ();
		void handleSimilarReady ();

		void handleGotLyrics (const Media::LyricsQuery&, const QStringList&);

		void handleScanProgress (int);
		void handlePlaylistSelected (const QModelIndex&);
		void showCollectionTrackProps ();
		void loadFromCollection ();
		void handleCollectionItemSelected (const QModelIndex&);

		void closeLMP ();
		void handleStateChanged (Phonon::State newState, Phonon::State oldState);
		void handleShowTrayIcon ();
		void handleChangedVolume (qreal delta);
		void handleTrayIconActivated (QSystemTrayIcon::ActivationReason reason);
	signals:
		void changeTabName (QWidget*, const QString&);
		void removeTab (QWidget*);
		void raiseTab (QWidget*);

		void fullRaiseRequested ();

		void gotEntity (const LeechCraft::Entity&);

		void tabRecoverDataChanged ();
	};
}
}
