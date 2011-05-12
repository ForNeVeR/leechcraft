/* This file is part of Pseudopodia.
 * Copyright (C) 2011 ForNeVeR
 *
 * Pseudopodia is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Pseudopodia is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Pseudopodia. If not, see <http://www.gnu.org/licenses/>.
 */
#include "client.hpp"

#include <QString>

#include "ConnectionStatus.hpp"

namespace Pseudopodia
{
    Client::Client(QObject *parent) :
        QObject(parent),
        connectionStatus(Disconnected),
        UIN(0)
    {
        connect(&socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    }
    
    void Client::simpleConnect(const quint64 &UIN, const QString &password,
            const QString &serverName, const quint16 port)
    {
        if (connectionStatus == Disconnected)
        {
            connectionStatus = Connecting;
            this->UIN = UIN;
            qDebug() << "Trying to connect " << serverName << " port " <<
                port << ".";
            socket.connectToHost(serverName, port);
        }
        else
        {
            emit connectionError(UIN);
        }
    }
    
    void Client::socketConnected()
    {
        qDebug() << "socketConnected()" << endl;
        // TODO: Send cli_ident packet.
        connectionStatus = Connected;
        emit connected(UIN);
    }
}
