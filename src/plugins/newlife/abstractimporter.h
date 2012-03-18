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

#pragma once

#include <QObject>
#include <QIcon>

class QWizardPage;

namespace LeechCraft
{
struct Entity;

namespace NewLife
{
	class AbstractImporter : public QObject
	{
		Q_OBJECT
	public:
		AbstractImporter (QObject* = 0);

		virtual QList<QIcon> GetIcons () const;
		virtual QStringList GetNames () const = 0;
		virtual QList<QWizardPage*> GetWizardPages () const = 0;
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
