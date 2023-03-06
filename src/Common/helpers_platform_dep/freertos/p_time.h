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
 * FreeRTOS specific implementation of "sopc_platform_time.h"
 */

#ifndef SOPC_P_TIME_H_
#define SOPC_P_TIME_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/** Definition of SOPC_RealTime */
typedef struct
{
    /* Internal unit is ticks which is user configurable by setting configTICK_RATE_HZ. Note that
     tick frequency cannot be highter than 1 kHz so precision cannot be highter than ms.
     ticksMs represent time from launch in ms */
    uint64_t ticksMs;
} SOPC_RealTime;

void P_TIME_SetInitialDateToBuildTime(void);

#endif /* FREERTOS_P_TIME_H_ */
