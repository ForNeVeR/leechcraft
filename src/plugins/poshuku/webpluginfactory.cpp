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

#include "webpluginfactory.h"
#include <util/defaulthookproxy.h>
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	WebPluginFactory::WebPluginFactory (QObject *parent)
	: QWebPluginFactory (parent)
	{
		Core::Instance ().GetPluginManager ()->
				RegisterHookable (this);
		Reload ();
	}

	WebPluginFactory::~WebPluginFactory ()
	{
	}

	QObject* WebPluginFactory::create (const QString& mime,
			const QUrl& url,
			const QStringList& args, const QStringList& params) const
	{
		QList<IWebPlugin*> plugins = MIME2Plugin_.values (mime);
		Q_FOREACH (IWebPlugin *plugin, plugins)
		{
			QObject *result = plugin->Create (mime, url, args, params);
			if (result)
				return result;
		}
		return 0;
	}

	QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins () const
	{
		QList<Plugin> result;
		Q_FOREACH (IWebPlugin *plugin, Plugins_)
		{
			try
			{
				result << plugin->Plugin (true);
			}
			catch (...)
			{
				// It's ok to do a plain catch(...) {},
				// plugins refuse to add themselves to the list with this.
			}
		}
		return result;
	}

	void WebPluginFactory::refreshPlugins ()
	{
		Reload ();
		QWebPluginFactory::refreshPlugins ();
	}

	void WebPluginFactory::Reload ()
	{
		Plugins_.clear ();
		MIME2Plugin_.clear ();

		emit hookWebPluginFactoryReload (IHookProxy_ptr (new Util::DefaultHookProxy),
				Plugins_);

		Q_FOREACH (IWebPlugin *plugin, Plugins_)
			Q_FOREACH (const QWebPluginFactory::MimeType mime,
					plugin->Plugin (false).mimeTypes)
				MIME2Plugin_.insertMulti (mime.name, plugin);
	}
}
}
