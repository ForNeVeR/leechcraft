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

#include "networkaccessmanager.h"
#include <stdexcept>
#include <algorithm>
#include <QNetworkRequest>
#include <QDir>
#include <QFile>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QSettings>
#include <util/util.h>
#include <util/customcookiejar.h>
#include <util/defaulthookproxy.h>
#include "core.h"
#include "networkdiskcache.h"
#include "authenticationdialog.h"
#include "sslerrorsdialog.h"
#include "xmlsettingsmanager.h"
#include "mainwindow.h"
#include "config.h"
#include "storagebackend.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

NetworkAccessManager::NetworkAccessManager (QObject *parent)
: QNetworkAccessManager (parent)
, CookieSaveTimer_ (new QTimer ())
{
	connect (this,
			SIGNAL (authenticationRequired (QNetworkReply*,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (QNetworkReply*,
					QAuthenticator*)));
	connect (this,
			SIGNAL (proxyAuthenticationRequired (const QNetworkProxy&,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (const QNetworkProxy&,
					QAuthenticator*)));
	connect (this,
			SIGNAL (sslErrors (QNetworkReply*,
					const QList<QSslError>&)),
			this,
			SLOT (handleSslErrors (QNetworkReply*,
					const QList<QSslError>&)));

	XmlSettingsManager::Instance ()->RegisterObject ("FilterTrackingCookies",
			this,
			"handleFilterTrackingCookies");

	CustomCookieJar *jar = new CustomCookieJar (this);
	setCookieJar (jar);
	handleFilterTrackingCookies ();

	try
	{
		CreateIfNotExists ("core/cache");
		NetworkDiskCache *cache = new NetworkDiskCache (this);
		setCache (cache);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< "so continuing without cache";
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (file.open (QIODevice::ReadOnly))
		jar->Load (file.readAll ());
	else
		qWarning () << Q_FUNC_INFO
			<< "could not open file"
			<< file.fileName ()
			<< file.errorString ();

	connect (CookieSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);

	QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
	settings.beginGroup ("NAMLocales");
	int size = settings.beginReadArray ("Locales");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		Locales_ << QLocale (settings.value ("LocaleName").toString ());
	}
	settings.endArray ();
	settings.endGroup ();
}

NetworkAccessManager::~NetworkAccessManager ()
{
	CustomCookieJar *jar = static_cast<CustomCookieJar*> (cookieJar ());
	if (!jar)
	{
		qWarning () << Q_FUNC_INFO
			<< "jar is NULL";
		return;
	}
	else
	{
		jar->CollectGarbage ();
		saveCookies ();
	}
}

QList<QLocale> NetworkAccessManager::GetAcceptLangs () const
{
	return Locales_;
}

void NetworkAccessManager::SetAcceptLangs (const QList<QLocale>& locales)
{
	Locales_ = locales;

	QStringList localesStrs;
	std::transform (locales.begin (), locales.end (), std::back_inserter (localesStrs), Util::GetInternetLocaleName);
	LocaleStr_ = localesStrs.join (", ");

	emit acceptableLanguagesChanged ();

	QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
	settings.beginGroup ("NAMLocales");
	settings.beginWriteArray ("Locales");
	settings.remove ("");
	for (int i = 0; i < Locales_.size (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("LocaleName", Locales_.at (i).name ());
	}
	settings.endArray ();
	settings.endGroup ();
}

QNetworkReply* NetworkAccessManager::createRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QIODevice *out)
{
	QNetworkRequest r = req;

	DefaultHookProxy_ptr proxy (new DefaultHookProxy);
	proxy->SetValue ("request", QVariant::fromValue<QNetworkRequest> (r));
	emit hookNAMCreateRequest (proxy, this, &op, &out);

	if (proxy->IsCancelled ())
		return proxy->GetReturnValue ().value<QNetworkReply*> ();

	proxy->FillValue ("request", r);

	if (r.url ().scheme ().startsWith ("http") && !LocaleStr_.isEmpty ())
		r.setRawHeader ("Accept-Language", LocaleStr_.toUtf8 ());

	QNetworkReply *result = QNetworkAccessManager::createRequest (op, r, out);
	emit requestCreated (op, r, result);
	return result;
}

void LeechCraft::NetworkAccessManager::DoCommonAuth (const QString& msg, QAuthenticator *authen)
{
	QString realm = authen->realm ();

	QString suggestedUser = authen->user ();
	QString suggestedPassword = authen->password ();

	StorageBackend *backend = Core::Instance ().GetStorageBackend ();

	if (suggestedUser.isEmpty ())
		backend->GetAuth (realm, suggestedUser, suggestedPassword);

	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				suggestedUser,
				suggestedPassword,
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	QString login = dia->GetLogin ();
	QString password = dia->GetPassword ();
	authen->setUser (login);
	authen->setPassword (password);

	if (dia->ShouldSave ())
		backend->SetAuth (realm, login, password);
}

void LeechCraft::NetworkAccessManager::handleAuthentication (QNetworkReply *reply,
		QAuthenticator *authen)
{
	QString msg = tr ("%1<br /><em>%2</em><br />requires authentication.")
		.arg (authen->realm ())
		.arg (QApplication::fontMetrics ()
				.elidedText (reply->url ().toString (),
						Qt::ElideMiddle, ELIDED_URL_WIDTH));

	DoCommonAuth (msg, authen);
}

void LeechCraft::NetworkAccessManager::handleAuthentication (const QNetworkProxy& proxy,
		QAuthenticator *authen)
{
	QString msg = tr ("%1<br /><em>%2</em><br />requires authentication.")
		.arg (authen->realm ())
		.arg (proxy.hostName ());

	DoCommonAuth (msg, authen);
}

void LeechCraft::NetworkAccessManager::handleSslErrors (QNetworkReply *reply,
		const QList<QSslError>& errors)
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("SSL exceptions");
	QStringList keys = settings.allKeys ();
	if (keys.contains (reply->url ().toString ()))
	{
		if (settings.value (reply->url ().toString ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else if (keys.contains (reply->url ().host ()))
	{
		if (settings.value (reply->url ().host ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else
	{
		QUrl url = reply->url ();
		QPointer<QNetworkReply> repGuarded (reply);
		QString msg = tr ("<code>%1</code><br />has SSL errors."
				" What do you want to do?")
			.arg (QApplication::fontMetrics ().elidedText(url.toString (), Qt::ElideMiddle, ELIDED_URL_WIDTH));

		std::auto_ptr<SslErrorsDialog> errDialog (new SslErrorsDialog ());
		errDialog->Update (msg, errors);

		bool ignore = (errDialog->exec () == QDialog::Accepted);
		SslErrorsDialog::RememberChoice choice = errDialog->GetRememberChoice ();

		if (choice != SslErrorsDialog::RCNot)
		{
			if (choice == SslErrorsDialog::RCFile)
				settings.setValue (url.toString (),
						ignore);
			else
				settings.setValue (url.host (),
						ignore);
		}

		if (ignore)
		{
			if (repGuarded)
				repGuarded->ignoreSslErrors ();
			else
				qWarning () << Q_FUNC_INFO
						<< "reply destructed while in errors dialog";
		}
	}
	settings.endGroup ();
}

void LeechCraft::NetworkAccessManager::saveCookies () const
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	if (!dir.exists ("core") &&
			!dir.mkdir ("core"))
	{
		emit error (tr ("Could not create Core directory."));
		return;
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		emit error (tr ("Could not save cookies, error opening cookie file."));
		qWarning () << Q_FUNC_INFO
			<< file.errorString ();
	}
	else
	{
		CustomCookieJar *jar = static_cast<CustomCookieJar*> (cookieJar ());
		if (!jar)
		{
			qWarning () << Q_FUNC_INFO
				<< "jar is NULL";
			return;
		}
		file.write (jar->Save ());
	}
}

void LeechCraft::NetworkAccessManager::handleFilterTrackingCookies ()
{
	qobject_cast<CustomCookieJar*> (cookieJar ())->
		SetFilterTrackingCookies (XmlSettingsManager::Instance ()->
				property ("FilterTrackingCookies").toBool ());
}

