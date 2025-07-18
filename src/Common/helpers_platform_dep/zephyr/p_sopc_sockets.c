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

#include <errno.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>
#include <zephyr/posix/fcntl.h>

/* s2opc includes */

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

/* platform dep includes */

#include "p_sopc_multicast.h"
#include "p_sopc_sockets.h"
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

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif

/* Max pending connection based on max pending connections allowed by zephyr */

#ifdef CONFIG_NET_SOCKETS_POLL_MAX
#define MAX_PENDING_CONNECTION CONFIG_NET_SOCKETS_POLL_MAX
#else
#define MAX_PENDING_CONNECTION 4
#endif

/* Private global definitions */

// *** Socket internal functions definitions

static inline SOPC_ReturnStatus P_SOCKET_Configure(SOPC_Socket sock, bool setNonBlocking);

// True when the module is initialized
static bool priv_Initialized = false;

// *** Public SOCKET API functions definitions ***

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
SOPC_ReturnStatus SOPC_Socket_AddrInfo_Get(const char* hostname,            // Hostname or address
                                           const char* port,                // Port
                                           SOPC_Socket_AddressInfo** addrs) // Socket address info object
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.addrInfo.ai_family = AF_UNSPEC; // AF_INET or AF_INET6  can be use to force IPV4 or IPV6
    hints.addrInfo.ai_socktype = SOCK_STREAM;
    hints.addrInfo.ai_flags = AI_PASSIVE;

    struct zsock_addrinfo* getAddrInfoRes = NULL;

    if ((NULL != hostname || NULL != port) && NULL != addrs)
    {
        int ret = zsock_getaddrinfo(hostname, port, &hints.addrInfo, &getAddrInfoRes);
        if (ret != 0)
        {
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
    if (NULL != addr)
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
    if (NULL != addrs)
    {
        zsock_freeaddrinfo(&(*addrs)->addrInfo);
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
        res = zsock_getpeername(sock->sock, (struct sockaddr*) sockAddrStorage, &sockAddrStorageLen);
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
        int res = zsock_getnameinfo(addr->address.ai_addr, addr->address.ai_addrlen, hostRes, NI_MAXHOST, serviceRes,
                                    NI_MAXSERV, flags);
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
    if (NULL != sock)
    {
        *sock = SOPC_INVALID_SOCKET;
    }
}

static inline SOPC_ReturnStatus P_SOCKET_SetNonBlocking(int sock)
{
    int fl = zsock_fcntl(sock, F_GETFL, 0);
    int setOptStatus = zsock_fcntl(sock, F_SETFL, fl | O_NONBLOCK);
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

static inline SOPC_ReturnStatus P_SOCKET_Configure(SOPC_Socket sock, bool setNonBlocking)
{
    if (SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    const int trueInt = true;

    // Deactivate Nagle's algorithm since we always write a TCP UA binary message (and not just few bytes)
    int setOptStatus = zsock_setsockopt(sock->sock, IPPROTO_TCP, TCP_NODELAY, (const void*) &trueInt, sizeof(int));
    if (0 == setOptStatus)
    {
        status = SOPC_STATUS_OK;
        if (setNonBlocking != false)
        {
            status = P_SOCKET_SetNonBlocking(sock->sock);
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
    int setOptStatus = -1;

    socketImpl->sock = zsock_socket(addr->addrInfo.ai_family, addr->addrInfo.ai_socktype, addr->addrInfo.ai_protocol);

    if (ZSOCK_ERROR == socketImpl->sock)
    {
        SOPC_Free(socketImpl);
        return SOPC_STATUS_NOK;
    }
    SOCKETS_DEBUG(" ** SOPC_Socket_CreateNew %d\n", socketImpl->sock);

    status = P_SOCKET_Configure(socketImpl, setNonBlocking);

    if (SOPC_STATUS_OK == status && setReuseAddr != false)
    {
        const int trueInt = true;
        setOptStatus =
            zsock_setsockopt(socketImpl->sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
        if (0 != setOptStatus)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    // From IPV6 sockets, allow IPV4 and IPV6 exchanges
    if (SOPC_STATUS_OK == status && AF_INET6 == addr->addrInfo.ai_family)
    {
        const int falseInt = false;
        setOptStatus =
            zsock_setsockopt(socketImpl->sock, IPPROTO_IPV6, IPV6_V6ONLY, (const void*) &falseInt, sizeof(int));
        if (0 != setOptStatus)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *sock = socketImpl;
    }
    else
    {
        int res = -1;
        S2OPC_TEMP_FAILURE_RETRY(res, zsock_close(socketImpl->sock));
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
    int bindListenStatus = ZSOCK_ERROR;
    if (NULL != addr)
    {
        bindListenStatus = zsock_bind(sock->sock, addr->addrInfo.ai_addr, addr->addrInfo.ai_addrlen);
        if (ZSOCK_ERROR != bindListenStatus)
        {
            bindListenStatus = zsock_listen(sock->sock, MAX_PENDING_CONNECTION);
            SOCKETS_DEBUG(" ** SOPC_Socket_Listen %d => status = %d\n", sock->sock, bindListenStatus);
        }
    }
    if (ZSOCK_ERROR != bindListenStatus)
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
    struct sockaddr remoteAddr;
    socklen_t addrLen = 0;
    if (listeningSock->sock != ZSOCK_ERROR)
    {
        S2OPC_TEMP_FAILURE_RETRY(acceptedImpl->sock, zsock_accept(listeningSock->sock, &remoteAddr, &addrLen));

        if (acceptedImpl->sock != ZSOCK_ERROR)
        {
            SOCKETS_DEBUG(" ** SOPC_Socket_Accept %d => %d, blocking = %d\n", listeningSock->sock, acceptedImpl->sock,
                          !setNonBlocking);
            status = P_SOCKET_Configure(acceptedImpl, setNonBlocking);
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
    if (addr != NULL && sock != SOPC_INVALID_SOCKET)
    {
        S2OPC_TEMP_FAILURE_RETRY(connectStatus,
                                 zsock_connect(sock->sock, addr->addrInfo.ai_addr, addr->addrInfo.ai_addrlen));
        if (connectStatus < 0)
        {
            if (errno == EINPROGRESS)
            {
                // Non blocking connection started
                connectStatus = 0;
                SOCKETS_DEBUG(" ** SOPC_Socket_Connect %d in progress\n", sock->sock);
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
    int ret = zsock_getsockname(to->sock, addr.addrInfo.ai_addr, &addr.addrInfo.ai_addrlen);
    if (0 == ret)
    {
        status = SOPC_Socket_Connect(from, &addr);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_CheckAckConnect(SOPC_Socket sock)
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
    //    SOCKETS_DEBUG(" ** SOPC_Socket_CheckAckConnect %d => ret= %d, error=%d\n", sock->, ret, error);
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
    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        ZSOCK_FD_SET(sock->sock, &sockSet->set);
        if (sock->sock > sockSet->fdmax)
        {
            sockSet->fdmax = sock->sock;
        }
    }
}

void SOPC_SocketSet_Remove(SOPC_Socket sock, SOPC_SocketSet* sockSet)
{
    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        ZSOCK_FD_CLR(sock->sock, &sockSet->set);
    }
}

bool SOPC_SocketSet_IsPresent(SOPC_Socket sock, SOPC_SocketSet* sockSet)
{
    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        if (false == ZSOCK_FD_ISSET(sock->sock, &sockSet->set))
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
    S2OPC_TEMP_FAILURE_RETRY(nbReady, zsock_select(fdmax + 1, readSet != NULL ? &readSet->set : NULL,
                                                   writeSet != NULL ? &writeSet->set : NULL,
                                                   exceptSet != NULL ? &exceptSet->set : NULL, val));

    if (nbReady < 0)
    {
        const int errId = errno;
        SOPC_UNUSED_ARG(errId); // In case SOCKETS_DEBUG is not defined
        SOCKETS_DEBUG(" ** SOPC_Socket_WaitSocketEvents failed with errno = %d (%s) \n", errId, strerror((errId)));
    }
    return (int32_t) nbReady;
}

SOPC_ReturnStatus SOPC_Socket_Write(SOPC_Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || count > INT32_MAX || NULL == sentBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ssize_t res = 0;
    errno = 0;
    S2OPC_TEMP_FAILURE_RETRY(res, zsock_send(sock->sock, data, count, 0));

    SOCKETS_DEBUG(" ** send Sock %d \n", sock->sock);
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
    if (result != SOPC_STATUS_OK)
    {
        const int err = errno;
        printk("[Error] Send FAILED on Sock %d. Err=%d(%s)\n", sock->sock, err, strerror(err));
    }
    return result;
}

SOPC_ReturnStatus SOPC_Socket_Read(SOPC_Socket sock, uint8_t* data, uint32_t dataSize, uint32_t* readCount)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || 0 >= dataSize || NULL == readCount)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
    errno = 0;
    ssize_t sReadCount = 0;
    S2OPC_TEMP_FAILURE_RETRY(sReadCount, zsock_recv(sock->sock, data, dataSize, 0));

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
            SOCKETS_DEBUG(" ** Read Sock SOPC_STATUS_WOULD_BLOCK\n");
            result = SOPC_STATUS_WOULD_BLOCK;
        }
        else if (0 == sReadCount)
        {
            SOCKETS_DEBUG(" ** Read Sock %d CLOSED\n", sock->sock);
            result = SOPC_STATUS_CLOSED;
        }
        else
        {
            const int err = errno;
            printk("[Error] Read FAILED on Sock %d. Err= %d(%s)\n", sock->sock, err, strerror(err));
        }
    }
    return result;
}

SOPC_ReturnStatus SOPC_Socket_BytesToRead(SOPC_Socket sock, uint32_t* bytesToRead)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(bytesToRead);
    // Function not available on Zephyr
    return SOPC_STATUS_NOK;
}

void SOPC_Socket_Close(SOPC_Socket* sock)
{
    if (NULL != sock && SOPC_INVALID_SOCKET != *sock)
    {
        SOCKETS_DEBUG(" ** zsock_shutdown Sock %d \n", (*sock)->sock);
        zsock_shutdown((*sock)->sock, ZSOCK_SHUT_RDWR);
        int res = 0;
        S2OPC_TEMP_FAILURE_RETRY(res, zsock_close((*sock)->sock));
        if (0 != res)
        {
            // Failed to close socket. This is unrecoverable
            SOPC_ASSERT(false);
        }
        SOPC_Free(*sock);
        *sock = SOPC_INVALID_SOCKET;
    }
}
