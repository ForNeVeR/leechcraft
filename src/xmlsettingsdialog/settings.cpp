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

#include "settings.h"
#include <QCoreApplication>

using namespace LeechCraft;

Settings::Settings ()
{
#ifdef Q_OS_WIN32
	ConfigDir_ = QCoreApplication::applicationDirPath () + "/leechcraft/";
#elif defined (Q_OS_MAC)
	ConfigDir_ = QCoreApplication::applicationDirPath () + "/../Resources/config";
#else
	ConfigDir_ = "/etc/leechcraft/";
#endif

	ThemesConfigDir_ = ConfigDir_ + "themes/";
}

const QString& Settings::GetConfigDir () const
{
	return ConfigDir_;
}

const QString& Settings::GetThemesConfigDir () const
{
	return ThemesConfigDir_;
}

