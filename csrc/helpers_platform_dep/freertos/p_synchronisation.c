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
#include "sopc_mem_alloc.h"

#include "p_synchronisation.h" /* synchronisation include */
#include "p_utils.h"           /* private list include */

/*****Private condition variable api*****/

// Clear condition variable: release all thread waiting on it
SOPC_ReturnStatus P_SYNCHRO_ClearConditionVariable(Condition* pConditionVariable)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t handle = NULL;
    uint32_t wClearSignal = 0;
    uint32_t wSignal = 0;

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
                                                                       &wSignal,                      //
                                                                       &wClearSignal,                 //
                                                                       NULL,                          //
                                                                       &wCurrentSlotId);              //
                    if (handle != NULL)
                    {
                        // Remove task before notify it
                        P_UTILS_LIST_RemoveElt(&pConditionVariable->taskList, //
                                               handle,                        //
                                               wSignal,                       //
                                               wClearSignal,                  //
                                               &wCurrentSlotId);              //

                        xTaskGenericNotify(handle, wClearSignal, eSetBits, NULL);
                    }
                } while (wCurrentSlotId != UINT16_MAX);

                // Task list destruction
                P_UTILS_LIST_DeInit(&pConditionVariable->taskList);
            }

            vQueueDelete(pConditionVariable->handleLockCounter); // End of critical section. Destroy it on clear
            pConditionVariable->handleLockCounter = NULL;
            DEBUG_decrementCpt();
            memset(pConditionVariable, 0, sizeof(Condition)); // Raz on leave memory
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return result;
}

// Initialize condition variable
SOPC_ReturnStatus P_SYNCHRO_InitConditionVariable(Condition* pConditionVariable, // Condition variable handle
                                                  uint16_t wMaxWaiters)          // max parallel waiting tasks
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    QueueHandle_t pMutex = NULL;

    if ((MAX_WAITERS < wMaxWaiters) || (NULL == pConditionVariable))
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        if (E_COND_VAR_STATUS_NOT_INITIALIZED == pConditionVariable->status) // Check if workspace already exists
        {
            // Raz allocated workspaace
            memset(pConditionVariable, 0, sizeof(Condition));
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
                    memset(pConditionVariable, 0, sizeof(Condition));
                    DEBUG_decrementCpt();
                    result = SOPC_STATUS_OUT_OF_MEMORY;
                }
                else
                {
                    pConditionVariable->handleLockCounter = pMutex;
                    pConditionVariable->status = E_COND_VAR_STATUS_INITIALIZED;
                    result = SOPC_STATUS_OK;
                }
            }
            else
            {
                /* Raz leaved memory*/
                memset(pConditionVariable, 0, sizeof(Condition));
                result = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        else
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
    }
    return result;
}

// Destruction of condition variable if created via CreateConditionVariable.
void P_SYNCHRO_DestroyConditionVariable(Condition** ppConditionVariable)
{
    if ((ppConditionVariable != NULL) && ((*ppConditionVariable) != NULL))
    {
        P_SYNCHRO_ClearConditionVariable(*ppConditionVariable);
        memset(*ppConditionVariable, 0, sizeof(Condition));
        SOPC_Free(*ppConditionVariable);
        *ppConditionVariable = NULL;
        DEBUG_decrementCpt();
    }
}

// Creation workspace.
Condition* P_SYNCHRO_CreateConditionVariable(uint16_t wMaxRDV)
{
    Condition* pConditionVariable = NULL;
    pConditionVariable = (Condition*) SOPC_Malloc(sizeof(Condition));
    if (pConditionVariable != NULL)
    {
        DEBUG_incrementCpt();
        // Raz handle
        memset(pConditionVariable, 0, sizeof(Condition));
        if (P_SYNCHRO_InitConditionVariable(pConditionVariable, wMaxRDV) != SOPC_STATUS_OK)
        {
            P_SYNCHRO_ClearConditionVariable(pConditionVariable);
            // Raz handle
            memset(pConditionVariable, 0, sizeof(Condition));
            SOPC_Free(pConditionVariable);
            pConditionVariable = NULL;
            DEBUG_decrementCpt();
        }
    }
    return pConditionVariable;
}

// Broadcast signal to all waiting task on signal passed in parameters
SOPC_ReturnStatus P_SYNCHRO_SignalConditionVariable(Condition* pConditionVariable, // Signal to broadcaset
                                                    bool bSignalAll)               // Signal one (false) or all (true)
{
    SOPC_ReturnStatus result = SOPC_STATUS_INVALID_STATE;
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t handle = NULL;
    uint32_t signal = 0;
    uint32_t clearsignal = 0;

    if (NULL == pConditionVariable)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if ((E_COND_VAR_STATUS_INITIALIZED != pConditionVariable->status) ||
        (NULL == pConditionVariable->handleLockCounter))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    xQueueSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY); // Critical section
    {
        wCurrentSlotId = UINT16_MAX;
        do
        {
            handle = (TaskHandle_t) P_UTILS_LIST_ParseValueElt(&pConditionVariable->taskList, // Task of list to notify
                                                               &signal,                       // signal to send
                                                               &clearsignal,                  // clear signal
                                                               NULL,                          //
                                                               &wCurrentSlotId);              // Slot id

            if (handle != NULL)
            {
                // Remove task before notify it
                P_UTILS_LIST_RemoveElt(&pConditionVariable->taskList, //
                                       handle,                        //
                                       signal,                        //
                                       clearsignal,                   //
                                       &wCurrentSlotId);              //

                configASSERT(pdPASS == xTaskGenericNotify(handle, signal, eSetBits, NULL));
                result = SOPC_STATUS_OK;
            }
        } while ((UINT16_MAX != wCurrentSlotId) && (true == bSignalAll));
    }
    xSemaphoreGive(pConditionVariable->handleLockCounter); // End critical section

    return result;
}

// Used by P_SYNCHRO_UnlockAndWaitForConditionVariable
static inline SOPC_ReturnStatus P_SYNCHRO_WaitSignal(uint32_t* pNotificationValue, // Notif received
                                                     uint32_t uwSignal,            // Signal waited
                                                     uint32_t uwClearSignal,       // Clear signal
                                                     TickType_t xTimeToWait)       // TimeOut
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    uint32_t notificationValue = 0;
    bool bQuit = false;
    TimeOut_t xTimeOut = {0, 0};
    BaseType_t resRtos = pdPASS;

    if (NULL == pNotificationValue)
    {
        result = SOPC_STATUS_NOK;
    }
    else
    {
        while (false == bQuit)
        {
            vTaskSetTimeOutState(&xTimeOut); // RAZ timeout
            // Wait for specified signal bit 0 -> 30. Bit 31 = CLEARING_SIGNAL
            resRtos = xTaskNotifyWait(0,                        //
                                      uwSignal | uwClearSignal, //
                                      &notificationValue,       //
                                      xTimeToWait);             //

            if (pdPASS != resRtos)
            {
                result = SOPC_STATUS_TIMEOUT;
                // TimeOut
                bQuit = true;
            }
            else
            {
                // Verify if notification arrived
                if (uwClearSignal == (notificationValue & uwClearSignal))
                {
                    // Clearing on going
                    result = SOPC_STATUS_INVALID_STATE;
                    bQuit = true;
                }
                else if (uwSignal != (notificationValue & uwSignal))
                {
                    // Update timeout status
                    resRtos = xTaskCheckForTimeOut(&xTimeOut, &xTimeToWait);

                    if (pdTRUE == resRtos)
                    {
                        // Timeout occurred
                        result = SOPC_STATUS_TIMEOUT;
                        bQuit = true;
                    }
                }
                else
                {
                    // Waited signal received. Quit waiting loop with E_COND_VAR_RESULT_OK
                    bQuit = true;
                }
            }
        }

        // If others notifications, forward it in order to generate a task event
        if (0 != (notificationValue & (~(uwSignal | uwClearSignal))))
        {
            xTaskGenericNotify(xTaskGetCurrentTaskHandle(),                       //
                               notificationValue & (~(uwSignal | uwClearSignal)), //
                               eSetBits,                                          //
                               NULL);                                             //
        }

        *pNotificationValue = notificationValue;
    }

    return result;
}

// Unlock recursive mutex in parameters before wait a signal.
// Mutex in parameter is optional. In this case, simple wait signal.
// On timeout, task is removed from task to notify.
SOPC_ReturnStatus P_SYNCHRO_UnlockAndWaitForConditionVariable(
    Condition* pConditionVariable, // Condition variable workspace
    QueueHandle_t* pMutex,         // Recursive mutex
    uint32_t uwSignal,             // Signal to wait
    uint32_t uwClearSignal,        // Clear signal
    uint32_t uwTimeOutMs)          // TimeOut
{
    SOPC_ReturnStatus result = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    TickType_t xTimeToWait = 0;
    TaskHandle_t handleTask = 0;
    uint32_t notificationValue = 0;

    if (NULL == pConditionVariable)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (uwTimeOutMs >= (portMAX_DELAY / (configTICK_RATE_HZ * ((uint32_t) 1000))))
    {
        xTimeToWait = portMAX_DELAY;
    }
    else
    {
        xTimeToWait = pdMS_TO_TICKS(uwTimeOutMs);
    }

    if (                                                              // Check workspace exists
        (E_COND_VAR_STATUS_INITIALIZED != pConditionVariable->status) // Check workspace initialization
        || (NULL == pConditionVariable->handleLockCounter))           // Check sem existence
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Critical section
    xQueueSemaphoreTake(pConditionVariable->handleLockCounter, portMAX_DELAY);
    {
        handleTask = xTaskGetCurrentTaskHandle();

        status = P_UTILS_LIST_AddElt(&pConditionVariable->taskList, // Task waiting
                                     handleTask,                    // Current task
                                     NULL,                          // No context
                                     uwSignal,                      // Signal
                                     uwClearSignal);                // Clear signal

        // Reset pendings signal, normally it's not necessary

        xTaskNotifyWait(uwSignal | uwClearSignal, //
                        uwSignal | uwClearSignal, //
                        &notificationValue,       //
                        0);                       //

        // If others notifications, forward it in order to generate a task event
        if (0 != (notificationValue & (~(uwSignal | uwClearSignal))))
        {
            xTaskGenericNotify(xTaskGetCurrentTaskHandle(),                       //
                               notificationValue & (~(uwSignal | uwClearSignal)), //
                               eSetBits,                                          //
                               NULL);                                             //
        }

        if (SOPC_STATUS_OK != status)
        {
            result = SOPC_STATUS_INVALID_STATE;
        }
        else
        {
            result = SOPC_STATUS_OK;
        }
    }
    xSemaphoreGive(pConditionVariable->handleLockCounter);

    if (SOPC_STATUS_OK != result)
    {
        return result;
    }

    // Give mutex from parameters
    if ((NULL != pMutex) && (NULL != (*pMutex)))
    {
        configASSERT(xQueueGiveMutexRecursive(*pMutex) == pdPASS);
    }
    // Wait signal or timeout
    // If signal or timeout, in both case, unstack signal

    result = P_SYNCHRO_WaitSignal(&notificationValue, // Notif
                                  uwSignal,           // Signal waited
                                  uwClearSignal,      // Clear signal
                                  xTimeToWait);       // Tick timeout

    // Remove from task list in case of SOPC_STATUS_TIMEOUT

    if ((uwClearSignal != (notificationValue & uwClearSignal))           // Not a clear signal
        && (E_COND_VAR_STATUS_INITIALIZED == pConditionVariable->status) // Workspace initilized
        && (NULL != pConditionVariable->handleLockCounter))              // Lock exists
    {
        // If timeout occured, remove the task from the list.
        if (SOPC_STATUS_TIMEOUT == result)
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
        }

        // Result updated by P_SYNCHRO_WaitSignal. Can be SOPC_OK or TIMEOUT !!!
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    // Take mutex in parameter if exists
    if ((NULL != pMutex) && (NULL != (*pMutex)))
    {
        configASSERT(xQueueTakeMutexRecursive(*pMutex, portMAX_DELAY) == pdPASS);
    }
    return result;
}

/*****Public s2opc condition variable and mutex api*****/
/*
Condition* Condition_Create(void)
{
    Condition* ptr = NULL;
    ptr = P_SYNCHRO_CreateConditionVariable(MAX_WAITERS);
    return (Condition*) ptr;
}
void Condition_Delete(Condition* cond)
{
    Condition* ptr = (Condition*) cond;
    if (ptr != NULL)
    {
        P_SYNCHRO_DestroyConditionVariable(&ptr);
        ptr = NULL;
    }
}
*/
SOPC_ReturnStatus Condition_Init(Condition* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != cond)
    {
        memset(cond, 0, sizeof(Condition));
        resSOPC = P_SYNCHRO_InitConditionVariable(cond, MAX_WAITERS);
    }
    return resSOPC;
}

/*Destroy a condition variable.*/
SOPC_ReturnStatus Condition_Clear(Condition* cond)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != cond)
    {
        resSOPC = P_SYNCHRO_ClearConditionVariable(cond);
        memset(cond, 0, sizeof(Condition));
    }
    return resSOPC;
}

SOPC_ReturnStatus Condition_SignalAll(Condition* cond)
{
    return P_SYNCHRO_SignalConditionVariable(cond, true);
}

// Must be called between lock and unlock of Mutex used to wait on condition
SOPC_ReturnStatus Mutex_UnlockAndTimedWaitCond(Condition* cond, Mutex* mut, uint32_t milliSecs)
{
    SOPC_ReturnStatus resSOPC = SOPC_STATUS_INVALID_PARAMETERS;
    QueueHandle_t* pFreeRtosMutex = mut;

    if (NULL != cond && NULL != mut && NULL != (*mut))
    {
        pFreeRtosMutex = ((QueueHandle_t*) mut);
        resSOPC = P_SYNCHRO_UnlockAndWaitForConditionVariable(cond,                //
                                                              pFreeRtosMutex,      //
                                                              APP_DEFAULT_SIGNAL,  //
                                                              APP_CLEARING_SIGNAL, //
                                                              milliSecs);          //
    }
    return resSOPC;
}

// Must be called between lock and unlock of Mutex used to wait on condition
SOPC_ReturnStatus Mutex_UnlockAndWaitCond(Condition* cond, Mutex* mut)
{
    return Mutex_UnlockAndTimedWaitCond(cond, mut, UINT32_MAX);
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
    QueueHandle_t freeRtosMutex = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if ((NULL == mut) || (NULL == (*mut)))
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        freeRtosMutex = (QueueHandle_t)(*mut);
        vSemaphoreDelete(freeRtosMutex);
        *mut = NULL;
    }
    return result;
}

// Lock recursive mutex
SOPC_ReturnStatus Mutex_Lock(Mutex* mut)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    BaseType_t resRtos = pdPASS;
    QueueHandle_t freeRtosMutex = NULL;
    if ((NULL == mut) || (NULL == (*mut)))
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        freeRtosMutex = *((QueueHandle_t*) (mut));

        resRtos = xQueueTakeMutexRecursive(freeRtosMutex, portMAX_DELAY);
        if (pdPASS == resRtos)
        {
            result = SOPC_STATUS_OK;
        }
        else
        {
            result = SOPC_STATUS_NOK;
        }
    }
    return result;
}

// Unlock recursive mutex
SOPC_ReturnStatus Mutex_Unlock(Mutex* mut)
{
    QueueHandle_t freeRtosMutex = NULL;
    BaseType_t resRtos = pdPASS;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if ((NULL == mut) || ((*mut) == NULL))
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        freeRtosMutex = *((QueueHandle_t*) (mut));
        resRtos = xQueueGiveMutexRecursive(freeRtosMutex);
        if (pdPASS == resRtos)
        {
            result = SOPC_STATUS_OK;
        }
        else
        {
            result = SOPC_STATUS_NOK;
        }
    }
    return result;
}
