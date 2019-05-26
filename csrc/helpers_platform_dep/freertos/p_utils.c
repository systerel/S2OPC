#include <limits.h>
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

eUtilsListResult P_UTILS_LIST_AddElt(tUtilsList* ptr, TaskHandle_t handleTask, void* pContext, unsigned int infos)
{
    eUtilsListResult result = E_UTILS_LIST_RESULT_OK;
    unsigned short wCurrentSlotId = 0;
    unsigned short wNextSlotId = 0;

    if ((ptr != NULL) && (handleTask != NULL) && (ptr->list != NULL))
    {
        if (ptr->wNbRegisteredTasks < ptr->wMaxWaitingTasks) // Continue only if at least one waiter free
        {
            // Search free slot position
            while (wCurrentSlotId < ptr->wMaxWaitingTasks)
            {
                if (ptr->list[wCurrentSlotId].value == 0)
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
                    ptr->list[wCurrentSlotId - 1].nxId = wCurrentSlotId;
                    ptr->list[wCurrentSlotId].prId = wCurrentSlotId - 1;
                }
                else
                {
                    ptr->list[wCurrentSlotId].prId = ptr->wMaxWaitingTasks; // MAX_WAITERS = NO PREVIOUS
                    ptr->first = wCurrentSlotId;
                }

                // Search next slot if exist (not the max and current slot < total already registered slots)
                if ((wCurrentSlotId < (ptr->wMaxWaitingTasks - 1)) && (wCurrentSlotId < ptr->wNbRegisteredTasks))
                {
                    wNextSlotId = wCurrentSlotId + 1;

                    // Search the not free slot
                    while (wNextSlotId < ptr->wMaxWaitingTasks)
                    {
                        if (ptr->list[wNextSlotId].value != 0)
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
                        ptr->list[wNextSlotId].prId = wCurrentSlotId;
                        ptr->list[wCurrentSlotId].nxId = wNextSlotId;
                    }
                    else
                    {
                        // No next, next info set to MAX WAITERS
                        ptr->list[wCurrentSlotId].nxId = ptr->wMaxWaitingTasks;
                    }
                }
                else
                {
                    // No next, next info set to MAX WAITERS
                    ptr->list[wCurrentSlotId].nxId = ptr->wMaxWaitingTasks;
                }

                ptr->list[wCurrentSlotId].value = handleTask;
                ptr->list[wCurrentSlotId].infos = infos;
                ptr->list[wCurrentSlotId].pContext = pContext;
                ptr->wNbRegisteredTasks = ptr->wNbRegisteredTasks < ptr->wMaxWaitingTasks ? ptr->wNbRegisteredTasks + 1
                                                                                          : ptr->wNbRegisteredTasks;
            } // if(wCurrentSlotId  < MAX_SIGNAL) no free slot
            else
            {
                result = E_UTILS_LIST_RESULT_ERROR_MAX_ELTS;
            }
        } // pv->nbWaiters = pv->nbWaiters < MAX_SIGNAL => no free slot
        else
        {
            result = E_UTILS_LIST_RESULT_ERROR_MAX_ELTS;
        }
    }
    return result;
}

unsigned short int P_UTILS_LIST_GetEltIndex(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos)
{
    unsigned short wCurrentSlotId = USHRT_MAX;
    if ((ptr != NULL) && (ptr->list != NULL) && (taskNotified != NULL))
    {
        wCurrentSlotId = ptr->first;
        while (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            if ((ptr->list[wCurrentSlotId].value == taskNotified) && (ptr->list[wCurrentSlotId].infos == infos))
            {
                break;
            }
            else
            {
                wCurrentSlotId = ptr->list[wCurrentSlotId].nxId;
            }
        }
    }
    return wCurrentSlotId;
}

unsigned short int P_UTILS_LIST_RemoveElt(tUtilsList* pv, TaskHandle_t taskNotified, unsigned int infos)
{
    unsigned short wCurrentSlotId = USHRT_MAX;
    if ((pv != NULL) && (pv->list != NULL) && (taskNotified != NULL) && (pv->wNbRegisteredTasks > 0))
    {
        // Search a task with handle and signal requested
        wCurrentSlotId = P_UTILS_LIST_GetEltIndex(pv, taskNotified, infos);
        // If found, -1 waiters, update list.
        if (wCurrentSlotId < pv->wMaxWaitingTasks)
        {
            // Safe decrement
            pv->wNbRegisteredTasks = pv->wNbRegisteredTasks > 0 ? pv->wNbRegisteredTasks - 1 : pv->wNbRegisteredTasks;

            // First slot removed, so index first on next
            if (wCurrentSlotId == pv->first)
            {
                pv->first = pv->list[wCurrentSlotId].nxId;
            }
            // Next of current not the last, so previous of next = previous of current
            if (pv->list[wCurrentSlotId].nxId < pv->wMaxWaitingTasks)
            {
                pv->list[pv->list[wCurrentSlotId].nxId].prId = pv->list[wCurrentSlotId].prId;
            }
            // previous of current not the last, so next of previous = next of current
            if (pv->list[wCurrentSlotId].prId < pv->wMaxWaitingTasks)
            {
                pv->list[pv->list[wCurrentSlotId].prId].nxId = pv->list[wCurrentSlotId].nxId;
            }

            // RAZ current
            pv->list[wCurrentSlotId].value = 0;
            pv->list[wCurrentSlotId].infos = 0;
            pv->list[wCurrentSlotId].pContext = NULL;
            pv->list[wCurrentSlotId].nxId = pv->wMaxWaitingTasks;
            pv->list[wCurrentSlotId].prId = pv->wMaxWaitingTasks;
        }
    }
    return wCurrentSlotId;
}

/*Alloc task*/
eUtilsListResult P_UTILS_LIST_Init(tUtilsList* ptr, unsigned short int wMaxRDV)
{
    unsigned short iIter = 0;
    if ((ptr != NULL) && (wMaxRDV <= MAX_SIGNAL))
    {
        if (ptr->list == NULL)
        {
            ptr->list = (tUtilsListElt*) pvPortMalloc(sizeof(tUtilsListElt) * wMaxRDV);
            if (ptr->list != NULL)
            {
                ptr->wMaxWaitingTasks = wMaxRDV;
                ptr->first = ptr->wMaxWaitingTasks;
                ptr->wNbRegisteredTasks = 0;
                for (iIter = 0; iIter < ptr->wMaxWaitingTasks; iIter++)
                {
                    ptr->list[iIter].value = 0;
                    ptr->list[iIter].infos = 0;
                    ptr->list[iIter].pContext = NULL;
                    ptr->list[iIter].nxId = ptr->wMaxWaitingTasks;
                    ptr->list[iIter].prId = ptr->wMaxWaitingTasks;
                }
                return E_UTILS_LIST_RESULT_OK;
            }
        }
        else
        {
            return E_UTILS_LIST_RESULT_ERROR_ALREADY_INIT;
        }
    }
    return E_UTILS_LIST_RESULT_ERROR_NOK;
}

void P_UTILS_LIST_DeInit(tUtilsList* ptr)
{
    if (ptr != NULL)
    {
        if (ptr->list != NULL)
        {
            vPortFree(ptr->list);
            ptr->list = NULL;
        }
        ptr->wMaxWaitingTasks = 0;
        ptr->first = 0;
        ptr->wNbRegisteredTasks = 0;
    }
}

eUtilsListResult P_UTILS_LIST_AddEltMT(tUtilsList* ptr, TaskHandle_t handleTask, void* pContext, unsigned int infos)
{
    eUtilsListResult result = E_UTILS_LIST_RESULT_ERROR_NOK;

    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        result = P_UTILS_LIST_AddElt(ptr, handleTask, pContext, infos);
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return result;
}

unsigned short int P_UTILS_LIST_GetEltIndexMT(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos)
{
    unsigned short wCurrentSlotId = USHRT_MAX;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        wCurrentSlotId = P_UTILS_LIST_GetEltIndex(ptr, taskNotified, infos);
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return wCurrentSlotId;
}

void* P_UTILS_LIST_GetContextFromHandleMT(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos)
{
    void* ptrContext = NULL;
    unsigned short wCurrentSlotId = USHRT_MAX;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        wCurrentSlotId = P_UTILS_LIST_GetEltIndex(ptr, taskNotified, infos);
        if (wCurrentSlotId < ptr->wMaxWaitingTasks)
        {
            ptrContext = ptr->list[wCurrentSlotId].pContext;
        }
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return ptrContext;
}

unsigned short int P_UTILS_LIST_RemoveEltMT(tUtilsList* ptr, TaskHandle_t taskNotified, unsigned int infos)
{
    unsigned short wCurrentSlotId = USHRT_MAX;
    if ((ptr != NULL) && (ptr->lockHandle != NULL))
    {
        xSemaphoreTakeRecursive(ptr->lockHandle, portMAX_DELAY);
        wCurrentSlotId = P_UTILS_LIST_RemoveElt(ptr, taskNotified, infos);
        xSemaphoreGiveRecursive(ptr->lockHandle);
    }
    return wCurrentSlotId;
}
