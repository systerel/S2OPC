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

#include <assert.h>

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_event_handler.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

static void* POISON_PILL = (void*) 0x01;

struct _SOPC_EventHandler
{
    SOPC_Looper* looper;
    SOPC_EventHandler_Callback* callback;
};

struct _SOPC_Looper
{
    Thread thread;
    SOPC_AsyncQueue* queue;
    SOPC_Array* handlers;
};

struct Event
{
    SOPC_EventHandler* handler;
    int32_t code;
    uint32_t id;
    uintptr_t params;
    uintptr_t auxParam;
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

    SOPC_ReturnStatus status = asNext ? SOPC_AsyncQueue_BlockingEnqueueFirstOut(handler->looper->queue, ev)
                                      : SOPC_AsyncQueue_BlockingEnqueue(handler->looper->queue, ev);

    if (status != SOPC_STATUS_OK)
    {
        SOPC_Free(ev);
    }

    return status;
}

static void* looper_loop(void* user_data)
{
    SOPC_AsyncQueue* queue = user_data;

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
        ev->handler->callback(ev->handler, ev->code, ev->id, ev->params, ev->auxParam);
        SOPC_Free(ev);
    }

    return NULL;
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

SOPC_Looper* SOPC_Looper_Create(const char* threadName)
{
    SOPC_Looper* looper = SOPC_Calloc(1, sizeof(SOPC_Looper));
    SOPC_AsyncQueue* queue = NULL;
    SOPC_Array* handlers =
        SOPC_Array_Create(sizeof(SOPC_EventHandler*), 0, (SOPC_Array_Free_Func*) event_handler_delete);

    if (looper == NULL || handlers == NULL || SOPC_AsyncQueue_Init(&queue, threadName) != SOPC_STATUS_OK ||
        SOPC_Thread_Create(&looper->thread, looper_loop, queue, threadName) != SOPC_STATUS_OK)
    {
        SOPC_AsyncQueue_Free(&queue);
        SOPC_Array_Delete(handlers);
        SOPC_Free(looper);
        return NULL;
    }

    looper->queue = queue;
    looper->handlers = handlers;

    return looper;
}

void SOPC_Looper_Delete(SOPC_Looper* looper)
{
    if (looper == NULL)
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_AsyncQueue_BlockingEnqueue(looper->queue, POISON_PILL);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_Thread_Join(looper->thread);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    SOPC_AsyncQueue_Free(&looper->queue);
    SOPC_Array_Delete(looper->handlers);
    SOPC_Free(looper);
}
