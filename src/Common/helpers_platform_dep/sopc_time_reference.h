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
 *  \file sopc_time_reference.h
 *
 *  \brief A platform independent API to handle time reference management
 */

#ifndef SOPC_TIME_REFERENCE_H_
#define SOPC_TIME_REFERENCE_H_

/**
 * \brief the toolkit provide and use monotonic clock for time references (used for timers)
 * Note: it is possible to set the clock as non monotonic defining variable on configuration. Otherwise default value is
 * true.
 *
 * PubSub publications require a time that has finer resolution than C99 time_t.
 * The OPC UA type *Duration* is a double representing ms delays between publications.
 * It's resolution goes finer than the nanosecond resolution.
 *
 * The SOPC_HighRes_TimeReference type should be defined to be as precise as possible to support sub-milliseconds
 * publications.
 */
#ifndef SOPC_MONOTONIC_CLOCK
#define SOPC_MONOTONIC_CLOCK true
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * Provides a reference point in time (in milliseconds). This representation is only
 * relevant when comparing two objects. It is mainly used to measure time intervals.
 */
typedef uint64_t SOPC_TimeReference;

/** SOPC_TimeReference
 * Provides a high resolution reference point in time. This representation is only
 * relevant when comparing two objects. It is mainly used to measure time intervals. The granularity depends on OS
 * specific capabilities.
 * \note Each platform must provide the implementation of SOPC_HighRes_TimeReference and all related functions. */
typedef struct SOPC_HighRes_TimeReference SOPC_HighRes_TimeReference;

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
 * \brief Store the current time in t.
 *
 * \param t A Non-null time reference
 *
 * \return false if failed.
 */
bool SOPC_HighRes_TimeReference_GetTime(SOPC_HighRes_TimeReference* t);

/**
 * \brief Compare two \p SOPC_HighRes_TimeReference elements into microseconds.
 * \param tRef The reference time. Shall be non-NULL
 * \param t A SOPC_HighRes_TimeReference value (current time is used if NULL).
 * \return the number of microseconds between the reference \p tRef and \p t. Positive if \p t
 *  is after \p tRef
 */
int64_t SOPC_HighRes_TimeReference_DeltaUs(const SOPC_HighRes_TimeReference* tRef, const SOPC_HighRes_TimeReference* t);

/**
 * \brief Adds an offset to a \a SOPC_HighRes_TimeReference object, ensuring a specific time offset towards a
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
void SOPC_HighRes_TimeReference_AddSynchedDuration(SOPC_HighRes_TimeReference* t,
                                                   uint64_t duration_us,
                                                   int32_t offset_us);

/**
 * \brief Checks is a date is in the future (relatively to another date)
 * \param t A non-NULL date
 * \param now if non-NULL, this date will be compared to t. If NULL, the current date will be
 *  used instead.
 *
 * \returns true if \a t < \a now. Typically, \code SOPC_HighRes_TimeReference_IsExpired(&t, NULL) \endcode returns true
 * if \a t is in the past (event reached), and false if \a t is in the future.
 *  */
bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now);

/** \brief Precise sleep until specified date.
 * \param date The date at which the function shall return
 * \return true in case of success
 * \note If date is in the past, the function yields but does not wait.
 * \note The calling thread must have appropriate scheduling policy and priority for precise timing.
 */
bool SOPC_HighRes_TimeReference_SleepUntil(const SOPC_HighRes_TimeReference* date);

/**
 * \brief Create a new time reference.
 *
 * \return A new time reference containing the current time
 * \note Every object initialized by \a SOPC_HighRes_TimeReference_Create must be cleared by \a
 * SOPC_HighRes_TimeReference_Delete.
 */
SOPC_HighRes_TimeReference* SOPC_HighRes_TimeReference_Create(void);

/**
 * @brief A copy of a non-NULL SOPC_HighRes_TimeReference structure
 *
 * @param to Pointer to the destination
 * @param from Pointer to the source
 * @return true in case of success
 * @return false if \a from or \a to is NULL
 */
bool SOPC_HighRes_TimeReference_Copy(SOPC_HighRes_TimeReference* to, const SOPC_HighRes_TimeReference* from);
/**
 * \brief Deletes a time reference.
 * \param t A reference returned by \a SOPC_HighRes_TimeReference_Create
 */
void SOPC_HighRes_TimeReference_Delete(SOPC_HighRes_TimeReference** t);

/**
 * \brief Checks is a date is in the future (relatively to another date)
 * \param t A non-NULL date
 * \param now if non-NULL, this date will be compared to t. If NULL, the current date will be
 *  used instead.
 *
 * \returns true if \a t < \a now. Typically, \code SOPC_HighRes_TimeReference_IsExpired(&t, NULL) \endcode returns true
 * if \a t is in the past (event reached), and false if \a t is in the future.
 *  */
bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now);

#endif // SOPC_TIME_REFERENCE_H_
