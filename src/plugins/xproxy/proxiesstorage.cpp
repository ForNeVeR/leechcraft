/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "proxiesstorage.h"
#include <QSettings>
#include <QCoreApplication>
#include <util/sll/qtutil.h>

namespace LeechCraft
{
namespace XProxy
{
	QList<Proxy> ProxiesStorage::FindMatching (const QString& reqHost, int reqPort, const QString& proto) const
	{
		static const std::map<QString, int> proto2port =
		{
			{ "http", 80 },
			{ "https", 443 }
		};

		if (reqPort < 0 && !proto.isEmpty ())
		{
			const auto pos = proto2port.find (proto.toLower ());
			if (pos != proto2port.end ())
				reqPort = pos->second;
		}

		QList<Proxy> result;
		for (const auto& pair : Util::Stlize (Proxies_))
		{
			if (std::any_of (pair.second.begin (), pair.second.end (),
					[&reqHost, reqPort, &proto] (const ReqTarget& target)
					{
						if (target.Port_ && reqPort > 0 && target.Port_ != reqPort)
							return false;

						if (!target.Protocols_.isEmpty () && !target.Protocols_.contains (proto))
							return false;

						if (!target.Host_.Matches (reqHost))
							return false;

						return true;
					}))
				result << pair.first;
		}
		return result;
	}

	void ProxiesStorage::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy");
		settings.beginGroup ("SavedProxies");
		for (const auto& entry : settings.value ("Entries").value<QList<Entry_t>> ())
			Proxies_ [entry.second] << entry.first;

		settings.endGroup ();
	}

	void ProxiesStorage::SaveSettings () const
	{
		QList<Entry_t> entries;
		for (const auto& pair : Util::Stlize (Proxies_))
			for (const auto& target : pair.second)
				entries.append ({ target, pair.first });

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy");
		settings.beginGroup ("SavedProxies");
		settings.setValue ("Entries", QVariant::fromValue (entries));
		settings.endGroup ();
	}
}
}
