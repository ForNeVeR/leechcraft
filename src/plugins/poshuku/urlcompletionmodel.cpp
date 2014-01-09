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

#include "urlcompletionmodel.h"
#include <stdexcept>
#include <QUrl>
#include <QtDebug>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/core/icoreproxy.h>
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	URLCompletionModel::URLCompletionModel (QObject *parent)
	: QAbstractItemModel (parent)
	, Valid_ (false)
	{
	}

	URLCompletionModel::~URLCompletionModel ()
	{
	}

	int URLCompletionModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	QVariant URLCompletionModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		if (role == Qt::DisplayRole)
			return Items_ [index.row ()].Title_ + " [" + Items_ [index.row ()].URL_ + "]";
		else if (role == Qt::DecorationRole)
			return Core::Instance ().GetIcon (QUrl (Items_ [index.row ()].URL_));
		else if (role == Qt::EditRole)
			return Base_ + index.row ();
		else if (role == RoleURL)
			return Items_ [index.row ()].URL_;
		else
			return QVariant ();
	}

	Qt::ItemFlags URLCompletionModel::flags (const QModelIndex&) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant URLCompletionModel::headerData (int, Qt::Orientation, int) const
	{
		return QVariant ();
	}

	QModelIndex URLCompletionModel::index (int row, int column,
			const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex URLCompletionModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int URLCompletionModel::rowCount (const QModelIndex& index) const
	{
		if (index.isValid ())
			return 0;

		return Items_.size ();
	}

	void URLCompletionModel::AddItem (const QString& title, const QString& url, size_t pos)
	{
		HistoryItem item =
		{
			title,
			QDateTime (),
			url
		};

		pos = std::min (static_cast<size_t> (Items_.size ()), pos);

		beginInsertRows (QModelIndex (), pos, pos);
		Items_.insert (Items_.begin () + pos, item);
		endInsertRows ();
	}

	void URLCompletionModel::setBase (const QString& str)
	{
		Valid_ = false;
		Base_ = str;

		Populate ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		int size = Items_.size ();
		emit hookURLCompletionNewStringRequested (proxy, this, str, size);
		if (proxy->IsCancelled ())
		{
			int newSize = Items_.size ();
			if (newSize == size)
				Items_.clear ();
			else
			{
				history_items_t newItems;
				std::copy (Items_.begin (), Items_.begin () + newSize - size,
						std::back_inserter (newItems));
				Items_ = newItems;
			}
		}
	}

	void URLCompletionModel::handleItemAdded (const HistoryItem&)
	{
		Valid_ = false;
	}

	void URLCompletionModel::Populate ()
	{
		if (!Valid_)
		{
			Valid_ = true;

			int size = Items_.size () - 1;
			if (size > 0)
				beginRemoveRows (QModelIndex (), 0, size);
			Items_.clear ();
			if (size > 0)
				endRemoveRows ();

			if (Base_.startsWith ('!'))
			{
				QStringList cats = Core::Instance ()
						.GetProxy ()->GetSearchCategories ();
				cats.sort ();
				Q_FOREACH (const QString& cat, cats)
				{
					HistoryItem item =
					{
						cat,
						QDateTime (),
						"!" + cat
					};
					Items_.push_back (item);;
				}
			}
			else
			{
				try
				{
					Core::Instance ().GetStorageBackend ()->LoadResemblingHistory (Base_, Items_);
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					Valid_ = false;
				}
			}

			size = Items_.size () - 1;
			if (size >= 0)
			{
				beginInsertRows (QModelIndex (), 0, size);
				endInsertRows ();
			}
		}
	}
}
}
