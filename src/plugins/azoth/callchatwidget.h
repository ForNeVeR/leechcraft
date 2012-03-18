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

#ifndef PLUGINS_AZOTH_CALLCHATWIDGET_H
#define PLUGINS_AZOTH_CALLCHATWIDGET_H
#include <QWidget>
#include "interfaces/imediacall.h"
#include "ui_callchatwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	class CallChatWidget : public QWidget
	{
		Q_OBJECT

		Ui::CallChatWidget Ui_;
		QObject *CallObject_;
		IMediaCall *Call_;
	public:
		CallChatWidget (QObject*, QWidget* = 0);
	private slots:
		void on_AcceptButton__released ();
		void on_HangupButton__released ();
		void handleStateChanged (LeechCraft::Azoth::IMediaCall::State);
		void scheduleDelete ();
	};
}
}

#endif
