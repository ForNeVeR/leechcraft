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
