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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNTCONFIGURATIONWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNTCONFIGURATIONWIDGET_H
#include <QWidget>
#include <QXmppTransferManager.h>
#include <QXmppConfiguration.h>
#include "ui_glooxaccountconfigurationwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccountConfigurationWidget : public QWidget
	{
		Q_OBJECT

		Ui::GlooxAccountConfigurationWidget Ui_;

		QString Password_;
	public:
		GlooxAccountConfigurationWidget (QWidget* = 0);

		QString GetJID () const;
		void SetJID (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetResource () const;
		void SetResource (const QString&);
		short GetPriority () const;
		void SetPriority (short);

		QString GetHost () const;
		void SetHost (const QString&);
		int GetPort () const;
		void SetPort (int);

		int GetKAInterval () const;
		void SetKAInterval (int);
		int GetKATimeout () const;
		void SetKATimeout (int);

		bool GetFileLogEnabled () const;
		void SetFileLogEnabled (bool);

		QXmppConfiguration::StreamSecurityMode GetTLSMode () const;
		void SetTLSMode (QXmppConfiguration::StreamSecurityMode);

		QXmppTransferJob::Methods GetFTMethods () const;
		void SetFTMethods (QXmppTransferJob::Methods);

		bool GetUseSOCKS5Proxy () const;
		void SetUseSOCKS5Proxy (bool);

		QString GetSOCKS5Proxy () const;
		void SetSOCKS5Proxy (const QString&);

		QString GetPassword () const;
	private slots:
		void on_UpdatePassword__released ();
	};
}
}
}

#endif
