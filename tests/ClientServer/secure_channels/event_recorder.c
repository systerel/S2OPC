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

#include "event_recorder.h"

#include <assert.h>

#include "sopc_dict.h"
#include "sopc_mem_alloc.h"

static SOPC_Dict* queues = NULL;

static uint64_t direct_hash(const void* data)
{
    return (uint64_t)(uintptr_t) data;
}

static bool direct_equal(const void* a, const void* b)
{
    return a == b;
}

static bool init_queues(void)
{
    if (queues != NULL)
    {
        return true;
    }

    queues = SOPC_Dict_Create(NULL, direct_hash, direct_equal, NULL, NULL);
    return queues != NULL;
}

static void on_event(SOPC_EventHandler* handler, int32_t event, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    bool found;
    SOPC_AsyncQueue* queue = SOPC_Dict_Get(queues, handler, &found);
    assert(found);
    SOPC_ReturnStatus status;

    SOPC_Event* ev = SOPC_Calloc(1, sizeof(SOPC_Event));
    assert(ev != NULL);

    ev->event = event;
    ev->eltId = id;
    ev->params = params;
    ev->auxParam = auxParam;

    status = SOPC_AsyncQueue_BlockingEnqueue(queue, ev);
    assert(SOPC_STATUS_OK == status);
}

static void queue_free(SOPC_AsyncQueue* queue)
{
    if (queue != NULL)
    {
        SOPC_AsyncQueue_Free(&queue);
    }
}

SOPC_EventRecorder* SOPC_EventRecorder_Create(void)
{
    if (!init_queues())
    {
        return NULL;
    }

    SOPC_Looper* looper = SOPC_Looper_Create("Test_SC_Threads");

    if (looper == NULL)
    {
        return NULL;
    }

    SOPC_EventHandler* handler = SOPC_EventHandler_Create(looper, on_event);
    SOPC_EventRecorder* recorder = SOPC_Calloc(1, sizeof(SOPC_EventRecorder));
    SOPC_AsyncQueue* queue;
    SOPC_AsyncQueue_Init(&queue, NULL);

    if (handler == NULL || recorder == NULL || queue == NULL || !SOPC_Dict_Insert(queues, handler, queue))
    {
        SOPC_Looper_Delete(looper);
        SOPC_Free(recorder);
        queue_free(queue);
        return NULL;
    }

    recorder->looper = looper;
    recorder->eventHandler = handler;
    recorder->events = queue;

    return recorder;
}

void SOPC_EventRecorder_Delete(SOPC_EventRecorder* recorder)
{
    if (recorder == NULL)
    {
        return;
    }

    SOPC_Looper_Delete(recorder->looper);
    queue_free(recorder->events);
    SOPC_Free(recorder);
}
