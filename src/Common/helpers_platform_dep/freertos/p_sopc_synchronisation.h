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

#ifndef P_SYNCHRONISATION_H
#define P_SYNCHRONISATION_H

#include <limits.h> /* stdlib includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mutexes.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "p_sopc_utils.h"

/*****Private condition variable api*****/

/* Warning: MAX_WAITERS shall be equal to or smaller than MAX_P_UTILS_LIST */
#define MAX_WAITERS (MAX_P_UTILS_LIST)

#define JOINTURE_SIGNAL (0x80000000)
#define JOINTURE_CLEAR_SIGNAL (0x40000000)
#define APP_DEFAULT_SIGNAL (0x20000000)
#define APP_CLEARING_SIGNAL (0x10000000)

typedef enum T_CONDITION_VARIABLE_STATUS
{
    E_COND_VAR_STATUS_NOT_INITIALIZED, // Condition variable not initialized
    E_COND_VAR_STATUS_INITIALIZED,     // Condition variable initialized
    E_COND_VAR_STATUS_SIZEOF = INT32_MAX
} eConditionVariableStatus;

struct SOPC_Condition_Impl
{
    eConditionVariableStatus status;     // Status condition variable
    SemaphoreHandle_t handleLockCounter; // Critical section token
    tUtilsList taskList;                 // List of task with signal expected, calling unlock and wait
};

struct SOPC_Mutex_Impl
{
    QueueHandle_t handle;
};

#endif
