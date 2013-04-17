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

#include "livestreammanager.h"
#include "livestreamdevice.h"

namespace LeechCraft
{
namespace Plugins
{
namespace BitTorrent
{
	LiveStreamManager::LiveStreamManager (QObject *parent)
	: QObject (parent)
	{
	}

	void LiveStreamManager::EnableOn (libtorrent::torrent_handle handle)
	{
		if (!Handle2Device_.contains (handle))
		{
			qDebug () << Q_FUNC_INFO
				<< "on"
#if LIBTORRENT_VERSION_NUM >= 1600
				<< QString::fromUtf8 (handle.save_path ().c_str ());
#else
				<< QString::fromUtf8 (handle.save_path ().string ().c_str ());
#endif
			LiveStreamDevice *lsd = new LiveStreamDevice (handle, this);
			Handle2Device_ [handle] = lsd;
			connect (lsd,
					SIGNAL (ready ()),
					this,
					SLOT (handleDeviceReady ()));
			lsd->CheckReady ();
		}
	}

	bool LiveStreamManager::IsEnabledOn (libtorrent::torrent_handle handle)
	{
		return Handle2Device_.contains (handle);
	}

	void LiveStreamManager::PieceRead (const libtorrent::read_piece_alert& a)
	{
		libtorrent::torrent_handle handle =
			a.handle;

		if (!Handle2Device_.contains (handle))
		{
			qWarning () << Q_FUNC_INFO
				<< "Handle2Device_ doesn't contain handle"
				<< Handle2Device_.size ();
			return;
		}

		Handle2Device_ [handle]->PieceRead (a);
	}

	void LiveStreamManager::handleDeviceReady ()
	{
		LiveStreamDevice *lsd = qobject_cast<LiveStreamDevice*> (sender ());
		if (!lsd)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender() is not a LiveStreamDevice"
				<< sender ();
			return;
		}

		Entity e;
		e.Entity_ = QVariant::fromValue<QIODevice*> (lsd);
		e.Parameters_ = FromUserInitiated;
		e.Mime_ = "x-leechcraft/media-qiodevice";
		emit gotEntity (e);
	}
}
}
}
