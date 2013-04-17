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

#include "groupeditordialog.h"
#include <QStringListModel>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	GroupEditorDialog::GroupEditorDialog (const QStringList& initial,
			const QStringList& allGroups,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.GroupsSelector_->setWindowFlags (Qt::Widget);
		Ui_.GroupsSelector_->setPossibleSelections (allGroups);
		Ui_.GroupsSelector_->SetSelections (initial);

		Util::TagsCompleter *tc = new Util::TagsCompleter (Ui_.CategoriesLineEdit_, this);
		tc->OverrideModel (new QStringListModel (allGroups, this));

		const QString& text = Core::Instance ()
				.GetProxy ()->GetTagsManager ()->Join (initial);
		Ui_.CategoriesLineEdit_->setText (text);
		Ui_.CategoriesLineEdit_->AddSelector ();

		connect (Ui_.CategoriesLineEdit_,
				SIGNAL (textChanged (const QString&)),
				Ui_.GroupsSelector_,
				SLOT (lineTextChanged (const QString&)));
	}

	QStringList GroupEditorDialog::GetGroups () const
	{
		const QString& text = Ui_.CategoriesLineEdit_->text ();
		return Core::Instance ().GetProxy ()->
				GetTagsManager ()->Split (text);
	}

	void GroupEditorDialog::on_GroupsSelector__tagsSelectionChanged (const QStringList& groups)
	{
		const QString& text = Core::Instance ()
				.GetProxy ()->GetTagsManager ()->Join (groups);
		Ui_.CategoriesLineEdit_->setText (text);
	}
}
}
