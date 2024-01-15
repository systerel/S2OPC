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

#include "sopc_common_constants.h"

/** SWAP OPERATIONS */
#define SOPC_SWAP_2_BYTES(x) (uint16_t)(((x) & (uint16_t) 0x00FF) << 8 | ((x) & (uint16_t) 0xFF00) >> 8)
#define SOPC_SWAP_3_BYTES(x) (((x) &0x0000FF) << 16 | ((x) &0x00FF00) | ((x) &0xFF0000) >> 16)
#define SOPC_SWAP_4_BYTES(x) \
    (((x) &0x000000FF) << 24 | ((x) &0x0000FF00) << 8 | ((x) &0xFF000000) >> 24 | ((x) &0x00FF0000) >> 8)
#define SOPC_SWAP_8_BYTES(x)                                                                               \
    (((x) &0x00000000000000FF) << 56 | ((x) &0x000000000000FF00) << 40 | ((x) &0x0000000000FF0000) << 24 | \
     ((x) &0x00000000FF000000) << 8 | ((x) &0xFF00000000000000) >> 56 | ((x) &0x00FF000000000000) >> 40 |  \
     ((x) &0x0000FF0000000000) >> 24 | ((x) &0x000000FF00000000) >> 8)
#define SOPC_SWAP_2_DWORDS(x) (((x) &0x00000000FFFFFFFF) << 32 | ((x) &0xFFFFFFFF00000000) >> 32)

/**
 * \brief Check that machine endianness configured (:SOPC_IS_LITTLE_ENDIAN and ::SOPC_IS_DOUBLE_MIDDLE_ENDIAN)
 *  matches actual CPU behavior.
 */
void SOPC_Helper_Endianness_Check(void);

/** Define integer swapping operation to encode Integers in buffer (As little endian) */
#if SOPC_IS_LITTLE_ENDIAN
#define SOPC_TO_LITTLE_ENDIAN_16BITS(iu16) (void) (iu16)
#define SOPC_TO_LITTLE_ENDIAN_32BITS(iu32) (void) (iu32)
#define SOPC_TO_LITTLE_ENDIAN_64BITS(iu64) (void) (iu64)
#define SOPC_TO_LITTLE_ENDIAN_FLOAT(fl32) (void) (fl32)
#define SOPC_TO_LITTLE_ENDIAN_DOUBLE(db64) (void) (db64)
#else
#define SOPC_TO_LITTLE_ENDIAN_16BITS(iu16) (iu16) = SOPC_SWAP_2_BYTES(iu16)
#define SOPC_TO_LITTLE_ENDIAN_32BITS(iu32) (iu32) = SOPC_SWAP_4_BYTES(iu32)
#define SOPC_TO_LITTLE_ENDIAN_32BITS(iu64) (iu64) = SOPC_SWAP_8_BYTES(iu64)
#define SOPC_TO_LITTLE_ENDIAN_FLOAT(fl32) (fl32) = SOPC_SWAP_4_BYTES(fl32)
#define SOPC_TO_LITTLE_ENDIAN_DOUBLE(db64) (db64) = SOPC_SWAP_8_BYTES(db64)
#endif

#if SOPC_IS_DOUBLE_MIDDLE_ENDIAN
#undef SOPC_TO_LITTLE_ENDIAN_DOUBLE
#define SOPC_TO_LITTLE_ENDIAN_DOUBLE(db64) (db64) = SOPC_SWAP_2_DWORDS(db64)
#endif

#endif /* SOPC_HELPER_ENDIANNESS_CFG_H_ */
