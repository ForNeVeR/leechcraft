/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "accountsettingsholder.h"
#include <QTimer>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccountconfigurationdialog.h"
#include "glooxaccount.h"
#include "core.h"
#include "accstatusrestorer.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	AccountSettingsHolder::AccountSettingsHolder (GlooxAccount *parent)
	: QObject (parent)
	, ReconnectScheduled_ (false)
	, Account_ (parent)
	, Port_ (-1)
	, KAParams_ (qMakePair (90, 60))
	, FileLogEnabled_ (false)
	, Priority_ (5)
	, TLSMode_ (QXmppConfiguration::TLSEnabled)
	, FTMethods_ (QXmppTransferJob::AnyMethod)
	, UseSOCKS5Proxy_ (false)
	, StunPort_ (3478)
	, TurnPort_ (3478)
	{
		connect (this,
				SIGNAL (jidChanged (QString)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (resourceChanged (QString)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (hostChanged (QString)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (portChanged (int)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (tlsModeChanged (QXmppConfiguration::StreamSecurityMode)),
				this,
				SLOT (scheduleReconnect ()));
	}

	void AccountSettingsHolder::Serialize (QDataStream& ostr) const
	{
		ostr << JID_
			<< Nick_
			<< Resource_
			<< Priority_
			<< Host_
			<< Port_
			<< KAParams_
			<< OurPhotoHash_
			<< FileLogEnabled_
			<< static_cast<bool> (FTMethods_ & QXmppTransferJob::InBandMethod)
			<< static_cast<bool> (FTMethods_ & QXmppTransferJob::SocksMethod)
			<< UseSOCKS5Proxy_
			<< SOCKS5Proxy_
			<< static_cast<quint8> (TLSMode_)
			<< StunHost_
			<< StunPort_
			<< TurnHost_
			<< TurnPort_
			<< TurnUser_
			<< TurnPass_;
	}

	void AccountSettingsHolder::Deserialize (QDataStream& in, quint16 version)
	{
		in >> JID_
			>> Nick_
			>> Resource_
			>> Priority_;
		if (version >= 2)
			in >> Host_
				>> Port_;
		if (version >= 3)
			in >> KAParams_;
		if (version >= 4)
			in >> OurPhotoHash_;
		if (version >= 5)
			in >> FileLogEnabled_;
		if (version >= 6)
		{
			bool useIBB = true, useSocks = true;
			in >> useIBB
				>> useSocks
				>> UseSOCKS5Proxy_
				>> SOCKS5Proxy_;

			FTMethods_ = QXmppTransferJob::NoMethod;
			if (useIBB)
				FTMethods_ |= QXmppTransferJob::InBandMethod;
			if (useSocks)
				FTMethods_ |= QXmppTransferJob::SocksMethod;
		}
		if (version >= 7)
		{
			quint8 mode = 0;
			in >> mode;
			TLSMode_ = static_cast<decltype (TLSMode_)> (mode);
		}
		if (version >= 8)
		{
			in >> StunHost_
				>> StunPort_
				>> TurnHost_
				>> TurnPort_
				>> TurnUser_
				>> TurnPass_;
		}
		if (version >= 9)
			in >> EnableMessageCarbons_;
	}

	void AccountSettingsHolder::OpenConfigDialog ()
	{
		GlooxAccountConfigurationDialog dia;
		if (!JID_.isEmpty ())
			dia.W ()->SetJID (JID_);
		if (!Nick_.isEmpty ())
			dia.W ()->SetNick (Nick_);
		if (!Resource_.isEmpty ())
			dia.W ()->SetResource (Resource_);
		if (!Host_.isEmpty ())
			dia.W ()->SetHost (Host_);
		if (Port_ >= 0)
			dia.W ()->SetPort (Port_);
		dia.W ()->SetPriority (Priority_);

		dia.W ()->SetKAInterval (KAParams_.first);
		dia.W ()->SetKATimeout (KAParams_.second);

		dia.W ()->SetFileLogEnabled (FileLogEnabled_);

		dia.W ()->SetFTMethods (GetFTMethods ());
		dia.W ()->SetUseSOCKS5Proxy (GetUseSOCKS5Proxy ());
		dia.W ()->SetSOCKS5Proxy (GetSOCKS5Proxy ());
		dia.W ()->SetTLSMode (GetTLSMode ());

		dia.W ()->SetStunServer (StunHost_);
		dia.W ()->SetStunPort (StunPort_);
		dia.W ()->SetTurnServer (TurnHost_);
		dia.W ()->SetTurnPort (TurnPort_);
		dia.W ()->SetTurnUser (TurnUser_);
		dia.W ()->SetTurnPassword (TurnPass_);

		if (dia.exec () == QDialog::Rejected)
			return;

		FillSettings (dia.W ());
	}

	void AccountSettingsHolder::FillSettings (GlooxAccountConfigurationWidget *w)
	{
		SetJID (w->GetJID ());
		SetNick (w->GetNick ());
		SetResource (w->GetResource ());
		SetPriority (w->GetPriority ());
		SetHost (w->GetHost ());
		SetPort (w->GetPort ());
		SetFileLogEnabled (w->GetFileLogEnabled ());
		SetFTMethods (w->GetFTMethods ());
		SetUseSOCKS5Proxy (w->GetUseSOCKS5Proxy ());
		SetSOCKS5Proxy (w->GetSOCKS5Proxy ());
		SetTLSMode (w->GetTLSMode ());
		SetStunParams (w->GetStunServer (), w->GetStunPort ());
		SetTurnParams (w->GetTurnServer (), w->GetTurnPort (),
				w->GetTurnUser (), w->GetTurnPassword ());

		const QString& pass = w->GetPassword ();
		if (!pass.isNull ())
			Account_->GetParentProtocol ()->GetProxyObject ()->SetPassword (pass, Account_);

		SetKAParams (qMakePair (w->GetKAInterval (), w->GetKATimeout ()));

		emit accountSettingsChanged ();
	}

	QString AccountSettingsHolder::GetJID () const
	{
		return JID_;
	}

	void AccountSettingsHolder::SetJID (const QString& jid)
	{
		if (jid == JID_)
			return;

		JID_ = jid;
		emit jidChanged (JID_);
	}

	QString AccountSettingsHolder::GetNick () const
	{
		return Nick_.isEmpty () ? JID_ : Nick_;
	}

	void AccountSettingsHolder::SetNick (const QString& nick)
	{
		if (nick == Nick_)
			return;

		Nick_ = nick;
		emit nickChanged (Nick_);
	}

	QString AccountSettingsHolder::GetResource () const
	{
		return Resource_;
	}

	void AccountSettingsHolder::SetResource (const QString& resource)
	{
		if (resource == Resource_)
			return;

		Resource_ = resource;
		emit resourceChanged (Resource_);
	}

	QString AccountSettingsHolder::GetFullJID () const
	{
		return JID_ + "/" + Resource_;
	}

	QString AccountSettingsHolder::GetHost () const
	{
		return Host_;
	}

	void AccountSettingsHolder::SetHost (const QString& host)
	{
		if (host == Host_)
			return;

		Host_ = host;
		emit hostChanged (Host_);
	}

	int AccountSettingsHolder::GetPort () const
	{
		return Port_;
	}

	void AccountSettingsHolder::SetPort (int port)
	{
		if (port == Port_)
			return;

		Port_ = port;
		emit portChanged (Port_);
	}

	QByteArray AccountSettingsHolder::GetPhotoHash () const
	{
		return OurPhotoHash_;
	}

	void AccountSettingsHolder::SetPhotoHash (const QByteArray& hash)
	{
		if (hash == OurPhotoHash_)
			return;

		OurPhotoHash_ = hash;
		emit photoHashChanged (hash);
		emit accountSettingsChanged ();
	}

	QPair<int, int> AccountSettingsHolder::GetKAParams () const
	{
		return KAParams_;
	}

	void AccountSettingsHolder::SetKAParams (const QPair<int, int>& params)
	{
		if (params == KAParams_)
			return;

		KAParams_ = params;
		emit kaParamsChanged (KAParams_);
	}

	bool AccountSettingsHolder::GetFileLogEnabled () const
	{
		return FileLogEnabled_;
	}

	void AccountSettingsHolder::SetFileLogEnabled (bool enabled)
	{
		if (enabled == FileLogEnabled_)
			return;

		FileLogEnabled_ = enabled;
		emit fileLogChanged (FileLogEnabled_);
	}

	int AccountSettingsHolder::GetPriority () const
	{
		return Priority_;
	}

	void AccountSettingsHolder::SetPriority (int prio)
	{
		if (prio == Priority_)
			return;

		Priority_ = prio;
		emit priorityChanged (Priority_);
	}

	QXmppTransferJob::Methods AccountSettingsHolder::GetFTMethods () const
	{
		return FTMethods_;
	}

	void AccountSettingsHolder::SetFTMethods (QXmppTransferJob::Methods methods)
	{
		if (methods == FTMethods_)
			return;

		FTMethods_ = methods;
		emit fileTransferSettingsChanged ();
	}

	QXmppConfiguration::StreamSecurityMode AccountSettingsHolder::GetTLSMode () const
	{
		return TLSMode_;
	}

	void AccountSettingsHolder::SetTLSMode (QXmppConfiguration::StreamSecurityMode mode)
	{
		if (mode == TLSMode_)
			return;

		TLSMode_ = mode;
		emit tlsModeChanged (TLSMode_);
	}

	bool AccountSettingsHolder::GetUseSOCKS5Proxy () const
	{
		return UseSOCKS5Proxy_;
	}

	void AccountSettingsHolder::SetUseSOCKS5Proxy (bool use)
	{
		if (use == UseSOCKS5Proxy_)
			return;

		UseSOCKS5Proxy_ = use;
		emit fileTransferSettingsChanged ();
	}

	QString AccountSettingsHolder::GetSOCKS5Proxy () const
	{
		return SOCKS5Proxy_;
	}

	void AccountSettingsHolder::SetSOCKS5Proxy (const QString& proxy)
	{
		if (proxy == SOCKS5Proxy_)
			return;

		SOCKS5Proxy_ = proxy;
		emit fileTransferSettingsChanged ();
	}

	QString AccountSettingsHolder::GetStunHost () const
	{
		return StunHost_;
	}

	int AccountSettingsHolder::GetStunPort () const
	{
		return StunPort_;
	}

	void AccountSettingsHolder::SetStunParams (const QString& host, int port)
	{
		if (host == StunHost_ && port == StunPort_)
			return;

		StunHost_ = host;
		StunPort_ = port;

		emit stunSettingsChanged ();
	}

	QString AccountSettingsHolder::GetTurnHost () const
	{
		return TurnHost_;
	}

	int AccountSettingsHolder::GetTurnPort () const
	{
		return TurnPort_;
	}

	QString AccountSettingsHolder::GetTurnUser () const
	{
		return TurnUser_;
	}

	QString AccountSettingsHolder::GetTurnPass () const
	{
		return TurnPass_;
	}

	void AccountSettingsHolder::SetTurnParams (const QString& host, int port, const QString& user, const QString& pass)
	{
		if (host == TurnHost_ && port == TurnPort_ && user == TurnUser_ && pass == TurnPass_)
			return;

		TurnHost_ = host;
		TurnPort_ = port;
		TurnUser_ = user;
		TurnPass_ = pass;
		emit turnSettingsChanged ();
	}

	bool AccountSettingsHolder::IsMessageCarbonsEnabled () const
	{
		return EnableMessageCarbons_;
	}

	void AccountSettingsHolder::SetMessageCarbonsEnabled (bool enabled)
	{
		if (EnableMessageCarbons_ == enabled)
			return;

		EnableMessageCarbons_ = enabled;
		emit accountSettingsChanged ();

		emit messageCarbonsSettingsChanged ();
	}

	void AccountSettingsHolder::scheduleReconnect ()
	{
		if (ReconnectScheduled_)
			return;

		ReconnectScheduled_ = true;
		QTimer::singleShot (10,
				this,
				SLOT (handleReconnect ()));
	}

	void AccountSettingsHolder::handleReconnect ()
	{
		ReconnectScheduled_ = false;

		if (Account_->GetState ().State_ == SOffline)
			return;

		auto conn = Account_->GetClientConnection ();
		if (!conn)
			return;

		const auto state = conn->GetLastState ();
		Account_->ChangeState (EntryStatus (SOffline, Account_->GetState ().StatusString_));
		conn->SetOurJID (GetFullJID ());
		new AccStatusRestorer (state, conn);
	}
}
}
}
