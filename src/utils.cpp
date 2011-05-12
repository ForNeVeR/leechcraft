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
#include "utils.hpp"

#include <QtEndian>

namespace Pseudopodia
{
    namespace Utils
    {
        QByteArray toBigEndian(quint16 word)
        {
            quint8 data[2] = { 0 };
            qToBigEndian(word, data);
            return QByteArray((char *)data, 2);
        }
    
        QByteArray toBigEndian(quint32 quad)
        {
            quint8 data[4] = { 0 };
            qToBigEndian(quad, data);
            return QByteArray((char *)data, 4);
        }
    }
}
