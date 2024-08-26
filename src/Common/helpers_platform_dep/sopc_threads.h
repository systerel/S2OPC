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
 * \file
 *
 * The platform-specific implementation for all Thread-related services. Each platform implementation
 *  shall provide the actual definition of:
 * - \ref SOPC_Thread for threads (e.g. pthread_t for linux)
 * - All functions defined in this header file.
 */

#ifndef SOPC_THREADS_H_
#define SOPC_THREADS_H_

#include "sopc_enums.h"
#include "sopc_mutexes.h"

/*****************************************************************************
 *   Abstract interface types
 *****************************************************************************/

/**
 * \brief Provides a threading mechanism.
 * \note The stack size of each thread is not configurable via the S2OPC API. Therefore, in the specific
 *       case of embedded target with limited RAM, the implementation must provide an internal mechanism to
 *       fine tune the thread stacks dimensioning.
 * \note Each platform must provide the implementation of \ref SOPC_Thread_Impl and all related functions
 * in \ref sopc_thread.h */
typedef struct SOPC_Thread_Impl SOPC_Thread_Impl;
typedef SOPC_Thread_Impl* SOPC_Thread;
#define SOPC_INVALID_THREAD NULL

/*****************************************************************************
 *   Platform-dependant functions interfaces
 *****************************************************************************/
/**
 *  \brief Function to create a thread
 *
 *  \param thread      Return parameter for the created thread
 *  \param startFct    Function called at thread start. The \a startFct function is called in a new thread context
 *      with \a startArgs as parameter. The return value is not relevant and shall be set to NULL.
 *  \param startArgs   Arguments of the start function
 *  \param taskName    Name of the created thread
 *
 *  \return            SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS
 *      or SOPC_STATUS_NOK otherwise.
 *
 *  \note              The created thread must be joined using ::SOPC_Thread_Join to ensure context deletion.
 *  \warning Depending on platform, the \p taskName length might be limited (e.g.: 16 characters including terminating
 * null byte for POSIX). In this case the \p taskName will be truncated to comply with limitation.
 */
SOPC_ReturnStatus SOPC_Thread_Create(SOPC_Thread* thread,
                                     void* (*startFct)(void*),
                                     void* startArgs,
                                     const char* taskName);

/**
 * \brief Function to create a high priority thread
 *
 * \note Only supported under Linux for now.

 * See SOPC_Thread_Create.
 * This function creates a thread with specific priority, which usually requires administrative privileges.
 * It should only be used to create threads that require to be woken up at regular but small intervals (< 1ms).
 * Note that this interface does not specify the 'order' of priorities regarding the value. (typically on Zephyr,
 * lower values are the highest priorities, whereas on Linux, this is the contrary).
 *
 * \param thread    Return parameter for the created thread
 * \param startFct  Function called at thread start
 * \param startArgs Arguments of the start function
 * \param priority  Priority of the thread (range depends on implementation) :
 *                      Linux: 1 .. 99,
 *                      FreeRTOS: 1 .. configMAX_PRIORITIES
 *                      ZEPHYR: 1  .. CONFIG_NUM_COOP_PRIORITIES + CONFIG_NUM_PREEMPT_PRIORITIES.
 *                        Note that this is a simple offset of (CONFIG_NUM_COOP_PRIORITIES + 1) regarding
 *                        the Zephyr native priorities. This is required to ensure consistency with S2OPC interface.
 *                        In prj.conf, the priorities configured MUST take into account this offset.
 * \param taskName  Name of the created thread
 *
 * \note            The created thread must be joined using ::SOPC_Thread_Join to ensure context deletion.
 * \return          SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS
 *                  or SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(SOPC_Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName);

/**
 *  \brief Function to wait for a thread to terminate
 *
 *  \param thread   Thread to wait for, created either by ::SOPC_Thread_Create or ::SOPC_Thread_CreatePrioritized.
 *                  Each thread can and shall be joined once only when terminating.
 *                  In case of success the pointed thread value becomes ::SOPC_INVALID_THREAD
 *
 * \return          SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS
 *                  or SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_Thread_Join(SOPC_Thread* thread);

/**
 *  \brief Suspend current thread execution for (at least) a millisecond interval
 *
 *  \param milliseconds  The milliseconds interval value for which execution must be suspended
 */
void SOPC_Sleep(unsigned int milliseconds);

#endif /* SOPC_THREADS_H_ */
