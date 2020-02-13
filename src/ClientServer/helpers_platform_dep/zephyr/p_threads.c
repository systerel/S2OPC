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

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "kernel.h"

/* s2opc includes */

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

/* platform dep includes */

#include "p_threads.h"

// *** Private enumeration definition ***

typedef enum E_THREAD_STATUS
{
    E_THREAD_STATUS_NOT_INITIALIZED,
    E_THREAD_STATUS_INITIALIZED,
} eThreadStatus;

typedef enum E_THREAD_WKS_STATUS
{
    E_THREAD_WKS_STATUS_NOT_INITIALIZED,
    E_THREAD_WKS_STATUS_INITIALIZING,
    E_THREAD_WKS_STATUS_INITIALIZED,
    E_THREAD_WKS_STATUS_SIZE = INT32_MAX
} eThreadWksStatus;

// *** Public thread handle definition ***

struct tThreadHandle
{
    volatile uint32_t slotId;
};

// *** Private thread workspace definition ***

typedef struct T_THREAD_WKS
{
    struct _k_thread_stack_element sym[MAX_STACK_SIZE] __attribute__((aligned(STACK_ALIGN)));
    struct k_thread threadControlBlock;
    k_tid_t threadHandle;
    uint32_t slotId;
    struct k_sem kSemThreadEnded;
    volatile uint32_t nbThreadsJoining;
    volatile eThreadStatus status;
    volatile struct tThreadHandle* debugExternalHandle;
    ptrFct userCallback;
    void* userContext;
} tThreadWks __attribute__((aligned(STACK_ALIGN)));

// *** Private threads workspaces tab ***

static struct T_GLOBAL_THREAD_WKS
{
    volatile eThreadWksStatus wGlobalWksStatus;
    struct k_mutex kLock;
    tThreadWks tab[MAX_NB_THREADS];
} gGlbThreadWks;

// Thread initialization. Called from P_THREAD_Create
static eThreadResult P_THREAD_Init(tThreadHandle* pWks, ptrFct callback, void* pCtx, const char* taskName);
static eThreadResult P_THREAD_Join(tThreadHandle* pWks);

// Thread internal callback
static void P_THREAD_InternalCallback(void* pContext, void* pNotUsed1, void* pNotUsed2)
{
    (void) pNotUsed1;
    (void) pNotUsed2;

    tThreadWks* pWks = (tThreadWks*) pContext;

    if (pWks->userCallback != NULL)
    {
        pWks->userCallback(pWks->userContext);
    }

    k_sem_give(&pWks->kSemThreadEnded);

    return;
}

// *** Private threads api ***

// Thread creation
tThreadHandle* P_THREAD_Create(ptrFct callback, void* pCtx, const char* taskName)
{
    eThreadResult result = E_THREAD_RESULT_OK;
    tThreadHandle* pWks = NULL;

    if (NULL == callback)
    {
        return NULL;
    }

    pWks = SOPC_Calloc(1, sizeof(struct tThreadHandle));

    if (NULL == pWks)
    {
        result = E_THREAD_RESULT_NOK;
    }

    if (E_THREAD_RESULT_OK == result)
    {
        result = P_THREAD_Init(pWks, callback, pCtx, taskName);
    }

    if (E_THREAD_RESULT_OK != result)
    {
        SOPC_Free(pWks);
        pWks = NULL;
    }

    return pWks;
}

// Thread destruction. Shall be called after join successful if multi join is used.
eThreadResult P_THREAD_Destroy(tThreadHandle** ppWks)
{
    eThreadResult result = E_THREAD_RESULT_OK;

    if (NULL == ppWks || NULL == (*ppWks))
    {
        return E_THREAD_RESULT_INVALID_PARAMETERS;
    }

    result = P_THREAD_Join(*ppWks);

    if (E_THREAD_RESULT_OK == result)
    {
        SOPC_Free(*ppWks);
        *ppWks = NULL;
    }

    return result;
}

// Thread initialization
static eThreadResult P_THREAD_Init(tThreadHandle* pWks, ptrFct callback, void* pCtx, const char* taskName)
{
    eThreadResult result = E_THREAD_RESULT_OK;
    uint32_t slotId = 0;
    bool bFound = false;

    if (NULL == pWks)
    {
        return E_THREAD_RESULT_INVALID_PARAMETERS;
    }

    pWks->slotId = 0;

    if (NULL == callback)
    {
        return E_THREAD_RESULT_INVALID_PARAMETERS;
    }
    // Check first thread creation, if yes create workspace critical section

    eThreadWksStatus fromStatus = __sync_val_compare_and_swap(&gGlbThreadWks.wGlobalWksStatus,     //
                                                              E_THREAD_WKS_STATUS_NOT_INITIALIZED, //
                                                              E_THREAD_WKS_STATUS_INITIALIZING);   //

    if (E_THREAD_WKS_STATUS_NOT_INITIALIZED == fromStatus)
    {
        memset(&gGlbThreadWks, 0, sizeof(struct T_GLOBAL_THREAD_WKS));
        k_mutex_init(&gGlbThreadWks.kLock);
        gGlbThreadWks.wGlobalWksStatus = E_THREAD_WKS_STATUS_INITIALIZED;
    }

    while (E_THREAD_WKS_STATUS_INITIALIZED != gGlbThreadWks.wGlobalWksStatus)
    {
        k_yield();
    }

    // Lock workspace
    k_mutex_lock(&gGlbThreadWks.kLock, K_FOREVER);

    // Search for empty slot
    for (uint32_t iIter = 0; false == bFound && iIter < MAX_NB_THREADS; iIter++)
    {
        if (E_THREAD_STATUS_NOT_INITIALIZED == gGlbThreadWks.tab[iIter].status)
        {
            bFound = true;
            slotId = iIter + 1;
        }
    }

    // If slot found, initialize signal for join and try to launch thread
    if (true == bFound)
    {
        // Initialize binary semaphore, used as single signal...
        k_sem_init(&gGlbThreadWks.tab[slotId - 1].kSemThreadEnded, 0, 1);

        // Launch thread
        gGlbThreadWks.tab[slotId - 1].debugExternalHandle = pWks;
        gGlbThreadWks.tab[slotId - 1].userCallback = callback;
        gGlbThreadWks.tab[slotId - 1].userContext = pCtx;
        gGlbThreadWks.tab[slotId - 1].slotId = slotId;
        gGlbThreadWks.tab[slotId - 1].threadHandle =
            k_thread_create(&gGlbThreadWks.tab[slotId - 1].threadControlBlock, //
                            gGlbThreadWks.tab[slotId - 1].sym,                 //
                            sizeof(gGlbThreadWks.tab[slotId - 1].sym),         //
                            P_THREAD_InternalCallback,                         //
                            &gGlbThreadWks.tab[slotId - 1],                    //
                            NULL,                                              //
                            NULL,                                              //
                            SOPC_THREAD_PRIORITY,                              //
                            0,                                                 //
                            K_NO_WAIT);                                        //

        // If thread not launched, raz slot
        if (NULL == gGlbThreadWks.tab[slotId - 1].threadHandle)
        {
            k_sem_reset(&gGlbThreadWks.tab[slotId - 1].kSemThreadEnded);
            memset(&gGlbThreadWks.tab[slotId - 1], 0, sizeof(tThreadWks));
            slotId = 0;
            result = E_THREAD_RESULT_NOK;
        }
        else
        {
            // If launched, mark slot as intialized and try to set taskname
            if (NULL != taskName)
            {
                k_thread_name_set(gGlbThreadWks.tab[slotId - 1].threadHandle, taskName);
            }
            gGlbThreadWks.tab[slotId - 1].status = E_THREAD_STATUS_INITIALIZED;
            result = E_THREAD_RESULT_OK;
        }
    }
    else
    {
        result = E_THREAD_RESULT_NOK;
    }

    k_mutex_unlock(&gGlbThreadWks.kLock);

    pWks->slotId = slotId;

    return result;
}

// Thread join
static eThreadResult P_THREAD_Join(tThreadHandle* pWks)
{
    eThreadResult result = E_THREAD_RESULT_OK;

    // Check parameters validity
    if (NULL == pWks || 0 == pWks->slotId || pWks->slotId > MAX_NB_THREADS || false == gGlbThreadWks.wGlobalWksStatus ||
        pWks != gGlbThreadWks.tab[pWks->slotId - 1].debugExternalHandle)
    {
        return E_THREAD_RESULT_INVALID_PARAMETERS;
    }

    uint32_t slotId = pWks->slotId;

    // Lock global list
    k_mutex_lock(&gGlbThreadWks.kLock, K_FOREVER);
    {
        // status not initialized, unlock then return;
        if (E_THREAD_STATUS_NOT_INITIALIZED == gGlbThreadWks.tab[slotId - 1].status)
        {
            result = E_THREAD_RESULT_NOK;
        }

        if (E_THREAD_RESULT_OK == result)
        {
            gGlbThreadWks.tab[slotId - 1].nbThreadsJoining++;
        }
    }
    k_mutex_unlock(&gGlbThreadWks.kLock);

    if (E_THREAD_RESULT_OK == result)
    {
        // Unlock then wait
        k_sem_take(&gGlbThreadWks.tab[slotId - 1].kSemThreadEnded, K_FOREVER);
        k_mutex_lock(&gGlbThreadWks.kLock, K_FOREVER);
        {
            // Remove joining thread using this hanlde
            gGlbThreadWks.tab[slotId - 1].nbThreadsJoining--;
            if (gGlbThreadWks.tab[slotId - 1].nbThreadsJoining > 0)
            {
                k_sem_give(&gGlbThreadWks.tab[slotId - 1].kSemThreadEnded);
            }

            // If end, OK, else JOINING on going...
            if (0 == gGlbThreadWks.tab[slotId - 1].nbThreadsJoining)
            {
                k_thread_abort(gGlbThreadWks.tab[slotId - 1].threadHandle);
                gGlbThreadWks.tab[slotId - 1].status = E_THREAD_STATUS_NOT_INITIALIZED;
                memset(&gGlbThreadWks.tab[slotId - 1], 0, sizeof(tThreadWks));
                pWks->slotId = 0;
                result = E_THREAD_RESULT_OK;
            }
            else
            {
                result = E_THREAD_RESULT_JOINING;
            }
        }
        k_mutex_unlock(&gGlbThreadWks.kLock);
    }

    return result;
}

// *** Public SOPC threads api ***

// Create a thread
SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* taskName)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *thread = P_THREAD_Create(startFct, startArgs, taskName);

    if (NULL == *thread)
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// Join then destroy a thread
SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus resultSOPC = SOPC_STATUS_OK;

    if (NULL == thread)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eThreadResult resultPTHREAD = P_THREAD_Destroy(&thread);

    if (E_THREAD_RESULT_OK == resultPTHREAD)
    {
        resultSOPC = SOPC_STATUS_OK;
    }
    else
    {
        resultSOPC = SOPC_STATUS_INVALID_STATE;
    }

    return resultSOPC;
}
