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

static unsigned int bOverflowDetected = 0;

/*****Private thread api*****/

static void cbInternalCallback(void* ptr)
{
    tThreadArgs* ptrArgs = (tThreadArgs*) ptr;
    unsigned int notificationValue = 0;

    if (ptr != NULL)
    {
        if (ptrArgs->cbExternalCallback != NULL)
        {
            ptrArgs->cbExternalCallback(ptrArgs->ptrStartArgs);
        }

        if (ptrArgs->cbWaitingForJoin != NULL)
        {
            ptrArgs->cbWaitingForJoin(ptrArgs->ptrStartArgs);
        }

        // Wait for at least one join call
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

    ptrWks = (tThreadWks*) pvPortMalloc(sizeof(tThreadWks));

    if (ptrWks == NULL)
        goto error;
    (void) memset(ptrWks, 0, sizeof(tThreadWks));

    ptrWks->args.lockRecHandle = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
    if (ptrWks->args.lockRecHandle == NULL)
        goto error;

    ptrWks->args.cbExternalCallback = fct;
    ptrWks->args.cbReadyToSignal = fctReadyToSignal;
    ptrWks->args.cbWaitingForJoin = fctWatingForJoin;
    ptrWks->args.ptrStartArgs = args;
    ptrWks->args.handleTask = 0;
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
    eThreadResult resPTHR = E_THREAD_RESULT_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    eUtilsListError resTList = E_UTILS_LIST_ERROR_OK;

    if ((p != NULL) && (p->args.lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->args.lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->args.handleTask == NULL)
            {
                // Creation task list a exclure
                resTList = P_UTILS_LIST_Init(&p->taskList, wMaxRDV);
                if (resTList == E_UTILS_LIST_ERROR_OK)
                {
                    resPSYNC = P_SYNCHRO_InitConditionVariable(p->args.pJointure, wMaxRDV);
                    if (resPSYNC == E_COND_VAR_RESULT_OK)
                    {
                        p->args.signalReadyToWait = xSemaphoreCreateBinary();
                        if (p->args.signalReadyToWait != NULL)
                        {
                            if (xTaskCreate(cbInternalCallback, "appThread", configMINIMAL_STACK_SIZE, &p->args,
                                            configMAX_PRIORITIES - 1, &p->args.handleTask) != pdPASS)
                            {
                                p->args.handleTask = NULL;
                                vQueueDelete(p->args.signalReadyToWait);
                                p->args.signalReadyToWait = NULL;
                                P_UTILS_LIST_DeInit(&p->taskList);
                                P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
                                resPTHR = E_THREAD_RESULT_ERROR_NOK;
                            }
                        }
                        else
                        {
                            P_UTILS_LIST_DeInit(&p->taskList);
                            P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
                            resPTHR = E_THREAD_RESULT_ERROR_NOK;
                        }
                    }
                    else
                    {
                        P_UTILS_LIST_DeInit(&p->taskList);
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
        }
        xSemaphoreGiveRecursive(p->args.lockRecHandle); //******Leave Critical section
    }

    return resPTHR;
}

eThreadResult P_THREAD_Join(tThreadWks* p)
{
    eThreadResult result = E_THREAD_RESULT_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;

    if ((p != NULL) && (p->args.lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->args.lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->args.handleTask != NULL)
            {
                // La tache en cours n'est pas celle sur laquelle on fait le join
                if (p->args.handleTask != xTaskGetCurrentTaskHandle())
                {
                    // La tache en cours n'est pas exclue par le thread à joindre
                    if (P_UTILS_LIST_GetEltIndex(&p->taskList, xTaskGetCurrentTaskHandle(), 0) ==
                        p->taskList.wMaxWaitingTasks)
                    {
                        // Le thread en cours est ajouté au handle du thread à joindre
                        P_UTILS_LIST_AddElt(&p->taskList, xTaskGetCurrentTaskHandle(), 0);
                        // Indicate that a thread is ready to wait for join
                        xTaskGenericNotify(p->args.handleTask, JOINTURE_READY, eSetBits, NULL);

                        // The recursive mutex is taken. So, push handle to stack to notify
                        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(p->args.pJointure, p->args.lockRecHandle,
                                                                               JOINTURE_SIGNAL, ULONG_MAX);

                        // After wait, clear condition variable. Unlock other join if necessary.
                        P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
                        P_UTILS_LIST_DeInit(&p->taskList);
                        if (p->args.signalReadyToWait != NULL)
                        {
                            vQueueDelete(p->args.signalReadyToWait);
                            p->args.signalReadyToWait = NULL;
                        }
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
