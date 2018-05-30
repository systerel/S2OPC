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

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_sockets_event_mgr.h"

#include "sopc_buffer.h"
#include "sopc_helper_uri.h"
#include "sopc_logger.h"
#include "sopc_secure_channels_api.h"
#include "sopc_sockets_api.h"
#include "sopc_sockets_internal_ctx.h"

#include "p_sockets.h"

static bool ParseURI(const char* uri, char** hostname, char** port)
{
    bool result = false;
    size_t hostnameLength = 0;
    size_t portIdx = 0;
    size_t portLength = 0;
    char* lHostname = NULL;
    char* lPort = NULL;

    if (uri != NULL && hostname != NULL && port != NULL)
    {
        result = SOPC_Helper_URI_ParseTcpUaUri(uri, &hostnameLength, &portIdx, &portLength);
    }

    if (result != false)
    {
        if (portIdx != 0 && hostnameLength != 0 && portLength != 0)
        {
            lHostname = malloc(sizeof(char) * (hostnameLength + 1));
            if (NULL == lHostname)
                return false;
            if (lHostname != memcpy(lHostname, &(uri[10]), hostnameLength))
            {
                free(lHostname);
                return false;
            }
            lHostname[hostnameLength] = '\0';

            lPort = malloc(sizeof(char) * (portLength + 1));
            if (NULL == lPort)
            {
                free(lHostname);
                return false;
            }
            if (lPort != memcpy(lPort, &(uri[portIdx]), portLength))
            {
                free(lHostname);
                free(lPort);
                return false;
            }
            lPort[portLength] = '\0';
            *hostname = lHostname;
            *port = lPort;
        }
        else
        {
            result = false;
        }
    }

    return result;
}

static bool SOPC_SocketsEventMgr_ConnectClient_NoLock(SOPC_Socket* connectSocket, Socket_AddressInfo* addr)
{
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (connectSocket != NULL && addr != NULL && connectSocket->state == SOCKET_STATE_CLOSED)
    {
        status = Socket_CreateNew(addr,
                                  false, // Do not reuse
                                  true,  // Non blocking socket
                                  &connectSocket->sock);
        if (SOPC_STATUS_OK == status)
        {
            result = true;
        }
        if (result != false)
        {
            status = Socket_Connect(connectSocket->sock, addr);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }
        }
        if (result != false)
        {
            connectSocket->state = SOCKET_STATE_CONNECTING;
        }

        if (false == result)
        {
            SOPC_SocketsInternalContext_CloseSocketLock(connectSocket->socketIdx);
        }
    }
    return result;
}

static bool SOPC_SocketsEventMgr_NextConnectClientAttempt_Lock(SOPC_Socket* connectSocket)
{
    bool result = false;
    if (connectSocket != false && connectSocket->state == SOCKET_STATE_CONNECTING)
    {
        Mutex_Lock(&socketsMutex);
        // Close precedently created socket
        Socket_Close(&connectSocket->sock);
        // Set state closed but do not reset rest of data (contains next attempt configuration
        connectSocket->state = SOCKET_STATE_CLOSED;

        // Check if next connection attempt available
        Socket_AddressInfo* nextAddr = (Socket_AddressInfo*) connectSocket->nextConnectAttemptAddr;
        if (nextAddr != NULL)
        {
            result = SOPC_SocketsEventMgr_ConnectClient_NoLock(connectSocket, nextAddr);
            if (result != false)
            {
                connectSocket->nextConnectAttemptAddr = NULL;
            }
            else
            {
                connectSocket->nextConnectAttemptAddr = Socket_AddrInfo_IterNext(nextAddr);
            }

            // No more attempts possible: free the attempts addresses
            if (NULL == connectSocket->nextConnectAttemptAddr)
            {
                Socket_AddrInfoDelete((Socket_AddressInfo**) &connectSocket->connectAddrs);
                connectSocket->connectAddrs = NULL;
            }
        }
        Mutex_Unlock(&socketsMutex);
    }
    return result;
}

static SOPC_Socket* SOPC_SocketsEventMgr_CreateClientSocket_Lock(const char* uri)
{
    SOPC_Socket* resultSocket = NULL;
    Socket_AddressInfo *res = NULL, *p = NULL;
    SOPC_Socket* freeSocket = NULL;
    bool result = false;
    bool connectResult = false;
    char* hostname = NULL;
    char* port = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (uri != NULL)
    {
        Mutex_Lock(&socketsMutex);
        result = ParseURI(uri, &hostname, &port);
        if (result != false)
        {
            freeSocket = SOPC_SocketsInternalContext_GetFreeSocketNoLock(false);
            if (NULL == freeSocket)
            {
                result = false;
            }
        }

        if (result != false)
        {
            status = Socket_AddrInfo_Get(hostname, port, &res);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }
        }

        if (result != false)
        {
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            for (p = res; p != NULL && false == connectResult; p = Socket_AddrInfo_IterNext(p))
            {
                connectResult = SOPC_SocketsEventMgr_ConnectClient_NoLock(freeSocket, p);
            }
            result = connectResult;
        }

        if (result != false)
        {
            if (p != NULL)
            {
                // Next attempts addresses for connections remaining: store to use in case of async. connection failure
                freeSocket->nextConnectAttemptAddr = p;
                freeSocket->connectAddrs = res;
            }

            resultSocket = freeSocket;
        }

        if (false == result || // connection already failed => do not keep addresses for next attempts
            (res != NULL &&
             NULL == freeSocket->connectAddrs) || // async connecting but NO next attempts remaining (if current fails)
            (NULL == p))                          // result is true but res is no more used
        {
            Socket_AddrInfoDelete(&res);
        }

        if (false == result && freeSocket != NULL)
        {
            // Set as closed to be removed from used socket
            SOPC_SocketsInternalContext_CloseSocketNoLock(freeSocket->socketIdx);
        }

        if (port != NULL)
        {
            free(port);
        }

        if (hostname != NULL)
        {
            free(hostname);
        }

        Mutex_Unlock(&socketsMutex);
    }

    return resultSocket;
}

static SOPC_Socket* SOPC_SocketsEventMgr_CreateServerSocket_Lock(const char* uri, uint8_t listenAllItfs)
{
    SOPC_Socket* resultSocket = NULL;
    bool result = false;
    Socket_AddressInfo *res = NULL, *p = NULL;
    bool attemptWithIPV6 = true;
    SOPC_Socket* freeSocket = NULL;
    bool listenResult = false;
    char* hostname = NULL;
    char* port = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (uri != NULL)
    {
        Mutex_Lock(&socketsMutex);
        result = ParseURI(uri, &hostname, &port);
        if (result != false)
        {
            freeSocket = SOPC_SocketsInternalContext_GetFreeSocketNoLock(true);
            if (NULL == freeSocket)
            {
                result = false;
            }
        }

        if (result != false)
        {
            if (listenAllItfs != false)
            {
                free(hostname);
                hostname = NULL;
            }

            status = Socket_AddrInfo_Get(hostname, port, &res);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }
        }

        if (result != false)
        {
            // Try to connect on IP addresses provided (IPV4 and IPV6)
            p = res;
            attemptWithIPV6 = true; // IPV6 first since it supports IPV4
            while ((p != NULL || attemptWithIPV6 != false) && false == listenResult)
            {
                if (NULL == p && attemptWithIPV6 != false)
                {
                    // Failed with IPV6 addresses (or none was present), now try with not IPV6 addresses
                    attemptWithIPV6 = false;
                    p = res;
                }
                else
                {
                    if ((attemptWithIPV6 != false && Socket_AddrInfo_IsIPV6(p) != false) ||
                        (attemptWithIPV6 == false && Socket_AddrInfo_IsIPV6(p) == false))
                    {
                        status = Socket_CreateNew(p,
                                                  true, // Reuse
                                                  true, // Non blocking socket
                                                  &freeSocket->sock);
                        if (SOPC_STATUS_OK == status)
                        {
                            result = true;
                        }
                        else
                        {
                            result = false;
                        }

                        if (result != false)
                        {
                            status = Socket_Listen(freeSocket->sock, p);
                            if (SOPC_STATUS_OK != status)
                            {
                                result = false;
                            }
                        }

                        if (result != false)
                        {
                            freeSocket->state = SOCKET_STATE_LISTENING;
                            listenResult = true;
                        }
                    }
                    p = Socket_AddrInfo_IterNext(p);
                }
            }
        }

        if (port != NULL)
        {
            free(port);
        }
        if (hostname != NULL)
        {
            free(hostname);
        }

        if (result != false)
        {
            resultSocket = freeSocket;
        }
        else
        {
            if (freeSocket != NULL)
            {
                SOPC_SocketsInternalContext_CloseSocketNoLock(freeSocket->socketIdx);
            }
        }

        Mutex_Unlock(&socketsMutex);
    }

    Socket_AddrInfoDelete(&res);

    return resultSocket;
}

static bool SOPC_SocketsEventMgr_TreatWriteBuffer_NoLock(SOPC_Socket* sock)
{
    bool nothingToDequeue = false;
    bool writeQueueResult = true;
    bool writeBlocked = false;
    SOPC_Buffer* buffer = NULL;
    uint8_t* data = NULL;
    uint32_t count = 0;
    uint32_t sentBytes = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == sock || NULL == sock->writeQueue || sock->sock == SOPC_INVALID_SOCKET || sock->isNotWritable != false)
    {
        writeQueueResult = false;
    }

    /* Dequeue message to write and sent through socket until nothing no message remain or socket write blocked */
    while (writeQueueResult != false && nothingToDequeue == false && writeBlocked == false)
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
        if (false != writeQueueResult && false == nothingToDequeue)
        {
            sentBytes = 0;
            data = &(buffer->data[buffer->position]);
            count = buffer->length - buffer->position;
            status = Socket_Write(sock->sock, data, count, &sentBytes);
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

    if (writeBlocked != false)
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

static void SOPC_SocketsEventMgr_SetInternalEventAsTreated_Lock(SOPC_Socket* socketElt)
{
    Mutex_Lock(&socketsMutex);
    // Event treated
    socketElt->waitTreatNetworkEvent = false;
    Mutex_Unlock(&socketsMutex);
}

void SOPC_SocketsEventMgr_Dispatcher(int32_t event, uint32_t eltId, void* params, uintptr_t auxParam)
{
    bool result = false;
    SOPC_Sockets_InputEvent socketEvent = (SOPC_Sockets_InputEvent) event;
    SOPC_Socket* socketElt = NULL;
    SOPC_Socket* acceptSock = NULL;
    SOPC_Buffer* buffer = NULL;
    uint32_t readBytes = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (socketEvent)
    {
    case SOCKET_CREATE_SERVER:
        SOPC_Logger_TraceDebug("SocketEvent: SOCKET_CREATE_SERVER epCfgIdx=%" PRIu32 " URI=%s allItfs=%s", eltId,
                               (char*) params, auxParam ? "true" : "false");
        /*
        id = endpoint description config index,
        params = (const char*) URI,
        auxParms = (bool) listenAllInterfaces
        */
        socketElt = SOPC_SocketsEventMgr_CreateServerSocket_Lock((const char*) params, (bool) auxParam);
        if (NULL != socketElt)
        {
            socketElt->connectionId = eltId;
            SOPC_SecureChannels_EnqueueEvent(SOCKET_LISTENER_OPENED, eltId, NULL, socketElt->socketIdx);
        }
        else
        {
            SOPC_SecureChannels_EnqueueEvent(SOCKET_LISTENER_FAILURE, eltId, NULL, 0);
        }
        break;
    case SOCKET_ACCEPTED_CONNECTION:
        SOPC_Logger_TraceDebug("SocketEvent: SOCKET_ACCEPTED_CONNECTION socketIdx=%" PRIu32 " scIdx=%" PRIuPTR, eltId,
                               auxParam);

        /* id = socket index,
         * auxParam = secure channel connection index associated to accepted connection */
        if (auxParam <= UINT32_MAX)
        {
            Mutex_Lock(&socketsMutex);
            socketElt = &socketsArray[eltId];
            if (socketElt->state == SOCKET_STATE_ACCEPTED)
            {
                socketElt->connectionId = (uint32_t) auxParam;
                socketElt->state = SOCKET_STATE_CONNECTED;
            }
            else
            {
                SOPC_SocketsInternalContext_CloseSocketNoLock(eltId);
            }
            Mutex_Unlock(&socketsMutex);
        }
        break;
    case SOCKET_CREATE_CLIENT:
        SOPC_Logger_TraceDebug("SocketEvent: SOCKET_CREATE_CLIENT scIdx=%" PRIu32 " URI=%s", eltId, (char*) params);
        /*
        id = secure channel connection index,
        params = (const char*) URI
        */
        socketElt = SOPC_SocketsEventMgr_CreateClientSocket_Lock((const char*) params);
        if (NULL != socketElt)
        {
            socketElt->connectionId = eltId;
        }
        else
        {
            SOPC_SecureChannels_EnqueueEvent(SOCKET_FAILURE, eltId, NULL, 0);
        }
        break;
    case SOCKET_CLOSE:
        SOPC_Logger_TraceDebug("SocketEvent: SOCKET_CLOSE socketIdx=%" PRIu32, eltId);
        /* id = socket index */
        socketElt = &socketsArray[eltId];

        if (socketElt->isServerConnection != false && socketElt->state != SOCKET_STATE_CLOSED)
        {
            // Management of number of connection on a listener
            if (socketsArray[socketElt->listenerSocketIdx].state == SOCKET_STATE_LISTENING &&
                socketsArray[socketElt->listenerSocketIdx].listenerConnections > 0)
            {
                socketsArray[socketElt->listenerSocketIdx].listenerConnections--;
            }
        }

        SOPC_SocketsInternalContext_CloseSocketLock(eltId);
        break;
    case SOCKET_WRITE:
        SOPC_Logger_TraceDebug("SocketEvent: SOCKET_WRITE socketIdx=%" PRIu32, eltId);
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
            status = SOPC_AsyncQueue_BlockingEnqueue(socketElt->writeQueue, params);
            assert(SOPC_STATUS_OK == status);
            result = true;
            if (socketElt->isNotWritable == false)
            {
                // If socket is in writable state: trigger the socket write treatment
                result = SOPC_SocketsEventMgr_TreatWriteBuffer_NoLock(socketElt);
            }
        }
        else
        {
            if (NULL != buffer)
            {
                // Free the buffer
                SOPC_Buffer_Delete(buffer);
            }
            result = false;
        }

        if (false == result)
        {
            SOPC_SecureChannels_EnqueueEvent(SOCKET_FAILURE, socketElt->connectionId, NULL, eltId);
            // Definitively close the socket
            SOPC_SocketsInternalContext_CloseSocketLock(eltId);
        }

        break;
    case INT_SOCKET_LISTENER_CONNECTION_ATTEMPT:
        SOPC_Logger_TraceDebug("SocketEvent: INT_SOCKET_LISTENER_CONNECTION_ATTEMPT socketIdx=%" PRIu32, eltId);
        socketElt = &socketsArray[eltId];

        // State was set to accepted by network event manager
        assert(socketElt->state == SOCKET_STATE_LISTENING);

        Mutex_Lock(&socketsMutex);
        if (socketElt->listenerConnections < SOPC_MAX_SOCKETS_CONNECTIONS)
        {
            acceptSock = SOPC_SocketsInternalContext_GetFreeSocketNoLock(false);
        }
        if (NULL == acceptSock)
        {
            SOPC_Logger_TraceWarning(
                "SocketsMgr: refusing new connection since maximum number of socket reached (%" PRIu32 "/%u)",
                socketElt->listenerConnections, SOPC_MAX_SOCKETS_CONNECTIONS);
        }
        else
        {
            status = Socket_Accept(socketElt->sock,
                                   1, // Non blocking socket
                                   &acceptSock->sock);
            if (SOPC_STATUS_OK == status)
            {
                acceptSock->isUsed = true;
                acceptSock->isServerConnection = true;
                acceptSock->listenerSocketIdx = socketElt->socketIdx;
                // Temporarly copy endpoint description config index (waiting for new SC connection
                // index once created)
                acceptSock->connectionId = socketElt->connectionId;
                // Set initial state of new socket
                acceptSock->state = SOCKET_STATE_ACCEPTED;

                // Increment number of connections on listener
                socketElt->listenerConnections++;

                // Send to the secure channel listener state manager and wait for SOCKET_ACCEPTED_CONNECTION for
                // association with connection index
                SOPC_SecureChannels_EnqueueEvent(SOCKET_LISTENER_CONNECTION,
                                                 acceptSock->connectionId, // endpoint description config index
                                                 NULL, acceptSock->socketIdx);
            }
        }

        // Release listening socket for network manager
        socketElt->waitTreatNetworkEvent = false;
        Mutex_Unlock(&socketsMutex);

        break;
    case INT_SOCKET_CONNECTION_ATTEMPT_FAILED:
        SOPC_Logger_TraceDebug("SocketEvent: INT_SOCKET_CONNECTION_ATTEMPT_FAILED socketIdx=%" PRIu32, eltId);
        socketElt = &socketsArray[eltId];
        // State is connecting
        assert(socketElt->state == SOCKET_STATE_CONNECTING);

        // Will do a new attempt with next possible address if possible
        result = SOPC_SocketsEventMgr_NextConnectClientAttempt_Lock(socketElt);
        if (false == result)
        {
            // No new attempt possible, indicates socket connection failed and close the socket
            SOPC_SecureChannels_EnqueueEvent(SOCKET_FAILURE,
                                             socketElt->connectionId, // endpoint description config index
                                             NULL, 0);
            // Definitively close the socket
            SOPC_SocketsInternalContext_CloseSocketLock(eltId);
        }
        else
        {
            SOPC_SocketsEventMgr_SetInternalEventAsTreated_Lock(socketElt);
        }

        break;
    case INT_SOCKET_CONNECTED:
        SOPC_Logger_TraceDebug("SocketEvent: INT_SOCKET_CONNECTED socketIdx=%" PRIu32, eltId);
        socketElt = &socketsArray[eltId];
        // State was set to connected by network manager
        assert(socketElt->state == SOCKET_STATE_CONNECTING);

        // No more attempts expected: free the attempts addresses
        if (socketElt->connectAddrs != NULL)
        {
            Socket_AddrInfoDelete((Socket_AddressInfo**) &socketElt->connectAddrs);
            socketElt->connectAddrs = NULL;
            socketElt->nextConnectAttemptAddr = NULL;
        }

        // Notify connection
        SOPC_SecureChannels_EnqueueEvent(SOCKET_CONNECTION,
                                         socketElt->connectionId, // secure channel connection index
                                         NULL, eltId);

        Mutex_Lock(&socketsMutex);
        // Event treated
        socketElt->waitTreatNetworkEvent = false;
        socketElt->state = SOCKET_STATE_CONNECTED;
        Mutex_Unlock(&socketsMutex);

        break;
    case INT_SOCKET_CLOSE:
        SOPC_Logger_TraceDebug("SocketEvent: INT_SOCKET_CLOSE socketIdx=%" PRIu32, eltId);
        socketElt = &socketsArray[eltId];

        if (socketElt->state == SOCKET_STATE_LISTENING)
        {
            SOPC_SecureChannels_EnqueueEvent(SOCKET_LISTENER_FAILURE, socketElt->connectionId, NULL, eltId);
        }
        else if (socketElt->state != SOCKET_STATE_CLOSED)
        {
            if (socketElt->isServerConnection != false)
            {
                // Management of number of connection on a listener
                if (socketsArray[socketElt->listenerSocketIdx].state == SOCKET_STATE_LISTENING &&
                    socketsArray[socketElt->listenerSocketIdx].listenerConnections > 0)
                {
                    socketsArray[socketElt->listenerSocketIdx].listenerConnections--;
                }
            }
            SOPC_SecureChannels_EnqueueEvent(SOCKET_FAILURE, socketElt->connectionId, NULL, eltId);
        }

        SOPC_SocketsInternalContext_CloseSocketLock(eltId);
        break;
    case INT_SOCKET_READY_TO_READ:
        SOPC_Logger_TraceDebug("SocketEvent: INT_SOCKET_READY_TO_READ socketIdx=%" PRIu32, eltId);
        socketElt = &socketsArray[eltId];

        if (socketElt->state == SOCKET_STATE_CONNECTED)
        {
            buffer = SOPC_Buffer_Create(SOPC_MAX_MESSAGE_LENGTH);
            if (NULL != buffer)
            {
                status = Socket_Read(socketElt->sock, buffer->data, SOPC_MAX_MESSAGE_LENGTH, &readBytes);
            }
            else
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            if (status == SOPC_STATUS_WOULD_BLOCK)
            {
                SOPC_Buffer_Delete(buffer);
                buffer = NULL;
                // wait next ready to read event
            }
            else if (SOPC_STATUS_OK == status)
            {
                // Update buffer lengtn
                SOPC_Buffer_SetDataLength(buffer, (uint32_t) readBytes);
                // Transmit to secure channel connection associated
                SOPC_SecureChannels_EnqueueEvent(SOCKET_RCV_BYTES, socketElt->connectionId, (void*) buffer, eltId);
            }
            else
            {
                SOPC_Buffer_Delete(buffer);
                buffer = NULL;
                SOPC_SecureChannels_EnqueueEvent(SOCKET_FAILURE, socketElt->connectionId, NULL, eltId);
                SOPC_SocketsInternalContext_CloseSocketLock(eltId);
            }

            SOPC_SocketsEventMgr_SetInternalEventAsTreated_Lock(socketElt);
        } // else: ignore event since socket could have been closed since event was triggered

        break;
    case INT_SOCKET_READY_TO_WRITE:
        SOPC_Logger_TraceDebug("SocketEvent: INT_SOCKET_READY_TO_WRITE socketIdx=%" PRIu32, eltId);
        socketElt = &socketsArray[eltId];

        // Socket is connected
        if (socketElt->state == SOCKET_STATE_CONNECTED)
        {
            if (socketElt->isNotWritable == false)
            {
                // Not expected: ignore
            }
            else
            {
                // Socket was not writable
                socketElt->isNotWritable = false;
                // Trigger the socket write treatment
                SOPC_SocketsEventMgr_TreatWriteBuffer_NoLock(socketElt);
            }

            SOPC_SocketsEventMgr_SetInternalEventAsTreated_Lock(socketElt);
        } // else: ignore event since socket could have been closed since event was triggered

        break;
    default:
        assert(false);
    }
}
