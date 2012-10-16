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

#include "accountsmanager.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QCoreApplication>
#include <xmlsettingsdialog/datasourceroles.h>

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	AccountsManager::AccountsManager (QObject *parent)
	: QObject (parent)
	, AccModel_ (new QStandardItemModel (this))
	{
		AccModel_->setHorizontalHeaderLabels (QStringList () << tr ("Account name"));
		AccModel_->horizontalHeaderItem (0)->setData (DataSources::DataFieldType::String, DataSources::DataSourceRole::FieldType);

		LoadAccounts ();
	}

	QAbstractItemModel* AccountsManager::GetAccModel () const
	{
		return AccModel_;
	}

	QStringList AccountsManager::GetAccounts () const
	{
		QStringList result;
		for (int i = 0; i < AccModel_->rowCount (); ++i)
			result << AccModel_->item (i)->text ();
		return result;
	}

	void AccountsManager::SaveAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_MP3Tunes");
		settings.beginGroup ("Accounts");
		settings.beginWriteArray ("List");
		for (int i = 0; i < AccModel_->rowCount (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("AccName", AccModel_->item (i)->text ());
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void AccountsManager::LoadAccounts ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_MP3Tunes");
		settings.beginGroup ("Accounts");
		const int size = settings.beginReadArray ("List");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const auto& str = settings.value ("AccName").toString ();
			AccModel_->appendRow (new QStandardItem (str));
		}
		settings.endArray ();
		settings.endGroup ();

		emit accountsChanged ();
	}

	void AccountsManager::addRequested (const QString&, const QVariantList& variants)
	{
		const auto& str = variants.value (0).toString ();
		if (str.isEmpty ())
			return;

		AccModel_->appendRow (new QStandardItem (str));

		SaveAccounts ();
		emit accountsChanged ();
	}

	void AccountsManager::removeRequested (const QString&, const QModelIndexList& indices)
	{
		QList<QStandardItem*> items;
		Q_FOREACH (const auto& idx, indices)
			items << AccModel_->itemFromIndex (idx);
		items.removeAll (0);

		Q_FOREACH (auto item, items)
			AccModel_->removeRow (item->row ());

		SaveAccounts ();

		emit accountsChanged ();
	}
}
}
}
