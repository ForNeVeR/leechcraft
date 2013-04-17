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

#pragma once

#include <QObject>
#include <QGraphicsProxyWidget>
#include "interfaces/monocle/idocument.h"

class QGraphicsView;
class QGraphicsScene;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QTreeWidget;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QButtonGroup;

namespace LeechCraft
{
namespace Monocle
{
	class PageGraphicsItem;
	class IFormField;
	class IFormFieldText;
	class IFormFieldChoice;
	class IFormFieldButton;

	class FormManager : public QObject
	{
		Q_OBJECT

		QGraphicsView * const View_;
		QGraphicsScene * const Scene_;

		QHash<QLineEdit*, std::shared_ptr<IFormFieldText>> Line2Field_;
		QHash<QTextEdit*, std::shared_ptr<IFormFieldText>> Multiline2Field_;
		QHash<QComboBox*, std::shared_ptr<IFormFieldChoice>> Combo2Field_;
		QHash<QTreeWidget*, std::shared_ptr<IFormFieldChoice>> List2Field_;
		QHash<QCheckBox*, std::shared_ptr<IFormFieldButton>> Check2Field_;
		QHash<QRadioButton*, std::shared_ptr<IFormFieldButton>> Radio2Field_;
		QHash<QPushButton*, std::shared_ptr<IFormFieldButton>> Button2Field_;

		QHash<QList<int>, QButtonGroup*> RadioGroups_;
	public:
		FormManager (QGraphicsView*, QObject* = 0);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);
	private:
		QGraphicsProxyWidget* AddTextField (std::shared_ptr<IFormField>);
		QGraphicsProxyWidget* AddChoiceField (std::shared_ptr<IFormField>);
		QGraphicsProxyWidget* AddButtonField (std::shared_ptr<IFormField>);
	private slots:
		void handleLineEditChanged (const QString&);
		void handleTextEditChanged ();

		void handleComboChanged ();
		void handleListChanged ();

		void handleCheckboxChanged ();
		void handleRadioChanged ();
		void handleButtonReleased ();
	};
}
}
