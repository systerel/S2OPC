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

#include <fcntl.h>
#include <stdlib.h>

#include "kernel.h"
#include "net/ethernet.h"
#include "net/net_if.h"
#include "net/socket.h"

/* s2opc includes */

#include "sopc_macros.h"
#include "sopc_mutexes.h"

/* platform dep includes */

#include "p_multicast.h"
#include "p_sockets.h"
#include "sopc_assert.h"
#include "sopc_raw_sockets.h"

// #define SOCKETS_DEBUG printk
#define SOCKETS_DEBUG(...) \
    do                     \
    {                      \
    } while (0)

#if EWOULDBLOCK != EAGAIN
#error "Expecting 'EWOULDBLOCK == EAGAIN' in socket implementation"
#endif

#define ZSOCK_ERROR -1

/* Max pending connection based on max pending connections allowed by zephyr */

#ifdef CONFIG_NET_SOCKETS_POLL_MAX
#define MAX_PENDING_CONNECTION CONFIG_NET_SOCKETS_POLL_MAX
#else
#define MAX_PENDING_CONNECTION 4
#endif

/* Private global definitions */

// *** Socket internal functions definitions

static inline SOPC_ReturnStatus P_SOCKET_Configure(Socket sock, bool setNonBlocking);

// True when the module is initialized
static bool priv_Initialized = false;

// *** Public SOCKET API functions definitions ***

// Initialize ethernet and loopback interface
// Returns true if well configured.
bool SOPC_Socket_Network_Initialize()
{
    SOPC_ASSERT(!priv_Initialized);
    priv_Initialized = true;
    return priv_Initialized;
}

// Not implmented, return always true.
bool SOPC_Socket_Network_Clear()
{
    SOPC_ASSERT(priv_Initialized);
    priv_Initialized = false;
    // Check that all allocated sockets are closed.
    return true;
}

// Create socket address information object. Shall be destroy after used.
SOPC_ReturnStatus SOPC_Socket_AddrInfo_Get(char* hostname,                  // Hostname or address
                                           char* port,                      // Port
                                           SOPC_Socket_AddressInfo** addrs) // Socket address info object
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6  can be use to force IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((NULL != hostname || NULL != port) && NULL != addrs)
    {
        int ret = zsock_getaddrinfo(hostname, port, &hints, addrs);
        if (ret != 0)
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

SOPC_Socket_AddressInfo* SOPC_Socket_AddrInfo_IterNext(SOPC_Socket_AddressInfo* addr)
{
    SOPC_Socket_AddressInfo* res = NULL;
    if (NULL != addr)
    {
        res = addr->ai_next;
    }
    return res;
}

uint8_t SOPC_Socket_AddrInfo_IsIPV6(SOPC_Socket_AddressInfo* addr)
{
    return PF_INET6 == addr->ai_family;
}

void SOPC_Socket_AddrInfoDelete(SOPC_Socket_AddressInfo** addrs)
{
    if (NULL != addrs)
    {
        zsock_freeaddrinfo(*addrs);
        *addrs = NULL;
    }
}

void SOPC_Socket_Clear(Socket* sock)
{
    *sock = SOPC_INVALID_SOCKET;
}

static inline SOPC_ReturnStatus P_SOCKET_SetNonBlocking(int sock)
{
    int fl = fcntl(sock, F_GETFL, 0);
    int setOptStatus = fcntl(sock, F_SETFL, fl | O_NONBLOCK);
    if (setOptStatus == 0)
    {
        return SOPC_STATUS_OK;
    }
    else
    {
        SOCKETS_DEBUG(" ** P_SOCKET_SetNonBlocking failed on %d\n", sock);
        return SOPC_STATUS_NOK;
    }
}

static inline SOPC_ReturnStatus P_SOCKET_Configure(Socket sock, bool setNonBlocking)
{
    if (SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    const int trueInt = true;

    // Deactivate Nagle's algorithm since we always write a TCP UA binary message (and not just few bytes)
    int setOptStatus = zsock_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*) &trueInt, sizeof(int));
    if (0 == setOptStatus)
    {
        status = SOPC_STATUS_OK;
        if (setNonBlocking != false)
        {
            status = P_SOCKET_SetNonBlocking(sock);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_CreateNew(SOPC_Socket_AddressInfo* addr,
                                        bool setReuseAddr,
                                        bool setNonBlocking,
                                        Socket* sock)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int setOptStatus = 0;

    if (NULL == addr || NULL == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *sock = zsock_socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (SOPC_INVALID_SOCKET == *sock)
    {
        return SOPC_STATUS_NOK;
    }
    SOCKETS_DEBUG(" ** SOPC_Socket_CreateNew %d\n", *sock);

    status = P_SOCKET_Configure(*sock, setNonBlocking);

    if (SOPC_STATUS_OK == status && setReuseAddr != false)
    {
        const int trueInt = true;
        setOptStatus = zsock_setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
        if (0 != setOptStatus)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    // From IPV6 sockets, allow IPV4 and IPV6 exchanges
    if (SOPC_STATUS_OK == status && AF_INET6 == addr->ai_family)
    {
        const int falseInt = false;
        setOptStatus = zsock_setsockopt(*sock, IPPROTO_IPV6, IPV6_V6ONLY, (const void*) &falseInt, sizeof(int));
        if (0 != setOptStatus)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_Socket_Close(sock);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_Listen(Socket sock, SOPC_Socket_AddressInfo* addr)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int bindListenStatus = ZSOCK_ERROR;
    if (NULL != addr)
    {
        bindListenStatus = zsock_bind(sock, addr->ai_addr, addr->ai_addrlen);
        if (ZSOCK_ERROR != bindListenStatus)
        {
            bindListenStatus = zsock_listen(sock, MAX_PENDING_CONNECTION);
            SOCKETS_DEBUG(" ** SOPC_Socket_Listen %d => status = %d\n", sock, bindListenStatus);
        }
    }
    if (ZSOCK_ERROR != bindListenStatus)
    {
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Accept(Socket listeningSock, bool setNonBlocking, Socket* acceptedSock)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    struct sockaddr remoteAddr;
    socklen_t addrLen = 0;
    if (SOPC_INVALID_SOCKET != listeningSock && NULL != acceptedSock)
    {
        *acceptedSock = zsock_accept(listeningSock, &remoteAddr, &addrLen);
        if (SOPC_INVALID_SOCKET != *acceptedSock)
        {
            SOCKETS_DEBUG(" ** SOPC_Socket_Accept %d => %d, blocking = %d\n", listeningSock, *acceptedSock,
                          !setNonBlocking);
            status = P_SOCKET_Configure(*acceptedSock, setNonBlocking);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Socket_Close(acceptedSock);
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Connect(Socket sock, SOPC_Socket_AddressInfo* addr)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int connectStatus = -1;
    if (addr != NULL && sock != -1)
    {
        connectStatus = zsock_connect(sock, addr->ai_addr, addr->ai_addrlen);
        if (connectStatus < 0)
        {
            if (errno == EINPROGRESS)
            {
                // Non blocking connection started
                connectStatus = 0;
                SOCKETS_DEBUG(" ** SOPC_Socket_Connect %d in progress\n", sock);
            }
        }
        if (connectStatus == 0)
        {
            status = SOPC_STATUS_OK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_ConnectToLocal(Socket from, Socket to)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo addr;
    struct sockaddr saddr;
    memset(&addr, 0, sizeof(SOPC_Socket_AddressInfo));
    memset(&saddr, 0, sizeof(struct sockaddr));
    addr.ai_addr = &saddr;
    addr.ai_addrlen = sizeof(struct sockaddr);
    int ret = zsock_getsockname(to, addr.ai_addr, &addr.ai_addrlen);
    if (0 == ret)
    {
        status = SOPC_Socket_Connect(from, &addr);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_CheckAckConnect(Socket sock)
{
    if (SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // TODO :ZEPHYR does not provide "SOL_SOCKET" option.
    // When supported, it may be implemented somehow like:
    //    int error = 0;
    //    socklen_t len = sizeof(int);
    //    int ret = zsock_getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
    //    SOCKETS_DEBUG(" ** SOPC_Socket_CheckAckConnect %d => ret= %d, error=%d\n", sock, ret, error);
    //
    //    if (ret < 0)
    //    {
    //        SOCKETS_DEBUG(" ** getsockopt(SO_ERROR) return errno = %d\n", errno);
    //        return SOPC_STATUS_NOK;
    //    }
    //    if (0 != error)
    //    {
    //        SOCKETS_DEBUG(" ** getsockopt(SO_ERROR) return error = %d\n", errno);
    //        return SOPC_STATUS_NOK;
    //    }
    return SOPC_STATUS_OK;
}

void SOPC_SocketSet_Add(Socket sock, SOPC_SocketSet* sockSet)
{
    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        ZSOCK_FD_SET(sock, &sockSet->set);
        if (sock > sockSet->fdmax)
        {
            sockSet->fdmax = sock;
        }
    }
}

void SOPC_SocketSet_Remove(Socket sock, SOPC_SocketSet* sockSet)
{
    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        ZSOCK_FD_CLR(sock, &sockSet->set);
    }
}

bool SOPC_SocketSet_IsPresent(Socket sock, SOPC_SocketSet* sockSet)
{
    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        if (false == ZSOCK_FD_ISSET(sock, &sockSet->set))
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
    if (NULL != sockSet)
    {
        ZSOCK_FD_ZERO(&sockSet->set);
        sockSet->fdmax = 0;
    }
}

int32_t SOPC_Socket_WaitSocketEvents(SOPC_SocketSet* readSet,
                                     SOPC_SocketSet* writeSet,
                                     SOPC_SocketSet* exceptSet,
                                     uint32_t waitMs)
{
    int32_t nbReady = 0;
    struct timeval timeout;
    struct timeval* val;
    int fdmax = 0;

    if (NULL == readSet && NULL == writeSet)
    {
        return -1;
    }

    if ((readSet != NULL) && (NULL == writeSet || readSet->fdmax > writeSet->fdmax))
    {
        fdmax = readSet->fdmax;
    }
    else
    {
        if (writeSet != NULL)
        {
            fdmax = writeSet->fdmax;
        }
    }

    if ((exceptSet != NULL) && (exceptSet->fdmax > fdmax))
    {
        fdmax = exceptSet->fdmax;
    }

    if (0 == waitMs)
    {
        val = NULL;
    }
    else
    {
        timeout.tv_sec = (time_t)(waitMs / 1000);
        timeout.tv_usec = (suseconds_t)(1000 * (waitMs % 1000));
        val = &timeout;
    }
    nbReady = zsock_select(fdmax + 1, readSet != NULL ? &readSet->set : NULL, writeSet != NULL ? &writeSet->set : NULL,
                           exceptSet != NULL ? &exceptSet->set : NULL, val);

    if (nbReady < 0)
    {
        const int errId = errno;
        SOPC_UNUSED_ARG(errId); // In case SOCKETS_DEBUG is not defined
        SOCKETS_DEBUG(" ** SOPC_Socket_WaitSocketEvents failed with errno = %d (%s) \n", errId, strerror((errId)));
    }
    return (int32_t) nbReady;
}

SOPC_ReturnStatus SOPC_Socket_Write(Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || count > INT32_MAX || NULL == sentBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ssize_t res = 0;
    errno = 0;
    res = send(sock, data, count, 0);
    SOCKETS_DEBUG(" ** send Sock %d \n", sock);
    SOPC_ReturnStatus result = SOPC_STATUS_NOK;

    if (res >= 0)
    {
        *sentBytes = (uint32_t) res;
        result = SOPC_STATUS_OK;
    }
    else if (EWOULDBLOCK == errno)
    {
        result = SOPC_STATUS_WOULD_BLOCK;
    }
    return result;
}

SOPC_ReturnStatus SOPC_Socket_Read(Socket sock, uint8_t* data, uint32_t dataSize, uint32_t* readCount)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || 0 >= dataSize || NULL == readCount)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
    errno = 0;
    ssize_t sReadCount = 0;
    sReadCount = zsock_recv(sock, data, dataSize, 0);
    const int errId = errno;

    if (sReadCount > 0)
    {
        *readCount = (uint32_t) sReadCount;
        result = SOPC_STATUS_OK;
    }
    else
    {
        *readCount = 0;
        if (EWOULDBLOCK == errId)
        {
            SOCKETS_DEBUG(" ** ReAD Sock SOPC_STATUS_WOULD_BLOCK\n");
            result = SOPC_STATUS_WOULD_BLOCK;
        }
        else if (0 == sReadCount)
        {
            SOCKETS_DEBUG(" ** Read Sock %d CLOSED\n", sock);
            result = SOPC_STATUS_CLOSED;
        }
        else
        {
            SOCKETS_DEBUG(" ** Read Sock %d errno = %d\n", sock, errId);
        }
    }
    return result;
}

SOPC_ReturnStatus SOPC_Socket_BytesToRead(Socket sock, uint32_t* bytesToRead)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(bytesToRead);
    // Function not available on Zephyr
    return SOPC_STATUS_NOK;
}

void SOPC_Socket_Close(Socket* sock)
{
    if (NULL != sock && SOPC_INVALID_SOCKET != *sock)
    {
        SOCKETS_DEBUG(" ** zsock_shutdown Sock %d \n", *sock);
        zsock_shutdown(*sock, ZSOCK_SHUT_RDWR);
        if (0 != zsock_close(*sock))
        {
            // Failed to close socket. This is unrecoverable
            SOPC_ASSERT(false);
        }
        *sock = SOPC_INVALID_SOCKET;
    }
}
