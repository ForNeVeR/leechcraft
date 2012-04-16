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

#ifndef PLUGINS_AUSCRIE_AUSCRIE_H
#define PLUGINS_AUSCRIE_AUSCRIE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>

namespace LeechCraft
{
namespace Auscrie
{
	class ShooterDialog;

	class Plugin : public QObject
					, public IInfo
					, public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IActionsExporter)

		ICoreProxy_ptr Proxy_;
		QAction *ShotAction_;
		ShooterDialog *Dialog_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	private slots:
		void makeScreenshot ();
		void shoot ();
	private:
		QPixmap GetPixmap () const;
		void Post (const QByteArray&);
	signals:
		void gotEntity (const LeechCraft::Entity&);

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

#endif
