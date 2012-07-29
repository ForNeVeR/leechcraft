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
#include <QDebug>
#include <QIcon>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex> // compiler complains without it
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ijobholder.h>

namespace LeechCraft
{
namespace Y7
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		auto taskbarPtr = reinterpret_cast<LPVOID *> (&Taskbar_);
		if (CoCreateInstance (CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, taskbarPtr) != S_OK)
		{
			qDebug() << "Cannot create TaskbarList";
			return;
		}

		QMessageBox::information (nullptr, "Y7", "Created TaskbarList.");
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000, this, SLOT(initProgress ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Y7";
	}

	void Plugin::Release ()
	{
		QMessageBox::information (nullptr, "Y7", "Releasing COM object...");
		Taskbar_->Release ();
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

	void Plugin::initProgress ()
	{
		auto pluginList = Proxy_->GetPluginsManager ()->GetAllCastableTo<IJobHolder *> ();
		QString plugins;
		Q_FOREACH(auto plugin, pluginList)
		{
			auto pluginInfo = reinterpret_cast<IInfo *> (plugin);
			plugins.append(pluginInfo->GetName ());
		}

		QMessageBox::information (nullptr, "Y7", plugins);
	}

	void Plugin::setProgress ()
	{
		auto handle = Proxy_->GetMainWindow ()->winId ();

		Taskbar_->SetProgressState (handle, TBPF_ERROR);

		QMessageBox::information (nullptr, "Y7", "Progress succesful set.");
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_y7, LeechCraft::Y7::Plugin);
