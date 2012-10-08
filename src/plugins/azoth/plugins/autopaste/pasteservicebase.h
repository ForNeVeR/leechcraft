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

#pragma once

#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;

namespace LeechCraft
{
struct Entity;

namespace Azoth
{
namespace Autopaste
{
	enum class Highlight
	{
		None,
		C,
		CPP,
		CPP0x,
		Haskell,
		Java,
		Python,
		XML
	};

	class PasteServiceBase : public QObject
	{
		Q_OBJECT

		QPointer<QObject> Entry_;
	public:
		struct PasteParams
		{
			QNetworkAccessManager *NAM_;
			QString Text_;
			Highlight High_;
		};

		PasteServiceBase (QObject *entry, QObject* = 0);

		virtual void Paste (const PasteParams&) = 0;
	protected:
		void InitReply (QNetworkReply*);
		void FeedURL (const QString&);
	protected slots:
		virtual void handleMetadata ();
		virtual void handleFinished ();
		virtual void handleError ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
