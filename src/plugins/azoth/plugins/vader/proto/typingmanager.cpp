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

#include "typingmanager.h"
#include <QTimer>

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	TypingManager::TypingManager (QObject *parent)
	: QObject (parent)
	, ExpTimer_ (new QTimer (this))
	, OutTimer_ (new QTimer (this))
	{
		ExpTimer_->setInterval (5000);
		connect (ExpTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkExpires ()));

		OutTimer_->setInterval (8000);
		connect (OutTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (sendOut ()));
	}

	void TypingManager::GotNotification (const QString& from)
	{
		if (!LastNotDates_.contains (from))
			emit startedTyping (from);

		if (LastNotDates_.isEmpty ())
			ExpTimer_->start ();

		LastNotDates_ [from] = QDateTime::currentDateTime ();
	}

	void TypingManager::SetTyping (const QString& to, bool isTyping)
	{
		if (!isTyping)
		{
			TypingTo_.remove (to);
			if (TypingTo_.isEmpty ())
				OutTimer_->stop ();
		}
		else
		{
			if (TypingTo_.isEmpty ())
				OutTimer_->start ();

			TypingTo_ << to;
			emit needNotify (to);
		}
	}

	void TypingManager::checkExpires ()
	{
		const auto& cur = QDateTime::currentDateTime ();
		Q_FOREACH (const QString& from, LastNotDates_.keys ())
		{
			if (LastNotDates_ [from].secsTo (cur) <= 10)
				continue;

			LastNotDates_.remove (from);
			emit stoppedTyping (from);
		}

		if (LastNotDates_.isEmpty ())
			ExpTimer_->stop ();
	}

	void TypingManager::sendOut ()
	{
		Q_FOREACH (const QString& to, TypingTo_)
			emit needNotify (to);
	}
}
}
}
}
