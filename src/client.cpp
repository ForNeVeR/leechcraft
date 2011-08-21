/* Copyright (C) 2011 by ForNeVeR
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
