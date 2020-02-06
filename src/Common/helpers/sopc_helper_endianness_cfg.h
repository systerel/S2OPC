/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 *  \file
 *
 *  \brief Evaluates and records the endianness configuration of the current machine.
 */

#ifndef SOPC_HELPER_ENDIANNESS_CFG_H_
#define SOPC_HELPER_ENDIANNESS_CFG_H_

/**
 * \brief Supported endianness configurations.
 */
typedef enum
{
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
 * \warning    Solely for tests.
 *
 * \param endianness  The integer endianness.
 */
void SOPC_Helper_Endianness_SetInteger(SOPC_Endianness endianness);

/**
 * \brief   Overrides machine endianness detection for float operations.
 *
 * \warning    Solely for tests.
 *
 * \param endianness  The floating-point endianness.
 */
void SOPC_Helper_Endianness_SetFloat(SOPC_Endianness endianness);

#endif /* SOPC_HELPER_ENDIANNESS_CFG_H_ */
