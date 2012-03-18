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

#ifndef PLUGINS_BITTORRENT_FIRSTSTEP_H
#define PLUGINS_BITTORRENT_FIRSTSTEP_H
#include <QWizardPage>
#include "ui_newtorrentfirststep.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class FirstStep : public QWizardPage, private Ui::NewTorrentFirstStep
			{
				Q_OBJECT
			public:
				FirstStep (QWidget *parent = 0);

				bool isComplete () const;
			private:
				QString PrepareDirectory () const;
			private slots:
				void on_BrowseOutput__released ();
				void on_BrowseFile__released ();
				void on_BrowseDirectory__released ();
			};
		};
	};
};

#endif

