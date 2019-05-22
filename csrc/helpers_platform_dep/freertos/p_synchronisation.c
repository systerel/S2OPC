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

typedef enum E_CONDITION_VARIABLE_STATUS
{
    E_CONDITION_VARIABLE_STATUS_NOT_INITIALIZED,
    E_CONDITION_VARIABLE_STATUS_INITIALIZED
} eConditionVariableStatus;

typedef struct T_ELT_TASK_LIST
{
    TaskHandle_t value;
    unsigned int uwWaitedSig;
    unsigned short nxId;
    unsigned short prId;
} tEltTaskList;

typedef struct T_CONDITION_VARIABLE
{
    QueueHandle_t handleLockCounter;
    eConditionVariableStatus wStatus;
    unsigned short wIndexFirstTask;
    unsigned short wNbRegisteredTasks;
    unsigned short wMaxWaitingTasks;
    tEltTaskList* taskList;
} tConditionVariable;

/*Add a waiting task to the waiting tasks list*/
static eConditionVariableResult RegisterWaitingTask(tConditionVariable* pv,         // Condition variable workspace
                                                    TaskHandle_t taskNotifiedValue, // Handle of the waiting task
                                                    unsigned int uwWaitedSignal)    // Signal mask
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    unsigned short wCurrentSlotId = 0;
    unsigned short wNextSlotId = 0;
    unsigned int wSignalToRegister = uwWaitedSignal > 0 ? uwWaitedSignal : SIGNAL_VALUE;
    wSignalToRegister &= ~CLEARING_SIGNAL;

    if (pv != NULL)
    {
        if (pv->wNbRegisteredTasks < pv->wMaxWaitingTasks) // Continue only if at least one waiter free
        {
            // Search free slot position
            while (wCurrentSlotId < pv->wMaxWaitingTasks)
            {
                if (pv->taskList[wCurrentSlotId].value == 0)
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
            if (wCurrentSlotId < pv->wMaxWaitingTasks)
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
                    pv->taskList[wCurrentSlotId].prId = pv->wMaxWaitingTasks; // MAX_WAITERS = NO PREVIOUS
                    pv->wIndexFirstTask = wCurrentSlotId;
                }

                // Search next slot if exist (not the max and current slot < total already registered slots)
                if ((wCurrentSlotId < (pv->wMaxWaitingTasks - 1)) && (wCurrentSlotId < pv->wNbRegisteredTasks))
                {
                    wNextSlotId = wCurrentSlotId + 1;

                    // Search the not free slot
                    while (wNextSlotId < pv->wMaxWaitingTasks)
                    {
                        if (pv->taskList[wNextSlotId].value != 0)
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
                    if (wNextSlotId < pv->wMaxWaitingTasks)
                    {
                        // Il indexe comme précédent le courant
                        // Le courant index le suivant
                        pv->taskList[wNextSlotId].prId = wCurrentSlotId;
                        pv->taskList[wCurrentSlotId].nxId = wNextSlotId;
                    }
                    else
                    {
                        // No next, next info set to MAX WAITERS
                        pv->taskList[wCurrentSlotId].nxId = pv->wMaxWaitingTasks;
                    }
                }
                else
                {
                    // No next, next info set to MAX WAITERS
                    pv->taskList[wCurrentSlotId].nxId = pv->wMaxWaitingTasks;
                }

                pv->taskList[wCurrentSlotId].value = taskNotifiedValue;
                pv->taskList[wCurrentSlotId].uwWaitedSig = wSignalToRegister;
                pv->wNbRegisteredTasks =
                    pv->wNbRegisteredTasks < pv->wMaxWaitingTasks ? pv->wNbRegisteredTasks + 1 : pv->wNbRegisteredTasks;
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

/*Delete a task to notify from waiting tasks list - not thread safe*/
static void RemoveWaitingTask(
    tConditionVariable* pv,      // Condition variable handle
    TaskHandle_t taskNotified,   // Task to remove
    unsigned int uwWaitedSignal) // Signal filtered associated. Must be same as the one used by RegisterWaitingTask
{
    unsigned short wCurrentSlotId = 0;
    unsigned int uwWaitedSig = uwWaitedSignal > 0 ? uwWaitedSignal : SIGNAL_VALUE;

    if ((pv->wNbRegisteredTasks > 0) && (taskNotified > 0))
    {
        // Search a task with handle and signal requested
        while (wCurrentSlotId < pv->wMaxWaitingTasks)
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
        if (wCurrentSlotId < pv->wMaxWaitingTasks)
        {
            // Safe decrement
            pv->wNbRegisteredTasks = pv->wNbRegisteredTasks > 0 ? pv->wNbRegisteredTasks - 1 : pv->wNbRegisteredTasks;

            // First slot removed, so index first on next
            if (wCurrentSlotId == pv->wIndexFirstTask)
            {
                pv->wIndexFirstTask = pv->taskList[wCurrentSlotId].nxId;
            }
            // Next of current not the last, so previous of next = previous of current
            if (pv->taskList[wCurrentSlotId].nxId < pv->wMaxWaitingTasks)
            {
                pv->taskList[pv->taskList[wCurrentSlotId].nxId].prId = pv->taskList[wCurrentSlotId].prId;
            }
            // previous of current not the last, so next of previous = next of current
            if (pv->taskList[wCurrentSlotId].prId < pv->wMaxWaitingTasks)
            {
                pv->taskList[pv->taskList[wCurrentSlotId].prId].nxId = pv->taskList[wCurrentSlotId].nxId;
            }

            // RAZ current
            pv->taskList[wCurrentSlotId].value = 0;
            pv->taskList[wCurrentSlotId].uwWaitedSig = 0;
            pv->taskList[wCurrentSlotId].nxId = pv->wMaxWaitingTasks;
            pv->taskList[wCurrentSlotId].prId = pv->wMaxWaitingTasks;
        }
    }
}

// Internal deinit condition variable, not thread safe.
static void ClearConditionVariable(tConditionVariable* pv)
{
    if (pv != NULL)
    {
        // Destruction taskList
        if (pv->taskList != NULL)
        {
            vPortFree(pv->taskList);
            pv->taskList = NULL;
        }

        pv->wIndexFirstTask = 0;
        pv->wMaxWaitingTasks = 0;
        pv->wNbRegisteredTasks = 0;
        pv->wStatus = E_CONDITION_VARIABLE_STATUS_NOT_INITIALIZED; // Indique que le workspace n'est plus initialisé
    }
}

/*Deinit condition variable*/
eConditionVariableResult P_SYNCHRO_ClearConditionVariable(tConditionVariable* pv)
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    unsigned short int wCurrentSlotId = 0;
    if ((pv != NULL) && (pv->handleLockCounter != NULL))
    {
        xSemaphoreTake(pv->handleLockCounter, portMAX_DELAY); // Section critique
        {
            if (pv->wStatus == E_CONDITION_VARIABLE_STATUS_INITIALIZED)
            {
                // Indicate for each registered task that clearing signal is pending
                wCurrentSlotId = pv->wIndexFirstTask; // Index first slot to parse
                if (pv->wNbRegisteredTasks > 0)       // Indicate if waiters are waiting
                {
                    while (wCurrentSlotId < pv->wMaxWaitingTasks)
                    {
                        if ((pv->taskList[wCurrentSlotId].value > 0))
                        {
                            xTaskGenericNotify(pv->taskList[wCurrentSlotId].value, CLEARING_SIGNAL, eSetBits, NULL);
                        }
                        // Go to next slot
                        wCurrentSlotId = pv->taskList[wCurrentSlotId].nxId;
                    }
                }
                // Set condition variable to not initialized state
                ClearConditionVariable(pv);
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
            }
        }
        xSemaphoreGive(pv->handleLockCounter); // Fin de section critique avant return
    }
    return result;
}

/*Make condition variable*/
eConditionVariableResult P_SYNCHRO_InitConditionVariable(tConditionVariable* pv,
                                                         unsigned short int wMaxWaiters) // max parallel waiting tasks
{
    unsigned short wIterTaskSlot = 0;
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;

    // Nombre de thread en attente maximum
    if ((wMaxWaiters > MAX_SIGNAL) || (pv == NULL) || (pv->handleLockCounter == NULL))
        goto error_parameter;

    xSemaphoreTake(pv->handleLockCounter, portMAX_DELAY); //********Section critique

    if (pv->wStatus == E_CONDITION_VARIABLE_STATUS_INITIALIZED)
        goto error_status;

    // Allocation liste de threads en attente signal
    pv->wMaxWaitingTasks = wMaxWaiters;
    pv->taskList = (tEltTaskList*) pvPortMalloc(sizeof(tEltTaskList) * pv->wMaxWaitingTasks);
    if (pv->taskList == NULL)
        goto error_init;

    // Index du premier élément. Si = maxWaiters, alors pas de thread.
    pv->wIndexFirstTask = pv->wMaxWaitingTasks;

    // Init "chainage"
    for (wIterTaskSlot = 0; wIterTaskSlot < pv->wMaxWaitingTasks; wIterTaskSlot++)
    {
        pv->taskList[wIterTaskSlot].value = 0;
        pv->taskList[wIterTaskSlot].uwWaitedSig = 0;
        pv->taskList[wIterTaskSlot].nxId = pv->wMaxWaitingTasks;
        pv->taskList[wIterTaskSlot].prId = pv->wMaxWaitingTasks;
    }

    // Flag condition var initialisée
    pv->wStatus = E_CONDITION_VARIABLE_STATUS_INITIALIZED;

    goto success;

error_parameter:
    result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    return result;
error_init:
    ClearConditionVariable(pv);
    result = E_COND_VAR_RESULT_ERROR_OUT_OF_MEM;
    xSemaphoreGive(pv->handleLockCounter); //*******Fin de section critique avant return
    return result;
error_status:
    result = E_COND_VAR_RESULT_ERROR_ALREADY_INITIALIZED;
    xSemaphoreGive(pv->handleLockCounter); //*******Fin de section critique avant return
    return result;
success:
    xSemaphoreGive(pv->handleLockCounter); //*******Fin de section critique avant return
    result = E_COND_VAR_RESULT_OK;
    return result;
}

// Destruction workspace. Not thread safe. P_SYNCHRO_ClearCondition variable doit être appelée avant celle-ci.
void P_SYNCHRO_DestroyConditionVariable(tConditionVariable** pv)
{
    if ((pv != NULL) && (*pv != NULL))
    {
        // Destruction section critique
        if ((*pv)->handleLockCounter != NULL)
        {
            vQueueDelete((*pv)->handleLockCounter);
            (*pv)->handleLockCounter = NULL;
        }
        vPortFree(*pv);
        *pv = NULL;
    }
}

// Creation workspace. Not thread safe. P_SYNCHRO_InitCondition variable doit être appelée just après celle-ci.
tConditionVariable* P_SYNCHRO_CreateConditionVariable(void)
{
    tConditionVariable* pv = NULL;

    // Allocation workspace
    pv = (tConditionVariable*) pvPortMalloc(sizeof(tConditionVariable));
    if (pv == NULL)
        goto error;

    // Raz memory
    (void) memset(pv, 0, sizeof(tConditionVariable));

    // Creation critical section
    pv->handleLockCounter = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
    if (pv->handleLockCounter == NULL)
        goto error;

    goto success;

error:
    P_SYNCHRO_DestroyConditionVariable(&pv);
success:
    return pv;
}

/*Broadcast signal to all waiting task on signal passed in parameters*/
eConditionVariableResult P_SYNCHRO_SignalAllConditionVariable(tConditionVariable* pv,   // Condition variable workspace
                                                              unsigned int signalValue) // Signal to broadcaset
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    unsigned short wCurrentSlotId = 0;
    unsigned int signal = signalValue > 0 ? signalValue : SIGNAL_VALUE;

    if ((pv != NULL) && (pv->handleLockCounter != NULL))
    {
        xQueueSemaphoreTake(pv->handleLockCounter, portMAX_DELAY); // Critical section
        {
            if (pv->wStatus == E_CONDITION_VARIABLE_STATUS_INITIALIZED)
            {
                wCurrentSlotId = pv->wIndexFirstTask;
                if (pv->wNbRegisteredTasks > 0)
                {
                    // Tant que Nb de tache en attente ET operation de signalement < nombre de taches en attente
                    while (wCurrentSlotId < pv->wMaxWaitingTasks)
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
            else
            {
                result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
            }
        }
        xSemaphoreGive(pv->handleLockCounter); // End critical section
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    return result;
}

/*Unlock recursive mutex in parameters before wait a signal. On timeout, task is removed from task to notify.*/
eConditionVariableResult P_SYNCHRO_UnlockAndWaitForConditionVariable(
    tConditionVariable* pv,             // Condition variable workspace
    QueueHandle_t handleRecursiveMutex, // Recursive mutex
    unsigned int uwSignal,              // Signal to wait
    unsigned int uwTimeOutMs)           // TimeOut
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    TickType_t xTimeToWait = 0;
    TaskHandle_t handleTask = 0;
    TimeOut_t xTimeOut = {0, 0};
    unsigned int notificationValue = 0;

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
            if (pv->wStatus == E_CONDITION_VARIABLE_STATUS_INITIALIZED) // Check init state.
            {
                // Récupération du handle de task et sauvegarde en pile
                if (pv->wNbRegisteredTasks < pv->wMaxWaitingTasks)
                {
                    handleTask = xTaskGetCurrentTaskHandle();
                    result = RegisterWaitingTask(pv, handleTask, uwSignal);
                }
                else
                {
                    result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
                }
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
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
                // Wait for specified signal bit 0 -> 30. Bit 31 = CLEARING_SIGNAL
                if (xTaskNotifyWait(0, uwSignal | CLEARING_SIGNAL, &notificationValue, xTimeToWait) != pdPASS)
                {
                    result = E_COND_VAR_RESULT_TIMEOUT;
                    // Pas de notification reçue pendant le délai imparti
                    break;
                }
                else
                {
                    // Arrivee notification, check si la notif est celle attendu.
                    if ((notificationValue & CLEARING_SIGNAL) == CLEARING_SIGNAL)
                    {
                        result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
                        break;
                    }
                    else if ((notificationValue & uwSignal) != uwSignal)
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
                if (pv->wStatus == E_CONDITION_VARIABLE_STATUS_INITIALIZED) // Check init state
                {
                    RemoveWaitingTask(pv, handleTask, uwSignal);
                }
                else
                {
                    result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
                }
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
Condition* Condition_Create(void)
{
    tConditionVariable* ptr = NULL;
    ptr = P_SYNCHRO_CreateConditionVariable();
    return (Condition*) ptr;
}

/*Initialize a condition variable*/
void Condition_Delete(Condition* cond)
{
    tConditionVariable* ptr = (tConditionVariable*) cond;
    P_SYNCHRO_DestroyConditionVariable(&ptr);
}

SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_OK;
    eConditionVariableResult resPSYNCH = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptr = (tConditionVariable*) cond;

    if (ptr != NULL)
    {
        resPSYNCH = P_SYNCHRO_InitConditionVariable(ptr, MAX_SIGNAL);
        switch (resPSYNCH)
        {
        case E_COND_VAR_RESULT_OK:
            resSOPC = SOPC_STATUS_OK;
            break;
        case E_COND_VAR_RESULT_ERROR_ALREADY_INITIALIZED:
            resSOPC = SOPC_STATUS_INVALID_STATE;
            break;
        default:
            resSOPC = SOPC_STATUS_NOK;
            break;
        }
    }
    else
    {
        resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return resSOPC;
}

/*Destroy a condition variable.*/
SOPC_ReturnStatus Condition_Clear(Condition* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptrCond = (tConditionVariable*) cond;
    if (ptrCond != NULL)
    {
        resPSYNC = P_SYNCHRO_ClearConditionVariable(ptrCond);
        switch (resPSYNC)
        {
        case E_COND_VAR_RESULT_OK:
            resSOPC = SOPC_STATUS_OK;
            break;
        case E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED:
            resSOPC = SOPC_STATUS_INVALID_STATE;
            break;
        default:
            resSOPC = SOPC_STATUS_NOK;
            break;
        }
    }
    else
    {
        resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return resSOPC;
}

SOPC_ReturnStatus Condition_SignalAll(Condition* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptrCond = (tConditionVariable*) cond;
    if (ptrCond != NULL)
    {
        resPSYNC = P_SYNCHRO_SignalAllConditionVariable(ptrCond, SIGNAL_VALUE);
        switch (resPSYNC)
        {
        case E_COND_VAR_RESULT_OK:
            resSOPC = SOPC_STATUS_OK;
            break;
        case E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED:
            resSOPC = SOPC_STATUS_INVALID_STATE;
            break;
        default:
            resSOPC = SOPC_STATUS_NOK;
            break;
        }
    }
    else
    {
        resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return resSOPC;
}

// Must be called between lock and unlock of Mutex used to wait on condition
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptrCond = (tConditionVariable*) cond;
    QueueHandle_t handleRecursiveMutex = NULL;

    if (ptrCond != NULL)
    {
        if (mut != NULL)
        {
            handleRecursiveMutex = *((QueueHandle_t*) mut);
        }

        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(ptrCond, handleRecursiveMutex, SIGNAL_VALUE, ULONG_MAX);
        switch (resPSYNC)
        {
        case E_COND_VAR_RESULT_OK:
            resSOPC = SOPC_STATUS_OK;
            break;
        case E_COND_VAR_RESULT_TIMEOUT:
            resSOPC = SOPC_STATUS_TIMEOUT;
            break;
        case E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED:
            resSOPC = SOPC_STATUS_INVALID_STATE;
            break;
        default:
            resSOPC = SOPC_STATUS_NOK;
            break;
        }
    }
    else
    {
        resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return resSOPC;
}

// Must be called between lock and unlock of Mutex used to wait on condition
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptrCond = (tConditionVariable*) cond;
    QueueHandle_t handleRecursiveMutex = NULL;

    if (ptrCond != NULL)
    {
        if (mut != NULL)
        {
            handleRecursiveMutex = *((QueueHandle_t*) mut);
        }

        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(ptrCond, handleRecursiveMutex, SIGNAL_VALUE, milliSecs);
        switch (resPSYNC)
        {
        case E_COND_VAR_RESULT_OK:
            resSOPC = SOPC_STATUS_OK;
            break;
        case E_COND_VAR_RESULT_TIMEOUT:
            resSOPC = SOPC_STATUS_TIMEOUT;
            break;
        case E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED:
            resSOPC = SOPC_STATUS_INVALID_STATE;
            break;
        default:
            resSOPC = SOPC_STATUS_NOK;
            break;
        }
    }
    else
    {
        resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return resSOPC;
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
