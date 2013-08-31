/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "clientconnectionerrormgr.h"
#include <QTimer>
#include <util/util.h>
#include <util/socketerrorstrings.h>
#include "clientconnection.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const int ErrorLimit = 5;

	ClientConnectionErrorMgr::ClientConnectionErrorMgr (ClientConnection *conn)
	: QObject (conn)
	, ClientConn_ (conn)
	, Client_ (ClientConn_->GetClient ())
	, SocketErrorAccumulator_ (0)
	{
		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleError (QXmppClient::Error)));

		QTimer *decrTimer = new QTimer (this);
		connect (decrTimer,
				SIGNAL (timeout ()),
				this,
				SLOT (decrementErrAccumulators ()));
		decrTimer->start (15000);
	}

	void ClientConnectionErrorMgr::Whitelist (const QString& id, bool add)
	{
		if (add && !id.isEmpty ())
			WhitelistedErrors_ << id;
	}

	void ClientConnectionErrorMgr::HandleIq (const QXmppIq& iq)
	{
		switch (iq.error ().type ())
		{
		case QXmppStanza::Error::Cancel:
		case QXmppStanza::Error::Continue:
		case QXmppStanza::Error::Modify:
		case QXmppStanza::Error::Auth:
		case QXmppStanza::Error::Wait:
			HandleError (iq);
			break;
		default:
			WhitelistedErrors_.remove (iq.id ());
			break;
		}
	}

	QString ClientConnectionErrorMgr::HandleErrorCondition (const QXmppStanza::Error::Condition& condition)
	{
		switch (condition)
		{
		case QXmppStanza::Error::BadRequest:
			return tr ("Bad request.");
		case QXmppStanza::Error::Conflict:
			return tr ("Conflict (possibly, resource conflict).");
		case QXmppStanza::Error::FeatureNotImplemented:
			return tr ("Feature not implemented.");
		case QXmppStanza::Error::Forbidden:
			return tr ("Forbidden.");
			//case QXmppStanza::Error::Gone:
		case QXmppStanza::Error::InternalServerError:
			return tr ("Internal server error.");
		case QXmppStanza::Error::ItemNotFound:
			return tr ("Item not found.");
		case QXmppStanza::Error::JidMalformed:
			return tr ("JID is malformed.");
		case QXmppStanza::Error::NotAcceptable:
			return tr ("Data is not acceptable.");
		case QXmppStanza::Error::NotAllowed:
			return tr ("Action is not allowed.");
		case QXmppStanza::Error::NotAuthorized:
			emit serverAuthFailed ();
			return tr ("Not authorized.");
		case QXmppStanza::Error::PaymentRequired:
			return tr ("Payment required.");
		case QXmppStanza::Error::RecipientUnavailable:
			return tr ("Recipient unavailable.");
		case QXmppStanza::Error::Redirect:
			return tr ("Got redirect.");
		case QXmppStanza::Error::RegistrationRequired:
			return tr ("Registration required.");
		case QXmppStanza::Error::RemoteServerNotFound:
			return tr ("Remote server not found.");
		case QXmppStanza::Error::RemoteServerTimeout:
			return tr ("Timeout contacting remote server.");
		case QXmppStanza::Error::ResourceConstraint:
			return tr ("Error due to resource constraint.");
		case QXmppStanza::Error::ServiceUnavailable:
			return tr ("Service is unavailable at the moment.");
		case QXmppStanza::Error::SubscriptionRequired:
			return tr ("Subscription is required to perform this action.");
			//case QXmppStanza::Error::UndefinedCondition:
			//case QXmppStanza::Error::UnexpectedRequest:
		default:
			return tr ("Other error.");
		}
	}

	void ClientConnectionErrorMgr::HandleError (const QXmppIq& iq)
	{
		const QXmppStanza::Error& error = iq.error ();
		if (!WhitelistedErrors_.remove (iq.id ()))
			switch (error.condition ())
			{
			case QXmppStanza::Error::FeatureNotImplemented:
			case QXmppStanza::Error::ItemNotFound:
			case QXmppStanza::Error::ServiceUnavailable:
				return;
			default:
				break;
			}

		QString typeText;
		if (!iq.from ().isEmpty ())
			typeText = tr ("Error from %1: ")
					.arg (iq.from ());
		typeText += HandleErrorCondition (error.condition ());

		if (!error.text ().isEmpty ())
			typeText += " " + tr ("Error text: %1.")
					.arg (error.text ());

		const Entity& e = Util::MakeNotification ("Azoth",
				typeText,
				PCritical_);
		Core::Instance ().SendEntity (e);

		const bool dontTryFurther = error.type () == QXmppStanza::Error::Cancel ||
			(error.type () == QXmppStanza::Error::Auth &&
			 error.condition () != QXmppStanza::Error::NotAuthorized);
		if (dontTryFurther && !ClientConn_->GetClient ()->isConnected ())
		{
			GlooxAccountState state =
			{
				SOffline,
				QString (),
				0
			};
			ClientConn_->SetState (state);
		}
	}

	void ClientConnectionErrorMgr::handleError (QXmppClient::Error error)
	{
		if (ClientConn_->GetLastState ().State_ == SOffline &&
				error == QXmppClient::SocketError)
		{
			qDebug () << Q_FUNC_INFO
					<< "killing stale error";
			Client_->disconnectFromServer ();
			return;
		}

		QString str;
		switch (error)
		{
		case QXmppClient::SocketError:
			if (SocketErrorAccumulator_ < ErrorLimit)
			{
				++SocketErrorAccumulator_;
				str = tr ("socket error: %1.")
						.arg (Util::GetSocketErrorString (Client_->socketError ()));
			}
			break;
		case QXmppClient::KeepAliveError:
			str = tr ("keep-alive error.");
			break;
		case QXmppClient::XmppStreamError:
			str = tr ("error while connecting: ");
			str += HandleErrorCondition (Client_->xmppStreamError ());
			break;
		case QXmppClient::NoError:
			str = tr ("no error.");
			break;
		}

		if (str.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "suppressed"
					<< str
					<< error
					<< Client_->socketError ()
					<< Client_->xmppStreamError ();
			return;
		}

		const Entity& e = Util::MakeNotification ("Azoth",
				tr ("Account %1:").arg (ClientConn_->GetOurJID ()) +
					' ' + str,
				PCritical_);
		Core::Instance ().SendEntity (e);
	}

	void ClientConnectionErrorMgr::decrementErrAccumulators ()
	{
		if (SocketErrorAccumulator_ > 0)
			--SocketErrorAccumulator_;
	}
}
}
}
