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

#include "p_sopc_sockets.h"

#include "sopc_common_constants.h"
#include "sopc_raw_sockets.h"

#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

static WSADATA wsaData;

SOPC_ReturnStatus SOPC_Socket_Network_Enable_Keepalive(SOPC_Socket sock,
                                                       unsigned int time,
                                                       unsigned int interval,
                                                       unsigned int counter)
{
    (void) sock;
    (void) time;
    (void) interval;
    (void) counter;
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_Socket_Network_Disable_Keepalive(SOPC_Socket sock)
{
    (void) sock;
    return SOPC_STATUS_NOT_SUPPORTED;
}

bool SOPC_Socket_Network_Initialize(void)
{
    bool status = true;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        status = false;
    }
    return status;
}

bool SOPC_Socket_Network_Clear(void)
{
    bool status = true;
    int result = WSACleanup();
    if (result != 0)
    {
        status = false;
    }
    return status;
}
#include <stdio.h>
SOPC_ReturnStatus SOPC_Socket_AddrInfo_Get(const char* hostname, const char* port, SOPC_Socket_AddressInfo** addrs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.addrInfo.ai_family = AF_UNSPEC; // AF_INET or AF_INET6  can be use to force IPV4 or IPV6
    hints.addrInfo.ai_socktype = SOCK_STREAM;
    hints.addrInfo.ai_flags = AI_PASSIVE;

    struct addrinfo* getAddrInfoRes = NULL;

    if ((hostname != NULL || port != NULL) && addrs != NULL)
    {
        if (getaddrinfo(hostname, port, &hints.addrInfo, &getAddrInfoRes) != 0)
        {
            SOPC_CONSOLE_PRINTF("ERROR: %d\n", getaddrinfo(hostname, port, &hints.addrInfo, &getAddrInfoRes));
            status = SOPC_STATUS_NOK;
        }
        else
        {
            *addrs = (SOPC_Socket_AddressInfo*) getAddrInfoRes;
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_Socket_AddressInfo* SOPC_Socket_AddrInfo_IterNext(SOPC_Socket_AddressInfo* addr)
{
    SOPC_Socket_AddressInfo* res = NULL;
    if (addr != NULL)
    {
        res = (SOPC_Socket_AddressInfo*) addr->addrInfo.ai_next;
    }
    return res;
}

uint8_t SOPC_Socket_AddrInfo_IsIPV6(const SOPC_Socket_AddressInfo* addr)
{
    return addr->addrInfo.ai_family == PF_INET6;
}

void SOPC_Socket_AddrInfoDelete(SOPC_Socket_AddressInfo** addrs)
{
    if (addrs != NULL)
    {
        freeaddrinfo(&(*addrs)->addrInfo);
        *addrs = NULL;
    }
}

void SOPC_SocketAddress_Delete(SOPC_Socket_Address** addr)
{
    if (NULL == addr)
    {
        return;
    }
    if (NULL != *addr)
    {
        SOPC_Free((*addr)->address.ai_addr);
    }
    SOPC_Free(*addr);
    *addr = NULL;
}

SOPC_Socket_Address* SOPC_Socket_CopyAddress(SOPC_Socket_AddressInfo* addr)
{
    SOPC_Socket_Address* result = SOPC_Calloc(1, sizeof(*result));
    if (NULL != result)
    {
        result->address.ai_addr = SOPC_Calloc(1, addr->addrInfo.ai_addrlen);
        result->address.ai_addrlen = addr->addrInfo.ai_addrlen;
        result->address.ai_family = addr->addrInfo.ai_family;
        if (NULL != result->address.ai_addr)
        {
            result->address.ai_addr =
                memcpy(result->address.ai_addr, addr->addrInfo.ai_addr, addr->addrInfo.ai_addrlen);
        }
        else
        {
            SOPC_Free(result);
            result = NULL;
        }
    }
    return result;
}

SOPC_Socket_Address* SOPC_Socket_GetPeerAddress(SOPC_Socket sock)
{
    if (sock == SOPC_INVALID_SOCKET)
    {
        return NULL;
    }
    SOPC_Socket_Address* result = SOPC_Calloc(1, sizeof(*result));
    struct sockaddr_storage* sockAddrStorage = SOPC_Calloc(1, sizeof(*sockAddrStorage));
    socklen_t sockAddrStorageLen = sizeof(*sockAddrStorage);
    int res = -1;
    if (NULL != result && NULL != sockAddrStorage)
    {
        res = getpeername(sock->sock, (struct sockaddr*) sockAddrStorage, &sockAddrStorageLen);
        if (0 == res)
        {
            result->address.ai_family = sockAddrStorage->ss_family;
            result->address.ai_addrlen = sockAddrStorageLen;
            result->address.ai_addr = (struct sockaddr*) sockAddrStorage;
        }
    }
    if (res != 0)
    {
        SOPC_Free(sockAddrStorage);
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

SOPC_ReturnStatus SOPC_SocketAddress_GetNameInfo(const SOPC_Socket_Address* addr, char** host, char** service)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == addr || (NULL == host && NULL == service))
    {
        return status;
    }
    int flags = 0;
    status = SOPC_STATUS_OK;
    char* hostRes = NULL;
    char* serviceRes = NULL;
    if (NULL != host)
    {
        flags |= NI_NUMERICHOST;

        hostRes = SOPC_Calloc(NI_MAXHOST, sizeof(*hostRes));
        if (hostRes == NULL)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status && NULL != service)
    {
        flags |= NI_NUMERICSERV;
        serviceRes = SOPC_Calloc(NI_MAXSERV, sizeof(*serviceRes));
        if (serviceRes == NULL)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        int res = getnameinfo(addr->address.ai_addr, (socklen_t) addr->address.ai_addrlen, hostRes, NI_MAXHOST,
                              serviceRes, NI_MAXSERV, flags);
        if (0 != res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(hostRes);
        SOPC_Free(serviceRes);
    }
    else
    {
        if (NULL != host)
        {
            *host = hostRes;
        }
        if (NULL != service)
        {
            *service = serviceRes;
        }
    }
    return status;
}

void SOPC_Socket_Clear(SOPC_Socket* sock)
{
    *sock = SOPC_INVALID_SOCKET;
}

static SOPC_ReturnStatus Socket_Configure(SOCKET sock, bool setNonBlocking)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const int trueInt = true;
    u_long ltrue = 1;
    int setOptStatus = -1;
    if (sock != INVALID_SOCKET)
    {
        status = SOPC_STATUS_OK;
        // Deactivate Nagle's algorithm since we always write a TCP UA binary message (and not just few bytes)
        setOptStatus = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*) &trueInt, sizeof(int));

        /*
        if(setOptStatus != SOCKET_ERROR){
            int rcvbufsize = UINT16_MAX;
            setOptStatus = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &rcvbufsize, sizeof(int));
        }

        if(setOptStatus != SOCKET_ERROR){
            int sndbufsize = UINT16_MAX;
            setOptStatus = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*) &sndbufsize, sizeof(int));
        }
        */

        if (setOptStatus != SOCKET_ERROR && setNonBlocking != false)
        {
            setOptStatus = ioctlsocket(sock, (long) FIONBIO, &ltrue); // True => Non blocking
        }

        if (setOptStatus == SOCKET_ERROR)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_CreateNew(SOPC_Socket_AddressInfo* addr,
                                        bool setReuseAddr,
                                        bool setNonBlocking,
                                        SOPC_Socket* sock)
{
    if (NULL == addr || NULL == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Socket_Impl* socketImpl = SOPC_Calloc(1, sizeof(*socketImpl));
    if (NULL == socketImpl)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    const int trueInt = true;
    int setOptStatus = SOCKET_ERROR;
    status = SOPC_STATUS_OK;
    socketImpl->sock = socket(addr->addrInfo.ai_family, addr->addrInfo.ai_socktype, addr->addrInfo.ai_protocol);
    if (socketImpl->sock != INVALID_SOCKET)
    {
        status = Socket_Configure(socketImpl->sock, setNonBlocking);

        if (status == SOPC_STATUS_OK)
        {
            setOptStatus = 0;
        } // else SOCKET_ERROR due to init

        if (setOptStatus != SOCKET_ERROR && setReuseAddr != false)
        {
            setOptStatus = setsockopt(socketImpl->sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
        }

        // Enforce IPV6 sockets can be used for IPV4 connections (if socket is IPV6)
        if (setOptStatus != SOCKET_ERROR && addr->addrInfo.ai_family == AF_INET6)
        {
            const int falseInt = false;
            setOptStatus =
                setsockopt(socketImpl->sock, IPPROTO_IPV6, IPV6_V6ONLY, (const void*) &falseInt, sizeof(int));
        }
    }
    if (setOptStatus == SOCKET_ERROR)
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        *sock = socketImpl;
    }
    else
    {
        closesocket(socketImpl->sock);
        SOPC_Free(socketImpl);
        *sock = SOPC_INVALID_SOCKET;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Listen(SOPC_Socket sock, SOPC_Socket_AddressInfo* addr)
{
    if (SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int bindListenStatus = -1;
    if (addr != NULL)
    {
        bindListenStatus = bind(sock->sock, addr->addrInfo.ai_addr, (int) addr->addrInfo.ai_addrlen);
        if (bindListenStatus != SOCKET_ERROR)
        {
            bindListenStatus = listen(sock->sock, SOMAXCONN);
        }
    }
    if (bindListenStatus != SOCKET_ERROR)
    {
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Accept(SOPC_Socket listeningSock, bool setNonBlocking, SOPC_Socket* acceptedSock)
{
    if (SOPC_INVALID_SOCKET == listeningSock || NULL == acceptedSock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Socket_Impl* acceptedImpl = SOPC_Calloc(1, sizeof(*acceptedImpl));
    if (NULL == acceptedImpl)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (listeningSock->sock != INVALID_SOCKET)
    {
        acceptedImpl->sock = accept(listeningSock->sock, NULL, NULL);
        if (acceptedImpl->sock != INVALID_SOCKET)
        {
            status = Socket_Configure(acceptedImpl->sock, setNonBlocking);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *acceptedSock = acceptedImpl;
    }
    else
    {
        SOPC_Socket_Close(&acceptedImpl);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Connect(SOPC_Socket sock, SOPC_Socket_AddressInfo* addr)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int connectStatus = -1;
    int wsaError = 0;
    if (addr != NULL && sock != SOPC_INVALID_SOCKET)
    {
        connectStatus = connect(sock->sock, addr->addrInfo.ai_addr, (int) addr->addrInfo.ai_addrlen);
        if (connectStatus == SOCKET_ERROR)
        {
            wsaError = WSAGetLastError();
            if (wsaError == WSAEWOULDBLOCK)
            {
                // Non blocking connection started
                connectStatus = 0;
            }
        }
        if (connectStatus == 0)
        {
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_ConnectToLocal(SOPC_Socket from, SOPC_Socket to)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo addr;
    struct sockaddr saddr;
    memset(&addr, 0, sizeof(SOPC_Socket_AddressInfo));
    memset(&saddr, 0, sizeof(struct sockaddr));
    addr.addrInfo.ai_addr = &saddr;
    addr.addrInfo.ai_addrlen = sizeof(struct sockaddr);

    if (0 == getsockname(to->sock, addr.addrInfo.ai_addr, (int*) &addr.addrInfo.ai_addrlen))
    {
        status = SOPC_Socket_Connect(from, &addr);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_CheckAckConnect(SOPC_Socket sock)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int error = 0;
    socklen_t len = sizeof(int);
    if (sock != SOPC_INVALID_SOCKET)
    {
        if (getsockopt(sock->sock, SOL_SOCKET, SO_ERROR, (char*) &error, &len) < 0 || error != 0)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_SocketSet* SOPC_SocketSet_Create(void)
{
    SOPC_SocketSet* res = SOPC_Calloc(1, sizeof(SOPC_SocketSet));
    if (NULL != res)
    {
        SOPC_SocketSet_Clear(res);
    }
    return res;
}

void SOPC_SocketSet_Delete(SOPC_SocketSet** set)
{
    if (NULL != set)
    {
        SOPC_Free(*set);
        *set = NULL;
    }
}

void SOPC_SocketSet_Add(SOPC_Socket sock, SOPC_SocketSet* sockSet)
{
    if (sock != SOPC_INVALID_SOCKET && sockSet != NULL)
    {
        FD_SET(sock->sock, &sockSet->set);
    }
}

bool SOPC_SocketSet_IsPresent(SOPC_Socket sock, SOPC_SocketSet* sockSet)
{
    if (sock != SOPC_INVALID_SOCKET && sockSet != NULL)
    {
        if (false == FD_ISSET(sock->sock, &sockSet->set))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

void SOPC_SocketSet_Clear(SOPC_SocketSet* sockSet)
{
    if (sockSet != NULL)
    {
        FD_ZERO(&sockSet->set);
    }
}

int32_t SOPC_Socket_WaitSocketEvents(SOPC_SocketSet* readSet,
                                     SOPC_SocketSet* writeSet,
                                     SOPC_SocketSet* exceptSet,
                                     uint32_t waitMs)
{
    int32_t nbReady = 0;
    const int ignored = 0; // ignored in winsocks
    struct timeval timeout;
    struct timeval* val;

    if (waitMs == 0)
    {
        val = NULL;
    }
    else
    {
        if (waitMs / 1000 > LONG_MAX)
        {
            timeout.tv_sec = LONG_MAX;
        }
        else
        {
            timeout.tv_sec = (long) (waitMs / 1000);
        }
        if (1000 * (waitMs % 1000) > LONG_MAX)
        {
            timeout.tv_usec = LONG_MAX;
        }
        else
        {
            timeout.tv_usec = (long) (1000 * (waitMs % 1000));
        }
        val = &timeout;
    }
    nbReady = select(ignored, &readSet->set, &writeSet->set, &exceptSet->set, val);
    if (nbReady == SOCKET_ERROR || nbReady > INT32_MAX || nbReady < INT32_MIN)
        return -1;
    return (int32_t) nbReady;
}

SOPC_ReturnStatus SOPC_Socket_Write(SOPC_Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int res = 0;
    int wsaError = 0;
    if (sock != SOPC_INVALID_SOCKET && data != NULL && count <= INT32_MAX && sentBytes != NULL)
    {
        status = SOPC_STATUS_NOK;
        res = send(sock->sock, (const char*) data, (int) count, 0);
        if (res >= 0)
        {
            /* MSDN send documentation indicates case res == 0 shall not occur in non blocking mode:
             * "If no buffer space is available within the transport system to hold the data to be transmitted, send
             * will block unless the socket has been placed in nonblocking mode. On nonblocking stream oriented sockets,
             * the number of bytes written can be between 1 and the requested length, depending on buffer availability
             * on both the client and server computers."
             * */
            *sentBytes = (uint32_t) res;
            status = SOPC_STATUS_OK;
        }
        else
        {
            *sentBytes = 0;

            wsaError = WSAGetLastError();
            // ERROR CASE
            if (wsaError == WSAEWOULDBLOCK)
            {
                // Try again in those cases
                status = SOPC_STATUS_WOULD_BLOCK;
            } // keep SOPC_STATUS_NOK
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Read(SOPC_Socket sock, uint8_t* data, uint32_t dataSize, uint32_t* readCount)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int res = 0;
    if (sock != SOPC_INVALID_SOCKET && data != NULL && dataSize > 0 && dataSize <= INT32_MAX && NULL != readCount)
    {
        res = recv(sock->sock, (char*) data, (int) dataSize, 0);
        if (res > 0)
        {
            *readCount = (uint32_t) res;
            status = SOPC_STATUS_OK;
        }
        else if (res == 0)
        {
            *readCount = 0;
            status = SOPC_STATUS_CLOSED;
        }
        else
        {
            *readCount = 0;
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                status = SOPC_STATUS_WOULD_BLOCK;
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_BytesToRead(SOPC_Socket sock, uint32_t* bytesToRead)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    u_long nbBytes = 0;
    if (sock != SOPC_INVALID_SOCKET && bytesToRead != NULL)
    {
        int res = ioctlsocket(sock->sock, FIONREAD, &nbBytes);
        if (res == 0)
        {
            if (nbBytes < UINT32_MAX)
            {
                *bytesToRead = (uint32_t) nbBytes;
            }
            else
            {
                *bytesToRead = UINT32_MAX;
            }

            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

void SOPC_Socket_Close(SOPC_Socket* sock)
{
    if (sock != NULL && *sock != SOPC_INVALID_SOCKET)
    {
        if (closesocket((*sock)->sock) != SOCKET_ERROR)
        {
            SOPC_Free(*sock);
            *sock = SOPC_INVALID_SOCKET;
        }
    }
}
