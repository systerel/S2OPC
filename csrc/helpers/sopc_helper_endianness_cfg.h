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

/**
 * \brief Supported endianness configurations.
 */
typedef enum {
    SOPC_Endianness_Undefined,
    SOPC_Endianness_LittleEndian,
    SOPC_Endianness_BigEndian,
    SOPC_Endianness_FloatARMMiddleEndian
} SOPC_Endianness;

/**
 * \brief Initializes machine endianness detection.
 */
void SOPC_Helper_EndiannessCfg_Initialize(void);

/**
 * \brief   Gets the endianness for integer operations.
 *
 * \return  SOPC_Endianness_Undefined when machine endianness is neither little endian nor big endian.
 */
SOPC_Endianness SOPC_Helper_Endianness_GetInteger(void);

/**
 * \brief   Gets the endianness for floating-point operations.
 *
 * \return  SOPC_Endianness_Undefined when machine endianness is not little endian, big endian, or ARM's special middle
 * endian.
 */
SOPC_Endianness SOPC_Helper_Endianness_GetFloat(void);

/**
 * \brief   Overrides machine endianness detection for integer operations.
 *
 * \warn    Solely for tests.
 *
 * \param endianness  The integer endianness.
 */
void SOPC_Helper_Endianness_SetInteger(SOPC_Endianness endianness);

/**
 * \brief   Overrides machine endianness detection for float operations.
 *
 * \warn    Solely for tests.
 *
 * \param endianness  The floating-point endianness.
 */
void SOPC_Helper_Endianness_SetFloat(SOPC_Endianness endianness);

#endif /* SOPC_HELPER_ENDIANNESS_CFG_H_ */
