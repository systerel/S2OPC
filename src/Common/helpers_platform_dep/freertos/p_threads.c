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

#include <limits.h> /* stdlib includes */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h" /* freeRtos includes */
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "sopc_enums.h" /* s2opc includes */
#include "sopc_mem_alloc.h"

#include "p_synchronisation.h" /* synchronisation include */
#include "p_threads.h"         /* private thread include */
#include "p_utils.h"           /* private list include */

#define MAX_THREADS MAX_WAITERS

/* Private structure definition */

typedef struct T_THREAD_ARGS
{
    tPtrFct* pCbExternalCallback; // External user callback
    void* ptrStartArgs;           // External user callback parameters
} tThreadArgs;

typedef struct T_THREAD_WKS
{
    tUtilsList taskList;                  // Task list joining this task
    tPtrFct* pCbWaitingForJoin;           // Debug callback
    tPtrFct* pCbReadyToSignal;            // Debug callback
    TaskHandle_t handleTask;              // Handle freeRtos task
    SemaphoreHandle_t lockRecHandle;      // Critical section
    SemaphoreHandle_t signalReadyToWait;  // Task wait for at least one join call
    SemaphoreHandle_t signalReadyToStart; // Authorize user callback execution
    Condition* pSignalThreadJoined;       // Cond var used to signal task end
    tThreadArgs args;
} tThreadWks;

/*****Private global definition*****/

static tUtilsList* pgTaskList = NULL;

/*****Private thread api*****/

// Callback encapsulate user callback. Abstract start and stop synchronisation.
static void cbInternalCallback(void* ptr)
{
    Thread ptrArgs = (Thread) ptr;

    if (NULL != ptrArgs)
    {
        if (NULL != ptrArgs->signalReadyToStart)
        {
            xSemaphoreTake(ptrArgs->signalReadyToStart, portMAX_DELAY);
        }

        if (NULL != ptrArgs->args.pCbExternalCallback)
        {
            (*ptrArgs->args.pCbExternalCallback)(ptrArgs->args.ptrStartArgs);
        }

        if (NULL != ptrArgs->pCbWaitingForJoin)
        {
            (*ptrArgs->pCbWaitingForJoin)(ptrArgs->args.ptrStartArgs);
        }

        if (NULL != ptrArgs->signalReadyToWait)
        {
            xSemaphoreTake(ptrArgs->signalReadyToWait, portMAX_DELAY);
        }

        if (NULL != ptrArgs->pCbReadyToSignal)
        {
            (*ptrArgs->pCbReadyToSignal)(ptrArgs->args.ptrStartArgs);
        }

        // At this level, wait for release mutex by condition variable called from join function
        if (NULL != ptrArgs->lockRecHandle)
        {
            xSemaphoreTakeRecursive(ptrArgs->lockRecHandle, portMAX_DELAY);

            // Signal terminaison thread
            P_SYNCHRO_SignalConditionVariable(ptrArgs->pSignalThreadJoined, false);

            xSemaphoreGiveRecursive(ptrArgs->lockRecHandle);
        }
    }
    DEBUG_decrementCpt();
    vTaskDelete(NULL);
}

// Initializes created thread then launches it.
SOPC_ReturnStatus P_THREAD_Init(Thread* ptrWks,   // Workspace
                                uint16_t wMaxRDV, // Max join
                                tPtrFct* pFct,    // Callback
                                void* args,       // Args
                                int priority,
                                const char* taskName,       // Name of the task
                                tPtrFct* fctWaitingForJoin, // Debug wait for join
                                tPtrFct* fctReadyToSignal)  // Debug wait for
{
    SOPC_ReturnStatus resPTHR = SOPC_STATUS_OK;
    SOPC_ReturnStatus resList = SOPC_STATUS_NOK;
    Thread handleWks = NULL;

    if (NULL == ptrWks || NULL == fct)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Create global task list for first call.
    /* TODO: outsource this initialization */
    if (NULL == pgTaskList)
    {
        pgTaskList = SOPC_Malloc(sizeof(tUtilsList));
        if (NULL != pgTaskList)
        {
            DEBUG_incrementCpt();
            memset(pgTaskList, 0, sizeof(tUtilsList));
            resList = P_UTILS_LIST_InitMT(pgTaskList, MAX_THREADS);
            if (SOPC_STATUS_OK != resList)
            {
                DEBUG_decrementCpt();
                SOPC_Free(pgTaskList);
                pgTaskList = NULL;
            }
        }
    }
    if (NULL == pgTaskList)
    {
        return SOPC_STATUS_NOK;
    }

    if (NULL != (*ptrWks))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    /* Create the tThreadWks structure and assign it to (*ptrWks) */
    handleWks = SOPC_Malloc(sizeof(tThreadWks));

    if (NULL == handleWks)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    DEBUG_incrementCpt();

    memset(handleWks, 0, sizeof(tThreadWks));

    handleWks->args.pCbExternalCallback = &fct;
    handleWks->pCbReadyToSignal = &fctReadyToSignal;
    handleWks->pCbWaitingForJoin = &fctWaitingForJoin;
    handleWks->args.ptrStartArgs = args;
    handleWks->handleTask = NULL;

    handleWks->signalReadyToWait = xSemaphoreCreateBinary();
    handleWks->signalReadyToStart = xSemaphoreCreateBinary();
    handleWks->lockRecHandle = xSemaphoreCreateRecursiveMutex();
    if (handleWks->signalReadyToWait)
    {
        DEBUG_incrementCpt();
    }
    if (handleWks->signalReadyToStart)
    {
        DEBUG_incrementCpt();
    }
    if (handleWks->lockRecHandle)
    {
        DEBUG_incrementCpt();
    }

    if (NULL == handleWks->signalReadyToWait || NULL == handleWks->signalReadyToStart ||
        NULL == handleWks->lockRecHandle)
    {
        resPTHR = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == resPTHR)
    {
        /* Semaphores (signals) are initialized "signaled", de-signal them */
        xSemaphoreTake(handleWks->signalReadyToStart, 0);
        xSemaphoreTake(handleWks->signalReadyToWait, 0);

        // List of task to exclude
        resList = P_UTILS_LIST_InitMT(&handleWks->taskList, wMaxRDV);
        if (SOPC_STATUS_OK != resList)
        {
            resPTHR = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == resPTHR)
    {
        handleWks->pSignalThreadJoined = P_SYNCHRO_CreateConditionVariable(wMaxRDV);

        if (handleWks->pSignalThreadJoined == NULL)
        {
            resPTHR = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == resPTHR)
    {
        BaseType_t resTaskCreate = xTaskCreate(cbInternalCallback, taskName == NULL ? "appThread" : taskName,
                                               configMINIMAL_STACK_SIZE, handleWks, priority, &handleWks->handleTask);

        if (pdPASS != resTaskCreate)
        {
            resPTHR = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == resPTHR)
    {
        DEBUG_incrementCpt();
        resList = P_UTILS_LIST_AddEltMT(pgTaskList,            // Thread list
                                        handleWks->handleTask, // Handle task
                                        handleWks,             // Workspace
                                        0, 0);

        if (SOPC_STATUS_OK != resList)
        {
            resPTHR = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == resPTHR)
    {
        // Set output parameter
        *ptrWks = handleWks;
        // Start thread
        xSemaphoreGive(handleWks->signalReadyToStart);
    }
    else
    {
        /* Error: clean partially initialized components */
        if (NULL != handleWks->handleTask)
        {
            vTaskSuspend(handleWks->handleTask);
            vTaskDelete(handleWks->handleTask);
            handleWks->handleTask = NULL;
            DEBUG_decrementCpt();
        }
        if (NULL != handleWks->lockRecHandle)
        {
            vSemaphoreDelete(handleWks->lockRecHandle);
            handleWks->lockRecHandle = NULL;
            DEBUG_decrementCpt();
        }
        if (NULL != handleWks->signalReadyToWait)
        {
            vQueueDelete(handleWks->signalReadyToWait);
            handleWks->signalReadyToWait = NULL;
            DEBUG_decrementCpt();
        }
        if (NULL != handleWks->signalReadyToStart)
        {
            vQueueDelete(handleWks->signalReadyToStart);
            handleWks->signalReadyToStart = NULL;
            DEBUG_decrementCpt();
        }

        P_UTILS_LIST_DeInitMT(&handleWks->taskList);
        P_SYNCHRO_DestroyConditionVariable(&handleWks->pSignalThreadJoined);
        // Reset structure memory
        memset(handleWks, 0, sizeof(tThreadWks));
    }

    return resPTHR;
}

// Joins thread. Thread joined becomes not initilized.
// Can be safely destroyed if just after return OK
SOPC_ReturnStatus P_THREAD_Join(Thread* pHandle)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    SOPC_ReturnStatus resPSYNC = SOPC_STATUS_NOK;
    SOPC_ReturnStatus resList = SOPC_STATUS_NOK;
    tThreadWks* ptrCurrentThread = NULL;
    tThreadWks* pOthersThread = NULL;
    uint16_t wSlotId = UINT16_MAX;
    uint16_t wSlotIdRes = UINT16_MAX;
    TaskHandle_t handle = NULL;

    if (NULL == pHandle)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pgTaskList)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    // Get current workspace from current task handle
    ptrCurrentThread = P_UTILS_LIST_GetContextFromHandleMT(pgTaskList,                  // Global thread list
                                                           xTaskGetCurrentTaskHandle(), //
                                                           0,                           //
                                                           0);                          //

    if (NULL == ptrCurrentThread || NULL == ptrCurrentThread->lockRecHandle)
    {
        return SOPC_STATUS_NOK;
    }

    tThreadWks* pThread = *pHandle;
    if (NULL == pThread || NULL == pThread->lockRecHandle)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    // Critical section release or deleted after unlock and wait
    xSemaphoreTakeRecursive(pThread->lockRecHandle, portMAX_DELAY);

    // Don't join the current thread or an invalid thread
    if (xTaskGetCurrentTaskHandle() == pThread->handleTask || NULL == pThread->handleTask)
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == result)
    {
        // Verify that current thread is not excluded from threads to join
        wSlotId = P_UTILS_LIST_GetEltIndexMT(&pThread->taskList,          // Local thread task list exception
                                             xTaskGetCurrentTaskHandle(), //
                                             0,                           //
                                             0);                          //

        if (wSlotId < UINT16_MAX)
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        // Verify that thread to join has not been joined by current thread
        wSlotId = P_UTILS_LIST_GetEltIndexMT(&ptrCurrentThread->taskList, //
                                             pThread->handleTask,         //
                                             0,                           //
                                             0);                          //

        if (wSlotId < UINT16_MAX)
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        // Add thread to join to the exclusion list of the current thread
        resList = P_UTILS_LIST_AddEltMT(&pThread->taskList,          //
                                        xTaskGetCurrentTaskHandle(), //
                                        NULL,                        //
                                        0,                           //
                                        0);                          //

        if (SOPC_STATUS_OK != resList)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        // Append current task to join-exclusion list of the threads that reference the task to join
        wSlotId = UINT16_MAX;
        do
        {
            pOthersThread = P_UTILS_LIST_ParseContextEltMT(pgTaskList, //
                                                           &wSlotId);  //

            if (pOthersThread != ptrCurrentThread && NULL != pOthersThread)
            {
                wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                        pThread->handleTask,      //
                                                        0,                        //
                                                        0);                       //
                if (wSlotIdRes < UINT16_MAX)
                {
                    resList = P_UTILS_LIST_AddEltMT(&pOthersThread->taskList,    //
                                                    xTaskGetCurrentTaskHandle(), //
                                                    NULL,                        //
                                                    0,                           //
                                                    0);                          //
                }
            }
        } while (UINT16_MAX != wSlotId && SOPC_STATUS_OK == resList);

        if (SOPC_STATUS_OK != resList)
        {
            // Restore if error occurred :
            // Threads that reference the task to join don't record in addition the current
            // thread
            wSlotId = UINT16_MAX;
            do
            {
                pOthersThread = P_UTILS_LIST_ParseContextEltMT(pgTaskList, &wSlotId);

                if (pOthersThread != ptrCurrentThread && NULL != pOthersThread)
                {
                    wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                            pThread->handleTask,      //
                                                            0,                        //
                                                            0);                       //

                    if (wSlotIdRes < UINT16_MAX)
                    {
                        P_UTILS_LIST_RemoveEltMT(&pOthersThread->taskList,    //
                                                 xTaskGetCurrentTaskHandle(), //
                                                 0,                           //
                                                 0,                           //
                                                 NULL);                       //
                    }
                }
            } while (UINT16_MAX != wSlotId);

            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        // Forward of all thread handle recorded by the current thread to thread to join
        // exclusion list
        wSlotId = UINT16_MAX;
        do
        {
            handle = (TaskHandle_t) P_UTILS_LIST_ParseValueEltMT(&ptrCurrentThread->taskList, //
                                                                 NULL,                        //
                                                                 NULL,                        //
                                                                 NULL,                        //
                                                                 &wSlotId);                   //

            if (NULL != handle)
            {
                resList = P_UTILS_LIST_AddEltMT(&pThread->taskList, //
                                                handle,             //
                                                NULL,               //
                                                0,                  //
                                                0);                 //
            }
        } while (UINT16_MAX != wSlotId && SOPC_STATUS_OK == resList);

        if (SOPC_STATUS_OK != resList)
        {
            // Restore in case of error
            wSlotId = UINT16_MAX;
            do
            {
                handle = (TaskHandle_t) P_UTILS_LIST_ParseValueEltMT(&ptrCurrentThread->taskList, //
                                                                     NULL,                        //
                                                                     NULL,                        //
                                                                     NULL,                        //
                                                                     &wSlotId);                   //
                if (NULL != handle)
                {
                    P_UTILS_LIST_RemoveEltMT(&pThread->taskList, //
                                             handle,             //
                                             0,                  //
                                             0,                  //
                                             NULL);              //
                }
            } while (UINT16_MAX != wSlotId);

            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == result)
    {
        // Indicate that a thread is ready to wait for join
        xSemaphoreGive(pThread->signalReadyToWait);

        // The recursive mutex is taken. So, push handle to stack to notify
        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(pThread->pSignalThreadJoined, //
                                                               &pThread->lockRecHandle,      //
                                                               JOINTURE_SIGNAL,              //
                                                               JOINTURE_CLEAR_SIGNAL,        //
                                                               ULONG_MAX);                   //
        // if OK, destroy workspace
        if (SOPC_STATUS_OK != resPSYNC)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        // Unlink the condition variable and the mutex to avoid deadlock in multiple
        // calls of Join
        SemaphoreHandle_t tempLock = pThread->lockRecHandle;
        pThread->lockRecHandle = NULL;

        // After wait, clear and destroy condition variable. Unlock other join if necessary.
        P_SYNCHRO_ClearConditionVariable(pThread->pSignalThreadJoined);
        P_SYNCHRO_DestroyConditionVariable(&pThread->pSignalThreadJoined);
        P_UTILS_LIST_DeInitMT(&pThread->taskList);

        if (NULL != pThread->signalReadyToWait)
        {
            vQueueDelete(pThread->signalReadyToWait);
            pThread->signalReadyToWait = NULL;
            DEBUG_decrementCpt();
        }
        if (NULL != pThread->signalReadyToStart)
        {
            vQueueDelete(pThread->signalReadyToStart);
            pThread->signalReadyToStart = NULL;
            DEBUG_decrementCpt();
        }

        // Remove thread from global list
        P_UTILS_LIST_RemoveEltMT(pgTaskList,          //
                                 pThread->handleTask, //
                                 0,                   //
                                 0,                   //
                                 NULL);               //

        // Remove thread handle from local thread list exclusion
        P_UTILS_LIST_RemoveEltMT(&ptrCurrentThread->taskList, //
                                 pThread->handleTask,         //
                                 0,                           //
                                 0,                           //
                                 NULL);                       //

        // Remove thread handle from other thread list exclusion
        wSlotId = UINT16_MAX;
        do
        {
            pOthersThread = P_UTILS_LIST_ParseContextEltMT(pgTaskList, //
                                                           &wSlotId);  //

            if (pOthersThread != ptrCurrentThread && NULL != pOthersThread)
            {
                wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                        pThread->handleTask,      //
                                                        0,                        //
                                                        0);                       //
                if (wSlotIdRes < UINT16_MAX)
                {
                    P_UTILS_LIST_RemoveEltMT(&pOthersThread->taskList, //
                                             pThread->handleTask,      //
                                             0,                        //
                                             0,                        //
                                             NULL);                    //
                }
            }
        } while (UINT16_MAX != wSlotId);

        pThread->handleTask = NULL;

        if (NULL != tempLock)
        {
            vSemaphoreDelete(tempLock);
            tempLock = NULL;
            DEBUG_decrementCpt();
        }

        /* Raz leaved memory */
        memset(pThread, 0, sizeof(tThreadWks));

        SOPC_Free(pThread);
        *pHandle = NULL;
        pThread = NULL;
        DEBUG_decrementCpt();
    }

    // Critical section release if not deleted after unlock and wait.
    // *pHandle might have changed in another P_THREAD_Join.
    /* TODO: check if this block is still needed */
    pThread = *pHandle;
    if (NULL != pThread && NULL != pThread->lockRecHandle)
    {
        xSemaphoreGiveRecursive(pThread->lockRecHandle);
    }

    return result;
}

// Relative task delay
/*****Public s2opc thread api*****/

// Create and initialize a thread
SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* taskName)
{
    return P_THREAD_Init(thread, MAX_THREADS, (tPtrFct) startFct, startArgs, 0, taskName, NULL, NULL);
}

SOPC_ReturnStatus SOPC_Thread_CreatePrioritized(Thread* thread,
                                                void* (*startFct)(void*),
                                                void* startArgs,
                                                int priority,
                                                const char* taskName)
{
    if (priority < 1 || priority > configMAX_PRIORITIES)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P_THREAD_Init(thread, MAX_THREADS, (tPtrFct) startFct, startArgs, priority, taskName, NULL, NULL);
}

// Join then destroy a thread
SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL != thread)
    {
        status = P_THREAD_Join(&thread);
    }

    return status;
}

// Pause thread execution
void SOPC_Sleep(unsigned int milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}
