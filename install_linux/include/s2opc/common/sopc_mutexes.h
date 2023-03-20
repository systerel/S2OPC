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
 *  \brief A platform independent API to use mutexes and condition variables.
 *
 *  Mutexes are recursive, and lock can be called inside a thread where the lock is already held
 *  without blocking.
 */

#ifndef SOPC_MUTEXES_H_
#define SOPC_MUTEXES_H_

#include <stdint.h>

#include "sopc_enums.h"

#include "sopc_threads.h"

/**
 * \brief Create a Condition variable.
 * @param cond A non-NULL pointer to a condition to be created
 * @return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Condition_Init(Condition* cond);

/**
 * \brief Delete a Condition variable.
 * \param cond A non-NULL pointer to a condition to be deleted
 * \note The related MUTEX shall be unlocked and shall be deleted after the condition variable
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Condition_Clear(Condition* cond);

/**
 * \brief Signals the condition variable to all waiting threads.
 * \param cond A non-NULL pointer to a condition to be signaled
 * \return SOPC_STATUS_OK in case of success
 * \note Must be called between lock and unlock of dedicated Mutex
 */
SOPC_ReturnStatus Condition_SignalAll(Condition* cond);

/**
 * \brief Create a Mutex
 * @param mut A non-NULL pointer to a Mutex to be created
 * @return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Mutex_Initialization(Mutex* mut);

/**
 * \brief Delete a Mutex.
 * \param mut A non-NULL pointer to a Mutex to be deleted
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Mutex_Clear(Mutex* mut);

/**
 * \brief Lock a Mutex. The function may be blocking as long as the mutex is
 * 			locked by another thread.
 * \note Mutex shall be recursive. (The same thread can lock several times the same mutex without
 *			being blocked)
 * \param mut A non-NULL pointer to a Mutex to be locked
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Mutex_Lock(Mutex* mut);

/**
 * \brief Release a Mutex.
 * \param mut A non-NULL pointer to a Mutex to be unlocked
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Mutex_Unlock(Mutex* mut);

/**
 * \brief Wait for a condition variable notification.
 * \param cond A non-NULL pointer to a Condition variable to wait for
 * \param mut A non-NULL pointer to the related Mutex. This Mutex shall be locked by caller before call
 * 			and unlocked after return.
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut);

/**
 * \brief Timed wait for a condition variable notification.
 * \param cond A non-NULL pointer to a Condition variable to wait for
 * \param mut A non-NULL pointer to the related Mutex. This Mutex shall be locked by caller before call
 * 			and unlocked after return.
 * \param milliSecs The maximum amount of wait time.
 * \return SOPC_STATUS_OK in case of success. SOPC_STATUS_TIMEOUT if the condition is not received within the
 * 		requested timeout
 */
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs);

#endif /* SOPC_MUTEXES_H_ */
