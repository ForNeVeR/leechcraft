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
#include <QModelIndex>
#include <QTimer>
#include <QVector>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ijobholder.h>

namespace LeechCraft
{
namespace Y7
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		ProgressModel_ = nullptr;

		auto taskbarPtr = reinterpret_cast<LPVOID *> (&Taskbar_);
		if (CoCreateInstance (CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, taskbarPtr) != S_OK)
		{
			qDebug () << "Cannot create TaskbarList";
			return;
		}

		QMessageBox::information (nullptr, "Y7", "Created TaskbarList.");
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000, this, SLOT(initButtons ()));
		QTimer::singleShot (5000, this, SLOT(initProgress ()));
		QTimer::singleShot (5000, this, SLOT(initTabs ()));
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

	void Plugin::InitProgressModel ()
	{
		connect (ProgressModel_, SIGNAL (rowsInserted (const QModelIndex &, int, int)),
			SLOT (progressRowsInserted (const QModelIndex &, int, int)));
		connect (ProgressModel_, SIGNAL (dataChanged (const QModelIndex &, const QModelIndex &)),
			SLOT (progressDataChanged (const QModelIndex &, const QModelIndex &)));
	}

	void Plugin::SetProgress ()
	{
		auto handle = Proxy_->GetMainWindow ()->winId ();

		Taskbar_->SetProgressState (handle, TBPF_ERROR);

		QMessageBox::information (nullptr, "Y7", "Progress succesful set.");
	}

	void Plugin::initButtons ()
	{
		const size_t SZTIP_SIZE = 260;

		QVector<THUMBBUTTON> buttons;

		THUMBBUTTON button = {};
		button.iId = 0;
		button.dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
		wcsncpy(button.szTip, L"Tooltip 1", SZTIP_SIZE);
		button.dwFlags = THBF_ENABLED;
		
		buttons.push_back(button);
		
		auto windowId = Proxy_->GetMainWindow ()->winId ();
		if (Taskbar_->ThumbBarAddButtons(windowId, buttons.size(), buttons.data()) != S_OK)
		{
			QMessageBox::information (nullptr, "Y7", "Cannot initialize taskbar.");
			return;
		}

		QMessageBox::information (nullptr, "Y7", "Taskbar initialized.");
	}

	void Plugin::initProgress ()
	{
		QMessageBox::information (nullptr, "Y7", "Start init progress");

		auto plugins = Proxy_->GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ();

		QString message("Traceable plugins detected:\n");
		Q_FOREACH (auto plugin, plugins)
		{
			auto pluginInfo = qobject_cast<IInfo*> (plugin);
			auto jobHolder = qobject_cast<IJobHolder*> (plugin);

			QMessageBox::information (nullptr, "Y7", pluginInfo->GetName ());

			message.append(pluginInfo->GetName () + "\n");
			JobHolders_.append(jobHolder);
		}

		QMessageBox::information (nullptr, "Y7", message);
		
		if (!plugins.empty())
		{
			auto currentPlugin = qobject_cast<IJobHolder*> (plugins.first());
			ProgressModel_ = currentPlugin->GetRepresentation ();
			InitProgressModel ();
		}
	}

	void Plugin::initTabs ()
	{
		auto tabWidget = Proxy_->GetTabWidget ();
		auto mainWindowId = Proxy_->GetMainWindow ()->winId ();

		for (auto index = 0; index < tabWidget->WidgetCount (); ++index)
		{
			auto widget = tabWidget->Widget (index);
			auto widgetWinId = widget->winId ();
			if (Taskbar_->RegisterTab (widgetWinId, mainWindowId) != S_OK)
			{
				QMessageBox::information (nullptr, "Y7", "Cannot initialize tab.");
				return;
			}

			if (Taskbar_->SetTabOrder(widgetWinId, nullptr) != S_OK)
			{
				QMessageBox::information (nullptr, "Y7", "Cannot set tab order.");
				return;
			}
		}

		QMessageBox::information (nullptr, "Y7", QString("%1 tabs initialized.").arg(tabWidget->WidgetCount ()));
	}

	void Plugin::progressRowsInserted (const QModelIndex &parent, int start, int end)
	{
		const QModelIndex &model = parent.isValid ()
			? parent
			: qobject_cast<QAbstractItemModel*> (sender ())->index (start, 2, parent);

		for (auto index = start; index <= end; ++index)
		{
			auto modelIndex = model.child (index, 2);
			auto rowRole = modelIndex.data (CustomDataRoles::RoleJobHolderRow).value<JobHolderRow> ();
			qlonglong done = modelIndex.data (ProcessState::Done).toLongLong ();
			qlonglong total = modelIndex.data (ProcessState::Total).toLongLong ();
			QMessageBox::information (nullptr, "Y7", QString("%1 / %2").arg(done).arg(total));
		}
	}

	void Plugin::progressDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
	{
		for (auto index = topLeft.row(); index <= bottomRight.row(); ++index)
		{
			auto modelIndex = topLeft.child (index, 2);
			auto rowRole = modelIndex.data (CustomDataRoles::RoleJobHolderRow).value<JobHolderRow> ();
			qlonglong done = modelIndex.data (ProcessState::Done).toLongLong ();
			qlonglong total = modelIndex.data (ProcessState::Total).toLongLong ();
			QMessageBox::information (nullptr, "Y7", QString("%1 / %2").arg(done).arg(total));
		}	
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_y7, LeechCraft::Y7::Plugin);
