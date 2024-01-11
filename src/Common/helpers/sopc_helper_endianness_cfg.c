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

#include <stdint.h>

#include "sopc_helper_endianness_cfg.h"

#include "sopc_assert.h"

static inline void check_integer_endianness(void)
{
    uint64_t x = 0x0123456789ABCDEF;
    uint8_t* pX = (uint8_t*) &x;
    SOPC_ASSERT((SOPC_IS_LITTLE_ENDIAN && (pX[0] == 0xEF && pX[1] == 0xCD && pX[2] == 0xAB && pX[3] == 0x89 &&
                                           pX[4] == 0x67 && pX[5] == 0x45 && pX[6] == 0x23 && pX[7] == 0x01)) ||
                ((!SOPC_IS_LITTLE_ENDIAN) && (pX[0] == 0xEF && pX[1] == 0xCD && pX[2] == 0xAB && pX[3] == 0x89 &&
                                              pX[4] == 0x67 && pX[5] == 0x45 && pX[6] == 0x23 && pX[7] == 0x01)));
}

static inline void check_float_endianness(void)
{
    double d = -0x1.3456789ABCDEFp-1005;
    uint8_t* pD = (uint8_t*) &d;

    /* Check whether the double is encoded in one of the three known forms.
     * Big endian has sign and exponent bytes first.
     * Little endian has sign and exponent bytes last.
     * ARM's half endianness is big endian in groups of 4 bytes,
     *  but the least significant 4 bytes are first.
     */
    SOPC_ASSERT((SOPC_IS_DOUBLE_MIDDLE_ENDIAN && (pD[0] == 0x67 && pD[1] == 0x45 && pD[2] == 0x23 && pD[3] == 0x81 &&
                                                  pD[4] == 0xEF && pD[5] == 0xCD && pD[6] == 0xAB && pD[7] == 0x89)) ||
                (SOPC_IS_LITTLE_ENDIAN && (pD[0] == 0xEF && pD[1] == 0xCD && pD[2] == 0xAB && pD[3] == 0x89 &&
                                           pD[4] == 0x67 && pD[5] == 0x45 && pD[6] == 0x23 && pD[7] == 0x81)) ||
                ((!SOPC_IS_LITTLE_ENDIAN) && (pD[0] == 0x81 && pD[1] == 0x23 && pD[2] == 0x45 && pD[3] == 0x67 &&
                                              pD[4] == 0x89 && pD[5] == 0xAB && pD[6] == 0xCD && pD[7] == 0xEF)));
}

void SOPC_Helper_EndiannessCfg_Initialize(void)
{
    check_integer_endianness();
    check_float_endianness();
}
