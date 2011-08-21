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
#include "consoleclient.hpp"

#include <cstdio>
#include <iostream>

#include <QCoreApplication>
#include <QTimer>

using namespace std;

namespace Pseudopodia
{
    ConsoleClient::ConsoleClient(QObject *parent) :
        QObject(parent),
        input(stdin),
        output(stdout),
        thread()
    {
        // Start executing loop right after event system activates:
        QTimer::singleShot(0, this, SLOT(init()));
    }

    void ConsoleClient::init()
    {
        thread.start();

        connect(&thread, SIGNAL(initialized(Pseudopodia::Client &)), this,
            SLOT(execute(Pseudopodia::Client &)));        
    }

    void ConsoleClient::execute(Client &client)
    {
        connect(&client, SIGNAL(connected(const quint64 &)), this,
            SLOT(connected(const quint64 &)));
        connect(&client, SIGNAL(connectionError(const quint64 &)), this,
            SLOT(connectionError(const quint64 &)));

        output << "Available commands: connect, exit." << endl;
        while(true)
        {
            output << "> ";
            output.flush();
            QString command = input.readLine();
            if(command == "connect")
            {
                output << "UIN:      ";
                output.flush();
                QString UIN = input.readLine();
                output << "Password: ";
                output.flush();
                QString password = input.readLine();
                client.simpleConnect(UIN.toLongLong(), password);
            }
            else if(command == "exit")
            {
                QCoreApplication::quit();
                return;
            }
            else
                output << "Unknown command." << endl;
            output.flush();
        }
    }
    
    void ConsoleClient::connected(const quint64 &UIN)
    {
        output << "connected(" << UIN << ") signal recieved." << endl;
        output.flush();
    }
    
    void ConsoleClient::connectionError(const quint64 &UIN)
    {
        output << "connectionError(" << UIN << ") signal recieved." << endl;
        output.flush();
    }
}
