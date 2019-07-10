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
#include "queue.h"
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
    tPtrFct cbExternalCallback; // External user callback
    void* ptrStartArgs;         // External user callback parameters
} tThreadArgs;

typedef struct T_THREAD_WKS
{
    tUtilsList taskList;              // Task list joining this task
    tPtrFct cbWaitingForJoin;         // Debug callback
    tPtrFct cbReadyToSignal;          // Debug callback
    TaskHandle_t handleTask;          // Handle freeRtos task
    QueueHandle_t lockRecHandle;      // Critical section
    QueueHandle_t signalReadyToWait;  // Task wait for at least one join call
    QueueHandle_t signalReadyToStart; // Autorize user callback execution
    Condition* pSignalThreadJoined;   // Cond var used to signal task end
    tThreadArgs args;
} tThreadWks;

/*****Private global definition*****/

static tUtilsList* pgTaskList = NULL;

/*****Private thread api*****/

// Callback encapsulate user callback. Abstract start and stop synchronisation.
static void cbInternalCallback(void* ptr)
{
    Thread ptrArgs = (Thread) ptr;

    if (ptrArgs != NULL)
    {
        if (ptrArgs->signalReadyToStart != NULL)
        {
            xSemaphoreTake(ptrArgs->signalReadyToStart, portMAX_DELAY);
        }

        if (ptrArgs->args.cbExternalCallback != NULL)
        {
            ptrArgs->args.cbExternalCallback(ptrArgs->args.ptrStartArgs);
        }

        if (ptrArgs->cbWaitingForJoin != NULL)
        {
            ptrArgs->cbWaitingForJoin(ptrArgs->args.ptrStartArgs);
        }

        if (ptrArgs->signalReadyToWait != NULL)
        {
            xSemaphoreTake(ptrArgs->signalReadyToWait, portMAX_DELAY);
        }

        if (ptrArgs->cbReadyToSignal != NULL)
        {
            ptrArgs->cbReadyToSignal(ptrArgs->args.ptrStartArgs);
        }

        // At this level, wait for release mutex by condition variable called from join function
        if (ptrArgs->lockRecHandle != NULL)
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

// Destruction handle
void P_THREAD_Destroy(Thread** ptr)
{
    if (NULL != ptr && NULL != (*ptr))
    {
        P_THREAD_Join(*ptr);
        // Raz leaved memory
        memset(*ptr, 0, sizeof(Thread));
        SOPC_Free(*ptr);
        *ptr = NULL;
        DEBUG_decrementCpt();
    }
}
// Creation workspace
Thread*                                   // Handle workspace
P_THREAD_Create(tPtrFct fct,              // Callback
                void* args,               // Argument callback
                const char* taskName,     // Name of the task
                tPtrFct fctWatingForJoin, // Debug thread waiting join
                tPtrFct fctReadyToSignal) // Debug thread ended
{
    Thread* ptrWks = NULL;

    ptrWks = SOPC_Malloc(sizeof(Thread));

    if (NULL != ptrWks)
    {
        DEBUG_incrementCpt();
        memset(ptrWks, 0, sizeof(Thread));

        // Initialization
        if (P_THREAD_Init(ptrWks,                              //
                          MAX_THREADS,                         //
                          fct,                                 //
                          args,                                //
                          taskName,                            //
                          fctWatingForJoin,                    //
                          fctReadyToSignal) != SOPC_STATUS_OK) //
        {
            P_THREAD_Destroy(&ptrWks);
            ptrWks = NULL;
        }
    }

    return ptrWks;
}

// Initializes created thread then launches it.
SOPC_ReturnStatus P_THREAD_Init(Thread* ptrWks,            // Workspace
                                uint16_t wMaxRDV,          // Max join
                                tPtrFct fct,               // Callback
                                void* args,                // Args
                                const char* taskName,      // Name of the task
                                tPtrFct fctWaitingForJoin, // Debug wait for join
                                tPtrFct fctReadyToSignal)  // Debug wait for
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

    handleWks->args.cbExternalCallback = fct;
    handleWks->cbReadyToSignal = fctReadyToSignal;
    handleWks->cbWaitingForJoin = fctWaitingForJoin;
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
        BaseType_t resTaskCreate = xTaskCreate(cbInternalCallback,       // Callback
                                               taskName,                 // Friendly name
                                               configMINIMAL_STACK_SIZE, // Stack size
                                               handleWks,                // Workspace thread
                                               configMAX_PRIORITIES - 1, // Priority
                                               &handleWks->handleTask);  // Task handle
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
        if (handleWks->handleTask != NULL)
        {
            vTaskSuspend(handleWks->handleTask);
            vTaskDelete(handleWks->handleTask);
            handleWks->handleTask = NULL;
            DEBUG_decrementCpt();
        }
        if (handleWks->lockRecHandle != NULL)
        {
            vSemaphoreDelete(handleWks->lockRecHandle);
            handleWks->lockRecHandle = NULL;
            DEBUG_decrementCpt();
        }
        if (handleWks->signalReadyToWait != NULL)
        {
            vQueueDelete(handleWks->signalReadyToWait);
            handleWks->signalReadyToWait = NULL;
            DEBUG_decrementCpt();
        }
        if (handleWks->signalReadyToStart != NULL)
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
        QueueHandle_t tempLock = pThread->lockRecHandle;
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
void P_THREAD_Sleep(uint32_t milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

/*****Public s2opc thread api*****/

// Create and initialize a thread
SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs, const char* name)
{
    return P_THREAD_Init(thread, MAX_THREADS, (tPtrFct) startFct, startArgs, name, NULL, NULL);
}

// Join then destroy a thread
SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (thread != NULL)
    {
        status = P_THREAD_Join(&thread);
    }

    return status;
}

// Pause thread execution
void SOPC_Sleep(unsigned int milliseconds)
{
    P_THREAD_Sleep(milliseconds);
}
