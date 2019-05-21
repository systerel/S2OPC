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

/*****Private condition variable api*****/

/*Add a waiting task to the waiting tasks list*/
static eConditionVariableResult PushSignal(tConditionVariable* pv,         // Condition variable workspace
                                           TaskHandle_t taskNotifiedValue, // Handle of the waiting task
                                           uint32_t uwWaitedSignal)        // Signal mask
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    uint16_t wCurrentSlotId = 0;
    uint16_t wNextSlotId = 0;

    if (pv != NULL)
    {
        if (pv->nbWaiters < pv->maxWaiters) // Continue only if at least one waiter free
        {
            // Search free slot position
            while (wCurrentSlotId < pv->maxWaiters)
            {
                if (pv->taskList[wCurrentSlotId].value == 0)
                {
                    break;
                }
                else
                {
                    wCurrentSlotId++;
                }
            }

            // If free slot found...
            if (wCurrentSlotId < pv->maxWaiters)
            {
                // Update previous slot id information in the current slot
                // and previous slot next slot id with current slot id
                // if the previous slot exist
                if (wCurrentSlotId > 0)
                {
                    pv->taskList[wCurrentSlotId - 1].nxId = wCurrentSlotId;
                    pv->taskList[wCurrentSlotId].prId = wCurrentSlotId - 1;
                }
                else
                {
                    pv->taskList[wCurrentSlotId].prId = pv->maxWaiters; // MAX_WAITERS = NO PREVIOUS
                    pv->first = wCurrentSlotId;
                }

                // Search next slot if exist (not the max and current slot < total already registered slots)
                if ((wCurrentSlotId < (pv->maxWaiters - 1)) && (wCurrentSlotId < pv->nbWaiters))
                {
                    wNextSlotId = wCurrentSlotId + 1;

                    // Search the not free slot
                    while (wNextSlotId < pv->maxWaiters)
                    {
                        if (pv->taskList[wNextSlotId].value != 0)
                        {
                            break;
                        }
                        else
                        {
                            wNextSlotId++;
                        }
                    }

                    // If a task handle has been found. (normally, it's the case,
                    // because wCurrentSlotId < pv->nbWaiters), update next slot
                    // with current id and current slot with this
                    if (wNextSlotId < pv->maxWaiters)
                    {
                        // Il indexe comme précédent le courant
                        // Le courant index le suivant
                        pv->taskList[wNextSlotId].prId = wCurrentSlotId;
                        pv->taskList[wCurrentSlotId].nxId = wNextSlotId;
                    }
                    else
                    {
                        // No next, next info set to MAX WAITERS
                        pv->taskList[wCurrentSlotId].nxId = pv->maxWaiters;
                    }
                }
                else
                {
                    // No next, next info set to MAX WAITERS
                    pv->taskList[wCurrentSlotId].nxId = pv->maxWaiters;
                }

                pv->taskList[wCurrentSlotId].value = taskNotifiedValue;
                pv->taskList[wCurrentSlotId].uwWaitedSig = uwWaitedSignal > 0 ? uwWaitedSignal : SIGNAL_VALUE;
                pv->nbWaiters = pv->nbWaiters < pv->maxWaiters ? pv->nbWaiters + 1 : pv->nbWaiters;
            } // if(wCurrentSlotId  < MAX_SIGNAL) no free slot
            else
            {
                result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
            }
        } // pv->nbWaiters = pv->nbWaiters < MAX_SIGNAL => no free slot
        else
        {
            result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
        }
    }
    return result;
}

/*Delete a task to notify from waiting tasks list*/
static void PopSignal(tConditionVariable* pv,    // Condition variable handle
                      TaskHandle_t taskNotified, // Task to remove
                      uint32_t uwWaitedSignal) // Signal filtered associated. Must be same as the one used by PushSignal
{
    uint16_t wCurrentSlotId = 0;
    uint32_t uwWaitedSig = uwWaitedSignal > 0 ? uwWaitedSignal : SIGNAL_VALUE;

    if ((pv->nbWaiters > 0) && (taskNotified > 0))
    {
        // Search a task with handle and signal requested
        while (wCurrentSlotId < pv->maxWaiters)
        {
            if ((pv->taskList[wCurrentSlotId].value == taskNotified) &&
                (pv->taskList[wCurrentSlotId].uwWaitedSig == uwWaitedSig))
            {
                break;
            }
            else
            {
                wCurrentSlotId = pv->taskList[wCurrentSlotId].nxId;
            }
        }
        // If found, -1 waiters, update list.
        if (wCurrentSlotId < pv->maxWaiters)
        {
            pv->nbWaiters = pv->nbWaiters > 0 ? pv->nbWaiters - 1 : pv->nbWaiters;

            if (wCurrentSlotId == pv->first)
            {
                pv->first = pv->taskList[wCurrentSlotId].nxId;
            }
            if (pv->taskList[wCurrentSlotId].nxId < pv->maxWaiters)
            {
                pv->taskList[pv->taskList[wCurrentSlotId].nxId].prId = pv->taskList[wCurrentSlotId].prId;
            }
            if (pv->taskList[wCurrentSlotId].prId < pv->maxWaiters)
            {
                pv->taskList[pv->taskList[wCurrentSlotId].prId].nxId = pv->taskList[wCurrentSlotId].nxId;
            }

            pv->taskList[wCurrentSlotId].value = 0;
            pv->taskList[wCurrentSlotId].uwWaitedSig = 0;
            pv->taskList[wCurrentSlotId].nxId = pv->maxWaiters;
            pv->taskList[wCurrentSlotId].prId = pv->maxWaiters;
        }
    }
}

/*Destroy condition variable*/
void DestroyConditionVariable(tConditionVariable** ppv)
{
    if ((ppv != NULL) && ((*ppv) != NULL))
    {
        // Destruction taskList
        if ((*ppv)->taskList != NULL)
        {
            vPortFree((*ppv)->taskList);
            (*ppv)->taskList = NULL;
        }

        // Destruction section critique
        if ((*ppv)->handleLockCounter != NULL)
        {
            vQueueDelete((*ppv)->handleLockCounter);
            (*ppv)->handleLockCounter = NULL;
        }

        // Destruction workspace
        vPortFree(*ppv);
        *ppv = NULL;
    }
}

/*Make condition variable*/
tConditionVariable* BuildConditionVariable(unsigned short int wMaxWaiters) // max parallel waiting tasks
{
    uint16_t iIter = 0;
    tConditionVariable* pv = NULL;

    // Nombre de thread en attente maximum
    if (wMaxWaiters > MAX_SIGNAL)
        goto error;

    // Allocation workspace
    pv = (tConditionVariable*) pvPortMalloc(sizeof(tConditionVariable));
    if (pv == NULL)
        goto error;
    (void) memset(pv, 0, sizeof(tConditionVariable));

    // Allocation liste de threads en attente signal
    pv->maxWaiters = wMaxWaiters;
    pv->taskList = (tEltTaskList*) pvPortMalloc(sizeof(tEltTaskList) * pv->maxWaiters);
    if (pv->taskList == NULL)
        goto error;

    // Index du premier élément. Si = maxWaiters, alors pas de thread.
    pv->first = pv->maxWaiters;
    // Init "chainage"
    for (iIter = 0; iIter < pv->maxWaiters; iIter++)
    {
        pv->taskList[iIter].nxId = pv->maxWaiters;
        pv->taskList[iIter].prId = pv->maxWaiters;
    }
    // Creation critical section
    pv->handleLockCounter = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
    if (pv->handleLockCounter == NULL)
        goto error;
    goto success;
error:
    DestroyConditionVariable(&pv);
success:
    return pv;
}

/*Broadcast signal to all waiting task on signal passed in parameters*/
eConditionVariableResult SignalAllConditionVariable(tConditionVariable* pv, // Condition variable workspace
                                                    uint32_t signalValue)   // Signal to broadcaset
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    uint16_t wCurrentSlotId = 0;
    uint32_t signal = signalValue > 0 ? signalValue : SIGNAL_VALUE;

    if ((pv != NULL) && (pv->handleLockCounter != NULL))
    {
        // Critical section
        xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY);
        {
            wCurrentSlotId = pv->first;
            if (pv->nbWaiters > 0)
            {
                // Tant que Nb de tache en attente ET operation de signalement < nombre de taches en attente
                while (wCurrentSlotId < pv->maxWaiters)
                {
                    if ((pv->taskList[wCurrentSlotId].value > 0) &&
                        (pv->taskList[wCurrentSlotId].uwWaitedSig == signal))
                    {
                        xTaskGenericNotify(pv->taskList[wCurrentSlotId].value, signal, eSetBits, NULL);
                        result = E_COND_VAR_RESULT_OK;
                    }
                    wCurrentSlotId = pv->taskList[wCurrentSlotId].nxId;
                }
            }
        }
        xSemaphoreGive(pv->handleLockCounter);
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    return result;
}

/*Unlock recursive mutex in parameters before wait a signal. On timeout, task is removed from task to notify.*/
eConditionVariableResult UnlockAndWaitForConditionVariable(tConditionVariable* pv, // Condition variable workspace
                                                           QueueHandle_t handleRecursiveMutex, // Recursive mutex
                                                           uint32_t uwSignal,                  // Signal to wait
                                                           uint32_t uwTimeOutMs)               // TimeOut
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    TickType_t xTimeToWait = 0;
    TaskHandle_t handleTask = 0;
    TimeOut_t xTimeOut = {0, 0};
    uint32_t notificationValue = 0;

    if ((pv != NULL) && (pv->handleLockCounter != NULL))
    {
        if (uwTimeOutMs == ULONG_MAX)
        {
            xTimeToWait = portMAX_DELAY;
        }
        else
        {
            xTimeToWait = pdMS_TO_TICKS(uwTimeOutMs);
        }

        // Critical section
        xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY);
        {
            // Récupération du handle de task et sauvegarde en pile
            if (pv->nbWaiters < pv->maxWaiters)
            {
                handleTask = xTaskGetCurrentTaskHandle();
                result = PushSignal(pv, handleTask, uwSignal);
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
            }
        }
        xSemaphoreGive(pv->handleLockCounter);

        // Libération du mutex passé en paramètre
        if (handleRecursiveMutex != NULL)
        {
            (void) xQueueGiveMutexRecursive(handleRecursiveMutex);
        }

        if (result == E_COND_VAR_RESULT_OK)
        {
            // Attente signalement ou Timeout
            // Dans les 2 cas, on depile le handle
            // (on indique à la tache qui signale un waiter de moins)

            for (;;)
            {
                vTaskSetTimeOutState(&xTimeOut); // RAZ timeout
                // Attente signal
                if (xTaskNotifyWait(0, uwSignal, &notificationValue, xTimeToWait) != pdPASS)
                {
                    result = E_COND_VAR_RESULT_TIMEOUT;
                    // Pas de notification reçue pendant le délai imparti
                    break;
                }
                else
                {
                    // Arrivee notification, check si la notif est celle attendu.
                    if ((notificationValue & uwSignal) != uwSignal)
                    {
                        // Si ce n'est pas le cas, update du temps restant à attendre la notification attendue
                        if (xTaskCheckForTimeOut(&xTimeOut, &xTimeToWait) == pdTRUE)
                        {
                            // Sinon timeout
                            result = E_COND_VAR_RESULT_TIMEOUT;
                            break;
                        }
                    }
                    else
                    {
                        // Notif attendue reçue
                        break;
                    }
                }
            }

            // Critical section
            xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY);
            {
                PopSignal(pv, handleTask, uwSignal);
            }
            xSemaphoreGive(pv->handleLockCounter);
        }

        // Prise du mutex passé en paramètre
        if (handleRecursiveMutex != NULL)
        {
            (void) xQueueTakeMutexRecursive(handleRecursiveMutex, portMAX_DELAY);
        }
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }

    return result;
}

/*****Public s2opc condition variable and mutex api*****/

/*Initialize a condition variable*/
SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tConditionVariable** pptr = (tConditionVariable**) cond;

    if ((pptr != NULL) && ((*pptr) == NULL))
    {
        *pptr = BuildConditionVariable(MAX_SIGNAL);
        if (*pptr == NULL)
        {
            *cond = NULL;
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            *cond = (Condition)(*pptr);
            result = SOPC_STATUS_OK;
        }
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return result;
}

/*Destroy a condition variable.*/
SOPC_ReturnStatus Condition_Clear(Condition* cond)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tConditionVariable** pptr = (tConditionVariable**) cond;
    if ((pptr != NULL) && ((*pptr) != NULL))
    {
        DestroyConditionVariable(pptr);
        *cond = NULL;
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return result;
}

SOPC_ReturnStatus Condition_SignalAll(Condition* cond)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    eConditionVariableResult presult = E_COND_VAR_RESULT_OK;
    tConditionVariable** pptr = (tConditionVariable**) cond;
    if ((pptr != NULL) && ((*pptr) != NULL))
    {
        presult = SignalAllConditionVariable(*pptr, SIGNAL_VALUE);
        if (presult != E_COND_VAR_RESULT_OK)
        {
            result = SOPC_STATUS_NOK;
        }
        else
        {
            result = SOPC_STATUS_OK;
        }
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return result;
}

// Must be called between lock and unlock of Mutex used to wait on condition
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    tConditionVariable** pptr = (tConditionVariable**) cond;
    QueueHandle_t handleRecursiveMutex = NULL;

    if ((pptr != NULL) && ((*pptr) != NULL))
    {
        if (mut != NULL)
        {
            handleRecursiveMutex = *((QueueHandle_t*) mut);
        }

        result = UnlockAndWaitForConditionVariable(*pptr, handleRecursiveMutex, SIGNAL_VALUE, ULONG_MAX);
        if (result == E_COND_VAR_RESULT_OK)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

// Must be called between lock and unlock of Mutex used to wait on condition
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    tConditionVariable** pptr = (tConditionVariable**) cond;
    QueueHandle_t handleRecursiveMutex = NULL;

    if ((pptr != NULL) && ((*pptr) != NULL))
    {
        if (mut != NULL)
        {
            handleRecursiveMutex = *((QueueHandle_t*) mut);
        }

        result = UnlockAndWaitForConditionVariable(*pptr, handleRecursiveMutex, SIGNAL_VALUE, milliSecs);
        if (result == E_COND_VAR_RESULT_OK)
        {
            status = SOPC_STATUS_OK;
        }
        else if (result == E_COND_VAR_RESULT_TIMEOUT)
        {
            status = SOPC_STATUS_TIMEOUT;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return status;
}

/*Create recursive mutex*/
SOPC_ReturnStatus Mutex_Initialization(Mutex* mut)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    QueueHandle_t handleRecursiveMutex = NULL;

    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        handleRecursiveMutex = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
        if (handleRecursiveMutex == NULL)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
            *mut = NULL;
        }
        else
        {
            result = SOPC_STATUS_OK;
            *mut = (Mutex) handleRecursiveMutex;
        }
    }
    return result;
}

/*Destroy recursive mutex*/
SOPC_ReturnStatus Mutex_Clear(Mutex* mut)
{
    QueueHandle_t handleRecursiveMutex = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        handleRecursiveMutex = *((QueueHandle_t*) (mut));
        if (handleRecursiveMutex == NULL)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            vQueueDelete(handleRecursiveMutex);
            result = SOPC_STATUS_OK;
        }
    }
    return result;
}

// Lock recursive mutex
SOPC_ReturnStatus Mutex_Lock(Mutex* mut)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    QueueHandle_t handleRecursiveMutex = NULL;
    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        handleRecursiveMutex = *((QueueHandle_t*) (mut));
        if (handleRecursiveMutex == NULL)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            if (xQueueTakeMutexRecursive(handleRecursiveMutex, portMAX_DELAY) == pdPASS)
            {
                result = SOPC_STATUS_OK;
            }
            else
            {
                result = SOPC_STATUS_NOK;
            }
        }
    }
    return result;
}

// Unlock recursive mutex
SOPC_ReturnStatus Mutex_Unlock(Mutex* mut)
{
    QueueHandle_t handleRecursiveMutex = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        handleRecursiveMutex = *((QueueHandle_t*) (mut));
        if (handleRecursiveMutex == NULL)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            if (xQueueGiveMutexRecursive(handleRecursiveMutex) == pdPASS)
            {
                result = SOPC_STATUS_OK;
            }
            else
            {
                result = SOPC_STATUS_NOK;
            }
        }
    }
    return result;
}
