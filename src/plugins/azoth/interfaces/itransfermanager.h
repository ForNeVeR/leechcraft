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

#ifndef PLUGINS_AZOTH_INTERFACES_ITRANSFERMANAGER_H
#define PLUGINS_AZOTH_INTERFACES_ITRANSFERMANAGER_H
#include <QObject>
#include <QString>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Represents the direction of the transfer.
	 */
	enum TransferDirection
	{
		/** File is transferred from remote party to us.
		 */
		TDIn,
		/** File is transferred from us to remote party.
		 */
		TDOut
	};

	/** @brief Represents the state of the file transfer job.
	 */
	enum TransferState
	{
		/** File is just offered and waiting for the other party to be
		 * accepted or rejected.
		 */
		TSOffer,
		
		/** Transfer is accepted by the remote party and is being
		 * initiated.
		 */
		TSStarting,
		
		/** File transfer is in progress.
		 */
		TSTransfer,
		
		/** File transfer is finished.
		 */
		TSFinished
	};

	/** @brief Represents the error condition of the transfer.
	 */
	enum TransferError
	{
		/** There is no error.
		 */
		TENoError,
		
		/** Transfer is refused by the other party or aborted in the
		 * progress.
		 */
		TEAborted,
		
		/** Error occured while trying to access the local file: read
		 * failure in case of outgoing transfer and write error in case
		 * of incoming transfer.
		 */
		TEFileAccessError,
		
		/** File is found to be corrupted during the transfer.
		 */
		TEFileCorruptError,
		
		/** A protocol error occurred.
		 */
		TEProtocolError
	};

	/** @brief This interface must be implemented by objects
	 * representing file transfer jobs.
	 */
	class ITransferJob
	{
	public:
		virtual ~ITransferJob () {}

		/** @brief Returns the ID of the other party.
		 * 
		 * The returned string should be compatible with the return
		 * value of ICLEntry::GetEntryID(), that is, it should be equal
		 * to that one of the corresponding ICLEntry.
		 * 
		 * @return The ID of the other party, as ICLEntry::GetEntryID().
		 */
		virtual QString GetSourceID () const = 0;

		/** @brief Returns the name of the file.
		 * 
		 * @return The name of the file.
		 */
		virtual QString GetName () const = 0;

		/** @brief Returns the size of the file.
		 * 
		 * @return The size of the file, or -1 if size is not known.
		 */
		virtual qint64 GetSize () const = 0;

		/** @brief Returns the direction of the transfer.
		 * 
		 * @return The direction of the transfer.
		 */
		virtual TransferDirection GetDirection () const = 0;

		/** @brief Accepts an incoming transfer.
		 * 
		 * This method only makes sense in incoming file transfers. It
		 * is used to accept the transfer and write the file the path
		 * given by the out parameter.
		 * 
		 * @param[in] out The file path to save the incoming data to.
		 */
		virtual void Accept (const QString& out) = 0;

		/** @brief Rejects or aborts a transfer.
		 * 
		 * This method is used to reject an incoming file transfer
		 * request or abort an already accepted one that's in progress.
		 */
		virtual void Abort () = 0;

		/** @brief Notifies about transfer progress.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] done The amount of data already transferred.
		 * @param[out] total The total amount of data.
		 */
		virtual void transferProgress (qint64 done, qint64 total) = 0;

		/** @brief Notifies about error.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] error The error condition.
		 * @param[out] msg The human-readable message describing the error.
		 */
		virtual void errorAppeared (TransferError error, const QString& msg) = 0;

		/** @brief Notifies about state changes.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] state The new state of the transfer job.
		 */
		virtual void stateChanged (TransferState state) = 0;
	};

	/** @brief This interface must be implemented by transfer managers
	 * returned from IAccount::GetTransferManager().
	 */
	class ITransferManager
	{
	public:
		virtual ~ITransferManager () {}

		/** @brief Requests a file transfer with the remote party.
		 * 
		 * The entry is identified by the ID, which is the result of
		 * ICLEntry::GetEntryID().
		 * 
		 * If the variant is an empty string, or there is no such
		 * variant, the file should be transferred the the variant with
		 * the highest priority.
		 * 
		 * The returned object represents the file transfer request,
		 * and, further on, the file transfer job, should it be
		 * accepted. The returned object must implement ITransferJob.
		 * Ownership is transferred to the caller.
		 * 
		 * @param[in] id The id of the remote party, as
		 * ICLEntry::GetEntryID().
		 * @param[in] variant The entry variant to transfer with.
		 * @param[in] name The path to the file that should be
		 * transferred.
		 * @return The transfer job object representing this transfer
		 * and implement ITransferJob.
		 */
		virtual QObject* SendFile (const QString& id,
				const QString& variant, const QString& name) = 0;

		/** @brief Notifies about incoming transfer request.
		 * 
		 * This signal should be emitted by the transfer manager
		 * whenever another party issues a file transfer request.
		 * 
		 * The passed obj represents the transfer job, and it must
		 * implement the ITransferJob interface.
		 * 
		 * Ownership of the obj is transferred to the signal handler.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] job The transfer job, implementing ITransferJob.
		 */
		virtual void fileOffered (QObject *job) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ITransferJob,
		"org.Deviant.LeechCraft.Azoth.ITransferJob/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::ITransferManager,
		"org.Deviant.LeechCraft.Azoth.ITransferManager/1.0");

#endif
