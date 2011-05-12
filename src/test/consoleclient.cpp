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
        output(stdout)
    {
        connect(&client, SIGNAL(connected(const quint64 &)), this,
            SLOT(connected(const quint64 &)));
        connect(&client, SIGNAL(connectionError(const quint64 &)), this,
            SLOT(connectionError(const quint64 &)));

        // Start executing loop right after event system activates:
        QTimer::singleShot(0, this, SLOT(execute()));
    }

    void ConsoleClient::execute()
    {
        output << "Available commands: connect, check, exit." << endl;
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
            else if(command == "check")
                QCoreApplication::processEvents();
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
