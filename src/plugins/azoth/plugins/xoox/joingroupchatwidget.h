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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JOINGROUPCHATWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JOINGROUPCHATWIDGET_H
#include <QDialog>
#include <interfaces/imucjoinwidget.h>
#include "ui_joingroupchatwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	class JoinGroupchatWidget : public QWidget
							  , public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMUCJoinWidget);

		Ui::JoinGroupchatWidget Ui_;
		GlooxAccount *SelectedAccount_;
	public:
		JoinGroupchatWidget (QWidget* = 0);

		QString GetServer () const;
		QString GetRoom () const;
		QString GetNickname () const;

		void AccountSelected (QObject *account);
		void Join (QObject *account);
		void Cancel ();

		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap& data);
	private slots:
		void checkValidity ();
		void on_ViewRooms__released ();
		void on_Server__textChanged (const QString&);
	signals:
		void validityChanged (bool);
	};
}
}
}

#endif
