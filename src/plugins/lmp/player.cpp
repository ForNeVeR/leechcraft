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

#include "player.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QMimeData>
#include <QUrl>
#include <QApplication>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <util/util.h>
#include "core.h"
#include "mediainfo.h"
#include "localfileresolver.h"
#include "util.h"
#include "localcollection.h"
#include "playlistmanager.h"
#include "staticplaylistmanager.h"
#include "xmlsettingsmanager.h"
#include "playlistparsers/playlistfactory.h"

Q_DECLARE_METATYPE (Phonon::MediaSource);

namespace Phonon
{
	uint qHash (const Phonon::MediaSource& src)
	{
		uint hash = 0;
		switch (src.type ())
		{
		case Phonon::MediaSource::LocalFile:
			hash = qHash (src.fileName ());
			break;
		case Phonon::MediaSource::Url:
			hash = qHash (src.url ());
			break;
		case Phonon::MediaSource::Disc:
			hash = src.discType ();
			break;
		case Phonon::MediaSource::Stream:
			hash = qHash (src.deviceName ());
			break;
		default:
			hash = 0;
			break;
		}
		return hash << src.type ();
	}
}

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class PlaylistModel : public QStandardItemModel
		{
			Player *Player_;
		public:
			PlaylistModel (Player *parent)
			: QStandardItemModel (parent)
			, Player_ (parent)
			{
				setSupportedDragActions (Qt::CopyAction | Qt::MoveAction);
			}

			QStringList mimeTypes () const
			{
				return QStringList ("text/uri-list");
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const
			{
				QList<QUrl> urls;
				Q_FOREACH (const auto& index, indexes)
				{
					const auto& sources = Player_->GetIndexSources (index);
					std::transform (sources.begin (), sources.end (), std::back_inserter (urls),
							[] (decltype (sources.front ()) source)
							{
								return source.type () == Phonon::MediaSource::LocalFile ?
										QUrl::fromLocalFile (source.fileName ()) :
										source.url ();
							});
				}
				urls.removeAll (QUrl ());

				QMimeData *result = new QMimeData;
				result->setUrls (urls);
				return result;
			}

			bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row, int, const QModelIndex& parent)
			{
				if (action == Qt::IgnoreAction)
					return true;

				if (!data->hasUrls ())
					return false;

				const auto& urls = data->urls ();
				QList<Phonon::MediaSource> sources;
				std::transform (urls.begin (), urls.end (), std::back_inserter (sources),
						[] (decltype (urls.front ()) url)
						{
							return url.scheme () == "file" ?
									Phonon::MediaSource (url.toLocalFile ()) :
									Phonon::MediaSource (url);
						});

				auto afterIdx = row >= 0 ?
						parent.child (row, 0) :
						parent;
				const auto& firstSrc = afterIdx.isValid () ?
						Player_->GetIndexSources (afterIdx).value (0) :
						Phonon::MediaSource ();

				auto existingQueue = Player_->GetQueue ();
				if (action == Qt::MoveAction)
					Q_FOREACH (const auto& src, sources)
					{
						auto pred = [&src] (decltype (existingQueue.front ()) item) -> bool
						{
							if (src.type () != item.type ())
								return false;
							if (src.type () == Phonon::MediaSource::LocalFile)
								return src.fileName () == item.fileName ();
							if (src.type () == Phonon::MediaSource::Url)
								return src.url () == item.url ();
							return false;
						};
						auto remPos = std::remove_if (existingQueue.begin (), existingQueue.end (), pred);
						existingQueue.erase (remPos, existingQueue.end ());
					}

				auto pos = std::find (existingQueue.begin (), existingQueue.end (), firstSrc);
				if (pos == existingQueue.end ())
					existingQueue << sources;
				else
				{
					Q_FOREACH (const auto& src, sources)
						pos = existingQueue.insert (pos, src) + 1;
				}

				Player_->ReplaceQueue (existingQueue);
				return true;
			}

			Qt::DropActions supportedDropActions () const
			{
				return Qt::CopyAction | Qt::MoveAction;
			}
		};
	}

	Player::Sorter::Sorter ()
	{
		Criteria_ << SortingCriteria::Artist
				<< SortingCriteria::Year
				<< SortingCriteria::Album
				<< SortingCriteria::TrackNumber;
	}

	bool Player::Sorter::operator() (const MediaInfo& left, const MediaInfo& right) const
	{
		Q_FOREACH (auto crit, Criteria_)
		{
			switch (crit)
			{
			case SortingCriteria::Artist:
				if (left.Artist_ != right.Artist_)
					return left.Artist_ < right.Artist_;
				break;
			case SortingCriteria::Year:
				if (left.Year_ != right.Year_)
					return left.Year_ < right.Year_;
				break;
			case SortingCriteria::Album:
				if (left.Album_ != right.Album_)
					return left.Album_ < right.Album_;
				break;
			case SortingCriteria::TrackNumber:
				if (left.TrackNumber_ != right.TrackNumber_)
					return left.TrackNumber_ < right.TrackNumber_;
				break;
			case SortingCriteria::TrackTitle:
				if (left.Title_ != right.Title_)
					return left.Title_ < right.Title_;
				break;
			case SortingCriteria::FilePath:
				if (left.LocalPath_ != right.LocalPath_)
					return left.LocalPath_ < right.LocalPath_;
				break;
			}
		}

		return left.LocalPath_ < right.LocalPath_;
	}

	Player::Player (QObject *parent)
	: QObject (parent)
	, PlaylistModel_ (new PlaylistModel (this))
	, Source_ (new Phonon::MediaObject (this))
	, Output_ (new Phonon::AudioOutput (Phonon::MusicCategory, this))
	, Path_ (Phonon::createPath (Source_, Output_))
	, RadioItem_ (0)
	, PlayMode_ (PlayMode::Sequential)
	{
		connect (Source_,
				SIGNAL (currentSourceChanged (Phonon::MediaSource)),
				this,
				SLOT (handleCurrentSourceChanged (Phonon::MediaSource)));
		connect (Source_,
				SIGNAL (aboutToFinish ()),
				this,
				SLOT (handleUpdateSourceQueue ()));
		Source_->setTickInterval (1000);
		Source_->setPrefinishMark (2000);

		XmlSettingsManager::Instance ().RegisterObject ("TransitionTime",
				this, "setTransitionTime");
		setTransitionTime ();

		connect (Source_,
				SIGNAL (finished ()),
				this,
				SLOT (handlePlaybackFinished ()));
		connect (Source_,
				SIGNAL (stateChanged (Phonon::State, Phonon::State)),
				this,
				SLOT (handleStateChanged (Phonon::State)));

		connect (Source_,
				SIGNAL (metaDataChanged ()),
				this,
				SLOT (handleMetadata ()));

		auto collection = Core::Instance ().GetLocalCollection ();
		if (collection->IsReady ())
			restorePlaylist ();
		else
			connect (collection,
					SIGNAL (collectionReady ()),
					this,
					SLOT (restorePlaylist ()));
	}

	QAbstractItemModel* Player::GetPlaylistModel () const
	{
		return PlaylistModel_;
	}

	Phonon::MediaObject* Player::GetSourceObject () const
	{
		return Source_;
	}

	Phonon::AudioOutput* Player::GetAudioOutput () const
	{
		return Output_;
	}

	Player::PlayMode Player::GetPlayMode () const
	{
		return PlayMode_;
	}

	void Player::SetPlayMode (Player::PlayMode playMode)
	{
		if (PlayMode_ == playMode)
			return;

		PlayMode_ = playMode;
		emit playModeChanged (PlayMode_);
	}

	void Player::SetSortingCriteria (const QList<Player::SortingCriteria>& criteria)
	{
		Sorter_.Criteria_ = criteria;

		AddToPlaylistModel (QList<Phonon::MediaSource> (), true);
	}

	namespace
	{
		QList<Phonon::MediaSource> FileToSource (const QString& file)
		{
			auto parser = MakePlaylistParser (file);
			if (parser)
				return parser (file);

			return QList<Phonon::MediaSource> () << Phonon::MediaSource (file);
		}
	}

	void Player::Enqueue (const QStringList& paths, bool sort)
	{
		QList<Phonon::MediaSource> sources;
		std::for_each (paths.begin (), paths.end (),
				[&sources] (decltype (paths.front ()) path)
					{ sources += FileToSource (path); });
		Enqueue (sources, sort);
	}

	void Player::Enqueue (const QList<Phonon::MediaSource>& sources, bool sort)
	{
		AddToPlaylistModel (sources, sort);
	}

	void Player::ReplaceQueue (const QList<Phonon::MediaSource>& queue, bool sort)
	{
		auto vals = Items_.values ();
		auto curSrcPos = std::find_if (vals.begin (), vals.end (),
				[] (decltype (vals.front ()) item) -> bool { return item->data (Player::Role::IsCurrent).toBool (); });
		const auto& currentSource = curSrcPos != vals.end () ?
				(*curSrcPos)->data (Role::Source).value<Phonon::MediaSource> () :
				Phonon::MediaSource ();

		PlaylistModel_->clear ();
		Items_.clear ();
		AlbumRoots_.clear ();
		CurrentQueue_.clear ();

		AddToPlaylistModel (queue, sort);

		if (Items_.contains (currentSource))
			Items_ [currentSource]->setData (true, Role::IsCurrent);
	}

	QList<Phonon::MediaSource> Player::GetQueue () const
	{
		return CurrentQueue_;
	}

	QList<Phonon::MediaSource> Player::GetIndexSources (const QModelIndex& index) const
	{
		QList<Phonon::MediaSource> sources;
		if (index.data (Role::IsAlbum).toBool ())
			for (int i = 0; i < PlaylistModel_->rowCount (index); ++i)
				sources << PlaylistModel_->index (i, 0, index).data (Role::Source).value<Phonon::MediaSource> ();
		else
			sources << index.data (Role::Source).value<Phonon::MediaSource> ();
		return sources;
	}

	namespace
	{
		void IncAlbumLength (QStandardItem *albumItem, int length)
		{
			const int prevLength = albumItem->data (Player::Role::AlbumLength).toInt ();
			albumItem->setData (length + prevLength, Player::Role::AlbumLength);
		}
	}

	void Player::Dequeue (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		Dequeue (GetIndexSources (index));
	}

	void Player::Dequeue (const QList<Phonon::MediaSource>& sources)
	{
		Q_FOREACH (const auto& source, sources)
		{
			if (!CurrentQueue_.removeAll (source))
				continue;

			auto item = Items_.take (source);
			auto parent = item->parent ();
			if (parent)
			{
				if (parent->rowCount () == 1)
				{
					Q_FOREACH (const auto& key, AlbumRoots_.keys ())
					{
						auto& items = AlbumRoots_ [key];
						if (!items.contains (parent))
							continue;

						items.removeAll (parent);
						if (items.isEmpty ())
							AlbumRoots_.remove (key);
					}
					PlaylistModel_->removeRow (parent->row ());
				}
				else
				{
					const auto& info = item->data (Role::Info).value<MediaInfo> ();
					if (!info.LocalPath_.isEmpty ())
						IncAlbumLength (parent, -info.Length_);
					parent->removeRow (item->row ());
				}
			}
			else
				PlaylistModel_->removeRow (item->row ());
		}

		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (CurrentQueue_);
	}

	void Player::SetStopAfter (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		Phonon::MediaSource stopSource;
		if (index.data (Role::IsAlbum).toBool ())
			stopSource = PlaylistModel_->index (0, 0, index).data (Role::Source).value<Phonon::MediaSource> ();
		else
			stopSource = index.data (Role::Source).value<Phonon::MediaSource> ();

		if (CurrentStopSource_.type () != Phonon::MediaSource::Empty)
			Items_ [CurrentStopSource_]->setData (false, Role::IsStop);

		if (CurrentStopSource_ == stopSource)
			CurrentStopSource_ = Phonon::MediaSource ();
		else
		{
			CurrentStopSource_ = stopSource;
			Items_ [stopSource]->setData (true, Role::IsStop);
		}
	}

	void Player::SetRadioStation (Media::IRadioStation_ptr station)
	{
		clear ();

		CurrentStation_ = station;

#ifdef GetObject
#undef GetObject
#endif

		connect (CurrentStation_->GetObject (),
				SIGNAL (gotError (const QString&)),
				this,
				SLOT (handleStationError (const QString&)));
		connect (CurrentStation_->GetObject (),
				SIGNAL (gotNewStream (QUrl, Media::AudioInfo)),
				this,
				SLOT (handleRadioStream (QUrl, Media::AudioInfo)));
		CurrentStation_->RequestNewStream ();

		auto radioName = station->GetRadioName ();
		if (radioName.isEmpty ())
			radioName = tr ("Radio");
		RadioItem_ = new QStandardItem (radioName);
		RadioItem_->setEditable (false);
		PlaylistModel_->appendRow (RadioItem_);
	}

	MediaInfo Player::GetCurrentMediaInfo () const
	{
		const auto& source = Source_->currentSource ();
		if (source.type () == Phonon::MediaSource::Empty)
			return MediaInfo ();

		auto info = GetMediaInfo (source);
		if (!info.LocalPath_.isEmpty ())
			return info;

		info = GetPhononMediaInfo ();
		return info;
	}

	QString Player::GetCurrentAAPath () const
	{
		const auto& info = GetCurrentMediaInfo ();
		auto coll = Core::Instance ().GetLocalCollection ();
		auto album = coll->GetAlbum (coll->FindAlbum (info.Artist_, info.Album_));
		return album ? album->CoverPath_ : QString ();;
	}

	MediaInfo Player::GetMediaInfo (const Phonon::MediaSource& source) const
	{
		return Items_.contains (source) ?
				Items_ [source]->data (Role::Info).value<MediaInfo> () :
				MediaInfo ();
	}

	MediaInfo Player::GetPhononMediaInfo () const
	{
		MediaInfo info;
		info.Artist_ = Source_->metaData (Phonon::ArtistMetaData).value (0);
		info.Album_ = Source_->metaData (Phonon::AlbumMetaData).value (0);
		info.Title_ = Source_->metaData (Phonon::TitleMetaData).value (0);
		info.Genres_ = Source_->metaData (Phonon::GenreMetaData);
		info.TrackNumber_ = Source_->metaData (Phonon::TracknumberMetaData).value (0).toInt ();
		info.Length_ = Source_->totalTime () / 1000;
		return info;
	}

	namespace
	{
		void FillItem (QStandardItem *item, const MediaInfo& info)
		{
			item->setText (QString ("%1 - %2 - %3")
						.arg (info.Artist_)
						.arg (info.Album_)
						.arg (info.Title_));
			item->setData (QVariant::fromValue (info), Player::Role::Info);
		}

		QStandardItem* MakeAlbumItem (const MediaInfo& info)
		{
			auto albumItem = new QStandardItem (QString ("%1 - %2")
							.arg (info.Artist_, info.Album_));
			albumItem->setEditable (false);
			albumItem->setData (true, Player::Role::IsAlbum);
			albumItem->setData (QVariant::fromValue (info), Player::Role::Info);
			auto art = FindAlbumArt (info.LocalPath_);
			if (art.isNull ())
				art = QIcon::fromTheme ("media-optical").pixmap (64, 64);
			albumItem->setData (art, Player::Role::AlbumArt);
			albumItem->setData (0, Player::Role::AlbumLength);
			return albumItem;
		}
	}

	void Player::AddToPlaylistModel (QList<Phonon::MediaSource> sources, bool sort)
	{
		if (!CurrentQueue_.isEmpty ())
		{
			ReplaceQueue (CurrentQueue_ + sources, sort);
			return;
		}

		PlaylistModel_->setHorizontalHeaderLabels (QStringList (tr ("Playlist")));

		if (sort)
			ApplyOrdering (sources);
		CurrentQueue_ = sources;

		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (CurrentQueue_);

		auto resolver = Core::Instance ().GetLocalFileResolver ();

		QPair<QString, QString> prevAlbumRoot;

		Q_FOREACH (const auto& source, sources)
		{
			auto item = new QStandardItem ();
			item->setEditable (false);
			item->setData (QVariant::fromValue (source), Role::Source);
			switch (source.type ())
			{
			case Phonon::MediaSource::Stream:
				item->setText (tr ("Stream"));
				PlaylistModel_->appendRow (item);
				break;
			case Phonon::MediaSource::Url:
			{
				const auto& url = source.url ();

				const auto info = Core::Instance ().TryURLResolve (url);
				if (info)
					FillItem (item, *info);
				else
					item->setText (url.toString ());

				PlaylistModel_->appendRow (item);
				break;
			}
			case Phonon::MediaSource::LocalFile:
			{
				MediaInfo info;
				try
				{
					info = resolver->ResolveInfo (source.fileName ());
				}
				catch (const ResolveError& error)
				{
					qWarning () << Q_FUNC_INFO
							<< "error resolving info for"
							<< error.GetPath ()
							<< error.what ();
					continue;
				}

				const auto& albumID = qMakePair (info.Artist_, info.Album_);
				FillItem (item, info);
				if (albumID != prevAlbumRoot ||
						AlbumRoots_ [albumID].isEmpty ())
				{
					PlaylistModel_->appendRow (item);
					AlbumRoots_ [albumID] << item;
				}
				else if (AlbumRoots_ [albumID].last ()->data (Role::IsAlbum).toBool ())
				{
					IncAlbumLength (AlbumRoots_ [albumID].last (), info.Length_);
					AlbumRoots_ [albumID].last ()->appendRow (item);
				}
				else
				{
					auto albumItem = MakeAlbumItem (info);

					const int row = AlbumRoots_ [albumID].last ()->row ();
					const auto& existing = PlaylistModel_->takeRow (row);
					albumItem->appendRow (existing);
					albumItem->appendRow (item);
					PlaylistModel_->insertRow (row, albumItem);

					const auto& existingInfo = existing.at (0)->data (Role::Info).value<MediaInfo> ();
					albumItem->setData (existingInfo.Length_, Role::AlbumLength);
					IncAlbumLength (albumItem, info.Length_);

					emit insertedAlbum (albumItem->index ());

					AlbumRoots_ [albumID].last () = albumItem;
				}
				prevAlbumRoot = albumID;
				break;
			}
			default:
				item->setText ("unknown");
				PlaylistModel_->appendRow (item);
				break;
			}

			Items_ [source] = item;
		}
	}

	void Player::ApplyOrdering (QList<Phonon::MediaSource>& sources)
	{
		if (Sorter_.Criteria_.isEmpty ())
			return;

		auto resolver = Core::Instance ().GetLocalFileResolver ();
		std::sort (sources.begin (), sources.end (),
				[resolver, this] (const Phonon::MediaSource& s1, const Phonon::MediaSource& s2) -> bool
				{
					if (s1.type () != Phonon::MediaSource::LocalFile ||
						s2.type () != Phonon::MediaSource::LocalFile)
						return qHash (s1) < qHash (s2);

					try
					{
						const auto& left = resolver->ResolveInfo (s1.fileName ());
						const auto& right = resolver->ResolveInfo (s2.fileName ());
						return Sorter_ (left, right);
					}
					catch (...)
					{
						return s1.fileName () < s2.fileName ();
					}
				});
	}

	bool Player::HandleCurrentStop (const Phonon::MediaSource& source)
	{
		if (source != CurrentStopSource_)
			return false;

		CurrentStopSource_ = Phonon::MediaSource ();
		Items_ [source]->setData (false, Role::IsStop);

		return true;
	}

	void Player::UnsetRadio ()
	{
		if (!CurrentStation_)
			return;

		PlaylistModel_->removeRow (RadioItem_->row ());
		RadioItem_ = 0;

		CurrentStation_.reset ();
	}

	Phonon::MediaSource Player::GetNextSource (const Phonon::MediaSource& current) const
	{
		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		switch (PlayMode_)
		{
		case PlayMode::Sequential:
			if (pos != CurrentQueue_.end () && ++pos != CurrentQueue_.end ())
				return *pos;
			else
				return Phonon::MediaSource ();
		case PlayMode::Shuffle:
			return CurrentQueue_.at (qrand () % CurrentQueue_.size ());
		case PlayMode::RepeatTrack:
			return current;
		case PlayMode::RepeatAlbum:
		{
			if (pos == CurrentQueue_.end ())
				return Phonon::MediaSource ();

			const auto& curAlbum = GetMediaInfo (*pos).Album_;
			if (++pos == CurrentQueue_.end () ||
					GetMediaInfo (*pos).Album_ != curAlbum)
				while (pos >= CurrentQueue_.begin () &&
						GetMediaInfo (*pos).Album_ == curAlbum)
					--pos;
			return *pos;
		}
		case PlayMode::RepeatWhole:
			if (pos == CurrentQueue_.end () || ++pos == CurrentQueue_.end ())
				pos = CurrentQueue_.begin ();
			return *pos;
		}

		return Phonon::MediaSource ();
	}

	void Player::play (const QModelIndex& index)
	{
		if (CurrentStation_)
		{
			auto item = PlaylistModel_->itemFromIndex (index);
			if (item == RadioItem_)
				return;
			else
				UnsetRadio ();
		}

		if (index.data (Role::IsAlbum).toBool ())
		{
			play (index.child (0, 0));
			return;
		}

		if (!index.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		Source_->stop ();
		const auto& source = index.data (Role::Source).value<Phonon::MediaSource> ();
		Source_->setCurrentSource (source);
		Source_->play ();
	}

	void Player::previousTrack ()
	{
		const auto& current = Source_->currentSource ();
		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		if (pos == CurrentQueue_.end () || pos == CurrentQueue_.begin ())
			return;

		Source_->stop ();
		Source_->setCurrentSource (*(--pos));
		Source_->play ();
	}

	void Player::nextTrack ()
	{
		if (CurrentStation_)
		{
			Source_->clear ();
			qApp->processEvents ();
			CurrentStation_->RequestNewStream ();
			return;
		}

		const auto& current = Source_->currentSource ();
		auto pos = std::find (CurrentQueue_.begin (), CurrentQueue_.end (), current);
		if (pos == CurrentQueue_.end () || pos == CurrentQueue_.end () - 1)
			return;

		Source_->stop ();
		Source_->setCurrentSource (*(++pos));
		Source_->play ();
	}

	void Player::togglePause ()
	{
		if (Source_->state () == Phonon::PlayingState)
			Source_->pause ();
		else
		{
			const auto& current = Source_->currentSource ();
			const bool invalidSource = current.type () == Phonon::MediaSource::Invalid ||
				current.type () == Phonon::MediaSource::Empty;
			if (invalidSource)
				Source_->setCurrentSource (CurrentQueue_.value (0));
			Source_->play ();
		}
	}

	void Player::setPause ()
	{
		Source_->pause ();
	}

	void Player::stop ()
	{
		Source_->stop ();
		emit songChanged (MediaInfo ());

		if (CurrentStation_)
			UnsetRadio ();
	}

	void Player::clear ()
	{
		UnsetRadio ();

		PlaylistModel_->clear ();
		Items_.clear ();
		AlbumRoots_.clear ();
		CurrentQueue_.clear ();
		Source_->clearQueue ();

		Core::Instance ().GetPlaylistManager ()->
				GetStaticManager ()->SetOnLoadPlaylist (CurrentQueue_);
	}

	void Player::restorePlaylist ()
	{
		auto staticMgr = Core::Instance ().GetPlaylistManager ()->GetStaticManager ();
		Enqueue (staticMgr->GetOnLoadPlaylist ());

		const auto& song = XmlSettingsManager::Instance ().property ("LastSong").toString ();
		if (!song.isEmpty ())
		{
			const auto pos = std::find_if (CurrentQueue_.begin (), CurrentQueue_.end (),
					[&song] (decltype (CurrentQueue_.front ()) item) { return song == item.fileName (); });
			if (pos != CurrentQueue_.end ())
				Source_->setCurrentSource (*pos);
		}
	}

	void Player::handleStationError (const QString& error)
	{
		const auto& e = Util::MakeNotification ("LMP",
				tr ("Radio station error: %1.")
					.arg (error),
				PCritical_);
		Core::Instance ().SendEntity (e);
	}

	void Player::handleRadioStream (const QUrl& url, const Media::AudioInfo& info)
	{
		Url2Info_ [url] = info;
		Source_->enqueue (url);

		qDebug () << Q_FUNC_INFO << Source_->state ();
		if (Source_->state () == Phonon::StoppedState)
			Source_->play ();
	}

	void Player::handleUpdateSourceQueue ()
	{
		if (CurrentStation_)
		{
			CurrentStation_->RequestNewStream ();
			return;
		}

		const auto& current = Source_->currentSource ();
		const auto& path = current.fileName ();
		if (!path.isEmpty ())
			QMetaObject::invokeMethod (Core::Instance ().GetLocalCollection (),
					"recordPlayedTrack",
					Qt::QueuedConnection,
					Q_ARG (QString, path));

		if (HandleCurrentStop (current))
			return;

		const auto& next = GetNextSource (current);
		if (next.type () != Phonon::MediaSource::Empty)
			Source_->enqueue (next);
	}

	void Player::handlePlaybackFinished ()
	{
		emit songChanged (MediaInfo ());
	}

	void Player::handleStateChanged (Phonon::State state)
	{
		qDebug () << Q_FUNC_INFO << state;
		if (state == Phonon::ErrorState)
			qDebug () << Source_->errorType () << Source_->errorString ();
	}

	void Player::handleCurrentSourceChanged (const Phonon::MediaSource& source)
	{
		XmlSettingsManager::Instance ().setProperty ("LastSong", source.fileName ());

		QStandardItem *curItem = 0;
		if (CurrentStation_)
			curItem = RadioItem_;
		else if (Items_.contains (source))
			curItem = Items_ [source];

		if (curItem)
			curItem->setData (true, Role::IsCurrent);

		if (CurrentStation_)
		{
			const auto& info = Url2Info_.take (source.url ());
			emit songChanged (info);
		}
		else if (curItem)
			emit songChanged (curItem->data (Role::Info).value<MediaInfo> ());
		else
			emit songChanged (MediaInfo ());

		Q_FOREACH (auto item, Items_.values ())
		{
			if (item == curItem)
				continue;
			if (item->data (Role::IsCurrent).toBool ())
			{
				item->setData (false, Role::IsCurrent);
				break;
			}
		}
	}

	void Player::handleMetadata ()
	{
		const auto& source = Source_->currentSource ();
		const auto& isUrl = source.type () == Phonon::MediaSource::Stream ||
				(source.type () == Phonon::MediaSource::Url && source.url ().scheme () != "file");
		if (!isUrl ||
				CurrentStation_ ||
				!Items_.contains (source))
			return;

		auto curItem = Items_ [source];

		const auto& info = GetPhononMediaInfo ();
		FillItem (curItem, info);
		emit songChanged (info);
	}

	void Player::setTransitionTime ()
	{
		const int time = XmlSettingsManager::Instance ()
				.property ("TransitionTime").toInt ();
		Source_->setTransitionTime (time);
	}
}
}
