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

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> /*stdlib*/
#include <string.h>

#include "sopc_enums.h" /*s2opc*/

#include "FreeRTOS.h" /*freeRtos*/
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_synchronisation.h" /*synchro*/
#include "p_threads.h"         /*thread*/
#include "p_utils.h"

#define MAX_THREADS 16

/*****Private structure definition*****/

typedef enum E_THREAD_STATUS
{
    E_THREAD_STATUS_NOT_INITIALIZED,
    E_THREAD_STATUS_INITIALIZED
} eThreadStatus;

typedef struct T_THREAD_ARGS
{
    tPtrFct cbExternalCallback;
    void* ptrStartArgs;
} tThreadArgs;

typedef struct T_THREAD_WKS
{
    eThreadStatus status;
    tUtilsList taskList;
    tPtrFct cbWaitingForJoin;
    tPtrFct cbReadyToSignal;
    TaskHandle_t handleTask;
    QueueHandle_t lockRecHandle;
    QueueHandle_t signalReadyToWait;
    QueueHandle_t signalReadyToStart;
    tConditionVariable* pJointure;
    tThreadArgs args;
} tThreadWks;

/*****Private global definition*****/

static tUtilsList gTaskList = {NULL, 0, 0, 0, NULL};
static unsigned int bOverflowDetected = 0;

/*****Private thread api*****/

// Callback encapsulate user callback. Abstract start and stop synchronisation.
static void cbInternalCallback(void* ptr)
{
    tThreadWks* ptrArgs = (tThreadWks*) ptr;
#ifndef WAIT_JOINTURE_READY_WITH_BIN_SEM
    unsigned int notificationValue = 0;
#endif

    if (ptr != NULL)
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

        // Wait for at least one join call
#ifndef WAIT_JOINTURE_READY_WITH_BIN_SEM
        while (1)
        {
            xTaskNotifyWait(0, JOINTURE_READY, &notificationValue, portMAX_DELAY);

            // If other notification

            if ((notificationValue & (~(JOINTURE_READY))) != 0)
            {
                xTaskGenericNotify(xTaskGetCurrentTaskHandle(), notificationValue & (~(JOINTURE_READY)), eSetBits,
                                   NULL);
            }

            // If expected notification
            if ((notificationValue & ((JOINTURE_READY))) == JOINTURE_READY)
            {
                break;
            }
        }
#else
        if (ptrArgs->signalReadyToWait != NULL)
        {
            xSemaphoreTake(ptrArgs->signalReadyToWait, portMAX_DELAY);
        }
#endif

        if (ptrArgs->cbReadyToSignal != NULL)
        {
            ptrArgs->cbReadyToSignal(ptrArgs->args.ptrStartArgs);
        }

        // At this level, wait for release mutex by condition variable called from join function
        if (ptrArgs->lockRecHandle != NULL)
        {
            xSemaphoreTakeRecursive(ptrArgs->lockRecHandle, portMAX_DELAY); //******Enter Critical section
            {
                // Signal terminaison thread
                P_SYNCHRO_SignalAllConditionVariable(ptrArgs->pJointure, JOINTURE_SIGNAL);
            }
            xSemaphoreGiveRecursive(ptrArgs->lockRecHandle); //******Leave Critical section
        }
    }

    vTaskDelete(NULL);
}

// Destruction handle
void P_THREAD_Destroy(tThreadWks** ptr)
{
    if ((ptr != NULL) && ((*ptr) != NULL))
    {
        if ((*ptr)->pJointure != NULL)
        {
            P_SYNCHRO_DestroyConditionVariable(&((*ptr)->pJointure));
        }

        P_UTILS_LIST_DeInit(&(*ptr)->taskList);

        if ((*ptr)->taskList.lockHandle != NULL)
        {
            vQueueDelete((*ptr)->taskList.lockHandle);
            (*ptr)->taskList.lockHandle = NULL;
        }

        if ((*ptr)->signalReadyToWait != NULL)
        {
            vQueueDelete((*ptr)->signalReadyToWait);
            (*ptr)->signalReadyToWait = NULL;
        }

        if ((*ptr)->signalReadyToStart != NULL)
        {
            vQueueDelete((*ptr)->signalReadyToStart);
            (*ptr)->signalReadyToStart = NULL;
        }

        if ((*ptr)->lockRecHandle != NULL)
        {
            vQueueDelete((*ptr)->lockRecHandle);
            (*ptr)->lockRecHandle = NULL;
        }

        vPortFree(*ptr);
        *ptr = NULL;
    }
}
// Creation workspace
tThreadWks* P_THREAD_Create(tPtrFct fct, void* args, tPtrFct fctWatingForJoin, tPtrFct fctReadyToSignal)
{
    tThreadWks* ptrWks = NULL;
    if (gTaskList.lockHandle == NULL)
    {
        if (P_UTILS_LIST_Init(&gTaskList, MAX_THREADS) != E_UTILS_LIST_RESULT_OK)
            goto error;
        gTaskList.lockHandle = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
        if (gTaskList.lockHandle == NULL)
            goto error;
    }

    ptrWks = (tThreadWks*) pvPortMalloc(sizeof(tThreadWks));

    if (ptrWks == NULL)
        goto error;
    (void) memset(ptrWks, 0, sizeof(tThreadWks));

    ptrWks->lockRecHandle = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
    if (ptrWks->lockRecHandle == NULL)
        goto error;

    ptrWks->args.cbExternalCallback = fct;
    ptrWks->cbReadyToSignal = fctReadyToSignal;
    ptrWks->cbWaitingForJoin = fctWatingForJoin;
    ptrWks->args.ptrStartArgs = args;
    ptrWks->handleTask = NULL;
    ptrWks->signalReadyToWait = NULL;
    ptrWks->signalReadyToStart = NULL;

    ptrWks->taskList.lockHandle = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
    if (ptrWks->taskList.lockHandle == NULL)
        goto error;

    ptrWks->pJointure = P_SYNCHRO_CreateConditionVariable();
    if (ptrWks->pJointure == NULL)
        goto error;

    goto success;
error:
    P_THREAD_Destroy(&ptrWks);
success:
    return ptrWks;
}

// Initializes created thread then launches it.
eThreadResult P_THREAD_Init(tThreadWks* p, unsigned short int wMaxRDV)
{
    eThreadResult resPTHR = E_THREAD_RESULT_ERROR_NOK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_ERROR_NOK;
    eUtilsListResult resTList = E_UTILS_LIST_RESULT_ERROR_NOK;

    if ((gTaskList.lockHandle != NULL) && (p != NULL) && (p->lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->handleTask == NULL)
            {
                // Creation task list a exclure
                resTList = P_UTILS_LIST_InitMT(&p->taskList, wMaxRDV);
                if (resTList == E_UTILS_LIST_RESULT_OK)
                {
                    resPSYNC = P_SYNCHRO_InitConditionVariable(p->pJointure, wMaxRDV);
                    if (resPSYNC == E_COND_VAR_RESULT_OK)
                    {
                        p->signalReadyToWait = xSemaphoreCreateBinary();
                        p->signalReadyToStart = xSemaphoreCreateBinary();
                        if ((p->signalReadyToWait != NULL) && (p->signalReadyToStart != NULL))
                        {
                            xSemaphoreTake(p->signalReadyToStart, 0);
                            xSemaphoreTake(p->signalReadyToWait, 0);
                            if (xTaskCreate(cbInternalCallback, "appThread", configMINIMAL_STACK_SIZE, p,
                                            configMAX_PRIORITIES - 1, &p->handleTask) != pdPASS)
                            {
                                resPTHR = E_THREAD_RESULT_ERROR_NOK;
                            }
                            else
                            {
                                p->status = E_THREAD_STATUS_INITIALIZED;
                                resTList = P_UTILS_LIST_AddEltMT(&gTaskList, p->handleTask, p, 0);
                                if (resTList != E_UTILS_LIST_RESULT_OK)
                                {
                                    resPTHR = E_THREAD_RESULT_ERROR_MAX_THREADS;
                                }
                                else
                                {
                                    resPTHR = E_THREAD_RESULT_OK;
                                }
                            }
                        }
                        else
                        {
                            resPTHR = E_THREAD_RESULT_ERROR_NOK;
                        }
                    }
                    else
                    {
                        resPTHR = E_THREAD_RESULT_ERROR_NOK;
                    }
                }
                else
                {
                    resPTHR = E_THREAD_RESULT_ERROR_NOK;
                }
            }
            else
            {
                resPTHR = E_THREAD_RESULT_ERROR_ALREADY_INITIALIZED;
            }

            if ((resPTHR != E_THREAD_RESULT_OK) && (resPTHR != E_THREAD_RESULT_ERROR_ALREADY_INITIALIZED))
            {
                if (p->handleTask != NULL)
                {
                    vTaskSuspend(p->handleTask);
                    vTaskDelete(p->handleTask);
                }
                p->handleTask = NULL;
                if (p->signalReadyToWait != NULL)
                {
                    vQueueDelete(p->signalReadyToWait);
                }
                if (p->signalReadyToStart != NULL)
                {
                    vQueueDelete(p->signalReadyToStart);
                }
                p->signalReadyToWait = NULL;
                p->signalReadyToStart = NULL;
                P_UTILS_LIST_DeInitMT(&p->taskList);
                P_SYNCHRO_ClearConditionVariable(p->pJointure);
            }
            else
            {
                if (resPTHR == E_THREAD_RESULT_OK)
                {
                    xSemaphoreGive(p->signalReadyToStart);
                }
            }
        }
        xSemaphoreGiveRecursive(p->lockRecHandle); //******Leave Critical section
    }

    return resPTHR;
}

// Joins thread. Thread joined becomes not initilized. Can be safely destroyed if just after
// P_THREAD_Join return, only if RESULT_OK
eThreadResult P_THREAD_Join(tThreadWks* p)
{
    eThreadResult result = E_THREAD_RESULT_ERROR_NOK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_ERROR_NOK;
    eUtilsListResult resTList = E_UTILS_LIST_RESULT_ERROR_NOK;
    tThreadWks* ptrCurrentThread = NULL;
    tThreadWks* pOthersThread = NULL;
    unsigned short int wSlotId = USHRT_MAX;
    unsigned short int wSlotIdRes = USHRT_MAX;
    TaskHandle_t handle = NULL;

    // Get current workspace from current task handle
    if ((gTaskList.lockHandle != NULL) && (p != NULL) && (p->lockRecHandle != NULL))
    {
        ptrCurrentThread = P_UTILS_LIST_GetContextFromHandleMT(&gTaskList, xTaskGetCurrentTaskHandle(), 0);
    }

    if ((ptrCurrentThread != NULL) && (ptrCurrentThread->lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->handleTask != NULL)
            {
                // Le thread en cours n'est pas celui que l'on cherche à joindre
                if (p->handleTask != xTaskGetCurrentTaskHandle())
                {
                    // Le thread en cours n'est pas exclu par le thread à joindre
                    wSlotId = P_UTILS_LIST_GetEltIndexMT(&p->taskList, xTaskGetCurrentTaskHandle(), 0);

                    if (wSlotId >= USHRT_MAX)
                    {
                        // Le thread à joindre n'a pas déjà été joint pas le thread à joindre
                        wSlotId = P_UTILS_LIST_GetEltIndexMT(&ptrCurrentThread->taskList, p->handleTask, 0);

                        if (wSlotId >= USHRT_MAX)
                        {
                            // La tache à joindre enregistre le handle courant du thread joingnant
                            resTList = P_UTILS_LIST_AddEltMT(&p->taskList, xTaskGetCurrentTaskHandle(), NULL, 0);

                            //---
                            if (resTList == E_UTILS_LIST_RESULT_OK)
                            {
                                // Parcours de l'ensemble des threads. Ceux qui référencent la tache à joindre
                                // enregistrent en plus le thread courant
                                wSlotId = USHRT_MAX;
                                do
                                {
                                    pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, &wSlotId);
                                    if ((pOthersThread != ptrCurrentThread) && (pOthersThread != NULL))
                                    {
                                        wSlotIdRes =
                                            P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, p->handleTask, 0);
                                        if (wSlotIdRes < USHRT_MAX)
                                        {
                                            resTList = P_UTILS_LIST_AddEltMT(&pOthersThread->taskList,
                                                                             xTaskGetCurrentTaskHandle(), NULL, 0);
                                        }
                                    }
                                } while ((wSlotId != USHRT_MAX) && (resTList == E_UTILS_LIST_RESULT_OK));

                                if (resTList != E_UTILS_LIST_RESULT_OK)
                                {
                                    // Error, restauration
                                    // Parcours de l'ensemble des threads. Ceux qui référencent la tache à joindre
                                    // n'enregistrent plus le thread courant
                                    wSlotId = USHRT_MAX;
                                    do
                                    {
                                        pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, &wSlotId);
                                        if ((pOthersThread != ptrCurrentThread) && (pOthersThread != NULL))
                                        {
                                            wSlotIdRes =
                                                P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList, p->handleTask, 0);
                                            if (wSlotIdRes < USHRT_MAX)
                                            {
                                                P_UTILS_LIST_RemoveEltMT(&pOthersThread->taskList,
                                                                         xTaskGetCurrentTaskHandle(), 0);
                                            }
                                        }
                                    } while (wSlotId != USHRT_MAX);

                                    result = E_THREAD_RESULT_ERROR_MAX_THREADS;
                                }
                                else
                                {
                                    // Le thread à joindre enregistre les handles des threads qui joignent le thread
                                    // courant
                                    wSlotId = USHRT_MAX;
                                    do
                                    {
                                        handle = (TaskHandle_t) P_UTILS_LIST_ParseValueEltMT(
                                            &ptrCurrentThread->taskList, &wSlotId);
                                        if (handle != NULL)
                                        {
                                            resTList = P_UTILS_LIST_AddEltMT(&p->taskList, handle, NULL, 0);
                                        }
                                    } while ((wSlotId != USHRT_MAX) && (resTList == E_UTILS_LIST_RESULT_OK));

                                    if (resTList != E_UTILS_LIST_RESULT_OK)
                                    {
                                        // Error, Le thread à joindre n'enregistre plus les handles des threads qui
                                        // joignent le thread courant
                                        wSlotId = USHRT_MAX;

                                        do
                                        {
                                            handle = (TaskHandle_t) P_UTILS_LIST_ParseValueEltMT(
                                                &ptrCurrentThread->taskList, &wSlotId);

                                            if (handle != NULL)
                                            {
                                                P_UTILS_LIST_RemoveEltMT(&p->taskList, handle, 0);
                                            }
                                        } while (wSlotId != USHRT_MAX);

                                        result = E_THREAD_RESULT_ERROR_MAX_THREADS;
                                    }
                                    else

                                    {
                                        // Indicate that a thread is ready to wait for join
#ifndef WAIT_JOINTURE_READY_WITH_BIN_SEM
                                        xTaskGenericNotify(p->args.handleTask, JOINTURE_READY, eSetBits, NULL);
#else
                                        xSemaphoreGive(p->signalReadyToWait);
#endif
                                        // The recursive mutex is taken. So, push handle to stack to notify
                                        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(
                                            p->pJointure, p->lockRecHandle, JOINTURE_SIGNAL, ULONG_MAX);

                                        if (resPSYNC == E_COND_VAR_RESULT_OK)
                                        {
                                            // Flag thread not initialized
                                            p->status = E_THREAD_STATUS_NOT_INITIALIZED;

                                            // After wait, clear condition variable. Unlock other join if necessary.
                                            P_SYNCHRO_ClearConditionVariable(p->pJointure);
                                            P_UTILS_LIST_DeInitMT(&p->taskList);
                                            if (p->signalReadyToWait != NULL)
                                            {
                                                vQueueDelete(p->signalReadyToWait);
                                                p->signalReadyToWait = NULL;
                                            }
                                            if (p->signalReadyToStart != NULL)
                                            {
                                                vQueueDelete(p->signalReadyToStart);
                                                p->signalReadyToStart = NULL;
                                            }

                                            // Remove thread from global list
                                            P_UTILS_LIST_RemoveEltMT(&gTaskList, p->handleTask, 0);

                                            // Remove thread handle from local thread list exclusion
                                            P_UTILS_LIST_RemoveEltMT(&ptrCurrentThread->taskList, p->handleTask, 0);

                                            // Remove thread handle from other thread list exclusion
                                            wSlotId = USHRT_MAX;
                                            do
                                            {
                                                pOthersThread = P_UTILS_LIST_ParseContextEltMT(&gTaskList, &wSlotId);
                                                if ((pOthersThread != ptrCurrentThread) && (pOthersThread != NULL))
                                                {
                                                    wSlotIdRes = P_UTILS_LIST_GetEltIndexMT(&pOthersThread->taskList,
                                                                                            p->handleTask, 0);
                                                    if (wSlotIdRes < USHRT_MAX)
                                                    {
                                                        P_UTILS_LIST_RemoveEltMT(&pOthersThread->taskList,
                                                                                 p->handleTask, 0);
                                                    }
                                                }
                                            } while (wSlotId != USHRT_MAX);

                                            p->handleTask = NULL;

                                            result = E_THREAD_RESULT_OK;
                                        }
                                        else
                                        {
                                            result = E_THREAD_RESULT_ERROR_NOK;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                result = E_THREAD_RESULT_ERROR_MAX_THREADS;
                            }
                        }
                        else
                        {
                            result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
                        }
                    }
                    else
                    {
                        result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
                    }
                }
                else
                {
                    result = E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD;
                }
            }
            else
            {
                result = E_THREAD_RESULT_ERROR_NOT_INITIALIZED;
            }
        }
        xSemaphoreGiveRecursive(p->lockRecHandle); //******Leave Critical section
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

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    eThreadResult result = E_THREAD_RESULT_ERROR_NOK;
    tThreadWks* ptr = NULL;

    if (thread != NULL)
    {
        ptr = P_THREAD_Create((tPtrFct) startFct, startArgs, NULL, NULL);
        if (ptr != NULL)
        {
            result = P_THREAD_Init(ptr, MAX_THREADS);
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
    }

    *thread = ptr;
    return status;
}

SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    tThreadWks* ptr = (tThreadWks*) thread;
    eThreadResult result = E_THREAD_RESULT_ERROR_NOK;

    if (ptr != NULL)
    {
        result = P_THREAD_Join(ptr);
        switch (result)
        {
        case E_THREAD_RESULT_OK:
            P_THREAD_Destroy(&ptr);
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

void SOPC_Sleep(unsigned int milliseconds)
{
    P_THREAD_Sleep(milliseconds);
}
