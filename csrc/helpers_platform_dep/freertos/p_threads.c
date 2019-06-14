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
    hCondVar signalThreadEnded;       // Cond var used to signal task end
    tThreadArgs args;
} tThreadWks;

/*****Private global definition*****/

static tUtilsList gTaskList = // Global task list
    {NULL, 0, 0, 0, 0, 0, 0, NULL};

static unsigned int bOverflowDetected = 0;

/*****Private thread api*****/

// Callback encapsulate user callback. Abstract start and stop synchronisation.
static void cbInternalCallback(void* ptr)
{
    hThread ptrArgs = (hThread) ptr;

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
            P_SYNCHRO_SignalAllConditionVariable(&ptrArgs->signalThreadEnded);

            xSemaphoreGiveRecursive(ptrArgs->lockRecHandle);
        }
    }
    DEBUG_decrementCpt();
    vTaskDelete(NULL);
}

// Destruction handle
void P_THREAD_Destroy(hThread** ptr)
{
    if (NULL != ptr && NULL != (*ptr))
    {
        P_THREAD_Join(*ptr);
        // Raz leaved memory
        memset(*ptr, 0, sizeof(hThread));
        vPortFree(*ptr);
        *ptr = NULL;
        DEBUG_decrementCpt();
    }
}
// Creation workspace
hThread* P_THREAD_Create(tPtrFct fct,              // Callback
                         void* args,               // Argument callback
                         tPtrFct fctWatingForJoin, // Debug thread waiting join
                         tPtrFct fctReadyToSignal) // Debug thread ended
{
    hThread* ptrWks = NULL;

    ptrWks = pvPortMalloc(sizeof(hThread));

    if (NULL != ptrWks)
    {
        DEBUG_incrementCpt();
        memset(ptrWks, 0, sizeof(hThread));

        // Initialization
        if (P_THREAD_Init(ptrWks, MAX_THREADS, fct, args, fctWatingForJoin, fctReadyToSignal) != E_THREAD_RESULT_OK)
        {
            P_THREAD_Destroy(&ptrWks);
            ptrWks = NULL;
        }
    }

    return ptrWks;
}

// Initializes created thread then launches it.
eThreadResult P_THREAD_Init(hThread* ptrWks,            // Workspace
                            unsigned short int wMaxRDV, // Max join
                            tPtrFct fct,                // Callback
                            void* args,                 // Args
                            tPtrFct fctWaitingForJoin,  // Debug wait for join
                            tPtrFct fctReadyToSignal)   // Debug wait for
{
    eThreadResult resPTHR = E_THREAD_RESULT_OK;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    hThread handleWks = NULL;

    if (NULL == ptrWks)
    {
        return E_THREAD_RESULT_ERROR_NOK;
    }

    // Create global task list for first call.
    if (NULL == gTaskList.lockHandle)
    {
        P_UTILS_LIST_InitMT(&gTaskList, MAX_THREADS);
    }
    if (NULL == gTaskList.lockHandle)
    {
        return E_THREAD_RESULT_ERROR_NOK;
    }

    if (NULL != (*ptrWks))
    {
        return E_THREAD_RESULT_ERROR_ALREADY_INITIALIZED;
    }

    /* Create the tThreadWks structure and assign it to (*ptrWks) */
    handleWks = pvPortMalloc(sizeof(tThreadWks));

    if (NULL == handleWks)
    {
        return E_THREAD_RESULT_ERROR_OUT_OF_MEM;
    }

    memset(handleWks, 0, sizeof(tThreadWks));

    handleWks->args.cbExternalCallback = fct;
    handleWks->cbReadyToSignal = fctReadyToSignal;
    handleWks->cbWaitingForJoin = fctWaitingForJoin;
    handleWks->args.ptrStartArgs = args;
    handleWks->handleTask = NULL;

    handleWks->signalReadyToWait = xSemaphoreCreateBinary();
    handleWks->signalReadyToStart = xSemaphoreCreateBinary();
    handleWks->lockRecHandle = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
    if ((*ptrWks)->signalReadyToWait)
    {
        DEBUG_incrementCpt();
    }
    if ((*ptrWks)->signalReadyToStart)
    {
        DEBUG_incrementCpt();
    }
    if ((*ptrWks)->lockRecHandle)
    {
        DEBUG_incrementCpt();
    }

    if (NULL == handleWks->signalReadyToWait || NULL == handleWks->signalReadyToStart ||
        NULL == handleWks->lockRecHandle)
    {
        resPTHR = E_THREAD_RESULT_ERROR_OUT_OF_MEM;
    }

    if (E_THREAD_RESULT_OK == resPTHR)
    {
        xSemaphoreTake(handleWks->signalReadyToStart, 0);
        xSemaphoreTake(handleWks->signalReadyToWait, 0);

        // List of task to exclude
        status = P_UTILS_LIST_InitMT(&handleWks->taskList, wMaxRDV);
        if (SOPC_STATUS_OK != status)
        {
            resPTHR = E_THREAD_RESULT_ERROR_OUT_OF_MEM;
        }
    }

    if (E_THREAD_RESULT_OK == resPTHR)
    {
        P_SYNCHRO_InitConditionVariable(&handleWks->signalThreadEnded, wMaxRDV);

        if (handleWks->signalThreadEnded == NULL)
        {
            resPTHR = E_THREAD_RESULT_ERROR_OUT_OF_MEM;
        }
    }
    if (E_THREAD_RESULT_OK == resPTHR)
    {
        BaseType_t resTaskCreate = xTaskCreate(cbInternalCallback,       // Callback
                                               "appThread",              // Friendly name
                                               configMINIMAL_STACK_SIZE, // Stack size
                                               handleWks,                // Workspace thread
                                               configMAX_PRIORITIES - 1, // Priority
                                               &handleWks->handleTask);  // Task handle
        if (pdPASS != resTaskCreate)
        {
            resPTHR = E_THREAD_RESULT_ERROR_NOK;
        }
    }

    if (E_THREAD_RESULT_OK == resPTHR)
    {
        DEBUG_incrementCpt();
        status = P_UTILS_LIST_AddEltMT(&gTaskList,            // Thread list
                                       handleWks->handleTask, // Handle task
                                       handleWks,             // Workspace
                                       0, 0);

        if (SOPC_STATUS_OK != status)
        {
            resPTHR = E_THREAD_RESULT_ERROR_MAX_THREADS;
        }
    }

    if (E_THREAD_RESULT_OK == resPTHR)
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
            vQueueDelete(handleWks->lockRecHandle);
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
        P_SYNCHRO_ClearConditionVariable(&handleWks->signalThreadEnded);

        // Reset structure memory
        memset(handleWks, 0, sizeof(tThreadWks));
    }

    return resPTHR;
}

// Joins thread. Thread joined becomes not initilized.
// Can be safely destroyed if just after return OK
eThreadResult P_THREAD_Join(hThread* pHandle)
{
    eThreadResult result = E_THREAD_RESULT_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_ERROR_NOK;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    tThreadWks* ptrCurrentThread = NULL;
    tThreadWks* pOthersThread = NULL;
    uint16_t wSlotId = UINT16_MAX;
    uint16_t wSlotIdRes = UINT16_MAX;
    TaskHandle_t handle = NULL;

    if (NULL == pHandle)
    {
        return E_THREAD_RESULT_ERROR_NOK;
    }

    if (NULL == gTaskList.lockHandle)
    {
        return E_THREAD_RESULT_ERROR_NOT_INITIALIZED;
    }

    // Get current workspace from current task handle
    ptrCurrentThread = P_UTILS_LIST_GetContextFromHandleMT(&gTaskList,                  // Global thread list
                                                           xTaskGetCurrentTaskHandle(), //
                                                           0,                           //
                                                           0);                          //

    if (NULL == ptrCurrentThread || NULL == ptrCurrentThread->lockRecHandle)
    {
        return E_THREAD_RESULT_ERROR_NOK;
    }

    tThreadWks* pThread = *pHandle;
    if (NULL == pThread || NULL == pThread->lockRecHandle)
    {
        return E_THREAD_RESULT_ERROR_NOT_INITIALIZED;
    }

    // Critical section release or deleted after unlock and wait
    xSemaphoreTakeRecursive(pThread->lockRecHandle, portMAX_DELAY);

    // Don't join the current thread or an invalid thread
    if (xTaskGetCurrentTaskHandle() == pThread->handleTask || NULL == pThread->handleTask)
    {
        result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
    }

    if (E_THREAD_RESULT_OK == result)
    {
        // Verify that current thread is not excluded from threads to join
        wSlotId = P_UTILS_LIST_GetEltIndexMT(&pThread->taskList,          // Local thread task list exception
                                             xTaskGetCurrentTaskHandle(), //
                                             0,                           //
                                             0);                          //

        if (wSlotId < UINT16_MAX)
        {
            result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
        }
    }

    if (E_THREAD_RESULT_OK == result)
    {
        // Verify that thread to join has not been joined by current thread
        wSlotId = P_UTILS_LIST_GetEltIndexMT(&ptrCurrentThread->taskList, //
                                             pThread->handleTask,         //
                                             0,                           //
                                             0);                          //

        if (wSlotId < UINT16_MAX)
        {
            result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
        }
    }

    if (E_THREAD_RESULT_OK == result)
    {
        // Add thread to join to the exclusion list of the current thread
        status = P_UTILS_LIST_AddEltMT(&pThread->taskList,          //
                                       xTaskGetCurrentTaskHandle(), //
                                       NULL,                        //
                                       0,                           //
                                       0);                          //

        if (SOPC_STATUS_OK != status)
        {
            result = E_THREAD_RESULT_ERROR_MAX_THREADS;
        }
    }

    if (E_THREAD_RESULT_OK == result)
    {
        // Append current task to join-exclusion list of the threads that reference the task to join
        wSlotId = UINT16_MAX;
        do
        {
            pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, //
                                                           &wSlotId);  //

            if (pOthersThread != ptrCurrentThread && NULL != pOthersThread)
            {
                wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                        pThread->handleTask,      //
                                                        0,                        //
                                                        0);                       //
                if (wSlotIdRes < UINT16_MAX)
                {
                    status = P_UTILS_LIST_AddEltMT(&pOthersThread->taskList,    //
                                                   xTaskGetCurrentTaskHandle(), //
                                                   NULL,                        //
                                                   0,                           //
                                                   0);                          //
                }
            }
        } while (UINT16_MAX != wSlotId && SOPC_STATUS_OK == status);

        if (SOPC_STATUS_OK != status)
        {
            // Restore if error occurred :
            // Threads that reference the task to join don't record in addition the current
            // thread
            wSlotId = UINT16_MAX;
            do
            {
                pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, &wSlotId);

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
                                                 0);                          //
                    }
                }
            } while (UINT16_MAX != wSlotId);

            result = E_THREAD_RESULT_ERROR_MAX_THREADS;
        }
    }

    if (E_THREAD_RESULT_OK == result)
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
                status = P_UTILS_LIST_AddEltMT(&pThread->taskList, //
                                               handle,             //
                                               NULL,               //
                                               0,                  //
                                               0);                 //
            }
        } while (UINT16_MAX != wSlotId && SOPC_STATUS_OK == status);

        if (SOPC_STATUS_OK != status)
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
                                             0);                 //
                }
            } while (UINT16_MAX != wSlotId);

            result = E_THREAD_RESULT_ERROR_MAX_THREADS;
        }
    }
    if (E_THREAD_RESULT_OK == result)
    {
        // Indicate that a thread is ready to wait for join
        xSemaphoreGive(pThread->signalReadyToWait);

        // The recursive mutex is taken. So, push handle to stack to notify
        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(&pThread->signalThreadEnded, //
                                                               &pThread->lockRecHandle,     //
                                                               JOINTURE_SIGNAL,             //
                                                               JOINTURE_CLEAR_SIGNAL,       //
                                                               ULONG_MAX);                  //
        // if OK, destroy workspace
        if (E_COND_VAR_RESULT_OK != resPSYNC)
        {
            result = E_THREAD_RESULT_ERROR_NOK;
        }
    }

    if (E_THREAD_RESULT_OK == result)
    {
        // Unlink the condition variable and the mutex to avoid deadlock in multiple
        // calls of Join
        QueueHandle_t tempLock = pThread->lockRecHandle;
        hCondVar tempCondVar = pThread->signalThreadEnded;
        pThread->lockRecHandle = NULL;
        pThread->signalThreadEnded = NULL;
        // After wait, clear condition variable. Unlock other join if necessary.
        P_SYNCHRO_ClearConditionVariable(&tempCondVar);
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
        P_UTILS_LIST_RemoveEltMT(&gTaskList,          //
                                 pThread->handleTask, //
                                 0,                   //
                                 0);                  //

        // Remove thread handle from local thread list exclusion
        P_UTILS_LIST_RemoveEltMT(&ptrCurrentThread->taskList, //
                                 pThread->handleTask,         //
                                 0,                           //
                                 0);                          //

        // Remove thread handle from other thread list exclusion
        wSlotId = UINT16_MAX;
        do
        {
            pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, //
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
                                             0);                       //
                }
            }
        } while (UINT16_MAX != wSlotId);

        pThread->handleTask = NULL;

        if (NULL != tempLock)
        {
            vQueueDelete(tempLock);
            tempLock = NULL;
            DEBUG_decrementCpt();
        }

        /* Raz leaved memory */
        memset(pThread, 0, sizeof(tThreadWks));

        vPortFree(pThread);
        *pHandle = NULL;
        pThread = NULL;
        DEBUG_decrementCpt();
    }

    // Critical section release if not deleted after unlock and wait
    if (NULL != pThread && NULL != pThread->lockRecHandle)
    {
        xSemaphoreGiveRecursive(pThread->lockRecHandle);
    }

    return result;
}

// Relative task delay
void P_THREAD_Sleep(unsigned int milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    bOverflowDetected = 0xAAAAAAAA;
}

/*****Public s2opc thread api*****/

// Create and initialize a thread
SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL != thread)
    {
        P_THREAD_Init(thread, MAX_THREADS, (tPtrFct) startFct, startArgs, NULL, NULL);

        if (NULL != *thread)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

// Join then destroy a thread
SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    hThread ptr = thread;
    eThreadResult result = E_THREAD_RESULT_ERROR_NOK;

    if (ptr != NULL)
    {
        result = P_THREAD_Join(&ptr);
        switch (result)
        {
        case E_THREAD_RESULT_OK:
            status = SOPC_STATUS_OK;
            break;
        case E_THREAD_RESULT_ERROR_NOK:
            status = SOPC_STATUS_NOK;
            break;
        case E_THREAD_RESULT_ERROR_MAX_THREADS:
            status = SOPC_STATUS_OUT_OF_MEMORY;
            break;
        case E_THREAD_RESULT_ERROR_NOT_INITIALIZED:
            status = SOPC_STATUS_INVALID_STATE;
            break;
        case E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD:
            status = SOPC_STATUS_NOK;
            break;
        case E_THREAD_RESULT_ERROR_ALREADY_INITIALIZED:
            status = SOPC_STATUS_INVALID_STATE;
            break;
        default:
            status = SOPC_STATUS_NOK;
            break;
        }
    }

    return status;
}

// Pause thread execution
void SOPC_Sleep(unsigned int milliseconds)
{
    P_THREAD_Sleep(milliseconds);
}
