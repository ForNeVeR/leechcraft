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

#include "speedselectoraction.h"
#include <functional>
#include <QComboBox>
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			SpeedSelectorAction::SpeedSelectorAction (const QString& s, QObject *parent)
			: QWidgetAction (parent)
			, Setting_ (s)
			{
			}

			int SpeedSelectorAction::CurrentData ()
			{
				QList<QWidget*> ws = createdWidgets ();
				if (ws.size ())
				{
					QComboBox *bx = static_cast<QComboBox*> (ws.at (0));
					return bx->itemData (bx->currentIndex ()).toInt ();
				}
				else
					return 0;
			}

			QWidget* SpeedSelectorAction::createWidget (QWidget *parent)
			{
				QComboBox *selector = new QComboBox (parent);
				connect (selector,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (syncSpeeds (int)));

				QTimer::singleShot (0,
						this,
						SLOT (handleSpeedsChanged ()));

				return selector;
			}

			void SpeedSelectorAction::deleteWidget (QWidget *w)
			{
				delete w;
			}

			void SpeedSelectorAction::handleSpeedsChanged ()
			{
				Call ([] (QComboBox *box) { box->clear (); });

				if (!XmlSettingsManager::Instance ()->
						property ("EnableFastSpeedControl").toBool ())
				{
					setVisible (false);
					return;
				}

				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Torrent");
				settings.beginGroup ("FastSpeedControl");
				int num = settings.beginReadArray ("Values");
				for (int i = 0; i < num; ++i)
				{
					settings.setArrayIndex (i);
					int dv = settings.value (Setting_ + "Value").toInt ();
					const auto& str = tr ("%1 KiB/s").arg (dv);
					Call ([dv, str] (QComboBox *box) { box->addItem (str, dv); });
				}
				settings.endArray ();
				settings.endGroup ();

				Call ([] (QComboBox *box) { box->addItem (QString::fromUtf8 ("\u221E"), 0); });
				Call ([] (QComboBox *box) { box->setCurrentIndex (box->count () - 1); });

				setVisible (true);
			}

			void SpeedSelectorAction::syncSpeeds (int s)
			{
				Call ([s] (QComboBox *box) { box->setCurrentIndex (s); });
				emit currentIndexChanged (s);
			}
		};
	};
};

