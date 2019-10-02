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

#ifndef SOPC_HELPER_STRING_H_
#define SOPC_HELPER_STRING_H_

#include <inttypes.h>
#include <stddef.h>

#include "sopc_enums.h"

/**
 *  \brief Compare 2 string in a case-insensitive manner.
 *  Comparison returns 0 if \p size characters were considered identical
 *  or \p s1 and \p s2 were identical and terminated by a '\0' character.
 *
 *  \param s1    A non null string terminated by '\0' character
 *  \param s2    A non null string terminated by '\0' character
 *  \param size  Maximum number of characters compared for computing result.
 *
 *  \return      0 if string are identical in a case-insensitive way, -1 if s1 < s2 and +1 if s1 > s2
 *              (based on first lower case character value comparison).
 */
int SOPC_strncmp_ignore_case(const char* s1, const char* s2, size_t size);

/**
 * \brief      Read a uint8_t from the string with strtoul.
 *
 * \param sz   A pointer to the CString containing the number.
 * \param n    A pointer to the uint8_t.
 * \param base The base in which the number is written. See strtoul for more about /p base.
 * \param cEnd Termination char. The first char of the unconsumed part of sz shall be this.
 *             May be '\0'.
 *
 * \return     SOPC_STATUS_OK if read was done successfully, in which case *n is modified.
 */
SOPC_ReturnStatus SOPC_strtouint8_t(const char* sz, uint8_t* n, int base, char cEnd);

/**
 * \brief      Read a uint16_t from the string with strtoul.
 *
 * \param sz   A pointer to the CString containing the number.
 * \param n    A pointer to the uint16_t.
 * \param base The base in which the number is written. See strtoul for more about /p base.
 * \param cEnd Termination char. The first char of the unconsumed part of sz shall be this.
 *             May be '\0'.
 *
 * \return     SOPC_STATUS_OK if read was done successfully, in which case *n is modified.
 */
SOPC_ReturnStatus SOPC_strtouint16_t(const char* sz, uint16_t* n, int base, char cEnd);

/**
 * \brief      Read a uint32_t from the string with strtoul.
 *
 * \param sz   A pointer to the CString containing the number.
 * \param n    A pointer to the uint32_t.
 * \param base The base in which the number is written. See strtoul for more about /p base.
 * \param cEnd Termination char. The first char of the unconsumed part of sz shall be this.
 *             May be '\0'.
 *
 * \return     SOPC_STATUS_OK if read was done successfully, in which case *n is modified.
 */
SOPC_ReturnStatus SOPC_strtouint32_t(const char* sz, uint32_t* n, int base, char cEnd);

#endif /* SOPC_HELPER_STRING_H_ */
