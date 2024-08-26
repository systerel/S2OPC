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
#include <p4ext_threads.h>

#include <stdbool.h>
#include <string.h>

#include "p_sopc_threads.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

static void thread_entry_point(void* arg)
{
    SOPC_Thread_Impl* pThrWks = (SOPC_Thread_Impl*) arg;
    if (pThrWks->pStartFct != NULL)
    {
        pThrWks->pStartFct(pThrWks->pStartArgs);
    }

    P4_e_t res = p4_barrier_wait(&pThrWks->joinSignal);
    SOPC_ASSERT((P4_E_OK == res || P4_E_LIMIT == res));

    p4ext_thr_exit();
    SOPC_ASSERT(0);
}

static inline SOPC_ReturnStatus create_thread(SOPC_Thread_Impl* pThreadWks, int priority, const char* taskName)
{
    p4ext_thr_attr_t attr;
    p4ext_thr_attr_init(&attr);
    attr.prio = priority;
    P4_e_t res = p4ext_thr_create(&pThreadWks->thread, &attr, taskName, thread_entry_point, 1, pThreadWks);

    return res == P4_E_OK ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

static SOPC_Thread_Impl* init_thread(int priority, pFct* pStartFct, void* pStartArgs, const char* pTaskName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Thread_Impl* pThreadContext = NULL;
    pThreadContext = SOPC_Malloc(sizeof(*pThreadContext));
    if (NULL == pThreadContext)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        memset(pThreadContext, 0, sizeof(*pThreadContext));

        pThreadContext->pStartArgs = pStartArgs;
        pThreadContext->pStartFct = pStartFct;
        P4_e_t res = p4ext_thr_num_alloc(&pThreadContext->thread);
        SOPC_ASSERT(P4_E_OK == res);

        res = p4_barrier_init(&pThreadContext->joinSignal, 2, 0);
        SOPC_ASSERT(P4_E_OK == res);

        status = create_thread(pThreadContext, priority, pTaskName);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(pThreadContext);
            pThreadContext = NULL;
        }
    }
    return pThreadContext;
}

SOPC_ReturnStatus SOPC_Thread_Create(SOPC_Thread* thread,
                                     void* (*startFct)(void*),
                                     void* startArgs,
                                     const char* taskName)
{
    /* Create a thread with priority of actual thread divided by 2 */
    P4_prio_t priority = 0;
    P4_e_t res = p4_thread_get_priority(P4_THREAD_MYSELF, &priority);
    if (P4_E_OK != res)
    {
        return SOPC_STATUS_NOK;
    }
    return SOPC_Thread_CreatePrioritized(thread, startFct, startArgs, priority / 2, taskName);
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(SOPC_Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName)
{
    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *thread = init_thread(priority, (void*) startFct, startArgs, taskName);
    if (NULL == *thread)
    {
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
}

static bool join_thread(SOPC_Thread_Impl* pThrWks)
{
    SOPC_ASSERT(NULL != pThrWks);
    bool result = true;
    if (0 == pThrWks->thread)
    {
        result = false;
    }
    if (result)
    {
        P4_e_t res = p4_barrier_wait(&pThrWks->joinSignal);
        if (P4_E_OK != res && P4_E_LIMIT != res)
        {
            result = false;
        }
    }
    return result;
}

static SOPC_ReturnStatus destroy_thread(SOPC_Thread_Impl** ppThrWks)
{
    if (NULL == ppThrWks || NULL == (*ppThrWks))
    {
        // note : SOPC_Thread_Join must return SOPC_STATUS_NOK
        return SOPC_STATUS_NOK;
    }
    const bool result = join_thread(*ppThrWks);

    if (result)
    {
        SOPC_Thread_Impl* pWks = (*ppThrWks);
        pWks->thread = 0;
        pWks->pStartFct = NULL;
        pWks->pStartArgs = NULL;
        SOPC_Free(*ppThrWks);
        return SOPC_STATUS_OK;
        *ppThrWks = SOPC_INVALID_THREAD;
    }
    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Thread_Join(SOPC_Thread* thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == thread)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    status = destroy_thread(thread);

    return status;
}

void SOPC_Sleep(unsigned int milliseconds)
{
    P4_e_t res = p4_sleep((P4_timeout_t) P4_MSEC(milliseconds));
    /* P4_E_CANCEL is sometimes return in debug context. To avoid constant assertions in multithreading context
     * check out this return parameter. But should be add back in production */
    // SOPC_ASSERT(P4_E_OK == res);
    SOPC_ASSERT(P4_E_BADTIMEOUT != res);
}
