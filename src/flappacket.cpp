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
#include "flappacket.hpp"

#include <QtEndian>
#include <QByteArray>

#include "tlvdata.hpp"
#include "utils.hpp"

namespace Pseudopodia
{
    FLAPPacket::FLAPPacket(qint8 channel, qint16 seqNumber) :
        channel(channel),
        seqNumber(seqNumber)
    {

    }

    void FLAPPacket::addRawBytes(const quint32 &bytes)
    {
        packetData.append(Utils::toBigEndian(bytes));
    }
    
    void FLAPPacket::addTLVData(const TLVData &data)
    {
        packetData.append(data.toByteArray());
    }
    
    QByteArray FLAPPacket::toByteArray() const
    {
        QByteArray packet;
        packet.append(FLAP_HEADER_START);
        packet.append(channel);
        packet.append(Utils::toBigEndian(seqNumber));
        
        quint16 length = packetData.length();
        packet.append(Utils::toBigEndian(length));
        
        packet.append(packetData);
        
        return packet;
    }
}
