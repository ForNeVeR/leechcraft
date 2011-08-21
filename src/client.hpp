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
#pragma once

#include <QtGlobal>
#include <QTcpSocket>

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
        Client(QObject *parent=NULL);
    
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
        
        /* Emitted whenever any connection error occurs.
         */
        void connectionError(const quint64 &UIN);
        
    private slots:
        void socketConnected();
    
    private:
        QTcpSocket socket;
        
        quint64 UIN;
        ConnectionStatus connectionStatus;
    };
}
