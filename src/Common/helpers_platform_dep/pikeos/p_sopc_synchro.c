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

#include <p4.h>

#include "p_sopc_synchro.h"
#include "sopc_enums.h"
#include "sopc_mutexes.h"

SOPC_ReturnStatus SOPC_Condition_Init(SOPC_Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    p4_cond_init(cond, 0);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Condition_Clear(SOPC_Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Nothing to do */
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Condition_SignalAll(SOPC_Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return P4_E_OK == p4_cond_broadcast(cond) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Mutex_Initialization(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    p4_mutex_init(mut, P4_MUTEX_RECURSIVE);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Mutex_Clear(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Nothing to do */
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Mutex_Lock(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P4_E_OK == p4_mutex_lock(mut, P4_TIMEOUT_INFINITE) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Mutex_Unlock(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P4_E_OK == p4_mutex_unlock(mut) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Mutex_UnlockAndTimedWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut, uint32_t milliSecs)
{
    if (NULL == cond || NULL == mut || 0 == milliSecs)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    P4_timeout_t timeout = milliSecs * 1000000; /* P4_timeout_t has nanosecond resolution */
    P4_e_t res = p4_cond_wait(cond, mut, timeout);
    if (P4_E_TIMEOUT == res)
    {
        return SOPC_STATUS_TIMEOUT;
    }
    return P4_E_OK == res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Mutex_UnlockAndWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut)
{
    return SOPC_Mutex_UnlockAndTimedWaitCond(cond, mut, (uint32_t) P4_TIMEOUT_INFINITE);
}
