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
#include <QByteArray>

namespace Pseudopodia
{
    class TLVData;

    class FLAPPacket
    {
    private:
        static const quint8 FLAP_HEADER_START = 0x2A;
        
    public:
        /* Creates empty FLAP packet.
         * channel - FLAP channel ID.
         * seqNumber - packet "sequence number".
         */
        FLAPPacket(qint8 channel, qint16 seqNumber);
        
        /* Adds quint32 as 4 bytes in big-endian order to packet.
         */
        void addRawBytes(const quint32 &bytes);
        
        /* Adds an array of TLV data into packet.
         */
        void addTLVData(const TLVData &data);
        
        /* Converts packet to byte array.
         */
        QByteArray toByteArray() const;
    
    private:
        // Header:
        quint8 channel;
        quint16 seqNumber;
        
        QByteArray packetData;
    };
}
