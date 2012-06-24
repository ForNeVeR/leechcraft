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

#include "monocle.h"
#include <QIcon>
#include "core.h"
#include "documenttab.h"

namespace LeechCraft
{
namespace Monocle
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Core::Instance ().SetProxy (proxy);

		TabClassInfo temp = {
			GetUniqueID () + "_Document",
			"Monocle",
			GetInfo (),
			GetIcon (),
			55,
			TFOpenableByRequest | TFSuggestOpening
		};
		DocTabInfo_ = temp;
		
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Modular document viewer for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_t() << DocTabInfo_;
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == DocTabInfo_.TabClass_)
		{
			auto tab = new DocumentTab (DocTabInfo_, this);
			emit addNewTab (DocTabInfo_.VisibleName_, tab);
			emit changeTabIcon (tab, DocTabInfo_.Icon_);
			emit raiseTab (tab);

			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			connect (tab,
					SIGNAL (changeTabName (QWidget*, QString)),
					this,
					SIGNAL (changeTabName (QWidget*, QString)));
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< id;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	void Plugin::AddPlugin (QObject *pluginObj)
	{
		Core::Instance ().AddPlugin (pluginObj);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle, LeechCraft::Monocle::Plugin);

