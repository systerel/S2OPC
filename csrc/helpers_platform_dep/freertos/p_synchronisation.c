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

/*****Private condition variable api*****/

// Clear condition variable: release all thread waiting on it
eConditionVariableResult P_SYNCHRO_ClearConditionVariable(tConditionVariable* pConditionVariable)
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t handle = NULL;
    uint32_t wClearSignal = 0;

    if (pConditionVariable != NULL)
    {
        if ((E_COND_VAR_STATUS_INITIALIZED == pConditionVariable->status) // Workspace initialized
            && (NULL != pConditionVariable->handleLockCounter))           // Critical section token exist
        {
            xSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY); // Critical section
            {
                pConditionVariable->status = E_COND_VAR_STATUS_NOT_INITIALIZED; // Mark as not initialized

                // Indicate for each registered task that clearing signal is pending
                wCurrentSlotId = UINT16_MAX;
                do
                {
                    handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(&pConditionVariable->taskList, //
                                                                       NULL,                          //
                                                                       &wClearSignal,                 //
                                                                       NULL,                          //
                                                                       &wCurrentSlotId);              //
                    if (handle != NULL)
                    {
                        xTaskGenericNotify(handle, wClearSignal, eSetBits, NULL);
                    }
                } while (wCurrentSlotId != UINT16_MAX);

                // Task list destruction
                P_UTILS_LIST_DeInit(&pConditionVariable->taskList);
            }

            vQueueDelete(pConditionVariable->handleLockCounter); // End of critical section. Destroy it on clear
            pConditionVariable->handleLockCounter = NULL;
            DEBUG_decrementCpt();
            memset(pConditionVariable, 0, sizeof(tConditionVariable)); // Raz on leave memory
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
eConditionVariableResult P_SYNCHRO_InitConditionVariable(
    tConditionVariable* pConditionVariable, // Condition variable handle
    uint16_t wMaxWaiters)                   // max parallel waiting tasks
{
    eConditionVariableResult result = E_COND_VAR_RESULT_OK;
    QueueHandle_t pMutex = NULL;

    if ((MAX_WAITERS < wMaxWaiters) || (NULL == pConditionVariable))
    {
        result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    }
    else
    {
        if (E_COND_VAR_STATUS_NOT_INITIALIZED == pConditionVariable->status) // Check if workspace already exists
        {
            // Raz allocated workspaace
            memset(pConditionVariable, 0, sizeof(tConditionVariable));
            pMutex = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);
            if (pMutex != NULL)
            {
                DEBUG_incrementCpt();
                // Allocate list of waiters
                P_UTILS_LIST_Init(&pConditionVariable->taskList, wMaxWaiters);
                if (pConditionVariable->taskList.list == NULL)
                {
                    vQueueDelete(pMutex);
                    pMutex = NULL;

                    // Raz leaved memory
                    memset(pConditionVariable, 0, sizeof(tConditionVariable));
                    DEBUG_decrementCpt();
                    result = E_COND_VAR_RESULT_ERROR_OUT_OF_MEM;
                }
                else
                {
                    pConditionVariable->handleLockCounter = pMutex;
                    pConditionVariable->status = E_COND_VAR_STATUS_INITIALIZED;
                    result = E_COND_VAR_RESULT_OK;
                }
            }
            else
            {
                /* Raz leaved memory*/
                memset(pConditionVariable, 0, sizeof(tConditionVariable));
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
void P_SYNCHRO_DestroyConditionVariable(tConditionVariable** ppConditionVariable)
{
    if ((ppConditionVariable != NULL) && ((*ppConditionVariable) != NULL))
    {
        P_SYNCHRO_ClearConditionVariable(*ppConditionVariable);
        memset(*ppConditionVariable, 0, sizeof(tConditionVariable));
        vPortFree(*ppConditionVariable);
        *ppConditionVariable = NULL;
        DEBUG_decrementCpt();
    }
}

// Creation workspace.
tConditionVariable* P_SYNCHRO_CreateConditionVariable(uint16_t wMaxRDV)
{
    tConditionVariable* pConditionVariable = NULL;
    pConditionVariable = (tConditionVariable*) pvPortMalloc(sizeof(tConditionVariable));
    if (pConditionVariable != NULL)
    {
        DEBUG_incrementCpt();
        // Raz handle
        memset(pConditionVariable, 0, sizeof(tConditionVariable));
        if (P_SYNCHRO_InitConditionVariable(pConditionVariable, wMaxRDV) != E_COND_VAR_RESULT_OK)
        {
            P_SYNCHRO_ClearConditionVariable(pConditionVariable);
            // Raz handle
            memset(pConditionVariable, 0, sizeof(tConditionVariable));
            vPortFree(pConditionVariable);
            pConditionVariable = NULL;
            DEBUG_decrementCpt();
        }
    }
    return pConditionVariable;
}

eConditionVariableResult P_SYNCHRO_SignalConditionVariable(tConditionVariable* pConditionVariable) // Signal to one
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    uint16_t wCurrentSlotId = USHRT_MAX;
    TaskHandle_t handle = NULL;
    uint32_t signal = 0;

    if (pConditionVariable != NULL)
    {
        if ((E_COND_VAR_STATUS_INITIALIZED == pConditionVariable->status) //
            && (NULL != pConditionVariable->handleLockCounter))           //
        {
            xQueueSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY); // Critical section
            {
                wCurrentSlotId = USHRT_MAX;

                handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(&pConditionVariable->taskList, // List of task
                                                                   &signal,                       // Signal to send
                                                                   NULL,                          //
                                                                   NULL,                          //
                                                                   &wCurrentSlotId);              // Slot id
                if (handle != NULL)
                {
                    xTaskGenericNotify(handle, signal, eSetBits, NULL);
                    result = E_COND_VAR_RESULT_OK;
                }
            }
            xSemaphoreGive(pConditionVariable->handleLockCounter); // End critical section
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
eConditionVariableResult P_SYNCHRO_SignalAllConditionVariable(
    tConditionVariable* pConditionVariable) // Signal to broadcaset
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_NO_WAITERS;
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t handle = NULL;
    uint32_t signal = 0;

    if (NULL != pConditionVariable)
    {
        if ((E_COND_VAR_STATUS_INITIALIZED == pConditionVariable->status) &&
            (NULL != pConditionVariable->handleLockCounter))
        {
            xQueueSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY); // Critical section
            {
                wCurrentSlotId = UINT16_MAX;
                do
                {
                    handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(
                        &pConditionVariable->taskList, // Task of list to notify
                        &signal,                       // signal to send
                        NULL,                          //
                        NULL,                          //
                        &wCurrentSlotId);              // Slot id
                    if (handle != NULL)
                    {
                        xTaskGenericNotify(handle, signal, eSetBits, NULL);
                        result = E_COND_VAR_RESULT_OK;
                    }
                } while (wCurrentSlotId != UINT16_MAX);
            }
            xSemaphoreGive(pConditionVariable->handleLockCounter); // End critical section
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

    if (NULL == pNotificationValue)
    {
        result = E_COND_VAR_RESULT_ERROR_NOK;
    }
    else
    {
        while (0 == bQuit)
        {
            vTaskSetTimeOutState(&xTimeOut); // RAZ timeout
            // Wait for specified signal bit 0 -> 30. Bit 31 = CLEARING_SIGNAL
            if (pdPASS != xTaskNotifyWait(0,                        //
                                          uwSignal | uwClearSignal, //
                                          &notificationValue,       //
                                          xTimeToWait))             //
            {
                result = E_COND_VAR_RESULT_ERROR_TIMEOUT;
                // Pas de notification reçue pendant le délai imparti
                bQuit = 1;
            }
            else
            {
                // If others notifications, forward it in order to generate a task event
                if (0 != (notificationValue & (~(uwSignal | uwClearSignal))))
                {
                    xTaskGenericNotify(xTaskGetCurrentTaskHandle(),                       //
                                       notificationValue & (~(uwSignal | uwClearSignal)), //
                                       eSetBits,                                          //
                                       NULL);                                             //
                }
                // Verify if notification arrived
                if (uwClearSignal == (notificationValue & uwClearSignal))
                {
                    // Clearing on going
                    result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
                    bQuit = 1;
                }
                else if (uwSignal != (notificationValue & uwSignal))
                {
                    // Update timeout status
                    if (pdTRUE == xTaskCheckForTimeOut(&xTimeOut, &xTimeToWait))
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
eConditionVariableResult P_SYNCHRO_UnlockAndWaitForConditionVariable(
    tConditionVariable* pConditionVariable, // Condition variable workspace
    QueueHandle_t* pMutex,                  // Recursive mutex
    uint32_t uwSignal,                      // Signal to wait
    uint32_t uwClearSignal,                 // Clear signal
    uint32_t uwTimeOutMs)                   // TimeOut
{
    eConditionVariableResult result = E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    TickType_t xTimeToWait = 0;
    TaskHandle_t handleTask = 0;
    uint32_t notificationValue = 0;

    if (pConditionVariable != NULL)
    {
        if (uwTimeOutMs >= (portMAX_DELAY / (configTICK_RATE_HZ * ((uint32_t) 1000))))
        {
            xTimeToWait = portMAX_DELAY;
        }
        else
        {
            xTimeToWait = pdMS_TO_TICKS(uwTimeOutMs);
        }

        if (                                                              // Check workspace exists
            (E_COND_VAR_STATUS_INITIALIZED == pConditionVariable->status) // Check workspace initialization
            && (NULL != pConditionVariable->handleLockCounter))           // Check sem existence
        {
            // Critical section
            xQueueSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY);
            {
                handleTask = xTaskGetCurrentTaskHandle();

                status = P_UTILS_LIST_AddElt(&pConditionVariable->taskList, // Task waiting
                                             handleTask,                    // Current task
                                             NULL,                          // No context
                                             uwSignal,                      // Signal
                                             uwClearSignal);                // Clear signal

                if (SOPC_STATUS_OK != status)
                {
                    result = E_COND_VAR_RESULT_ERROR_MAX_WAITERS;
                }
                else
                {
                    result = E_COND_VAR_RESULT_OK;
                }
            }
            xSemaphoreGive(pConditionVariable->handleLockCounter);
        }
        else
        {
            notificationValue = uwClearSignal;
            result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
        }

        // Give mutex from parameters
        if (NULL != (*pMutex))
        {
            xQueueGiveMutexRecursive(*pMutex);
        }

        if (E_COND_VAR_RESULT_OK == result)
        {
            // Wait signal or timeout
            // If signal or timeout, in both case, unstack signal
            // (we signal to the waiting task that nb of waiting task is decremented)

            result = _P_SYNCHRO_WaitSignal(&notificationValue, // Notif
                                           uwSignal,           // Signal waited
                                           uwClearSignal,      // Clear signal
                                           xTimeToWait);       // Tick timeout

            // Critical section

            if ((uwClearSignal != (notificationValue & uwClearSignal))           // Not a clear signal
                && (E_COND_VAR_STATUS_INITIALIZED == pConditionVariable->status) // Workspace initilized
                && (NULL != pConditionVariable->handleLockCounter))              // Lock exists
            {
                xQueueSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY); // Critical section
                {
                    P_UTILS_LIST_RemoveElt(&pConditionVariable->taskList, //
                                           handleTask,                    //
                                           uwSignal,                      //
                                           uwClearSignal,                 //
                                           NULL);                         //
                }
                xSemaphoreGive(pConditionVariable->handleLockCounter);

                result = E_COND_VAR_RESULT_OK;
            }
            else
            {
                result = E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED;
            }
        }

        // Take mutex in parameter if exists
        if (NULL != *pMutex)
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
tConditionVariable* Condition_Create(void)
{
    tConditionVariable* ptr = NULL;
    ptr = P_SYNCHRO_CreateConditionVariable(MAX_WAITERS);
    return (tConditionVariable*) ptr;
}

/*Initialize a condition variable*/
void Condition_Delete(tConditionVariable* cond)
{
    tConditionVariable* ptr = (tConditionVariable*) cond;
    if (ptr != NULL)
    {
        P_SYNCHRO_DestroyConditionVariable(&ptr);
        ptr = NULL;
    }
}

SOPC_ReturnStatus Condition_Init(tConditionVariable* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_OK;
    eConditionVariableResult resPSYNCH = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptr = (tConditionVariable*) cond;

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
SOPC_ReturnStatus Condition_Clear(tConditionVariable* cond)
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

SOPC_ReturnStatus Condition_SignalAll(tConditionVariable* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptrCond = (tConditionVariable*) cond;
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
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(tConditionVariable* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;
    tConditionVariable* ptrCond = (tConditionVariable*) cond;
    QueueHandle_t* pFreeRtosMutex = mut;

    if (ptrCond != NULL)
    {
        if (mut != NULL)
        {
            pFreeRtosMutex = ((QueueHandle_t*) mut);
        }

        resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(ptrCond,             //
                                                               pFreeRtosMutex,      //
                                                               APP_DEFAULT_SIGNAL,  //
                                                               APP_CLEARING_SIGNAL, //
                                                               milliSecs);          //
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
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(tConditionVariable* cond, Mutex* mut)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;

    resSOPC = Mutex_UnlockAndTimedWaitCond(cond, mut, UINT32_MAX);

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
        freeRtosMutex = xSemaphoreCreateRecursiveMutex();
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
            vSemaphoreDelete(handleRecursiveMutex);
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
