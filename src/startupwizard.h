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

#ifndef STARTUPWIZARD_H
#define STARTUPWIZARD_H
#include <QWizard>
#include <QMap>

namespace LeechCraft
{
	class StartupWizard : public QWizard
	{
		Q_OBJECT

		QList<QWizardPage*> Pages_;
		QMap<QWizardPage*, int> Page2ID_;
		int TypeChoseID_;
	public:
		enum Type
		{
			TBasic,
			TAdvanced
		};
	private:
		Type Type_;
	public:
		StartupWizard (QWidget* = 0);

		int nextId () const;
	private:
		void AddPages ();
	private slots:
		void handleTypeChanged (StartupWizard::Type);

		void handleAccepted ();
		void handleRejected ();
	};
};

#endif

