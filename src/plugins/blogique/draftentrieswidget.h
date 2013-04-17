/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include <QWidget>
#include "interfaces/blogique/iaccount.h"
#include "ui_draftentrieswidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Blogique
{
	class IBloggingPlatform;
	class EntriesFilterProxyModel;

	class DraftEntriesWidget : public QWidget
	{
		Q_OBJECT

		Ui::DraftEntriesWidget Ui_;

		QStandardItemModel *DraftEntriesModel_;
		EntriesFilterProxyModel *FilterProxyModel_;
		QHash<QStandardItem*, Entry> Item2Entry_;

	public:
		explicit DraftEntriesWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
		QString GetName () const;
	private:
		void FillView (const QList<Entry>& entries);
		void FillStatistic ();
		void FillCurrentTab (const QModelIndex& index = QModelIndex ());

		void RemoveDraftEntry (qint64 id);

	public slots:
		void clear ();
		void loadDraftEntries ();
	private slots:
		void saveSplitterPosition (int pos, int index);
		void loadDraftsByDate (const QDate& date);
		void handleOpenDraftEntryInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenDraftEntryInNewTab (const QModelIndex& index = QModelIndex ());
		void on_DraftEntriesFilter__textChanged (const QString& text);
		void on_RemoveDraftEntry__released ();
		void on_PublishDraftEntry__released ();
		void on_DraftEntriesView__doubleClicked (const QModelIndex& index);
		void handleCalendarVisibilityChanged (bool visible);

	signals:
		void fillCurrentWidgetWithDraftEntry (const Entry& e);
		void fillNewWidgetWithDraftEntry (const Entry& e, const QByteArray& array = QByteArray ());
	};
}
}
