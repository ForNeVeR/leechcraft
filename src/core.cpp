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

#include <limits>
#include <stdexcept>
#include <list>
#include <functional>
#include <QMainWindow>
#include <QMessageBox>
#include <QtDebug>
#include <QTimer>
#include <QNetworkProxy>
#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QKeyEvent>
#include <QDir>
#include <QTextCodec>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QAbstractNetworkCache>
#include <QClipboard>
#include <util/util.h>
#include <util/customcookiejar.h>
#include <util/defaulthookproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/iwindow.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/isummaryrepresentation.h>
#include <interfaces/structures.h>
#include <interfaces/entitytesthandleresult.h>
#include "application.h"
#include "mainwindow.h"
#include "pluginmanager.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "sqlstoragebackend.h"
#include "handlerchoicedialog.h"
#include "tagsmanager.h"
#include "fancypopupmanager.h"
#include "application.h"
#include "newtabmenumanager.h"
#include "networkaccessmanager.h"
#include "tabmanager.h"
#include "directorywatcher.h"
#include "clipboardwatcher.h"
#include "localsockethandler.h"
#include "storagebackend.h"
#include "coreinstanceobject.h"
#include "coreplugin2manager.h"

using namespace LeechCraft::Util;

namespace LeechCraft
{
	Core::Core ()
	: NetworkAccessManager_ (new NetworkAccessManager)
	, StorageBackend_ (new SQLStorageBackend)
	, DirectoryWatcher_ (new DirectoryWatcher)
	, ClipboardWatcher_ (new ClipboardWatcher)
	, LocalSocketHandler_ (new LocalSocketHandler)
	, NewTabMenuManager_ (new NewTabMenuManager)
	, CoreInstanceObject_ (new CoreInstanceObject)
	, IsShuttingDown_ (false)
	{
		CoreInstanceObject_->GetCorePluginManager ()->RegisterHookable (NetworkAccessManager_.get ());

		connect (CoreInstanceObject_->GetSettingsDialog ().get (),
				SIGNAL (pushButtonClicked (const QString&)),
				this,
				SLOT (handleSettingClicked (const QString&)));

		connect (LocalSocketHandler_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SLOT (queueEntity (const LeechCraft::Entity&)));
		connect (NetworkAccessManager_.get (),
				SIGNAL (error (const QString&)),
				this,
				SIGNAL (error (const QString&)));

		connect (DirectoryWatcher_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SLOT (handleGotEntity (LeechCraft::Entity)));
		connect (ClipboardWatcher_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SLOT (handleGotEntity (LeechCraft::Entity)));

		StorageBackend_->Prepare ();

		QStringList paths;
		boost::program_options::variables_map map = qobject_cast<Application*> (qApp)->GetVarMap ();
		if (map.count ("plugin"))
		{
			const std::vector<std::string>& plugins = map ["plugin"].as<std::vector<std::string> > ();
			Q_FOREACH (const std::string& plugin, plugins)
				paths << QDir (QString::fromUtf8 (plugin.c_str ())).absolutePath ();
		}
		PluginManager_ = new PluginManager (paths, this);

		QList<QByteArray> proxyProperties;
		proxyProperties << "ProxyEnabled"
			<< "ProxyHost"
			<< "ProxyPort"
			<< "ProxyLogin"
			<< "ProxyPassword"
			<< "ProxyType";
		XmlSettingsManager::Instance ()->RegisterObject (proxyProperties,
				this, "handleProxySettings");

		handleProxySettings ();
	}

	Core::~Core ()
	{
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Release ()
	{
		IsShuttingDown_ = true;
		LocalSocketHandler_.reset ();
		XmlSettingsManager::Instance ()->setProperty ("FirstStart", "false");
		ClipboardWatcher_.reset ();
		DirectoryWatcher_.reset ();

		PluginManager_->Release ();
		delete PluginManager_;

		CoreInstanceObject_.reset ();

		NetworkAccessManager_.reset ();

		StorageBackend_.reset ();
	}

	bool Core::IsShuttingDown () const
	{
		return IsShuttingDown_;
	}

	void Core::SetReallyMainWindow (MainWindow *win)
	{
		ReallyMainWindow_ = win;
		ReallyMainWindow_->GetTabWidget ()->installEventFilter (this);
		ReallyMainWindow_->installEventFilter (this);

		LocalSocketHandler_->SetMainWindow (win);
	}

	MainWindow* Core::GetReallyMainWindow ()
	{
		return ReallyMainWindow_;
	}

	IShortcutProxy* Core::GetShortcutProxy () const
	{
		return ReallyMainWindow_->GetShortcutProxy ();
	}

	QObjectList Core::GetSettables () const
	{
		return PluginManager_->GetAllCastableRoots<IHaveSettings*> ();
	}

	QObjectList Core::GetShortcuts () const
	{
		return PluginManager_->GetAllCastableRoots<IHaveShortcuts*> ();
	}

	QList<QList<QAction*> > Core::GetActions2Embed () const
	{
		const QList<IActionsExporter*>& plugins =
				PluginManager_->GetAllCastableTo<IActionsExporter*> ();
		QList<QList<QAction*> > actions;
		Q_FOREACH (const IActionsExporter *plugin, plugins)
		{
			const QList<QAction*>& list = plugin->GetActions (AEPCommonContextMenu);
			if (!list.size ())
				continue;
			actions << list;
		}
		return actions;
	}

	QAbstractItemModel* Core::GetPluginsModel () const
	{
		return PluginManager_;
	}

	PluginManager* Core::GetPluginManager () const
	{
		return PluginManager_;
	}

	StorageBackend* Core::GetStorageBackend () const
	{
		return StorageBackend_.get ();
	}

	CoreInstanceObject* Core::GetCoreInstanceObject () const
	{
		return CoreInstanceObject_.get ();
	}

	QToolBar* Core::GetToolBar (int index) const
	{
		return TabManager_->GetToolBar (index);
	}

	void Core::Setup (QObject *plugin)
	{
		const IJobHolder *ijh = qobject_cast<IJobHolder*> (plugin);

		InitDynamicSignals (plugin);

		if (ijh)
			InitJobHolder (plugin);

		if (qobject_cast<IHaveTabs*> (plugin))
			InitMultiTab (plugin);
	}

	void Core::PostSecondInit (QObject *plugin)
	{
		if (qobject_cast<IHaveTabs*> (plugin))
			GetNewTabMenuManager ()->AddObject (plugin);
	}

	void Core::DelayedInit ()
	{
		connect (this,
				SIGNAL (error (QString)),
				ReallyMainWindow_,
				SLOT (catchError (QString)));

		TabManager_.reset (new TabManager (ReallyMainWindow_->GetTabWidget (),
					ReallyMainWindow_->GetTabWidget ()));

		PluginManager_->Init ();

		NewTabMenuManager_->SetToolbarActions (GetActions2Embed ());

		disconnect (LocalSocketHandler_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SLOT (queueEntity (const LeechCraft::Entity&)));

		connect (LocalSocketHandler_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SLOT (handleGotEntity (const LeechCraft::Entity&)));

		QTimer::singleShot (1000,
				LocalSocketHandler_.get (),
				SLOT (pullCommandLine ()));

		QTimer::singleShot (2000,
				this,
				SLOT (pullEntityQueue ()));

		QTimer::singleShot (10000,
				this,
				SLOT (handlePluginLoadErrors ()));
	}

	void Core::TryToAddJob (QString name)
	{
		Entity e;
		if (QFile::exists (name))
			e.Entity_ = QUrl::fromLocalFile (name);
		else
		{
			const QUrl url (name);
			e.Entity_ = url.isValid () ? url : name;
		}
		e.Parameters_ = FromUserInitiated;

		if (!handleGotEntity (e))
			emit error (tr ("No plugins are able to download \"%1\"").arg (name));
	}

	QPair<qint64, qint64> Core::GetSpeeds () const
	{
		qint64 download = 0;
		qint64 upload = 0;

		Q_FOREACH (QObject *plugin, PluginManager_->GetAllPlugins ())
		{
			IDownload *di = qobject_cast<IDownload*> (plugin);
			if (di)
			{
				try
				{
					download += di->GetDownloadSpeed ();
					upload += di->GetUploadSpeed ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< "unable to get speeds"
						<< e.what ()
						<< plugin;
				}
				catch (...)
				{
					qWarning () << Q_FUNC_INFO
						<< "unable to get speeds"
						<< plugin;
				}
			}
		}

		return QPair<qint64, qint64> (download, upload);
	}

	QNetworkAccessManager* Core::GetNetworkAccessManager () const
	{
		return NetworkAccessManager_.get ();
	}

	QModelIndex Core::MapToSource (const QModelIndex& index) const
	{
		const QList<ISummaryRepresentation*>& summaries =
			PluginManager_->GetAllCastableTo<ISummaryRepresentation*> ();
		Q_FOREACH (const ISummaryRepresentation *summary, summaries)
		{
			const QModelIndex& mapped = summary->MapToSource (index);
			if (mapped.isValid ())
				return mapped;
		}
		return QModelIndex ();
	}

	TabManager* Core::GetTabManager () const
	{
		return TabManager_.get ();
	}

	NewTabMenuManager* Core::GetNewTabMenuManager () const
	{
		return NewTabMenuManager_.get ();
	}

	bool Core::eventFilter (QObject *watched, QEvent *e)
	{
		if (ReallyMainWindow_ &&
				watched == ReallyMainWindow_)
		{
			if (e->type () == QEvent::DragEnter)
			{
				QDragEnterEvent *event = static_cast<QDragEnterEvent*> (e);

				Q_FOREACH (const QString& format, event->mimeData ()->formats ())
				{
					const Entity& e = Util::MakeEntity (event->
								mimeData ()->data (format),
							QString (),
							FromUserInitiated,
							format);

					if (CouldHandle (e))
					{
						event->acceptProposedAction ();
						break;
					}
				}

				return true;
			}
			else if (e->type () == QEvent::Drop)
			{
				QDropEvent *event = static_cast<QDropEvent*> (e);

				Q_FOREACH (const QString& format, event->mimeData ()->formats ())
				{
					const Entity& e = Util::MakeEntity (event->
								mimeData ()->data (format),
							QString (),
							FromUserInitiated,
							format);

					if (handleGotEntity (e))
					{
						event->acceptProposedAction ();
						break;
					}
				}

				return true;
			}
		}
		return QObject::eventFilter (watched, e);
	}

	void Core::handleProxySettings () const
	{
		const bool enabled = XmlSettingsManager::Instance ()->property ("ProxyEnabled").toBool ();
		QNetworkProxy pr;
		if (enabled)
		{
			pr.setHostName (XmlSettingsManager::Instance ()->property ("ProxyHost").toString ());
			pr.setPort (XmlSettingsManager::Instance ()->property ("ProxyPort").toInt ());
			pr.setUser (XmlSettingsManager::Instance ()->property ("ProxyLogin").toString ());
			pr.setPassword (XmlSettingsManager::Instance ()->property ("ProxyPassword").toString ());
			const QString& type = XmlSettingsManager::Instance ()->property ("ProxyType").toString ();
			QNetworkProxy::ProxyType pt = QNetworkProxy::HttpProxy;
			if (type == "socks5")
				pt = QNetworkProxy::Socks5Proxy;
			else if (type == "tphttp")
				pt = QNetworkProxy::HttpProxy;
			else if (type == "chttp")
				pr = QNetworkProxy::HttpCachingProxy;
			else if (type == "cftp")
				pr = QNetworkProxy::FtpCachingProxy;
			pr.setType (pt);
		}
		else
			pr.setType (QNetworkProxy::NoProxy);
		QNetworkProxy::setApplicationProxy (pr);
		NetworkAccessManager_->setProxy (pr);
	}

	void Core::handleSettingClicked (const QString& name)
	{
		if (name == "ClearCache")
		{
			if (QMessageBox::question (ReallyMainWindow_,
						"LeechCraft",
						tr ("Do you really want to clear the network cache?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return;

			QAbstractNetworkCache *cache = NetworkAccessManager_->cache ();
			if (cache)
				cache->clear ();
		}
		else if (name == "ClearCookies")
		{
			if (QMessageBox::question (ReallyMainWindow_,
						"LeechCraft",
						tr ("Do you really want to clear cookies?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return;

			CustomCookieJar *jar = static_cast<CustomCookieJar*> (NetworkAccessManager_->cookieJar ());
			jar->setAllCookies (QList<QNetworkCookie> ());
			jar->Save ();
		}
	}

	bool Core::CouldHandle (Entity e) const
	{
		if (!(e.Parameters_ & OnlyHandle))
			if (GetObjects (e, OTDownloaders, true).size ())
				return true;

		if (!(e.Parameters_ & OnlyDownload))
			if (GetObjects (e, OTHandlers, true).size ())
				return true;

		return false;
	}

	namespace
	{
		bool DoDownload (IDownload *sd,
				Entity p,
				int *id,
				QObject **pr)
		{
			int l = -1;
			try
			{
				l = sd->AddJob (p);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not add job"
					<< e.what ();
				return false;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not add job";
				return false;
			}

			if (id)
				*id = l;
			if (pr)
			{
				const QObjectList& plugins = Core::Instance ().GetPluginManager ()->
					GetAllCastableRoots<IDownload*> ();
				*pr = *std::find_if (plugins.begin (), plugins.end (),
						[sd] (QObject *d) { return qobject_cast<IDownload*> (d) == sd; });
			}

			return true;
		}

		bool DoHandle (IEntityHandler *sh,
				Entity p)
		{
			try
			{
				sh->Handle (p);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not handle job"
					<< e.what ();
				return false;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not add job";
				return false;
			}
			return true;
		}
	};

	QList<QObject*> Core::GetObjects (const Entity& p,
			Core::ObjectType type, bool detectOnly) const
	{
		QObjectList plugins;
		switch (type)
		{
			case OTDownloaders:
				plugins = PluginManager_->GetAllCastableRoots<IDownload*> ();
				break;
			case OTHandlers:
				plugins = PluginManager_->GetAllCastableRoots<IEntityHandler*> ();
				break;
		}

		QObjectList result;
		for (QObjectList::const_iterator it = plugins.begin (), end = plugins.end (); it != end; ++it)
		{
			if (*it == sender () &&
					!(p.Parameters_ & ShouldQuerySource))
				continue;

			try
			{
				EntityTestHandleResult r;
				switch (type)
				{
					case OTDownloaders:
						{
							IDownload *id = qobject_cast<IDownload*> (*it);
							r = id->CouldDownload (p);
						}
						break;
					case OTHandlers:
						{
							IEntityHandler *ih = qobject_cast<IEntityHandler*> (*it);
							r = ih->CouldHandle (p);
						}
						break;
				}

				if (r.HandlePriority_ <= 0)
					continue;

				const bool single = type == OTHandlers && r.CancelOthers_;
				if (single)
					result.clear ();

				result << *it;

				if (single)
					break;

				if (detectOnly && result.size ())
					break;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not query"
					<< e.what ()
					<< *it;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not query"
					<< *it;
			}
		}

		return result;
	}

	bool Core::handleGotEntity (Entity p, int *id, QObject **pr)
	{
		const QString& string = Util::GetUserText (p);

		std::auto_ptr<HandlerChoiceDialog> dia (new HandlerChoiceDialog (string, ReallyMainWindow_));

		int numDownloaders = 0;
		if (!(p.Parameters_ & OnlyHandle))
			Q_FOREACH (QObject *plugin, GetObjects (p, OTDownloaders, false))
				numDownloaders +=
					dia->Add (qobject_cast<IInfo*> (plugin), qobject_cast<IDownload*> (plugin));

		int numHandlers = 0;
		// Handlers don't fit when we want to delegate.
		if (!id && !(p.Parameters_ & OnlyDownload))
			Q_FOREACH (QObject *plugin, GetObjects (p, OTHandlers, false))
				numHandlers +=
					dia->Add (qobject_cast<IInfo*> (plugin), qobject_cast<IEntityHandler*> (plugin));

		if (!(numHandlers + numDownloaders))
		{
			if (p.Entity_.toUrl ().isValid () &&
					(p.Parameters_ & FromUserInitiated) &&
					!(p.Parameters_ & OnlyDownload))
			{
				QDesktopServices::openUrl (p.Entity_.toUrl ());
				return true;
			}
			else
				return false;
		}

		const bool bcastCandidate = !id && !pr && numHandlers;

		if (p.Parameters_ & FromUserInitiated &&
				!(p.Parameters_ & AutoAccept))
		{
			bool ask = true;
			if (XmlSettingsManager::Instance ()->
					property ("DontAskWhenSingle").toBool ())
				ask = (numDownloaders || numHandlers != 1);

			IDownload *sd = 0;
			IEntityHandler *sh = 0;
			if (ask)
			{
				dia->SetFilenameSuggestion (p.Location_);
				if (dia->exec () == QDialog::Rejected)
					return false;
				sd = dia->GetDownload ();
				sh = dia->GetEntityHandler ();
			}
			else
			{
				sd = dia->GetFirstDownload ();
				sh = dia->GetFirstEntityHandler ();
			}

			if (sd)
			{
				const QString& dir = dia->GetFilename ();
				if (dir.isEmpty ())
					return false;

				p.Location_ = dir;

				if (!DoDownload (sd, p, id, pr))
				{
					if (dia->NumChoices () > 1 &&
							QMessageBox::question (ReallyMainWindow_,
							tr ("Error"),
							tr ("Could not add task to the selected downloader, "
								"would you like to try another one?"),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
						return handleGotEntity (p, id, pr);
					else
						return false;
				}
				else
					return true;
			}
			if (sh)
			{
				if (!DoHandle (sh, p))
				{
					if (dia->NumChoices () > 1 &&
							QMessageBox::question (ReallyMainWindow_,
							tr ("Error"),
							tr ("Could not handle task with the selected handler, "
								"would you like to try another one?"),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
						return handleGotEntity (p, id, pr);
					else
						return false;
				}
				else
					return true;
			}
		}
		else if (bcastCandidate)
		{
			bool success = false;
			Q_FOREACH (IEntityHandler *ieh, dia->GetAllEntityHandlers ())
				success = DoHandle (ieh, p) || success;
			return success;
		}
		else if (dia->GetDownload ())
		{
			IDownload *sd = dia->GetDownload ();
			if (p.Location_.isEmpty ())
				p.Location_ = QDir::tempPath ();
			return DoDownload (sd, p, id, pr);
		}
		else if (((p.Parameters_ & AutoAccept) ||
					(numHandlers == 1 &&
					XmlSettingsManager::Instance ()->
						property ("DontAskWhenSingle").toBool ())) &&
				dia->GetFirstEntityHandler ())
			return DoHandle (dia->GetFirstEntityHandler (), p);
		else if (p.Mime_ == "x-leechcraft/notification")
		{
			HandleNotify (p);
			return true;
		}
		else
		{
			emit log (tr ("Could not handle download entity %1.")
					.arg (string));
			return false;
		}

		return true;
	}

	void Core::handleCouldHandle (const Entity& e, bool *could)
	{
		*could = CouldHandle (e);
	}

	void Core::queueEntity (Entity e)
	{
		QueuedEntities_ << e;
	}

	void Core::pullEntityQueue ()
	{
		Q_FOREACH (const Entity& e, QueuedEntities_)
			handleGotEntity (e);
		QueuedEntities_.clear ();
	}

	void Core::handlePluginLoadErrors ()
	{
		Q_FOREACH (const QString& error, PluginManager_->GetPluginLoadErrors ())
			handleGotEntity (Util::MakeNotification (tr ("Plugin load error"),
					error, PCritical_));
	}

	void Core::handleStatusBarChanged (QWidget *contents, const QString& origMessage)
	{
		if (contents->visibleRegion ().isEmpty ())
			return;

		ReallyMainWindow_->statusBar ()->showMessage (origMessage, 30000);
	}

	void Core::HandleNotify (const Entity& entity)
	{
		const bool show = XmlSettingsManager::Instance ()->
			property ("ShowFinishedDownloadMessages").toBool ();

		QString pname;
		IInfo *ii = qobject_cast<IInfo*> (sender ());
		if (ii)
		{
			try
			{
				pname = ii->GetName ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< e.what ()
					<< sender ();
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< sender ();
			}
		}

		const QString& nheader = entity.Entity_.toString ();
		const QString& ntext = entity.Additional_ ["Text"].toString ();
		const int priority = entity.Additional_ ["Priority"].toInt ();

		QString header;

		const QString str ("%1: %2");

		if (pname.isEmpty () || nheader.isEmpty ())
			header = pname + nheader;
		else
			header = str.arg (pname).arg (nheader);

		const QString& text = str.arg (header).arg (ntext);

		emit log (text);

		if (priority != PLog_ &&
				show)
			ReallyMainWindow_->GetFancyPopupManager ()->ShowMessage (entity);
	}

	void Core::InitDynamicSignals (QObject *plugin)
	{
		const QMetaObject *qmo = plugin->metaObject ();

		if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
						"couldHandle (const LeechCraft::Entity&, bool*)"
						).constData ()) != -1)
			connect (plugin,
					SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
					this,
					SLOT (handleCouldHandle (const LeechCraft::Entity&, bool*)));

		if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
						"gotEntity (const LeechCraft::Entity&)"
						).constData ()) != -1)
			connect (plugin,
					SIGNAL (gotEntity (const LeechCraft::Entity&)),
					this,
					SLOT (handleGotEntity (LeechCraft::Entity)),
					Qt::QueuedConnection);

		if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
						"delegateEntity (const LeechCraft::Entity&, int*, QObject**)"
						).constData ()) != -1)
			connect (plugin,
					SIGNAL (delegateEntity (const LeechCraft::Entity&,
							int*, QObject**)),
					this,
					SLOT (handleGotEntity (LeechCraft::Entity,
							int*, QObject**)));
	}

	void Core::InitJobHolder (QObject *plugin)
	{
	}

	void Core::InitEmbedTab (QObject *plugin)
	{
		InitCommonTab (plugin);
	}

	void Core::InitMultiTab (QObject *plugin)
	{
		connect (plugin,
				SIGNAL (addNewTab (const QString&, QWidget*)),
				TabManager_.get (),
				SLOT (add (const QString&, QWidget*)));
		connect (plugin,
				SIGNAL (removeTab (QWidget*)),
				TabManager_.get (),
				SLOT (remove (QWidget*)));

		InitCommonTab (plugin);
	}

	void Core::InitCommonTab (QObject *plugin)
	{
		connect (plugin,
				SIGNAL (changeTabName (QWidget*, const QString&)),
				TabManager_.get (),
				SLOT (changeTabName (QWidget*, const QString&)),
				Qt::UniqueConnection);
		connect (plugin,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				TabManager_.get (),
				SLOT (changeTabIcon (QWidget*, const QIcon&)),
				Qt::UniqueConnection);
		connect (plugin,
				SIGNAL (changeTooltip (QWidget*, QWidget*)),
				TabManager_.get (),
				SLOT (changeTooltip (QWidget*, QWidget*)),
				Qt::UniqueConnection);
		connect (plugin,
				SIGNAL (statusBarChanged (QWidget*, const QString&)),
				this,
				SLOT (handleStatusBarChanged (QWidget*, const QString&)),
				Qt::UniqueConnection);
		connect (plugin,
				SIGNAL (raiseTab (QWidget*)),
				TabManager_.get (),
				SLOT (bringToFront (QWidget*)),
				Qt::UniqueConnection);
	}
}
