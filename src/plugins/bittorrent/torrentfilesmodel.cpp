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

#include "torrentfilesmodel.h"
#include <iterator>
#include <boost/functional/hash.hpp>
#include <QUrl>
#include <QtDebug>
#include <util/models/treeitem.h>
#include <util/util.h>
#include "core.h"

using namespace LeechCraft::Util;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			TorrentFilesModel::TorrentFilesModel (bool addDia, QObject *parent)
			: QAbstractItemModel (parent)
			, AdditionDialog_ (addDia)
			, FilesInTorrent_ (0)
			{
				QList<QVariant> rootData;
				if (AdditionDialog_)
					rootData << tr ("Name") << tr ("Size");
				else
					rootData << tr ("Name") << tr ("Priority") << tr ("Progress");
				RootItem_ = new TreeItem (rootData);
			}

			TorrentFilesModel::~TorrentFilesModel ()
			{
				delete RootItem_;
			}

			int TorrentFilesModel::columnCount (const QModelIndex& index) const
			{
				if (index.isValid ())
					return static_cast<TreeItem*> (index.internalPointer ())->ColumnCount ();
				else
					return RootItem_->ColumnCount ();
			}

			QVariant TorrentFilesModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();

				if (AdditionDialog_)
				{
					if (role == Qt::CheckStateRole &&
							index.column () == 0)
						return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column (), role);
					else if (role == Qt::DisplayRole)
						return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
					else
						return QVariant ();
				}
				else
					switch (role)
					{
						case Qt::DisplayRole:
							return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
						case RoleSize:
						case RoleProgress:
							{
								double result = static_cast<TreeItem*> (index.internalPointer ())->
									Data (0, role).toDouble ();
								if (result < 0)
									result = 0;
								return result;
							}
						default:
							return QVariant ();
					}
			}

			Qt::ItemFlags TorrentFilesModel::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;

				if (AdditionDialog_ &&
						index.column () == 0)
					return Qt::ItemIsSelectable |
						Qt::ItemIsEnabled |
						Qt::ItemIsUserCheckable;
				else if (!AdditionDialog_ &&
						((index.column () == 1 &&
						  !rowCount (index.sibling (index.row (), 0))) ||
						  index.column () == 0))
					return Qt::ItemIsSelectable |
						Qt::ItemIsEnabled |
						Qt::ItemIsEditable;
				else
					return Qt::ItemIsSelectable |
						Qt::ItemIsEnabled;
			}

			QVariant TorrentFilesModel::headerData (int h, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return RootItem_->Data (h);

				return QVariant ();
			}

			QModelIndex TorrentFilesModel::index (int row, int col, const QModelIndex& parent) const
			{
				if (!hasIndex (row, col, parent))
					return QModelIndex ();

				TreeItem *parentItem;

				if (!parent.isValid ())
					parentItem = RootItem_;
				else
					parentItem = static_cast<TreeItem*> (parent.internalPointer ());

				TreeItem *childItem = parentItem->Child (row);
				if (childItem)
					return createIndex (row, col, childItem);
				else
					return QModelIndex ();
			}

			QModelIndex TorrentFilesModel::parent (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return QModelIndex ();

				TreeItem *childItem = static_cast<TreeItem*> (index.internalPointer ()),
						 *parentItem = childItem->Parent ();

				if (parentItem == RootItem_)
					return QModelIndex ();

				return createIndex (parentItem->Row (), 0, parentItem);
			}

			int TorrentFilesModel::rowCount (const QModelIndex& parent) const
			{
				TreeItem *parentItem;
				if (parent.column () > 0)
					return 0;

				if (!parent.isValid ())
					parentItem = RootItem_;
				else
					parentItem = static_cast<TreeItem*> (parent.internalPointer ());

				return parentItem->ChildCount ();
			}

			bool TorrentFilesModel::setData (const QModelIndex& index, const QVariant& value, int role)
			{
				if (!index.isValid ())
					return false;

				if (role == Qt::CheckStateRole)
				{
					static_cast<TreeItem*> (index.internalPointer ())->
						ModifyData (0, value, Qt::CheckStateRole);
					emit dataChanged (index, index);

					int rows = rowCount (index);
					for (int i = 0; i < rows; ++i)
						setData (this->index (i, 0, index), value, role);

					QModelIndex pi = parent (index);
					while (pi.isValid ())
					{
						bool hasChecked = false;
						bool hasUnchecked = false;
						int prows = rowCount (pi);
						for (int i = 0; i < prows; ++i)
						{
							int state = this->index (i, 0, pi).data (role).toInt ();
							switch (static_cast<Qt::CheckState> (state))
							{
								case Qt::Checked:
									hasChecked = true;
									break;
								case Qt::Unchecked:
									hasUnchecked = true;
									break;
								default:
									hasChecked = true;
									hasUnchecked = true;
									break;
							}
							if (hasChecked && hasUnchecked)
								break;
						}
						Qt::CheckState state = Qt::Unchecked;
						if (hasChecked && hasUnchecked)
							state = Qt::PartiallyChecked;
						else if (hasChecked)
							state = Qt::Checked;
						else if (hasUnchecked)
							state = Qt::Unchecked;
						else
							qWarning () << Q_FUNC_INFO
								<< pi
								<< "we have neither checked nor unchecked items. Strange.";
						static_cast<TreeItem*> (pi.internalPointer ())->
							ModifyData (0, state, Qt::CheckStateRole);
						emit dataChanged (pi, pi);
						pi = parent (pi);
					}

					return true;
				}
				else if (role == Qt::EditRole)
				{
					if (index.column () == 1)
					{
						TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
						Core::Instance ()->
							SetFilePriority (item->Data (1, RolePath).toInt (), value.toInt ());
						item->ModifyData (index.column (), value);
						emit dataChanged (index, index);
						return true;
					}
					else if (index.column () == 0)
					{
						TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
						Core::Instance ()->
							SetFilename (item->Data (1, RolePath).toInt (),
									value.toString ());
						return true;
					}
					else
						return false;
				}
				else
					return false;
			}

			void TorrentFilesModel::Clear ()
			{
				if (!RootItem_->ChildCount ())
					return;

				BasePath_ = boost::filesystem::path ();

				beginRemoveRows (QModelIndex (), 0, RootItem_->ChildCount () - 1);
				while (RootItem_->ChildCount ())
					RootItem_->RemoveChild (0);
				endRemoveRows ();
				FilesInTorrent_ = 0;
				Path2TreeItem_.clear ();
			}

			void TorrentFilesModel::ResetFiles (libtorrent::torrent_info::file_iterator begin,
					const libtorrent::torrent_info::file_iterator& end)
			{
				Clear ();

				libtorrent::torrent_info::file_iterator orig = begin;
				int distance = std::distance (begin, end);
				if (!distance)
					return;

				beginInsertRows (QModelIndex (), 0, 0);

				FilesInTorrent_ = distance;
				Path2TreeItem_ [boost::filesystem::path ()] = RootItem_;

				for (int pos = 0; begin != end; ++begin, ++pos)
				{
					Path2TreeItem_t::key_type parentPath = begin->path.branch_path ();
					MkParentIfDoesntExist (begin->path);

					QList<QVariant> displayData;
					displayData << QString::fromUtf8 (begin->path.leaf ().c_str ())
						<< Util::MakePrettySize (begin->size);

					TreeItem *parentItem = Path2TreeItem_ [parentPath],
							 *item = new TreeItem (displayData, parentItem);
					item->ModifyData (2, static_cast<qulonglong> (begin->size), RawDataRole);
					item->ModifyData (1, static_cast<int> (std::distance (orig, begin)), RolePath);
					item->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
					parentItem->AppendChild (item);
					Path2TreeItem_ [begin->path] = item;
					Path2OriginalPosition_ [begin->path] = pos;
				}

				for (int i = 0; i < RootItem_->ChildCount (); ++i)
					UpdateSizeGraph (RootItem_->Child (i));

				endInsertRows ();
			}

			void TorrentFilesModel::ResetFiles (const boost::filesystem::path& basePath,
					const QList<FileInfo>& infos)
			{
				Clear ();

				BasePath_ = basePath;

				beginInsertRows (QModelIndex (), 0, 0);
				FilesInTorrent_ = infos.size ();
				Path2TreeItem_ [boost::filesystem::path ()] = RootItem_;

				for (int i = 0; i < infos.size (); ++i)
				{
					FileInfo fi = infos.at (i);
					Path2TreeItem_t::key_type parentPath = fi.Path_.branch_path ();
					MkParentIfDoesntExist (fi.Path_);

					QString pathStr = QString::fromUtf8 (fi.Path_.string ().c_str ());

					QList<QVariant> displayData;
					displayData << QString::fromUtf8 (fi.Path_.leaf ().c_str ())
						<< QString::number (fi.Priority_)
						<< QString::number (fi.Progress_, 'f', 3);

					TreeItem *parentItem = Path2TreeItem_ [parentPath],
							 *item = new TreeItem (displayData, parentItem);
					item->ModifyData (0, pathStr, RawDataRole);
					item->ModifyData (2, static_cast<qulonglong> (fi.Size_), RawDataRole);
					item->ModifyData (1, i, RolePath);
					item->ModifyData (0, static_cast<qulonglong> (fi.Size_), RoleSize);
					item->ModifyData (0, fi.Progress_, RoleProgress);
					parentItem->AppendChild (item);
					Path2TreeItem_ [fi.Path_] = item;
					Path2OriginalPosition_ [fi.Path_] = i;
				}

				for (int i = 0; i < RootItem_->ChildCount (); ++i)
					UpdateSizeGraph (RootItem_->Child (i));

				endInsertRows ();
			}

			void TorrentFilesModel::UpdateFiles (const boost::filesystem::path& basePath,
					const QList<FileInfo>& infos)
			{
				BasePath_ = basePath;
				if (Path2TreeItem_.empty () ||
						Path2TreeItem_.size () == 1)
				{
					ResetFiles (BasePath_, infos);
					return;
				}

				for (int i = 0; i < infos.size (); ++i)
				{
					FileInfo fi = infos.at (i);
					if (!Path2TreeItem_.count (fi.Path_))
					{
						Path2TreeItem_.clear ();
						ResetFiles (BasePath_, infos);
					}

					TreeItem *item = Path2TreeItem_ [fi.Path_];
					item->ModifyData (ColumnProgress, QString::number (fi.Progress_, 'f', 3));
					item->ModifyData (ColumnPath, static_cast<qulonglong> (fi.Size_), RoleSize);
					item->ModifyData (ColumnPath, fi.Progress_, RoleProgress);
				}

				for (int i = 0; i < RootItem_->ChildCount (); ++i)
					UpdateSizeGraph (RootItem_->Child (i));

				emit dataChanged (index (0, 2), index (RootItem_->ChildCount () - 1, 2));
			}

			QVector<bool> TorrentFilesModel::GetSelectedFiles () const
			{
				QVector<bool> result (FilesInTorrent_);
				for (Path2TreeItem_t::const_iterator i = Path2TreeItem_.begin (),
						end = Path2TreeItem_.end (); i != end; ++i)
					if (!i->second->ChildCount ())
					{
						result [Path2OriginalPosition_.at (i->first)] =
							(i->second->Data (0, Qt::CheckStateRole).toInt () == Qt::Checked);
					}
				return result;
			}

			void TorrentFilesModel::MarkAll ()
			{
				if (!RootItem_->ChildCount ())
					return;

				for (Path2TreeItem_t::const_iterator i = Path2TreeItem_.begin (),
						end = Path2TreeItem_.end (); i != end; ++i)
					if (!i->second->ChildCount ())
						i->second->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
				emit dataChanged (index (0, 0), index (RootItem_->ChildCount () - 1, 1));
			}

			void TorrentFilesModel::UnmarkAll ()
			{
				if (!RootItem_->ChildCount ())
					return;

				for (Path2TreeItem_t::const_iterator i = Path2TreeItem_.begin (),
						end = Path2TreeItem_.end (); i != end; ++i)
					if (!i->second->ChildCount ())
						i->second->ModifyData (0, Qt::Unchecked, Qt::CheckStateRole);
				emit dataChanged (index (0, 0), index (RootItem_->ChildCount () - 1, 1));
			}

			void TorrentFilesModel::MarkIndexes (const QList<QModelIndex>& indexes)
			{
				for (int i = 0; i < indexes.size (); ++i)
				{
					TreeItem *item = static_cast<TreeItem*> (indexes.at (i).internalPointer ());
					if (!item->ChildCount ())
						item->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
					emit dataChanged (index (indexes.at (i).row (), 0), index (indexes.at (i).row (), 1));
				}
			}

			void TorrentFilesModel::UnmarkIndexes (const QList<QModelIndex>& indexes)
			{
				for (int i = 0; i < indexes.size (); ++i)
				{
					TreeItem *item = static_cast<TreeItem*> (indexes.at (i).internalPointer ());
					if (!item->ChildCount ())
						item->ModifyData (0, Qt::Unchecked, Qt::CheckStateRole);
					emit dataChanged (index (indexes.at (i).row (), 0), index (indexes.at (i).row (), 1));
				}
			}

			void TorrentFilesModel::HandleFileActivated (QModelIndex index)
			{
				if (!index.isValid ())
					return;

				if (index.column ())
					index = index.sibling (index.row (), 0);

				TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
				for (Path2TreeItem_t::const_iterator i = Path2TreeItem_.begin (),
						end = Path2TreeItem_.end (); i != end; ++i)
				{
					if (i->second == item)
					{
						if (item->Data (0, RoleProgress).toDouble () != 1)
						{
							QString filename = QString::fromUtf8 (i->first.filename ().c_str ());
							emit gotEntity (Util::MakeNotification ("BitTorrent",
									tr ("The file %1 hasn't finished downloading yet.")
										.arg (filename),
									PWarning_));
						}
						else
						{
							boost::filesystem::path full = BasePath_ / i->first;
							QString path = QString::fromUtf8 (full.string ().c_str ());
							Entity e = Util::MakeEntity (QUrl::fromLocalFile (path),
									QString (),
									FromUserInitiated);
							emit gotEntity (e);
						}

						break;
					}
				}
			}

			void TorrentFilesModel::MkParentIfDoesntExist (const boost::filesystem::path& path)
			{
				Path2TreeItem_t::key_type parentPath = path.branch_path ();
				if (Path2TreeItem_.count (parentPath))
					return;

				MkParentIfDoesntExist (parentPath);
				TreeItem *parent = Path2TreeItem_ [parentPath.branch_path ()];

				QList<QVariant> data;
				data << QString::fromUtf8 (parentPath.leaf ().c_str ()) << QString ("");
				if (!AdditionDialog_)
					data << QString ("") << QString ("");
				TreeItem *item = new TreeItem (data, parent);
				if (AdditionDialog_)
					item->ModifyData (0, Qt::Checked, Qt::CheckStateRole);
                item->ModifyData (0, QString::fromUtf8 (parentPath.string ().c_str ()), RawDataRole);
				parent->AppendChild (item);
				Path2TreeItem_ [parentPath] = item;
			}

			void TorrentFilesModel::UpdateSizeGraph (TreeItem *item)
			{
				if (!item->ChildCount ())
					return;

				qulonglong size = 0;
                qulonglong done = 0;
				for (int i = 0; i < item->ChildCount (); ++i)
				{
					UpdateSizeGraph (item->Child (i));
                    qulonglong current = item->Child (i)->Data (2, RawDataRole).value<qulonglong> ();
					size += current;
                    done += static_cast<qulonglong> (item->Child (i)->Data (0, RoleProgress).toDouble () * current);
				}
				item->ModifyData (2, size, RawDataRole);
				item->ModifyData (2, Util::MakePrettySize (size));
                item->ModifyData (0, size, RoleSize);
                item->ModifyData (0, static_cast<double> (done) / size, RoleProgress);
			}
		};
	};
};

