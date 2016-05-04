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

#include <QVector>
#include <QFile>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/alert_types.hpp>

namespace LeechCraft
{
namespace BitTorrent
{
	class CachedStatusKeeper;

	class LiveStreamDevice : public QIODevice
	{
		Q_OBJECT

		CachedStatusKeeper * const StatusKeeper_;

		const libtorrent::torrent_handle Handle_;
		const libtorrent::torrent_info TI_;
		const int NumPieces_ = TI_.num_pieces ();

		int LastIndex_ = 0;
		// Which piece would be read next.
		int ReadPos_ = 0;
		// Offset in the next piece pointed by ReadPos_;
		int Offset_ = 0;
		bool IsReady_ = 0;
		QFile File_;
	public:
		LiveStreamDevice (const libtorrent::torrent_handle&, CachedStatusKeeper*, QObject* = nullptr);

		virtual qint64 bytesAvailable () const;
		virtual bool isSequential () const;
		virtual bool isWritable () const;
		virtual bool open (OpenMode);
		virtual qint64 pos () const;
		virtual bool seek (qint64);
		virtual qint64 size () const;

		void PieceRead (const libtorrent::read_piece_alert&);
		void CheckReady ();
	protected:
		virtual qint64 readData (char*, qint64);
		virtual qint64 writeData (const char*, qint64);
	private:
		void CheckNextChunk ();
	private slots:
		void reschedule ();
	signals:
		void ready (LiveStreamDevice*);
	};
}
}
