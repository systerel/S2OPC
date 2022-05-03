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
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "sopc_sockets_network_event_mgr.h"

#include "sopc_sockets_event_mgr.h"
#include "sopc_sockets_internal_ctx.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

static struct
{
    int32_t initDone;
    int32_t stopFlag;
    Thread thread;
    Socket sigServerListeningSock;  /* A local server used to connect a local client for signaling interruption */
    Socket sigClientSock;           /* A local client used to send signal interrupting "select" blocking call */
    Socket sigServerConnectionSock; /* Accepted local client connection used to receive signal interrupting "select"
                                       blocking call*/
} receptionThread = {.initDone = false,
                     .stopFlag = 0,
                     .sigServerListeningSock = SOPC_INVALID_SOCKET,
                     .sigClientSock = SOPC_INVALID_SOCKET,
                     .sigServerConnectionSock = SOPC_INVALID_SOCKET};

/* Variables used to consume signals to interrupt: no functional use */
#define MAX_CONSUMED_SIG_BYTES 100               // maximum number of signals consumed per loop
static uint8_t sigBytes[MAX_CONSUMED_SIG_BYTES]; // read buffer

static bool SOPC_Internal_InitSocketsToInterruptSelect(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool serverListening = false;

    SOPC_Socket_AddressInfo* addrs = NULL;
    SOPC_Socket_AddressInfo* iter = NULL;

    SOPC_SocketSet readSet, writeSet, exceptSet;
    SOPC_SocketSet_Clear(&readSet);
    SOPC_SocketSet_Clear(&writeSet);
    SOPC_SocketSet_Clear(&exceptSet);

    /* Retrieve addressing information for local loopback address */
    status = SOPC_Socket_AddrInfo_Get("127.0.0.1", NULL, &addrs);
    if (SOPC_STATUS_OK == status)
    {
        /* Listen on local loopback address */
        iter = addrs;
        while (!serverListening && iter != NULL)
        {
            status = SOPC_Socket_CreateNew(iter, true, false, &receptionThread.sigServerListeningSock);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Socket_Listen(receptionThread.sigServerListeningSock, iter);
            }
            if (SOPC_STATUS_OK == status)
            {
                serverListening = true;
            }
            else
            {
                iter = SOPC_Socket_AddrInfo_IterNext(iter);
            }
        }

        if (!serverListening)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Connect on local loopback address of the server listening */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Socket_CreateNew(iter, true, false, &receptionThread.sigClientSock);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Socket_ConnectToLocal(receptionThread.sigClientSock, receptionThread.sigServerListeningSock);
    }
    /* Blocking accept on socket connection that will be used to interrupt "select" on sockets */
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_Socket_Accept(receptionThread.sigServerListeningSock, true, &receptionThread.sigServerConnectionSock);
    }

    SOPC_Socket_AddrInfoDelete(&addrs);

    return SOPC_STATUS_OK == status;
}

static bool SOPC_Internal_ConsumeSigBytes(Socket sigSocket, SOPC_SocketSet* readSet)
{
    uint32_t readSigBytes;

    if (SOPC_SocketSet_IsPresent(sigSocket, readSet))
    {
        SOPC_ReturnStatus status = SOPC_Socket_Read(sigSocket, sigBytes, MAX_CONSUMED_SIG_BYTES, &readSigBytes);
        if (SOPC_STATUS_CLOSED == status)
        {
            return false;
        }
    }
    return true;
}

// Treat sockets events if some are present or wait for events (until timeout)
static bool SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(Socket sigSocket)
{
    bool result = true;
    uint32_t idx = 0;
    int32_t nbReady = 0;
    SOPC_Socket* uaSock = NULL;
    SOPC_SocketSet readSet, writeSet, exceptSet;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SOPC_SocketSet_Clear(&readSet);
    SOPC_SocketSet_Clear(&writeSet);
    SOPC_SocketSet_Clear(&exceptSet);

    // Add the signal socket to interrupt "select" (WaitSocketsEvents)
    SOPC_SocketSet_Add(sigSocket, &readSet);

    // Add used sockets in the correct socket sets
    for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
    {
        uaSock = &(socketsArray[idx]);
        if (uaSock->isUsed != false &&
            (uaSock->state == SOCKET_STATE_CONNECTED || uaSock->state == SOCKET_STATE_CONNECTING ||
             uaSock->state == SOCKET_STATE_LISTENING))
        { // Note: accepted state is a state in which we do not know what to do
          //       in case of data received (no high-level connection set).
          //       Wait CONNECTED state for those sockets to treat events again.
            if (uaSock->state == SOCKET_STATE_CONNECTING ||
                (uaSock->state == SOCKET_STATE_CONNECTED && uaSock->isNotWritable != false))
            {
                // Wait for an event indicating connection succeeded/failed in CONNECTING state
                // or Wait for an event indicating connection is writable again in CONNECTED state
                SOPC_SocketSet_Add(uaSock->sock, &writeSet);
            }
            else
            {
                SOPC_SocketSet_Add(uaSock->sock, &readSet);
            }
            SOPC_SocketSet_Add(uaSock->sock, &exceptSet);
        }
    }

    // Returns number of ready descriptor or -1 in case of error
    nbReady = SOPC_Socket_WaitSocketEvents(&readSet, &writeSet, &exceptSet, 0);

    if (nbReady < 0)
    {
        // Call to wait events failed since there is at least 1 socket (sigSocket)
        result = false;
    }
    else if (nbReady > 0)
    {
        /* Consumes bytes sent to signal input event and set result to false in case of close signal */
        result = SOPC_Internal_ConsumeSigBytes(sigSocket, &readSet);

        /* Treat the input events available from upper layer level */
        status = SOPC_STATUS_OK;
        while (SOPC_STATUS_OK == status)
        {
            status = SOPC_Sockets_DequeueAndDispatchInputEvent();
        }

        /* Treat the network events available */
        for (idx = 0; idx < SOPC_MAX_SOCKETS; idx++)
        {
            uaSock = &(socketsArray[idx]);
            if (uaSock->isUsed != false)
            {
                if (uaSock->state == SOCKET_STATE_CONNECTING)
                {
                    /* Socket is currently in connecting attempt: check WRITE events */

                    if (SOPC_SocketSet_IsPresent(uaSock->sock, &writeSet) != false)
                    {
                        // Check connection errors: mandatory when non blocking connection
                        status = SOPC_Socket_CheckAckConnect(uaSock->sock);
                        if (SOPC_STATUS_OK != status)
                        {
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_CONNECTION_ATTEMPT_FAILED, uaSock);
                        }
                        else
                        {
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_CONNECTED, uaSock);
                        }
                    }
                }
                else
                {
                    /* Socket is not in connecting state: check READ and WRITE events */

                    if (SOPC_SocketSet_IsPresent(uaSock->sock, &readSet) != false)
                    {
                        if (uaSock->state == SOCKET_STATE_CONNECTED)
                        {
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_READY_TO_READ, uaSock);
                        }
                        else if (uaSock->state == SOCKET_STATE_LISTENING)
                        {
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_LISTENER_CONNECTION_ATTEMPT, uaSock);
                        }
                        else
                        {
                            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                                   "SocketNetworkMgr: unexpected read event on socketIdx=%" PRIu32,
                                                   idx);
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_CLOSE, uaSock);
                        }
                    }
                    else if (SOPC_SocketSet_IsPresent(uaSock->sock, &writeSet) != false)
                    {
                        if (uaSock->state == SOCKET_STATE_CONNECTED)
                        {
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_READY_TO_WRITE, uaSock);
                        }
                        else
                        {
                            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                                   "SocketNetworkMgr: unexpected write event on socketIdx=%" PRIu32,
                                                   idx);
                            SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_CLOSE, uaSock);
                        }
                    }
                }

                // In any state check EXCEPT events
                if (SOPC_SocketSet_IsPresent(uaSock->sock, &exceptSet) != false)
                {
                    // TODO: retrieve exception code
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "SocketNetworkMgr: exception event on socketIdx=%" PRIu32, idx);
                    SOPC_SocketsInternalEventMgr_Dispatcher(INT_SOCKET_CLOSE, uaSock);
                }
            }
        }
    }

    return result;
}

static void* SOPC_SocketsNetworkEventMgr_ThreadLoop(void* nullData)
{
    SOPC_UNUSED_ARG(nullData);
    bool result = true;
    while (result)
    {
        result = SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(receptionThread.sigServerConnectionSock);
    }
    assert(SOPC_Atomic_Int_Get(&receptionThread.stopFlag) != 0);
    return NULL;
}

static bool SOPC_SocketsNetworkEventMgr_LoopThreadStart(void)
{
    if (SOPC_Atomic_Int_Get(&receptionThread.initDone))
    {
        return false;
    }

    /* Initialize the sockets used to interrupt "select" blocking call */
    bool result = SOPC_Internal_InitSocketsToInterruptSelect();

    if (!result)
    {
        return false;
    }

    receptionThread.stopFlag = 0;

    if (SOPC_Thread_Create(&receptionThread.thread, SOPC_SocketsNetworkEventMgr_ThreadLoop, NULL, "Sockets") !=
        SOPC_STATUS_OK)
    {
        return false;
    }

    SOPC_Atomic_Int_Set(&receptionThread.initDone, true);

    return true;
}

static void SOPC_SocketsNetworkEventMgr_LoopThreadStop(void)
{
    if (!SOPC_Atomic_Int_Get(&receptionThread.initDone))
    {
        return;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    // stop the reception thread
    SOPC_Atomic_Int_Set(&receptionThread.stopFlag, 1);

    /* Send signal to interrupt "select" and stop */
    SOPC_Socket_Close(&receptionThread.sigClientSock);

    status = SOPC_Thread_Join(receptionThread.thread);
    assert(status == SOPC_STATUS_OK);

    /* Close all sockets created to interrupt select */
    SOPC_Socket_Close(&receptionThread.sigServerConnectionSock);
    SOPC_Socket_Close(&receptionThread.sigServerListeningSock);

    SOPC_Atomic_Int_Set(&receptionThread.initDone, false);
}

void SOPC_SocketsNetworkEventMgr_Initialize(void)
{
    bool result = SOPC_SocketsNetworkEventMgr_LoopThreadStart();
    SOPC_ASSERT(result);
}

void SOPC_SocketsNetworkEventMgr_Clear(void)
{
    SOPC_SocketsNetworkEventMgr_LoopThreadStop();
}

void SOPC_SocketsNetworkEventMgr_InterruptForInputEvent(void)
{
    const uint8_t data = 0;
    uint32_t length = 1;

    if (!SOPC_Atomic_Int_Get(&receptionThread.initDone))
    {
        return;
    }

    /* Send signal to interrupt "select" and treat input event */
    SOPC_Socket_Write(receptionThread.sigClientSock, &data, length, &length);
}
