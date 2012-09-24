/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "lmp.h"
#include <QIcon>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QUrl>
#include <QtDeclarative>
#include <QGraphicsEffect>
#include <phonon/mediaobject.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
#include "playertab.h"
#include "player.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "rootpathsettingsmanager.h"
#include "collectionstatsdialog.h"

namespace LeechCraft
{
namespace LMP
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("lmp");

		const auto& paths = QCoreApplication::libraryPaths ();
		if (std::find_if (paths.begin (), paths.end (),
				[] (const QString& path) { return path.contains ("kde4"); }) == paths.end ())
		{
			QCoreApplication::addLibraryPath ("/usr/lib/kde4/plugins");
			QCoreApplication::addLibraryPath ("/usr/lib64/kde4/plugins");
		}

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpsettings.xml");

		qmlRegisterType<QGraphicsBlurEffect> ("Effects", 1, 0, "Blur");
		qmlRegisterType<QGraphicsColorizeEffect> ("Effects", 1, 0, "Colorize");
		qmlRegisterType<QGraphicsDropShadowEffect> ("Effects", 1, 0, "DropShadow");
		qmlRegisterType<QGraphicsOpacityEffect> ("Effects", 1, 0, "OpacityEffect");

		PlayerTC_ =
		{
			GetUniqueID () + "_player",
			"LMP",
			GetInfo (),
			GetIcon (),
			40,
			TFSingle | TFByDefault | TFOpenableByRequest
		};

		Core::Instance ().SetProxy (proxy);
		Core::Instance ().PostInit ();

		auto mgr = new RootPathSettingsManager (this);
		XSD_->SetDataSource ("RootPathsView", mgr->GetModel ());

		PlayerTab_ = new PlayerTab (PlayerTC_, this);

		connect (PlayerTab_,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (PlayerTab_,
				SIGNAL (changeTabName (QWidget*, QString)),
				this,
				SIGNAL (changeTabName (QWidget*, QString)));
		connect (PlayerTab_,
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));
		connect (PlayerTab_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		connect (PlayerTab_,
				SIGNAL (fullRaiseRequested ()),
				this,
				SLOT (handleFullRaiseRequested ()));

		ActionRescan_ = new QAction (tr ("Rescan collection"), this);
		ActionRescan_->setProperty ("ActionIcon", "view-refresh");
		connect (ActionRescan_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (rescan ()));

		ActionCollectionStats_ = new QAction (tr ("Collection statistics"), this);
		ActionCollectionStats_->setProperty ("ActionIcon", "view-statistics");
		connect (ActionCollectionStats_,
				SIGNAL (triggered ()),
				this,
				SLOT (showCollectionStats ()));

		Entity e = Util::MakeEntity (QVariant (), QString (), 0,
				"x-leechcraft/global-action-register");
		e.Additional_ ["Receiver"] = QVariant::fromValue<QObject*> (PlayerTab_->GetPlayer ());
		auto initShortcut = [&e, this] (const QByteArray& method, const QKeySequence& seq)
		{
			Entity thisE = e;
			thisE.Additional_ ["ActionID"] = "LMP_Global_" + method;
			thisE.Additional_ ["Method"] = method;
			thisE.Additional_ ["Shortcut"] = QVariant::fromValue (seq);
			GlobAction2Entity_ ["LMP_Global_" + method] = thisE;
		};
		initShortcut (SLOT (togglePause ()), QString ("Meta+C"));
		initShortcut (SLOT (previousTrack ()), QString ("Meta+V"));
		initShortcut (SLOT (nextTrack ()), QString ("Meta+B"));
		initShortcut (SLOT (stop ()), QString ("Meta+X"));

		e.Additional_ ["Receiver"] = QVariant::fromValue<QObject*> (PlayerTab_);
		initShortcut (SLOT (handleLoveTrack ()), QString ("Meta+L"));

		auto setInfo = [this, proxy] (const QByteArray& method,
				const QString& userText, const QString& icon)
		{
			const auto& id = "LMP_Global_" + method;
			const auto& seq = GlobAction2Entity_ [id].Additional_ ["Shortcut"].value<QKeySequence> ();
			GlobAction2Info_ [id] = { userText, seq, proxy->GetIcon (icon) };
		};
		setInfo (SLOT (togglePause ()), tr ("Play/pause"), "media-playback-start");
		setInfo (SLOT (previousTrack ()), tr ("Previous track"), "media-skip-backward");
		setInfo (SLOT (nextTrack ()), tr ("Next track"), "media-skip-forward");
		setInfo (SLOT (stop ()), tr ("Stop playback"), "media-playback-stop");
		setInfo (SLOT (handleLoveTrack ()), tr ("Love track"), "emblem-favorite");
	}

	void Plugin::SecondInit ()
	{
		Q_FOREACH (const auto& e, GlobAction2Entity_.values ())
			emit gotEntity (e);

		PlayerTab_->InitWithOtherPlugins ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& sequences)
	{
		if (!GlobAction2Entity_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown id"
					<< id;
			return;
		}

		auto& e = GlobAction2Entity_ [id];
		e.Additional_ ["Shortcut"] = QVariant::fromValue (sequences.value (0));
		emit gotEntity (e);
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return GlobAction2Info_;
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "LMP";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LeechCraft Music Player.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/lmp/resources/images/lmp.svg");
		return icon;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t tcs;
		tcs << PlayerTC_;
		return tcs;
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == PlayerTC_.TabClass_)
		{
			emit addNewTab ("LMP", PlayerTab_);
			emit raiseTab (PlayerTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		QString path = e.Entity_.toString ();
		const QUrl& url = e.Entity_.toUrl ();
		if (path.isEmpty () &&
					url.isValid () &&
					url.scheme () == "file")
			path = url.toLocalFile ();

		if (!path.isEmpty ())
		{
			const auto& goodExt = XmlSettingsManager::Instance ()
					.property ("TestExtensions").toString ()
					.split (' ', QString::SkipEmptyParts);
			const QFileInfo fi = QFileInfo (path);
			if (fi.exists () && goodExt.contains (fi.suffix ()))
				return EntityTestHandleResult (EntityTestHandleResult::PHigh);
			else
				return EntityTestHandleResult ();
		}

		return EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		QString path = e.Entity_.toString ();
		const QUrl& url = e.Entity_.toUrl ();
		if (path.isEmpty () &&
					url.isValid () &&
					url.scheme () == "file")
			path = url.toLocalFile ();

		if (e.Parameters_ & Internal)
		{
			auto obj = Phonon::createPlayer (Phonon::NotificationCategory, path);
			obj->play ();
			connect (obj,
					SIGNAL (finished ()),
					obj,
					SLOT (deleteLater ()));
		}
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		const auto& name = GetName ();

		QMap<QString, QList<QAction*>> result;
		result [name] << ActionRescan_;
		result [name] << ActionCollectionStats_;
		return result;
	}

	void Plugin::RecoverTabs (const QList<LeechCraft::TabRecoverInfo>& infos)
	{
		Q_FOREACH (const auto& recInfo, infos)
		{
			qDebug () << Q_FUNC_INFO << recInfo.Data_;

			if (recInfo.Data_ == "playertab")
			{
				Q_FOREACH (const auto& pair, recInfo.DynProperties_)
					PlayerTab_->setProperty (pair.first, pair.second);

				TabOpenRequested (PlayerTC_.TabClass_);
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown context"
						<< recInfo.Data_;
		}
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.General";
		result << "org.LeechCraft.LMP.CollectionSync";
		result << "org.LeechCraft.LMP.CloudStorage";
		result << "org.LeechCraft.LMP.PlaylistProvider";
		return result;
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	void Plugin::handleFullRaiseRequested ()
	{
		TabOpenRequested (PlayerTC_.TabClass_);
	}

	void Plugin::showCollectionStats ()
	{
		auto dia = new CollectionStatsDialog ();
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp, LeechCraft::LMP::Plugin);
