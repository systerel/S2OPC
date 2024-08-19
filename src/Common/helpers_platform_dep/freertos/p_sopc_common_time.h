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

#ifndef P_SOPC_COMMON_TIME_H
#define P_SOPC_COMMON_TIME_H

#include <stdint.h>
#include <time.h>

/* Number of ticks since FreeRTOS' EPOCH, which is 01/01/1970 00:00:00 UTC.
 * There are configTICK_RATE_HZ per second.
 */
uint64_t P_SOPC_COMMON_TIME_get_tick(void);

/* Appy an offset in second to internal reference
 */
void P_SOPC_COMMON_TIME_SetDateOffset(int64_t nbSecOffset);

#endif
