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

#ifndef SOPC_HELPER_ENDIANNESS_CFG_H_
#define SOPC_HELPER_ENDIANNESS_CFG_H_

typedef enum {
    SOPC_Endianness_Undefined,
    SOPC_Endianness_LittleEndian,
    SOPC_Endianness_BigEndian,
    SOPC_Endianness_FloatARMMiddleEndian
} SOPC_Endianness;

void SOPC_Helper_EndiannessCfg_Initialize(void);
SOPC_Endianness SOPC_Helper_Endianness_GetInteger(void);
SOPC_Endianness SOPC_Helper_Endianness_GetFloat(void);
void SOPC_Helper_Endianness_SetInteger(SOPC_Endianness endianness);
void SOPC_Helper_Endianness_SetFloat(SOPC_Endianness endianness);

#endif /* SOPC_HELPER_ENDIANNESS_CFG_H_ */
