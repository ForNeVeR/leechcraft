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

#ifndef PLUGINS_POSHUKU_PLUGINS_WYFV_RELATEDITEM_H
#define PLUGINS_POSHUKU_PLUGINS_WYFV_RELATEDITEM_H
#include <QWidget>
#include <QBuffer>
#include <QUrl>
#include "ui_relateditem.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace WYFV
{
	struct Related;

	class RelatedItem : public QWidget
	{
		Q_OBJECT

		Ui::RelatedItem Ui_;
		QBuffer PixmapData_;
		QUrl URL_;
	public:
		RelatedItem (QWidget* = 0);
		void SetRelated (const Related&);
	protected:
		bool eventFilter (QObject*, QEvent*);
	private slots:
		void addToPixmap ();
		void handlePixmapFinished ();
	signals:
		void navigate (const QUrl&);
	};
}
}
}

#endif
