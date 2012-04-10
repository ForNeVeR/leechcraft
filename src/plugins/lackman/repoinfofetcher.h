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

#ifndef PLUGINS_LACKMAN_REPOINFOFETCHER_H
#define PLUGINS_LACKMAN_REPOINFOFETCHER_H
#include <QObject>
#include <QUrl>
#include <QProcess>
#include <QHash>
#include <interfaces/idownload.h>
#include "repoinfo.h"

namespace LeechCraft
{
namespace LackMan
{
	class RepoInfoFetcher : public QObject
	{
		Q_OBJECT

		struct PendingRI
		{
			QUrl URL_;
			QString Location_;
		};
		QHash<int, PendingRI> PendingRIs_;

		struct PendingComponent
		{
			QUrl URL_;
			QString Location_;
			QString Component_;
			int RepoID_;
		};
		QHash<int, PendingComponent> PendingComponents_;

		struct ScheduledPackageFetch
		{
			QUrl BaseUrl_;
			QString PackageName_;
			QList<QString> NewVersions_;
			int ComponentId_;
		};
		QList<ScheduledPackageFetch> ScheduledPackages_;

		struct PendingPackage
		{
			QUrl URL_;
			QUrl BaseURL_;
			QString Location_;
			QString PackageName_;
			QList<QString> NewVersions_;
			int ComponentId_;
		};
		QHash<int, PendingPackage> PendingPackages_;
	public:
		RepoInfoFetcher (QObject*);

		void FetchFor (QUrl);
		void FetchComponent (QUrl, int, const QString& component);
		void ScheduleFetchPackageInfo (const QUrl& url,
				const QString& name,
				const QList<QString>& newVers,
				int componentId);
	private:
		void FetchPackageInfo (const QUrl& url,
				const QString& name,
				const QList<QString>& newVers,
				int componentId);
	private slots:
		void rotatePackageFetchQueue ();

		void handleRIFinished (int);
		void handleRIRemoved (int);
		void handleRIError (int, IDownload::Error);
		void handleComponentFinished (int);
		void handleComponentRemoved (int);
		void handleComponentError (int, IDownload::Error);
		void handlePackageFinished (int);
		void handlePackageRemoved (int);
		void handlePackageError (int, IDownload::Error);

		void handleRepoUnarchFinished (int, QProcess::ExitStatus);
		void handleComponentUnarchFinished (int, QProcess::ExitStatus);
		void handlePackageUnarchFinished (int, QProcess::ExitStatus);
		void handleUnarchError (QProcess::ProcessError);
	signals:
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);

		void infoFetched (const RepoInfo&);
		void componentFetched (const PackageShortInfoList& packages,
				const QString& component, int repoId);
		void packageFetched (const PackageInfo&, int componentId);
	};
}
}

#endif
