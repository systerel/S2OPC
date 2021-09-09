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

#ifndef SOPC_HELPER_TIME_H_
#define SOPC_HELPER_TIME_H_

#include "sopc_enums.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * \brief S2OPC equivalent of standard struct tm.
 *        Fields use fix-length numerical values.
 */
typedef struct SOPC_tm
{
    int16_t year;         /**< Year value (might be negative) */
    uint8_t month;        /**< Month value in [1, 12] */
    uint8_t day;          /**< Day value in [1, 31] and constrained to valid day/month/year dates */
    uint8_t hour;         /**< Hour value in [0, 24], minutes and seconds are constrained to 0 when 24 */
    uint8_t minute;       /**< Minute value in [0, 59] */
    uint8_t second;       /**< Second value in [0, 59] */
    double secondAndFrac; /**< Second value with fraction of seconds */
    bool UTC; /**< UTC flag set to true if the provided time is UTC (or not specified) and false if UTC with an offset.
                    Flag is still set true for +/-00:00 offsets */
    bool UTC_neg_off;     /**< Negative UTC flag set to true if the provided time offset is negative */
    uint8_t UTC_hour_off; /**< UTC hours offset from UTC if UTC flag is false, 0 otherwise */
    uint8_t UTC_min_off;  /**< UTC minutes offset from UTC if UTC flag is false, 0 otherwise */
} SOPC_tm;

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
 * \param pTm              Pointer to broken-down time structure used as output parameter.
 *                         It is filled in case of success.
 *
 * \return                 Returns true if the datetime parsing succeeded and \p len characters parsed, false otherwise
 */
bool SOPC_tm_FromXsdDateTime(const char* datetime, size_t len, SOPC_tm* tm);

/**
 * \brief Converts a string using XSD DateTime format (see ::SOPC_stringToDateTime) to a time expressed in 100ns slices
 * since 1601/01/01 00:00:00 UTC.
 *
 * \warning This function uses similar limitation as indicated for encoding for OPC UA binary DateTime (see part 6):
 *  - A date/time value is encoded as 0 if the year is earlier than 1601
 *  - A date/time is encoded as the maximum value for an Int64 if the year is greater than 9999
 *  Since those values cannot be encoded as indicated by OPC UA standard, there is no need to manage converting those.
 *
 * \param dateTime  The string containing a XSD datetime value to parse
 * \param len       The length of the the string (strlen(datetime) if datetime is a C string)
 * \param res       The resulting time
 * \return          \ref SOPC_STATUS_OK on success, an error code on failure
 */
SOPC_ReturnStatus SOPC_Time_FromXsdDateTime(const char* dateTime, size_t len, int64_t* res);

#endif /* SOPC_HELPER_TIME_H_ */
