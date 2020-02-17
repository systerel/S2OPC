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

#ifndef SOPC_SYNCHRO_H
#define SOPC_SYNCHRO_H

#include <stdint.h>
#include <inttypes.h>

/*****Private synchro api*****/

#define MAX_COND_VAR_WAITERS (8)
#define MAX_MUT_VAR (32)
#define MAX_COND_VAR (MAX_MUT_VAR * 2)

typedef enum E_SYNCHRO_RESULT
{
    E_SYNCHRO_RESULT_OK,
    E_SYNCHRO_RESULT_NOK,
    E_SYNCHRO_RESULT_TMO,
    E_SYNCHRO_RESULT_NO_WAITERS,
    E_SYNCHRO_RESULT_INVALID_STATE,
    E_SYNCHRO_RESULT_INVALID_PARAMETERS
} eSynchroResult;

typedef uint32_t tCondVarHandle;
typedef uint32_t tMutVarHandle;

typedef void (*pLockCb)(void*);
typedef void (*pUnlockCb)(void*);

// Condition variable API

tCondVarHandle P_SYNCHRO_CONDITION_Initialize(void);
eSynchroResult P_SYNCHRO_CONDITION_Clear(tCondVarHandle syncId);
eSynchroResult P_SYNCHRO_CONDITION_SignalAll(tCondVarHandle syncId);
eSynchroResult P_SYNCHRO_CONDITION_UnlockAndWait(tCondVarHandle syncId,
                                                 void* pMutex,
                                                 pLockCb cbLock,
                                                 pUnlockCb cbUnlock,
                                                 uint32_t timeoutMs);

// Mutex variable API

tMutVarHandle P_SYNCHRO_MUTEX_Initialize(void);
eSynchroResult P_SYNCHRO_MUTEX_Clear(tMutVarHandle idSlot);
eSynchroResult P_SYNCHRO_MUTEX_Lock(tMutVarHandle idSlot);
eSynchroResult P_SYNCHRO_MUTEX_Unlock(tMutVarHandle idSlot);

/*****Public synchro api*****/

typedef tCondVarHandle Condition;
typedef tMutVarHandle Mutex;

#endif /* SOPC_SYNCHRO_H */
