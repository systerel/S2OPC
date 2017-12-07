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

static SOPC_Endianess compute_float_endianness(void)
{
    double d = -0x1.3456789ABCDEFp-1005;
    uint8_t *pD = (uint8_t*) &d;
    SOPC_Endianess endianness = SOPC_Endianess_Undefined;

    /* Check whether the double is encoded in one of the three known forms.
     * Big endian has sign and exponent bytes first.
     * Little endian has sign and exponent bytes last.
     * ARM's half endianness is big endian in groups of 4 bytes,
     *  but the least significant 4 bytes are first.
     */
    if (pD[0] == 0x81 &&
        pD[1] == 0x23 &&
        pD[2] == 0x45 &&
        pD[3] == 0x67 &&
        pD[4] == 0x89 &&
        pD[5] == 0xAB &&
        pD[6] == 0xCD &&
        pD[7] == 0xEF)
    {
        endianness = SOPC_Endianess_BigEndian;
    }
    else if (pD[0] == 0xEF &&
             pD[1] == 0xCD &&
             pD[2] == 0xAB &&
             pD[3] == 0x89 &&
             pD[4] == 0x67 &&
             pD[5] == 0x45 &&
             pD[6] == 0x23 &&
             pD[7] == 0x81)
    {
        endianness = SOPC_Endianess_LittleEndian;
    }
    else if (pD[0] == 0x67 &&
             pD[1] == 0x45 &&
             pD[2] == 0x23 &&
             pD[3] == 0x81 &&
             pD[4] == 0xEF &&
             pD[5] == 0xCD &&
             pD[6] == 0xAB &&
             pD[7] == 0x89)
    {
        endianness = SOPC_Endianness_FloatARMMiddleEndian;
    }

    return endianness;
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
    sopc_floatEndianess = compute_float_endianness();
}
