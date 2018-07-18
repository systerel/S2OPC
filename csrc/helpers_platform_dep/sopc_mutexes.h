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

// Import Mutex type from platform dependent code
#include "p_threads.h"

SOPC_ReturnStatus Condition_Init(Condition* cond);
SOPC_ReturnStatus Condition_Clear(Condition* cond);

// Must be called between lock and unlock of Mutex ued to wait on condition
SOPC_ReturnStatus Condition_SignalAll(Condition* cond);

SOPC_ReturnStatus Mutex_Initialization(Mutex* mut);
SOPC_ReturnStatus Mutex_Clear(Mutex* mut);
SOPC_ReturnStatus Mutex_Lock(Mutex* mut);
SOPC_ReturnStatus Mutex_Unlock(Mutex* mut);

// Lock on return
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut);
// Lock on return. Return SOPC_STATUS_TIMEOUT in case of timeout before condition signaled
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs);

#endif /* SOPC_MUTEXES_H_ */
