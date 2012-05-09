/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy, Von Never
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

#include "y7.h"
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QIcon>
#include <QMessageBox>

namespace LeechCraft
{
namespace Y7
{
	Plugin *Plugin::instance ()
	{
		static Plugin plugin;
		return &plugin;
	}

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		//QAbstractEventDispatcher::instance()->setEventFilter(&EventFilter);
		//messageId_ = RegisterWindowMessage(L"TaskbarButtonCreated");
		//qDebug () << "Message ID: " << messageId_;

		if (CoCreateInstance (CLSID_TaskbarList, nullptr, CLSCTX_ALL,
			IID_ITaskbarList3, reinterpret_cast<LPVOID *>(&taskbar_)) != S_OK)
		{
			qDebug() << "Cannot create TaskbarList";
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Y7";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Y7";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Windows 7 integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	//DWORD Plugin::GetMessageId () const
	//{
	// 	return messageId_;
	//}

	//bool Plugin::EventFilter(void *message)
	//{
	// 	auto msg = (DWORD) message;
	// 	qDebug() << "Recv: " << msg;
	// 	if ((DWORD) message == instance ()->GetMessageId ())
	// 	{
	// 		QMessageBox::information (nullptr, "Info", "Message has cometh!");
	// 		return true;
	// 	}
	// 
	// 	return false;
	//}
}
}

LC_EXPORT_PLUGIN (leechcraft_y7, LeechCraft::Y7::Plugin);

