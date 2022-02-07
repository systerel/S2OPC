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

#include <stdbool.h>
#include <stdlib.h>

#include <inttypes.h>

/* Zephyr includes */

#include "kernel.h"

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

/* s2opc includes */

#include "sopc_enums.h"

/* platform dep includes */

#include "p_synchro.h"

/* debug printk activation */

#define P_SYNCHRO_CONDITION_DEBUG (1)
#define P_SYNCHRO_MUTEX_DEBUG (0)

/***************************************************
 * DECLARATION OF LOCAL FUNCTIONS
 **************************************************/
static inline const SOPC_ReturnStatus P_SYNCHRO_kResultToSopcStatus(const int kResult)
{
    return (kResult == 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}

/***************************************************
 * IMPLEMENTATION OF EXTERNAL FUNCTIONS
 **************************************************/

/***************************************************/
SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P_SYNCHRO_kResultToSopcStatus(k_condvar_init(cond));
}

/***************************************************/
SOPC_ReturnStatus Condition_Clear(Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Nothing to do.
    return SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus Condition_SignalAll(Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    k_condvar_broadcast(cond);
    return SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus Mutex_Initialization(Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P_SYNCHRO_kResultToSopcStatus(k_mutex_init(mut));
}

/***************************************************/
SOPC_ReturnStatus Mutex_Clear(Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Nothing to do.
    return SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus Mutex_Lock(Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return P_SYNCHRO_kResultToSopcStatus(k_mutex_lock(mut, K_FOREVER));
}

/***************************************************/
SOPC_ReturnStatus Mutex_Unlock(Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return P_SYNCHRO_kResultToSopcStatus(k_mutex_unlock(mut));
}

/***************************************************/
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    if (NULL == cond || NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const k_timeout_t kWait = K_MSEC(milliSecs);
    return P_SYNCHRO_kResultToSopcStatus(k_condvar_wait(cond, mut, kWait));
}

/***************************************************/
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    if (NULL == cond || NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return P_SYNCHRO_kResultToSopcStatus(k_condvar_wait(cond, mut, K_FOREVER));
}
