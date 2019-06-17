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

#include <inttypes.h> /* stdlib includes */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h" /* freeRtos includes */
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "sopc_enums.h" /* s2opc includes */

#include "p_synchronisation.h" /* synchronisation include */
#include "p_utils.h"           /* private list include */

/*****Private declarations***************/

typedef enum T_CONDITION_VARIABLE_STATUS
{
    E_COND_VAR_STATUS_NOT_INITIALIZED, // Condition variable not initialized
    E_COND_VAR_STATUS_INITIALIZED      // Condition variable initialized
} eConditionVariableStatus;

struct T_CONDITION_VARIABLE
{
    eConditionVariableStatus status; // Status condition variable
    QueueHandle_t handleLockCounter; // Critical section token
    tUtilsList taskList;             // List of task with signal expected, calling unlock and wait
};

/*****Private condition variable api*****/

// Clear condition variable: release all thread waiting on it
eConditionVariableResult P_SYNCHRO_ClearConditionVariable(hCondVar* pv)
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t handle = NULL;
    uint32_t wClearSignal = 0;

    if (pv != NULL)
    {
        if (((*pv) != NULL)                                     // Workspace initialized
            && ((*pv)->status == E_COND_VAR_STATUS_INITIALIZED) // Workspace initialized
            && ((*pv)->handleLockCounter != NULL))              // Critical section token exist
        {
            xSemaphoreTake((*pv)->handleLockCounter, portMAX_DELAY); // Critical section
            {
                (*pv)->status = E_COND_VAR_STATUS_NOT_INITIALIZED; // Mark as not initialized

                // Indicate for each registered task that clearing signal is pending
                wCurrentSlotId = UINT16_MAX;
                do
                {
                    handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(&(*pv)->taskList, //
                                                                       NULL,             //
                                                                       &wClearSignal,    //
                                                                       NULL,             //
                                                                       &wCurrentSlotId); //
                    if (handle != NULL)
                    {
                        xTaskGenericNotify(handle, wClearSignal, eSetBits, NULL);
                    }
                } while (wCurrentSlotId != UINT16_MAX);

                // Task list destruction
                P_UTILS_LIST_DeInit(&(*pv)->taskList);
            }

            vQueueDelete((*pv)->handleLockCounter); // End of critical section. Destroy it on clear
            (*pv)->handleLockCounter = NULL;
            DEBUG_decrementCpt();
            if ((*pv) != NULL) // Destroy workspace
            {
                memset(*pv, 0, sizeof(tConditionVariable)); // Raz on leave memory
                vPortFree(*pv);
                *pv = NULL;
                DEBUG_decrementCpt();
            }
        }
        else
        {
            result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
        }
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    return result;
}

// Initialize condition variable
eConditionVariableResult P_SYNCHRO_InitConditionVariable(hCondVar* pv,         // Condition variable handle
                                                         uint16_t wMaxWaiters) // max parallel waiting tasks
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    QueueHandle_t pMutex = NULL;
    hCondVar condVar = NULL;

    if ((wMaxWaiters > MAX_P_UTILS_LIST) || (pv == NULL))
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    else
    {
        if ((*pv) == NULL) // Check if workspace already exists
        {
            condVar = (tConditionVariable*) pvPortMalloc(sizeof(tConditionVariable));
            if (condVar != NULL)
            {
                DEBUG_incrementCpt();
                // Raz allocated workspaace
                memset(condVar, 0, sizeof(tConditionVariable));

                pMutex = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
                if (pMutex != NULL)
                {
                    DEBUG_incrementCpt();
                    // Allocate list of waiters
                    P_UTILS_LIST_Init(&condVar->taskList, wMaxWaiters);
                    if (condVar->taskList.list == NULL)
                    {
                        vQueueDelete(pMutex);
                        pMutex = NULL;

                        // Raz leaved memory
                        memset(condVar, 0, sizeof(tConditionVariable));
                        vPortFree(condVar);
                        condVar = NULL;
                        DEBUG_decrementCpt();
                        DEBUG_decrementCpt();
                        result = E_COND_VAR_RESULT_ERROR_OUT_OF_MEM;
                    }
                    else
                    {
                        condVar->handleLockCounter = pMutex;
                        condVar->status = E_COND_VAR_STATUS_INITIALIZED;
                        *pv = condVar;
                        result = E_COND_VAR_RESULT_OK;
                    }
                }
                else
                {
                    /* Raz leaved memory*/
                    memset(condVar, 0, sizeof(tConditionVariable));
                    vPortFree(condVar);
                    condVar = NULL;
                    DEBUG_decrementCpt();
                    result = E_COND_VAR_RESULT_ERROR_OUT_OF_MEM;
                }
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_OUT_OF_MEM;
            }
        }
        else
        {
            result = E_COND_VAR_RESULT_ERROR_ALREADY_INITIALIZED;
        }
    }
    return result;
}

// Destruction of condition variable if created via CreateConditionVariable.
void P_SYNCHRO_DestroyConditionVariable(hCondVar** ppv)
{
    hCondVar* ptrHCondVar = NULL;

    if (ppv != NULL)
    {
        ptrHCondVar = *ppv;
        if (ptrHCondVar != NULL)
        {
            P_SYNCHRO_ClearConditionVariable(ptrHCondVar);
            memset(ptrHCondVar, 0, sizeof(hCondVar));
            vPortFree(ptrHCondVar);
            *ppv = NULL;
        }
        DEBUG_decrementCpt();
    }
}

// Creation workspace.
hCondVar* P_SYNCHRO_CreateConditionVariable(void)
{
    hCondVar* ptr = NULL;
    ptr = (hCondVar*) pvPortMalloc(sizeof(hCondVar));
    if (ptr != NULL)
    {
        DEBUG_incrementCpt();
        // Raz handle
        memset(ptr, 0, sizeof(hCondVar));
        if (P_SYNCHRO_InitConditionVariable(ptr, MAX_WAITERS) != E_COND_VAR_RESULT_OK)
        {
            P_SYNCHRO_ClearConditionVariable(ptr);
            // Raz handle
            memset(ptr, 0, sizeof(hCondVar));
            vPortFree(ptr);
            ptr = NULL;
            DEBUG_decrementCpt();
        }
    }
    return ptr;
}

eConditionVariableResult P_SYNCHRO_SignalConditionVariable(hCondVar* pv) // Signal to one
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    uint16_t wCurrentSlotId = USHRT_MAX;
    TaskHandle_t handle = NULL;
    uint32_t signal = 0;

    if (pv != NULL)
    {
        if (((*pv) != NULL)                                     //
            && ((*pv)->status == E_COND_VAR_STATUS_INITIALIZED) //
            && ((*pv)->handleLockCounter != NULL))              //
        {
            xQueueSemaphoreTake((*pv)->handleLockCounter, portMAX_DELAY); // Critical section
            {
                wCurrentSlotId = USHRT_MAX;

                handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(&(*pv)->taskList, // List of task
                                                                   &signal,          // Signal to send
                                                                   NULL,             //
                                                                   NULL,             //
                                                                   &wCurrentSlotId); // Slot id
                if (handle != NULL)
                {
                    xTaskGenericNotify(handle, signal, eSetBits, NULL);
                    result = E_COND_VAR_RESULT_OK;
                }
            }
            xSemaphoreGive((*pv)->handleLockCounter); // End critical section
        }
        else
        {
            result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
        }
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    return result;
}

// Broadcast signal to all waiting task on signal passed in parameters
eConditionVariableResult P_SYNCHRO_SignalAllConditionVariable(hCondVar* pv) // Signal to broadcaset
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t handle = NULL;
    uint32_t signal = 0;

    if (pv != NULL)
    {
        if (((*pv) != NULL) && ((*pv)->status == E_COND_VAR_STATUS_INITIALIZED) && ((*pv)->handleLockCounter != NULL))
        {
            xQueueSemaphoreTake((*pv)->handleLockCounter, portMAX_DELAY); // Critical section
            {
                wCurrentSlotId = UINT16_MAX;
                do
                {
                    handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(&(*pv)->taskList, // Task of list to notify
                                                                       &signal,          // signal to send
                                                                       NULL,             //
                                                                       NULL,             //
                                                                       &wCurrentSlotId); // Slot id
                    if (handle != NULL)
                    {
                        xTaskGenericNotify(handle, signal, eSetBits, NULL);
                        result = E_COND_VAR_RESULT_OK;
                    }
                } while (wCurrentSlotId != UINT16_MAX);
            }
            xSemaphoreGive((*pv)->handleLockCounter); // End critical section
        }
        else
        {
            result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
        }
    }
    else
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    return result;
}

// Used by P_SYNCHRO_UnlockAndWaitForConditionVariable
static inline eConditionVariableResult _P_SYNCHRO_WaitSignal(uint32_t* pNotificationValue, // Notif received
                                                             uint32_t uwSignal,            // Signal waited
                                                             uint32_t uwClearSignal,       // Clear signal
                                                             TickType_t xTimeToWait)       // TimeOut
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    uint32_t notificationValue = 0;
    uint8_t bQuit = 0;
    TimeOut_t xTimeOut = {0, 0};

    if (pNotificationValue == NULL)
    {
        result = E_COND_VAR_RESULT_ERROR_NOK;
    }
    else
    {
        while (bQuit == 0)
        {
            vTaskSetTimeOutState(&xTimeOut); // RAZ timeout
            // Wait for specified signal bit 0 -> 30. Bit 31 = CLEARING_SIGNAL
            if (xTaskNotifyWait(0,                        //
                                uwSignal | uwClearSignal, //
                                &notificationValue,       //
                                xTimeToWait) != pdPASS)   //
            {
                result = E_COND_VAR_RESULT_ERROR_TIMEOUT;
                // Pas de notification reçue pendant le délai imparti
                bQuit = 1;
            }
            else
            {
                // If others notifications, forward it in order to generate a task event
                if ((notificationValue & (~(uwSignal | uwClearSignal))) != 0)
                {
                    xTaskGenericNotify(xTaskGetCurrentTaskHandle(),                       //
                                       notificationValue & (~(uwSignal | uwClearSignal)), //
                                       eSetBits,                                          //
                                       NULL);                                             //
                }
                // Verify if notification arrived
                if ((notificationValue & uwClearSignal) == uwClearSignal)
                {
                    // Clearing on going
                    result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
                    bQuit = 1;
                }
                else if ((notificationValue & uwSignal) != uwSignal)
                {
                    // Update timeout status
                    if (xTaskCheckForTimeOut(&xTimeOut, &xTimeToWait) == pdTRUE)
                    {
                        // Timeout occurred
                        result = E_COND_VAR_RESULT_ERROR_TIMEOUT;
                        bQuit = 1;
                    }
                }
                else
                {
                    // Waiting notification
                    bQuit = 1;
                }
            }
        }

        *pNotificationValue = notificationValue;
    }

    return result;
}

// Unlock recursive mutex in parameters before wait a signal.
// On timeout, task is removed from task to notify.
eConditionVariableResult P_SYNCHRO_UnlockAndWaitForConditionVariable(hCondVar* pv, // Condition variable workspace
                                                                     QueueHandle_t* pMutex,  // Recursive mutex
                                                                     uint32_t uwSignal,      // Signal to wait
                                                                     uint32_t uwClearSignal, // Clear signal
                                                                     uint32_t uwTimeOutMs)   // TimeOut
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    TickType_t xTimeToWait = 0;
    TaskHandle_t handleTask = 0;
    uint32_t notificationValue = 0;

    if (pv != NULL)
    {
        if (uwTimeOutMs == ULONG_MAX)
        {
            xTimeToWait = portMAX_DELAY;
        }
        else
        {
            xTimeToWait = pdMS_TO_TICKS(uwTimeOutMs);
        }

        if (((*pv) != NULL)                                     // Check workspace exists
            && ((*pv)->status == E_COND_VAR_STATUS_INITIALIZED) // Check workspace initialization
            && ((*pv)->handleLockCounter != NULL))              // Check sem existence
        {
            // Critical section
            xQueueSemaphoreTake((*pv)->handleLockCounter, portMAX_DELAY);
            {
                handleTask = xTaskGetCurrentTaskHandle();

                status = P_UTILS_LIST_AddElt(&(*pv)->taskList, // Task waiting
                                             handleTask,       // Current task
                                             NULL,             // No context
                                             uwSignal,         // Signal
                                             uwClearSignal);   // Clear signal

                if (SOPC_STATUS_OK != status)
                {
                    result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
                }
                else
                {
                    result = E_COND_VAR_RESULT_OK;
                }
            }
            xSemaphoreGive((*pv)->handleLockCounter);
        }
        else
        {
            notificationValue = uwClearSignal;
            result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
        }

        // Give mutex from parameters
        if ((*pMutex) != NULL)
        {
            xQueueGiveMutexRecursive(*pMutex);
        }

        if (result == E_COND_VAR_RESULT_OK)
        {
            // Wait signal or timeout
            // If signal or timeout, in both case, unstack signal
            // (we signal to the waiting task that nb of waiting task is decremented)

            result = _P_SYNCHRO_WaitSignal(&notificationValue, // Notif
                                           uwSignal,           // Signal waited
                                           uwClearSignal,      // Clear signal
                                           xTimeToWait);       // Tick timeout

            // Critical section

            if (((notificationValue & uwClearSignal) != uwClearSignal) // Not a clear signal
                && ((*pv) != NULL)                                     // Workspace exists
                && ((*pv)->status == E_COND_VAR_STATUS_INITIALIZED)    // Workspace initilized
                && ((*pv)->handleLockCounter != NULL))                 // Lock exists
            {
                xQueueSemaphoreTake((*pv)->handleLockCounter, portMAX_DELAY); // Critical section
                {
                    P_UTILS_LIST_RemoveElt(&(*pv)->taskList, //
                                           handleTask,       //
                                           uwSignal,         //
                                           uwClearSignal,    //
                                           NULL);            //
                }
                xSemaphoreGive((*pv)->handleLockCounter);

                result = E_COND_VAR_RESULT_OK;
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
            }
        }

        // Take mutex in parameter if exists
        if (*pMutex != NULL)
        {
            xQueueTakeMutexRecursive(*pMutex, portMAX_DELAY);
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
    hCondVar* ptr = NULL;
    ptr = P_SYNCHRO_CreateConditionVariable();
    return (Condition*) ptr;
}

/*Initialize a condition variable*/
void Condition_Delete(Condition* cond)
{
    hCondVar* ptr = (hCondVar*) cond;
    if (ptr != NULL)
    {
        P_SYNCHRO_DestroyConditionVariable(&ptr);
        ptr = NULL;
    }
}

SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_OK;
    eConditionVariableResult resPSYNCH = E_COND_VAR_RESULT_OK;
    hCondVar* ptr = (hCondVar*) cond;

    if (ptr != NULL)
    {
        resPSYNCH = P_SYNCHRO_InitConditionVariable(ptr, MAX_P_UTILS_LIST);
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
    hCondVar* ptrCond = (hCondVar*) cond;
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
    hCondVar* ptrCond = (hCondVar*) cond;
    if (ptrCond != NULL)
    {
        resPSYNC = P_SYNCHRO_SignalAllConditionVariable(ptrCond);
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
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    hCondVar* ptrCond = (hCondVar*) cond;
    QueueHandle_t* pFreeRtosMutex = mut;

    if (ptrCond != NULL)
    {
        if (mut != NULL)
        {
            pFreeRtosMutex = ((QueueHandle_t*) mut);
        }

        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(ptrCond, pFreeRtosMutex, APP_DEFAULT_SIGNAL,
                                                               APP_CLEARING_SIGNAL, milliSecs);
        switch (resPSYNC)
        {
        case E_COND_VAR_RESULT_OK:
            resSOPC = SOPC_STATUS_OK;
            break;
        case E_COND_VAR_RESULT_ERROR_TIMEOUT:
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
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;

    resSOPC = Mutex_UnlockAndTimedWaitCond(cond, mut, ULONG_MAX);

    return resSOPC;
}

/*Create recursive mutex*/
SOPC_ReturnStatus Mutex_Initialization(Mutex* mut)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    QueueHandle_t freeRtosMutex = NULL;

    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        freeRtosMutex = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
        if (freeRtosMutex == NULL)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
            *mut = NULL;
        }
        else
        {
            result = SOPC_STATUS_OK;
            *mut = (Mutex) freeRtosMutex;
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
    QueueHandle_t freeRtosMutex = NULL;
    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        freeRtosMutex = *((QueueHandle_t*) (mut));
        if (freeRtosMutex == NULL)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            if (xQueueTakeMutexRecursive(freeRtosMutex, portMAX_DELAY) == pdPASS)
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
    QueueHandle_t freeRtosMutex = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (mut == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        freeRtosMutex = *((QueueHandle_t*) (mut));
        if (freeRtosMutex == NULL)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            if (xQueueGiveMutexRecursive(freeRtosMutex) == pdPASS)
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
