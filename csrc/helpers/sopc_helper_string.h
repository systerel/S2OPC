/*
 *  Copyright (C) 2018 Systerel and others.
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

#ifndef SOPC_HELPER_STRING_H_
#define SOPC_HELPER_STRING_H_

#include <inttypes.h>
#include <stdlib.h>

#include "sopc_toolkit_constants.h"

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
 *
 * \return     SOPC_STATUS_OK if read was done successfully, in which case *n is modified.
 */
SOPC_ReturnStatus SOPC_strtouint8_t(const char* sz, uint8_t* n, int base);

/**
 * \brief      Read a uint16_t from the string with strtoul.
 *
 * \param sz   A pointer to the CString containing the number.
 * \param n    A pointer to the uint16_t.
 * \param base The base in which the number is written. See strtoul for more about /p base.
 *
 * \return     SOPC_STATUS_OK if read was done successfully, in which case *n is modified.
 */
SOPC_ReturnStatus SOPC_strtouint16_t(const char* sz, uint16_t* n, int base);

/**
 * \brief      Read a uint32_t from the string with strtoul.
 *
 * \param sz   A pointer to the CString containing the number.
 * \param n    A pointer to the uint32_t.
 * \param base The base in which the number is written. See strtoul for more about /p base.
 *
 * \return     SOPC_STATUS_OK if read was done successfully, in which case *n is modified.
 */
SOPC_ReturnStatus SOPC_strtouint32_t(const char* sz, uint32_t* n, int base);

#endif /* SOPC_HELPER_STRING_H_ */
