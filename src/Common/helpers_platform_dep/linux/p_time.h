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
#include <time.h>

/** Type used to measure milliseconds and sub-millisecond times */
typedef struct timespec SOPC_RealTime;

/** Create a new timestamp
 *
 * \return the current time if copy is NULL
 */
SOPC_RealTime* SOPC_RealTime_Create(const SOPC_RealTime* copy);
void SOPC_RealTime_Delete(SOPC_RealTime** t);

/** Store the current time in t.
 *
 * \return false if failed.
 */
bool SOPC_RealTime_GetTime(SOPC_RealTime* t);

/** Performs t += duration (milliseconds) with highest available resolution on the system. */
void SOPC_RealTime_AddDuration(SOPC_RealTime* t, double duration_ms);

/** Returns now >= t */
bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now);

/** \brief Precise sleep until specified date.
 *
 * \note Does not check that date is in the future.
 * \note The calling thread must have appropriate scheduling policy and priority for precise timing.
 */
bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date);

#endif /* SOPC_P_TIME_H_ */
