/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <QHash>

namespace LeechCraft
{
namespace KBSwitch
{
	class KeyboardLayoutSwitcher : public QObject
	{
		Q_OBJECT

		enum SwitchingPolicy
		{
			Global,
			Plugin,
			Tab
		};

		SwitchingPolicy CurrentSwitchingPloicy_;

		QHash<QWidget*, int> Widget2KBLayoutIndex_;
		QHash<QByteArray, int> TabClass2KBLayoutIndex_;

		QWidget *LastCurrentWidget_;
	public:
		KeyboardLayoutSwitcher (QObject *parent = 0);

		bool IsGlobalPolicy () const;
	public slots:
		void updateKBLayouts (QWidget *current, QWidget *prev);
	private slots:
		void setSwitchingPolicy ();
		void handleRemoveWidget (QWidget *widget);
	};
}
}
