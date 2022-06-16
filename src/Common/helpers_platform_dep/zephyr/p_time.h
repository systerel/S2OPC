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

/** \file
 *
 * PubSub publications require a time that has finer resolution than C99 time_t.
 * The OPC UA type *Duration* is a double representing ms delays between publications.
 * It's resolution goes finer than the nanosecond resolution.
 *
 * The SOPC_RealTime type should be defined to be as precise as possible to support sub-milliseconds publications.
 *
 * Under Linux, the implementation should use CLOCK_MONOTONIC, as we want to send regular notifications,
 * even if the system time gets back or forth (e.g. because of ntp).
 */

#ifndef SOPC_P_TIME_H_
#define SOPC_P_TIME_H_

#include <stdbool.h>
#include <stdint.h>

/** Definition of SOPC_RealTime for SOPC_TIME.H */
typedef struct
{
    /* Internal unit is 100ns. 64 bits are enough to store more than 1000 years.
       Reference is boot time*/
    uint64_t tick100ns;
} SOPC_RealTime;

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
 * \brief Store the current time in t.
 *
 * \param t A Non-null time reference
 * \return false if failed.
 */
bool SOPC_RealTime_GetTime(SOPC_RealTime* t);

/**
 * \brief Adds an offset to a \a SOPC_RealTime object, ensuring a specific time offset towards a
 *        synchronized clock within a periodic time window .
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
 * \param offset_us The offset in microseconds in the time window
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
 * \return true if the function failed
 * \note If date is in the past, the function yields but does not wait.
 * \note The calling thread must have appropriate scheduling policy and priority for precise timing.
 */
bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date);

#endif /* SOPC_P_TIME_H_ */
