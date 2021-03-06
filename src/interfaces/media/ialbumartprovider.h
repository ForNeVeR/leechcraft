/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#pragma once

#include <QString>
#include <QList>
#include <QImage>
#include <QHash>
#include <QMetaType>

class QUrl;

namespace Media
{
	/** @brief Information about an album used in IAlbumArtProvider.
	 */
	struct AlbumInfo
	{
		/** @brief The artist name of this album.
		 */
		QString Artist_;

		/** @brief The album name.
		 */
		QString Album_;
	};

	/** @brief Compares to AlbumInfo structures.
	 */
	inline bool operator== (const AlbumInfo& a1, const AlbumInfo& a2)
	{
		return a1.Artist_ == a2.Artist_ &&
			a1.Album_ == a2.Album_;
	}

	/** @brief A hash function for AlbumInfo to use it in QHash.
	 */
	inline uint qHash (const AlbumInfo& info)
	{
		return qHash (info.Album_.toUtf8 () + '\0' + info.Artist_.toUtf8 ());
	}

	/** @brief Pending album art search handle.
	 *
	 * Interface for a pending album art search in an IAlbumArtProvider.
	 * An object implementing this interface is returned from
	 * IAlbumArtProvider::RequestAlbumArt() method and is used to track
	 * the status of album art requests.
	 *
	 * This class has some signals (ready() and urlsReady()), and the
	 * GetQObject() method can be used to get an object of this class as
	 * a QObject to connect to those signals.
	 *
	 * The urlsReady() signal is emitted as soon as the URLs of the
	 * album images are fetched, and then the user of this object can
	 * either wait for the convenience ready() signal after which the
	 * object would be destroyed, or delete the object himself.
	 *
	 * @note The object of this class should schedule its deletion (via
	 * <code>QObject::deleteLater()</code>, for example) after the ready()
	 * signal is emitted. Thus the calling code should never delete it
	 * explicitly after this signal, neither it should use this object
	 * after ready() signal or connect to its signals via
	 * <code>Qt::QueuedConnection</code>.
	 *
	 * @sa IAlbumArtProvider
	 */
	class IPendingAlbumArt
	{
	public:
		virtual ~IPendingAlbumArt () {}

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the information about the album.
		 *
		 * The returned object is invalid if called before the urlsReady()
		 * signal is emitted.
		 *
		 * @return The information about the album.
		 *
		 * @sa urlsReady()
		 * @sa ready()
		 */
		virtual AlbumInfo GetAlbumInfo () const = 0;

		/** @brief Returns the fetched cover art for the album.
		 *
		 * The returned list is empty if called before the ready() signal
		 * is emitted.
		 *
		 * @return The list of fetched cover art corresponding to this
		 * album.
		 *
		 * @sa ready()
		 */
		virtual QList<QImage> GetImages () const = 0;

		/** @brief Returns the URLs of the covert art for the album.
		 *
		 * The returned list is empty if called before the urlsReady()
		 * signal is emitted.
		 *
		 * @return The list of covert art images URLs corresponding to
		 * this album.
		 */
		virtual QList<QUrl> GetImageUrls () const = 0;
	protected:
		/** @brief Emitted when the album info and the cover art is ready
		 * and fetched.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 *
		 * If you only need the URLs of the images but not the images
		 * themselves (like if you will show the images by URLs in a QML
		 * view), consider deleting this object after urlsReady() signal
		 * instead of waiting for this signal.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] info The information about the album.
		 * @param[out] images The images corresponding to this album.
		 *
		 * @sa urlsReady()
		 */
		virtual void ready (const AlbumInfo& info, const QList<QImage>& images) = 0;

		/** @brief Emitted when the album info and the URLs of cover art
		 * are ready.
		 *
		 * After emitting this signal the object will start fetching the
		 * images at the given \em urls and emit ready() after fetching
		 * them. If the images themselves are not required, some bandwidth
		 * and CPU cycles can be saved by deleting the object in a slot
		 * connected to this signal.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] info The information about the album.
		 * @param[out] urls The cover art corresponding to this album.
		 *
		 * @sa ready()
		 */
		virtual void urlsReady (const AlbumInfo& info, const QList<QUrl>& urls) = 0;
	};

	/** @brief Interface for plugins that can search for album art.
	 *
	 * Plugins that can search for album art (like on Amazon or Last.FM)
	 * should implement this interface.
	 *
	 * Album art lookup is asynchronous in nature: one first initiates a
	 * search via RequestAlbumArt() method and then listens for the
	 * gotAlbumArt() signal.
	 */
	class Q_DECL_EXPORT IAlbumArtProvider
	{
	public:
		virtual ~IAlbumArtProvider () {}

		/** @brief Returns the human-readable name of this provider.
		 *
		 * @return The human-readable name of the provider, like Last.FM.
		 */
		virtual QString GetAlbumArtProviderName () const = 0;

		/** @brief Initiates search for album art of the given album.
		 *
		 * This function initiates searching for the album art of the
		 * given \em album and returns a search proxy that can be used to
		 * be notified when the search finishes.
		 *
		 * @param[in] album The information about the album.
		 * @return The pending search object that will emit
		 * IPendingAlbumArt::ready() signal once ready.
		 */
		virtual IPendingAlbumArt* RequestAlbumArt (const AlbumInfo& album) const = 0;
	};
}

Q_DECLARE_METATYPE (Media::AlbumInfo)
Q_DECLARE_INTERFACE (Media::IAlbumArtProvider, "org.LeechCraft.Media.IAlbumArtProvider/1.0")
Q_DECLARE_INTERFACE (Media::IPendingAlbumArt, "org.LeechCraft.Media.IPendingAlbumArt/1.0")
