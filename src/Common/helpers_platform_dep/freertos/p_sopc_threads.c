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

/* stdlib includes */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h" /* freeRtos includes */
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* s2opc includes */
#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "p_sopc_synchronisation.h" /* synchronisation include */
#include "p_sopc_threads.h"         /* private thread include */
#include "p_sopc_utils.h"           /* private list include */

#define MAX_THREADS MAX_WAITERS

// The FreeRTOS "No Task" marker
#define NO_FREERTOS_TASK ((TaskHandle_t) NULL)

/* Private structure definition */

typedef struct T_THREAD_ARGS
{
    tPtrFct* pEntry; // External user callback
    void* ptrArgs;   // External user callback parameters
} tThreadArgs;

struct SOPC_Thread_Impl

{
    TaskHandle_t handleTask;        // Handle freeRtos task
    SemaphoreHandle_t signalJoined; // Task received a join call
    tThreadArgs args;
};

/*****Private global definition*****/
static SemaphoreHandle_t gMutex; // Critical section
static SOPC_Thread_Impl gThreads[MAX_THREADS];

/*****Private thread api*****/
static void criticalSectionIn(void)
{
    if (gMutex == NULL)
    {
        gMutex = xSemaphoreCreateBinary();
    }
    SOPC_ASSERT(NULL != gMutex);
    xSemaphoreTake(gMutex, 0);
}

static inline void criticalSectionOut(void)
{
    SOPC_ASSERT(NULL != gMutex);
    xSemaphoreGive(gMutex);
}

static SOPC_Thread_Impl* getFreeThread(void)
{
    SOPC_Thread_Impl* result = NULL;
    SOPC_Thread_Impl* pThr = gThreads;
    for (size_t i = 0; i < MAX_THREADS && NULL == result; i++, pThr++)
    {
        if (pThr->handleTask == NO_FREERTOS_TASK)
        {
            result = pThr;
        }
    }
    return result;
}

static bool checkThread(const SOPC_Thread_Impl* pThr)
{
    bool result = false;
    SOPC_Thread_Impl* pThrLoop = &gThreads[0];
    for (size_t i = 0; i < MAX_THREADS && !result; i++, pThrLoop++)
    {
        if (pThrLoop == pThr)
        {
            result = true;
        }
    }
    return result;
}

static inline void freeThread(SOPC_Thread_Impl* pThr)
{
    SOPC_ASSERT(NULL != pThr);
    if (NULL != pThr->signalJoined)
    {
        vSemaphoreDelete(pThr->signalJoined);
    }
    memset(pThr, 0, sizeof(*pThr));
}

static configSTACK_DEPTH_TYPE getStckSizeByName(const char* name)
{
    static const uint32_t oneKb = 1024;
    uint32_t nbBytes = oneKb;
    if (name != NULL)
    {
        if (0 == strcmp(name, "Services") || 0 == strcmp(name, "Secure_Channels"))
        {
            nbBytes = oneKb * 6;
        }
        else if (0 == strcmp(name, "Publisher") || 0 == strcmp(name, "Sockets") || 0 == strcmp(name, "SubSocketMgr") ||
                 0 == strcmp(name, "Application"))
        {
            nbBytes = oneKb * 3;
        }
    }

    return nbBytes / sizeof(configSTACK_DEPTH_TYPE);
}

// Callback encapsulate user callback. Abstract start and stop synchronization.
static void cbInternalCallback(void* ptr)
{
    SOPC_Thread ptrArgs = (SOPC_Thread) ptr;
    SOPC_ASSERT(NULL != ptrArgs && NULL != ptrArgs->args.pEntry && NULL != ptrArgs->signalJoined);

    // Call user task
    (*ptrArgs->args.pEntry)(ptrArgs->args.ptrArgs);

    // on return, notify that task is ready to join
    xSemaphoreGive(ptrArgs->signalJoined);

    vTaskDelete(NULL);
}

// Initializes created thread then launches it.
SOPC_ReturnStatus P_THREAD_Init(SOPC_Thread* ptrWks, // Workspace
                                tPtrFct* pFct,       // Callback
                                void* args,          // Args
                                int priority,
                                const char* taskName) // Debug wait for
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == ptrWks || NULL == pFct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL != (*ptrWks))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    /* Create the SOPC_Thread_Impl structure and assign it to (*ptrWks) */
    criticalSectionIn();
    struct SOPC_Thread_Impl* handleWks = getFreeThread();

    if (NULL == handleWks)
    {
        criticalSectionOut();
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SemaphoreHandle_t semJoin = xSemaphoreCreateBinary();
    if (semJoin == NULL)
    {
        criticalSectionOut();
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    handleWks->args.pEntry = pFct;
    handleWks->args.ptrArgs = args;
    handleWks->handleTask = NO_FREERTOS_TASK;
    handleWks->signalJoined = semJoin;

    /* Semaphores (signals) are initialized "signaled", de-signal them */
    xSemaphoreTake(handleWks->signalJoined, 0);
    criticalSectionOut();

    BaseType_t resTaskCreate = xTaskCreate(cbInternalCallback, taskName == NULL ? "appThread" : taskName,
                                           getStckSizeByName(taskName), handleWks, priority, &handleWks->handleTask);

    if (pdPASS != resTaskCreate)
    {
        result = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK != result)
    {
        /* Error: clean partially initialized components */
        criticalSectionIn();
        freeThread(handleWks);
        criticalSectionOut();
    }
    else
    {
        *ptrWks = handleWks;
    }

    return result;
}

SOPC_ReturnStatus P_THREAD_Join(SOPC_Thread pHandle)
{
    if (NULL == pHandle)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Thread_Impl* pThr = pHandle;
    if (!checkThread(pThr))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Note : the same thread cannot be joined several times
    SOPC_ASSERT(NULL != pThr->signalJoined && NO_FREERTOS_TASK != pThr->handleTask);

    // Wait for the thread to terminate.
    xSemaphoreTake(pThr->signalJoined, portMAX_DELAY);

    // Remove task from list
    criticalSectionIn();
    freeThread(pThr);
    criticalSectionOut();

    return SOPC_STATUS_OK;
}

// Relative task delay
/*****Public s2opc thread api*****/

// Create and initialize a thread
SOPC_ReturnStatus SOPC_Thread_Create(SOPC_Thread* thread,
                                     void* (*startFct)(void*),
                                     void* startArgs,
                                     const char* taskName)
{
    static const int priority = 1; // 0 Is the lowest priority (IDLE)
    return P_THREAD_Init(thread, (tPtrFct*) startFct, startArgs, priority, taskName);
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(SOPC_Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                int cpuAffinity,
                                                const char* taskName)
{
    SOPC_UNUSED_ARG(cpuAffinity);
    if (priority < 1 || priority > configMAX_PRIORITIES)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P_THREAD_Init(thread, (tPtrFct*) startFct, startArgs, priority, taskName);
}

// Join then destroy a thread
SOPC_ReturnStatus SOPC_Thread_Join(SOPC_Thread* thread)
{
    if (NULL == thread)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (SOPC_INVALID_THREAD != *thread)
    {
        status = P_THREAD_Join(*thread);
        if (SOPC_STATUS_OK == status)
        {
            *thread = SOPC_INVALID_THREAD;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

// Pause thread execution
void SOPC_Sleep(unsigned int milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

// Hook used to show stack overflows
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    SOPC_UNUSED_ARG(xTask);
    SOPC_UNUSED_ARG(pcTaskName);
    vTaskDelay(500);
    SOPC_Assert_Failure(0);
}
