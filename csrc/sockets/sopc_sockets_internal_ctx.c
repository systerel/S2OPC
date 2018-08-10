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

#include "sopc_sockets_internal_ctx.h"

#include "sopc_async_queue.h"
#include "sopc_buffer.h"
#include "sopc_raw_sockets.h"
#include "sopc_sockets_event_mgr.h"

SOPC_Socket socketsArray[SOPC_MAX_SOCKETS];
Mutex socketsMutex;
SOPC_Looper* socketsLooper = NULL;
SOPC_EventHandler* socketsInputEventHandler = NULL;
SOPC_EventHandler* socketsEventHandler = NULL;

void SOPC_SocketsInternalContext_Initialize()
{
    uint32_t idx = 0;
    memset(socketsArray, 0, sizeof(SOPC_Socket) * SOPC_MAX_SOCKETS);
    for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
    {
        socketsArray[idx].socketIdx = idx;
        SOPC_Socket_Clear(&(socketsArray[idx].sock));
    }
    Mutex_Initialization(&socketsMutex);

    socketsLooper = SOPC_Looper_Create();
    assert(socketsLooper != NULL);

    socketsInputEventHandler = SOPC_EventHandler_Create(socketsLooper, SOPC_SocketsEventMgr_Dispatcher);
    assert(socketsInputEventHandler != NULL);
}

void SOPC_SocketsInternalContext_Clear()
{
    // Close any not closed remaining socket
    uint32_t idx = 0;
    Mutex_Lock(&socketsMutex);
    for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
    {
        if (false != socketsArray[idx].isUsed)
        {
            SOPC_Socket_Close(&(socketsArray[idx].sock));
            socketsArray[idx].isUsed = false;
        }
    }

    SOPC_Looper_Delete(socketsLooper);

    Mutex_Unlock(&socketsMutex);
    Mutex_Clear(&socketsMutex);
}

SOPC_Socket* SOPC_SocketsInternalContext_GetFreeSocketNoLock(bool isListener)
{
    SOPC_Socket* result = NULL;
    uint32_t idx = 1; // index 0 is forbidden => reserved for invalid index
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    do
    {
        if (false == socketsArray[idx].isUsed)
        {
            socketsArray[idx].isUsed = true;
            result = &socketsArray[idx];
        }
        idx++;
    } while (NULL == result && idx < SOPC_MAX_SOCKETS);

    if (NULL != result && isListener == false)
    {
        status = SOPC_AsyncQueue_Init(&result->writeQueue, "Socket write msgs");
        assert(SOPC_STATUS_OK == status);
    }
    return result;
}

void SOPC_SocketsInternalContext_CloseSocketNoLock(uint32_t socketIdx)
{
    SOPC_Socket* sock = NULL;
    void* elt = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (socketIdx < SOPC_MAX_SOCKETS && socketsArray[socketIdx].isUsed != false)
    {
        sock = &socketsArray[socketIdx];
        SOPC_Socket_Close(&sock->sock);
        sock->isUsed = false;
        sock->state = SOCKET_STATE_CLOSED;
        sock->waitTreatNetworkEvent = false;
        sock->isServerConnection = false;
        sock->listenerSocketIdx = 0;
        sock->listenerConnections = 0;
        if (sock->connectAddrs != NULL)
        {
            SOPC_Socket_AddrInfoDelete((SOPC_Socket_AddressInfo**) &sock->connectAddrs);
        }
        sock->connectAddrs = NULL;
        sock->nextConnectAttemptAddr = NULL;
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
    }
}

void SOPC_SocketsInternalContext_CloseSocketLock(uint32_t socketIdx)
{
    Mutex_Lock(&socketsMutex);
    SOPC_SocketsInternalContext_CloseSocketNoLock(socketIdx);
    Mutex_Unlock(&socketsMutex);
}

void SOPC_Sockets_Emit(SOPC_Sockets_OutputEvent event, uint32_t eltId, void* params, uintptr_t auxParam)
{
    assert(socketsEventHandler != NULL);
    SOPC_ReturnStatus status = SOPC_EventHandler_Post(socketsEventHandler, (int32_t) event, eltId, params, auxParam);
    assert(status == SOPC_STATUS_OK);
}
