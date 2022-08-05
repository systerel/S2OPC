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
#include <stddef.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_buffer.h"
#include "sopc_mem_alloc.h"
#include "sopc_raw_sockets.h"
#include "sopc_sockets_event_mgr.h"
#include "sopc_sockets_internal_ctx.h"

SOPC_Socket socketsArray[SOPC_MAX_SOCKETS];
Mutex socketsMutex;
SOPC_Looper* socketsLooper = NULL;
SOPC_AsyncQueue* socketsInputEventQueue = NULL;
SOPC_EventHandler* socketsEventHandler = NULL;
uint32_t maxBufferSize = SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE;

struct Event
{
    SOPC_Sockets_InputEvent event;
    uint32_t id;
    uintptr_t params;
    uintptr_t auxParam;
};

void SOPC_SocketsInternalContext_Initialize(void)
{
    uint32_t idx = 0;
    memset(socketsArray, 0, sizeof(SOPC_Socket) * SOPC_MAX_SOCKETS);
    for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
    {
        socketsArray[idx].socketIdx = idx;
        SOPC_Socket_Clear(&(socketsArray[idx].sock));
    }

    SOPC_ReturnStatus status = SOPC_AsyncQueue_Init(&socketsInputEventQueue, "SocketsInternalContext");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    maxBufferSize = SOPC_Internal_Common_GetEncodingConstants()->buffer_size;
}

void SOPC_SocketsInternalContext_Clear(void)
{
    // Close any not closed remaining socket
    uint32_t idx = 0;
    for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
    {
        if (socketsArray[idx].isUsed)
        {
            SOPC_Socket_Close(&(socketsArray[idx].sock));
            socketsArray[idx].isUsed = false;
        }
    }

    SOPC_AsyncQueue_Free(&socketsInputEventQueue);
}

SOPC_Socket* SOPC_SocketsInternalContext_GetFreeSocket(bool isListener)
{
    SOPC_Socket* result = NULL;
    uint32_t idx = 1; // index 0 is forbidden => reserved for invalid index
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    do
    {
        if (!socketsArray[idx].isUsed)
        {
            socketsArray[idx].isUsed = true;
            result = &socketsArray[idx];
        }
        idx++;
    } while (NULL == result && idx < SOPC_MAX_SOCKETS);

    if (NULL != result && !isListener)
    {
        status = SOPC_AsyncQueue_Init(&result->writeQueue, "Socket write msgs");
        assert(SOPC_STATUS_OK == status);
    }
    return result;
}

void SOPC_SocketsInternalContext_CloseSocket(uint32_t socketIdx)
{
    SOPC_Socket* sock = NULL;
    void* elt = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (socketIdx < SOPC_MAX_SOCKETS && socketsArray[socketIdx].isUsed)
    {
        sock = &socketsArray[socketIdx];
        SOPC_Socket_Close(&sock->sock);
        SOPC_Socket_Clear(&sock->sock);

        if (sock->connectAddrs != NULL)
        {
            SOPC_Socket_AddrInfoDelete((SOPC_Socket_AddressInfo**) &sock->connectAddrs);
        }

        if (sock->writeQueue != NULL)
        {
            // Clear all buffers in the queue
            status = SOPC_AsyncQueue_NonBlockingDequeue(sock->writeQueue, &elt);
            while (SOPC_STATUS_OK == status && NULL != elt)
            {
                SOPC_Buffer_Delete((SOPC_Buffer*) elt);
                elt = NULL;
                status = SOPC_AsyncQueue_NonBlockingDequeue(sock->writeQueue, &elt);
            }
            // Clear the queue
            SOPC_AsyncQueue_Free(&sock->writeQueue);
        }

        if (sock->state != SOCKET_STATE_CLOSED)
        {
            if (sock->isServerConnection)
            {
                assert(sock->listenerSocketIdx < SOPC_MAX_SOCKETS);

                // Management of number of connection on a listener
                if (socketsArray[sock->listenerSocketIdx].state == SOCKET_STATE_LISTENING &&
                    socketsArray[sock->listenerSocketIdx].listenerConnections > 0)
                {
                    socketsArray[sock->listenerSocketIdx].listenerConnections--;
                }
            }
        }

        /* Equivalent to
         * sock->isUsed = false;
         * sock->state = SOCKET_STATE_CLOSED;
         * ...
         * */
        memset(sock, 0, sizeof(SOPC_Socket));

        sock->socketIdx = socketIdx;
    }
}

void SOPC_Sockets_Emit(SOPC_Sockets_OutputEvent event, uint32_t eltId, uintptr_t params, uintptr_t auxParam)
{
    assert(socketsEventHandler != NULL);
    SOPC_ReturnStatus status = SOPC_EventHandler_Post(socketsEventHandler, (int32_t) event, eltId, params, auxParam);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
}

SOPC_ReturnStatus SOPC_Sockets_EnqueueInputEvent(SOPC_Sockets_InputEvent socketEvent,
                                                 uint32_t id,
                                                 uintptr_t params,
                                                 uintptr_t auxParam)
{
    struct Event* ev = SOPC_Calloc(1, sizeof(struct Event));
    if (NULL == ev)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    ev->event = socketEvent;
    ev->id = id;
    ev->params = params;
    ev->auxParam = auxParam;
    SOPC_ReturnStatus status = SOPC_AsyncQueue_BlockingEnqueue(socketsInputEventQueue, ev);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(ev);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Sockets_DequeueAndDispatchInputEvent(void)
{
    struct Event* ev = NULL;
    SOPC_ReturnStatus status = SOPC_AsyncQueue_NonBlockingDequeue(socketsInputEventQueue, (void**) &ev);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SocketsEventMgr_Dispatcher(ev->event, ev->id, ev->params, ev->auxParam);
        SOPC_Free(ev);
    }
    return status;
}
