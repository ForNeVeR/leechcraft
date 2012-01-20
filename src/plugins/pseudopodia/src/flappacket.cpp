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
