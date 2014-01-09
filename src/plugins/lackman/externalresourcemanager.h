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

#ifndef PLUGINS_LACKMAN_EXTERNALRESOURCEMANAGER_H
#define PLUGINS_LACKMAN_EXTERNALRESOURCEMANAGER_H
#include <boost/optional.hpp>
#include <QObject>
#include <QUrl>
#include <QDir>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

namespace LeechCraft
{
namespace LackMan
{
	/** Manages external resources like images, icons,
		* screenshots and even package files.
		*
		* Resource manager keeps fetched resource in its own cache
		* and is generally asynchronous. Thus, if a requested
		* resource isn't available and it isn't fetched yet,
		* the manager would schedule resource fetching and emit the
		* resourceFetched() signal once the resource is fetched.
		*/
	class ExternalResourceManager : public QObject
	{
		Q_OBJECT

		struct PendingResource
		{
			QUrl URL_;
		};
		QMap<int, PendingResource> PendingResources_;

		QDir ResourcesDir_;
	public:
		ExternalResourceManager (QObject* = 0);

		/** @brief Fetches resource at \em url.
		 *
		 * Starts fetching the resource identified by \em url. After the
		 * resource is fetched, the resourceFetched() signal is emitted.
		 *
		 * If the resource identified by the \em url is already fetched,
		 * this function does nothing.
		 *
		 * @param[in] url URL of the resource to fetch.
		 *
		 * @sa resourceFetched()
		 */
		void GetResourceData (const QUrl& url);

		/** @brief Returns the path of the resource at a given
		 * url.
		 *
		 * This function returns the proper path even if the
		 * resource hasn't been fetched yet. In this case, there
		 * would be just no file at the returned path. The file
		 * at the returned path is guaranteed to exist and be
		 * valid only after resourceFetched() signal has been
		 * emitted for this url or if GetResourceData() returns
		 * proper data.
		 *
		 * @param[in] url URL of the resource to get the path
		 * for.
		 * @return The local path of the fetched copy of the
		 * url.
		 */
		QString GetResourcePath (const QUrl& url) const;

		/** Clears all fetched resources.
			*/
		void ClearCaches ();

		/** @brief Clears fetched resource identified by url.
		 *
		 * If the resource identified by url isn't fetched, this
		 * function does nothing.
		 *
		 * @param[in] url URL of the resource to remove from
		 * cache.
		 */
		void ClearCachedResource (const QUrl& url);
	private slots:
		void handleResourceFinished (int);
		void handleResourceRemoved (int);
		void handleResourceError (int, IDownload::Error);
	signals:
		/** @brief Emitted once the resource identified by url
		 * is fetched.
		 *
		 * After this signal GetResourceData() function is
		 * guaranteed to return valid and actual data for the
		 * resource identified by the emitted url.
		 *
		 * @param[out] url URL of the resource just fetched.
		 *
		 * @sa GetResourceData()
		 */
		void resourceFetched (const QUrl& url);

		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}

#endif
