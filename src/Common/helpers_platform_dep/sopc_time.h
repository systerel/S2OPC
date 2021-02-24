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
 *  \file sopc_time.h
 *
 *  \brief Tools for time management
 */

#ifndef SOPC_TIME_H_
#define SOPC_TIME_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sopc_enums.h"

/**
 * \brief the toolkit provide and use monotonic clock for time references (used for timers)
 * Note: it is possible to set the clock as non monotonic defining variable on configuration. Otherwise default value is
 * true.
 */
#ifndef SOPC_MONOTONIC_CLOCK
#define SOPC_MONOTONIC_CLOCK true
#endif

/**
 * Time reference type (milliseconds)
 */
typedef uint64_t SOPC_TimeReference;

/**
 *  \brief Suspend current thread execution for (at least) a millisecond interval
 *
 *  \param milliseconds  The milliseconds interval value for which execution must be suspended
 */
void SOPC_Sleep(unsigned int milliseconds);

/**
 * \brief return the current time in DateTime format which is 100 nanoseconds from 1601/01/01 00:00:00 UTC
 *
 * Note: since the clock is not monotonic, it should not be used to measure elapsed time
 *
 * \return the current time in DateTime format
 *
 */
int64_t SOPC_Time_GetCurrentTimeUTC(void);

/**
 * \brief returns a C string representation of the given time in DateTime format
 * E.g.:
 * - compact == false: "2018/01/30 13:15:52.694\0"
 * - compact == true: "20180130_131552_694\0"
 *
 * \param time     time value in DataTime format, which is 100 nanoseconds from 1601/01/01 00:00:00 UTC
 * \param local    provides local time if set, UTC time otherwise
 * \param compact  provides compact version when flag is set
 *
 */
char* SOPC_Time_GetString(int64_t time, bool local, bool compact);

/**
 * \brief return the current local time as a C String, e.g.:
 * - compact == false: "2018/01/30 13:15:52.694\0"
 * - compact == true: "20180130_131552_694\0"
 *
 * \param compact  provides compact version when flag is set
 *
 * \return the current local time as C string (to be deallocated after use)
 */
char* SOPC_Time_GetStringOfCurrentLocalTime(bool compact);

/**
 * \brief return the current UTC time as a C String, e.g.:
 * - compact == false: "2018/01/30 13:15:52.694\0"
 * - compact == true: "20180130_131552_694\0"
 *
 * \param compact  provides compact version when flag is set
 *
 * \return the current UTC time as C string (to be deallocated after use)
 */
char* SOPC_Time_GetStringOfCurrentTimeUTC(bool compact);

/**
 * \brief return the current time reference
 *
 * Note: clock is monotonic if SOPC_MONOTONIC_CLOCK is true
 *
 * \return the current time reference
 *
 */
SOPC_TimeReference SOPC_TimeReference_GetCurrent(void);

/**
 * \brief return the time reference corresponding to the given time reference incremented by the given duration in
 * milliseconds
 *
 * \param timeRef the time reference to be incremented
 * \param ms      the duration in milliseconds to use for increment
 *
 * \return the new time reference incremented by the given duration or with the maximum value in case of overflow
 *         or NULL incase timerRef == NULL or new memory allocation failed
 *
 */
SOPC_TimeReference SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference timeRef, uint64_t ms);

/**
 * \brief return the comparison of given time references
 *
 * \param left  the left time reference operand (NULL pointer considered less than any other value)
 * \param right the right time reference operand (NULL pointer considered less than any other value)
 *
 * \return -1 if \p left < \p right operand, 0 if \p left = \p right and 1 if \p left > right
 *
 */
int8_t SOPC_TimeReference_Compare(SOPC_TimeReference left, SOPC_TimeReference right);

/**
 * \brief Breaks down a timestamp to its structured representation in local time.
 *
 * \param t   the timestamp.
 * \param tm  the structured representation of the timestamp in local time.
 *
 * \return \ref SOPC_STATUS_OK in case of success, \ref SOPC_STATUS_NOK in case of error.
 */
SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm);

/**
 * \brief Breaks down a timestamp to its structured representation in UTC time.
 *
 * \param t   the timestamp.
 * \param tm  the structured representation of the timestamp in UTC time.
 *
 * \return \ref SOPC_STATUS_OK in case of success, \ref SOPC_STATUS_NOK in case of error.
 */
SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm);

/**
 * \brief Converts a UNIX timestamp to a time expressed in 100ns slices since
 *        1601/01/01 00:00:00 UTC.
 *
 * \param time  the UNIX timestamp
 * \param res   the resulting time
 * \return     \ref SOPC_STATUS_OK on success, an error code on failure
 */
SOPC_ReturnStatus SOPC_Time_FromTimeT(time_t time, int64_t* res);

/**
 * \brief Converts a time expressed in 100ns slices since 1601/01/01 00:00:00 UTC
 *        to a UNIX timestamp.
 *
 * \param dt   the input time
 * \param res  the resulting time_t
 * \return     \ref SOPC_STATUS_OK on success, an error code on failure
 */
SOPC_ReturnStatus SOPC_Time_ToTimeT(int64_t dt, time_t* res);

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

#endif /* SOPC_TIME_H_ */
