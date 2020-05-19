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
#include <string.h>

#include <inttypes.h>

#include "kernel.h"

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif
#ifndef K_FOREVER
#define K_FOREVER (-1)
#endif
#ifndef K_NO_WAIT
#define K_NO_WAIT 0
#endif

/* s2opc includes */

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

/* platform dep includes */

#include "p_threads.h"

#define P_THREAD_DEBUG (0)

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

/* Stack memory definition */
K_THREAD_STACK_ARRAY_DEFINE(gThreadStacks, MAX_NB_THREADS, MAX_STACK_SIZE);

// *** Private thread workspace definition ***

typedef struct T_THREAD_WKS
{
    struct k_thread threadControlBlock;
    k_tid_t threadHandle;
    uint32_t slotId;
    struct k_sem kSemThreadEnded;
    volatile uint32_t nbThreadsJoining;
    volatile eThreadStatus status;
    volatile struct tThreadHandle* debugExternalHandle;
    ptrFct userCallback;
    void* userContext;
} tThreadWks;

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

#if (P_THREAD_DEBUG == 1)
    printk("\r\nP_THREAD: Thread callback entry -> thread has been started : %d\r\n", pWks->slotId);
#endif

    if (pWks->userCallback != NULL)
    {
        pWks->userCallback(pWks->userContext);
    }

#if (P_THREAD_DEBUG == 1)
    printk("\r\nP_THREAD: Thread callback exit -> thread will be ended : %d\r\n", pWks->slotId);
#endif

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
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Thread joined, handle well destroyed\r\n");
#endif
        SOPC_Free(*ppWks);
        *ppWks = NULL;
    }
    else
    {
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Thread not joined, handle NOT destroyed\r\n");
#endif
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

    eThreadWksStatus expectedStatus = E_THREAD_WKS_STATUS_NOT_INITIALIZED;
    eThreadWksStatus desiredStatus = E_THREAD_WKS_STATUS_INITIALIZING;
    bool bTransition = __atomic_compare_exchange(&gGlbThreadWks.wGlobalWksStatus, //
                                                 &expectedStatus,                 //
                                                 &desiredStatus,                  //
                                                 false,                           //
                                                 __ATOMIC_SEQ_CST,                //
                                                 __ATOMIC_SEQ_CST);               //

    if (bTransition)
    {
        memset(&gGlbThreadWks, 0, sizeof(struct T_GLOBAL_THREAD_WKS));
        k_mutex_init(&gGlbThreadWks.kLock);
        desiredStatus = E_THREAD_WKS_STATUS_INITIALIZED;
        __atomic_store(&gGlbThreadWks.wGlobalWksStatus, &desiredStatus, __ATOMIC_SEQ_CST);
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Thread middleware not initialized, so initializing it...\r\n");
#endif
    }

    __atomic_load(&gGlbThreadWks.wGlobalWksStatus, &expectedStatus, __ATOMIC_SEQ_CST);

    while (E_THREAD_WKS_STATUS_INITIALIZED != expectedStatus)
    {
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Thread middleware initializing, so wait...\r\n");
#endif
        k_yield();
        __atomic_load(&gGlbThreadWks.wGlobalWksStatus, &expectedStatus, __ATOMIC_SEQ_CST);
    }

    // Lock workspace
    k_mutex_lock(&gGlbThreadWks.kLock, K_FOREVER);

    // Search for empty slot
    for (uint32_t iIter = 0; !bFound && iIter < MAX_NB_THREADS; iIter++)
    {
        if (E_THREAD_STATUS_NOT_INITIALIZED == gGlbThreadWks.tab[iIter].status)
        {
            bFound = true;
            slotId = iIter + 1;
#if (P_THREAD_DEBUG == 1)
            printk("\r\nP_THREAD: Slot found to create new thread %d", slotId);
#endif
        }
    }

    // If slot found, initialize signal for join and try to launch thread
    if (bFound)
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
                            gThreadStacks[slotId - 1],                         //
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

#if (P_THREAD_DEBUG == 1)
    printk("\r\nP_THREAD: Thread slot returned = %d for thread handle %08lX", slotId, (long unsigned int) pWks);
#endif

    pWks->slotId = slotId;

    return result;
}

// Thread join
static eThreadResult P_THREAD_Join(tThreadHandle* pWks)
{
    eThreadResult result = E_THREAD_RESULT_OK;

    // Check parameters validity

    eThreadWksStatus readStatus = E_THREAD_WKS_STATUS_NOT_INITIALIZED;
    __atomic_load(&gGlbThreadWks.wGlobalWksStatus, &readStatus, __ATOMIC_SEQ_CST);

    if (NULL == pWks || 0 == pWks->slotId || pWks->slotId > MAX_NB_THREADS || !readStatus ||
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
#if (P_THREAD_DEBUG == 1)
            printk("\r\nP_THREAD: Error, thread %d not initialized - handle thread = %08lX\r\n", //
                   slotId,                                                                       //
                   (long unsigned int) gGlbThreadWks.tab[slotId - 1].debugExternalHandle);       //
#endif
            result = E_THREAD_RESULT_NOK;
        }

        if (E_THREAD_RESULT_OK == result)
        {
            gGlbThreadWks.tab[slotId - 1].nbThreadsJoining++;
#if (P_THREAD_DEBUG == 1)
            printk("\r\nP_THREAD: Try to join %d, nb joining this thread = %d - handle thread = %08lX\r\n", //
                   slotId,                                                                                  //
                   gGlbThreadWks.tab[slotId - 1].nbThreadsJoining,                                          //
                   (long unsigned int) gGlbThreadWks.tab[slotId - 1].debugExternalHandle                    //
            );
#endif
        }
    }
    k_mutex_unlock(&gGlbThreadWks.kLock);

    if (E_THREAD_RESULT_OK == result)
    {
        // Unlock then wait
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Waiting %d - handle thread = %08lX\r\n",                 //
               slotId,                                                                 //
               (long unsigned int) gGlbThreadWks.tab[slotId - 1].debugExternalHandle); //
#endif
        k_sem_take(&gGlbThreadWks.tab[slotId - 1].kSemThreadEnded, K_FOREVER);

#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Signal ended received for %d - handle thread = %08lX\r\n", //
               slotId,                                                                   //
               (long unsigned int) gGlbThreadWks.tab[slotId - 1].debugExternalHandle);   //
#endif

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
#if (P_THREAD_DEBUG == 1)
                printk("\r\nP_THREAD: Last join operation on thread %d - handle thread = %08lX\r\n", //
                       slotId,                                                                       //
                       (long unsigned int) gGlbThreadWks.tab[slotId - 1].debugExternalHandle);       //
#endif
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
#if (P_THREAD_DEBUG == 1)
    printk("\r\nP_THREAD: Join on thread %d, handle = %08lX\r\n", //
           thread->slotId,                                        //
           (long unsigned int) thread);                           //
#endif
    eThreadResult resultPTHREAD = P_THREAD_Destroy(&thread);

    if (E_THREAD_RESULT_OK == resultPTHREAD)
    {
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Destroy for thread handle = %08lX\r\n", //
               (long unsigned int) thread);                           //
#endif
        // P_THREAD_Destroy(&thread);
        resultSOPC = SOPC_STATUS_OK;
    }
    else
    {
#if (P_THREAD_DEBUG == 1)
        printk("\r\nP_THREAD: Error on join, do not destroy thread handle = %08lX\r\n", //
               (long unsigned int) thread);                                             //
#endif
        resultSOPC = SOPC_STATUS_INVALID_STATE;
    }

    return resultSOPC;
}
