/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_hash.h"

uint64_t SOPC_DJBHash(const uint8_t* data, size_t len)
{
    return SOPC_DJBHash_Step(5381, data, len);
}

uint64_t SOPC_DJBHash_Step(uint64_t current, const uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        current = (current << 5) + current + data[i];
    }

    return current;
}
