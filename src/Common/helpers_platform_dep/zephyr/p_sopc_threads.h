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

#include "sopc_threads.h"

/*****Private threads api*****/

typedef void* ptrFct(void* pCtx); // Thread callback

/** @brief
 *   Create an handle of a thread and initialize it.
 *  @param callback The new thread entry point
 *  @param pCtx Any user-defined context parameter. This will be passed to "callback" call.
 *  @param taskName A task name. Caller should ensure uniqueness of names. Possibly can be NULL.
 *  @param priority The ZEPHYR priority of the new thread.
 */
SOPC_Thread P_THREAD_Create(ptrFct* callback,     // Callback
                            void* pCtx,           // Context
                            const char* taskName, // Thread name
                            const int priority);  // Priority

/** @brief
 *  Join and destroy a thread
 *  @param ppHandle A pointer to a tThreadHandle* object, previously returned by P_THREAD_Create
 *  @return true in case of success. Otherwise, the thread is not guaranteed to be terminated
 */
bool P_THREAD_Destroy(SOPC_Thread* ppHandle);

#endif /* SOPC_ZEPHYR_P_THREADS_H_ */
