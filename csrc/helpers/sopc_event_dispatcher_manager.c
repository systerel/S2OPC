/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_event_dispatcher_manager.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_threads.h"

struct SOPC_EventDispatcherManager
{
    SOPC_AsyncQueue* queue;
    SOPC_EventDispatcherFct* pDispatcherFct;
    bool* stopMgr;
    Thread mgrThread;
};

static void* SOPC_ThreadStartEventDispatcherManager(void* pEventMgr)
{
    assert(NULL != pEventMgr);
    SOPC_EventDispatcherManager* pMgr = (SOPC_EventDispatcherManager*) pEventMgr;
    bool localStopMgr = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_EventDispatcherParams* pParams = NULL;
    void* pAnonParam = NULL;
    while (false == localStopMgr)
    {
        status = SOPC_AsyncQueue_BlockingDequeue(pMgr->queue, &pAnonParam);
        if (SOPC_STATUS_OK == status)
        {
            if (pAnonParam != NULL)
            {
                if (pAnonParam == pMgr->stopMgr)
                { // It is the stop flag address
                    assert(*pMgr->stopMgr != false);
                    localStopMgr = true;
                    free(pMgr->stopMgr);
                    pMgr->stopMgr = NULL;
                }
                else
                { // Nominal case
                    pParams = (SOPC_EventDispatcherParams*) pAnonParam;
                    pMgr->pDispatcherFct(pParams->event, pParams->eltId, pParams->params, pParams->auxParam);
                    free(pParams);
                }
            }
        }
    }
    return NULL;
}

SOPC_EventDispatcherManager* SOPC_EventDispatcherManager_CreateAndStart(SOPC_EventDispatcherFct fctPointer,
                                                                        const char* name)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_EventDispatcherManager* pEventMgr = NULL;
    pEventMgr = calloc(1, sizeof(SOPC_EventDispatcherManager));
    if (NULL != pEventMgr)
    {
        pEventMgr->stopMgr = calloc(1, sizeof(bool));
        if (NULL != pEventMgr->stopMgr)
        {
            status = SOPC_AsyncQueue_Init(&pEventMgr->queue, name);
        }
        if (SOPC_STATUS_OK == status)
        {
            pEventMgr->pDispatcherFct = fctPointer;
            *pEventMgr->stopMgr = false;
            status =
                SOPC_Thread_Create(&pEventMgr->mgrThread, SOPC_ThreadStartEventDispatcherManager, (void*) pEventMgr);
        }
        if (SOPC_STATUS_OK != status)
        {
            if (NULL != pEventMgr->stopMgr)
            {
                free(pEventMgr->stopMgr);
            }
            free(pEventMgr);
            pEventMgr = NULL;
        }
    }
    return pEventMgr;
}

static SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEventInternal(SOPC_EventDispatcherManager* eventMgr,
                                                                      int32_t event,
                                                                      uint32_t eltId,
                                                                      void* params,
                                                                      uint32_t auxParam,
                                                                      const char* debugName,
                                                                      bool enqueueAsFirstOut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_EventDispatcherParams* pParams = NULL;
    if (NULL != eventMgr)
    {
        status = SOPC_STATUS_INVALID_STATE;
        if (false == *eventMgr->stopMgr)
        {
            pParams = calloc(1, sizeof(SOPC_EventDispatcherParams));
            if (NULL != pParams)
            {
                pParams->event = event;
                pParams->eltId = eltId;
                pParams->params = params;
                pParams->auxParam = auxParam;
                pParams->debugName = debugName;
                if (false == enqueueAsFirstOut)
                {
                    // Nominal case
                    status = SOPC_AsyncQueue_BlockingEnqueue(eventMgr->queue, pParams);
                }
                else
                {
                    status = SOPC_AsyncQueue_BlockingEnqueueFirstOut(eventMgr->queue, pParams);
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEvent(SOPC_EventDispatcherManager* eventMgr,
                                                       int32_t event,
                                                       uint32_t eltId,
                                                       void* params,
                                                       uint32_t auxParam,
                                                       const char* debugName)
{
    return SOPC_EventDispatcherManager_AddEventInternal(eventMgr, event, eltId, params, auxParam, debugName, false);
}

SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEventAsNext(SOPC_EventDispatcherManager* eventMgr,
                                                             int32_t event,
                                                             uint32_t eltId,
                                                             void* params,
                                                             uint32_t auxParam,
                                                             const char* debugName)
{
    return SOPC_EventDispatcherManager_AddEventInternal(eventMgr, event, eltId, params, auxParam, debugName, true);
}

SOPC_ReturnStatus SOPC_EventDispatcherManager_StopAndDelete(SOPC_EventDispatcherManager** eventMgr)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != eventMgr && NULL != *eventMgr)
    {
        status = SOPC_STATUS_INVALID_STATE;
        if (false == *(*eventMgr)->stopMgr)
        {
            *(*eventMgr)->stopMgr = true;
            // Use stopMgr flag address as indicator all precedent actions were treated
            status = SOPC_AsyncQueue_BlockingEnqueue((*eventMgr)->queue, (*eventMgr)->stopMgr);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Thread_Join((*eventMgr)->mgrThread);
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_AsyncQueue_Free(&(*eventMgr)->queue);
            free(*eventMgr);
            *eventMgr = NULL;
        }
    }
    return status;
}
