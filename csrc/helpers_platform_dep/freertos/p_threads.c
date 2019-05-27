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

#define MAX_THREADS 128

static tUtilsList gTaskList = {NULL, 0, 0, 0, NULL};
static unsigned int bOverflowDetected = 0;

/*****Private thread api*****/

static void cbInternalCallback(void* ptr)
{
    tThreadArgs* ptrArgs = (tThreadArgs*) ptr;
#ifndef WAIT_JOINTURE_READY_WITH_BIN_SEM
    unsigned int notificationValue = 0;
#endif

    if (ptr != NULL)
    {
        xSemaphoreTake(ptrArgs->signalReadyToStart, portMAX_DELAY);

        if (ptrArgs->cbExternalCallback != NULL)
        {
            ptrArgs->cbExternalCallback(ptrArgs->ptrStartArgs);
        }

        if (ptrArgs->cbWaitingForJoin != NULL)
        {
            ptrArgs->cbWaitingForJoin(ptrArgs->ptrStartArgs);
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
        xSemaphoreTake(ptrArgs->signalReadyToWait, portMAX_DELAY);
#endif

        if (ptrArgs->cbReadyToSignal != NULL)
        {
            ptrArgs->cbReadyToSignal(ptrArgs->ptrStartArgs);
        }

        // At this level, wait for release mutex by condition variable called from join function
        xSemaphoreTakeRecursive(ptrArgs->lockRecHandle, portMAX_DELAY); //******Enter Critical section
        {
            // Signal terminaison thread
            P_SYNCHRO_SignalAllConditionVariable(ptrArgs->pJointure, JOINTURE_SIGNAL);
        }
        xSemaphoreGiveRecursive(ptrArgs->lockRecHandle); //******Leave Critical section
    }

    vTaskDelete(NULL);
}

// Non thread safe
void P_THREAD_Destroy(tThreadWks** ptr)
{
    if ((ptr != NULL) && ((*ptr) != NULL))
    {
        if ((*ptr)->args.pJointure != NULL)
        {
            P_SYNCHRO_DestroyConditionVariable(&((*ptr)->args.pJointure));
        }

        P_UTILS_LIST_DeInit(&(*ptr)->taskList);

        if ((*ptr)->args.signalReadyToWait != NULL)
        {
            vQueueDelete((*ptr)->args.signalReadyToWait);
            (*ptr)->args.signalReadyToWait = NULL;
        }

        if ((*ptr)->args.signalReadyToStart != NULL)
        {
            vQueueDelete((*ptr)->args.signalReadyToStart);
            (*ptr)->args.signalReadyToStart = NULL;
        }

        if ((*ptr)->args.lockRecHandle != NULL)
        {
            vQueueDelete((*ptr)->args.lockRecHandle);
            (*ptr)->args.lockRecHandle = NULL;
        }

        vPortFree(*ptr);
        *ptr = NULL;
    }
}
// Non thread safe
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

    ptrWks->args.lockRecHandle = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
    if (ptrWks->args.lockRecHandle == NULL)
        goto error;

    ptrWks->args.cbExternalCallback = fct;
    ptrWks->args.cbReadyToSignal = fctReadyToSignal;
    ptrWks->args.cbWaitingForJoin = fctWatingForJoin;
    ptrWks->args.ptrStartArgs = args;
    ptrWks->args.handleTask = 0;
    ptrWks->args.signalReadyToWait = NULL;
    ptrWks->args.signalReadyToStart = NULL;

    ptrWks->args.pJointure = P_SYNCHRO_CreateConditionVariable();
    if (ptrWks->args.pJointure == NULL)
        goto error;

    goto success;
error:
    P_THREAD_Destroy(&ptrWks);
success:
    return ptrWks;
}

eThreadResult P_THREAD_Init(tThreadWks* p, unsigned short int wMaxRDV)
{
    eThreadResult resPTHR = E_THREAD_RESULT_ERROR_NOK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_ERROR_NOK;
    eUtilsListResult resTList = E_UTILS_LIST_RESULT_ERROR_NOK;

    if ((gTaskList.lockHandle != NULL) && (p != NULL) && (p->args.lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->args.lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->args.handleTask == NULL)
            {
                // Creation task list a exclure
                resTList = P_UTILS_LIST_Init(&p->taskList, wMaxRDV);
                if (resTList == E_UTILS_LIST_RESULT_OK)
                {
                    resPSYNC = P_SYNCHRO_InitConditionVariable(p->args.pJointure, wMaxRDV);
                    if (resPSYNC == E_COND_VAR_RESULT_OK)
                    {
                        p->args.signalReadyToWait = xSemaphoreCreateBinary();
                        p->args.signalReadyToStart = xSemaphoreCreateBinary();
                        if ((p->args.signalReadyToWait != NULL) && (p->args.signalReadyToStart != NULL))
                        {
                            xSemaphoreTake(p->args.signalReadyToStart, 0);
                            xSemaphoreTake(p->args.signalReadyToWait, 0);
                            if (xTaskCreate(cbInternalCallback, "appThread", configMINIMAL_STACK_SIZE, &p->args,
                                            configMAX_PRIORITIES - 1, &p->args.handleTask) != pdPASS)
                            {
                                resPTHR = E_THREAD_RESULT_ERROR_NOK;
                            }
                            else
                            {
                                resTList = P_UTILS_LIST_AddEltMT(&gTaskList, p->args.handleTask, p, 0);
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
                if (p->args.handleTask != NULL)
                {
                    vTaskSuspend(p->args.handleTask);
                    vTaskDelete(p->args.handleTask);
                }
                p->args.handleTask = NULL;
                if (p->args.signalReadyToWait != NULL)
                {
                    vQueueDelete(p->args.signalReadyToWait);
                }
                if (p->args.signalReadyToStart != NULL)
                {
                    vQueueDelete(p->args.signalReadyToStart);
                }
                p->args.signalReadyToWait = NULL;
                p->args.signalReadyToStart = NULL;
                P_UTILS_LIST_DeInit(&p->taskList);
                P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
            }
            else
            {
                if (resPTHR == E_THREAD_RESULT_OK)
                {
                    xSemaphoreGive(p->args.signalReadyToStart);
                }
            }
        }
        xSemaphoreGiveRecursive(p->args.lockRecHandle); //******Leave Critical section
    }

    return resPTHR;
}

eThreadResult P_THREAD_Join(tThreadWks* p)
{
    eThreadResult result = E_THREAD_RESULT_ERROR_NOK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_ERROR_NOK;
    eUtilsListResult resTList = E_UTILS_LIST_RESULT_ERROR_NOK;
    tThreadWks* ptrCurrentThread = NULL;

    // Get current workspace from current task handle
    if ((gTaskList.lockHandle != NULL) && (p != NULL) && (p->args.lockRecHandle != NULL))
    {
        ptrCurrentThread = P_UTILS_LIST_GetContextFromHandleMT(&gTaskList, xTaskGetCurrentTaskHandle(), 0);
    }

    if (ptrCurrentThread != NULL)
    {
        xSemaphoreTakeRecursive(p->args.lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->args.handleTask != NULL)
            {
                // La tache en cours n'est pas celle sur laquelle on fait le join
                if (p->args.handleTask != xTaskGetCurrentTaskHandle())
                {
                    // La tache en cours n'est pas exclue par le thread à joindre
                    if (P_UTILS_LIST_GetEltIndex(&p->taskList, xTaskGetCurrentTaskHandle(), 0) >=
                        p->taskList.wMaxWaitingTasks)
                    {
                        // Ajout join indirect

                        // Le thread en cours ajoute à sa task list d'exclusion le handle de la task à joindre
                        resTList =
                            P_UTILS_LIST_AddElt(&ptrCurrentThread->taskList, xTaskGetCurrentTaskHandle(), NULL, 0);

                        if (resTList == E_UTILS_LIST_RESULT_OK)
                        {
                            // Indicate that a thread is ready to wait for join
#ifndef WAIT_JOINTURE_READY_WITH_BIN_SEM
                            xTaskGenericNotify(p->args.handleTask, JOINTURE_READY, eSetBits, NULL);
#else
                            xSemaphoreGive(p->args.signalReadyToWait);
#endif
                            // The recursive mutex is taken. So, push handle to stack to notify
                            resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(
                                p->args.pJointure, p->args.lockRecHandle, JOINTURE_SIGNAL, ULONG_MAX);

                            // After wait, clear condition variable. Unlock other join if necessary.
                            P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
                            P_UTILS_LIST_DeInit(&p->taskList);
                            if (p->args.signalReadyToWait != NULL)
                            {
                                vQueueDelete(p->args.signalReadyToWait);
                                p->args.signalReadyToWait = NULL;
                            }
                            if (p->args.signalReadyToStart != NULL)
                            {
                                vQueueDelete(p->args.signalReadyToStart);
                                p->args.signalReadyToStart = NULL;
                            }

                            // Remove thread from global list
                            P_UTILS_LIST_RemoveEltMT(&gTaskList, p->args.handleTask, 0);
                            p->args.handleTask = NULL;

                            switch (resPSYNC) // resPSYNC is not E_COND_VAR_RESULT_OK if a clear has been performed
                            {
                            case E_COND_VAR_RESULT_OK:
                                result = E_THREAD_RESULT_OK;
                                break;
                            default:
                                result = E_THREAD_RESULT_ERROR_NOK;
                                break;
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
                result = E_THREAD_RESULT_ERROR_NOT_INITIALIZED;
            }
        }
        xSemaphoreGiveRecursive(p->args.lockRecHandle); //******Leave Critical section
    }

    return result;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    bOverflowDetected = 0xAAAAAAAA;
}

/*****Public s2opc thread api*****/

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    return status;
}

SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    return status;
}

void SOPC_Sleep(unsigned int milliseconds) {}
