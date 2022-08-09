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
#include <stdio.h>
#include <string.h>

#include "sopc_buffer.h"
#include "sopc_helper_uri.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_sockets_api.h"
#include "sopc_sockets_event_mgr.h"
#include "sopc_sockets_internal_ctx.h"

#include "p_sockets.h"

static bool SOPC_SocketsEventMgr_ConnectClient(SOPC_Socket* connectSocket, SOPC_Socket_AddressInfo* addr)
{
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (connectSocket != NULL && addr != NULL && connectSocket->state == SOCKET_STATE_CLOSED)
    {
        status = SOPC_Socket_CreateNew(addr,
                                       false, // Do not reuse
                                       true,  // Non blocking socket
                                       &connectSocket->sock);
        if (SOPC_STATUS_OK == status)
        {
            result = true;
        }
        if (result)
        {
            status = SOPC_Socket_Connect(connectSocket->sock, addr);
            result = (SOPC_STATUS_OK == status);
        }
        if (result)
        {
            connectSocket->state = SOCKET_STATE_CONNECTING;
        }
        else
        {
            SOPC_Socket_Close(&connectSocket->sock);
        }
    }
    return result;
}

static bool SOPC_SocketsEventMgr_NextConnectClientAttempt(SOPC_Socket* connectSocket)
{
    bool result = false;
    if (NULL != connectSocket && connectSocket->state == SOCKET_STATE_CONNECTING)
    {
        // Close precedently created socket
        SOPC_Socket_Close(&connectSocket->sock);
        // Set state closed but do not reset rest of data (contains next attempt configuration
        connectSocket->state = SOCKET_STATE_CLOSED;

        // Check if next connection attempt available
        SOPC_Socket_AddressInfo* nextAddr = (SOPC_Socket_AddressInfo*) connectSocket->nextConnectAttemptAddr;
        while (!result && nextAddr != NULL)
        {
            result = SOPC_SocketsEventMgr_ConnectClient(connectSocket, nextAddr);
            nextAddr = SOPC_Socket_AddrInfo_IterNext(nextAddr);
            connectSocket->nextConnectAttemptAddr = nextAddr;
        }
    }
    return result;
}

static SOPC_Socket* SOPC_SocketsEventMgr_CreateClientSocket(const char* uri)
{
    SOPC_Socket* resultSocket = NULL;
    SOPC_Socket_AddressInfo *res = NULL, *p = NULL;
    SOPC_Socket* freeSocket = NULL;
    bool connectResult = false;
    char* hostname = NULL;
    char* port = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_UriType type = SOPC_URI_UNDETERMINED;

    if (uri != NULL)
    {
        status = SOPC_Helper_URI_SplitUri(uri, &type, &hostname, &port);
        if (SOPC_STATUS_OK == status)
        {
            if (type != SOPC_URI_TCPUA)
            {
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            freeSocket = SOPC_SocketsInternalContext_GetFreeSocket(false);
            if (NULL == freeSocket)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Socket_AddrInfo_Get(hostname, port, &res);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            for (p = res; p != NULL && !connectResult; p = SOPC_Socket_AddrInfo_IterNext(p))
            {
                connectResult = SOPC_SocketsEventMgr_ConnectClient(freeSocket, p);
            }
            if (!connectResult)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            if (p != NULL)
            {
                // Next attempts addresses for connections remaining: store to use in case of async. connection failure
                freeSocket->nextConnectAttemptAddr = p;
                freeSocket->connectAddrs = res;
            }

            resultSocket = freeSocket;
        }

        if (SOPC_STATUS_OK != status || // connection already failed => do not keep addresses for next attempts
            (res != NULL && NULL == p)) // async connecting but NO next attempts remaining (if current fails)
        {
            SOPC_Socket_AddrInfoDelete(&res);
        }

        if (SOPC_STATUS_OK != status && freeSocket != NULL)
        {
            // Set as closed to be removed from used socket
            SOPC_SocketsInternalContext_CloseSocket(freeSocket->socketIdx);
        }

        if (port != NULL)
        {
            SOPC_Free(port);
        }

        if (hostname != NULL)
        {
            SOPC_Free(hostname);
        }
    }

    return resultSocket;
}

static SOPC_Socket* SOPC_SocketsEventMgr_CreateServerSocket(const char* uri, uint8_t listenAllItfs)
{
    SOPC_Socket* resultSocket = NULL;
    SOPC_Socket_AddressInfo *res = NULL, *p = NULL;
    bool attemptWithIPV6 = true;
    SOPC_Socket* freeSocket = NULL;
    bool listenResult = false;
    char* hostname = NULL;
    char* port = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_UriType type = SOPC_URI_UNDETERMINED;

    if (uri != NULL)
    {
        status = SOPC_Helper_URI_SplitUri(uri, &type, &hostname, &port);
        if (SOPC_STATUS_OK == status)
        {
            if (type != SOPC_URI_TCPUA)
            {
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            freeSocket = SOPC_SocketsInternalContext_GetFreeSocket(true);
            if (NULL == freeSocket)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            if (listenAllItfs)
            {
                SOPC_Free(hostname);
                hostname = NULL;
            }

            status = SOPC_Socket_AddrInfo_Get(hostname, port, &res);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            p = res;
            attemptWithIPV6 = true; // IPV6 first since it supports IPV4
            while ((p != NULL || attemptWithIPV6) && !listenResult)
            {
                if (NULL == p && attemptWithIPV6)
                {
                    // Failed with IPV6 addresses (or none was present), now try with not IPV6 addresses
                    attemptWithIPV6 = false;
                    p = res;
                }
                else
                {
                    if ((attemptWithIPV6 && SOPC_Socket_AddrInfo_IsIPV6(p)) ||
                        (!attemptWithIPV6 && !SOPC_Socket_AddrInfo_IsIPV6(p)))
                    {
                        status = SOPC_Socket_CreateNew(p,
                                                       true, // Reuse
                                                       true, // Non blocking socket
                                                       &freeSocket->sock);
                        if (SOPC_STATUS_OK == status)
                        {
                            status = SOPC_Socket_Listen(freeSocket->sock, p);
                        }

                        if (SOPC_STATUS_OK == status)
                        {
                            freeSocket->state = SOCKET_STATE_LISTENING;
                            listenResult = true;
                        }
                    }
                    p = SOPC_Socket_AddrInfo_IterNext(p);
                }
            }
        }

        if (port != NULL)
        {
            SOPC_Free(port);
        }
        if (hostname != NULL)
        {
            SOPC_Free(hostname);
        }

        if (SOPC_STATUS_OK == status)
        {
            resultSocket = freeSocket;
        }
        else
        {
            if (freeSocket != NULL)
            {
                SOPC_SocketsInternalContext_CloseSocket(freeSocket->socketIdx);
            }
        }
    }

    SOPC_Socket_AddrInfoDelete(&res);

    return resultSocket;
}

static SOPC_ReturnStatus SOPC_SocketsEventMgr_Socket_WriteAll(SOPC_Socket* sock,
                                                              const uint8_t* data,
                                                              uint32_t count,
                                                              uint32_t* finalSentBytes)
{
    assert(sock != NULL);
    assert(data != NULL);
    assert(finalSentBytes != NULL);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t sentBytes = 0;
    uint32_t totalSentBytes = 0;

    if (count == 0)
    {
        // Nothing to send in buffer
        *finalSentBytes = 0;
        return SOPC_STATUS_OK;
    }

    do // While not blocking and bytes sent on socket
    {
        status = SOPC_Socket_Write(sock->sock, data + totalSentBytes, count - totalSentBytes, &sentBytes);
        if (SOPC_STATUS_OK == status)
        {
            totalSentBytes += sentBytes;
        }
    } while (SOPC_STATUS_OK == status && totalSentBytes < count && sentBytes > 0);

    if (sentBytes == 0 && SOPC_STATUS_OK == status) // Consider that 0 bytes sent without blocking is an error on socket
    {
        status = SOPC_STATUS_NOK;
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Non blocking call to Socket_Write returned 0 bytes written (socketIdx=%" PRIu32
                               ", connectionId=%" PRIu32,
                               sock->socketIdx, sock->connectionId);
    }

    *finalSentBytes = totalSentBytes;
    return status;
}

static bool SOPC_SocketsEventMgr_TreatWriteBuffer(SOPC_Socket* sock)
{
    bool nothingToDequeue = false;
    bool writeQueueResult = true;
    bool writeBlocked = false;
    SOPC_Buffer* buffer = NULL;
    uint8_t* data = NULL;
    uint32_t count = 0;
    uint32_t sentBytes = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == sock || NULL == sock->writeQueue || sock->sock == SOPC_INVALID_SOCKET || sock->isNotWritable)
    {
        writeQueueResult = false;
    }

    /* Dequeue message to write and sent through socket until nothing no message remain or socket write blocked */
    while (writeQueueResult && !nothingToDequeue && !writeBlocked)
    {
        status = SOPC_AsyncQueue_NonBlockingDequeue(sock->writeQueue, (void**) &buffer);
        if (SOPC_STATUS_WOULD_BLOCK == status)
        {
            nothingToDequeue = true;
        }
        else if (SOPC_STATUS_OK != status || NULL == buffer)
        {
            writeQueueResult = false;
        }
        if (writeQueueResult && !nothingToDequeue)
        {
            // Treat current buffer to be written on socket
            data = &(buffer->data[buffer->position]);
            count = buffer->length - buffer->position;

            status = SOPC_SocketsEventMgr_Socket_WriteAll(sock, data, count, &sentBytes);

            if (SOPC_STATUS_WOULD_BLOCK == status)
            {
                writeBlocked = true;
            }
            else if (SOPC_STATUS_OK != status)
            {
                writeQueueResult = false;
                SOPC_Buffer_Delete(buffer);
            }
            else
            {
                SOPC_Buffer_Delete(buffer);
            }
        }
    }

    if (writeBlocked)
    {
        // Socket write blocked, wait for a ready to write event
        sock->isNotWritable = true;
        // (Re-enqueue) updated buffer position for next attempt
        buffer->position = buffer->position + sentBytes;
        // Re-enqueue in LIFO mode to be the next buffer to treat
        status = SOPC_AsyncQueue_BlockingEnqueueFirstOut(sock->writeQueue, buffer);
        assert(SOPC_STATUS_OK == status);
    }

    return writeQueueResult;
}

static SOPC_ReturnStatus on_ready_read(SOPC_Socket* socket, uint32_t socket_id)
{
    if (socket->state != SOCKET_STATE_CONNECTED)
    {
        // ignore event since socket have been closed and event have been triggered
        return SOPC_STATUS_OK;
    }

    SOPC_Buffer* buffer = NULL;

    uint32_t bytesToRead = 0;
    uint32_t readBytes = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = SOPC_Socket_BytesToRead(socket->sock, &bytesToRead);

    if (SOPC_STATUS_OK != status)
    {
        // No information, use the minimum size of buffer
        bytesToRead = SOPC_MIN_BYTE_BUFFER_SIZE_READ_SOCKET;
        status = SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (0 == bytesToRead)
        {
            // Nothing to read, still allocate minimum size and make an attempt to at least check the socket closed case
            bytesToRead = SOPC_MIN_BYTE_BUFFER_SIZE_READ_SOCKET;
        }
        else if (bytesToRead > maxBufferSize)
        {
            // Too many bytes to read for a chunk maximum size, allocate the maximum chunk size
            bytesToRead = maxBufferSize;
        }

        buffer = SOPC_Buffer_Create(bytesToRead);
        status = NULL == buffer ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Socket_Read(socket->sock, buffer->data, bytesToRead, &readBytes);
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_Buffer_Delete(buffer);
        return (status == SOPC_STATUS_WOULD_BLOCK) ? SOPC_STATUS_OK : status;
    }

    status = SOPC_Buffer_SetDataLength(buffer, readBytes);
    assert(status == SOPC_STATUS_OK);

    SOPC_Sockets_Emit(SOCKET_RCV_BYTES, socket->connectionId, (uintptr_t) buffer, socket_id);

    return SOPC_STATUS_OK;
}

void SOPC_SocketsEventMgr_Dispatcher(SOPC_Sockets_InputEvent socketEvent,
                                     uint32_t eltId,
                                     uintptr_t params,
                                     uintptr_t auxParam)
{
    bool result = false;
    SOPC_Socket* socketElt = NULL;
    SOPC_Buffer* buffer = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (socketEvent)
    {
    case SOCKET_CREATE_SERVER:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: SOCKET_CREATE_SERVER epCfgIdx=%" PRIu32 " URI=%s allItfs=%s", eltId,
                               (char*) params, auxParam ? "true" : "false");
        /*
        id = endpoint description config index,
        params = (const char*) URI,
        auxParms = (bool) listenAllInterfaces
        */
        socketElt = SOPC_SocketsEventMgr_CreateServerSocket((const char*) params, (bool) auxParam);
        if (NULL != socketElt)
        {
            socketElt->connectionId = eltId;
            SOPC_Sockets_Emit(SOCKET_LISTENER_OPENED, eltId, (uintptr_t) NULL, socketElt->socketIdx);
        }
        else
        {
            SOPC_Sockets_Emit(SOCKET_LISTENER_FAILURE, eltId, (uintptr_t) NULL, 0);
        }
        break;
    case SOCKET_ACCEPTED_CONNECTION:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: SOCKET_ACCEPTED_CONNECTION socketIdx=%" PRIu32 " scIdx=%" PRIuPTR, eltId,
                               auxParam);

        /* id = socket index,
         * auxParam = secure channel connection index associated to accepted connection */
        assert(auxParam <= UINT32_MAX);
        assert(eltId < SOPC_MAX_SOCKETS);

        socketElt = &socketsArray[eltId];
        if (socketElt->state == SOCKET_STATE_ACCEPTED)
        {
            socketElt->connectionId = (uint32_t) auxParam;
            socketElt->state = SOCKET_STATE_CONNECTED;
        }
        else
        {
            SOPC_SocketsInternalContext_CloseSocket(eltId);
        }
        break;
    case SOCKET_CREATE_CLIENT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: SOCKET_CREATE_CLIENT scIdx=%" PRIu32 " URI=%s", eltId, (char*) params);
        /*
        id = secure channel connection index,
        params = (const char*) URI
        */
        socketElt = SOPC_SocketsEventMgr_CreateClientSocket((const char*) params);
        if (NULL != socketElt)
        {
            socketElt->connectionId = eltId;
        }
        else
        {
            SOPC_Sockets_Emit(SOCKET_FAILURE, eltId, (uintptr_t) NULL, 0);
        }
        break;
    case SOCKET_CLOSE:
        assert(eltId < SOPC_MAX_SOCKETS);
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: SOCKET_CLOSE socketIdx=%" PRIu32 " connectionIdx=%" PRIuPTR, eltId,
                               auxParam);
        /* id = socket index */
        socketElt = &socketsArray[eltId];

        /* Check upper level request is still valid: expected socket state and upper connection id */
        if (socketElt->state != SOCKET_STATE_CLOSED && socketElt->state != SOCKET_STATE_LISTENING &&
            socketElt->connectionId == (uint32_t) auxParam)
        {
            SOPC_SocketsInternalContext_CloseSocket(eltId);
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "SocketEvent: SOCKET_CLOSE ignored due to socketState=%d connectionIdx=%" PRIu32,
                                   (int) socketElt->state, socketElt->connectionId);
        }
        break;
    case SOCKET_CLOSE_SERVER:
        assert(eltId < SOPC_MAX_SOCKETS);
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: SOCKET_CLOSE_SERVER socketIdx=%" PRIu32 " endpointIdx=%" PRIuPTR, eltId,
                               auxParam);
        /* id = socket index */
        socketElt = &socketsArray[eltId];

        /* Check upper level request is still valid: expected socket state and upper connection id */
        if (socketElt->state == SOCKET_STATE_LISTENING && socketElt->connectionId == (uint32_t) auxParam)
        {
            SOPC_SocketsInternalContext_CloseSocket(eltId);
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "SocketEvent: SOCKET_CLOSE ignored due to socketState=%d connectionIdx=%" PRIu32,
                                   (int) socketElt->state, socketElt->connectionId);
        }
        break;
    case SOCKET_WRITE:
        assert(eltId < SOPC_MAX_SOCKETS);
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "SocketEvent: SOCKET_WRITE socketIdx=%" PRIu32, eltId);
        /*
        id = socket index,
        params = (SOPC_Buffer*) msg buffer
        */
        socketElt = &socketsArray[eltId];
        buffer = (SOPC_Buffer*) params;

        if (socketElt->state == SOCKET_STATE_CONNECTED && NULL != buffer)
        {
            // Note: No need to lock mutex,
            // socket state cannot be changed from CONNECTED state by another thread

            // Prepare buffer to be written (position set to 0 since it has been written precedently)
            status = SOPC_Buffer_SetPosition(buffer, 0);
            assert(SOPC_STATUS_OK == status);
            // Enqueue message buffer to send
            status = SOPC_AsyncQueue_BlockingEnqueue(socketElt->writeQueue, buffer);
            assert(SOPC_STATUS_OK == status);
            result = true;
            if (!socketElt->isNotWritable)
            {
                // If socket is in writable state: trigger the socket write treatment
                result = SOPC_SocketsEventMgr_TreatWriteBuffer(socketElt);
            }
        }
        else
        {
            // Free the buffer if not NULL
            SOPC_Buffer_Delete(buffer);
            result = false;
        }

        if (!result)
        {
            SOPC_Sockets_Emit(SOCKET_FAILURE, socketElt->connectionId, (uintptr_t) NULL, eltId);
            // Definitively close the socket
            SOPC_SocketsInternalContext_CloseSocket(eltId);
        }

        break;
    default:
        assert(false);
    }
}

void SOPC_SocketsInternalEventMgr_Dispatcher(SOPC_Sockets_InternalInputEvent event, SOPC_Socket* socketElt)
{
    bool result = false;
    SOPC_Socket* acceptSock = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t socketIdx = socketElt->socketIdx;

    switch (event)
    {
    case INT_SOCKET_LISTENER_CONNECTION_ATTEMPT:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: INT_SOCKET_LISTENER_CONNECTION_ATTEMPT socketIdx=%" PRIu32, socketIdx);

        // State was set to accepted by network event manager
        assert(socketElt->state == SOCKET_STATE_LISTENING);

        if (socketElt->listenerConnections < SOPC_MAX_SOCKETS_CONNECTIONS)
        {
            acceptSock = SOPC_SocketsInternalContext_GetFreeSocket(false);
        }
        if (NULL == acceptSock)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SocketsMgr: refusing new connection since maximum number of socket reached (%" PRIu32 "/%d)",
                socketElt->listenerConnections, SOPC_MAX_SOCKETS_CONNECTIONS);
        }
        else
        {
            status = SOPC_Socket_Accept(socketElt->sock,
                                        1, // Non blocking socket
                                        &acceptSock->sock);
            if (SOPC_STATUS_OK == status)
            {
                acceptSock->isServerConnection = true;
                acceptSock->listenerSocketIdx = socketElt->socketIdx;
                // Set initial state of new socket
                acceptSock->state = SOCKET_STATE_ACCEPTED;

                // Increment number of connections on listener
                socketElt->listenerConnections++;

                // Send to the secure channel listener state manager and wait for SOCKET_ACCEPTED_CONNECTION for
                // association with connection index
                SOPC_Sockets_Emit(SOCKET_LISTENER_CONNECTION,
                                  socketElt->connectionId, // endpoint description config index
                                  (uintptr_t) NULL, acceptSock->socketIdx);
            }
            else
            {
                SOPC_SocketsInternalContext_CloseSocket(acceptSock->socketIdx);
            }
        }

        break;
    case INT_SOCKET_CONNECTION_ATTEMPT_FAILED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: INT_SOCKET_CONNECTION_ATTEMPT_FAILED socketIdx=%" PRIu32, socketIdx);

        // State is connecting
        assert(socketElt->state == SOCKET_STATE_CONNECTING);

        // Will do a new attempt with next possible address if possible
        result = SOPC_SocketsEventMgr_NextConnectClientAttempt(socketElt);
        if (!result)
        {
            // No new attempt possible, indicates socket connection failed and close the socket
            SOPC_Sockets_Emit(SOCKET_FAILURE,
                              socketElt->connectionId, // endpoint description config index
                              (uintptr_t) NULL, 0);
            // Definitively close the socket
            SOPC_SocketsInternalContext_CloseSocket(socketIdx);
        }

        break;
    case INT_SOCKET_CONNECTED:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "SocketEvent: INT_SOCKET_CONNECTED socketIdx=%" PRIu32,
                               socketIdx);

        // State was set to connected by network manager
        assert(socketElt->state == SOCKET_STATE_CONNECTING);

        // No more attempts expected: free the attempts addresses
        if (socketElt->connectAddrs != NULL)
        {
            SOPC_Socket_AddrInfoDelete((SOPC_Socket_AddressInfo**) &socketElt->connectAddrs);
            socketElt->nextConnectAttemptAddr = NULL;
        }

        // Notify connection
        SOPC_Sockets_Emit(SOCKET_CONNECTION,
                          socketElt->connectionId, // secure channel connection index
                          (uintptr_t) NULL, socketIdx);
        socketElt->state = SOCKET_STATE_CONNECTED;

        break;
    case INT_SOCKET_CLOSE:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "SocketEvent: INT_SOCKET_CLOSE socketIdx=%" PRIu32,
                               socketIdx);

        if (socketElt->state == SOCKET_STATE_LISTENING)
        {
            SOPC_Sockets_Emit(SOCKET_LISTENER_FAILURE, socketElt->connectionId, (uintptr_t) NULL, socketIdx);
        }
        else if (socketElt->state != SOCKET_STATE_CLOSED)
        {
            SOPC_Sockets_Emit(SOCKET_FAILURE, socketElt->connectionId, (uintptr_t) NULL, socketIdx);
        }

        SOPC_SocketsInternalContext_CloseSocket(socketIdx);

        break;
    case INT_SOCKET_READY_TO_READ:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "SocketEvent: INT_SOCKET_READY_TO_READ socketIdx=%" PRIu32,
                               socketIdx);

        status = on_ready_read(socketElt, socketIdx);

        if (status != SOPC_STATUS_OK)
        {
            SOPC_Sockets_Emit(SOCKET_FAILURE, socketElt->connectionId, (uintptr_t) NULL, socketIdx);
            SOPC_SocketsInternalContext_CloseSocket(socketIdx);
        }

        break;
    case INT_SOCKET_READY_TO_WRITE:
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SocketEvent: INT_SOCKET_READY_TO_WRITE socketIdx=%" PRIu32, socketIdx);

        // Socket is connected
        if (socketElt->state == SOCKET_STATE_CONNECTED)
        {
            if (!socketElt->isNotWritable)
            {
                // Not expected: ignore
            }
            else
            {
                // Socket was not writable
                socketElt->isNotWritable = false;
                // Trigger the socket write treatment
                SOPC_SocketsEventMgr_TreatWriteBuffer(socketElt);
            }

        } // else: ignore event since socket could have been closed since event was triggered

        break;
    default:
        assert(false);
    }
}
