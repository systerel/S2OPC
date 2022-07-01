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
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

#include "kernel.h"

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

#include "sopc_threads.h"
#include "zephyr/p_threads.h"

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
#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
    uint8_t marker;
#endif
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
K_KERNEL_STACK_DEFINE(gThreadStack_Timers, SOPC_UTILITY_TASK_STACK_SIZE);

K_KERNEL_STACK_DEFINE(gThreadStack_Application, SOPC_MAIN_TASK_STACK_SIZE);
K_KERNEL_STACK_DEFINE(gThreadStack_SubSocketMgr, SOPC_MAIN_TASK_STACK_SIZE);
K_KERNEL_STACK_DEFINE(gThreadStack_Sockets, SOPC_MAIN_TASK_STACK_SIZE);
K_KERNEL_STACK_DEFINE(gThreadStack_Publisher, SOPC_MAIN_TASK_STACK_SIZE);

K_KERNEL_STACK_DEFINE(gThreadStack_Secure_Chann, SOPC_LARGE_TASK_STACK_SIZE);
K_KERNEL_STACK_DEFINE(gThreadStack_Services, SOPC_LARGE_TASK_STACK_SIZE);

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
K_THREAD_STACK_ARRAY_DEFINE(gThreadStacks, CONFIG_SOPC_MAX_USER_TASKS, CONFIG_SOPC_USER_STACK_SIZE* KILOBYTE);
static sopcThreadConfig gUserTasks[CONFIG_SOPC_MAX_USER_TASKS];

// This is the handle returned to caller
struct tThreadHandle
{
    const sopcThreadConfig* pCfg;
};

static bool gInitialized = false;
static struct k_mutex gLock;

////////////////////////////////////////////////////////
/////////// DEBUG SECTION //////////////////////////////
////////////////////////////////////////////////////////
#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
#define PRINTK_DEBUG printk

#define STRING_SECURE(s) ((s) ? (s) : "<NULL>")

// Start stack marker value at any random value, It is incremented for each task, so that
// each task stack will be filled with an unique remarkable value.
static uint8_t gNextMarker = 0x90;

// Note: additionnal slot for termination indication.Last element is empty
SOPC_Thread_Info allThread_Infos[T_THREAD_NB_THREADS + 1];
static size_t threadsCount = 0;

static bool SOPC_Thread_Debug_FillSingle(const sopcThreadConfig* pCfg, size_t s)
{
    bool result = false;
    SOPC_Thread_Info* pInfo = &allThread_Infos[s];
    if (pCfg->pWks->userCallback != NULL)
    {
        // Fill in result
        pInfo->name = pCfg->pWks->taskName;
        pInfo->stack_size = pCfg->stackSize;

        // Compute stack usage for thread. (stack begins on top)
        const uint32_t* stackBottom = (const uint32_t*) (pCfg->stackBase);
        const uint32_t* stackTop = (const uint32_t*) (((const char*) pCfg->stackBase) + pCfg->stackSize);
        const uint32_t marker = ((uint32_t) pCfg->pWks->marker) * 0x01010101;

        const uint32_t* ptr = stackBottom;

        while (marker == (*ptr) && ptr < stackTop)
        {
            ptr++;
        }
        pInfo->stack_usage = sizeof(*ptr) * (uintptr_t)(stackTop - ptr);
        pInfo->priority = pCfg->pWks->priority;
        result = true;
    }
    else
    {
        pInfo->name = NULL;
        pInfo->stack_size = 0;
        pInfo->priority = 255;
        pInfo->stack_usage = 0;
    }
    return result;
}
const SOPC_Thread_Info* SOPC_Thread_GetAllThreadsInfo(void)
{
    size_t s = 0;
    sopcThreadConfig* pCfg = gSopcTasks;
    bool result;
    for (size_t k = 0; k < NB_SOPC_THREADS; k++)
    {
        result = SOPC_Thread_Debug_FillSingle(pCfg, s);
        if (result)
        {
            s++;
        }
        pCfg++;
    }
    pCfg = gUserTasks;
    for (size_t k = 0; k < CONFIG_SOPC_MAX_USER_TASKS; k++)
    {
        result = SOPC_Thread_Debug_FillSingle(pCfg, s);
        if (result)
        {
            s++;
        }
        pCfg++;
    }
    return allThread_Infos;
}

static void DEBUG_DELETE_THREAD(const tThreadWks* pWks) {}

static void DEBUG_CREATE_THREAD(const char* pName, const sopcThreadConfig* pCfg)
{
    // note: protected by gLock
    char* stackBottom = (char*) pCfg->stackBase;
    pCfg->pWks->marker = gNextMarker;
    gNextMarker++;

    // Fill stack memory with specific value (marker), so that we can later estimate how much has been used.
    memset(stackBottom, pCfg->pWks->marker, pCfg->stackSize);
    PRINTK_DEBUG("Initialize stack [%p.. %p] for thread %s MARK=%02X\n", stackBottom, stackBottom + pCfg->stackSize,
                 STRING_SECURE(pName), pCfg->pWks->marker);
    threadsCount++;
}
#else
#define PRINTK_DEBUG(...)
#define DEBUG_CREATE_THREAD(name, cfg)
#define DEBUG_DELETE_THREAD(name)
#endif
////////////////////////////////////////////////////////
///////// DEBUG SECTION END ////////////////////////////
////////////////////////////////////////////////////////

/**** Configure thread-specific tuning ****/
/**
 * @brief returns the expected stack size of a given task, based on its name.
 * @param taskName The task name.
 * @return The stack configuration to use.
 */
static const sopcThreadConfig* P_THREAD_GetStackCfg(const char* taskName, const bool isSOPCThread)
{
    const sopcThreadConfig* result = NULL;
    if (isSOPCThread)
    {
        SOPC_ASSERT(NULL != taskName);
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
    else
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
        PRINTK_DEBUG("Entering Thread %s\n", STRING_SECURE(pWks->taskName));
        pWks->userCallback(pCtx);
        PRINTK_DEBUG("Exit Thread %s\n", STRING_SECURE(pWks->taskName));
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
        PRINTK_DEBUG("Invalid priority %d\n", priority);
        return false;
    }
    if (NULL == pCfg)
    {
        PRINTK_DEBUG("Cannot create task <%s>. (No free slots)\n", STRING_SECURE(taskName));
        return false;
    }
    tThreadWks* pWks = pCfg->pWks;

    SOPC_ASSERT(NULL != pWks);

    // Launch thread
    k_sem_init(&pWks->kSemThreadEnded, 0, 1);
    pWks->taskName = taskName;
    pWks->userCallback = callback;
    pWks->joined = false;
    pWks->priority = priority - CONFIG_NUM_COOP_PRIORITIES;
    PRINTK_DEBUG("Create task %s Stack=%p(0x%04X), entry=%p, filter=%s\n", STRING_SECURE(pWks->taskName),
                 pCfg->stackBase, (unsigned) pCfg->stackSize, pWks->userCallback, STRING_SECURE(pCfg->nameFilter));
    DEBUG_CREATE_THREAD(STRING_SECURE(taskName), pCfg);
    pWks->threadHandle = k_thread_create(&pWks->threadControlBlock, pCfg->stackBase, pCfg->stackSize,
                                         P_THREAD_InternalCallback, pWks, pCtx, NULL, pWks->priority, 0, K_NO_WAIT);

    // Initialize signal for join and try to launch thread
    if (NULL == pWks->threadHandle)
    {
        k_sem_reset(&pWks->kSemThreadEnded);
        memset(pWks, 0, sizeof(tThreadWks));
        pCfg = NULL;
        PRINTK_DEBUG("Create task %s failed \n", STRING_SECURE(taskName));
        DEBUG_DELETE_THREAD(pWks);
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
tThreadHandle* P_THREAD_Create(ptrFct* callback,
                               void* pCtx,
                               const char* taskName,
                               const int priority,
                               const bool isSOPCThread)
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
        }

        // Fill-in user threads configuration
        gInitialized = true;
        PRINTK_DEBUG("STACK AREA = \n");
        for (int k = 0; k < NB_SOPC_THREADS; k++)
        {
            PRINTK_DEBUG("- [%p .. %p] : (SOPC) %s\n", gSopcTasks[k].stackBase,
                         ((char*) gSopcTasks[k].stackBase) + gSopcTasks[k].stackSize,
                         STRING_SECURE(gSopcTasks[k].nameFilter));
        }
        PRINTK_DEBUG("- [%p .. %p] : (%d * user task)\n", gThreadStacks[0],
                     (void*) (((uintptr_t) gThreadStacks) + sizeof(gThreadStacks)), (int) CONFIG_SOPC_MAX_USER_TASKS);
        // Find idle thread
        PRINTK_DEBUG("- [%p .. %p] : (Idle)\n", (void*) _kernel.cpus[0].idle_thread->stack_info.start,
                     (void*) (((uintptr_t) _kernel.cpus[0].idle_thread->stack_info.start) +
                              _kernel.cpus[0].idle_thread->stack_info.size));
    }

    k_mutex_lock(&gLock, K_FOREVER);

    pHandle = SOPC_Calloc(1, sizeof(struct tThreadHandle));

    if (NULL != pHandle)
    {
        const sopcThreadConfig* stack_cfg = P_THREAD_GetStackCfg(taskName, isSOPCThread);
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
        DEBUG_DELETE_THREAD(pWks);
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
SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* taskName)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *thread = P_THREAD_Create(startFct, startArgs, taskName, CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, true);

    if (NULL == *thread)
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName)
{
    // No specific limit on priorites on ZEPHYR.
    if (priority < -CONFIG_NUM_COOP_PRIORITIES || priority >= CONFIG_NUM_PREEMPT_PRIORITIES)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == thread || NULL == startFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *thread = P_THREAD_Create(startFct, startArgs, taskName, priority, true);

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
