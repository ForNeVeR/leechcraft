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

#ifndef PLUGINS_SYNCER_SERVERCONNECTION_H
#define PLUGINS_SYNCER_SERVERCONNECTION_H
#include <QObject>

class QTcpSocket;

namespace LeechCraft
{
namespace Syncer
{
	class ServerConnection : public QObject
	{
		Q_OBJECT

		QTcpSocket *Socket_;
		QByteArray Chain_;
	public:
		enum ErrorCode
		{
			ECUnknownCommand = 0,
			ECUserRegistered,
			ECUserNotRegistered,
			ECWrongPassword,
			ECAlreadyConnected,
			ECOddFilterParameters,
			ECWrongDeltaID
		};

		ServerConnection (const QByteArray&, QObject* = 0);

		static QByteArray FmtMsg (const QList<QByteArray>&);
		static QList<QByteArray> UnfmtMsg (const QByteArray&);

	public slots:
		void performLogin ();
		void reqMaxDelta ();
		void getDeltas (quint32 from);
		void putDeltas (const QList<QByteArray>&, quint32);
	private slots:
		void handleConnected ();
		void handleReadyRead ();
	signals:
		void success (const QList<QByteArray>&);
		void fail ();
		void deltaOutOfOrder ();
		void maxDeltaIDReceived (quint32);
	};
}
}

#endif
