/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "sopc_async_queue.h"
#include "sopc_atomic.h"
#include "sopc_threads.h"

struct SOPC_EventDispatcherManager
{
    SOPC_AsyncQueue* queue;
    SOPC_EventDispatcherFct* pDispatcherFct;
    int32_t stop;
    Thread mgrThread;
};

static void* POISON_PILL = (void*) 0x01;

static void* SOPC_ThreadStartEventDispatcherManager(void* pEventMgr)
{
    assert(NULL != pEventMgr);
    SOPC_EventDispatcherManager* pMgr = (SOPC_EventDispatcherManager*) pEventMgr;

    while (true)
    {
        void* pAnonParam = NULL;
        SOPC_ReturnStatus status = SOPC_AsyncQueue_BlockingDequeue(pMgr->queue, &pAnonParam);

        if (status != SOPC_STATUS_OK || pAnonParam == NULL)
        {
            continue;
        }

        if (pAnonParam == POISON_PILL)
        {
            break;
        }

        SOPC_EventDispatcherParams* pParams = (SOPC_EventDispatcherParams*) pAnonParam;
        pMgr->pDispatcherFct(pParams->event, pParams->eltId, pParams->params, pParams->auxParam);
        free(pParams);
    }

    return NULL;
}

SOPC_EventDispatcherManager* SOPC_EventDispatcherManager_CreateAndStart(SOPC_EventDispatcherFct fctPointer,
                                                                        const char* name)
{
    SOPC_EventDispatcherManager* pEventMgr = calloc(1, sizeof(SOPC_EventDispatcherManager));

    if (pEventMgr == NULL)
    {
        return NULL;
    }

    pEventMgr->stop = 0;
    pEventMgr->pDispatcherFct = fctPointer;

    if (SOPC_AsyncQueue_Init(&pEventMgr->queue, name) != SOPC_STATUS_OK ||
        SOPC_Thread_Create(&pEventMgr->mgrThread, SOPC_ThreadStartEventDispatcherManager, (void*) pEventMgr) !=
            SOPC_STATUS_OK)
    {
        SOPC_AsyncQueue_Free(&pEventMgr->queue);
        free(pEventMgr);
        return NULL;
    }

    return pEventMgr;
}

static SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEventInternal(SOPC_EventDispatcherManager* eventMgr,
                                                                      int32_t event,
                                                                      uint32_t eltId,
                                                                      void* params,
                                                                      uintptr_t auxParam,
                                                                      const char* debugName,
                                                                      bool enqueueAsFirstOut)
{
    if (eventMgr == NULL || SOPC_Atomic_Int_Get(&eventMgr->stop) != 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_EventDispatcherParams* pParams = calloc(1, sizeof(SOPC_EventDispatcherParams));

    if (pParams == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    pParams->event = event;
    pParams->eltId = eltId;
    pParams->params = params;
    pParams->auxParam = auxParam;
    pParams->debugName = debugName;

    if (false == enqueueAsFirstOut)
    {
        return SOPC_AsyncQueue_BlockingEnqueue(eventMgr->queue, pParams);
    }
    else
    {
        return SOPC_AsyncQueue_BlockingEnqueueFirstOut(eventMgr->queue, pParams);
    }
}

SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEvent(SOPC_EventDispatcherManager* eventMgr,
                                                       int32_t event,
                                                       uint32_t eltId,
                                                       void* params,
                                                       uintptr_t auxParam,
                                                       const char* debugName)
{
    return SOPC_EventDispatcherManager_AddEventInternal(eventMgr, event, eltId, params, auxParam, debugName, false);
}

SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEventAsNext(SOPC_EventDispatcherManager* eventMgr,
                                                             int32_t event,
                                                             uint32_t eltId,
                                                             void* params,
                                                             uintptr_t auxParam,
                                                             const char* debugName)
{
    return SOPC_EventDispatcherManager_AddEventInternal(eventMgr, event, eltId, params, auxParam, debugName, true);
}

SOPC_ReturnStatus SOPC_EventDispatcherManager_StopAndDelete(SOPC_EventDispatcherManager** eventMgr)
{
    if (eventMgr == NULL || *eventMgr == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Atomic_Int_Set(&(*eventMgr)->stop, 1);

    if (SOPC_AsyncQueue_BlockingEnqueue((*eventMgr)->queue, POISON_PILL) != SOPC_STATUS_OK ||
        SOPC_Thread_Join((*eventMgr)->mgrThread) != SOPC_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_AsyncQueue_Free(&(*eventMgr)->queue);
    free(*eventMgr);
    *eventMgr = NULL;

    return SOPC_STATUS_OK;
}
