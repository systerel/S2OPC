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

#include <zephyr/kernel.h>

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

/* s2opc includes */

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

/* platform dep includes */

#include "p_sopc_synchro.h"

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
SOPC_ReturnStatus SOPC_Condition_Init(SOPC_Condition* cond)
{
    SOPC_ReturnStatus result = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != cond)
    {
        struct SOPC_Condition_Impl* condI = SOPC_Calloc(1, sizeof(*condI));

        if (SOPC_INVALID_COND == condI)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            result = P_SYNCHRO_kResultToSopcStatus(k_condvar_init(&condI->cond));
            // unrecoverable issue if CONDVAR cannot be created
            SOPC_ASSERT(result == SOPC_STATUS_OK);
        }
        *cond = condI;
    }
    return result;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Condition_Clear(SOPC_Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Free(*cond);
    *cond = SOPC_INVALID_COND;
    return SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Condition_SignalAll(SOPC_Condition* cond)
{
    if (NULL == cond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct SOPC_Condition_Impl* condI = (SOPC_Condition_Impl*) (*cond);
    SOPC_ASSERT(SOPC_INVALID_COND != condI); // see SOPC_Condition_Init

    k_condvar_broadcast(&condI->cond);
    return SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Mutex_Initialization(SOPC_Mutex* mut)
{
    SOPC_ReturnStatus result = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != mut)
    {
        struct SOPC_Mutex_Impl* mutI = SOPC_Calloc(1, sizeof(*mutI));

        if (SOPC_INVALID_MUTEX == mutI)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            result = P_SYNCHRO_kResultToSopcStatus(k_mutex_init(&mutI->mutex));
            // unrecoverable issue if CONDVAR cannot be created
            SOPC_ASSERT(result == SOPC_STATUS_OK);
        }
        *mut = mutI;
    }
    return result;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Mutex_Clear(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Free(*mut);
    *mut = SOPC_INVALID_MUTEX;
    return SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Mutex_Lock(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
    SOPC_ASSERT(SOPC_INVALID_MUTEX != mutI); // See SOPC_Mutex_Initialization
    return P_SYNCHRO_kResultToSopcStatus(k_mutex_lock(&mutI->mutex, K_FOREVER));
}

/***************************************************/
SOPC_ReturnStatus SOPC_Mutex_Unlock(SOPC_Mutex* mut)
{
    if (NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
    SOPC_ASSERT(SOPC_INVALID_MUTEX != mutI); // See SOPC_Mutex_Initialization
    return P_SYNCHRO_kResultToSopcStatus(k_mutex_unlock(&mutI->mutex));
}

/***************************************************/
SOPC_ReturnStatus SOPC_Mutex_UnlockAndTimedWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut, uint32_t milliSecs)
{
    if (NULL == cond || NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
    struct SOPC_Condition_Impl* condI = (SOPC_Condition_Impl*) (*cond);
    SOPC_ASSERT(SOPC_INVALID_COND != condI && SOPC_INVALID_MUTEX != mutI);

    const k_timeout_t kWait = K_MSEC(milliSecs);
    return P_SYNCHRO_kResultToSopcStatus(k_condvar_wait(&condI->cond, &mutI->mutex, kWait));
}

/***************************************************/
SOPC_ReturnStatus SOPC_Mutex_UnlockAndWaitCond(SOPC_Condition* cond, SOPC_Mutex* mut)
{
    if (NULL == cond || NULL == mut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct SOPC_Mutex_Impl* mutI = (SOPC_Mutex_Impl*) (*mut);
    struct SOPC_Condition_Impl* condI = (SOPC_Condition_Impl*) (*cond);
    SOPC_ASSERT(SOPC_INVALID_COND != condI && SOPC_INVALID_MUTEX != mutI);
    return P_SYNCHRO_kResultToSopcStatus(k_condvar_wait(&condI->cond, &mutI->mutex, K_FOREVER));
}
