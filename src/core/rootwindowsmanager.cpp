/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "rootwindowsmanager.h"
#include <iterator>
#include <algorithm>
#include <interfaces/ihavetabs.h>
#include "core.h"
#include "mainwindow.h"
#include "mwproxy.h"
#include "tabmanager.h"
#include "dockmanager.h"
#include "xmlsettingsmanager.h"

#ifdef Q_OS_UNIX
#include <X11/Xutil.h>
#include <QX11Info>
#endif

namespace LeechCraft
{
	RootWindowsManager::RootWindowsManager (QObject *parent)
	: QObject (parent)
	{
	}

	void RootWindowsManager::Release ()
	{
		for (const auto& win : Windows_)
			win.Window_->handleQuit ();
	}

	MainWindow* RootWindowsManager::MakeMainWindow ()
	{
		return CreateWindow ();
	}

	TabManager* RootWindowsManager::GetTabManager (MainWindow *win) const
	{
		return GetTabManager (GetWindowIndex (win));
	}

	TabManager* RootWindowsManager::GetTabManager (int index) const
	{
		return Windows_.value (index).TM_;
	}

	bool RootWindowsManager::WindowCloseRequested (MainWindow *win)
	{
		if (Windows_ [0].Window_ == win)
			return false;

		const int index = GetWindowIndex (win);
		const auto& data = Windows_ [index];
		for (int i = data.TM_->GetWidgetCount () - 1; i >= 0; --i)
			qobject_cast<ITabWidget*> (data.TM_->GetWidget (i))->Remove ();

		auto mainWindow = Windows_ [0].Window_;

		auto dm = Core::Instance ().GetDockManager ();
		for (const auto& dock : dm->GetWindowDocks (win))
			dm->MoveDock (dock, win, mainWindow);

		emit windowRemoved (index);

		Windows_.removeAt (index);

		win->deleteLater ();

		return true;
	}

	QObject* RootWindowsManager::GetQObject ()
	{
		return this;
	}

	int RootWindowsManager::GetWindowsCount () const
	{
		return Windows_.size ();
	}

	int RootWindowsManager::GetPreferredWindowIndex () const
	{
		const auto active = QApplication::activeWindow ();
		if (!active)
			return 0;

		for (int i = 0; i < GetWindowsCount (); ++i)
			if (Windows_ [i].Window_ == active)
				return i;

		return 0;
	}

	int RootWindowsManager::GetPreferredWindowIndex (ITabWidget *itw) const
	{
		const auto& winMode = XmlSettingsManager::Instance ()->
				property ("WindowSelectionMode").toString ();
		if (winMode == "current")
			return GetPreferredWindowIndex ();

		const auto& thisTC = itw->GetTabClassInfo ().TabClass_;

		QPair<int, int> currentMax { -1, 0 };
		for (int i = 0; i < GetWindowsCount (); ++i)
		{
			const auto tm = Windows_ [i].TM_;

			int count = 0;
			const auto widgetCount = tm->GetWidgetCount ();
			if (!widgetCount)
				return i;

			for (int j = 0; j < widgetCount; ++j)
			{
				auto other = qobject_cast<ITabWidget*> (tm->GetWidget (j));
				if (other->GetTabClassInfo ().TabClass_ == thisTC)
					++count;
			}

			if (count > currentMax.second)
				currentMax = { i, count };
		}

		return currentMax.first;
	}

	int RootWindowsManager::GetWindowForTab (ITabWidget *tab) const
	{
		auto tabWidget = dynamic_cast<QWidget*> (tab);
		for (int i = 0; i < GetWindowsCount (); ++i)
		{
			const auto tw = Windows_ [i].Window_->GetTabWidget ();
			if (tw->IndexOf (tabWidget) >= 0)
				return i;
		}

		return -1;
	}

	int RootWindowsManager::GetWindowIndex (QMainWindow *w) const
	{
		auto pos = std::find_if (Windows_.begin (), Windows_.end (),
				[w] (decltype (Windows_.at (0)) item) { return item.Window_ == w; });
		return pos == Windows_.end () ? -1 : std::distance (Windows_.begin (), pos);
	}

	QMainWindow* RootWindowsManager::GetMainWindow (int index) const
	{
		return Windows_ [index].Window_;
	}

	IMWProxy* RootWindowsManager::GetMWProxy (int index) const
	{
		return Windows_ [index].Proxy_;
	}

	ICoreTabWidget* RootWindowsManager::GetTabWidget (int index) const
	{
		return Windows_ [index].Window_->GetTabWidget ();
	}

	MainWindow* RootWindowsManager::CreateWindow ()
	{
		auto win = new MainWindow;
		auto proxy = new MWProxy (win);
		auto tm = new TabManager (win->GetTabWidget (), win, win->GetTabWidget ());

		connect (tm,
				SIGNAL (currentTabChanged (QWidget*)),
				Core::Instance ().GetDockManager (),
				SLOT (handleTabChanged (QWidget*)));

		Windows_.push_back ({ win, proxy, tm });
		win->Init ();

		emit windowAdded (Windows_.size () - 1);

		return win;
	}

	void RootWindowsManager::PerformWithTab (std::function<void (TabManager*, int)> f, QWidget *w)
	{
		const int idx = GetWindowForTab (qobject_cast<ITabWidget*> (w));
		if (idx < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "no window for tab"
					<< w;
			return;
		}

		f (Windows_ [idx].TM_, idx);
	}

	void RootWindowsManager::MoveTab (int tabIdx, int fromWin, int toWin)
	{
		auto widget = Windows_ [fromWin].TM_->GetWidget (tabIdx);

		const auto sourceTW = Windows_ [fromWin].Window_->GetTabWidget ();
		const auto& name = sourceTW->TabText (tabIdx);
		const auto& icon = sourceTW->TabIcon (tabIdx);

		emit tabIsMoving (fromWin, toWin, tabIdx);

		Windows_ [fromWin].TM_->remove (widget);
		Windows_ [toWin].TM_->add (name, widget);
		Windows_ [toWin].TM_->changeTabIcon (widget, icon);

		emit tabMoved (fromWin, toWin, Windows_ [toWin].TM_->FindTabForWidget (widget));
	}

	void RootWindowsManager::moveTabToNewWindow ()
	{
		CreateWindow ();

		MoveTab (sender ()->property ("TabIndex").toInt (),
				sender ()->property ("FromWindowIndex").toInt (),
				GetWindowsCount () - 1);
	}

	void RootWindowsManager::moveTabToExistingWindow ()
	{
		MoveTab (sender ()->property ("TabIndex").toInt (),
				sender ()->property ("FromWindowIndex").toInt (),
				sender ()->property ("ToWindowIndex").toInt ());
	}

	namespace
	{
#if defined (Q_OS_UNIX) && defined (HAVE_X11)
		void SetWMClass (QWidget *w, QByteArray name)
		{
			XClassHint hint;
			hint.res_class = name.data ();
			hint.res_name = strdup ("leechcraft");
			XSetClassHint (QX11Info::display (), w->winId (), &hint);
			free (hint.res_name);
		}
#else
		void SetWMClass (QWidget*, QByteArray)
		{
		}
#endif
	}

	void RootWindowsManager::add (const QString& name, QWidget *w)
	{
		auto itw = qobject_cast<ITabWidget*> (w);

		if (GetWindowForTab (itw) != -1)
			return;

		int winIdx = GetPreferredWindowIndex (itw);

		const int oldWinIdx = GetWindowForTab (itw);
		if (oldWinIdx >= 0 && oldWinIdx != winIdx)
		{
			const auto& oldData = Windows_ [oldWinIdx];
			emit tabIsRemoving (winIdx, oldData.Window_->GetTabWidget ()->IndexOf (w));
			oldData.TM_->remove (w);
		}

		if (winIdx == -1)
		{
			CreateWindow ();
			winIdx = Windows_.size () - 1;
		}

		const auto& tc = itw->GetTabClassInfo ().TabClass_;
		Windows_ [winIdx].Window_->setWindowRole (tc);
		SetWMClass (Windows_ [winIdx].Window_, tc);
		Windows_ [winIdx].TM_->add (name, w);
		emit tabAdded (winIdx, Windows_ [winIdx].Window_->GetTabWidget ()->IndexOf (w));
	}

	void RootWindowsManager::remove (QWidget *w)
	{
		PerformWithTab ([this, w] (TabManager *tm, int winIdx)
			{
				emit tabIsRemoving (winIdx, tm->FindTabForWidget (w));
				tm->remove (w);
			}, w);
	}

	void RootWindowsManager::changeTabName (QWidget *w, const QString& name)
	{
		PerformWithTab ([w, &name] (TabManager *tm, int) { tm->changeTabName (w, name); }, w);
	}

	void RootWindowsManager::changeTabIcon (QWidget *w, const QIcon& icon)
	{
		PerformWithTab ([w, &icon] (TabManager *tm, int) { tm->changeTabIcon (w, icon); }, w);
	}

	void RootWindowsManager::bringToFront (QWidget *w)
	{
		PerformWithTab ([w] (TabManager *tm, int) { tm->bringToFront (w); }, w);
	}
}
