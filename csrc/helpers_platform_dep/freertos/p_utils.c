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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> /*stdlib*/
#include <string.h>

#include "FreeRTOS.h" /*freeRtos*/
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_utils.h"

/*Alloc task*/
eUtilsListResult P_UTILS_LIST_Init(tUtilsList* ptr, uint16_t wMaxRDV)
{
    if (NULL == ptr || MAX_P_UTILS_LIST >= wMaxRDV)
    {
        return E_UTILS_LIST_RESULT_ERROR_NOK;
    }
    if (NULL != ptr->list)
    {
        return E_UTILS_LIST_RESULT_ERROR_ALREADY_INIT;
    }

    ptr->list = (tUtilsListElt*) pvPortMalloc(sizeof(tUtilsListElt) * wMaxRDV);
    if (NULL == ptr->list)
    {
        return E_UTILS_LIST_RESULT_ERROR_NOK;
    }

    memset(ptr->list, 0, sizeof(tUtilsListElt) * wMaxRDV);
#ifdef FOLLOW_ALLOC
    incrementCpt();
#endif
    ptr->wMaxWaitingTasks = wMaxRDV;
    ptr->firstValid = UINT16_MAX;
    ptr->firstFreeNextOQP = UINT16_MAX;
    ptr->firstFreePreviousOQP = UINT16_MAX;
    ptr->firstFree = 0;
    ptr->wNbRegisteredTasks = 0;
    ptr->lockHandle = NULL;

    return E_UTILS_LIST_RESULT_OK;
}

void P_UTILS_LIST_DeInit(tUtilsList* ptr)
{
    if (ptr != NULL)
    {
        if (ptr->list != NULL)
        {
            memset(ptr->list, 0, ptr->wMaxWaitingTasks * sizeof(tUtilsListElt));
            vPortFree(ptr->list);
            ptr->list = NULL;
#ifdef FOLLOW_ALLOC
            decrement();
#endif
        }
        ptr->wMaxWaitingTasks = 0;
        ptr->firstValid = UINT16_MAX;
        ptr->firstFreeNextOQP = UINT16_MAX;
        ptr->firstFreePreviousOQP = UINT16_MAX;
        ptr->firstFree = 0;
        ptr->wNbRegisteredTasks = 0;
    }
}

eUtilsListResult P_UTILS_LIST_AddElt(tUtilsList* ptr,
                                     TaskHandle_t handleTask,
                                     void* pContext,
                                     uint32_t infos,
                                     uint32_t infos2)
{
    uint16_t wCurrentSlotId = 0;
    uint16_t firstPrevOQP = 0;
    uint16_t firstNextOQP = 0;

    if (NULL == ptr || NULL == handleTask || NULL == ptr->list)
    {
        return E_UTILS_LIST_RESULT_ERROR_NOK;
    }

    wCurrentSlotId = ptr->firstFree;

    // Check if it's a valid slot
    if (ptr->wNbRegisteredTasks >= ptr->wMaxWaitingTasks || wCurrentSlotId >= ptr->wMaxWaitingTasks)
    {
        ptr->firstFree = UINT16_MAX;
        ptr->firstFreeNextOQP = UINT16_MAX;
        ptr->firstFreePreviousOQP = UINT16_MAX;
        return E_UTILS_LIST_RESULT_ERROR_MAX_ELTS;
    }

    ptr->list[wCurrentSlotId].prId = ptr->firstFreePreviousOQP;
    ptr->list[wCurrentSlotId].nxId = ptr->firstFreeNextOQP;
    firstPrevOQP = ptr->firstFreePreviousOQP;
    firstNextOQP = ptr->firstFreeNextOQP;

    if (firstPrevOQP >= ptr->wMaxWaitingTasks)
    {
        ptr->firstValid = wCurrentSlotId;
    }

    // Slot free before this slot, next not changed, this previous becomes the next free slot
    if ((wCurrentSlotId > 0) && ((firstPrevOQP >= ptr->wMaxWaitingTasks) || (firstPrevOQP < (wCurrentSlotId - 1))))
    {
        ptr->firstFree = wCurrentSlotId - 1;
        ptr->firstFreeNextOQP = wCurrentSlotId;
    }
    // Slot free after this slot, this next becomes the next free slot
    else if (((wCurrentSlotId + 1) < ptr->wMaxWaitingTasks) &&
             ((firstNextOQP >= ptr->wMaxWaitingTasks) || (firstNextOQP > (wCurrentSlotId + 1))))
    {
        ptr->firstFree = wCurrentSlotId + 1;
        ptr->firstFreePreviousOQP = wCurrentSlotId;
    }

    // Update previous and next slot indexation
    if (firstNextOQP < UINT16_MAX)
    {
        ptr->list[firstNextOQP].prId = wCurrentSlotId;
    }
    if (firstPrevOQP < UINT16_MAX)
    {
        ptr->list[firstPrevOQP].nxId = wCurrentSlotId;
    }

    ptr->list[wCurrentSlotId].value = handleTask;
    ptr->list[wCurrentSlotId].infosField1 = infos;
    ptr->list[wCurrentSlotId].infosField2 = infos2;
    ptr->list[wCurrentSlotId].pContext = pContext;
    ptr->wNbRegisteredTasks =
        ptr->wNbRegisteredTasks < ptr->wMaxWaitingTasks ? ptr->wNbRegisteredTasks + 1 : ptr->wNbRegisteredTasks;

    // If slot between 2 full slots, search for an empty slot in the whole list
    if (ptr->firstFree == wCurrentSlotId)
    {
        bool bSlotFound = false;
        wCurrentSlotId = 0;
        while ((wCurrentSlotId < ptr->wMaxWaitingTasks) && (bSlotFound == 0))
        {
            if (ptr->list[wCurrentSlotId].value == 0)
            {
                // Slot found
                bSlotFound = true;
            }
            else
            {
                // Slot not found
                wCurrentSlotId++;
            }
        }

        if (bSlotFound)
        {
            ptr->firstFree = wCurrentSlotId;
            ptr->firstFreePreviousOQP = wCurrentSlotId > 0 ? wCurrentSlotId - 1 : UINT16_MAX;
            bSlotFound = false;
            // Then search for the new firstNextOQP
            wCurrentSlotId++;
            while ((wCurrentSlotId < ptr->wMaxWaitingTasks) && (bSlotFound == 0))
            {
                if ((ptr->list[wCurrentSlotId].value) != 0)
                {
                    // Slot found
                    bSlotFound = true;
                }
                else
                {
                    // Slot not found
                    wCurrentSlotId++;
                }
            }
            if (bSlotFound)
            {
                ptr->firstFreeNextOQP = wCurrentSlotId;
            }
            else
            {
                ptr->firstFreeNextOQP = UINT16_MAX;
            }
        }
        else
        {
            // No free slot was found
            ptr->firstFree = UINT16_MAX;
            ptr->firstFreeNextOQP = UINT16_MAX;
            ptr->firstFreePreviousOQP = UINT16_MAX;
        }
    }

    return E_UTILS_LIST_RESULT_OK;
}

uint16_t P_UTILS_LIST_RemoveElt(tUtilsList* pv, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2)
{
    uint16_t wCurrentSlotId = UINT16_MAX;
    if ((pv != NULL) && (pv->list != NULL) && (taskNotified != NULL) && (pv->wNbRegisteredTasks > 0))
    {
        // Search a task with handle and signal requested
        wCurrentSlotId = P_UTILS_LIST_GetEltIndex(pv, taskNotified, infos1, infos2);
        // If found, -1 waiters, update list.
        if (wCurrentSlotId < pv->wMaxWaitingTasks)
        {
            // Safe decrement
            pv->wNbRegisteredTasks = pv->wNbRegisteredTasks > 0 ? pv->wNbRegisteredTasks - 1 : pv->wNbRegisteredTasks;

            // First slot removed, so index first on next
            if (wCurrentSlotId == pv->firstValid)
            {
                pv->firstValid = pv->list[wCurrentSlotId].nxId;
            }
            // Next of current not the last, so previous of next = previous of current
            if (pv->list[wCurrentSlotId].nxId < pv->wMaxWaitingTasks)
            {
                pv->list[pv->list[wCurrentSlotId].nxId].prId = pv->list[wCurrentSlotId].prId;
                pv->firstFreeNextOQP = pv->list[wCurrentSlotId].nxId;
            }
            else
            {
                pv->firstFreeNextOQP = UINT16_MAX;
            }
            // previous of current not the last, so next of previous = next of current
            if (pv->list[wCurrentSlotId].prId < pv->wMaxWaitingTasks)
            {
                pv->list[pv->list[wCurrentSlotId].prId].nxId = pv->list[wCurrentSlotId].nxId;
                pv->firstFreePreviousOQP = pv->list[wCurrentSlotId].prId;
            }
            else
            {
                pv->firstFreePreviousOQP = UINT16_MAX;
            }

            // RAZ current
            pv->list[wCurrentSlotId].value = NULL;
            pv->list[wCurrentSlotId].infosField1 = 0;
            pv->list[wCurrentSlotId].infosField2 = 0;
            pv->list[wCurrentSlotId].pContext = NULL;
            pv->list[wCurrentSlotId].nxId = UINT16_MAX;
            pv->list[wCurrentSlotId].prId = UINT16_MAX;
            pv->firstFree = wCurrentSlotId;
        }
    }
    return wCurrentSlotId;
}

TaskHandle_t P_UTILS_LIST_ParseValueElt(tUtilsList* ptr,
                                        uint32_t* pOutValue,
                                        uint32_t* pOutValue2,
                                        void** ppOutContext,
                                        uint16_t* pCurrentSlotId)
{
    uint16_t wCurrentSlotId = UINT16_MAX;
    TaskHandle_t taskHandle = 0;
    if ((ptr != NULL) && (ptr->list != NULL) && (pCurrentSlotId != NULL))
    {
        wCurrentSlotId = *pCurrentSlotId;
        if (wCurrentSlotId == UINT16_MAX)
        {
            wCurrentSlotId = ptr->firstValid;
        }

        if (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            taskHandle = ptr->list[wCurrentSlotId].value;
            if (pOutValue != NULL)
            {
                *pOutValue = ptr->list[wCurrentSlotId].infosField1;
            }
            if (pOutValue2 != NULL)
            {
                *pOutValue2 = ptr->list[wCurrentSlotId].infosField2;
            }
            if (ppOutContext != NULL)
            {
                *ppOutContext = ptr->list[wCurrentSlotId].pContext;
            }
            wCurrentSlotId = ptr->list[wCurrentSlotId].nxId;
        }
        else
        {
            taskHandle = 0;
            wCurrentSlotId = UINT16_MAX;
        }
    }

    *pCurrentSlotId = wCurrentSlotId;
    return taskHandle;
}

void* P_UTILS_LIST_ParseContextElt(tUtilsList* ptr, uint16_t* pCurrentSlotId)
{
    uint16_t wCurrentSlotId = UINT16_MAX;
    void* pContext = 0;
    if ((ptr != NULL) && (ptr->list != NULL) && (pCurrentSlotId != NULL))
    {
        wCurrentSlotId = *pCurrentSlotId;
        if (wCurrentSlotId == UINT16_MAX)
        {
            wCurrentSlotId = ptr->firstValid;
        }

        if (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            pContext = ptr->list[wCurrentSlotId].pContext;
            wCurrentSlotId = ptr->list[wCurrentSlotId].nxId;
        }
        else
        {
            wCurrentSlotId = UINT16_MAX;
            pContext = 0;
        }
    }

    *pCurrentSlotId = wCurrentSlotId;
    return pContext;
}

uint16_t P_UTILS_LIST_GetEltIndex(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2)
{
    uint16_t wCurrentSlotId = UINT16_MAX;
    if ((ptr != NULL) && (ptr->list != NULL) && (taskNotified != NULL))
    {
        wCurrentSlotId = ptr->firstValid;
        while (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            if ((ptr->list[wCurrentSlotId].value == taskNotified) &&
                (ptr->list[wCurrentSlotId].infosField1 == infos1) && (ptr->list[wCurrentSlotId].infosField2 == infos2))
            {
                break;
            }
            else
            {
                wCurrentSlotId = ptr->list[wCurrentSlotId].nxId;
            }
        }
    }

    if (wCurrentSlotId >= ptr->wMaxWaitingTasks)
    {
        wCurrentSlotId = UINT16_MAX;
    }
    return wCurrentSlotId;
}

eUtilsListResult P_UTILS_LIST_InitMT(tUtilsList* ptr, uint16_t wMaxRDV)
{
    eUtilsListResult result = E_UTILS_LIST_RESULT_ERROR_NOK;

    if ((ptr != NULL) && (ptr->lockHandle == NULL))
    {
        result = P_UTILS_LIST_Init(ptr, wMaxRDV);
        if (result == E_UTILS_LIST_RESULT_OK)
        {
            ptr->lockHandle = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
            if (ptr->lockHandle == NULL)
            {
                P_UTILS_LIST_DeInit(ptr);
            }
            else
            {
#ifdef FOLLOW_ALLOC
                incrementCpt();
#endif
            }
        }
    }
    return result;
}

void P_UTILS_LIST_DeInitMT(tUtilsList* ptr)
{
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        {
            P_UTILS_LIST_DeInit(ptr);
        }
        vQueueDelete(ptr->lockHandle);
        ptr->lockHandle = NULL;
#ifdef FOLLOW_ALLOC
        decrementCpt();
#endif
    }
}

uint16_t P_UTILS_LIST_RemoveEltMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2)
{
    uint16_t wCurrentSlotId = UINT16_MAX;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        wCurrentSlotId = P_UTILS_LIST_RemoveElt(ptr, taskNotified, infos1, infos2);
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return wCurrentSlotId;
}

eUtilsListResult P_UTILS_LIST_AddEltMT(tUtilsList* ptr,
                                       TaskHandle_t handleTask,
                                       void* pContext,
                                       uint32_t infos1,
                                       uint32_t infos2)
{
    eUtilsListResult result = E_UTILS_LIST_RESULT_ERROR_NOK;

    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        result = P_UTILS_LIST_AddElt(ptr, handleTask, pContext, infos1, infos2);
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return result;
}

TaskHandle_t P_UTILS_LIST_ParseValueEltMT(tUtilsList* ptr,
                                          uint32_t* pOutValue,
                                          uint32_t* pOutValue2,
                                          void** ppContext,
                                          uint16_t* pCurrentSlotId)
{
    TaskHandle_t taskHandle = 0;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        {
            taskHandle = P_UTILS_LIST_ParseValueElt(ptr, pOutValue, pOutValue2, ppContext, pCurrentSlotId);
        }
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return taskHandle;
}

void* P_UTILS_LIST_ParseContextEltMT(tUtilsList* ptr, uint16_t* pCurrentSlotId)
{
    void* pContext = NULL;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        {
            pContext = P_UTILS_LIST_ParseContextElt(ptr, pCurrentSlotId);
        }
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return pContext;
}

uint16_t P_UTILS_LIST_GetEltIndexMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2)
{
    uint16_t wCurrentSlotId = UINT16_MAX;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        wCurrentSlotId = P_UTILS_LIST_GetEltIndex(ptr, taskNotified, infos1, infos2);
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return wCurrentSlotId;
}

void* P_UTILS_LIST_GetContextFromHandleMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2)
{
    void* ptrContext = NULL;
    uint16_t wCurrentSlotId = UINT16_MAX;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        wCurrentSlotId = P_UTILS_LIST_GetEltIndex(ptr, taskNotified, infos1, infos2);
        if (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            ptrContext = ptr->list[wCurrentSlotId].pContext;
        }
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return ptrContext;
}

#ifdef FOLLOW_ALLOC
uint32_t cptAlloc = 0;
uint32_t cptFree = 0;
QueueHandle_t cptMutex = NULL;

void incrementCpt(void)
{
    if (cptMutex == NULL)
    {
        cptMutex = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
    }
    xSemaphoreTakeRecursive(cptMutex, portMAX_DELAY);
    cptAlloc++;
    xSemaphoreGiveRecursive(cptMutex);
}
void decrementCpt(void)
{
    if (cptMutex == NULL)
    {
        cptMutex = xQueueCreateMutex(queueQUEUE_TYPE_RECURSIVE_MUTEX);
    }
    xSemaphoreTakeRecursive(cptMutex, portMAX_DELAY);
    cptFree++;
    xSemaphoreGiveRecursive(cptMutex);
}
#endif
