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

#ifndef SOPC_NUMERIC_RANGE_H
#define SOPC_NUMERIC_RANGE_H

#include <stddef.h>
#include <stdint.h>

#include "sopc_enums.h"

typedef struct _SOPC_Dimension
{
    uint32_t start; // Inclusive
    uint32_t end;   // Inclusive
} SOPC_Dimension;

typedef struct _SOPC_NumericRange
{
    size_t n_dimensions;
    SOPC_Dimension* dimensions;
} SOPC_NumericRange;

/**
 * \brief Parses a numeric range as described in Part 4 § 7.22
 *
 * \param range   the string describing the range
 * \param result  the allocated parsed range in case of success, or NULL in case
 *                of failure
 *
 * \return \c SOPC_STATUS_OK in case of success, or an error code in case of
 *         failure.
 *
 * The specification does not explicitly tell on how many bits range bounds are
 * to be encoded. Array/string/bytestring sizes in OPC-UA are 32 bit signed
 * integers. In order to reflect the positive nature of range bounds while
 * covering the range of valid value sizes and minimizing memory usage, we use
 * unsigned 32 bit integers for range bounds. Passing range bounds that don't
 * fit in an unsigned 32 bit integer will make this function return
 * \c SOPC_STATUS_NOK .
 */
SOPC_ReturnStatus SOPC_NumericRange_Parse(const char* range, SOPC_NumericRange** result);
void SOPC_NumericRange_Delete(SOPC_NumericRange* range);

#endif
