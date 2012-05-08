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
#include <algorithm>
#include <QCryptographicHash>
#include <QUrl>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "sitessearcher.h"

namespace LeechCraft
{
namespace DeadLyrics
{
	Core::Core ()
	{
		qRegisterMetaType<Lyrics> ("LeechCraft::DeadLyrics::Lyrics");
		qRegisterMetaTypeStreamOperators<Lyrics> ("LeechCraft::DeadLyrics::Lyrics");

		Searchers_ << Searcher_ptr (new SitesSearcher (":/deadlyrics/resources/sites.xml"));
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Release ()
	{
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	QNetworkAccessManager* Core::GetNetworkAccessManager () const
	{
		return Proxy_->GetNetworkAccessManager ();
	}
}
}
