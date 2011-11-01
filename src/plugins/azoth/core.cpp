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

#include "core.h"
#include <boost/bind.hpp>
#include <QIcon>
#include <QAction>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDir>
#include <QMenu>
#include <QMetaMethod>
#include <QInputDialog>
#include <QMainWindow>
#include <QStringListModel>
#include <QMessageBox>
#include <QClipboard>
#include <QtDebug>
#include <util/resourceloader.h>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include <util/categoryselector.h>
#include <util/notificationactionhandler.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/iprotocolplugin.h"
#include "interfaces/iprotocol.h"
#include "interfaces/iaccount.h"
#include "interfaces/iclentry.h"
#include "interfaces/iadvancedclentry.h"
#include "interfaces/imucentry.h"
#include "interfaces/imucperms.h"
#include "interfaces/iauthable.h"
#include "interfaces/iresourceplugin.h"
#include "interfaces/iurihandler.h"
#include "interfaces/irichtextmessage.h"
#include "interfaces/iextselfinfoaccount.h"
#ifdef ENABLE_CRYPT
#include "interfaces/isupportpgp.h"
#include "pgpkeyselectiondialog.h"
#endif
#include "chattabsmanager.h"
#include "pluginmanager.h"
#include "proxyobject.h"
#include "xmlsettingsmanager.h"
#include "joinconferencedialog.h"
#include "groupeditordialog.h"
#include "transferjobmanager.h"
#include "accounthandlerchooserdialog.h"
#include "util.h"
#include "eventsnotifier.h"
#include "drawattentiondialog.h"
#include "activitydialog.h"
#include "mooddialog.h"
#include "callmanager.h"
#include "addcontactdialog.h"
#include "acceptriexdialog.h"
#include "shareriexdialog.h"
#include "mucinvitedialog.h"
#include "clmodel.h"

namespace LeechCraft
{
namespace Azoth
{
	QDataStream& operator<< (QDataStream& out, const EntryStatus& status)
	{
		quint8 version = 1;
		out << version
			<< static_cast<quint8> (status.State_)
			<< status.StatusString_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, EntryStatus& status)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		quint8 state;
		in >> state
			>> status.StatusString_;
		status.State_ = static_cast<State> (state);
		return in;
	}

	namespace
	{
		QByteArray GetStyleOptName (QObject *entry)
		{
			if (XmlSettingsManager::Instance ().property ("CustomMUCStyle").toBool () &&
					qobject_cast<IMUCEntry*> (entry))
				return "MUCWindowStyle";
			else
				return "ChatWindowStyle";
		}
	}

	Core::Core ()
	: LinkRegexp_ ("((?:(?:\\w+://)|(?:xmpp:|mailto:|www\\.|magnet:|irc:))\\S+)",
			Qt::CaseInsensitive, QRegExp::RegExp2)
	, ImageRegexp_ ("(\\b(?:data:image/)[\\w\\d/\\?.=:@&%#_;\\(?:\\)\\+\\-\\~\\*\\,]+)",
			Qt::CaseInsensitive, QRegExp::RegExp2)
#ifdef ENABLE_CRYPT
	, QCAInit_ (new QCA::Initializer)
	, KeyStoreMgr_ (new QCA::KeyStoreManager)
	, QCAEventHandler_ (new QCA::EventHandler)
#endif
	, CLModel_ (new CLModel (this))
	, ChatTabsManager_ (new ChatTabsManager (this))
	, ItemIconManager_ (new AnimatedIconManager<QStandardItem*> (boost::bind (&QStandardItem::setIcon, _1, _2)))
	, SmilesOptionsModel_ (new SourceTrackingModel<IEmoticonResourceSource> (QStringList (tr ("Smile pack"))))
	, ChatStylesOptionsModel_ (new SourceTrackingModel<IChatStyleResourceSource> (QStringList (tr ("Chat style"))))
	, PluginManager_ (new PluginManager)
	, PluginProxyObject_ (new ProxyObject)
	, XferJobManager_ (new TransferJobManager)
	, CallManager_ (new CallManager)
	, EventsNotifier_ (new EventsNotifier)
	{
		FillANFields ();

#ifdef ENABLE_CRYPT
		connect (QCAEventHandler_.get (),
				SIGNAL (eventReady (int, const QCA::Event&)),
				this,
				SLOT (handleQCAEvent (int, const QCA::Event&)));
		if (KeyStoreMgr_->isBusy ())
			connect (KeyStoreMgr_.get (),
					SIGNAL (busyFinished ()),
					this,
					SLOT (handleQCABusyFinished ()),
					Qt::QueuedConnection);
		QCAEventHandler_->start ();
		KeyStoreMgr_->start ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("PublicEntryKeys");
		Q_FOREACH (const QString& entryId, settings.childKeys ())
			StoredPublicKeys_ [entryId] = settings.value (entryId).toString ();
		settings.endGroup ();
#endif
		ResourceLoaders_ [RLTStatusIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/contactlist/", this));
		ResourceLoaders_ [RLTClientIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/clients/", this));
		ResourceLoaders_ [RLTAffIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/affiliations/", this));
		ResourceLoaders_ [RLTSystemIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/system/", this));
		ResourceLoaders_ [RLTActivityIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/activities/", this));
		ResourceLoaders_ [RLTMoodIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/moods/", this));

		Q_FOREACH (boost::shared_ptr<Util::ResourceLoader> rl, ResourceLoaders_.values ())
		{
			rl->AddLocalPrefix ();
			rl->AddGlobalPrefix ();
		}

		connect (ChatTabsManager_,
				SIGNAL (clearUnreadMsgCount (QObject*)),
				this,
				SLOT (handleClearUnreadMsgCount (QObject*)));
		connect (XferJobManager_.get (),
				SIGNAL (jobNoLongerOffered (QObject*)),
				this,
				SLOT (handleJobDeoffered (QObject*)));
		connect (EventsNotifier_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (ChatTabsManager_,
				SIGNAL (entryMadeCurrent (QObject*)),
				EventsNotifier_.get (),
				SLOT (handleEntryMadeCurrent (QObject*)));

		PluginManager_->RegisterHookable (this);
		PluginManager_->RegisterHookable (CLModel_);

		SmilesOptionsModel_->AddModel (new QStringListModel (QStringList (QString ())));

		qRegisterMetaType<IMessage*> ("LeechCraft::Azoth::IMessage*");
		qRegisterMetaType<IMessage*> ("IMessage*");

		XmlSettingsManager::Instance ().RegisterObject ("StatusIcons",
				this, "updateStatusIconset");
		XmlSettingsManager::Instance ().RegisterObject ("GroupContacts",
				this, "handleGroupContactsChanged");
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		ResourceLoaders_.clear ();

#ifdef ENABLE_CRYPT
		QCAEventHandler_.reset ();
		KeyStoreMgr_.reset ();
		QCAInit_.reset ();
#endif
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	QList<ANFieldData> Core::GetANFields () const
	{
		return ANFields_;
	}

	Util::ResourceLoader* Core::GetResourceLoader (Core::ResourceLoaderType type) const
	{
		return ResourceLoaders_ [type].get ();
	}

	QAbstractItemModel* Core::GetSmilesOptionsModel () const
	{
		return SmilesOptionsModel_.get ();
	}

	IEmoticonResourceSource* Core::GetCurrentEmoSource () const
	{
		const QString& pack = XmlSettingsManager::Instance ()
				.property ("SmileIcons").toString ();
		return SmilesOptionsModel_->GetSourceForOption (pack);
	}

	QAbstractItemModel* Core::GetChatStylesOptionsModel()
	{
		return ChatStylesOptionsModel_.get ();
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin";
		return classes;
	}

	void Core::AddPlugin (QObject *plugin)
	{
		IPlugin2 *plugin2 = qobject_cast<IPlugin2*> (plugin);
		if (!plugin2)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "isn't a IPlugin2";
			return;
		}

		QByteArray sig = QMetaObject::normalizedSignature ("initPlugin (QObject*)");
		if (plugin->metaObject ()->indexOfMethod (sig) != -1)
			QMetaObject::invokeMethod (plugin,
					"initPlugin",
					Q_ARG (QObject*, PluginProxyObject_.get ()));

		PluginManager_->AddPlugin (plugin);

		QSet<QByteArray> classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"))
			AddProtocolPlugin (plugin);

		if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin"))
			AddResourceSourcePlugin (plugin);
	}

	void Core::RegisterHookable (QObject *object)
	{
		PluginManager_->RegisterHookable (object);
	}

	bool Core::CouldHandle (const Entity& e) const
	{
		if (!e.Entity_.canConvert<QUrl> ())
			return false;

		const QUrl& url = e.Entity_.toUrl ();
		if (!url.isValid ())
			return false;

		Q_FOREACH (QObject *obj, ProtocolPlugins_)
		{
			IProtocolPlugin *protoPlug = qobject_cast<IProtocolPlugin*> (obj);
			if (!protoPlug)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< obj
						<< "to IProtocolPlugin";
				continue;
			}

			Q_FOREACH (QObject *protoObj, protoPlug->GetProtocols ())
			{
				IURIHandler *handler = qobject_cast<IURIHandler*> (protoObj);
				if (!handler)
					continue;
				if (handler->SupportsURI (url))
					return true;
			}
		}

		return false;
	}

	void Core::Handle (Entity e)
	{
		const QUrl& url = e.Entity_.toUrl ();
		if (!url.isValid ())
			return;

		QList<QObject*> accounts;
		Q_FOREACH (QObject *obj, ProtocolPlugins_)
		{
			IProtocolPlugin *protoPlug = qobject_cast<IProtocolPlugin*> (obj);
			if (!protoPlug)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< obj
						<< "to IProtocolPlugin";
				continue;
			}

			Q_FOREACH (QObject *protoObj, protoPlug->GetProtocols ())
			{
				IURIHandler *handler = qobject_cast<IURIHandler*> (protoObj);
				if (!handler)
					continue;
				if (!handler->SupportsURI (url))
					continue;

				IProtocol *proto = qobject_cast<IProtocol*> (protoObj);
				if (!proto)
				{
					qWarning () << Q_FUNC_INFO
							<< protoObj
							<< "doesn't implement IProtocol";
					continue;
				}
				accounts << proto->GetRegisteredAccounts ();
			}
		}

		if (accounts.isEmpty ())
			return;

		QObject *selected = 0;

		if (accounts.size () > 1)
		{
			std::auto_ptr<AccountHandlerChooserDialog> dia (new AccountHandlerChooserDialog (accounts,
						tr ("Please select account to handle URI %1")
							.arg (url.toString ())));
			if (dia->exec () != QDialog::Accepted)
				return;

			selected = dia->GetSelectedAccount ();
		}
		else
			selected = accounts.at (0);

		if (!selected)
			return;

		QObject *selProto = qobject_cast<IAccount*> (selected)->GetParentProtocol ();
		qobject_cast<IURIHandler*> (selProto)->HandleURI (url, selected);
	}

	const QObjectList& Core::GetProtocolPlugins () const
	{
		return ProtocolPlugins_;
	}

	QAbstractItemModel* Core::GetCLModel () const
	{
		return CLModel_;
	}

	ChatTabsManager* Core::GetChatTabsManager () const
	{
		return ChatTabsManager_;
	}

	QList<IAccount*> Core::GetAccounts () const
	{
		QList<IAccount*> result;
		Q_FOREACH (QObject *protoObj, ProtocolPlugins_)
		{
			IProtocolPlugin *protoPlug =
					qobject_cast<IProtocolPlugin*> (protoObj);
			Q_FOREACH (QObject *protoObj, protoPlug->GetProtocols ())
			{
				IProtocol *proto = qobject_cast<IProtocol*> (protoObj);
				Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
				{
					IAccount *acc = qobject_cast<IAccount*> (accObj);
					if (!acc)
					{
						qWarning () << Q_FUNC_INFO
								<< "account object from protocol"
								<< proto->GetProtocolID ()
								<< "doesn't implement IAccount"
								<< accObj;
						continue;
					}
					result << acc;
				}
			}
		}
		return result;
	}

	QList<IProtocol*> Core::GetProtocols () const
	{
		QList<IProtocol*> result;
		Q_FOREACH (QObject *protoPlugin, ProtocolPlugins_)
		{
			QObjectList protos = qobject_cast<IProtocolPlugin*> (protoPlugin)->GetProtocols ();
			Q_FOREACH (QObject *obj, protos)
				result << qobject_cast<IProtocol*> (obj);
		}
		result.removeAll (0);
		return result;
	}

#ifdef ENABLE_CRYPT
	QList<QCA::PGPKey> Core::GetPublicKeys () const
	{
		QList<QCA::PGPKey> result;

		QCA::KeyStore store ("qca-gnupg", KeyStoreMgr_.get ());

		Q_FOREACH (const QCA::KeyStoreEntry& entry, store.entryList ())
		{
			const QCA::PGPKey& key = entry.pgpPublicKey ();
			if (!key.isNull ())
				result << key;
		}

		return result;
	}

	QList<QCA::PGPKey> Core::GetPrivateKeys () const
	{
		QList<QCA::PGPKey> result;

		QCA::KeyStore store ("qca-gnupg", KeyStoreMgr_.get ());

		Q_FOREACH (const QCA::KeyStoreEntry& entry, store.entryList ())
		{
			const QCA::PGPKey& key = entry.pgpSecretKey ();
			if (!key.isNull ())
				result << key;
		}

		return result;
	}

	void Core::AssociatePrivateKey (IAccount *acc, const QCA::PGPKey& key) const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("PrivateKeys");
		if (key.isNull ())
			settings.remove (acc->GetAccountID ());
		else
			settings.setValue (acc->GetAccountID (), key.keyId ());
		settings.endGroup ();
	}
#endif

	QStringList Core::GetChatGroups () const
	{
		QStringList result;
		Q_FOREACH (const ICLEntry *entry, Entry2Items_.keys ())
		{
			if (entry->GetEntryType () != ICLEntry::ETChat)
				continue;

			Q_FOREACH (const QString& group, entry->Groups ())
				if (!result.contains (group))
					result << group;
		}
		result.sort ();
		return result;
	}

	void Core::SendEntity (const LeechCraft::Entity& e)
	{
		emit gotEntity (e);
	}

	QObject* Core::GetEntry (const QString& id) const
	{
		return ID2Entry_.value (id);
	}

	void Core::OpenChat (const QModelIndex& contactIndex)
	{
		ChatTabsManager_->OpenChat (contactIndex);
	}

	TransferJobManager* Core::GetTransferJobManager () const
	{
		return XferJobManager_.get ();
	}

	CallManager* Core::GetCallManager () const
	{
		return CallManager_.get ();
	}

	bool Core::ShouldCountUnread (const ICLEntry *entry,
			IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookShouldCountUnread (proxy, msg->GetObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toBool ();

		return !ChatTabsManager_->IsActiveChat (entry) &&
				(msg->GetMessageType () == IMessage::MTChatMessage ||
				 msg->GetMessageType () == IMessage::MTMUCMessage);
	}

	bool Core::IsHighlightMessage (IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookIsHighlightMessage (proxy, msg->GetObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toBool ();

		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (msg->ParentCLEntry ());
		if (!mucEntry)
			return false;

		return msg->GetBody ().contains (mucEntry->GetNick ());
	}

	void Core::AddProtocolPlugin (QObject *plugin)
	{
		IProtocolPlugin *ipp =
			qobject_cast<IProtocolPlugin*> (plugin);
		if (!ipp)
			qWarning () << Q_FUNC_INFO
				<< "plugin"
				<< plugin
				<< "tells it implements the IProtocolPlugin but cast failed";
		else
		{
			ProtocolPlugins_ << plugin;

			QIcon icon = qobject_cast<IInfo*> (plugin)->GetIcon ();
			Q_FOREACH (QObject *protoObj, ipp->GetProtocols ())
			{
				IProtocol *proto = qobject_cast<IProtocol*> (protoObj);

				Q_FOREACH (QObject *accObj,
						proto->GetRegisteredAccounts ())
					addAccount (accObj);

				connect (proto->GetObject (),
						SIGNAL (accountAdded (QObject*)),
						this,
						SLOT (addAccount (QObject*)));
				connect (proto->GetObject (),
						SIGNAL (accountRemoved (QObject*)),
						this,
						SLOT (handleAccountRemoved (QObject*)));
			}
		}
	}

	void Core::AddResourceSourcePlugin (QObject *rp)
	{
		IResourcePlugin *irp = qobject_cast<IResourcePlugin*> (rp);
		if (!irp)
		{
			qWarning () << Q_FUNC_INFO
					<< rp
					<< "doesn't implement IResourcePlugin";
			return;
		}

		Q_FOREACH (QObject *object, irp->GetResourceSources ())
		{
			IEmoticonResourceSource *smileSrc = qobject_cast<IEmoticonResourceSource*> (object);
			if (smileSrc)
				AddSmileResourceSource (smileSrc);

			IChatStyleResourceSource *chatStyleSrc =
					qobject_cast<IChatStyleResourceSource*> (object);
			if (chatStyleSrc)
				AddChatStyleResourceSource (chatStyleSrc);
		}
	}

	void Core::AddSmileResourceSource (IEmoticonResourceSource *src)
	{
		SmilesOptionsModel_->AddSource (src);
	}

	void Core::AddChatStyleResourceSource (IChatStyleResourceSource *src)
	{
		ChatStylesOptionsModel_->AddSource (src);
	}

	QString Core::GetSelectedChatTemplate (QObject *entry, QWebFrame *frame) const
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (entry);
		if (!src)
			return QString ();

		const QString& opt = XmlSettingsManager::Instance ()
				.property (GetStyleOptName (entry)).toString ();
		return src->GetHTMLTemplate (opt, entry, frame);
	}

	QUrl Core::GetSelectedChatTemplateURL (QObject *entry) const
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (entry);
		if (!src)
			return QUrl ();

		const QString& opt = XmlSettingsManager::Instance ()
				.property (GetStyleOptName (entry)).toString ();
		return src->GetBaseURL (opt);
	}

	bool Core::AppendMessageByTemplate (QWebFrame *frame,
			QObject *message, const ChatMsgAppendInfo& info)
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (qobject_cast<IMessage*> (message)->ParentCLEntry ());
		if (!src)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty result for"
					<< message;
			return false;
		}

		return src->AppendMessage (frame, message, info);
	}

	void Core::FrameFocused (QObject *entry, QWebFrame *frame)
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (entry);
		if (!src)
			return;

		src->FrameFocused (frame);
	}

	namespace
	{
		qreal Fix (qreal h)
		{
			while (h < 0)
				h += 1;
			while (h >= 1)
				h -= 1;
			return h;
		}
	}

	QList<QColor> Core::GenerateColors (const QString& coloring) const
	{
		QList<QColor> result;
		if (coloring == "hash" ||
				coloring.isEmpty ())
		{
			const QColor& bg = QApplication::palette ().color (QPalette::Base);

			const qreal lower = 25. / 360.;
			const qreal delta = 50. / 360.;
			const qreal higher = 180. / 360. - delta / 2;

			const qreal alpha = bg.alphaF ();

			qreal h = bg.hueF ();

			QColor color;
			for (qreal d = lower; d <= higher; d += delta)
			{
				color.setHsvF (Fix (h + d), 1, 0.6, alpha);
				result << color;
				color.setHsvF (Fix (h - d), 1, 0.6, alpha);
				result << color;
				color.setHsvF (Fix (h + d), 1, 0.9, alpha);
				result << color;
				color.setHsvF (Fix (h - d), 1, 0.9, alpha);
				result << color;
			}
		}
		else
			Q_FOREACH (const QString& str,
					coloring.split (' ', QString::SkipEmptyParts))
				result << QColor (str);

		return result;
	}

	QString Core::GetNickColor (const QString& nick, const QList<QColor>& colors) const
	{
		if (colors.isEmpty ())
			return "green";

		int hash = 0;
		for (int i = 0; i < nick.length (); ++i)
		{
			const QChar& c = nick.at (i);
			hash += c.toLatin1 () ?
					c.toLatin1 () :
					c.unicode ();
			hash += nick.length ();
		}
		QColor nc = colors.at (hash % colors.size ());
		return nc.name ();
	}

	QString Core::FormatDate (QDateTime dt, IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatDateTime (proxy, this, dt, msg->GetObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("dateTime", dt);

		return dt.time ().toString ();
	}

	QString Core::FormatNickname (QString nick, IMessage *msg, const QString& color)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatNickname (proxy, this, nick, msg->GetObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("nick", nick);

		QString string;

		if (msg->GetMessageType () == IMessage::MTMUCMessage)
		{
			QUrl url ("azoth://insertnick/");
			url.addEncodedQueryItem ("nick", QUrl::toPercentEncoding (nick));

			string.append ("<span class='nickname'><a href=\"");
			string.append (url.toEncoded ());
			string.append ("\" class='nicklink' style='text-decoration:none; color:");
			string.append (color);
			string.append ("'>");
			string.append (nick);
			string.append ("</a></span>");
		}
		else
			string = QString ("<span class='nickname'>%1</span>")
					.arg (nick);

		return string;
	}

	QString Core::FormatBody (QString body, IMessage *msg)
	{
		QObject *msgObj = msg->GetObject ();

		IRichTextMessage *rtMsg = qobject_cast<IRichTextMessage*> (msgObj);
		const bool isRich = rtMsg && rtMsg->GetRichBody () == body;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		proxy->SetValue ("body", body);
		emit hookFormatBodyBegin (proxy, this, msgObj);
		if (!proxy->IsCancelled ())
		{
			proxy->FillValue ("body", body);

			if (!isRich)
			{
				int pos = 0;
				while ((pos = LinkRegexp_.indexIn (body, pos)) != -1)
				{
					QString link = LinkRegexp_.cap (1);
					if (pos > 0 &&
							(body.at (pos - 1) == '"' || body.at (pos - 1) == '='))
					{
						pos += link.size ();
						continue;
					}

					QString str = QString ("<a href=\"%1\">%1</a>")
							.arg (link);
					body.replace (pos, link.length (), str);

					pos += str.length ();
				}

				body.replace ('\n', "<br />");
				body.replace ("  ", "&nbsp; ");
			}

			body = HandleSmiles (body);

			proxy.reset (new Util::DefaultHookProxy);
			proxy->SetValue ("body", body);
			emit hookFormatBodyEnd (proxy, this, msgObj);
			proxy->FillValue ("body", body);
		}

		return proxy->IsCancelled () ?
				proxy->GetReturnValue ().toString () :
				body;
	}

	QString Core::HandleSmiles (QString body)
	{
		const QString& pack = XmlSettingsManager::Instance ()
				.property ("SmileIcons").toString ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookGonnaHandleSmiles (proxy, body, pack);
		if (proxy->IsCancelled ())
		{
			const QString& cand = proxy->GetReturnValue ().toString ();
			return cand.isEmpty () ? body : cand;
		}

		if (pack.isEmpty ())
			return body;

		IEmoticonResourceSource *src = SmilesOptionsModel_->GetSourceForOption (pack);
		if (!src)
			return body;

		const QString& img = QString ("<img src=\"%1\" title=\"%2\" />");
		Q_FOREACH (const QString& str, src->GetEmoticonStrings (pack))
		{
			const QString& escaped = Qt::escape (str);
			if (!body.contains (escaped))
				continue;
			const QByteArray& rawData = src->GetImage (pack, str);
			const QString& smileStr = img
					.arg (QString ("data:image/png;base64," + rawData.toBase64 ()))
					.arg (str);
			body.replace (escaped, smileStr);
		}

		return body;
	}

	namespace
	{
		QStringList GetDisplayGroups (const ICLEntry *clEntry)
		{
			QStringList groups;
			if (clEntry->GetEntryType () == ICLEntry::ETUnauthEntry)
				groups << Core::tr ("Unauthorized users");
			else if (clEntry->GetEntryType () != ICLEntry::ETChat ||
					XmlSettingsManager::Instance ()
						.property ("GroupContacts").toBool ())
				groups = clEntry->Groups ();
			else
				groups << Core::tr ("Contacts");
			return groups;
		}
	};

	void Core::AddCLEntry (ICLEntry *clEntry,
			QStandardItem *accItem)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookAddingCLEntryBegin (proxy, clEntry->GetObject ());
		if (proxy->IsCancelled ())
			return;

		connect (clEntry->GetObject (),
				SIGNAL (statusChanged (const EntryStatus&, const QString&)),
				this,
				SLOT (handleStatusChanged (const EntryStatus&, const QString&)));
		connect (clEntry->GetObject (),
				SIGNAL (availableVariantsChanged (const QStringList&)),
				this,
				SLOT (invalidateClientsIconCache ()));
		connect (clEntry->GetObject (),
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleEntryGotMessage (QObject*)));
		connect (clEntry->GetObject (),
				SIGNAL (nameChanged (const QString&)),
				this,
				SLOT (handleEntryNameChanged (const QString&)));
		connect (clEntry->GetObject (),
				SIGNAL (groupsChanged (const QStringList&)),
				this,
				SLOT (handleEntryGroupsChanged (const QStringList&)));
		connect (clEntry->GetObject (),
				SIGNAL (permsChanged ()),
				this,
				SLOT (handleEntryPermsChanged ()));
		connect (clEntry->GetObject (),
				SIGNAL (avatarChanged (const QImage&)),
				this,
				SLOT (invalidateSmoothAvatarCache ()));

		if (qobject_cast<IMUCEntry*> (clEntry->GetObject ()))
		{
			connect (clEntry->GetObject (),
					SIGNAL (nicknameConflict (const QString&)),
					this,
					SLOT (handleNicknameConflict (const QString&)));
			connect (clEntry->GetObject (),
					SIGNAL (beenKicked (const QString&)),
					this,
					SLOT (handleBeenKicked (const QString&)));
			connect (clEntry->GetObject (),
					SIGNAL (beenBanned (const QString&)),
					this,
					SLOT (handleBeenBanned (const QString&)));
		}

		if (qobject_cast<IAdvancedCLEntry*> (clEntry->GetObject ()))
		{
			connect (clEntry->GetObject (),
					SIGNAL (attentionDrawn (const QString&, const QString&)),
					this,
					SLOT (handleAttentionDrawn (const QString&, const QString&)));
			connect (clEntry->GetObject (),
					SIGNAL (activityChanged (const QString&)),
					this,
					SLOT (handleEntryPEPEvent (const QString&)));
			connect (clEntry->GetObject (),
					SIGNAL (moodChanged (const QString&)),
					this,
					SLOT (handleEntryPEPEvent (const QString&)));
			connect (clEntry->GetObject (),
					SIGNAL (tuneChanged (const QString&)),
					this,
					SLOT (handleEntryPEPEvent (const QString&)));
			connect (clEntry->GetObject (),
					SIGNAL (locationChanged (const QString&)),
					this,
					SLOT (handleEntryPEPEvent (const QString&)));
		}

#ifdef ENABLE_CRYPT
		if (!KeyStoreMgr_->isBusy ())
			RestoreKeyForEntry (clEntry);
#endif

		EventsNotifier_->RegisterEntry (clEntry);

		const QString& id = clEntry->GetEntryID ();
		ID2Entry_ [id] = clEntry->GetObject ();

		const QStringList& groups = GetDisplayGroups (clEntry);
		QList<QStandardItem*> catItems =
				GetCategoriesItems (groups, accItem);
		Q_FOREACH (QStandardItem *catItem, catItems)
			AddEntryTo (clEntry, catItem);

		HandleStatusChanged (clEntry->GetStatus (), clEntry, QString ());

		if (clEntry->GetEntryType () == ICLEntry::ETPrivateChat)
			handleEntryPermsChanged (clEntry);

		ChatTabsManager_->UpdateEntryMapping (id, clEntry->GetObject ());
		ChatTabsManager_->SetChatEnabled (id, true);

		proxy.reset (new Util::DefaultHookProxy);
		emit hookAddingCLEntryEnd (proxy, clEntry->GetObject ());
	}

	QList<QStandardItem*> Core::GetCategoriesItems (QStringList cats, QStandardItem *account)
	{
		if (cats.isEmpty ())
			cats << tr ("General");

		QList<QStandardItem*> result;
		Q_FOREACH (const QString& cat, cats)
		{
			if (!Account2Category2Item_ [account].keys ().contains (cat))
			{
				QStandardItem *catItem = new QStandardItem (cat);
				catItem->setEditable (false);
				catItem->setData (account->data (CLRAccountObject), CLRAccountObject);
				catItem->setData (QVariant::fromValue<CLEntryType> (CLETCategory),
						CLREntryType);
				catItem->setData (cat, CLREntryCategory);
				catItem->setFlags (catItem->flags () | Qt::ItemIsDropEnabled);
				Account2Category2Item_ [account] [cat] = catItem;
				account->appendRow (catItem);
			}

			result << Account2Category2Item_ [account] [cat];
		}

		return result;
	}

	QStandardItem* Core::GetAccountItem (const QObject *accountObj)
	{
		for (int i = 0, size = CLModel_->rowCount ();
				i < size; ++i)
			if (CLModel_->item (i)->
						data (CLRAccountObject).value<QObject*> () ==
					accountObj)
				return CLModel_->item (i);
		return 0;
	}

	QStandardItem* Core::GetAccountItem (const QObject *accountObj,
			QMap<const QObject*, QStandardItem*>& accountItemCache)
	{
		if (accountItemCache.contains (accountObj))
			return accountItemCache [accountObj];
		else
		{
			QStandardItem *accountItem = GetAccountItem (accountObj);
			if (accountItem)
				accountItemCache [accountObj] = accountItem;
			return accountItem;
		}
	}

	namespace
	{
		QString Status2Str (const EntryStatus& status, boost::shared_ptr<IProxyObject> obj)
		{
			QString result = obj->StateToString (status.State_);
			const QString& statusString = Qt::escape (status.StatusString_);
			if (!statusString.isEmpty ())
				result += " (" + statusString + ")";
			return result;
		}
	}

	namespace
	{
		void FormatMood (QString& tip, const QMap<QString, QVariant>& moodInfo)
		{
			tip += "<br />" + Core::tr ("Mood:") + ' ';
			tip += MoodDialog::ToHumanReadable (moodInfo ["mood"].toString ());
			const QString& text = moodInfo ["text"].toString ();
			if (!text.isEmpty ())
				tip += " (" + text + ")";
		}

		void FormatActivity (QString& tip, const QMap<QString, QVariant>& actInfo)
		{
			tip += "<br />" + Core::tr ("Activity:") + ' ';
			tip += ActivityDialog::ToHumanReadable (actInfo ["general"].toString ());
			const QString& specific = ActivityDialog::ToHumanReadable (actInfo ["specific"].toString ());
			if (!specific.isEmpty ())
				tip += " (" + specific + ")";
			const QString& text = actInfo ["text"].toString ();
			if (!text.isEmpty ())
				tip += " (" + text + ")";
		}

		void FormatTune (QString& tip, const QMap<QString, QVariant>& tuneInfo)
		{
			const QString& artist = tuneInfo ["artist"].toString ();
			const QString& source = tuneInfo ["source"].toString ();
			const QString& title = tuneInfo ["title"].toString ();

			tip += "<br />" + Core::tr ("Now listening to:") + ' ';
			if (!artist.isEmpty () && !title.isEmpty ())
				tip += "<em>" + artist + "</em>" +
						QString::fromUtf8 (" — ") +
						"<em>" + title + "</em>";
			else if (!artist.isEmpty ())
				tip += "<em>" + artist + "</em>";
			else if (!title.isEmpty ())
				tip += "<em>" + title + "</em>";

			if (!source.isEmpty ())
				tip += ' ' + Core::tr ("from") +
						" <em>" + source + "</em>";

			const int length = tuneInfo ["length"].toInt ();
			if (length)
				tip += " (" + Util::MakeTimeFromLong (length) + ")";
		}
	}

	QString Core::MakeTooltipString (ICLEntry *entry) const
	{
		QString tip = "<strong>" + entry->GetEntryName () + "</strong>";
		tip += "<br />" + entry->GetHumanReadableID () + "<br />";
		tip += Status2Str (entry->GetStatus (), PluginProxyObject_);
		if (entry->GetEntryType () != ICLEntry::ETPrivateChat)
		{
			tip += "<br />";
			tip += tr ("In groups: ") + entry->Groups ().join ("; ");
		}

		const QStringList& variants = entry->Variants ();

		IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (entry->GetParentCLEntry ());
		if (mucEntry)
		{
			const QString& jid = mucEntry->GetRealID (entry->GetObject ());
			tip += "<br />" + tr ("Real ID:") + ' ' + (jid.isEmpty () ? tr ("unknown") : jid);
		}

		IMUCPerms *mucPerms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (mucPerms)
		{
			tip += "<hr />";
			const QMap<QByteArray, QByteArray>& perms =
					mucPerms->GetPerms (entry->GetObject ());
			Q_FOREACH (const QByteArray& permClass, perms.keys ())
			{
				tip += mucPerms->GetUserString (permClass);
				tip += ": ";
				tip += mucPerms->GetUserString (perms [permClass]);
				tip += "<br />";
			}
		}

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		proxy->SetValue ("tooltip", tip);
		emit hookTooltipBeforeVariants (proxy, entry->GetObject ());
		proxy->FillValue ("tooltip", tip);

		if (entry->GetEntryType () != ICLEntry::ETPrivateChat)
			Q_FOREACH (const QString& variant, variants)
			{
				const QMap<QString, QVariant>& info = entry->GetClientInfo (variant);
				if (info.isEmpty ())
					continue;

				tip += "<hr />";
				if (!variant.isEmpty ())
					tip += "<strong>" + variant + "</strong>";

				if (info.contains ("priority"))
					tip += " (" + QString::number (info.value ("priority").toInt ()) + ")";
				tip += ": ";
				tip += Status2Str (entry->GetStatus (variant), PluginProxyObject_);

				if (info.contains ("client_name"))
					tip += "<br />" + tr ("Using:") + ' ' + info.value ("client_name").toString ();
				if (info.contains ("client_version"))
					tip += " " + info.value ("client_version").toString ();

				if (info.contains ("user_mood"))
					FormatMood (tip, info ["user_mood"].toMap ());
				if (info.contains ("user_activity"))
					FormatActivity (tip, info ["user_activity"].toMap ());
				if (info.contains ("user_tune"))
					FormatTune (tip, info ["user_tune"].toMap ());

				if (info.contains ("custom_user_visible_map"))
				{
					const QVariantMap& map = info ["custom_user_visible_map"].toMap ();
					Q_FOREACH (const QString& key, map.keys ())
						tip += "<br />" + key + ": " + map [key].toString () + "<br />";
				}
			}

		return tip;
	}

	Entity Core::BuildStatusNotification (const EntryStatus& entrySt,
		ICLEntry *entry, const QString& variant)
	{
		if (entry->GetEntryType () != ICLEntry::ETChat)
			return Entity ();

		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!LastAccountStatusChange_.contains (acc) ||
				LastAccountStatusChange_ [acc].secsTo (QDateTime::currentDateTime ()) < 5)
			return Entity ();

		IExtSelfInfoAccount *extAcc =
				qobject_cast<IExtSelfInfoAccount*> (entry->GetParentAccount ());
		if (extAcc &&
				extAcc->GetSelfContact () == entry->GetObject ())
			return Entity ();

		const QString& name = entry->GetEntryName ();
		const QString& status = Status2Str (entrySt, PluginProxyObject_);

		const QString& text = variant.isEmpty () ?
				Core::tr ("%1 is now %2.")
					.arg (name)
					.arg (status) :
				Core::tr ("%1/%2 is now %3.")
					.arg (name)
					.arg (variant)
					.arg (status);

		Entity e = Util::MakeNotification ("LeechCraft", text, PInfo_);
		e.Mime_ += "+advanced";

		BuildNotification (e, entry);
		e.Additional_ ["org.LC.AdvNotifications.EventType"] = "org.LC.AdvNotifications.IM.StatusChange";
		e.Additional_ ["NotificationPixmap"] =
				QVariant::fromValue<QPixmap> (QPixmap::fromImage (entry->GetAvatar ()));

		e.Additional_ ["org.LC.AdvNotifications.FullText"] = text;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = text;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;

		e.Additional_ ["org.LC.Plugins.Azoth.Msg"] = entrySt.StatusString_;
		e.Additional_ ["org.LC.Plugins.Azoth.NewStatus"] =
				PluginProxyObject_->StateToString (entrySt.State_);

		return e;
	}

	void Core::HandleStatusChanged (const EntryStatus& status,
			ICLEntry *entry, const QString& variant, bool asSignal)
	{
		emit hookEntryStatusChanged (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy),
				entry->GetObject (), variant);

		invalidateClientsIconCache (entry);
		const QString& tip = MakeTooltipString (entry);

		const State state = entry->GetStatus ().State_;
		const QString& icon = GetIconPathForState (state);

		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
		{
			item->setToolTip (tip);
			ItemIconManager_->SetIcon (item, icon);
		}

		const QString& id = entry->GetEntryID ();
		if (!XferJobManager_->GetPendingIncomingJobsFor (id).isEmpty ())
			CheckFileIcon (id);

		if (asSignal)
		{
			const Entity& e = BuildStatusNotification (status, entry, variant);
			if (!e.Mime_.isEmpty ())
				emit gotEntity (e);
		}
	}

	void Core::CheckFileIcon (const QString& id)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (GetEntry (id));
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null entry for"
					<< id;
			return;
		}

		if (XferJobManager_->GetPendingIncomingJobsFor (id).isEmpty ())
		{
			const QString& variant = entry->Variants ().value (0);
			HandleStatusChanged (entry->GetStatus (variant), entry, variant);
			return;
		}

		const QString& filename = XmlSettingsManager::Instance ()
				.property ("StatusIcons").toString () + "/file";
		const QString& fileIcon = ResourceLoaders_ [RLTStatusIconLoader]->GetIconPath (filename);

		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
			ItemIconManager_->SetIcon (item, fileIcon);
	}

	void Core::IncreaseUnreadCount (ICLEntry* entry, int amount)
	{
		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
			{
				int prevValue = item->data (CLRUnreadMsgCount).toInt ();
				item->setData (std::max (0, prevValue + amount), CLRUnreadMsgCount);
				RecalculateUnreadForParents (item);
			}
	}

	QString Core::GetIconPathForState (State state) const
	{
		QString iconName;
		switch (state)
		{
		case SOnline:
			iconName = "online";
			break;
		case SChat:
			iconName = "chatty";
			break;
		case SAway:
			iconName = "away";
			break;
		case SDND:
			iconName = "dnd";
			break;
		case SXA:
			iconName = "xa";
			break;
		case SOffline:
			iconName = "offline";
			break;
		case SConnecting:
			iconName = "connect";
			break;
		default:
			iconName = "perr";
			break;
		}

		QString filename = XmlSettingsManager::Instance ()
				.property ("StatusIcons").toString ();
		filename += '/';
		filename += iconName;
		QStringList variants;
		variants << filename + ".svg"
				<< filename + ".png"
				<< filename + ".jpg";

		return ResourceLoaders_ [RLTStatusIconLoader]->GetPath (variants);
	}

	QIcon Core::GetIconForState (State state) const
	{
		return QIcon (GetIconPathForState (state));
	}

	QIcon Core::GetAffIcon (const QByteArray& affName) const
	{
		QString filename = XmlSettingsManager::Instance ()
				.property ("AffIcons").toString ();
		filename += '/';
		filename += affName;

		const QString& path = ResourceLoaders_ [RLTAffIconLoader]->GetIconPath (filename);
		return QIcon (path);
	}

	QMap<QString, QIcon> Core::GetClientIconForEntry (ICLEntry *entry)
	{
		if (EntryClientIconCache_.contains (entry))
			return EntryClientIconCache_ [entry];

		QMap<QString, QIcon> result;

		const QString& pack = XmlSettingsManager::Instance ()
					.property ("ClientIcons").toString () + '/';
		Q_FOREACH (const QString& variant, entry->Variants ())
		{
			const QString& filename = pack + entry->GetClientInfo (variant) ["client_type"].toString ();

			QString path = ResourceLoaders_ [RLTClientIconLoader]->GetIconPath (filename);
			if (path.isNull ())
				path = ResourceLoaders_ [RLTClientIconLoader]->GetIconPath (pack + "unknown");

			result [variant] = QIcon (path);
		}

		EntryClientIconCache_ [entry] = result;
		return result;
	}

	QImage Core::GetAvatar (ICLEntry *entry, int size)
	{
		if (Entry2SmoothAvatarCache_.contains (entry) &&
				(Entry2SmoothAvatarCache_ [entry].width () == size ||
				 Entry2SmoothAvatarCache_ [entry].height () == size))
			return Entry2SmoothAvatarCache_ [entry];

		QImage avatar = entry ? entry->GetAvatar () : QImage ();
		if (avatar.isNull () || !avatar.width ())
		{
			const QString& name = XmlSettingsManager::Instance ()
					.property ("SystemIcons").toString () + "/default_avatar";
			avatar = QImage (ResourceLoaders_ [RLTSystemIconLoader]->GetIconPath (name));
		}

		const QImage& scaled = avatar.scaled (size, size,
				Qt::KeepAspectRatio, Qt::SmoothTransformation);
		Entry2SmoothAvatarCache_ [entry] = scaled;
		return scaled;
	}

	QList<QAction*> Core::GetEntryActions (ICLEntry *entry)
	{
		if (!entry)
			return QList<QAction*> ();

		if (!Entry2Actions_.contains (entry))
			CreateActionsForEntry (entry);
		UpdateActionsForEntry (entry);

		const QHash<QByteArray, QAction*>& id2action = Entry2Actions_ [entry];
		QList<QAction*> result;
		result << id2action.value ("openchat");
		result << id2action.value ("drawattention");
		result << id2action.value ("sep_afterinitiate");
		result << id2action.value ("rename");
		result << id2action.value ("changegroups");
		result << id2action.value ("remove");
		result << id2action.value ("sep_afterrostermodify");
		result << id2action.value ("authorization");
		IMUCPerms *perms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (perms)
			Q_FOREACH (const QByteArray& permClass, perms->GetPossiblePerms ().keys ())
				result << id2action.value (permClass);
		result << id2action.value ("sep_afterroles");
		result << id2action.value ("add_contact");
		result << id2action.value ("copy_id");
		result << id2action.value ("sep_afterjid");
		result << id2action.value ("managepgp");
		result << id2action.value ("shareRIEX");
		result << id2action.value ("vcard");
		result << id2action.value ("invite");
		result << id2action.value ("leave");
		result << id2action.value ("authorize");
		result << id2action.value ("denyauth");
		result << entry->GetActions ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		proxy->SetReturnValue (QVariantList ());
		emit hookEntryActionsRequested (proxy, entry->GetObject ());
		Q_FOREACH (const QVariant& var, proxy->GetReturnValue ().toList ())
		{
			QObject *obj = var.value<QObject*> ();
			QAction *act = qobject_cast<QAction*> (obj);
			if (!act)
				continue;

			result << act;

			proxy.reset (new Util::DefaultHookProxy);
			emit hookEntryActionAreasRequested (proxy, act, entry->GetObject ());
			Q_FOREACH (const QString& place, proxy->GetReturnValue ().toStringList ())
			{
				if (place == "contactListContextMenu")
					Action2Areas_ [act] << CLEAAContactListCtxtMenu;
				else if (place == "tabContextMenu")
					Action2Areas_ [act] << CLEAATabCtxtMenu;
				else if (place == "applicationMenu")
					Action2Areas_ [act] << CLEAAApplicationMenu;
				else if (place == "toolbar")
					Action2Areas_ [act] << CLEAAToolbar;
				else
					qWarning () << Q_FUNC_INFO
							<< "unknown embed place ID"
							<< place;
			}
		}

		result.removeAll (0);

		Proxy_->UpdateIconset (result);

		return result;
	}

	QList<Core::CLEntryActionArea> Core::GetAreasForAction (const QAction *action) const
	{
		return Action2Areas_.value (action,
				QList<CLEntryActionArea> () << CLEAAContactListCtxtMenu);
	}

	void Core::RecalculateUnreadForParents (QStandardItem *clItem)
	{
		QStandardItem *category = clItem->parent ();
		int sum = 0;
		for (int i = 0, rc = category->rowCount ();
				i < rc; ++i)
			sum += category->child (i)->data (CLRUnreadMsgCount).toInt ();
		category->setData (sum, CLRUnreadMsgCount);
	}

	void Core::CreateActionsForEntry (ICLEntry *entry)
	{
		if (!entry)
			return;

		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetObject ());

		if (Entry2Actions_.contains (entry))
			Q_FOREACH (const QAction *action,
						Entry2Actions_.take (entry).values ())
			{
				Action2Areas_.remove (action);
				delete action;
			}

		QAction *openChat = new QAction (tr ("Open chat"), entry->GetObject ());
		openChat->setProperty ("ActionIcon", "azoth_openchat");
		connect (openChat,
				SIGNAL (triggered ()),
				this,
				SLOT (handleActionOpenChatTriggered ()));
		Entry2Actions_ [entry] ["openchat"] = openChat;
		Action2Areas_ [openChat] << CLEAAContactListCtxtMenu;

		if (advEntry)
		{
			QAction *drawAtt = new QAction (tr ("Draw attention..."), entry->GetObject ());
			connect (drawAtt,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionDrawAttention ()));
			drawAtt->setProperty ("ActionIcon", "draw_attention");
			Entry2Actions_ [entry] ["drawattention"] = drawAtt;
			Action2Areas_ [drawAtt] << CLEAAContactListCtxtMenu;
		}

		QAction *rename = new QAction (tr ("Rename"), entry->GetObject ());
		connect (rename,
				SIGNAL (triggered ()),
				this,
				SLOT (handleActionRenameTriggered ()));
		rename->setProperty ("ActionIcon", "azoth_rename");
		Entry2Actions_ [entry] ["rename"] = rename;
		Action2Areas_ [rename] << CLEAAContactListCtxtMenu;

		if (entry->GetEntryFeatures () & ICLEntry::FSupportsGrouping)
		{
			QAction *changeGroups = new QAction (tr ("Change groups..."), entry->GetObject ());
			connect (changeGroups,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionChangeGroupsTriggered ()));
			changeGroups->setProperty ("ActionIcon", "azoth_changegroups");
			Entry2Actions_ [entry] ["changegroups"] = changeGroups;
			Action2Areas_ [changeGroups] << CLEAAContactListCtxtMenu;
		}

		if (entry->GetEntryFeatures () & ICLEntry::FSupportsAuth)
		{
			QMenu *authMenu = new QMenu (tr ("Authorization"));
			authMenu->menuAction ()->setProperty ("ActionIcon", "azoth_menu_authorization");
			Entry2Actions_ [entry] ["authorization"] = authMenu->menuAction ();
			Action2Areas_ [authMenu->menuAction ()] << CLEAAContactListCtxtMenu;

			QAction *grantAuth = authMenu->addAction (tr ("Grant"),
					this, SLOT (handleActionGrantAuthTriggered ()));
			grantAuth->setProperty ("ActionIcon", "azoth_auth_grant");
			grantAuth->setProperty ("Azoth/WithReason", false);

			QAction *grantAuthReason = authMenu->addAction (tr ("Grant with reason..."),
					this, SLOT (handleActionGrantAuthTriggered ()));
			grantAuthReason->setProperty ("ActionIcon", "azoth_auth_grant");
			grantAuthReason->setProperty ("Azoth/WithReason", true);

			QAction *revokeAuth = authMenu->addAction (tr ("Revoke"),
					this, SLOT (handleActionRevokeAuthTriggered ()));
			revokeAuth->setProperty ("ActionIcon", "azoth_auth_revoke");
			revokeAuth->setProperty ("Azoth/WithReason", false);

			QAction *revokeAuthReason = authMenu->addAction (tr ("Revoke with reason..."),
					this, SLOT (handleActionRevokeAuthTriggered ()));
			revokeAuthReason->setProperty ("ActionIcon", "azoth_auth_revoke_reason");
			revokeAuthReason->setProperty ("Azoth/WithReason", true);

			QAction *unsubscribe = authMenu->addAction (tr ("Unsubscribe"),
					this, SLOT (handleActionUnsubscribeTriggered ()));
			unsubscribe->setProperty ("ActionIcon", "azoth_auth_unsubscribe");
			unsubscribe->setProperty ("Azoth/WithReason", false);

			QAction *unsubscribeReason = authMenu->addAction (tr ("Unsubscribe with reason..."),
					this, SLOT (handleActionUnsubscribeTriggered ()));
			unsubscribeReason->setProperty ("ActionIcon", "azoth_auth_unsubscribe");
			unsubscribeReason->setProperty ("Azoth/WithReason", true);

			QAction *rerequest = authMenu->addAction (tr ("Rerequest authentication"),
					this, SLOT (handleActionRerequestTriggered ()));
			rerequest->setProperty ("ActionIcon", "azoth_auth_rerequest");
			rerequest->setProperty ("Azoth/WithReason", false);

			QAction *rerequestReason = authMenu->addAction (tr ("Rerequest authentication with reason..."),
					this, SLOT (handleActionRerequestTriggered ()));
			rerequestReason->setProperty ("ActionIcon", "azoth_auth_rerequest");
			rerequestReason->setProperty ("Azoth/WithReason", true);
		}

#ifdef ENABLE_CRYPT
		if (qobject_cast<ISupportPGP*> (entry->GetParentAccount ()))
		{
			QAction *manageGPG = new QAction (tr ("Manage PGP keys..."), entry->GetObject ());
			connect (manageGPG,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionManagePGPTriggered ()));
			manageGPG->setProperty ("ActionIcon", "encryption");
			Entry2Actions_ [entry] ["managepgp"] = manageGPG;
			Action2Areas_ [manageGPG] << CLEAAContactListCtxtMenu;
		}
#endif

		if (qobject_cast<ISupportRIEX*> (entry->GetParentAccount ()))
		{
			QAction *shareRIEX = new QAction (tr ("Share contacts..."), entry->GetObject ());
			connect (shareRIEX,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionShareContactsTriggered ()));
			Entry2Actions_ [entry] ["shareRIEX"] = shareRIEX;
			Action2Areas_ [shareRIEX] << CLEAAContactListCtxtMenu;
		}

		if (entry->GetEntryType () != ICLEntry::ETMUC)
		{
			QAction *vcard = new QAction (tr ("VCard"), entry->GetObject ());
			connect (vcard,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionVCardTriggered ()));
			vcard->setProperty ("ActionIcon", "personalinfo");
			Entry2Actions_ [entry] ["vcard"] = vcard;
			Action2Areas_ [vcard] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu
					<< CLEAAToolbar;
		}

		IMUCPerms *perms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat)
		{
			if (perms)
			{
				const QMap<QByteArray, QList<QByteArray> >& possible = perms->GetPossiblePerms ();
				Q_FOREACH (const QByteArray& permClass, possible.keys ())
				{
					QMenu *changeClass = new QMenu (perms->GetUserString (permClass));
					Entry2Actions_ [entry] [permClass] = changeClass->menuAction ();
					Action2Areas_ [changeClass->menuAction ()] << CLEAAContactListCtxtMenu;

					Q_FOREACH (const QByteArray& perm, possible [permClass])
					{
						QAction *permAct = changeClass->addAction (perms->GetUserString (perm),
								this,
								SLOT (handleActionPermTriggered ()));
						permAct->setParent (entry->GetObject ());
						permAct->setCheckable (true);
						permAct->setProperty ("Azoth/TargetPermClass", permClass);
						permAct->setProperty ("Azoth/TargetPerm", perm);
					}
				}

				QAction *sep = Util::CreateSeparator (entry->GetObject ());
				Entry2Actions_ [entry] ["sep_afterroles"] = sep;
				Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
			}

			QAction *addContact = new QAction (tr ("Add to contact list..."), entry->GetObject ());
			addContact->setProperty ("ActionIcon", "add");
			connect (addContact,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionAddContactFromMUC ()));
			Entry2Actions_ [entry] ["add_contact"] = addContact;
			Action2Areas_ [addContact] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			QAction *copyId = new QAction (tr ("Copy ID"), entry->GetObject ());
			copyId->setProperty ("ActionIcon", "copy");
			connect (copyId,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionCopyMUCPartID ()));
			Entry2Actions_ [entry] ["copy_id"] = copyId;
			Action2Areas_ [copyId] << CLEAAContactListCtxtMenu;

			QAction *sep = Util::CreateSeparator (entry->GetObject ());
			Entry2Actions_ [entry] ["sep_afterjid"] = sep;
			Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::ETMUC)
		{
			QAction *invite = new QAction (tr ("Invite..."), entry->GetObject ());
			invite->setProperty ("ActionIcon", "azoth_invite");
			connect (invite,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionInviteTriggered ()));
			Entry2Actions_ [entry] ["invite"] = invite;
			Action2Areas_ [invite] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			QAction *leave = new QAction (tr ("Leave"), entry->GetObject ());
			leave->setProperty ("ActionIcon", "azoth_leave");
			connect (leave,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionLeaveTriggered ()));
			Entry2Actions_ [entry] ["leave"] = leave;
			Action2Areas_ [leave] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::ETUnauthEntry)
		{
			QAction *authorize = new QAction (tr ("Authorize"), entry->GetObject ());
			authorize->setProperty ("ActionIcon", "azoth_authorize");
			connect (authorize,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionAuthorizeTriggered ()));
			Entry2Actions_ [entry] ["authorize"] = authorize;
			Action2Areas_ [authorize] << CLEAAContactListCtxtMenu;

			QAction *denyAuth = new QAction (tr ("Deny authorization"), entry->GetObject ());
			denyAuth->setProperty ("ActionIcon", "azoth_denyauth");
			connect (denyAuth,
						SIGNAL (triggered ()),
						this,
						SLOT (handleActionDenyAuthTriggered ()));
			Entry2Actions_ [entry] ["denyauth"] = denyAuth;
			Action2Areas_ [denyAuth] << CLEAAContactListCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::ETChat)
		{
			QAction *remove = new QAction (tr ("Remove"), entry->GetObject ());
			remove->setProperty ("ActionIcon", "remove");
			connect (remove,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionRemoveTriggered ()));
			Entry2Actions_ [entry] ["remove"] = remove;
			Action2Areas_ [remove] << CLEAAContactListCtxtMenu;
		}

		QAction *sep = Util::CreateSeparator (entry->GetObject ());
		Entry2Actions_ [entry] ["sep_afterinitiate"] = sep;
		Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
		sep = Util::CreateSeparator (entry->GetObject ());
		Entry2Actions_ [entry] ["sep_afterrostermodify"] = sep;
		Action2Areas_ [sep] << CLEAAContactListCtxtMenu;

		struct Entrifier
		{
			QVariant Entry_;

			Entrifier (const QVariant& entry)
			: Entry_ (entry)
			{
			}

			void Do (const QList<QAction*>& actions)
			{
				Q_FOREACH (QAction *act, actions)
				{
					act->setProperty ("Azoth/Entry", Entry_);
					act->setParent (Entry_.value<ICLEntry*> ()->GetObject ());
					QMenu *menu = act->menu ();
					if (menu)
						Do (menu->actions ());
				}
			}
		} entrifier (QVariant::fromValue<ICLEntry*> (entry));
		entrifier.Do (Entry2Actions_ [entry].values ());
	}

	void Core::UpdateActionsForEntry (ICLEntry *entry)
	{
		if (!entry)
			return;

		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetObject ());

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		const bool isOnline = account->GetState ().State_ != SOffline;
		if (entry->GetEntryType () != ICLEntry::ETMUC)
		{
			bool enableVCard =
					account->GetAccountFeatures () & IAccount::FCanViewContactsInfoInOffline ||
					isOnline;
			Entry2Actions_ [entry] ["vcard"]->setEnabled (enableVCard);
		}

		Entry2Actions_ [entry] ["rename"]->setEnabled (entry->GetEntryFeatures () & ICLEntry::FSupportsRenames);

		if (advEntry)
		{
			bool suppAtt = advEntry->GetAdvancedFeatures () & IAdvancedCLEntry::AFSupportsAttention;
			Entry2Actions_ [entry] ["drawattention"]->setEnabled (suppAtt);
		}

		if (entry->GetEntryType () == ICLEntry::ETChat)
		{
			Entry2Actions_ [entry] ["remove"]->setEnabled (isOnline);
			if (Entry2Actions_ [entry] ["authorization"])
				Entry2Actions_ [entry] ["authorization"]->setEnabled (isOnline);
		}

		IMUCEntry *thisMuc = qobject_cast<IMUCEntry*> (entry->GetObject ());
		if (thisMuc)
			Entry2Actions_ [entry] ["invite"]->
					setEnabled (thisMuc->GetMUCFeatures () & IMUCEntry::MUCFCanInvite);

		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (entry->GetParentCLEntry ());
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat &&
				!mucEntry)
			qWarning () << Q_FUNC_INFO
					<< "parent of"
					<< entry->GetObject ()
					<< entry->GetParentCLEntry ()
					<< "doesn't implement IMUCEntry";

		IMUCPerms *mucPerms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat)
		{
			if (mucPerms)
			{
				const QMap<QByteArray, QList<QByteArray> > possible = mucPerms->GetPossiblePerms ();
				QObject *entryObj = entry->GetObject ();
				Q_FOREACH (const QByteArray& permClass, possible.keys ())
					Q_FOREACH (QAction *action,
							Entry2Actions_ [entry] [permClass]->menu ()->actions ())
					{
						const QByteArray& perm = action->property ("Azoth/TargetPerm").toByteArray ();
						action->setEnabled (mucPerms->MayChangePerm (entryObj,
									permClass, perm));
						action->setChecked (perm == mucPerms->GetPerms (entryObj) [permClass]);
					}
			}

			const QString& realJid = mucEntry->GetRealID (entry->GetObject ());
			Entry2Actions_ [entry] ["add_contact"]->setEnabled (!realJid.isEmpty ());
			Entry2Actions_ [entry] ["add_contact"]->setProperty ("Azoth/RealID", realJid);
			Entry2Actions_ [entry] ["copy_id"]->setEnabled (!realJid.isEmpty ());
			Entry2Actions_ [entry] ["copy_id"]->setProperty ("Azoth/RealID", realJid);
		}
	}

	QString Core::GetReason (const QString&, const QString& text)
	{
		return QInputDialog::getText (0,
					tr ("Enter reason"),
					text);
	}

	void Core::ManipulateAuth (const QString& id, const QString& text,
			boost::function<void (IAuthable*, const QString&)> func)
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IAuthable *authable =
				qobject_cast<IAuthable*> (entry->GetObject ());
		if (!authable)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "doesn't implement IAuthable";
			return;
		}

		QString reason;
		if (action->property ("Azoth/WithReason").toBool ())
		{
			reason = GetReason (id, text.arg (entry->GetEntryName ()));
			if (reason.isEmpty ())
				return;
		}
		func (authable, reason);
	}

	void Core::RemoveCLItem (QStandardItem *item)
	{
		QObject *entryObj = item->data (CLREntryObject).value<QObject*> ();
		Entry2Items_ [qobject_cast<ICLEntry*> (entryObj)].removeAll (item);

		QStandardItem *category = item->parent ();
		QString text = category->text ();
		ItemIconManager_->Cancel (item);
		category->removeRow (item->row ());

		if (!category->rowCount ())
		{
			QStandardItem *account = category->parent ();
			ItemIconManager_->Cancel (category);
			account->removeRow (category->row ());
			Account2Category2Item_ [account].remove (text);
		}
	}

	void Core::AddEntryTo (ICLEntry *clEntry, QStandardItem *catItem)
	{
		QStandardItem *clItem = new QStandardItem (clEntry->GetEntryName ());
		clItem->setEditable (false);
		QObject *accObj = clEntry->GetParentAccount ();
		clItem->setData (QVariant::fromValue<QObject*> (accObj),
				CLRAccountObject);
		clItem->setData (QVariant::fromValue<QObject*> (clEntry->GetObject ()),
				CLREntryObject);
		clItem->setData (QVariant::fromValue<CLEntryType> (CLETContact),
				CLREntryType);
		clItem->setData (catItem->data (CLREntryCategory),
				CLREntryCategory);

		clItem->setFlags (clItem->flags () |
				Qt::ItemIsDragEnabled |
				Qt::ItemIsDropEnabled);

		catItem->appendRow (clItem);

		Entry2Items_ [clEntry] << clItem;
	}

	void Core::SuggestJoiningMUC (IAccount *acc, const QVariantMap& ident)
	{
		QList<IAccount*> accs;
		accs << acc;

		JoinConferenceDialog *dia = new JoinConferenceDialog (accs, Proxy_->GetMainWindow ());
		dia->SetIdentifyingData (ident);
		dia->show ();
	}

	IChatStyleResourceSource* Core::GetCurrentChatStyle (QObject *entry) const
	{
		const QString& opt = XmlSettingsManager::Instance ()
				.property (GetStyleOptName (entry)).toString ();
		IChatStyleResourceSource *src = ChatStylesOptionsModel_->GetSourceForOption (opt);
		if (!src)
			qWarning () << Q_FUNC_INFO
					<< "empty result for"
					<< opt;
		return src;
	}

	void Core::FillANFields ()
	{
		const QStringList commonFields = QStringList ("org.LC.AdvNotifications.IM.MUCHighlightMessage")
						<< "org.LC.AdvNotifications.IM.MUCMessage"
						<< "org.LC.AdvNotifications.IM.IncomingMessage"
						<< "org.LC.AdvNotifications.IM.AttentionDrawn"
						<< "org.LC.AdvNotifications.IM.Subscr.Granted"
						<< "org.LC.AdvNotifications.IM.Subscr.Revoked"
						<< "org.LC.AdvNotifications.IM.Subscr.Requested"
						<< "org.LC.AdvNotifications.IM.StatusChange";

		ANFields_ << ANFieldData ("org.LC.Plugins.Azoth.Msg",
				tr ("Message body"),
				tr ("Original human-readable message body."),
				QVariant::String,
				commonFields);

		ANFields_ << ANFieldData ("org.LC.Plugins.Azoth.SourceName",
				tr ("Sender name"),
				tr ("Human-readable name of the sender of the message."),
				QVariant::String,
				commonFields);

		ANFields_ << ANFieldData ("org.LC.Plugins.Azoth.SourceID",
				tr ("Sender ID"),
				tr ("Human-readable ID of the sender (protocol-specific)."),
				QVariant::String,
				commonFields);

		ANFields_ << ANFieldData ("org.LC.Plugins.Azoth.SourceGroups",
				tr ("Sender groups"),
				tr ("Groups to which the sender belongs."),
				QVariant::StringList,
				commonFields);

		ANFields_ << ANFieldData ("org.LC.Plugins.Azoth.NewStatus",
				tr ("New status"),
				tr ("The new status string of the contact."),
				QVariant::String,
				QStringList ("org.LC.AdvNotifications.IM.StatusChange"));
	}

#ifdef ENABLE_CRYPT
	void Core::RestoreKeyForAccount (IAccount *acc)
	{
		ISupportPGP *pgp = qobject_cast<ISupportPGP*> (acc->GetObject ());
		if (!pgp)
			return;

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("PrivateKeys");
		const QString& keyId = settings.value (acc->GetAccountID ()).toString ();
		settings.endGroup ();

		if (keyId.isEmpty ())
			return;

		Q_FOREACH (const QCA::PGPKey& key, GetPrivateKeys ())
			if (key.keyId () == keyId)
			{
				pgp->SetPrivateKey (key);
				break;
			}
	}

	void Core::RestoreKeyForEntry (ICLEntry *clEntry)
	{
		if (!StoredPublicKeys_.contains (clEntry->GetEntryID ()))
			return;

		ISupportPGP *pgp = qobject_cast<ISupportPGP*> (clEntry->GetParentAccount ());
		if (!pgp)
		{
			qWarning () << Q_FUNC_INFO
					<< clEntry->GetObject ()
					<< clEntry->GetParentAccount ()
					<< "doesn't implement ISupportGPG though "
						"we have the key";
			return;
		}

		const QString& keyId = StoredPublicKeys_.take (clEntry->GetEntryID ());
		Q_FOREACH (const QCA::PGPKey& key, GetPublicKeys ())
			if (key.keyId () == keyId)
			{
				pgp->SetEntryKey (clEntry->GetObject (), key);
				break;
			}
	}
#endif

	void Core::handleMucJoinRequested ()
	{
		QList<IAccount*> accounts;
		Q_FOREACH (QObject *protoPlugin, ProtocolPlugins_)
		{
			QObjectList protocols =
					qobject_cast<IProtocolPlugin*> (protoPlugin)->GetProtocols ();
			Q_FOREACH (QObject *protoObj, protocols)
			{
				IProtocol *proto = qobject_cast<IProtocol*> (protoObj);
				if (!(proto->GetFeatures () & IProtocol::PFMUCsJoinable))
					continue;

				QObjectList accountObjs = proto->GetRegisteredAccounts ();
				Q_FOREACH (QObject *accountObj, accountObjs)
					accounts << qobject_cast<IAccount*> (accountObj);
			}
		}

		JoinConferenceDialog *dia = new JoinConferenceDialog (accounts, Proxy_->GetMainWindow ());
		dia->show ();
	}

	void Core::addAccount (QObject *accObject)
	{
		IAccount *account =
				qobject_cast<IAccount*> (accObject);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObject
					<< sender ();
			return;
		}

		emit accountAdded (account);

#ifdef ENABLE_CRYPT
		if (!KeyStoreMgr_->isBusy ())
			RestoreKeyForAccount (account);
#endif

		QStandardItem *accItem = new QStandardItem (account->GetAccountName ());
		accItem->setData (QVariant::fromValue<QObject*> (accObject),
				CLRAccountObject);
		accItem->setData (QVariant::fromValue<CLEntryType> (CLETAccount),
				CLREntryType);
		ItemIconManager_->SetIcon (accItem, GetIconPathForState (account->GetState ().State_));
		CLModel_->appendRow (accItem);

		accItem->setEditable (false);

		QList<QStandardItem*> clItems;
		Q_FOREACH (QObject *clObj, account->GetCLEntries ())
		{
			ICLEntry *clEntry = qobject_cast<ICLEntry*> (clObj);
			if (!clEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< "entry doesn't implement ICLEntry"
						<< clObj
						<< account;
				continue;
			}

			AddCLEntry (clEntry, accItem);
		}

		connect (accObject,
				SIGNAL (gotCLItems (const QList<QObject*>&)),
				this,
				SLOT (handleGotCLItems (const QList<QObject*>&)));
		connect (accObject,
				SIGNAL (removedCLItems (const QList<QObject*>&)),
				this,
				SLOT (handleRemovedCLItems (const QList<QObject*>&)));
		connect (accObject,
				SIGNAL (authorizationRequested (QObject*, const QString&)),
				this,
				SLOT (handleAuthorizationRequested (QObject*, const QString&)));

		connect (accObject,
				SIGNAL (itemSubscribed (QObject*, const QString&)),
				this,
				SLOT (handleItemSubscribed (QObject*, const QString&)));
		connect (accObject,
				SIGNAL (itemUnsubscribed (QObject*, const QString&)),
				this,
				SLOT (handleItemUnsubscribed (QObject*, const QString&)));
		connect (accObject,
				SIGNAL (itemUnsubscribed (const QString&, const QString&)),
				this,
				SLOT (handleItemUnsubscribed (const QString&, const QString&)));
		connect (accObject,
				SIGNAL (itemCancelledSubscription (QObject*, const QString&)),
				this,
				SLOT (handleItemCancelledSubscription (QObject*, const QString&)));
		connect (accObject,
				SIGNAL (itemGrantedSubscription (QObject*, const QString&)),
				this,
				SLOT (handleItemGrantedSubscription (QObject*, const QString&)));
		connect (accObject,
				SIGNAL (mucInvitationReceived (QVariantMap, QString, QString)),
				this,
				SLOT (handleMUCInvitation (QVariantMap, QString, QString)));

		connect (accObject,
				SIGNAL (statusChanged (const EntryStatus&)),
				this,
				SLOT (handleAccountStatusChanged (const EntryStatus&)));

		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());
		if (proto)
		{
			const QByteArray& id = proto->GetProtocolID () + account->GetAccountID ();
			const QVariant& var = XmlSettingsManager::Instance ().property (id);
			if (!var.isNull () && var.canConvert<QByteArray> ())
			{
				EntryStatus s;
				QDataStream stream (var.toByteArray ());
				stream >> s;
				account->ChangeState (s);
			}
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "account's parent proto isn't IProtocol"
					<< account->GetParentProtocol ();

		QObject *xferMgr = account->GetTransferManager ();
		if (xferMgr)
		{
			XferJobManager_->AddAccountManager (xferMgr);

			connect (xferMgr,
					SIGNAL (fileOffered (QObject*)),
					this,
					SLOT (handleFileOffered (QObject*)));
		}

		CallManager_->AddAccount (account->GetObject ());

		if (qobject_cast<ISupportRIEX*> (account->GetObject ()))
			connect (account->GetObject (),
					SIGNAL (riexItemsSuggested (QList<LeechCraft::Azoth::RIEXItem>, QObject*, QString)),
					this,
					SLOT (handleRIEXItemsSuggested (QList<LeechCraft::Azoth::RIEXItem>, QObject*, QString)));
	}

	void Core::handleAccountRemoved (QObject *account)
	{
		IAccount *accFace =
				qobject_cast<IAccount*> (account);
				if (!accFace)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< account
					<< sender ();
			return;
		}

		emit accountRemoved (accFace);

		for (int i = 0; i < CLModel_->rowCount (); ++i)
		{
			QStandardItem *item = CLModel_->item (i);
			QObject *obj = item->data (CLRAccountObject).value<QObject*> ();
			if (obj == account)
			{
				ItemIconManager_->Cancel (item);
				CLModel_->removeRow (i);
				break;
			}
		}

		Q_FOREACH (ICLEntry *entry, Entry2Items_.keys ())
			if (entry->GetParentAccount () == account)
				Entry2Items_.remove (entry);
	}

	void Core::handleGotCLItems (const QList<QObject*>& items)
	{
		QMap<const QObject*, QStandardItem*> accountItemCache;
		Q_FOREACH (QObject *item, items)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (item);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< item
						<< "is not a valid ICLEntry";
				continue;
			}

			if (Entry2Items_.contains (entry))
				continue;

			QObject *accountObj = entry->GetParentAccount ();
			if (!accountObj)
			{
				qWarning () << Q_FUNC_INFO
						<< "account object of"
						<< item
						<< "is null";
				continue;
			}

			QStandardItem *accountItem = GetAccountItem (accountObj, accountItemCache);

			if (!accountItem)
			{
				qWarning () << Q_FUNC_INFO
						<< "could not find account item for"
						<< item
						<< accountObj;
				continue;
			}

			AddCLEntry (entry, accountItem);

			if (entry->GetEntryType () & ICLEntry::ETMUC)
			{
				QStandardItem *item = Entry2Items_ [entry].first ();
				OpenChat (CLModel_->indexFromItem (item));
			}

			ChatTabsManager_->HandleEntryAdded (entry);
		}
	}

	void Core::handleRemovedCLItems (const QList<QObject*>& items)
	{
		Q_FOREACH (QObject *clitem, items)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (clitem);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< clitem
						<< "is not a valid ICLEntry";
				continue;
			}

			disconnect (clitem,
					0,
					this,
					0);

			ChatTabsManager_->HandleEntryRemoved (entry);

			Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
				RemoveCLItem (item);

			Entry2Items_.remove (entry);
			Entry2Actions_.remove (entry);

			ID2Entry_.remove (entry->GetEntryID ());

			Entry2SmoothAvatarCache_.remove (entry);
			invalidateClientsIconCache (clitem);
		}
	}

	void Core::handleAccountStatusChanged (const EntryStatus& status)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not an IAccount"
					<< sender ();
			return;
		}

		IProtocol *proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "account's proto is not a IProtocol"
					<< acc->GetParentProtocol ();
			return;
		}

		if (status.State_ == SOffline)
			LastAccountStatusChange_.remove (acc);
		else if (!LastAccountStatusChange_.contains (acc))
			LastAccountStatusChange_ [acc] = QDateTime::currentDateTime ();

		const QByteArray& id = proto->GetProtocolID () + acc->GetAccountID ();
		QByteArray serializedStatus;
		{
			QDataStream stream (&serializedStatus, QIODevice::WriteOnly);
			stream << status;
		}
		XmlSettingsManager::Instance ().setProperty (id,
				serializedStatus);

		for (int i = 0, size = CLModel_->rowCount (); i < size; ++i)
		{
			QStandardItem *item = CLModel_->item (i);
			if (item->data (CLRAccountObject).value<QObject*> () != sender ())
				continue;

			ItemIconManager_->SetIcon (item, GetIconPathForState (status.State_));
			return;
		}

		qWarning () << Q_FUNC_INFO
				<< "item for account"
				<< sender ()
				<< "not found";
	}

	void Core::handleStatusChanged (const EntryStatus& status, const QString& variant)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a ICLEntry:"
					<< sender ();
			return;
		}

		HandleStatusChanged (status, entry, variant, true);
	}

	void Core::handleEntryPEPEvent (const QString&)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a ICLEntry"
					<< sender ();
			return;
		}

		const QString& tip = MakeTooltipString (entry);
		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
			item->setToolTip (tip);
	}

	void Core::handleEntryNameChanged (const QString& newName)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a ICLEntry:"
					<< sender ();
			return;
		}

		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
			item->setText (newName);

		if (entry->Variants ().size ())
			HandleStatusChanged (entry->GetStatus (), entry, entry->Variants ().first ());
	}

	void Core::handleEntryGroupsChanged (QStringList newGroups, QObject *perform)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (perform ? perform : sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "could not be casted to ICLEntry";
			return;
		}

		if (entry->GetEntryType () == ICLEntry::ETChat)
			newGroups = GetDisplayGroups (entry);

		if (!Entry2Items_.contains (entry))
			return;

		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
		{
			const QString& oldCat = item->data (CLREntryCategory).toString ();
			if (newGroups.removeAll (oldCat))
				continue;

			RemoveCLItem (item);
		}

		if (newGroups.isEmpty () && Entry2Items_ [entry].size ())
			return;

		QStandardItem *accItem =
				GetAccountItem (entry->GetParentAccount ());

		QList<QStandardItem*> catItems =
				GetCategoriesItems (newGroups, accItem);
		Q_FOREACH (QStandardItem *catItem, catItems)
			AddEntryTo (entry, catItem);

		HandleStatusChanged (entry->GetStatus (), entry, QString ());
	}

	void Core::handleEntryPermsChanged (ICLEntry *suggest)
	{
		ICLEntry *entry = suggest ? suggest : qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "could not be casted to ICLEntry";
			return;
		}

		QObject *entryObj = entry->GetObject ();
		IMUCPerms *mucPerms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (!mucPerms)
			return;

		const QString& tip = MakeTooltipString (entry);
		const QString& name = mucPerms->GetAffName (entryObj);
		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
		{
			item->setData (name, CLRAffiliation);
			item->setToolTip (tip);
		}
	}

	void Core::handleEntryGotMessage (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());

		if (!other && msg->OtherPart ())
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part cannot be cast to ICLEntry"
					<< msg->OtherPart ();
			return;
		}

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookGotMessage (proxy, msgObj);
		if (proxy->IsCancelled ())
			return;

		if (msg->GetMessageType () != IMessage::MTMUCMessage &&
				msg->GetMessageType () != IMessage::MTChatMessage)
			return;

		ICLEntry *parentCL = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());

		if (ShouldCountUnread (parentCL, msg))
			IncreaseUnreadCount (parentCL);

		if (msg->GetDirection () != IMessage::DIn ||
				ChatTabsManager_->IsActiveChat (parentCL))
			return;

		bool showMsg = XmlSettingsManager::Instance ()
				.property ("ShowMsgInNotifications").toBool ();

		QString msgString;
		bool isHighlightMsg = false;
		switch (msg->GetMessageType ())
		{
		case IMessage::MTChatMessage:
			if (XmlSettingsManager::Instance ()
					.property ("NotifyAboutIncomingMessages").toBool ())
			{
				if (!showMsg)
					msgString = tr ("Incoming chat message from <em>%1</em>.")
							.arg (other->GetEntryName ());
				else
				{
					const QString& body = msg->GetBody ();
					const QString& notifMsg = body.size () > 50 ?
							body.left (50) + "..." :
							body;
					msgString = tr ("Incoming chat message from <em>%1</em>: <em>%2</em>.")
							.arg (other->GetEntryName ())
							.arg (notifMsg);
				}
			}
			break;
		case IMessage::MTMUCMessage:
		{
			isHighlightMsg = IsHighlightMessage (msg);
			if (isHighlightMsg && XmlSettingsManager::Instance ()
					.property ("NotifyAboutConferenceHighlights").toBool ())
			{
				if (!showMsg)
					msgString = tr ("Highlighted in conference <em>%1</em> by <em>%2</em>.")
							.arg (parentCL->GetEntryName ())
							.arg (other->GetEntryName ());
				else
				{
					const QString& body = msg->GetBody ();
					const QString& notifMsg = body.size () > 50 ?
							body.left (50) + "..." :
							body;
					msgString = tr ("Highlighted in conference <em>%1</em> by <em>%2</em>: <em>%3</em>.")
							.arg (parentCL->GetEntryName ())
							.arg (other->GetEntryName ())
							.arg (notifMsg);
				}
			}
			break;
		}
		default:
			return;
		}

		Entity e = Util::MakeNotification ("Azoth",
				msgString,
				PInfo_);

		if (msgString.isEmpty ())
			e.Mime_ += "+advanced";

		QStandardItem *someItem = Entry2Items_ [msg->GetMessageType () == IMessage::MTMUCMessage ?
						parentCL : other].value (0);
		const int count = someItem ?
				someItem->data (CLRUnreadMsgCount).toInt () :
				0;
		if (msg->GetMessageType () == IMessage::MTMUCMessage)
		{
			BuildNotification (e, parentCL);
			e.Additional_ ["org.LC.AdvNotifications.EventType"] = isHighlightMsg ?
					"org.LC.AdvNotifications.IM.MUCHighlightMessage" :
					"org.LC.AdvNotifications.IM.MUCMessage";
			e.Additional_ ["NotificationPixmap"] =
					QVariant::fromValue<QPixmap> (QPixmap::fromImage (other->GetAvatar ()));

			if (isHighlightMsg)
				e.Additional_ ["org.LC.AdvNotifications.FullText"] =
					tr ("%n message(s) from", 0, count) + ' ' + other->GetEntryName () +
							" <em>(" + parentCL->GetEntryName () + ")</em>";
			else
				e.Additional_ ["org.LC.AdvNotifications.FullText"] =
					tr ("%n message(s) in", 0, count) + ' ' + parentCL->GetEntryName ();
		}
		else
		{
			BuildNotification (e, other);
			e.Additional_ ["org.LC.AdvNotifications.EventType"] =
					"org.LC.AdvNotifications.IM.IncomingMessage";
			e.Additional_ ["org.LC.AdvNotifications.FullText"] =
				tr ("%n message(s) from", 0, count) +
						' ' + other->GetEntryName ();
		}

		e.Additional_ ["org.LC.AdvNotifications.Count"] = count;

		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = tr ("%n message(s)", 0, count);
		e.Additional_ ["org.LC.Plugins.Azoth.Msg"] = msg->GetBody ();

		Util::NotificationActionHandler *nh =
				new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open chat"),
				boost::bind (static_cast<QWidget* (ChatTabsManager::*) (const ICLEntry*)> (&ChatTabsManager::OpenChat),
						ChatTabsManager_,
						parentCL));
		nh->AddDependentObject (parentCL->GetObject ());

		emit gotEntity (e);
	}

	namespace
	{
		void AuthorizeEntry (ICLEntry *entry)
		{
			IAccount *account =
					qobject_cast<IAccount*> (entry->GetParentAccount ());
			if (!account)
			{
				qWarning () << Q_FUNC_INFO
						<< "parent account doesn't implement IAccount:"
						<< entry->GetParentAccount ();
				return;
			}
			const QString& id = entry->GetHumanReadableID ();
			account->Authorize (entry->GetObject ());
			account->RequestAuth (id);
		}

		void DenyAuthForEntry (ICLEntry *entry)
		{
			IAccount *account =
					qobject_cast<IAccount*> (entry->GetParentAccount ());
			if (!account)
			{
				qWarning () << Q_FUNC_INFO
						<< "parent account doesn't implement IAccount:"
						<< entry->GetParentAccount ();
				return;
			}
			account->DenyAuth (entry->GetObject ());
		}
	}

	void Core::handleAuthorizationRequested (QObject *entryObj, const QString& msg)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return;
		}

		QString str = msg.isEmpty () ?
				tr ("Subscription requested by %1.")
					.arg (entry->GetEntryName ()) :
				tr ("Subscription requested by %1: %2.")
					.arg (entry->GetEntryName ())
					.arg (msg);
		Entity e = Util::MakeNotification ("Azoth", str, PInfo_);

		BuildNotification (e, entry);
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.EventType"] =
				"org.LC.AdvNotifications.IM.Subscr.Requested";
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = str;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;
		e.Additional_ ["org.LC.Plugins.Azoth.Msg"] = msg;

		Util::NotificationActionHandler *nh =
				new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Authorize"),
				boost::bind (AuthorizeEntry,
						entry));
		nh->AddFunction (tr ("Deny"),
				boost::bind (DenyAuthForEntry,
						entry));
		nh->AddFunction (tr ("View info"),
				boost::bind (&ICLEntry::ShowInfo,
						entry));
		nh->AddDependentObject (entry->GetObject ());
		emit gotEntity (e);
	}

	void Core::handleAttentionDrawn (const QString& text, const QString& variant)
	{
		if (XmlSettingsManager::Instance ()
				.property ("IgnoreDrawAttentions").toBool ())
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry";
			return;
		}

		const QString& str = text.isEmpty () ?
				tr ("%1 requests your attention")
					.arg (entry->GetEntryName ()) :
				tr ("%1 requests your attention: %2")
					.arg (entry->GetEntryName ())
					.arg (text);

		Entity e = Util::MakeNotification ("Azoth", str, PInfo_);
		BuildNotification (e, entry);
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.AttentionDrawnBy/" + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.DeltaCount"] = 1;
		e.Additional_ ["org.LC.AdvNotifications.EventType"] =
				"org.LC.AdvNotifications.IM.AttentionDrawn";
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = tr ("Attention requested");
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = tr ("Attention requested by %1")
				.arg (entry->GetEntryName ());
		e.Additional_ ["org.LC.Plugins.Azoth.Msg"] = text;

		Util::NotificationActionHandler *nh =
				new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open chat"),
				boost::bind (static_cast<QWidget* (ChatTabsManager::*) (const ICLEntry*)> (&ChatTabsManager::OpenChat),
						ChatTabsManager_,
						entry));
		nh->AddDependentObject (entry->GetObject ());

		emit gotEntity (e);
	}

	void Core::handleNicknameConflict (const QString& usedNick)
	{
		ICLEntry *clEntry = qobject_cast<ICLEntry*> (sender ());
		IMUCEntry *entry = qobject_cast<IMUCEntry*> (sender ());
		if (!entry || !clEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry or IMUCEntry";
			return;
		}

		QString altNick;
		if (XmlSettingsManager::Instance ().property ("UseAltNick").toBool ())
		{
			altNick = XmlSettingsManager::Instance ()
				.property ("AlternativeNickname").toString ();
			if (altNick.isEmpty ())
				altNick = usedNick + "_azoth";
		}

		if ((altNick.isEmpty () || altNick == usedNick) &&
				QMessageBox::question (0,
						tr ("Nickname conflict"),
						tr ("You have specified a nickname for %1 that's "
							"already used. Would you like to try to "
							"join with another nick?")
							.arg (clEntry->GetEntryName ()),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const QString& newNick = altNick.isEmpty () || altNick == usedNick ?
				QInputDialog::getText (0,
						tr ("Enter new nick"),
						tr ("Enter new nick for joining %1 (%2 is already used):")
							.arg (clEntry->GetEntryName ())
							.arg (usedNick),
						QLineEdit::Normal,
						usedNick) :
				altNick;
		if (newNick.isEmpty ())
			return;

		entry->SetNick (newNick);
		entry->Join ();
	}

	void Core::handleBeenKicked (const QString& reason)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (sender ());
		if (!entry || !mucEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry or IMUCEntry";
			return;
		}

		const QString& text = reason.isEmpty () ?
				tr ("You have been kicked from %1. Do you want to rejoin?")
					.arg (entry->GetEntryName ()) :
				tr ("You have been kicked from %1: %2. Do you want to rejoin?")
					.arg (entry->GetEntryName ())
					.arg (reason);

		if (QMessageBox::question (0,
				"LeechCraft Azoth",
				text,
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			mucEntry->Join ();
	}

	void Core::handleBeenBanned (const QString& reason)
	{
		ICLEntry* entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry";
			return;
		}

		const QString& text = reason.isEmpty () ?
				tr ("You have been banned from %1.")
					.arg (entry->GetEntryName ()) :
				tr ("You have been banned from %1: %2.")
					.arg (entry->GetEntryName ())
					.arg (reason);
		QMessageBox::warning (0,
				"LeechCraft Azoth",
				text);
	}

	void Core::NotifyWithReason (QObject *entryObj, const QString& msg,
			const char *func, const QString& eventType,
			const QString& patternLite, const QString& patternFull)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << func
					<< entryObj
					<< "doesn't implement ICLEntry";
			return;
		}

		QString str = msg.isEmpty () ?
				patternLite
					.arg (entry->GetEntryName ())
					.arg (entry->GetHumanReadableID ()) :
				patternFull
					.arg (entry->GetEntryName ())
					.arg (entry->GetHumanReadableID ())
					.arg (msg);

		Entity e = Util::MakeNotification ("Azoth", str, PInfo_);
		BuildNotification (e, entry);

		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.Event/" + eventType + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.EventType"] = eventType;
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = str;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;
		e.Additional_ ["org.LC.Plugins.Azoth.Msg"] = msg;

		emit gotEntity (e);
	}

	/** @todo Option for disabling notifications of subscription events.
		*/
	void Core::handleItemSubscribed (QObject *entryObj, const QString& msg)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("NotifySubscriptions").toBool ())
			return;

		NotifyWithReason (entryObj, msg, Q_FUNC_INFO,
				"org.LC.AdvNotifications.IM.Subscr.Subscribed",
				tr ("%1 (%2) subscribed to us."),
				tr ("%1 (%2) subscribed to us: %3."));
	}

	/** @todo Option for disabling notifications of unsubscription events.
		*/
	void Core::handleItemUnsubscribed (QObject *entryObj, const QString& msg)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("NotifyUnsubscriptions").toBool ())
			return;

		NotifyWithReason (entryObj, msg, Q_FUNC_INFO,
				"org.LC.AdvNotifications.IM.Subscr.Unsubscribed",
				tr ("%1 (%2) unsubscribed from us."),
				tr ("%1 (%2) unsubscribed from us: %3."));
	}

	/** @todo Option for disabling notifications of unsubscription events from
		* non-roster items.
		*/
	void Core::handleItemUnsubscribed (const QString& entryId, const QString& msg)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("NotifyAboutNonrosterUnsub").toBool ())
			return;

		QString str = msg.isEmpty () ?
				tr ("%1 unsubscribed from us.")
					.arg (entryId) :
				tr ("%1 unsubscribed from us: %2.")
					.arg (entryId)
					.arg (msg);
		emit gotEntity (Util::MakeNotification ("Azoth", str, PInfo_));
	}

	void Core::handleItemCancelledSubscription (QObject *entryObj, const QString& msg)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("NotifySubCancels").toBool ())
			return;

		NotifyWithReason (entryObj, msg, Q_FUNC_INFO,
				"org.LC.AdvNotifications.IM.Subscr.Revoked",
				tr ("%1 (%2) cancelled our subscription."),
				tr ("%1 (%2) cancelled our subscription: %3."));
	}

	void Core::handleItemGrantedSubscription (QObject *entryObj, const QString& msg)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("NotifySubGrants").toBool ())
			return;

		NotifyWithReason (entryObj, msg, Q_FUNC_INFO,
				"org.LC.AdvNotifications.IM.Subscr.Granted",
				tr ("%1 (%2) granted subscription."),
				tr ("%1 (%2) granted subscription: %3."));
	}

	void Core::handleMUCInvitation (const QVariantMap& ident,
			const QString& inviter, const QString& reason)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement IAccount";
			return;
		}

		const QString& name = ident ["HumanReadableName"].toString ();

		const QString str = reason.isEmpty () ?
				tr ("You have been invited to %1 by %2.")
					.arg (name)
					.arg (inviter) :
				tr ("You have been invited to %1 by %2: %3")
					.arg (name)
					.arg (inviter)
					.arg (reason);

		Entity e = Util::MakeNotification ("Azoth", str, PInfo_);
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] =
				"org.LC.AdvNotifications.IM";
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (name);
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.Invited/" + name + '/' + inviter;
		e.Additional_ ["org.LC.AdvNotifications.EventType"] = "org.LC.AdvNotifications.IM.MUCInvitation";
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = str;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;
		e.Additional_ ["org.LC.Plugins.Azoth.Msg"] = reason;

		Util::NotificationActionHandler *nh = new Util::NotificationActionHandler (e);
		nh->AddFunction (tr ("Join"), boost::bind (&Core::SuggestJoiningMUC,
					this, acc, ident));
		nh->AddDependentObject (acc->GetObject ());

		emit gotEntity (e);
	}

	void Core::updateStatusIconset ()
	{
		QMap<State, QString> State2IconCache_;
		Q_FOREACH (ICLEntry *entry, Entry2Items_.keys ())
		{
			State state = entry->GetStatus ().State_;
			if (!State2IconCache_.contains (state))
				State2IconCache_ [state] = GetIconPathForState (state);

			Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
				ItemIconManager_->SetIcon (item, State2IconCache_ [state]);
		}
	}

	void Core::handleGroupContactsChanged ()
	{
		Q_FOREACH (ICLEntry *entry, Entry2Items_.keys ())
			if (entry->GetEntryType () == ICLEntry::ETChat)
				handleEntryGroupsChanged (GetDisplayGroups (entry), entry->GetObject ());
	}

	void Core::showVCard ()
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender doesn't implement ICLEntry"
					<< sender ();
			return;
		}

		entry->ShowInfo ();
	}

	void Core::updateItem ()
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender doesn't implement ICLEntry"
					<< sender ();
			return;
		}

		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
			item->setText (entry->GetEntryName ());
	}

	void Core::handleClearUnreadMsgCount (QObject *object)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (object);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< object
					<< "doesn't implement ICLEntry";
			return;
		}

		Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
		{
			item->setData (0, CLRUnreadMsgCount);
			RecalculateUnreadForParents (item);
		}

		Entity e = Util::MakeNotification ("Azoth", QString (), PInfo_);
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.IncomingMessageFrom/" + entry->GetEntryID ();
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] =
				"org.LC.AdvNotifications.Cancel";

		emit gotEntity (e);

		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.AttentionDrawnBy/" + entry->GetEntryID ();

		emit gotEntity (e);
	}

	void Core::handleFileOffered (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}

		const QString& id = job->GetSourceID ();
		IncreaseUnreadCount (qobject_cast<ICLEntry*> (GetEntry (id)));

		CheckFileIcon (id);
	}

	void Core::handleJobDeoffered (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}

		const QString& id = job->GetSourceID ();
		IncreaseUnreadCount (qobject_cast<ICLEntry*> (GetEntry (id)), -1);
		CheckFileIcon (id);
	}

	namespace
	{
		void FilterRIEXItems (QList<RIEXItem>& items, const QHash<QString, ICLEntry*>& clEntries)
		{
			Q_FOREACH (const RIEXItem& item, items)
			{
				ICLEntry *entry = clEntries.value (item.ID_);
				if (!entry &&
						(item.Action_ == RIEXItem::AModify ||
						item.Action_ == RIEXItem::ADelete))
				{
					qWarning () << Q_FUNC_INFO
							<< "skipping non-existent"
							<< item.ID_;
					items.removeAll (item);
					continue;
				}

				if (item.Action_ == RIEXItem::ADelete &&
						entry &&
						!item.Groups_.isEmpty ())
				{
					bool found = false;
					const QStringList& origGroups = entry->Groups ();
					Q_FOREACH (const QString& group, item.Groups_)
						if (origGroups.contains (group))
						{
							found = true;
							break;
						}

					if (!found)
						items.removeAll (item);
				}
			}
		}

		void AddRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
		{
			if (!entries.contains (item.ID_))
			{
				acc->RequestAuth (item.ID_, QString (), item.Nick_, item.Groups_);
				return;
			}

			ICLEntry *entry = entries [item.ID_];

			bool allGroups = true;
			Q_FOREACH (const QString& group, item.Groups_)
				if (!entry->Groups ().contains (group))
				{
					allGroups = false;
					break;
				}

			if (!allGroups)
			{
				QStringList newGroups = item.Groups_ + entry->Groups ();
				newGroups.removeDuplicates ();
				entry->SetGroups (newGroups);
			}
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping already-existing"
						<< item.ID_;
				return;
			}
		}

		void ModifyRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
		{
			if (!entries.contains (item.ID_))
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping non-existent"
						<< item.ID_;
				return;
			}

			ICLEntry *entry = entries [item.ID_];

			if (!item.Groups_.isEmpty ())
				entry->SetGroups (item.Groups_);

			if (!item.Nick_.isEmpty ())
				entry->SetEntryName (item.Nick_);
		}

		void DeleteRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
		{
			if (!entries.contains (item.ID_))
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping non-existent"
						<< item.ID_;
				return;
			}

			ICLEntry *entry = entries [item.ID_];
			if (item.Groups_.isEmpty ())
				acc->RemoveEntry (entry->GetObject ());
			else
			{
				QStringList newGroups = entry->Groups ();
				Q_FOREACH (const QString& group, item.Groups_)
					newGroups.removeAll (group);

				entry->SetGroups (newGroups);
			}
		}
	}

	void Core::handleRIEXItemsSuggested (QList<RIEXItem> items, QObject *from, QString message)
	{
		if (items.isEmpty () ||
				!from)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (from);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< from
					<< "doesn't implement ICLEntry";
			return;
		}

		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		QHash<QString, ICLEntry*> clEntries;
		Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry ||
					(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
				continue;

			clEntries [entry->GetHumanReadableID ()] = entry;
		}

		FilterRIEXItems (items, clEntries);

		AcceptRIEXDialog dia (items, from, message);
		if (dia.exec () != QDialog::Accepted)
			return;

		Q_FOREACH (const RIEXItem& item, dia.GetSelectedItems ())
		{
			switch (item.Action_)
			{
			case RIEXItem::AAdd:
				AddRIEX (item, clEntries, acc);
				break;
			case RIEXItem::AModify:
				ModifyRIEX (item, clEntries, acc);
				break;
			case RIEXItem::ADelete:
				DeleteRIEX (item, clEntries, acc);
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown action"
						<< item.Action_
						<< "for item"
						<< item.ID_
						<< item.Nick_
						<< item.Groups_;
				break;
			}
		}
	}

	void Core::invalidateClientsIconCache (QObject *passedObj)
	{
		QObject *obj = passedObj ? passedObj : sender ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "could not be casted to ICLEntry";
			return;
		}

		invalidateClientsIconCache (entry);
	}

	void Core::invalidateClientsIconCache (ICLEntry *entry)
	{
		EntryClientIconCache_.remove (entry);
	}

	void Core::invalidateSmoothAvatarCache ()
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "could not be casted to ICLEntry";
			return;
		}

		Entry2SmoothAvatarCache_.remove (entry);
		updateItem ();
	}

	void Core::handleActionOpenChatTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		ChatTabsManager_->OpenChat (entry);
	}

	void Core::handleActionDrawAttention ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetObject ());
		if (!advEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "doesn't implement IAdvancedCLEntry";
			return;
		}

		const QStringList& vars = entry->Variants ();
		DrawAttentionDialog dia (vars);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& variant = dia.GetResource ();
		const QString& text = dia.GetText ();

		QStringList varsToDraw;
		if (!variant.isEmpty ())
			varsToDraw << variant;
		else if (vars.isEmpty ())
			varsToDraw << QString ();
		else
			varsToDraw = vars;

		Q_FOREACH (const QString& var, varsToDraw)
			advEntry->DrawAttention (text, var);
	}

	void Core::handleActionRenameTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();

		const QString& oldName = entry->GetEntryName ();
		const QString& newName = QInputDialog::getText (0,
				tr ("Rename contact"),
				tr ("Please enter new name for the contact %1:")
					.arg (oldName),
				QLineEdit::Normal,
				oldName);

		if (newName.isEmpty () ||
				oldName == newName)
			return;

		entry->SetEntryName (newName);
	}

	void Core::handleActionChangeGroupsTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();

		const QStringList& groups = entry->Groups ();
		const QStringList& allGroups = GetChatGroups ();

		GroupEditorDialog *dia = new GroupEditorDialog (groups, allGroups);
		if (dia->exec () != QDialog::Accepted)
			return;

		entry->SetGroups (dia->GetGroups ());
	}

	void Core::handleActionRemoveTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IAccount *account =
				qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "doesn't return proper IAccount:"
					<< entry->GetParentAccount ();
			return;
		}

		account->RemoveEntry (entry->GetObject ());
	}

	void Core::handleActionGrantAuthTriggered()
	{
		ManipulateAuth ("grantauth",
				tr ("Enter reason for granting authorization to %1:"),
				&IAuthable::ResendAuth);
	}

	void Core::handleActionRevokeAuthTriggered ()
	{
		ManipulateAuth ("revokeauth",
				tr ("Enter reason for revoking authorization from %1:"),
				&IAuthable::RevokeAuth);
	}

	void Core::handleActionUnsubscribeTriggered ()
	{
		ManipulateAuth ("unsubscribe",
				tr ("Enter reason for unsubscribing from %1:"),
				&IAuthable::Unsubscribe);
	}

	void Core::handleActionRerequestTriggered ()
	{
		ManipulateAuth ("rerequestauth",
				tr ("Enter reason for rerequesting authorization from %1:"),
				&IAuthable::RerequestAuth);
	}

#ifdef ENABLE_CRYPT
	void Core::handleActionManagePGPTriggered ()
	{
		ICLEntry *entry = sender ()->
				property ("Azoth/Entry").value<ICLEntry*> ();

		QObject *accObj = entry->GetParentAccount ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		ISupportPGP *pgp = qobject_cast<ISupportPGP*> (accObj);

		if (!pgp)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "doesn't implement ISupportPGP";
			QMessageBox::warning (0,
					"LeechCraft",
					tr ("The parent account %1 for entry %2 doesn't "
						"support encryption.")
							.arg (acc->GetAccountName ())
							.arg (entry->GetEntryName ()));
			return;
		}

		const QString& str = tr ("Please select the key for %1 (%2).")
				.arg (entry->GetEntryName ())
				.arg (entry->GetHumanReadableID ());
		PGPKeySelectionDialog dia (str, PGPKeySelectionDialog::TPublic);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QCA::PGPKey& key = dia.GetSelectedKey ();

		pgp->SetEntryKey (entry->GetObject (), key);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("PublicEntryKeys");
		if (key.isNull ())
			settings.remove (entry->GetEntryID ());
		else
			settings.setValue (entry->GetEntryID (), key.keyId ());
		settings.endGroup ();
	}
#endif

	void Core::handleActionShareContactsTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();

		ISupportRIEX *riex = qobject_cast<ISupportRIEX*> (entry->GetParentAccount ());
		if (!riex)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetParentAccount ()
					<< "doesn't implement ISupportRIEX";
			return;
		}

		ShareRIEXDialog dia (entry);
		if (dia.exec () != QDialog::Accepted)
			return;

		const bool shareGroups = dia.ShouldSuggestGroups ();

		QList<RIEXItem> items;
		Q_FOREACH (ICLEntry *toShare, dia.GetSelectedEntries ())
		{
			RIEXItem item =
			{
				RIEXItem::AAdd,
				toShare->GetHumanReadableID (),
				toShare->GetEntryName (),
				shareGroups ? toShare->Groups () : QStringList ()
			};
			items << item;
		}

		riex->SuggestItems (items, entry->GetObject (), dia.GetMessage ());
	}

	void Core::handleActionVCardTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		entry->ShowInfo ();
	}

	void Core::handleActionInviteTriggered ()
	{
		ICLEntry *entry = sender ()->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (entry->GetObject ());

		MUCInviteDialog dia (qobject_cast<IAccount*> (entry->GetParentAccount ()));
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& id = dia.GetID ();
		const QString& msg = dia.GetMessage ();
		if (id.isEmpty ())
			return;

		mucEntry->InviteToMUC (id, msg);
	}

	void Core::handleActionLeaveTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (entry->GetObject ());
		if (!mucEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< "hm, requested leave on an entry"
					<< entry->GetObject ()
					<< "that doesn't implement IMUCEntry"
					<< sender ();
			return;
		}

		if (XmlSettingsManager::Instance ().property ("CloseConfOnLeave").toBool ())
		{
			ChatTabsManager_->CloseChat (entry);
			Q_FOREACH (QObject *partObj, mucEntry->GetParticipants ())
			{
				ICLEntry *partEntry = qobject_cast<ICLEntry*> (partObj);
				if (!partEntry)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< partObj
							<< "to ICLEntry";
					continue;
				}

				ChatTabsManager_->CloseChat (partEntry);
			}
		}

		mucEntry->Leave ();
	}

	void Core::handleActionAuthorizeTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		AuthorizeEntry (entry);
	}

	void Core::handleActionDenyAuthTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		DenyAuthForEntry (entry);
	}

	void Core::handleActionAddContactFromMUC ()
	{
		const QString& id = sender ()->property ("Azoth/RealID").toString ();
		if (id.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty ID"
					<< sender ()
					<< sender ()->property ("Azoth/RealID");
			return;
		}

		ICLEntry *entry = sender ()->
				property ("Azoth/Entry").value<ICLEntry*> ();
		const QString& nick = entry->GetEntryName ();

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());

		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (account));
		dia->SetContactID (id);
		dia->SetNick (nick);
		if (dia->exec () != QDialog::Accepted)
			return;

		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}

	void Core::handleActionCopyMUCPartID ()
	{
		const QString& id = sender ()->property ("Azoth/RealID").toString ();
		if (id.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty ID"
					<< sender ()
					<< sender ()->property ("Azoth/RealID");
			return;
		}

		QApplication::clipboard ()->setText (id, QClipboard::Clipboard);
		QApplication::clipboard ()->setText (id, QClipboard::Selection);
	}

	void Core::handleActionPermTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		const QByteArray& permClass = action->property ("Azoth/TargetPermClass").toByteArray ();
		const QByteArray& perm = action->property ("Azoth/TargetPerm").toByteArray ();
		if (permClass.isEmpty () || perm.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid perms set"
					<< action->property ("Azoth/TargetPermClass")
					<< action->property ("Azoth/TargetPerm");
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IMUCPerms *mucPerms =
				qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (!mucPerms)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetParentCLEntry ()
					<< "doesn't implement IMUCPerms";
			return;
		}

		mucPerms->SetPerm (entry->GetObject (), permClass, perm, QString ());
	}

#ifdef ENABLE_CRYPT
	void Core::handleQCAEvent (int id, const QCA::Event& event)
	{
		qDebug () << Q_FUNC_INFO << id << event.type ();
	}

	void Core::handleQCABusyFinished ()
	{
		Q_FOREACH (IAccount *acc, GetAccounts ())
		{
			RestoreKeyForAccount (acc);

			Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
			{
				ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< entry
							<< "doesn't implement ICLEntry";
					continue;
				}

				RestoreKeyForEntry (entry);
			}
		}
	}
#endif
}
}
