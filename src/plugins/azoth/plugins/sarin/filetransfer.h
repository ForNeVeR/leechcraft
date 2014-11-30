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

#include <memory>
#include <QObject>
#include <interfaces/azoth/itransfermanager.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	class ToxThread;

	class FileTransfer : public QObject
					   , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ITransferJob)

		const QString AzothId_;
		const QByteArray PubKey_;
		const QString FilePath_;
		const std::shared_ptr<ToxThread> Thread_;

		const int64_t Filesize_;
	public:
		FileTransfer (const QString& azothId,
				const QByteArray& pubkey,
				const QString& filename,
				const std::shared_ptr<ToxThread>& thread,
				QObject *parent = nullptr);

		QString GetSourceID () const override;
		QString GetName () const override;
		qint64 GetSize () const override;
		QString GetComment () const override;
		TransferDirection GetDirection () const override;

		void Accept (const QString&) override;
		void Abort () override;
	signals:
		void transferProgress (qint64 done, qint64 total) override;
		void errorAppeared (TransferError error, const QString& msg) override;
		void stateChanged (TransferState state) override;
	};
}
}
}
