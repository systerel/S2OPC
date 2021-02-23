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
#include <stdbool.h>
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
 *              (based on first lower case character value comparison). -1000 in case of invalid parameter.
 */
int SOPC_strncmp_ignore_case(const char* s1, const char* s2, size_t size);

/**
 *  \brief Compare 2 string in a case-insensitive manner.
 *  Comparison returns 0 if all characters are identical
 *
 *  \param s1    A non null string terminated by '\0' character
 *  \param s2    A non null string terminated by '\0' character
 *
 *  \return      0 if string are identical in a case-insensitive way, -1 if s1 < s2 and +1 if s1 > s2
 *              (based on first lower case character value comparison).  -1000 in case of invalid parameter.
 */
int SOPC_strcmp_ignore_case(const char* s1, const char* s2);

/**
 *  \brief Compare 2 string in a case-insensitive manner until \p endCharacter or '\0' character found.
 *  Comparison returns 0 if all characters are identical and \p s1 and \p s2 end is reached with
 *  \p endCharacter or '\0' character.
 *
 *  \param s1            A non null string terminated by '\0' character (and which might contain \p endCharacter)
 *  \param s2            A non null string terminated by '\0' character (and which might contain \p endCharacter)
 *  \param endCharacter  An alternative endCharacter to consider for stopping comparison.
 *
 *  \return      0 if string are identical in a case-insensitive way, -1 if s1 < s2 and +1 if s1 > s2
 *              (based on first lower case character value comparison). -1000 in case of invalid parameter.
 */
int SOPC_strcmp_ignore_case_alt_end(const char* s1, const char* s2, char endCharacter);

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

/**
 * \brief        Read a signed integer from the string with strtoll.
 *
 * \param data   A pointer to the CString containing the number in base 10
 * \param len    The length of the CString to use for parsing, it shall be <= 20
 * \param width  The number of bits of the signed integer
 * \param dest   The destination pointer containing the integer value parsed, it shall be of integer type int<width>_t
 *
 * \return       true in case of success false otherwise
 */
bool SOPC_strtoint(const char* data, size_t len, uint8_t width, void* dest);

/**
 * \brief        Read an unsigned integer from the string with strtoull.
 *
 * \param data   A pointer to the CString containing the number in base 10
 * \param len    The length of the CString to use for parsing, it shall be <= 20
 * \param width  The number of bits of the unsigned integer
 * \param dest   The destination pointer containing the integer value parsed, it shall be of integer type uint<width>_t
 *
 * \return       true in case of success false otherwise
 */
bool SOPC_strtouint(const char* data, size_t len, uint8_t width, void* dest);

/**
 * \brief        Read a double from the string with strtod.
 *
 * \param data   A pointer to the CString containing the double value
 * \param len    The length of the CString to use for parsing, it shall be <= 339
 * \param width  The number of bits of the double
 * \param dest   The destination pointer containing the integer value parsed,
 *               it shall be of type float if width == 32 and double if width == 64
 *
 * \return       true in case of success false otherwise
 */
bool SOPC_strtodouble(const char* data, size_t len, uint8_t width, void* dest);

/**
 * \brief    Duplicate the given C string and return copy
 *
 * \param s  The C string to duplicate
 *
 * \return   The duplicated C string or NULL in case of copy failure
 */
char* SOPC_strdup(const char* s);

/**
 * \brief    Parse a string containing an XSD format datetime '[YYYY]-[MM]-[DD]T[hh]:[mm]:[ss]',
 *           it might be followed by 'Z' to indicate UTC timezone or '+/-[hh]:[mm]' for an offset of UTC.
 *           See https://www.w3.org/TR/2012/REC-xmlschema11-2-20120405/datatypes.html#dateTime,
 *           the equivalent regular expression is:
 *           -?([1-9][0-9]{3,}|0[0-9]{3})
 *           -(0[1-9]|1[0-2])
 *           -(0[1-9]|[12][0-9]|3[01])
 *           T(([01][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9](\.[0-9]+)?|(24:00:00(\.0+)?))
 *           (Z|(\+|-)((0[0-9]|1[0-3]):[0-5][0-9]|14:00))?
 *
 * \note     Output NULL pointers are not set and do not lead to return function failure
 *
 * \param datetime         The string containing a XSD datetime value to parse
 * \param len              The length of the the string (strlen(datetime) if datetime is a C string)
 *
 * \param pYear            Pointer set to year provided in datetime string as a 4 digits number.
 *                         It might be preceded by a '-' sign.
 * \param pMonth           Pointer set to month provided in datetime string as a 2 digit number.
 * \param pDay             Pointer set to day provided in datetime string as a 2 digit number.
 * \param pHour            Pointer set to hour provided in datetime string as a 2 digit number.
 * \param pMinute          Pointer set to minutes provided in datetime string as 2 digit number.
 * \param pSecond          Pointer set to the whole number of seconds provided in datetime string as 2 digit number.
 * \param pSecondAndFrac   Pointer set to seconds provided in datetime string as 2 digit number
 *                         with optionally seconds fraction after a '.'.
 * \param p_UTC             Pointer set to true if the provided time is UTC (or not specified),
 *                         false if UTC with an offset. Flag is still set for +/-00:00 offsets.
 * \param pUTC_neg_off      Pointer set to true if the provided time offset is negative
 * \param pUTC_hour_off    Pointer set to hour offset from UTC if \p utc is false, 0 otherwise
 * \param pUTC_min_off     Pointer set to minute offset from UTC if \p utc is false, 0 otherwise
 *
 * \return                 Returns true if the datetime parsing succeeded and \p len characters parsed, false otherwise
 */
bool SOPC_stringToDateTime(const char* datetime,
                           size_t len,
                           int16_t* pYear,
                           uint8_t* pMonth,
                           uint8_t* pDay,
                           uint8_t* pHour,
                           uint8_t* pMinute,
                           uint8_t* pSecond,
                           float* pSecondAndFrac,
                           bool* p_UTC,
                           bool* pUTC_neg_off,
                           uint8_t* pUTC_hour_off,
                           uint8_t* pUTC_min_off);

#endif /* SOPC_HELPER_STRING_H_ */
