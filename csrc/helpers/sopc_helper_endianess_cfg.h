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

#ifndef SOPC_HELPER_ENDIANESS_CFG_H_
#define SOPC_HELPER_ENDIANESS_CFG_H_

typedef enum {
    SOPC_Endianess_Undefined,
    SOPC_Endianess_LittleEndian,
    SOPC_Endianess_BigEndian,
    SOPC_Endianness_FloatARMMiddleEndian
} SOPC_Endianess;

// Undefined before call to initialize
extern SOPC_Endianess sopc_endianess;
extern SOPC_Endianess sopc_floatEndianess;

void SOPC_Helper_EndianessCfg_Initialize(void);

#endif /* SOPC_HELPER_ENDIANESS_CFG_H_ */
