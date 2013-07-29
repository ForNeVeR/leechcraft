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

#include "rulesstorage.h"
#include <QFile>
#include <QtDebug>
#include <QStringList>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

namespace LeechCraft
{
namespace KBSwitch
{
	namespace
	{
		QString FindX11Dir ()
		{
			static const auto dirs =
			{
				"/etc/X11",
				"/usr/share/X11",
				"/usr/local/share/X11",
				"/usr/X11R6/lib/X11",
				"/usr/X11R6/lib64/X11",
				"/usr/local/X11R6/lib/X11",
				"/usr/local/X11R6/lib64/X11",
				"/usr/lib/X11",
				"/usr/lib64/X11",
				"/usr/local/lib/X11",
				"/usr/local/lib64/X11",
				"/usr/pkg/share/X11",
				"/usr/pkg/xorg/lib/X11"
			};

			for (const auto& item : dirs)
			{
				const QString itemStr (item);
				if (QFile::exists (itemStr + "/xkb/rules"))
					return itemStr;
			}

			return {};
		}

		QString FindRulesFile (Display *display)
		{
			const auto& x11dir = FindX11Dir ();

			XkbRF_VarDefsRec vd;
			char *path = 0;
			if (XkbRF_GetNamesProp (display, &path, &vd) && path)
			{
				const QString pathStr (path);
				free (path);
				return x11dir + "/xkb/rules/" + pathStr;
			}

			for (auto rfName : { "base", "xorg", "xfree86" })
			{
				const auto rf = QString ("/xkb/rules/") + rfName;
				const auto& path = x11dir + rf;
				if (QFile::exists (path))
					return path;
			}

			return {};
		}
	}

	RulesStorage::RulesStorage (Display *dpy, QObject *parent)
	: QObject (parent)
	, Display_ (dpy)
	{
		const auto& rf = FindRulesFile (Display_);
		if (rf.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "rules file wasn't found";
			return;
		}

		char *locale = { 0 };
		const auto xkbRules = XkbRF_Load (QFile::encodeName (rf).data (),
				locale, true, true);
		if (!xkbRules)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot load rules from"
					<< rf;
			return;
		}

		for (int i = 0; i < xkbRules->layouts.num_desc; ++i)
		{
			const auto& desc = xkbRules->layouts.desc [i];
			LayName2Desc_ [desc.name] = desc.desc;
			LayDesc2Name_ [desc.desc] = desc.name;
		}

		QStringList pcModelStrings;
		for (int i = 0; i < xkbRules->models.num_desc; ++i)
		{
			const auto& model = xkbRules->models.desc [i];
			KBModels_ [model.name] = model.desc;

			const auto& kbString = QString ("%1 (%2)").arg (model.desc).arg (model.name);
			if (QString (model.name).startsWith ("pc10"))
				pcModelStrings << kbString;
			else
				KBModelsStrings_ << kbString;

			KBModelString2Code_ [kbString] = model.name;
		}
		pcModelStrings.sort ();
		KBModelsStrings_.sort ();
		KBModelsStrings_ = pcModelStrings + KBModelsStrings_;
	}

	const QHash<QString, QString>& RulesStorage::GetLayoutsD2N () const
	{
		return LayDesc2Name_;
	}

	const QHash<QString, QString>& RulesStorage::GetLayoutsN2D () const
	{
		return LayName2Desc_;
	}

	const QHash<QString, QString>& RulesStorage::GetKBModels () const
	{
		return KBModels_;
	}

	const QStringList& RulesStorage::GetKBModelsStrings () const
	{
		return KBModelsStrings_;
	}
}
}
