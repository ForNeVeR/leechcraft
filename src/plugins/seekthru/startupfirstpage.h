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

#ifndef PLUGINS_SEEKTHRU_STARTUPFIRSTPAGE_H
#define PLUGINS_SEEKTHRU_STARTUPFIRSTPAGE_H
#include <QWizardPage>
#include "ui_startupfirstpage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			struct EngineInfo
			{
				QString ResourceFileName_;
				QString DefaultTags_;
				QString Name_;

				EngineInfo (const QString&, const QString&, const QString&);
			};
			typedef QList<EngineInfo> EngineInfos_t;

			class StartupFirstPage : public QWizardPage
			{
				Q_OBJECT

				Ui::SeekThruStartupFirstPageWidget Ui_;
				QMap<QString, EngineInfos_t> Sets_;
				enum
				{
					RoleSet = Qt::UserRole + 127,
					RoleFile
				};
			public:
				StartupFirstPage (QWidget* = 0);

				void initializePage ();
			private:
				void Populate (const QString&);
			private slots:
				void handleAccepted ();
				void handleCurrentIndexChanged (const QString&);
			};
		};
	};
};

#endif

