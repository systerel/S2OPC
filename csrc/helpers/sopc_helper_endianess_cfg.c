/*
 *  Copyright (C) 2016 Systerel and others.
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

#include <assert.h>
#include <stdint.h>

#include "sopc_helper_endianess_cfg.h"

SOPC_Endianess sopc_endianess = SOPC_Endianess_Undefined;
SOPC_Endianess sopc_floatEndianess = SOPC_Endianess_Undefined;

static uint32_t little_endian(void)
{
    uint32_t x = 0x0001;
    return (x == *((uint8_t*) &x));
}

static uint32_t float_big_endian(void)
{
    float f = -0.0;
    // Check if sign bit is the first
    // GCC version with binary constants extension
    //    assert((0b10000000 & *((char*) &f)) == 0b10000000 ||
    //           (0b10000000 & ((char*) &f)[3]) == 0b10000000);
    //    return (0b10000000 & *((char*) &f)) == 0b10000000;
    assert((0x80 & *((char*) &f)) == 0x80 || (0x80 & ((char*) &f)[3]) == 0x80);
    return (0x80 & *((char*) &f)) == 0x80;
}

void SOPC_Helper_EndianessCfg_Initialize()
{
    if (little_endian() == 0)
    {
        sopc_endianess = SOPC_Endianess_BigEndian;
    }
    else
    {
        sopc_endianess = SOPC_Endianess_LittleEndian;
    }
    if (float_big_endian() == 0)
    {
        sopc_floatEndianess = SOPC_Endianess_LittleEndian;
    }
    else
    {
        sopc_floatEndianess = SOPC_Endianess_BigEndian;
    }
}
