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
 * @brief
 *  Implementation of SOPC threads in scope of ZEPHYR OS
 */

#ifndef SOPC_ZEPHYR_P_THREADS_H_
#define SOPC_ZEPHYR_P_THREADS_H_

#include "p_synchro.h"

/*****Private threads api*****/

typedef struct tThreadHandle tThreadHandle; // Thread handle
typedef void* ptrFct(void* pCtx);           // Thread callback

/** @brief
 *   Create an handle of a thread and initialize it.
 *  @param callback The new thread entry point
 *  @param pCtx Any user-defined context parameter. This will be passed to "callback" call.
 *  @param taskName A task name. Caller should ensure uniqueness of names. Possibly can be NULL.
 *  @param priority The ZEPHYR priority of the new thread.
 *  @param isSOPCThread True for SOPC internal thread. User-defined thread shall use "false" value
 */
tThreadHandle* P_THREAD_Create(ptrFct* callback,         // Callback
                               void* pCtx,               // Context
                               const char* taskName,     // Thread name
                               const int priority,       // Priority
                               const bool isSOPCThread); // True for S2OPC thread. False for applicative thread

/** @brief
 *  Join and destroy a thread
 *  @param ppHandle A pointer to a tThreadHandle* object, previously returned by P_THREAD_Create
 *  @return true in case of success. Otherwise, the thread is not guaranteed to be terminated
 */
bool P_THREAD_Destroy(tThreadHandle** ppHandle);

#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
typedef struct SOPC_Thread_Info
{
    const char* name;
    int priority;
    size_t stack_size;
    size_t stack_usage;
} SOPC_Thread_Info;

/**
 * @brief
 *  retrieves all active tasks stack and status informations.
 * @return
 *  An array of SOPC_Thread_Info elements. This array is statically allocated by the
 *  callee which ensures that it is large enough to hold all tasks status.
 *  The caller must stop array when reaching an empty element (stack_size == NULL)
 */
const SOPC_Thread_Info* SOPC_Thread_GetAllThreadsInfo(void);

/**
 * @return Returns the number of allocations (unfreed)
 */
const size_t SOPC_MemAlloc_Nb_Allocs(void);

#endif

typedef tThreadHandle* Thread;
#endif /* SOPC_ZEPHYR_P_THREADS_H_ */
