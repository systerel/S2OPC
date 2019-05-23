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

/*****Private thread api*****/

static eTaskListError AddToTaskList(tTaskList* ptr, TaskHandle_t handleTask) // Signal mask
{
    eTaskListError result = E_TASK_LIST_ERROR_OK;
    unsigned short wCurrentSlotId = 0;
    unsigned short wNextSlotId = 0;

    if ((ptr != NULL) && (handleTask != NULL) && (ptr->taskList != NULL))
    {
        if (ptr->wNbRegisteredTasks < ptr->wMaxWaitingTasks) // Continue only if at least one waiter free
        {
            // Search free slot position
            while (wCurrentSlotId < ptr->wMaxWaitingTasks)
            {
                if (ptr->taskList[wCurrentSlotId].value == 0)
                {
                    // Slot found
                    break;
                }
                else
                {
                    // Slot not found
                    wCurrentSlotId++;
                }
            }

            // If free slot found...
            if (wCurrentSlotId < ptr->wMaxWaitingTasks)
            {
                // Update previous slot id information in the current slot
                // and previous slot next slot id with current slot id
                // if the previous slot exist
                if (wCurrentSlotId > 0)
                {
                    ptr->taskList[wCurrentSlotId - 1].nxId = wCurrentSlotId;
                    ptr->taskList[wCurrentSlotId].prId = wCurrentSlotId - 1;
                }
                else
                {
                    ptr->taskList[wCurrentSlotId].prId = ptr->wMaxWaitingTasks; // MAX_WAITERS = NO PREVIOUS
                    ptr->first = wCurrentSlotId;
                }

                // Search next slot if exist (not the max and current slot < total already registered slots)
                if ((wCurrentSlotId < (ptr->wMaxWaitingTasks - 1)) && (wCurrentSlotId < ptr->wNbRegisteredTasks))
                {
                    wNextSlotId = wCurrentSlotId + 1;

                    // Search the not free slot
                    while (wNextSlotId < ptr->wMaxWaitingTasks)
                    {
                        if (ptr->taskList[wNextSlotId].value != 0)
                        {
                            // Slot found;
                            break;
                        }
                        else
                        {
                            // Slot not found
                            wNextSlotId++;
                        }
                    }

                    // If a task handle has been found. (normally, it's the case,
                    // because wCurrentSlotId < pv->nbWaiters), update next slot
                    // with current id and current slot with this
                    if (wNextSlotId < ptr->wMaxWaitingTasks)
                    {
                        // Il indexe comme précédent le courant
                        // Le courant index le suivant
                        ptr->taskList[wNextSlotId].prId = wCurrentSlotId;
                        ptr->taskList[wCurrentSlotId].nxId = wNextSlotId;
                    }
                    else
                    {
                        // No next, next info set to MAX WAITERS
                        ptr->taskList[wCurrentSlotId].nxId = ptr->wMaxWaitingTasks;
                    }
                }
                else
                {
                    // No next, next info set to MAX WAITERS
                    ptr->taskList[wCurrentSlotId].nxId = ptr->wMaxWaitingTasks;
                }

                ptr->taskList[wCurrentSlotId].value = handleTask;
                ptr->wNbRegisteredTasks = ptr->wNbRegisteredTasks < ptr->wMaxWaitingTasks ? ptr->wNbRegisteredTasks + 1
                                                                                          : ptr->wNbRegisteredTasks;
            } // if(wCurrentSlotId  < MAX_SIGNAL) no free slot
            else
            {
                result = E_TASK_LIST_ERROR_MAX_ELTS;
            }
        } // pv->nbWaiters = pv->nbWaiters < MAX_SIGNAL => no free slot
        else
        {
            result = E_TASK_LIST_ERROR_MAX_ELTS;
        }
    }
    return result;
}

static unsigned short int CheckIfTaskPresent(tTaskList* ptr,            // Condition variable handle
                                             TaskHandle_t taskNotified) // Task to remove
{
    unsigned short wCurrentSlotId = USHRT_MAX;
    if ((ptr != NULL) && (ptr->taskList != NULL) && (taskNotified != NULL))
    {
        wCurrentSlotId = ptr->first;
        while (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            if (ptr->taskList[wCurrentSlotId].value == taskNotified)
            {
                break;
            }
            else
            {
                wCurrentSlotId = ptr->taskList[wCurrentSlotId].nxId;
            }
        }
    }
    return wCurrentSlotId;
}

/*Alloc task*/
static eTaskListError InitTaskList(tTaskList* ptr, unsigned short int wMaxRDV)
{
    unsigned short iIter = 0;
    if ((ptr != NULL) && (wMaxRDV <= MAX_SIGNAL))
    {
        ptr->taskList = (tTaskListElt*) pvPortMalloc(sizeof(tTaskListElt) * wMaxRDV);
        if (ptr->taskList != NULL)
        {
            ptr->wMaxWaitingTasks = wMaxRDV;
            ptr->first = ptr->wMaxWaitingTasks;
            ptr->wNbRegisteredTasks = 0;
            for (iIter = 0; iIter < ptr->wMaxWaitingTasks; iIter++)
            {
                ptr->taskList[iIter].nxId = ptr->wMaxWaitingTasks;
                ptr->taskList[iIter].prId = ptr->wMaxWaitingTasks;
            }
            return E_TASK_LIST_ERROR_OK;
        }
    }
    return E_TASK_LIST_ERROR_NOK;
}

static void DeInitTaskList(tTaskList* ptr)
{
    if (ptr != NULL)
    {
        if (ptr->taskList != NULL)
        {
            vPortFree(ptr->taskList);
            ptr->taskList = NULL;
            ptr->wMaxWaitingTasks = 0;
            ptr->first = 0;
            ptr->wNbRegisteredTasks = 0;
        }
    }
}

static void cbInternalCallback(void* ptr)
{
    tThreadArgs* ptrArgs = (tThreadArgs*) ptr;

    if (ptr != NULL)
    {
        if (ptrArgs->cbExternalCallback != NULL)
        {
            ptrArgs->cbExternalCallback(ptrArgs->ptrStartArgs);
        }

        // Wait for at least one join call
        // xTaskNotifyWait(0,JOINTURE_READY,NULL,portMAX_DELAY);

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

        if ((*ptr)->taskList.taskList != NULL)
        {
            vPortFree((*ptr)->taskList.taskList);
            (*ptr)->taskList.taskList = NULL;
            (void) memset(&(*ptr)->taskList, 0, sizeof(tTaskList));
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
tThreadWks* P_THREAD_Create(ptrTaskCallback fct, void* args)
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
    eTaskListError resTList = E_TASK_LIST_ERROR_OK;

    if ((p != NULL) && (p->args.lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->args.lockRecHandle, portMAX_DELAY); //******Critical section
        {
            if (p->args.handleTask == NULL)
            {
                // Creation task list a exclure
                resTList = InitTaskList(&p->taskList, wMaxRDV);
                if (resTList == E_TASK_LIST_ERROR_OK)
                {
                    resPSYNC = P_SYNCHRO_InitConditionVariable(p->args.pJointure, wMaxRDV);
                    if (resPSYNC == E_COND_VAR_RESULT_OK)
                    {
                        if (xTaskCreate(cbInternalCallback, "appThread", configMINIMAL_STACK_SIZE, &p->args,
                                        configMAX_PRIORITIES - 1, &p->args.handleTask) != pdPASS)
                        {
                            p->args.handleTask = NULL;
                            P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
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
                if (p->args.handleTask != xTaskGetCurrentTaskHandle())
                {
                    if (CheckIfTaskPresent(&p->taskList, xTaskGetCurrentTaskHandle()) < p->taskList.wMaxWaitingTasks)
                    {
                        AddToTaskList(&p->taskList, xTaskGetCurrentTaskHandle());
                        // Indicate that a thread is ready to wait for join
                        // xTaskGenericNotify(p->args.handleTask,JOINTURE_READY,eSetBits,NULL);

                        // The recursive mutex is taken. So, push handle to stack to notify
                        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(p->args.pJointure, p->args.lockRecHandle,
                                                                               JOINTURE_SIGNAL, ULONG_MAX);

                        // After wait, clear condition variable. Unlock other join if necessary.
                        (void) P_SYNCHRO_ClearConditionVariable(p->args.pJointure);

                        DeInitTaskList(&p->taskList);

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
