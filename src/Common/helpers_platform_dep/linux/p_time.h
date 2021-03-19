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

#ifndef SOPC_P_TIME_H_
#define SOPC_P_TIME_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* TODO: generalize the precise time interface */
/* CLOCK_MONOTONIC under linux, as we want to send regular notifications, even if the time gets back or forth */
typedef struct timespec SOPC_RealTime; /* Or SensitiveTime or SpecialTime or PreciseTime */

 /* ret = now if copy is NULL */
SOPC_RealTime *SOPC_RealTime_Create(const SOPC_RealTime *copy);
bool SOPC_RealTime_GetTime(SOPC_RealTime *t);
void SOPC_RealTime_Delete(SOPC_RealTime **t);
/* t += double in ms */
void SOPC_RealTime_AddDuration(SOPC_RealTime *t, double duration_ms);
/* t <= now */
bool SOPC_RealTime_IsExpired(const SOPC_RealTime *t, const SOPC_RealTime *now);
/* Precise sleep until there, does not check that date is in the future
 * The calling thread must have appropriate scheduling policy and priority for precise timing */
bool SOPC_RealTime_SleepUntil(const SOPC_RealTime *date);

#endif /* SOPC_P_TIME_H_ */
