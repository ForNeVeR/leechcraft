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

#include <QWizardPage>
#include "ui_chooseuserpage.h"

namespace LeechCraft
{
namespace Dolozhee
{
	class ChooseUserPage : public QWizardPage
	{
		Q_OBJECT

		Ui::ChooseUserPage Ui_;
	public:
		enum User
		{
			Anonymous,
			New,
			Existing
		};

		ChooseUserPage (QWidget* = 0);

		int nextId () const;

		User GetUser () const;
		QString GetLogin () const;
		QString GetPassword () const;
		QString GetEmail () const;
		QString GetFirstName () const;
		QString GetLastName () const;
	};
}
}
