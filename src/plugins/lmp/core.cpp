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

#include "core.h"
#include <QUrl>
#include <QTextCodec>
#include <QMainWindow>
#include <QNetworkReply>
#include <interfaces/core/icoreproxy.h>
#include "xmlsettingsmanager.h"
#include "defaultwidget.h"
#include "keyinterceptor.h"

using namespace Phonon;

namespace LeechCraft
{
namespace LMP
{
	Core::Core ()
	: ShowAction_ (new QAction (QIcon (":/plugins/lmp/resources/images/lmp.svg"),
				tr ("Show LMP"), this))
	, DefaultWidget_ (0)
	{
		ShowAction_->setEnabled (false);
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Release ()
	{
		delete DefaultWidget_;
		Player_.reset ();
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetCoreProxy () const
	{
		return Proxy_;
	}

	PlayerWidget* Core::CreateWidget () const
	{
		PlayerWidget *result = new PlayerWidget;
		connect (result,
			SIGNAL (gotEntity (const LeechCraft::Entity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::Entity&)));
		KeyInterceptor *ki = new KeyInterceptor (result, result);
		QList<QWidget*> children = result->findChildren<QWidget*> ();
		children << result;
		for (QList<QWidget*>::iterator i = children.begin (),
				end = children.end (); i != end; ++i)
			(*i)->installEventFilter (ki);
		return result;
	}

	IVideoWidget* Core::GetDefaultWidget () const
	{
		if (!DefaultWidget_)
			DefaultWidget_ = new DefaultWidget;
		return DefaultWidget_;
	}

	void Core::Reinitialize ()
	{
	}

	void Core::Play ()
	{
		Player_->Play ();
	}

	void Core::Pause ()
	{
		Player_->Pause ();
	}

	void Core::Stop ()
	{
		Player_->Stop ();
	}

	void Core::Clear ()
	{
		Player_->Clear ();
	}

	void Core::Enqueue (const QUrl& url)
	{
		Player_->Enqueue (new MediaSource (url));
	}

	void Core::Enqueue (QIODevice *data)
	{
		Player_->Enqueue (new MediaSource (data));
	}

	QAction* Core::GetShowAction () const
	{
		return ShowAction_;
	}

	void Core::Handle (const Entity& e)
	{
		MediaSource *source = 0;
		if (e.Entity_.canConvert<QNetworkReply*> ())
			source = new MediaSource (e.Entity_.value<QNetworkReply*> ());
		else if (e.Entity_.canConvert<QIODevice*> ())
			source = new MediaSource (e.Entity_.value<QIODevice*> ());
		else if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl url = e.Entity_.toUrl ();
			if (url.scheme () == "file")
				source = new MediaSource (url.toLocalFile ());
			else
				source = new MediaSource (url);
		}
		else
			return;

		if (!(e.Parameters_ & FromUserInitiated) &&
				e.Parameters_ & Internal)
		{
			MediaObject *music = createPlayer (NotificationCategory, *source);
			delete source;
			music->play ();
			connect (music,
					SIGNAL (finished ()),
					music,
					SLOT (deleteLater ()),
					Qt::QueuedConnection);
			return;
		}

		if (!Player_.get ())
		{
			Player_.reset (new Player (Proxy_->GetMainWindow ()));
			ShowAction_->setEnabled (true);
			connect (ShowAction_,
					SIGNAL (triggered ()),
					Player_.get (),
					SLOT (show ()));
			connect (Player_.get (),
					SIGNAL (gotEntity (const LeechCraft::Entity&)),
					this,
					SIGNAL (gotEntity (const LeechCraft::Entity&)));
		}
		Player_->Enqueue (source);
		if (e.Parameters_ & FromUserInitiated)
		{
			Player_->show ();
			Player_->Play ();
		}
	}
}
}
