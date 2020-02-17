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

#include "sopc_mutexes.h"

/* platform dep includes */

#include "p_sockets.h"

/* Max pending connection based on max pending connections allowed by zephyr */

#ifdef CONFIG_NET_SOCKETS_POLL_MAX
#define MAX_PENDING_CONNECTION CONFIG_NET_SOCKETS_POLL_MAX
#else
#define MAX_PENDING_CONNECTION 4
#endif

/* Max socket based on max connections allowed by zephyr */

#ifdef CONFIG_NET_MAX_CONN
#define MAX_ZEPHYR_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_ZEPHYR_SOCKET 4
#endif

/* Private global definitions */

static volatile uint32_t priv_P_SOCKET_nbSockets = 0; // Allow to avoid max socket allocation

// *** Socket internal functions definitions

static inline SOPC_ReturnStatus P_SOCKET_Configure(Socket sock, bool setNonBlocking);

// *** Private api functions definitions used by P_SOCKET and P_SOCKET_UDP ***

// Get configuration status. True if configured (ip, gw, mask...) Declared as weak function
__weak bool P_SOCKET_NETWORK_IsConfigured(void)
{
    return true;
}

// Increment socket counter. Can be compared to CONFIG_NET_MAX_CONN - 2
// If result is above this value, user shall close one socket before continue.
// Returns socket counter
uint32_t P_SOCKET_increment_nb_sockets(void)
{
    uint32_t currentValue = 0;
    uint32_t newValue = 0;
    bool bTransition = false;
    do
    {
        currentValue = priv_P_SOCKET_nbSockets;
        newValue = currentValue + 1;
        bTransition = __sync_bool_compare_and_swap(&priv_P_SOCKET_nbSockets, currentValue, newValue);

    } while (!bTransition);
    return newValue;
}

// Decrment socket counter. Can be compared to CONFIG_NET_MAX_CONN - 2
// If result is above this value, user shall close one socket before continue.
// Returns socket counter
uint32_t P_SOCKET_decrement_nb_sockets(void)
{
    uint32_t currentValue = 0;
    uint32_t newValue = 0;
    bool bTransition = false;
    do
    {
        currentValue = priv_P_SOCKET_nbSockets;
        if (currentValue > 0)
        {
            newValue = currentValue - 1;
        }
        else
        {
            newValue = currentValue;
        }
        bTransition = __sync_bool_compare_and_swap(&priv_P_SOCKET_nbSockets, currentValue, newValue);
    } while (!bTransition);
    return newValue;
}

// *** Public SOCKET API functions definitions ***

// Initialize ethernet and loopback interface
// Returns true if well configured.
bool SOPC_Socket_Network_Initialize()
{
    return P_SOCKET_NETWORK_Initialize();
}

// Not implmented, return always true.
bool SOPC_Socket_Network_Clear()
{
    return true;
}

// Create socket address information object. Shall be destroy after used.
SOPC_ReturnStatus SOPC_Socket_AddrInfo_Get(char* hostname,                  // Hostname or address
                                           char* port,                      // Port
                                           SOPC_Socket_AddressInfo** addrs) // Socket address info object
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

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
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return NULL;
    }

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

static inline SOPC_ReturnStatus P_SOCKET_Configure(Socket sock, bool setNonBlocking)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const int trueInt = true;

    // Deactivate Nagle's algorithm since we always write a TCP UA binary message (and not just few bytes)
    int setOptStatus = zsock_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*) &trueInt, sizeof(int));
    if (0 != setOptStatus)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status && setNonBlocking != false)
    {
        setOptStatus = zsock_fcntl(sock, F_SETFL, O_NONBLOCK);
        if (0 != setOptStatus)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_Socket_CreateNew(SOPC_Socket_AddressInfo* addr,
                                        bool setReuseAddr,
                                        bool setNonBlocking,
                                        Socket* sock)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == addr || NULL == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t valAuthorization = P_SOCKET_increment_nb_sockets();
    if (valAuthorization <= MAX_ZEPHYR_SOCKET)
    {
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        int setOptStatus = 0;

        *sock = zsock_socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (SOPC_INVALID_SOCKET == *sock)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = P_SOCKET_Configure(*sock, setNonBlocking);
        }

        if (SOPC_STATUS_OK == status && setReuseAddr != false)
        {
            const int trueInt = true;
            setOptStatus = zsock_setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
            if (0 != setOptStatus)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        // Enforce IPV6 sockets can be used for IPV4 connections (if socket is IPV6)
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
            P_SOCKET_decrement_nb_sockets();
        }

        return status;
    }
    else
    {
        P_SOCKET_decrement_nb_sockets();
    }
    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Socket_Listen(Socket sock, SOPC_Socket_AddressInfo* addr)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int bindListenStatus = -1;
    if (NULL != addr)
    {
        bindListenStatus = zsock_bind(sock, addr->ai_addr, addr->ai_addrlen);
        if (-1 != bindListenStatus)
        {
            bindListenStatus = zsock_listen(sock, MAX_PENDING_CONNECTION);
        }
    }
    if (-1 != bindListenStatus)
    {
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Accept(Socket listeningSock, bool setNonBlocking, Socket* acceptedSock)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    struct sockaddr remoteAddr;
    socklen_t addrLen = 0;
    if (SOPC_INVALID_SOCKET != listeningSock && NULL != acceptedSock)
    {
        uint32_t valAuthorization = P_SOCKET_increment_nb_sockets();
        if (valAuthorization <= MAX_ZEPHYR_SOCKET)
        {
            *acceptedSock = zsock_accept(listeningSock, &remoteAddr, &addrLen);
            if (SOPC_INVALID_SOCKET != *acceptedSock)
            {
                status = P_SOCKET_Configure(*acceptedSock, setNonBlocking);
            }
            else
            {
                P_SOCKET_decrement_nb_sockets();
            }
        }
        else
        {
            do
            {
                valAuthorization = P_SOCKET_increment_nb_sockets();
                if ((MAX_ZEPHYR_SOCKET + 2) >= valAuthorization)
                {
                    *acceptedSock = zsock_accept(listeningSock, &remoteAddr, &addrLen);
                    if (*acceptedSock >= 0)
                    {
                        zsock_close(*acceptedSock);
                    }
                    *acceptedSock = SOPC_INVALID_SOCKET;
                    P_SOCKET_decrement_nb_sockets();
                }
                else
                {
                    P_SOCKET_decrement_nb_sockets();
                    k_yield();
                }

            } while (valAuthorization > (MAX_ZEPHYR_SOCKET + 2));

            P_SOCKET_decrement_nb_sockets();
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Connect(Socket sock, SOPC_Socket_AddressInfo* addr)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == addr || SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    int connectStatus = -1;
    connectStatus = zsock_connect(sock, addr->ai_addr, addr->ai_addrlen);
    if (connectStatus < 0)
    {
        int optErr = 0;
        socklen_t optErrSize = sizeof(optErr);
        int ret = zsock_getsockopt(sock, SOL_SOCKET, SO_ERROR, &optErr, &optErrSize);
        if (ret < 0)
        {
            return SOPC_STATUS_NOK;
        }
        if (EINPROGRESS == optErr)
        {
            // Non blocking connection started
            connectStatus = 0;
        }
    }
    if (0 == connectStatus)
    {
        return SOPC_STATUS_OK;
    }

    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Socket_ConnectToLocal(Socket from, Socket to)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

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
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    int error = 0;
    socklen_t len = sizeof(int);
    int ret = zsock_getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
    if (ret < 0 || 0 != error)
    {
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
}

void SOPC_SocketSet_Add(Socket sock, SOPC_SocketSet* sockSet)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return;
    }

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
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return;
    }

    if (SOPC_INVALID_SOCKET != sock && NULL != sockSet)
    {
        ZSOCK_FD_CLR(sock, &sockSet->set);
    }
}

bool SOPC_SocketSet_IsPresent(Socket sock, SOPC_SocketSet* sockSet)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return false;
    }

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
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return;
    }

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
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

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

    return (int32_t) nbReady;
}

SOPC_ReturnStatus SOPC_Socket_Write(Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || count > INT32_MAX || NULL == sentBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    ssize_t res = 0;
    res = send(sock, data, count, 0);

    if (res >= 0)
    {
        *sentBytes = (uint32_t) res;
        return SOPC_STATUS_OK;
    }

    *sentBytes = 0;
    int optErr = 0;
    socklen_t optErrSize = sizeof(optErr);
    res = zsock_getsockopt(sock, SOL_SOCKET, SO_ERROR, &optErr, &optErrSize);
    if (res >= 0 && (EAGAIN == optErr || EWOULDBLOCK == optErr))
    {
        return SOPC_STATUS_WOULD_BLOCK;
    }

    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Socket_Read(Socket sock, uint8_t* data, uint32_t dataSize, uint32_t* readCount)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || 0 >= dataSize)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    ssize_t sReadCount = 0;
    sReadCount = zsock_recv(sock, data, dataSize, 0);

    if (sReadCount > 0)
    {
        if (readCount != NULL)
            *readCount = (uint32_t) sReadCount;
        return SOPC_STATUS_OK;
    }

    if (0 == sReadCount)
    {
        if (readCount != NULL)
            *readCount = 0;
        return SOPC_STATUS_CLOSED;
    }

    if (readCount != NULL)
        *readCount = 0;
    int optErr = 0;
    socklen_t optErrSize = sizeof(optErr);

    int res = zsock_getsockopt(sock, SOL_SOCKET, SO_ERROR, &optErr, &optErrSize);
    if (res >= 0 && (EAGAIN == optErr || EWOULDBLOCK == optErr))
    {
        return SOPC_STATUS_WOULD_BLOCK;
    }

    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Socket_BytesToRead(Socket sock, uint32_t* bytesToRead)
{
    // TODO: to be implemented
    return SOPC_STATUS_NOK;
}

void SOPC_Socket_Close(Socket* sock)
{
    if (NULL != sock && SOPC_INVALID_SOCKET != *sock && P_SOCKET_NETWORK_IsConfigured())
    {
        zsock_shutdown(*sock, ZSOCK_SHUT_RDWR);
        if (0 == zsock_close(*sock))
        {
            *sock = SOPC_INVALID_SOCKET;
            P_SOCKET_decrement_nb_sockets();
        }
    }
}

// *** Weak functions : mcast and network initialization ***/

// Initialize ethernet and loopback interface
// Returns true if well configured.
__weak bool P_SOCKET_NETWORK_Initialize(void)
{
    return false;
}

// Multicast private function definitions

// Register multicast address to L1 and associate socket to this one.
// Multicast address is added, if not already added, to ethernet interface (L2)
__weak SOPC_ReturnStatus P_SOCKET_MCAST_join_mcast_group(int32_t sock, struct in_addr* pAddr)
{
    return SOPC_STATUS_NOK;
}

// Check if address is a knowned multicast address registered with socket in parameter
__weak bool P_SOCKET_MCAST_soft_filter(int sock, struct in_addr* pAddr)
{
    return false;
}

// Unregister socket from multicast address.
// Remove mcast from L1 if not used by any socket.
__weak SOPC_ReturnStatus P_SOCKET_MCAST_leave_mcast_group(int sock, struct in_addr* add)
{
    return SOPC_STATUS_NOK;
}

// Remove socket from all mcast.
__weak void P_SOCKET_MCAST_remove_sock_from_mcast(int sock)
{
    return;
}

// Add socket to mcast
__weak SOPC_ReturnStatus P_SOCKET_MCAST_add_sock_to_mcast(int sock, struct in_addr* add)
{
    return SOPC_STATUS_NOK;
}
