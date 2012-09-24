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

#include "syncmanagerbase.h"

namespace LeechCraft
{
namespace LMP
{
	class ISyncPlugin;
	class TranscodeManager;
	class CopyManager;
	struct TranscodingParams;

	class SyncManager : public SyncManagerBase
	{
		Q_OBJECT

		QMap<QString, CopyManager*> Mount2Copiers_;

		struct SyncTo
		{
			ISyncPlugin *Syncer_;
			QString MountPath_;
		};
		QMap<QString, SyncTo> Source2Params_;
	public:
		SyncManager (QObject* = 0);

		void AddFiles (ISyncPlugin*, const QString& mount, const QStringList&, const TranscodingParams&);
	private:
		void CreateSyncer (const QString&);
	protected slots:
		void handleFileTranscoded (const QString& from, const QString&, QString);
	};
}
}
