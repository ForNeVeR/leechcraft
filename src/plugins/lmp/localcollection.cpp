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

#include "localcollection.h"
#include <functional>
#include <algorithm>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QtDebug>
#include "localcollectionstorage.h"
#include "core.h"
#include "util.h"
#include "localfileresolver.h"
#include "player.h"
#include "albumartmanager.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		template<typename T>
		bool VarCompare (const QVariant& left, const QVariant& right)
		{
			return left.value<T> () < right.value<T> ();
		}

		struct Comparators
		{
			typedef std::function<bool (const QVariant&, const QVariant&)> Comparator_t;
			QHash<LocalCollection::Role, Comparator_t> Role2Cmp_;

			Comparators ()
			{
				Role2Cmp_ [LocalCollection::Role::ArtistName] = VarCompare<QString>;
				Role2Cmp_ [LocalCollection::Role::AlbumName] = VarCompare<QString>;
				Role2Cmp_ [LocalCollection::Role::AlbumYear] = VarCompare<int>;
				Role2Cmp_ [LocalCollection::Role::TrackNumber] = VarCompare<int>;
				Role2Cmp_ [LocalCollection::Role::TrackTitle] = VarCompare<QString>;
				Role2Cmp_ [LocalCollection::Role::TrackPath] = VarCompare<QString>;
			}
		};

		bool RoleCompare (const QModelIndex& left, const QModelIndex& right,
				QList<LocalCollection::Role> roles)
		{
			static Comparators comparators;
			while (!roles.isEmpty ())
			{
				auto role = roles.takeFirst ();
				const auto& lData = left.data (role);
				const auto& rData = right.data (role);
				if (lData != rData)
					return comparators.Role2Cmp_ [role] (lData, rData);
			}
			return false;
		}

		class CollectionSorter : public QSortFilterProxyModel
		{
		public:
			CollectionSorter (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
			}
		protected:
			bool lessThan (const QModelIndex& left, const QModelIndex& right) const
			{
				const auto type = left.data (LocalCollection::Role::Node).toInt ();
				QList<LocalCollection::Role> roles;
				switch (type)
				{
				case LocalCollection::NodeType::Artist:
					roles << LocalCollection::Role::ArtistName;
					break;
				case LocalCollection::NodeType::Album:
					roles << LocalCollection::Role::AlbumYear
							<< LocalCollection::Role::AlbumName;
					break;
				case LocalCollection::NodeType::Track:
					roles << LocalCollection::Role::TrackNumber
							<< LocalCollection::Role::TrackTitle
							<< LocalCollection::Role::TrackPath;
					break;
				default:
					return QSortFilterProxyModel::lessThan (left, right);
				}
				return RoleCompare (left, right, roles);
			}
		};

		class CollectionModel : public QStandardItemModel
		{
			LocalCollection *Collection_;
		public:
			CollectionModel (LocalCollection *parent)
			: QStandardItemModel (parent)
			, Collection_ (parent)
			{
				setSupportedDragActions (Qt::CopyAction);
			}

			QStringList mimeTypes () const
			{
				return QStringList ("text/uri-list");
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const
			{
				QMimeData *result = new QMimeData;

				QList<QUrl> urls;
				Q_FOREACH (const auto& index, indexes)
				{
					const auto& paths = Collection_->CollectPaths (index);
					std::transform (paths.begin (), paths.end (), std::back_inserter (urls),
							[] (const QString& path) { return QUrl::fromLocalFile (path); });
				}

				result->setUrls (urls);

				return result;
			}
		};
	}

	LocalCollection::LocalCollection (QObject *parent)
	: QObject (parent)
	, IsReady_ (false)
	, Storage_ (new LocalCollectionStorage (this))
	, CollectionModel_ (new CollectionModel (this))
	, Sorter_ (new CollectionSorter (this))
	, AlbumArtMgr_ (new AlbumArtManager (this))
	, Watcher_ (new QFutureWatcher<MediaInfo> (this))
	{
		connect (Watcher_,
				SIGNAL (finished ()),
				this,
				SLOT (handleScanFinished ()));
		connect (Watcher_,
				SIGNAL (progressValueChanged (int)),
				this,
				SIGNAL (scanProgressChanged (int)));

		auto loadWatcher = new QFutureWatcher<Collection::Artists_t> ();
		connect (loadWatcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleLoadFinished ()));
		auto worker = [] () { return LocalCollectionStorage ().Load (); };
		auto future = QtConcurrent::run (std::function<Collection::Artists_t ()> (worker));
		loadWatcher->setFuture (future);

		Sorter_->setSourceModel (CollectionModel_);
		Sorter_->setDynamicSortFilter (true);
		Sorter_->sort (0);
	}

	void LocalCollection::FinalizeInit ()
	{
		ArtistIcon_ = Core::Instance ().GetProxy ()->GetIcon ("view-media-artist");
	}

	bool LocalCollection::IsReady () const
	{
		return IsReady_;
	}

	QAbstractItemModel* LocalCollection::GetCollectionModel () const
	{
		return Sorter_;
	}

	void LocalCollection::Enqueue (const QModelIndex& index, Player *player)
	{
		player->Enqueue (CollectPaths (Sorter_->mapToSource (index)));
	}

	void LocalCollection::Clear ()
	{
		Storage_->Clear ();
		CollectionModel_->clear ();
		PresentPaths_.clear ();
		Artist2Item_.clear ();
		Album2Item_.clear ();
	}

	void LocalCollection::Scan (const QString& path)
	{
		auto resolver = Core::Instance ().GetLocalFileResolver ();
		auto paths = QSet<QString>::fromList (RecIterate (path));
		paths.subtract (PresentPaths_);
		if (paths.isEmpty ())
			return;

		PresentPaths_ += paths;
		emit scanStarted (paths.size ());
		auto worker = [resolver] (const QString& path) -> LMP::MediaInfo
		{
			try
			{
				return resolver->ResolveInfo (path);
			}
			catch (const ResolveError& error)
			{
				qWarning () << Q_FUNC_INFO
						<< "error resolving media info for"
						<< error.GetPath ()
						<< error.what ();
				return MediaInfo ();
			}
		};
		QFuture<MediaInfo> future = QtConcurrent::mapped (paths,
				std::function<MediaInfo (const QString&)> (worker));
		Watcher_->setFuture (future);
	}

	int LocalCollection::FindAlbum (const QString& artist, const QString& album) const
	{
		auto artistPos = std::find_if (Artists_.begin (), Artists_.end (),
				[&artist] (decltype (Artists_.front ()) item) { return item.Name_ == artist; });
		if (artistPos == Artists_.end ())
			return -1;

		auto albumPos = std::find_if (artistPos->Albums_.begin (), artistPos->Albums_.end (),
				[&album] (decltype (artistPos->Albums_.front ()) item) { return item->Name_ == album; });
		if (albumPos == artistPos->Albums_.end ())
			return -1;

		return (*albumPos)->ID_;
	}

	void LocalCollection::SetAlbumArt (int id, const QString& path)
	{
		if (Album2Item_.contains (id))
			Album2Item_ [id]->setData (path, Role::AlbumArt);

		if (AlbumID2Album_.contains (id))
			AlbumID2Album_ [id]->CoverPath_ = path;

		Storage_->SetAlbumArt (id, path);
	}

	int LocalCollection::FindTrack (const QString& path) const
	{
		return Path2Track_.value (path, -1);
	}

	Collection::Album_ptr LocalCollection::GetTrackAlbum (int trackId) const
	{
		return AlbumID2Album_ [Track2Album_ [trackId]];
	}

	QList<int> LocalCollection::GetDynamicPlaylist (DynamicPlaylist type) const
	{
		QList<int> result;
		const auto& keys = Track2Path_.keys ();
		switch (type)
		{
		case DynamicPlaylist::Random50:
			for (int i = 0; i < 50; ++i)
				result << keys [qrand () % keys.size ()];
			break;
		}
		return result;
	}

	QStringList LocalCollection::TrackList2PathList (const QList<int>& tracks) const
	{
		QStringList result;
		std::transform (tracks.begin (), tracks.end (), std::back_inserter (result),
				[this] (int id) { return Track2Path_ [id]; });
		result.removeAll (QString ());
		return result;
	}

	Collection::TrackStats LocalCollection::GetTrackStats (const QString& path)
	{
		if (!Path2Track_.contains (path))
			return Collection::TrackStats ();

		try
		{
			return Storage_->GetTrackStats (Path2Track_ [path]);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching stats for track"
					<< path
					<< Path2Track_ [path]
					<< e.what ();
			return Collection::TrackStats ();
		}
	}

	QStringList LocalCollection::CollectPaths (const QModelIndex& index)
	{
		const auto type = index.data (Role::Node).toInt ();
		if (type == NodeType::Track)
			return QStringList (index.data (Role::TrackPath).toString ());

		QStringList paths;
		for (int i = 0; i < CollectionModel_->rowCount (index); ++i)
			paths += CollectPaths (CollectionModel_->index (i, 0, index));
		return paths;
	}

	namespace
	{
		template<typename T, typename U, typename Init, typename Parent>
		QStandardItem* GetItem (T& c, U idx, Init f, Parent parent)
		{
			if (c.contains (idx))
				return c [idx];

			auto item = new QStandardItem ();
			item->setEditable (false);
			f (item);
			parent->appendRow (item);
			c [idx] = item;
			return item;
		}
	}

	void LocalCollection::HandleNewArtists (const Collection::Artists_t& artists)
	{
		Q_FOREACH (const auto& artist, artists)
		{
			auto artistItem = GetItem (Artist2Item_,
					artist.ID_,
					[&artist, this] (QStandardItem *item)
					{
						item->setIcon (ArtistIcon_);
						item->setText (artist.Name_);
						item->setData (artist.Name_, LocalCollection::Role::ArtistName);
						item->setData (LocalCollection::NodeType::Artist, LocalCollection::Role::Node);
					},
					CollectionModel_);
			Q_FOREACH (auto album, artist.Albums_)
			{
				AlbumArtMgr_->CheckAlbumArt (artist, album);

				auto albumItem = GetItem (Album2Item_,
						album->ID_,
						[album, this] (QStandardItem *item)
						{
							item->setText (QString::fromUtf8 ("%1 — %2")
									.arg (album->Year_)
									.arg (album->Name_));
							item->setData (album->Year_, LocalCollection::Role::AlbumYear);
							item->setData (album->Name_, LocalCollection::Role::AlbumName);
							item->setData (LocalCollection::NodeType::Album, LocalCollection::Role::Node);
							if (!album->CoverPath_.isEmpty ())
								item->setData (album->CoverPath_, LocalCollection::Role::AlbumArt);
						},
						artistItem);

				AlbumID2Album_ [album->ID_] = album;

				Q_FOREACH (const auto& track, album->Tracks_)
				{
					const QString& name = QString::fromUtf8 ("%1 — %2")
							.arg (track.Number_)
							.arg (track.Name_);
					auto item = new QStandardItem (name);
					item->setEditable (false);
					item->setData (track.Number_, Role::TrackNumber);
					item->setData (track.Name_, Role::TrackTitle);
					item->setData (track.FilePath_, Role::TrackPath);
					item->setData (NodeType::Track, Role::Node);
					albumItem->appendRow (item);

					Path2Track_ [track.FilePath_] = track.ID_;
					Track2Path_ [track.ID_] = track.FilePath_;

					Track2Album_ [track.ID_] = album->ID_;
				}
			}
		}
	}

	void LocalCollection::recordPlayedTrack (const QString& path)
	{
		if (!Path2Track_.contains (path))
			return;

		try
		{
			Storage_->RecordTrackPlayed (Path2Track_ [path]);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error recording played info for track"
					<< e.what ();
		}
	}

	void LocalCollection::handleLoadFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<Collection::Artists_t>*> (sender ());
		watcher->deleteLater ();
		Artists_ = watcher->result ();

		Q_FOREACH (const auto& artist, Artists_)
			Q_FOREACH (auto album, artist.Albums_)
				Q_FOREACH (const auto& track, album->Tracks_)
					PresentPaths_ << track.FilePath_;

		HandleNewArtists (Artists_);

		IsReady_ = true;

		emit collectionReady ();
	}

	void LocalCollection::handleScanFinished ()
	{
		auto future = Watcher_->future ();
		QList<MediaInfo> infos;
		std::copy_if (future.begin (), future.end (), std::back_inserter (infos),
				[] (const MediaInfo& info) { return !info.LocalPath_.isEmpty (); });

		emit scanProgressChanged (infos.size ());

		auto newArts = Storage_->AddToCollection (infos);
		HandleNewArtists (newArts);
	}
}
}
