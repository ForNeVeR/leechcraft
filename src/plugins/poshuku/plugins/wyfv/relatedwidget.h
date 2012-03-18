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

#ifndef PLUGINS_POSHUKU_PLUGINS_WYFV_RELATEDWIDGET_H
#define PLUGINS_POSHUKU_PLUGINS_WYFV_RELATEDWIDGET_H
#include <QGraphicsView>
#include "related.h"

class QGraphicsScene;
class QGraphicsWidget;
class QGraphicsLinearLayout;
class QPropertyAnimation;

namespace LeechCraft
{
namespace Poshuku
{
namespace WYFV
{
	class RelatedWidget : public QGraphicsView
	{
		Q_OBJECT

		QGraphicsScene *Scene_;
		QGraphicsLinearLayout *Layout_;
		bool Shown_;
		QPropertyAnimation *OpacityAnimation_;
		QList<QWidget*> RelatedItems_;
		QGraphicsWidget *Form_;
	public:
		RelatedWidget (QWidget* = 0);
		virtual ~RelatedWidget ();

		void SetRelated (QList<Related>);
		void ToggleVisibility ();
	private slots:
		void handleAnimationFinished ();
	signals:
		void navigate (const QUrl&);
	};
}
}
}

#endif
