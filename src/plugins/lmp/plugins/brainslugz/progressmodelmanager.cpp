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

#include "progressmodelmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <util/xpc/util.h>
#include "checker.h"

namespace LeechCraft
{
namespace LMP
{
namespace BrainSlugz
{
	ProgressModelManager::ProgressModelManager (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	{
	}

	QAbstractItemModel* ProgressModelManager::GetModel () const
	{
		return Model_;
	}

	void ProgressModelManager::handleCheckStarted (Checker *checker)
	{
		if (!Row_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "seems like a check is already in progress";
			return;
		}

		InitialCount_ = checker->GetRemainingCount ();
		const auto& label = tr ("Checking new releases of %n artist(s)...", 0, InitialCount_);
		Row_ = QList<QStandardItem*>
		{
			new QStandardItem { label },
			new QStandardItem { tr ("Checking...") },
			new QStandardItem {}
		};

		Util::InitJobHolderRow (Row_);
		handleProgress (InitialCount_);

		Model_->appendRow (Row_);

		connect (checker,
				SIGNAL (progress (int)),
				this,
				SLOT (handleProgress (int)));
		connect (checker,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
	}

	void ProgressModelManager::handleProgress (int remaining)
	{
		const auto done = InitialCount_ - remaining;
		Util::SetJobHolderProgress (Row_, done, InitialCount_, tr ("%1 of %2"));
	}

	void ProgressModelManager::handleFinished ()
	{
		Model_->removeRow (0);
		Row_.clear ();
	}
}
}
}
