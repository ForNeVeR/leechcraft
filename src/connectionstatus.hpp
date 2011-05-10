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

namespace Pseudopodia
{
    /* Describes current connection status.
     */
    enum ConnectionStatus
    {
        Disconnected,
        Connecting,
        Connected
    };
}