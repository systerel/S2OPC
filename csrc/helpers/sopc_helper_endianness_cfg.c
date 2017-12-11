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

#include "sopc_helper_endianness_cfg.h"


static SOPC_Endianness endianness_integer = SOPC_Endianness_Undefined;
static SOPC_Endianness endianness_float = SOPC_Endianness_Undefined;


static SOPC_Endianness compute_endianness_integer(void)
{
    uint64_t x = 0x0123456789ABCDEF;
    uint8_t *pX = (uint8_t*) &x;
    SOPC_Endianness endianness = SOPC_Endianness_Undefined;

    if (pX[0] == 0x01 &&
        pX[1] == 0x23 &&
        pX[2] == 0x45 &&
        pX[3] == 0x67 &&
        pX[4] == 0x89 &&
        pX[5] == 0xAB &&
        pX[6] == 0xCD &&
        pX[7] == 0xEF)
    {
        endianness = SOPC_Endianness_BigEndian;
    }
    else if (pX[0] == 0xEF &&
             pX[1] == 0xCD &&
             pX[2] == 0xAB &&
             pX[3] == 0x89 &&
             pX[4] == 0x67 &&
             pX[5] == 0x45 &&
             pX[6] == 0x23 &&
             pX[7] == 0x01)
    {
        endianness = SOPC_Endianness_LittleEndian;
    }

    return endianness;
}


static SOPC_Endianness compute_endianness_float(void)
{
    double d = -0x1.3456789ABCDEFp-1005;
    uint8_t *pD = (uint8_t*) &d;
    SOPC_Endianness endianness = SOPC_Endianness_Undefined;

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
        endianness = SOPC_Endianness_BigEndian;
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
        endianness = SOPC_Endianness_LittleEndian;
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


void SOPC_Helper_EndiannessCfg_Initialize()
{
    endianness_integer = compute_endianness_integer();
    endianness_float = compute_endianness_float();
}


SOPC_Endianness SOPC_Helper_Endianness_GetInteger(void)
{
    return endianness_integer;
}

SOPC_Endianness SOPC_Helper_Endianness_GetFloat(void)
{
    return endianness_float;
}

void SOPC_Helper_Endianness_SetInteger(SOPC_Endianness endianness)
{
    endianness_integer = endianness;
}

void SOPC_Helper_Endianness_SetFloat(SOPC_Endianness endianness)
{
    endianness_float = endianness;
}

