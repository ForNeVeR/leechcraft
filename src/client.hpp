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
#pragma once

#include <QtGlobal>
#include <QUdpSocket>

class QObject;
class QString;

namespace Pseudopodia
{
    enum ConnectionStatus;

    /* Main client class.
     */
    class Client : public QObject
    {
        Q_OBJECT
    public:
        /* Constructs a Client object.
         */
        Client(QObject *parent);
    
    public slots:
        /* Connects to server.
         *
         * UIN - aka ICQ number.
         * password - you know what ;)
         */
        void simpleConnect(const quint64 &UIN, const QString &password,
            const QString &serverName="login.icq.com",
            const quint16 port=5190);
    
    signals:
        /* Emitted after connection to server initiated successfully.
         */
        void connected(const quint64 &UIN);
        void connectionError(const quint64 &UIN);
        
    private slots:
        void socketConnected();
    
    private:
        QUdpSocket socket;
        
        ConnectionStatus connectionStatus;
    };
}
