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
 *  \file
 *
 *  \brief A platform independent API to use threads
 */

#ifndef SOPC_THREADS_H_
#define SOPC_THREADS_H_

/**
 * The platform-specific implementation of "p_threads.h" shall provide the actual definition of
 * - \ref SOPC_Condition for Condition Variables  (e.g. pthread_mutex_t for LINUX)
 * - \ref SOPC_Mutex for Mutexes. (e.g. pthread_cond_t for LINUX)
 * - \ref SOPC_Thread for threads (e.g. pthread_t for linux)
 */
#include "p_sopc_threads.h"

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
 *  \param thread   Thread to wait for
 *
 */
SOPC_ReturnStatus SOPC_Thread_Join(SOPC_Thread thread);

#endif /* SOPC_THREADS_H_ */
