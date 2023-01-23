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
 *  \file sopc_platform_time.h
 *
 *  \brief A platform independent API to handle time management
 */

#ifndef SOPC_PLATFORM_TIME_H_
#define SOPC_PLATFORM_TIME_H_

/**
 * \brief the toolkit provide and use monotonic clock for time references (used for timers)
 * Note: it is possible to set the clock as non monotonic defining variable on configuration. Otherwise default value is
 * true.
 *
 * PubSub publications require a time that has finer resolution than C99 time_t.
 * The OPC UA type *Duration* is a double representing ms delays between publications.
 * It's resolution goes finer than the nanosecond resolution.
 *
 * The SOPC_RealTime type should be defined to be as precise as possible to support sub-milliseconds publications.
 */
#ifndef SOPC_MONOTONIC_CLOCK
#define SOPC_MONOTONIC_CLOCK true
#endif

/**
 * The platform-specific implementation of "p_time.h" shall provide the actual definition of
 * - \ref SOPC_RealTime to hold a RealTime information (date with no specific origin). It is
 *      only used to compute time differences. However, in case the system has an external
 *      PtP synchronization source, it is mandatory that this date is aligned to that PtP source
 *      clock modulo 1 second. The implementation must also use a monotonic clock in all platform
 *      implementation even if the system time gets back or forth (e.g. because of ntp).
 * - all functions defined in this header except ::SOPC_RealTime_Create and ::SOPC_RealTime_Delete.
 */
#include "p_time.h"

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"

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
SOPC_DateTime SOPC_Time_GetCurrentTimeUTC(void);

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
 * \brief Store the current time in t.
 *
 * \param t A Non-null time reference
 * \return false if failed.
 */
bool SOPC_RealTime_GetTime(SOPC_RealTime* t);

/**
 * \brief Convert compare two \p SOPC_RealTime elements into microseconds.
 * \param tRef The reference time. Shall be non-NULL
 * \param t A SOPC_RealTime value (current time is used if NULL).
 * \return the number of microseconds between the reference \p tRef and \p t. Positive if \p t
 *  is after \p tRef
 */
int64_t SOPC_RealTime_DeltaUs(const SOPC_RealTime* tRef, const SOPC_RealTime* t);

/**
 * \brief Adds an offset to a \a SOPC_RealTime object, ensuring a specific time offset towards a
 *        synchronized clock within a periodic time window.
 *        The offset is measured regarding the dateTime Clock, thus implying that it
 *        is synchronized to some external PtP reference.
 *
 *        For example, if \a duration_us is 100k (100ms) and \a offset_us is 20k (20 ms), and if we name
 *        "dtss" the "sub-second part of DateTime corresponding to returned \a t":
 *
 *        dtss = k * 100ms + 20ms (with k integer)
 * \param t A Non-null time reference
 * \param duration_us The timeslice period (must be a divisor of 1 second, otherwise result is undefined)
 *        if offset_us is negative, then no window is used and the function simply adds duration_us to \a t
 * \param offset_us The offset in microseconds in the time window. If no PtP source is synchronized, or if this
 *        offset is not used, it must be set to -1.
 **/
void SOPC_RealTime_AddSynchedDuration(SOPC_RealTime* t, uint64_t duration_us, int32_t offset_us);

/**
 * \brief Checks is a date is in the future (relatively to another date)
 * \param t A non-NULL date
 * \param now if non-NULL, this date will be compared to t. If NULL, the current date will be
 *  used instead.
 *
 * \returns true if \a t < \a now. Typically, \code SOPC_RealTime_IsExpired(&t, NULL) \endcode returns true if
 *      \a t is in the past (event reached), and false if \a t is in the future.
 *  */
bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now);

/** \brief Precise sleep until specified date.
 * \param date The date at which the function shall return
 * \return true in case of success
 * \note If date is in the past, the function yields but does not wait.
 * \note The calling thread must have appropriate scheduling policy and priority for precise timing.
 */
bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date);

/**
 * \brief Create a new time reference.
 * \param copy A possibly NULL time reference.
 *
 * \return A new time reference containing the current time if \a copy is NULL,
 *          or a copy of \a copy otherwise
 * \note Every object initialized by \a SOPC_RealTime_Create must be cleared by \a SOPC_RealTime_Delete
 */
SOPC_RealTime* SOPC_RealTime_Create(const SOPC_RealTime* copy);

/**
 * \brief Deletes a time reference.
 * \param t A reference returned by \a SOPC_RealTime_Create
 */
void SOPC_RealTime_Delete(SOPC_RealTime** t);

/**
 * \brief Checks is a date is in the future (relatively to another date)
 * \param t A non-NULL date
 * \param now if non-NULL, this date will be compared to t. If NULL, the current date will be
 *  used instead.
 *
 * \returns true if \a t < \a now. Typically, \code SOPC_RealTime_IsExpired(&t, NULL) \endcode returns true if
 *      \a t is in the past (event reached), and false if \a t is in the future.
 *  */
bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now);

#endif /* SOPC_PLATFORM_TIME_H_ */
