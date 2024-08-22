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
 * \file The platform-specific implementation for all mutex-related services. Each platform implementation
 *  shall provide the actual definition of:
 * - \ref SOPC_Condition for Condition Variables  (e.g. pthread_mutex_t for LINUX)
 * - \ref SOPC_Mutex for Mutexes. (e.g. pthread_cond_t for LINUX)
 * - All functions defined in this header file.
 */

#ifndef SOPC_MUTEXES_H_
#define SOPC_MUTEXES_H_

#include <stdint.h>

#include "sopc_enums.h"

/*****************************************************************************
 *   Abstract interface types
 *****************************************************************************/
/**
 * \brief Provides a condition-variable mechanism that supports single and broadcast notifications.
 * \note Each platform must provide the implementation of \ref SOPC_Condition_Impl and all related functions.
 * in \ref sopc_mutexes.h */
typedef struct SOPC_Condition_Impl SOPC_Condition_Impl;
typedef SOPC_Condition_Impl* SOPC_Condition;
#define SOPC_INVALID_COND NULL

/**
 * \brief Provides a mutex mechanism that supports recursive locking.
 * \note Each platform must provide the implementation of \ref SOPC_Mutex and all related functions
 * in \ref sopc_mutexes.h */
typedef struct SOPC_Mutex_Impl SOPC_Mutex_Impl;
typedef SOPC_Mutex_Impl* SOPC_Mutex;
#define SOPC_INVALID_MUTEX NULL

/*****************************************************************************
 *   Platform-dependant functions interfaces
 *****************************************************************************/
/**
 * \brief Create a Condition variable.
 * @param cond A non-NULL pointer to a condition to be created
 * @return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Condition_Init(SOPC_Condition* cond);

/**
 * \brief Delete a Condition variable.
 * \param cond A non-NULL pointer to a condition to be deleted
 * \note The related MUTEX shall be unlocked and shall be deleted after the condition variable
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Condition_Clear(SOPC_Condition* cond);

/**
 * \brief Signals the condition variable to all waiting threads.
 * \param cond A non-NULL pointer to a condition to be signaled
 * \return SOPC_STATUS_OK in case of success
 * \note Must be called between lock and unlock of dedicated Mutex
 */
SOPC_ReturnStatus SOPC_Condition_SignalAll(SOPC_Condition* cond);

/**
 * \brief Create a Mutex
 * @param mut A non-NULL pointer to a Mutex to be created
 * @return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Mutex_Initialization(SOPC_Mutex* mut);

/**
 * \brief Delete a Mutex.
 * \param mut A non-NULL pointer to a Mutex to be deleted
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Mutex_Clear(SOPC_Mutex* mut);

/**
 * \brief Lock a Mutex. The function may be blocking as long as the mutex is
 * 			locked by another thread.
 * \note Mutex shall be recursive. (The same thread can lock several times the same mutex without
 *			being blocked)
 * \param mut A non-NULL pointer to a Mutex to be locked
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Mutex_Lock(SOPC_Mutex* mut);

/**
 * \brief Release a Mutex.
 * \param mut A non-NULL pointer to a Mutex to be unlocked
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Mutex_Unlock(SOPC_Mutex* mut);

/**
 * \brief Wait for a condition variable notification.
 * \param cond A non-NULL pointer to a Condition variable to wait for
 * \param mut A non-NULL pointer to the related Mutex. This Mutex shall be locked by caller before call
 * 			and unlocked after return.
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus SOPC_Mutex_UnlockAndWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut);

/**
 * \brief Timed wait for a condition variable notification.
 * \param cond A non-NULL pointer to a Condition variable to wait for
 * \param mut A non-NULL pointer to the related Mutex. This Mutex shall be locked by caller before call
 * 			and unlocked after return.
 * \param milliSecs The maximum amount of wait time.
 * \return SOPC_STATUS_OK in case of success. SOPC_STATUS_TIMEOUT if the condition is not received within the
 * 		requested timeout
 */
SOPC_ReturnStatus SOPC_Mutex_UnlockAndTimedWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut, uint32_t milliSecs);

#endif /* SOPC_MUTEXES_H_ */
