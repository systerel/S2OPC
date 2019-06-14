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
    if ((ptr != NULL) && ((*ptr) != NULL))
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

    ptrWks = (hThread*) pvPortMalloc(sizeof(hThread));

    if (ptrWks != NULL)
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
        NULL == handleWks->lockRecHandle == NULL)
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
    eThreadResult result = E_THREAD_RESULT_ERROR_NOK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_ERROR_NOK;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    tThreadWks* ptrCurrentThread = NULL;
    tThreadWks* pOthersThread = NULL;
    uint16_t wSlotId = UINT16_MAX;
    uint16_t wSlotIdRes = UINT16_MAX;
    TaskHandle_t handle = NULL;

    // Get current workspace from current task handle
    if ((gTaskList.lockHandle != NULL) && (pHandle != NULL) && ((*pHandle)->lockRecHandle != NULL))
    {
        ptrCurrentThread = P_UTILS_LIST_GetContextFromHandleMT(&gTaskList,                  // Global thread list
                                                               xTaskGetCurrentTaskHandle(), //
                                                               0,                           //
                                                               0);                          //
    }

    if ((ptrCurrentThread == NULL) || (ptrCurrentThread->lockRecHandle == NULL))
    {
        result = E_THREAD_RESULT_ERROR_NOK;
    }
    else
    {
        if (((*pHandle) == NULL) || ((*pHandle)->lockRecHandle == NULL))
        {
            result = E_THREAD_RESULT_ERROR_NOT_INITIALIZED;
        }
        else
        {
            // Critical section release or deleted after unlock and wait
            xSemaphoreTakeRecursive((*pHandle)->lockRecHandle, portMAX_DELAY);

            // Current thread is not the thread to join
            if (((*pHandle)->handleTask == xTaskGetCurrentTaskHandle()) || (((*pHandle)->handleTask == NULL)))
            {
                result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
            }
            else
            {
                // Current thread is not exclude by thread to join
                wSlotId = P_UTILS_LIST_GetEltIndexMT(&(*pHandle)->taskList,       // Local thread task list exception
                                                     xTaskGetCurrentTaskHandle(), //
                                                     0,                           //
                                                     0);                          //

                if (wSlotId < UINT16_MAX)
                {
                    result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
                }
                else
                {
                    // Thread to join has not been joined by current thread
                    wSlotId = P_UTILS_LIST_GetEltIndexMT(&ptrCurrentThread->taskList, //
                                                         (*pHandle)->handleTask,      //
                                                         0,                           //
                                                         0);                          //

                    if (wSlotId < UINT16_MAX)
                    {
                        result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
                    }
                    else
                    {
                        // THread to join add to its list current thread
                        status = P_UTILS_LIST_AddEltMT(&(*pHandle)->taskList,       //
                                                       xTaskGetCurrentTaskHandle(), //
                                                       NULL,                        //
                                                       0,                           //
                                                       0);                          //

                        if (SOPC_STATUS_OK != status)
                        {
                            result = E_THREAD_RESULT_ERROR_MAX_THREADS;
                        }
                        else
                        {
                            // Threads that reference the task to join record in addition the current thread
                            wSlotId = UINT16_MAX;
                            do
                            {
                                pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, //
                                                                               &wSlotId);  //

                                if ((pOthersThread != ptrCurrentThread) && (pOthersThread != NULL))
                                {
                                    wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                                            (*pHandle)->handleTask,   //
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
                            } while ((wSlotId != UINT16_MAX) && (SOPC_STATUS_OK == status));

                            if (SOPC_STATUS_OK != status)
                            {
                                // Restore if error occurred :
                                // Threads that reference the task to join don't record in addition the current
                                // thread
                                wSlotId = UINT16_MAX;
                                do
                                {
                                    pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, &wSlotId);

                                    if ((pOthersThread != ptrCurrentThread) && (pOthersThread != NULL))
                                    {
                                        wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                                                (*pHandle)->handleTask,   //
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
                                } while (wSlotId != UINT16_MAX);

                                result = E_THREAD_RESULT_ERROR_MAX_THREADS;
                            }
                            else
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

                                    if (handle != NULL)
                                    {
                                        status = P_UTILS_LIST_AddEltMT(&(*pHandle)->taskList, //
                                                                       handle,                //
                                                                       NULL,                  //
                                                                       0,                     //
                                                                       0);                    //
                                    }
                                } while ((wSlotId != UINT16_MAX) && (SOPC_STATUS_OK == status));

                                if (SOPC_STATUS_OK != status)
                                {
                                    // Restore in case of error
                                    wSlotId = UINT16_MAX;
                                    do
                                    {
                                        handle =
                                            (TaskHandle_t) P_UTILS_LIST_ParseValueEltMT(&ptrCurrentThread->taskList, //
                                                                                        NULL,                        //
                                                                                        NULL,                        //
                                                                                        NULL,                        //
                                                                                        &wSlotId);                   //
                                        if (handle != NULL)
                                        {
                                            P_UTILS_LIST_RemoveEltMT(&(*pHandle)->taskList, //
                                                                     handle,                //
                                                                     0,                     //
                                                                     0);                    //
                                        }
                                    } while (wSlotId != UINT16_MAX);

                                    result = E_THREAD_RESULT_ERROR_MAX_THREADS;
                                }
                                else
                                {
                                    // Indicate that a thread is ready to wait for join
                                    xSemaphoreGive((*pHandle)->signalReadyToWait);

                                    // The recursive mutex is taken. So, push handle to stack to notify
                                    resPSYNC =
                                        P_SYNCHRO_UnlockAndWaitForConditionVariable(&(*pHandle)->signalThreadEnded, //
                                                                                    &(*pHandle)->lockRecHandle,     //
                                                                                    JOINTURE_SIGNAL,                //
                                                                                    JOINTURE_CLEAR_SIGNAL,          //
                                                                                    ULONG_MAX);                     //
                                    // if OK, destroy workspace
                                    if (resPSYNC == E_COND_VAR_RESULT_OK)
                                    {
                                        // Unlink the condition variable and the mutex to avoid deadlock in multiple
                                        // calls of Join
                                        QueueHandle_t tempLock = (*pHandle)->lockRecHandle;
                                        hCondVar tempCondVar = (*pHandle)->signalThreadEnded;
                                        (*pHandle)->lockRecHandle = NULL;
                                        (*pHandle)->signalThreadEnded = NULL;
                                        // After wait, clear condition variable. Unlock other join if necessary.
                                        P_SYNCHRO_ClearConditionVariable(&tempCondVar);
                                        P_UTILS_LIST_DeInitMT(&(*pHandle)->taskList);

                                        if ((*pHandle)->signalReadyToWait != NULL)
                                        {
                                            vQueueDelete((*pHandle)->signalReadyToWait);
                                            (*pHandle)->signalReadyToWait = NULL;
                                            DEBUG_decrementCpt();
                                        }
                                        if ((*pHandle)->signalReadyToStart != NULL)
                                        {
                                            vQueueDelete((*pHandle)->signalReadyToStart);
                                            (*pHandle)->signalReadyToStart = NULL;
                                            DEBUG_decrementCpt();
                                        }

                                        // Remove thread from global list
                                        P_UTILS_LIST_RemoveEltMT(&gTaskList,             //
                                                                 (*pHandle)->handleTask, //
                                                                 0,                      //
                                                                 0);                     //

                                        // Remove thread handle from local thread list exclusion
                                        P_UTILS_LIST_RemoveEltMT(&ptrCurrentThread->taskList, //
                                                                 (*pHandle)->handleTask,      //
                                                                 0,                           //
                                                                 0);                          //

                                        // Remove thread handle from other thread list exclusion
                                        wSlotId = UINT16_MAX;
                                        do
                                        {
                                            pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, //
                                                                                           &wSlotId);  //

                                            if ((pOthersThread != ptrCurrentThread) && (pOthersThread != NULL))
                                            {
                                                wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, //
                                                                                        (*pHandle)->handleTask,   //
                                                                                        0,                        //
                                                                                        0);                       //
                                                if (wSlotIdRes < UINT16_MAX)
                                                {
                                                    P_UTILS_LIST_RemoveEltMT(&pOthersThread->taskList, //
                                                                             (*pHandle)->handleTask,   //
                                                                             0,                        //
                                                                             0);                       //
                                                }
                                            }
                                        } while (wSlotId != UINT16_MAX);

                                        (*pHandle)->handleTask = NULL;

                                        if (tempLock != NULL)
                                        {
                                            vQueueDelete(tempLock);
                                            tempLock = NULL;
                                            DEBUG_decrementCpt();
                                        }

                                        /* Raz leaved memory */
                                        memset(*pHandle, 0, sizeof(tThreadWks));

                                        vPortFree(*pHandle);
                                        *pHandle = NULL;
                                        DEBUG_decrementCpt();
                                        result = E_THREAD_RESULT_OK;
                                    }
                                    else
                                    {
                                        result = E_THREAD_RESULT_ERROR_NOK;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Critical section release if not deleted after unlock and wait
            if (((*pHandle) != NULL) && ((*pHandle)->lockRecHandle != NULL))
            {
                xSemaphoreGiveRecursive((*pHandle)->lockRecHandle);
            }
        }
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

    if (thread != NULL)
    {
        P_THREAD_Init(thread, MAX_THREADS, (tPtrFct) startFct, startArgs, NULL, NULL);

        if (*thread != NULL)
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
