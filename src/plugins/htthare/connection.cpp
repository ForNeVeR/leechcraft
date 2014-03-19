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

#include "connection.h"
#include <QtDebug>
#include "requesthandler.h"

namespace LeechCraft
{
namespace HttHare
{
	Connection::Connection (boost::asio::io_service& service,
			const StorageManager& stMgr, IconResolver *resolver, TrManager *trMgr)
	: Strand_ { service }
	, Socket_ { service }
	, StorageMgr_ (stMgr)
	, IconResolver_ { resolver }
	, TrManager_ { trMgr }
	, Buf_ { 2 * 1024 }
	{
	}

	boost::asio::ip::tcp::socket& Connection::GetSocket ()
	{
		return Socket_;
	}

	boost::asio::io_service::strand& Connection::GetStrand ()
	{
		return Strand_;
	}

	IconResolver* Connection::GetIconResolver () const
	{
		return IconResolver_;
	}

	TrManager* Connection::GetTrManager () const
	{
		return TrManager_;
	}

	const StorageManager& Connection::GetStorageManager () const
	{
		return StorageMgr_;
	}

	void Connection::Start ()
	{
		auto conn = shared_from_this ();
		boost::asio::async_read_until (Socket_,
				Buf_,
				std::string { "\r\n\r\n" },
				Strand_.wrap ([conn] (const boost::system::error_code& ec, ulong transferred)
					{ conn->HandleHeader (ec, transferred); }));
	}

	void Connection::HandleHeader (const boost::system::error_code&, unsigned long transferred)
	{
		QByteArray data;
		data.resize (transferred);

		std::istream istr (&Buf_);
		istr.read (data.data (), transferred);

		RequestHandler { shared_from_this () } (data);
	}
}
}
