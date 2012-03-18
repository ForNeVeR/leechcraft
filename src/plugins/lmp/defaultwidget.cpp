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

#include "defaultwidget.h"
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	DefaultWidget::DefaultWidget ()
	{
	}

	void DefaultWidget::Play ()
	{
		Core::Instance ().Play ();
	}

	void DefaultWidget::Pause ()
	{
		Core::Instance ().Pause ();
	}

	void DefaultWidget::Stop ()
	{
		Core::Instance ().Stop ();
	}

	void DefaultWidget::Clear ()
	{
		Core::Instance ().Clear ();
	}

	void DefaultWidget::Enqueue (const QUrl& url)
	{
		Core::Instance ().Enqueue (url);
	}

	void DefaultWidget::Enqueue (QIODevice* data)
	{
		Core::Instance ().Enqueue (data);
	}

	QWidget* DefaultWidget::Widget ()
	{
		return 0;
	}
}
}
