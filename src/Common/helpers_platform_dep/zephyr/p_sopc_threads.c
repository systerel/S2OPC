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
 * @brief
 * So as to optimize stack usage, each S2OPC task has a specific-tuned stack size.
 * S2OPC tasks are regrouped into 3 sections 'Utility' (smaller size), 'Main' and 'Large' (bigger size).
 * See SOPC_UTILITY_TASK_STACK_SIZE, SOPC_MAIN_TASK_STACK_SIZE and SOPC_LARGE_TASK_STACK_SIZE.
 * Other user tasks are configured with a common CONFIG_SOPC_USER_STACK_SIZE value.
 * When a thread is created, filters are checked (gSopcTasks), and if a taskName is matching the new task name,
 * then, the parameter of that filter are used (stack size & position).
 * Otherwise, a free slot is taken amongst gUserTasks (No filter : filterName == NULL).
 *
 * Use the SOPC_DEBUG_THREADS parameter to provide access to instrumentation and log debug.
 * See SOPC_Thread_GetAllThreadsInfo
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

#include <zephyr/kernel.h>

#include "sopc_assert.h"

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

#define KILOBYTE (1024u)

/* s2opc includes */

#include "sopc_enums.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_build_info.h"

/* platform dep includes */

#include "p_sopc_threads.h"
#include "sopc_threads.h"

/* Stack memory definition */
#define SOPC_UTILITY_TASK_STACK_SIZE (KILOBYTE * CONFIG_SOPC_UTILITY_TASK_STACK_SIZE) // Application & Timers
#define SOPC_MAIN_TASK_STACK_SIZE (KILOBYTE * CONFIG_SOPC_MAIN_TASK_STACK_SIZE)   // SubSocketMgr & Sockets & Publisher
#define SOPC_LARGE_TASK_STACK_SIZE (KILOBYTE * CONFIG_SOPC_LARGE_TASK_STACK_SIZE) // Secure_Chann & Services

// *** Private thread workspace definition ***
/**
 * A structure containing all thread-related informations. As all objects are created statically, the
 * callback filed is used to indicate if the thread is active or not (NULL if inactive). This also indicates if
 * the slot is free for a new task.
 */
typedef struct T_THREAD_WKS
{
    struct k_thread threadControlBlock;
    k_tid_t threadHandle;
    struct k_sem kSemThreadEnded;
    ptrFct* userCallback;
    const char* taskName;
    bool joined;
    int priority;
} tThreadWks;

/** This structure is used to force thread creation to use a predefined stack, based on its name
 * When a thread is created, if any sopcThreadConfig nameFilter matches the created thread, then
 * This structure will be used as stack parameter. Otherwise, the created thread will use any free
 * configuration with nameFilter==NULL
 */
typedef struct sopcThreadConfig
{
    const char* nameFilter;
    size_t stackSize;
    void* stackBase;
    tThreadWks* pWks;
} sopcThreadConfig;

#define NB_SOPC_THREADS 7

#define T_THREAD_NB_THREADS (NB_SOPC_THREADS + CONFIG_SOPC_MAX_USER_TASKS)
static tThreadWks gGlbThreadWks[T_THREAD_NB_THREADS];

// Define various tasks

#ifdef CONFIG_SOPC_ALLOC_SECTION
#define SECTION __attribute__((section(CONFIG_SOPC_ALLOC_SECTION)))
#define SOPC_STACK_DEFINE(sym, size) Z_KERNEL_STACK_DEFINE_IN(sym, size, SECTION);
#else
#define SOPC_STACK_DEFINE(sym, size) K_KERNEL_STACK_DEFINE(sym, size);
#endif
SOPC_STACK_DEFINE(gThreadStack_Timers, SOPC_UTILITY_TASK_STACK_SIZE);

SOPC_STACK_DEFINE(gThreadStack_Application, SOPC_MAIN_TASK_STACK_SIZE);
SOPC_STACK_DEFINE(gThreadStack_SubSocketMgr, SOPC_MAIN_TASK_STACK_SIZE);
SOPC_STACK_DEFINE(gThreadStack_Sockets, SOPC_MAIN_TASK_STACK_SIZE);
SOPC_STACK_DEFINE(gThreadStack_Publisher, SOPC_MAIN_TASK_STACK_SIZE);

SOPC_STACK_DEFINE(gThreadStack_Secure_Chann, SOPC_LARGE_TASK_STACK_SIZE);
SOPC_STACK_DEFINE(gThreadStack_Services, SOPC_LARGE_TASK_STACK_SIZE);

// All SOPC tasks. These threads have predefined stack sizes tuned to actual needs.
static sopcThreadConfig gSopcTasks[NB_SOPC_THREADS] = {{.nameFilter = "Application",
                                                        .stackSize = SOPC_MAIN_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_Application,
                                                        .pWks = &gGlbThreadWks[0]},
                                                       {.nameFilter = "Timers",
                                                        .stackSize = SOPC_UTILITY_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_Timers,
                                                        .pWks = &gGlbThreadWks[1]},
                                                       {.nameFilter = "SubSocketMgr",
                                                        .stackSize = SOPC_MAIN_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_SubSocketMgr,
                                                        .pWks = &gGlbThreadWks[2]},
                                                       {.nameFilter = "Sockets",
                                                        .stackSize = SOPC_MAIN_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_Sockets,
                                                        .pWks = &gGlbThreadWks[3]},
                                                       {.nameFilter = "Publisher",
                                                        .stackSize = SOPC_MAIN_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_Publisher,
                                                        .pWks = &gGlbThreadWks[4]},
                                                       {.nameFilter = "Secure_Chann",
                                                        .stackSize = SOPC_LARGE_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_Secure_Chann,
                                                        .pWks = &gGlbThreadWks[5]},
                                                       {.nameFilter = "Services",
                                                        .stackSize = SOPC_LARGE_TASK_STACK_SIZE,
                                                        .stackBase = &gThreadStack_Services,
                                                        .pWks = &gGlbThreadWks[6]}};

// All user stack size
#ifdef CONFIG_SOPC_ALLOC_SECTION
Z_KERNEL_STACK_ARRAY_DEFINE_IN(gThreadStacks,
                               CONFIG_SOPC_MAX_USER_TASKS,
                               CONFIG_SOPC_USER_STACK_SIZE* KILOBYTE,
                               SECTION);
#else
K_KERNEL_STACK_ARRAY_DEFINE(gThreadStacks, CONFIG_SOPC_MAX_USER_TASKS, CONFIG_SOPC_USER_STACK_SIZE* KILOBYTE);
#endif

static sopcThreadConfig gUserTasks[CONFIG_SOPC_MAX_USER_TASKS];

// This is the handle returned to caller
struct tThreadHandle
{
    const sopcThreadConfig* pCfg;
};

static bool gInitialized = false;
static struct k_mutex gLock;

#define STRING_SECURE(s) ((s) ? (s) : "<NULL>")

/**** Configure thread-specific tuning ****/
/**
 * @brief returns the expected stack size of a given task, based on its name.
 * @param taskName The task name.
 * @return The stack configuration to use.
 */
static const sopcThreadConfig* P_THREAD_GetStackCfg(const char* taskName)
{
    const sopcThreadConfig* result = NULL;
    if (NULL != taskName)
    {
        // Search in S2OPC predefined tasks
        for (size_t i = 0; i < NB_SOPC_THREADS && (NULL == result); i++)
        {
            const sopcThreadConfig* pCfg = &gSopcTasks[i];
            if (strncmp(pCfg->nameFilter, taskName, strlen(pCfg->nameFilter)) == 0)
            {
                // Ensure there are not 2 tasks with an S2OPC name...
                SOPC_ASSERT(pCfg->pWks != NULL && pCfg->pWks->userCallback == NULL);
                result = pCfg;
            }
        }
    }
    if (result == NULL)
    {
        // Consider it as an applicative task
        // try to create an applicative task
        for (size_t i = 0; i < CONFIG_SOPC_MAX_USER_TASKS && (NULL == result); i++)
        {
            const sopcThreadConfig* pCfg = &gUserTasks[i];
            if (pCfg->pWks != NULL && pCfg->pWks->userCallback == NULL)
            {
                result = pCfg;
            }
        }
    }
    return result;
}

// *** Private threads workspaces tab ***

// Thread initialization. Called from P_THREAD_Create
static bool P_THREAD_Init(tThreadHandle* pHandle,
                          ptrFct* callback,
                          void* pCtx,
                          const char* taskName,
                          const sopcThreadConfig* pCfg,
                          const int priority);
static bool P_THREAD_Join(tThreadHandle* pHandle);

// Thread internal callback
static void P_THREAD_InternalCallback(void* pContext, void* pCtx, void* pNotUsed)
{
    SOPC_UNUSED_ARG(pNotUsed);

    tThreadWks* pWks = (tThreadWks*) pContext;

    if (pWks->userCallback != NULL)
    {
        pWks->userCallback(pCtx);
    }

    k_sem_give(&pWks->kSemThreadEnded);

    return;
}

// *** Private threads api ***

// Thread initialization
static bool P_THREAD_Init(tThreadHandle* pHandle,
                          ptrFct* callback,
                          void* pCtx,
                          const char* taskName,
                          const sopcThreadConfig* pCfg,
                          const int priority)
{
    if (NULL == pHandle || NULL == callback)
    {
        return false;
    }
    if (priority < 0 || priority >= (CONFIG_NUM_COOP_PRIORITIES + CONFIG_NUM_PREEMPT_PRIORITIES))
    {
        printk("\n!!Invalid priority %d\n", priority);
        return false;
    }
    if (NULL == pCfg)
    {
        printk("\n!!Cannot create task <%s>. (No free slots). Consider revising CONFIG_SOPC_MAX_USER_TASKS\n",
               STRING_SECURE(taskName));
        return false;
    }
    tThreadWks* pWks = pCfg->pWks;

    SOPC_ASSERT(NULL != pWks);

    // Launch thread
    k_sem_init(&pWks->kSemThreadEnded, 0, 1);
    pWks->taskName = taskName;
    pWks->userCallback = callback;
    pWks->joined = false;
    pWks->priority = priority - CONFIG_NUM_COOP_PRIORITIES - 1;
    pWks->threadHandle = k_thread_create(&pWks->threadControlBlock, pCfg->stackBase, pCfg->stackSize,
                                         P_THREAD_InternalCallback, pWks, pCtx, NULL, pWks->priority, 0, K_NO_WAIT);

    // Initialize signal for join and try to launch thread
    if (NULL == pWks->threadHandle)
    {
        k_sem_reset(&pWks->kSemThreadEnded);
        memset(pWks, 0, sizeof(tThreadWks));
        pCfg = NULL;
        printk("\n!!Create task %s failed \n", STRING_SECURE(taskName));
    }
    else
    {
        // If launched, mark slot as intialized and try to set taskname
        if (NULL != taskName)
        {
            k_thread_name_set(pWks->threadHandle, taskName);
        }
    }

    pHandle->pCfg = pCfg;
    return (pCfg != NULL);
}

// Thread creation
tThreadHandle* P_THREAD_Create(ptrFct* callback, void* pCtx, const char* taskName, const int priority)
{
    tThreadHandle* pHandle = NULL;

    if (NULL == callback)
    {
        return NULL;
    }

    if (gInitialized == false)
    {
        // Threads 0 .. NB_SOPC_THREADS -1 are statically configured

        // Setup User tasks
        memset(&gGlbThreadWks, 0, sizeof(gGlbThreadWks));
        k_mutex_init(&gLock);
        for (size_t i = 0; i < CONFIG_SOPC_MAX_USER_TASKS; i++)
        {
            sopcThreadConfig* pCfg = &gUserTasks[i];
            pCfg->stackBase = gThreadStacks[i];
            pCfg->stackSize = CONFIG_SOPC_USER_STACK_SIZE * KILOBYTE;
            pCfg->nameFilter = "";
            pCfg->pWks = &gGlbThreadWks[i + NB_SOPC_THREADS];
            pCfg->pWks->userCallback = NULL;
        }

        // Fill-in user threads configuration
        gInitialized = true;
    }

    k_mutex_lock(&gLock, K_FOREVER);

    pHandle = SOPC_Calloc(1, sizeof(struct tThreadHandle));

    if (NULL != pHandle)
    {
        const sopcThreadConfig* stack_cfg = P_THREAD_GetStackCfg(taskName);
        const bool result = P_THREAD_Init(pHandle, callback, pCtx, taskName, stack_cfg, priority);
        if (!result)
        {
            SOPC_Free(pHandle);
            pHandle = NULL;
        }
    }

    k_mutex_unlock(&gLock);
    return pHandle;
}

// Thread destruction. Shall be called after join successful if multi join is used.
bool P_THREAD_Destroy(tThreadHandle** ppHandle)
{
    if (NULL == ppHandle || NULL == (*ppHandle))
    {
        return false;
    }

    const bool result = P_THREAD_Join(*ppHandle);

    if (result)
    {
        tThreadWks* pWks = (*ppHandle)->pCfg->pWks;
        pWks->userCallback = NULL;
        pWks->taskName = NULL;
        pWks->threadHandle = 0;
        pWks->priority = 255;
        SOPC_Free(*ppHandle);
        *ppHandle = NULL;
    }

    return result;
}

// Thread join
static bool P_THREAD_Join(tThreadHandle* pHandle)
{
    SOPC_ASSERT(gInitialized);
    // Check parameters validity
    SOPC_ASSERT(pHandle != NULL && pHandle->pCfg != NULL);
    tThreadWks* pWks = pHandle->pCfg->pWks;
    SOPC_ASSERT(NULL != pWks);

    bool mustJoin = false;
    k_mutex_lock(&gLock, K_FOREVER);
    if (!pWks->joined)
    {
        pWks->joined = true;
        mustJoin = true;
    }
    k_mutex_unlock(&gLock);
    if (mustJoin)
    {
        // Unlock then wait
        k_sem_take(&pWks->kSemThreadEnded, K_FOREVER);
    }
    return true;
}

// *** Public SOPC threads api ***

// Create a thread
SOPC_ReturnStatus SOPC_Thread_Create(SOPC_Thread* thread,
                                     void* (*startFct)(void*),
                                     void* startArgs,
                                     const char* taskName)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *thread = P_THREAD_Create(startFct, startArgs, taskName, CONFIG_SOPC_THREAD_DEFAULT_PRIORITY);

    if (NULL == *thread)
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(SOPC_Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName)
{
    // No specific limit on priorites on ZEPHYR.
    if (priority <= 0 || priority > CONFIG_NUM_PREEMPT_PRIORITIES + CONFIG_NUM_COOP_PRIORITIES)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *thread = P_THREAD_Create(startFct, startArgs, taskName, priority);

    if (NULL == *thread)
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// Join then destroy a thread
SOPC_ReturnStatus SOPC_Thread_Join(SOPC_Thread thread)
{
    SOPC_ReturnStatus resultSOPC = SOPC_STATUS_OK;

    if (NULL == thread)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (P_THREAD_Destroy(&thread))
    {
        resultSOPC = SOPC_STATUS_OK;
    }
    else
    {
        resultSOPC = SOPC_STATUS_INVALID_STATE;
    }

    return resultSOPC;
}
