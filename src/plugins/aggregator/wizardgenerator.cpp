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

#include "wizardgenerator.h"
#include "xmlsettingsmanager.h"
#include "startupfirstpage.h"
#include "startupsecondpage.h"
#include "startupthirdpage.h"

namespace LeechCraft
{
namespace Aggregator
{
	QList<QWizardPage*> WizardGenerator::GetPages ()
	{
		QList<QWizardPage*> result;
		int version = XmlSettingsManager::Instance ()->
			Property ("StartupVersion", 0).toInt ();
		if (version <= 0)
			result << new StartupFirstPage ();
		if (version <= 1)
			result << new StartupSecondPage ();
		if (version <= 2)
			result << new StartupThirdPage ();
		return result;
	}
}
}
