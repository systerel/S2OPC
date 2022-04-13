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

#ifndef SOPC_PLATFORM_TIME_H_
#define SOPC_PLATFORM_TIME_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sopc_builtintypes.h"
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

#endif /* SOPC_PLATFORM_TIME_H_ */
