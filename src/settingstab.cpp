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

#include "settingstab.h"
#include <algorithm>
#include <numeric>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QToolButton>
#include <QStyledItemDelegate>
#include <QPainter>
#include "util/gui/flowlayout.h"
#include "xmlsettingsdialog/xmlsettingsdialog.h"
#include "interfaces/ihavesettings.h"
#include "interfaces/iplugin2.h"
#include "interfaces/ipluginready.h"
#include "core.h"
#include "coreinstanceobject.h"

namespace LeechCraft
{
	const int ButtonWidth = 96;

	SettingsTab::SettingsTab (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar (tr ("Settings bar")))
	, ActionBack_ (new QAction (tr ("Back"), this))
	, ActionApply_ (new QAction (tr ("Apply"), this))
	, ActionCancel_ (new QAction (tr ("Cancel"), this))
	{
		Ui_.setupUi (this);
		Ui_.ListContents_->setLayout (new QVBoxLayout);
		Ui_.DialogContents_->setLayout (new QVBoxLayout);

		ActionBack_->setProperty ("ActionIcon", "go-previous");
		connect (ActionBack_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleBackRequested ()));

		ActionApply_->setProperty ("ActionIcon", "dialog-ok");
		connect (ActionApply_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleApply ()));

		ActionCancel_->setProperty ("ActionIcon", "dialog-cancel");
		connect (ActionCancel_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCancel ()));
	}

	namespace
	{
		QList<QPair<QString, QString>> GetFirstClassPlugins (IPlugin2 *ip2)
		{
			const QSet<QByteArray>& classes = ip2->GetPluginClasses ();
			const QObjectList& pReady = Core::Instance ()
					.GetPluginManager ()->GetAllCastableRoots<IPluginReady*> ();

			QList<QPair<QString, QString>> result;

			Q_FOREACH (QObject *obj, pReady)
			{
				if (obj == Core::Instance ().GetCoreInstanceObject ())
					continue;

				IPluginReady *ipr = qobject_cast<IPluginReady*> (obj);
				if (ipr->GetExpectedPluginClasses ().intersect (classes).isEmpty ())
					continue;

				IInfo *ii = qobject_cast<IInfo*> (obj);
				result << qMakePair (SettingsTab::tr ("Plugins for %1")
								.arg (ii->GetName ()),
							ii->GetName ());
			}

			if (result.isEmpty ())
				result << qMakePair (SettingsTab::tr ("General second-level plugins"), QString ());
			return result;
		}

		QString NameForGroup (const QString& origName, const QString& group)
		{
			QStringList origSplit = origName.split (' ', QString::SkipEmptyParts);
			QStringList groupSplit = group.split (' ', QString::SkipEmptyParts);

			while (origSplit.value (0) == groupSplit.value (0) &&
					origSplit.size () > 1)
			{
				origSplit.removeFirst ();
				if (!groupSplit.isEmpty ())
					groupSplit.removeFirst ();
			}

			const auto& fm = QApplication::fontMetrics ();
			const int pad = 3;
			for (auto i = origSplit.begin (), end = origSplit.end (); i != end; ++i)
				if (fm.width (*i) > ButtonWidth - 2 * pad)
					*i = fm.elidedText (*i, Qt::ElideRight, ButtonWidth - 2 * pad);

			return origSplit.join ("\n");
		}

		QMap<QObject*, QList<QPair<QString, QString>>> BuildGroups (const QObjectList& settables)
		{
			QMap<QObject*, QList<QPair<QString, QString>>> result;
			Q_FOREACH (QObject *obj, settables)
			{
				IPlugin2 *ip2 = qobject_cast<IPlugin2*> (obj);
				const auto& firstClass = ip2 ?
						GetFirstClassPlugins (ip2) :
						QList<QPair<QString, QString>> ();

				if (obj == Core::Instance ().GetCoreInstanceObject ())
					result [obj] << qMakePair (QString ("LeechCraft"), QString ());
				else if (firstClass.isEmpty ())
					result [obj] << qMakePair (SettingsTab::tr ("General plugins"), QString ());
			}

			return result;
		}
	}

	void SettingsTab::Initialize ()
	{
		const QObjectList& settables = Core::Instance ()
				.GetPluginManager ()->GetAllCastableRoots<IHaveSettings*> ();

		const auto& obj2groups = BuildGroups (settables);
		QSet<QPair<QString, QString>> allGroups;
		Q_FOREACH (auto list, obj2groups.values ())
			allGroups += QSet<QPair<QString, QString>>::fromList (list);

		QMap<QString, QGroupBox*> group2box;
		Q_FOREACH (auto pair, allGroups)
		{
			QGroupBox *box = new QGroupBox (pair.first);
			box->setLayout (new Util::FlowLayout);
			group2box [pair.first] = box;
		}

		QStringList keys = group2box.keys ();
		if (keys.contains (tr ("General plugins")))
		{
			keys.removeAll (tr ("General plugins"));
			keys.prepend (tr ("General plugins"));
		}

		if (keys.removeAll ("LeechCraft"))
			keys.prepend ("LeechCraft");

		Q_FOREACH (const QString& key, keys)
			Ui_.ListContents_->layout ()->addWidget (group2box [key]);

		QMap<QString, QList<QToolButton*>> group2buttons;
		Q_FOREACH (QObject *obj, settables)
		{
			IInfo *ii = qobject_cast<IInfo*> (obj);
			const QIcon& icon = ii->GetIcon ().isNull () ?
					QIcon (":/resources/images/defaultpluginicon.svg") :
					ii->GetIcon ();
			Q_FOREACH (auto pair, obj2groups [obj])
			{
				QToolButton *butt = new QToolButton;
				butt->setToolButtonStyle (Qt::ToolButtonTextUnderIcon);

				const QString& name = NameForGroup (ii->GetName (), pair.second);
				butt->setText (name);

				butt->setToolTip (ii->GetInfo ());
				butt->setIconSize (QSize (72, 72));
				butt->setIcon (icon);
				butt->setProperty ("SettableObject", QVariant::fromValue<QObject*> (obj));
				butt->setFixedWidth (ButtonWidth);
				butt->setAutoRaise (true);

				connect (butt,
						SIGNAL (released ()),
						this,
						SLOT (handleSettingsCalled ()));
				group2box [pair.first]->layout ()->addWidget (butt);
				group2buttons [pair.first] << butt;
			}
		}

		qobject_cast<QBoxLayout*> (Ui_.ListContents_->layout ())->addStretch ();

		Q_FOREACH (const QString& group, group2buttons.keys ())
		{
			const auto& buttons = group2buttons [group];
			const auto height = std::accumulate (buttons.begin (), buttons.end (), 0,
					[] (int height, QToolButton *button) { return std::max (height, button->sizeHint ().height ()); });
			std::for_each (buttons.begin (), buttons.end (),
					[height] (QToolButton *button) { button->setFixedHeight (height); });
		}
	}

	TabClassInfo SettingsTab::GetTabClassInfo () const
	{
		TabClassInfo setInfo =
		{
			"org.LeechCraft.SettingsPane",
			tr ("Settings"),
			tr ("LeechCraft-wide settings dashboard"),
			Core::Instance ().GetCoreInstanceObject ()->GetIcon (),
			0,
			static_cast<TabFeatures> (TFSingle | TFOpenableByRequest)
		};
		return setInfo;
	}

	QObject* SettingsTab::ParentMultiTabs ()
	{
		return Core::Instance ().GetCoreInstanceObject ();
	}

	void SettingsTab::Remove ()
	{
		emit remove (this);
	}

	QToolBar* SettingsTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void SettingsTab::FillPages (QObject *obj, bool sub)
	{
		IInfo *ii = qobject_cast<IInfo*> (obj);
		IHaveSettings *ihs = qobject_cast<IHaveSettings*> (obj);
		auto sd = ihs->GetSettingsDialog ();

		const QStringList& pages = sd->GetPages ();
		int pgId = 0;
		Q_FOREACH (const QString& page, pages)
		{
			QString itemName;
			if (sub)
				itemName = pages.size () == 1 && ii->GetName ().contains (page) ?
						ii->GetName () :
						(ii->GetName () + ": " + page);
			else
				itemName = page;
			auto item = new QListWidgetItem (ii->GetIcon (), itemName);
			item->setTextAlignment (Qt::AlignCenter);
			Ui_.Cats_->addItem (item);

			Item2Page_ [item] = qMakePair (ihs, pgId++);
		}

		if (auto ihp = qobject_cast<IPluginReady*> (obj))
		{
			const auto& expected = ihp->GetExpectedPluginClasses ();

			const auto& settables = Core::Instance ()
					.GetPluginManager ()->GetAllCastableRoots<IHaveSettings*> ();
			Q_FOREACH (auto settableObj, settables)
			{
				auto ip2 = qobject_cast<IPlugin2*> (settableObj);
				if (!ip2 || !expected.contains (ip2->GetPluginClasses ()))
					continue;

				FillPages (settableObj, true);
			}
		}
	}

	void SettingsTab::showSettingsFor (QObject *obj)
	{
		Item2Page_.clear ();

		IInfo *ii = qobject_cast<IInfo*> (obj);
		Ui_.SectionName_->setText (tr ("Settings for %1")
				.arg (ii->GetName ()));

		IHaveSettings *ihs = qobject_cast<IHaveSettings*> (obj);
		auto sd = ihs->GetSettingsDialog ();
		Ui_.DialogContents_->layout ()->addWidget (sd.get ());
		sd->show ();

		FillPages (obj, false);

		Ui_.StackedWidget_->setCurrentIndex (1);
		Toolbar_->addAction (ActionBack_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (ActionApply_);
		Toolbar_->addAction (ActionCancel_);

		const int width = Ui_.Cats_->viewport ()->width ();
		auto gridSize = Ui_.Cats_->gridSize ();
		gridSize.setWidth (width);
		Q_FOREACH (auto item, Item2Page_.keys ())
			item->setSizeHint (gridSize);
		Ui_.Cats_->setGridSize (gridSize);
	}

	void SettingsTab::handleSettingsCalled ()
	{
		QObject *obj = sender ()->property ("SettableObject").value<QObject*> ();
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty object"
					<< sender ();
			return;
		}

		handleBackRequested ();
		showSettingsFor (obj);
	}

	void SettingsTab::handleBackRequested ()
	{
		Toolbar_->clear ();
		Ui_.StackedWidget_->setCurrentIndex (0);

		Ui_.Cats_->clear ();
		if (Ui_.DialogContents_->layout ()->count ())
		{
			QLayoutItem *item = Ui_.DialogContents_->layout ()->takeAt (0);
			item->widget ()->hide ();
			delete item;
		}

		Item2Page_.clear ();
	}

	void SettingsTab::handleApply ()
	{
		Q_FOREACH (const auto& pair, Item2Page_.values ())
			pair.first->GetSettingsDialog ()->accept ();
	}

	void SettingsTab::handleCancel ()
	{
		Q_FOREACH (const auto& pair, Item2Page_.values ())
			pair.first->GetSettingsDialog ()->reject ();
	}

	void SettingsTab::on_Cats__currentItemChanged (QListWidgetItem *current)
	{
		const auto& pair = Item2Page_ [current];
		if (!pair.first)
			return;

		auto sd = pair.first->GetSettingsDialog ();
		sd->SetPage (pair.second);

		if (Ui_.DialogContents_->layout ()->count ())
		{
			QLayoutItem *item = Ui_.DialogContents_->layout ()->takeAt (0);
			item->widget ()->hide ();
			delete item;
		}
		Ui_.DialogContents_->layout ()->addWidget (sd.get ());
		sd->show ();
	}
}
