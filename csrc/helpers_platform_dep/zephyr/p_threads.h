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

#ifndef SOPC_P_THREADS_H_
#define SOPC_P_THREADS_H_

#include <stdint.h>

#include "sopc_enums.h"

#include "p_synchro.h"

/*****Private threads api*****/

#define MAX_NB_THREADS (8)
#define MAX_STACK_SIZE (4096)
#define SOPC_THREAD_PRIORITY (5)

typedef enum E_THREAD_RESULT
{
    E_THREAD_RESULT_OK,
    E_THREAD_RESULT_NOK,
    E_THREAD_RESULT_INVALID_PARAMETERS,
    E_THREAD_RESULT_JOINING
} eThreadResult;

typedef struct tThreadHandle tThreadHandle; // Thread handle
typedef void* (*ptrFct)(void* pCtx);        // Thread callback

// Create an handle of a thread and initialize it.
tThreadHandle* P_THREAD_Create(ptrFct callback,       // Callback
                               void* pCtx,            // Context
                               const char* taskName); // Thread name

// Join and destroy a thread
eThreadResult P_THREAD_Destroy(tThreadHandle** ppWks);

/*****Public threads api*****/

typedef tThreadHandle* Thread;

#endif /* SRC_P_THREAD_H_ */
