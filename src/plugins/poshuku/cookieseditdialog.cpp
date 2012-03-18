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

#include "cookieseditdialog.h"
#include <QPushButton>
#include "cookieseditmodel.h"
#include "cookiesfilter.h"

namespace LeechCraft
{
namespace Poshuku
{
	CookiesEditDialog::CookiesEditDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (false);
	
		Filter_ = new CookiesFilter (this);
		Model_ = new CookiesEditModel (this);
	
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (const QString&)),
				Filter_,
				SLOT (setFilterWildcard (const QString&)));
	
		Filter_->setSourceModel (Model_);
		Ui_.CookiesView_->setModel (Filter_);
		connect (Ui_.CookiesView_,
				SIGNAL (clicked (const QModelIndex&)),
				this,
				SLOT (handleClicked (const QModelIndex&)));
	
		connect (Ui_.ButtonBox_->button (QDialogButtonBox::Apply),
				SIGNAL (released ()),
				this,
				SLOT (handleAccepted ()));
	}
	
	void CookiesEditDialog::handleClicked (const QModelIndex& si)
	{
		QModelIndex index = Filter_->mapToSource (si);
		QNetworkCookie cookie;
		try
		{
			cookie = Model_->GetCookie (index);
		}
		catch (...)
		{
			Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (false);
			return;
		}
	
		Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (true);
		Ui_.DomainEdit_->setEnabled (true);
		Ui_.DomainEdit_->setText (cookie.domain ());
		Ui_.NameEdit_->setEnabled (true);
		Ui_.NameEdit_->setText (cookie.name ());
		Ui_.ExpirationEdit_->setEnabled (true);
		Ui_.ExpirationEdit_->setDateTime (cookie.expirationDate ());
		Ui_.PathEdit_->setEnabled (true);
		Ui_.PathEdit_->setText (cookie.path ());
		Ui_.ValueEdit_->setEnabled (true);
		Ui_.ValueEdit_->setText (cookie.value ());
		Ui_.SecureEdit_->setEnabled (true);
		Ui_.SecureEdit_->setCheckState (cookie.isSecure () ?
				Qt::Checked : Qt::Unchecked);
	}
	
	void CookiesEditDialog::handleAccepted ()
	{
		if (Ui_.RawCookieEdit_->toPlainText ().isEmpty ())
		{
			QNetworkCookie cookie (Ui_.NameEdit_->text ().toUtf8 (),
					Ui_.ValueEdit_->text ().toUtf8 ());
			cookie.setDomain (Ui_.DomainEdit_->text ());
			cookie.setExpirationDate (Ui_.ExpirationEdit_->dateTime ());
			cookie.setPath (Ui_.PathEdit_->text ());
			cookie.setSecure (Ui_.SecureEdit_->checkState () == Qt::Checked);
	
			Model_->SetCookie (Filter_->mapToSource (Ui_.CookiesView_->currentIndex ()),
					cookie);
		}
		else
		{
			Q_FOREACH (QNetworkCookie cookie,
					QNetworkCookie::parseCookies (Ui_.RawCookieEdit_->
						toPlainText ().toUtf8 ()))
				Model_->SetCookie (QModelIndex (), cookie);
	
			Ui_.RawCookieEdit_->clear ();
		}
	}
	
	void CookiesEditDialog::handleDomainChanged ()
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (true);
	}
	
	void CookiesEditDialog::handleNameChanged ()
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (true);
	}
	
	void CookiesEditDialog::on_Delete__released ()
	{
		Model_->RemoveCookie (Filter_->mapToSource (Ui_.CookiesView_->currentIndex ()));
	}
}
}
