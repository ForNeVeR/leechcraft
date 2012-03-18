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
#include "startupfirstpage.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Aggregator
{
	StartupFirstPage::StartupFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		setTitle ("Aggregator");
		setSubTitle (tr ("Set default options"));
	}

	void StartupFirstPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()),
				Qt::UniqueConnection);
		XmlSettingsManager::Instance ()->
				setProperty ("StartupVersion", 1);
	}

	void StartupFirstPage::handleAccepted ()
	{
		XmlSettingsManager::Instance ()->setProperty ("ShowIconInTray",
				Ui_.ShowIconInTray_->isChecked ());
		XmlSettingsManager::Instance ()->setProperty ("UpdateInterval",
				Ui_.UpdateInterval_->value ());
		XmlSettingsManager::Instance ()->setProperty ("ItemsPerChannel",
				Ui_.ItemsPerChannel_->value ());
		XmlSettingsManager::Instance ()->setProperty ("ItemsMaxAge",
				Ui_.ItemsMaxAge_->value ());
	}
}
}
