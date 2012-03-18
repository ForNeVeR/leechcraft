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

#include "findproxy.h"
#include <QTextCodec>
#include <QToolBar>
#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <interfaces/structures.h>
#include <util/util.h>
#include "categoriesselector.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			FindProxy::FindProxy (const Request& r, CategoriesSelector *cs, FindProxyType type)
			: Toolbar_ (new QToolBar)
			, R_ (r)
			, FindProxyType_ (type)
			, CategoriesSelector_ (cs)
			{
				ActionDownload_ = Toolbar_->addAction (tr ("Download"));
				ActionDownload_->setProperty ("ActionIcon", "download");
				connect (ActionDownload_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleDownload ()));

				ActionHandle_ =  Toolbar_->addAction (tr ("Handle"));
				ActionHandle_->setProperty ("ActionIcon", "media-playback-start");
				connect (ActionHandle_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleHandle ()));

				ActionCopyToClipboard_ = new QAction (tr ("Copy URL to clipboard"),
						this);
				ActionCopyToClipboard_->setProperty ("ActionIcon", "edit-copy");
				connect (ActionCopyToClipboard_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleCopyToClipboard ()));

				ContextMenu_ = new QMenu (tr ("vGrabber menu"));
				ContextMenu_->addAction (ActionDownload_);
				ContextMenu_->addAction (ActionHandle_);
				ContextMenu_->addSeparator ();
				ContextMenu_->addAction (ActionCopyToClipboard_);
			}

			FindProxy::~FindProxy ()
			{
				delete Toolbar_;
				delete ContextMenu_;
			}

			void FindProxy::Start ()
			{
				if (R_.String_.isEmpty ())
				{
					SetError (tr ("Empty search string"));
					return;
				}

				QUrl url = GetURL ();

				QString fname = Util::GetTemporaryName ();
				Entity e =
					Util::MakeEntity (url,
						fname,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);

				int id = -1;
				QObject *pr = 0;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit error (tr ("Job for request<br />%1<br />wasn't delegated.")
							.arg (url.toString ()));
					return;
				}

				Jobs_ [id] = fname;
				HandleProvider (pr);
			}

			QAbstractItemModel* FindProxy::GetModel ()
			{
				return this;
			}

			QByteArray FindProxy::GetUniqueSearchID () const
			{
				return QString ("org.LeechCraft.vGrabber.%1")
						.arg (GetURL ().toString ())
						.toUtf8 ();
			}

			QStringList FindProxy::GetCategories () const
			{
				return CategoriesSelector_->GetHRCategories ();
			}

			int FindProxy::columnCount (const QModelIndex&) const
			{
				return 3;
			}

			Qt::ItemFlags FindProxy::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;
				else
					return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}

			QVariant FindProxy::headerData (int, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return QString ("");
				else
					return QVariant ();
			}

			QModelIndex FindProxy::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();

				return createIndex (row, column);
			}

			QModelIndex FindProxy::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}

			void FindProxy::handleCopyToClipboard ()
			{
				QUrl url = qobject_cast<QAction*> (sender ())->data ().value<QUrl> ();
				QString urlStr = url.toString ();
				QApplication::clipboard ()->setText (urlStr, QClipboard::Clipboard);
				if (QApplication::clipboard ()->supportsSelection ())
					QApplication::clipboard ()->setText (urlStr, QClipboard::Selection);
			}

			void FindProxy::handleJobFinished (int id)
			{
				if (!Jobs_.contains (id))
					return;

				QString filename = Jobs_ [id];
				Jobs_.remove (id);

				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					emit error (tr ("Could not open file %1.")
							.arg (filename));
					return;
				}

				QString contents = QTextCodec::codecForName ("Windows-1251")->
					toUnicode (file.readAll ());

				Handle (contents);
			}

			void FindProxy::handleJobError (int id)
			{
				if (!Jobs_.contains (id))
					return;

				emit error (tr ("Search request for URL<br />%1<br />was delegated, but it failed.")
						.arg (GetURL ().toString ()));
				Jobs_.remove (id);
			}

			void FindProxy::SetError (const QString& error)
			{
				if (error.isEmpty () &&
						Error_)
				{
					beginRemoveRows (QModelIndex (), 0, 0);
					Error_ = boost::optional<QString> ();
					endRemoveRows ();
				}
				else
				{
					bool insert = !Error_;
					if (insert)
						beginInsertRows (QModelIndex (), 0, 0);
					Error_ = error;
					if (insert)
						endInsertRows ();
					else
						emit dataChanged (index (0, 0), index (0, columnCount () - 1));
				}
			}

			void FindProxy::EmitWith (LeechCraft::TaskParameter param, const QUrl& url)
			{
				if (!url.isValid ())
				{
					qWarning () << Q_FUNC_INFO
						<< "url is not valid"
						<< url;
				}

				Entity e = Util::MakeEntity (url,
						QString (),
						FromUserInitiated |
							param);
				emit gotEntity (e);
			}

			void FindProxy::HandleProvider (QObject *provider)
			{
				if (Downloaders_.contains (provider))
					return;

				Downloaders_ << provider;
				connect (provider,
						SIGNAL (jobFinished (int)),
						this,
						SLOT (handleJobFinished (int)));
				connect (provider,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handleJobError (int)));
			}
		};
	};
};

