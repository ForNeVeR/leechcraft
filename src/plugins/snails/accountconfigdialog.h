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

#ifndef PLUGINS_SNAILS_ACCOUNTCONFIGDIALOG_H
#define PLUGINS_SNAILS_ACCOUNTCONFIGDIALOG_H
#include <QDialog>
#include "ui_accountconfigdialog.h"
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	class AccountConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountConfigDialog Ui_;
	public:
		AccountConfigDialog (QWidget* = 0);

		QString GetName () const;
		void SetName (const QString&);

		QString GetLogin () const;
		void SetLogin (const QString&);

		Account::InType GetInType () const;
		void SetInType (Account::InType);

		QString GetInHost () const;
		void SetInHost (const QString&);

		int GetInPort () const;
		void SetInPort (int);

		Account::OutType GetOutType () const;
		void SetOutType (Account::OutType);

		QString GetOutHost () const;
		void SetOutHost (const QString&);

		int GetOutPort () const;
		void SetOutPort (int);

		QString GetOutLogin () const;
		void SetOutLogin (const QString&);

		bool GetUseSASL () const;
		void SetUseSASL (bool);

		bool GetSASLRequired () const;
		void SetSASLRequired (bool);

		bool GetUseTLS () const;
		void SetUseTLS (bool);

		bool GetTLSRequired () const;
		void SetTLSRequired (bool);

		bool GetSMTPAuth () const;
		void SetSMTPAuth (bool);

		bool GetAPOP () const;
		void SetAPOP (bool);

		bool GetAPOPRequired () const;
		void SetAPOPRequired (bool);
	private slots:
		void resetInPort ();
	};
}
}

#endif
