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

#include "glanceitem.h"
#include <QPropertyAnimation>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QIcon>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "core.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Glance
{
	GlanceItem::GlanceItem (const QPixmap& px, const QRect& closeButtonRect, QGraphicsItem *parent)
	: QGraphicsPixmapItem (px, parent)
	, Scale_ (0)
	, ScaleAnim_ (new QPropertyAnimation (this, "Scale"))
	, Current_ (false)
	, CloseButtonRect_ (closeButtonRect)
	, Pixmap_ (px)
	{
		setAcceptHoverEvents (true);
		setTransformationMode (Qt::SmoothTransformation);
		setCacheMode (ItemCoordinateCache);

		DrawCloseButton(false);
	}

	void GlanceItem::SetIndex (int idx)
	{
		Index_ = idx;
	}

	void GlanceItem::SetIdealScale (qreal scale)
	{
		Scale_ = scale;
	}

	void GlanceItem::QueueScaleAnim (qreal start, qreal end)
	{
		ScaleAnim_->stop ();
		ScaleAnim_->setDuration (300);
		ScaleAnim_->setStartValue (start);
		ScaleAnim_->setEndValue (end);

		ScaleAnim_->start ();
	}

	void GlanceItem::hoverEnterEvent (QGraphicsSceneHoverEvent*)
	{
		Q_FOREACH (GlanceItem* item, ItemsList_)
			if (item->IsCurrent () && item != this)
				item->SetCurrent (false);
		SetCurrent (true);
	}

	void GlanceItem::hoverMoveEvent (QGraphicsSceneHoverEvent *e)
	{
		if (CloseButtonRect_.contains (e->pos ().toPoint ()))
				DrawCloseButton(true);
		else
				DrawCloseButton(false);
	}

	void GlanceItem::hoverLeaveEvent (QGraphicsSceneHoverEvent*)
	{
		SetCurrent (false);
		DrawCloseButton (false);
	}

	void GlanceItem::mousePressEvent (QGraphicsSceneMouseEvent *e)
	{
		QGraphicsPixmapItem::mousePressEvent (e);
		e->accept ();
	}

	void GlanceItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *e)
	{
		const auto& clickPoint = e->buttonDownPos (Qt::LeftButton).toPoint ();
		if (CloseButtonRect_.contains (clickPoint))
			emit clicked (Index_, true);
		else
			emit clicked (Index_, false);
	}

	void GlanceItem::SetCurrent (bool cur)
	{
		if (cur)
		{
			setZValue (1);
			QueueScaleAnim (scale (), std::max (0.5, Scale_ * 1.3));
		}
		else
		{
			setZValue (0);
			QueueScaleAnim (scale (), Scale_);
		}
		Current_ = cur;
	}

	bool GlanceItem::IsCurrent () const
	{
		return Current_;
	}

	void GlanceItem::SetItemList (QList<QGraphicsItem*> list)
	{
		Q_FOREACH (QGraphicsItem* item, list)
			ItemsList_ << qgraphicsitem_cast<GlanceItem*> (item);
	}

	void GlanceItem::DrawCloseButton (bool selected)
	{
		QPixmap px (Pixmap_);
		QPainter p (&px);

		QIcon closeIcon = Core::Instance ().GetProxy ()->GetIcon ("window-close");
		closeIcon.paint (&p, CloseButtonRect_, Qt::AlignCenter, selected ? QIcon::Selected : QIcon::Normal);

		p.end ();
		setPixmap (px);
	}
};
};
};
