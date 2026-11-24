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

#include "sopc_event_handler.h"

#include <inttypes.h>
#include <stdbool.h>

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

static void* POISON_PILL = (void*) 0x01;

struct _SOPC_EventHandler
{
    SOPC_Looper* looper;
    SOPC_EventHandler_Callback* callback;
    SOPC_TimeReference currentEventPostTime; // Post time of the event being processed (only accessed by looper thread)
    SOPC_EventHandler_CategoryFct* categoryFct;   /* NULL if the posted events are not counted */
    SOPC_EventHandler_FreeEventFct* freeEventFct; /* NULL if the untreated events parameters are not freed */
};

struct _SOPC_Looper
{
    const char* name;
    SOPC_Thread thread;
    bool stopped; /* Thread joined: only accessed by the thread calling SOPC_Looper_Stop/Delete */
    SOPC_AsyncQueue* queue;
    SOPC_Array* handlers;
    /* Pending events per category (atomic): incremented before enqueue, decremented once dequeued */
    int32_t pendingByCategory[SOPC_EVENT_HANDLER_NB_CATEGORIES];
};

struct Event
{
    SOPC_EventHandler* handler;
    int32_t code;
    uint32_t id;
    uintptr_t params;
    uintptr_t auxParam;
    SOPC_TimeReference postTime;
    int32_t category; /* SOPC_EVENT_HANDLER_NO_CATEGORY if not counted */
};

static void event_handler_delete(SOPC_EventHandler** handler)
{
    SOPC_Free(*handler);
}

static SOPC_ReturnStatus post(SOPC_EventHandler* handler,
                              int32_t event,
                              uint32_t eltId,
                              uintptr_t params,
                              uintptr_t auxParam,
                              bool asNext)
{
    if (NULL == handler)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    struct Event* ev = SOPC_Calloc(1, sizeof(struct Event));

    if (ev == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    ev->handler = handler;
    ev->code = event;
    ev->id = eltId;
    ev->params = params;
    ev->auxParam = auxParam;
    ev->postTime = SOPC_TimeReference_GetCurrent();
    ev->category = SOPC_EVENT_HANDLER_NO_CATEGORY;
    if (NULL != handler->categoryFct)
    {
        ev->category = handler->categoryFct(event, eltId, params, auxParam);
        SOPC_ASSERT(SOPC_EVENT_HANDLER_NO_CATEGORY == ev->category ||
                    (ev->category >= 0 && ev->category < SOPC_EVENT_HANDLER_NB_CATEGORIES));
    }
    // Counted before the enqueue: the counter never becomes negative once the event is dequeued
    if (SOPC_EVENT_HANDLER_NO_CATEGORY != ev->category)
    {
        SOPC_Atomic_Int_Add(&handler->looper->pendingByCategory[ev->category], 1);
    }

    SOPC_ReturnStatus status = asNext ? SOPC_AsyncQueue_BlockingEnqueueFirstOut(handler->looper->queue, ev)
                                      : SOPC_AsyncQueue_BlockingEnqueue(handler->looper->queue, ev);

    if (status != SOPC_STATUS_OK)
    {
        if (SOPC_EVENT_HANDLER_NO_CATEGORY != ev->category)
        {
            SOPC_Atomic_Int_Add(&handler->looper->pendingByCategory[ev->category], -1);
        }
        SOPC_Free(ev);
    }

    return status;
}

static void* looper_loop(void* user_data)
{
    SOPC_Looper* looper = user_data;
    SOPC_AsyncQueue* queue = looper->queue;

    while (true)
    {
        void* item = NULL;
        SOPC_ReturnStatus status = SOPC_AsyncQueue_BlockingDequeue(queue, (void**) &item);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        if (item == POISON_PILL)
        {
            return NULL;
        }

        struct Event* ev = item;
        ev->handler->currentEventPostTime = ev->postTime;
        // No longer pending once dequeued: the treatment of the event sees the remaining pending events
        if (SOPC_EVENT_HANDLER_NO_CATEGORY != ev->category)
        {
            SOPC_Atomic_Int_Add(&looper->pendingByCategory[ev->category], -1);
        }
        ev->handler->callback(ev->handler, ev->code, ev->id, ev->params, ev->auxParam);
        ev->handler->currentEventPostTime = 0;
        SOPC_Free(ev);
    }

    return NULL;
}

SOPC_EventHandler* SOPC_EventHandler_CreateWithCategory(SOPC_Looper* looper,
                                                        SOPC_EventHandler_Callback* callback,
                                                        SOPC_EventHandler_CategoryFct* categoryFct)
{
    if (NULL == categoryFct)
    {
        return NULL;
    }
    SOPC_EventHandler* handler = SOPC_EventHandler_Create(looper, callback);
    if (NULL != handler)
    {
        handler->categoryFct = categoryFct;
    }
    return handler;
}

uint32_t SOPC_EventHandler_Get_PendingCountByCategory(SOPC_EventHandler* handler, int32_t category)
{
    if (NULL == handler || category < 0 || category >= SOPC_EVENT_HANDLER_NB_CATEGORIES)
    {
        return 0;
    }
    const int32_t count = SOPC_Atomic_Int_Get(&handler->looper->pendingByCategory[category]);
    return (count > 0) ? (uint32_t) count : 0;
}

void SOPC_EventHandler_SetFreeEventFct(SOPC_EventHandler* handler, SOPC_EventHandler_FreeEventFct* freeFct)
{
    if (NULL != handler)
    {
        handler->freeEventFct = freeFct;
    }
}

SOPC_EventHandler* SOPC_EventHandler_Create(SOPC_Looper* looper, SOPC_EventHandler_Callback* callback)
{
    SOPC_EventHandler* handler = SOPC_Calloc(1, sizeof(SOPC_EventHandler));

    if (handler == NULL)
    {
        return NULL;
    }

    handler->looper = looper;
    handler->callback = callback;

    if (!SOPC_Array_Append(looper->handlers, handler))
    {
        event_handler_delete(&handler);
        return NULL;
    }

    return handler;
}

uint32_t SOPC_EventHandler_Get_QueueSize(SOPC_EventHandler* handler)
{
    return (NULL == handler ? 0 : SOPC_AsyncQueue_GetSize(handler->looper->queue));
}

SOPC_TimeReference SOPC_EventHandler_GetCurrentEventPostTime(const SOPC_EventHandler* handler)
{
    return (NULL == handler ? 0 : handler->currentEventPostTime);
}

SOPC_ReturnStatus SOPC_EventHandler_Post(SOPC_EventHandler* handler,
                                         int32_t event,
                                         uint32_t eltId,
                                         uintptr_t params,
                                         uintptr_t auxParam)
{
    return post(handler, event, eltId, params, auxParam, false);
}

SOPC_ReturnStatus SOPC_EventHandler_PostAsNext(SOPC_EventHandler* handler,
                                               int32_t event,
                                               uint32_t eltId,
                                               uintptr_t params,
                                               uintptr_t auxParam)
{
    return post(handler, event, eltId, params, auxParam, true);
}

SOPC_Looper* SOPC_Looper_CreatePrioritized(const char* threadName, int threadPriority, int cpuAffinity)
{
    SOPC_Looper* looper = SOPC_Calloc(1, sizeof(SOPC_Looper));
    SOPC_AsyncQueue* queue = NULL;
    SOPC_Array* handlers =
        SOPC_Array_Create(sizeof(SOPC_EventHandler*), 0, (SOPC_Array_Free_Func*) event_handler_delete);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == looper || NULL == handlers)
    {
        status = SOPC_STATUS_NOK;
    }
    else
    {
        status = SOPC_AsyncQueue_Init(&queue, threadName);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set before the thread creation: the looper thread uses its queue and pending counters
        looper->name = threadName;
        looper->queue = queue;
        looper->handlers = handlers;
        status = SOPC_Thread_CreatePrioritized(&looper->thread, looper_loop, looper, threadPriority, cpuAffinity,
                                               threadName);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_AsyncQueue_Free(&queue);
        SOPC_Array_Delete(handlers);
        SOPC_Free(looper);
        return NULL;
    }

    return looper;
}

SOPC_Looper* SOPC_Looper_Create(const char* threadName)
{
    return SOPC_Looper_CreatePrioritized(threadName, 0, -1);
}

void SOPC_Looper_Stop(SOPC_Looper* looper)
{
    if (NULL == looper || looper->stopped)
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_AsyncQueue_BlockingEnqueue(looper->queue, POISON_PILL);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_Thread_Join(&looper->thread);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    looper->stopped = true;
}

void SOPC_Looper_Delete(SOPC_Looper* looper)
{
    if (looper == NULL)
    {
        return;
    }

    SOPC_Looper_Stop(looper);

    // The poison pill has been dequeued: remaining events were posted after it, handlers are still allocated
    uint32_t nbUntreated = 0;
    struct Event* ev = NULL;
    while (SOPC_STATUS_OK == SOPC_AsyncQueue_NonBlockingDequeue(looper->queue, (void**) &ev) && NULL != ev)
    {
        if (NULL != ev->handler->freeEventFct)
        {
            ev->handler->freeEventFct(ev->code, ev->id, ev->params, ev->auxParam);
        }
        SOPC_Free(ev);
        ev = NULL;
        nbUntreated++;
    }
    if (nbUntreated > 0)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON, "Looper %s deleted with %" PRIu32 " untreated event(s)",
                                 looper->name, nbUntreated);
    }

    SOPC_AsyncQueue_Free(&looper->queue);
    SOPC_Array_Delete(looper->handlers);
    SOPC_Free(looper);
}
