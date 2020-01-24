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
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "kernel.h"
#include "net/ethernet.h"
#include "net/net_if.h"
#include "net/socket.h"
#include "sys/printk.h"

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

#ifndef MY_IP_LB
#define MY_IP_LB ((const char*) ("127.0.0.1"))
#endif

#ifndef MY_IP_ETH0
#define MY_IP_ETH0 ((const char*) ("192.168.1.102"))
#endif

#ifndef MY_IP_MASK
#define MY_IP_MASK ((const char*) ("255.255.255.0"))
#endif

#ifndef MY_IP_GW
#define MY_IP_GW ((const char*) ("192.168.1.111"))
#endif

/* s2opc includes */

#include "sopc_mutexes.h"

/* platform dep includes */

#include "p_sockets.h"

/* debug printk activation*/

#define P_SOCKET_DEBUG (0)
#define P_SOCKET_MCAST_DEBUG (0)

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

/* Constant definitions MAX_MCAST, based on value allowed by zephyr configuration */
#ifdef CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT
#define MAX_MCAST CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT
#else
#define MAX_MCAST 16
#endif

typedef enum E_NETWORK_CONFIG_STATUS
{
    NETWORK_CONFIG_STATUS_NOT_INITIALIZED,
    NETWORK_CONFIG_STATUS_INITIALIZING,
    NETWORK_CONFIG_STATUS_INITIALIZED,
    NETWORK_CONFIG_STATUS_SIZE = INT32_MAX
} eNetworkConfigStatus;

/* Private global definitions */

static volatile uint32_t priv_P_SOCKET_nbSockets = 0; // Allow to avoid max socket allocation
static volatile eNetworkConfigStatus priv_P_SOCKET_networkConfigStatus = NETWORK_CONFIG_STATUS_NOT_INITIALIZED;

// *** Multicast variable definitions ***

static Mutex priv_lockL2; // tabMCast protection
struct mCastRegistered
{
    int sock[MAX_ZEPHYR_SOCKET]; // Socket associated to mcast addr
    struct in_addr addr;         // Mcast addr
    bool bIsJoined;              // Mcast in use
};

static struct mCastRegistered tabMCast[MAX_MCAST];

// *** Multicast internal functions declarations ***

static void inline P_SOCKET_LockMcastTable(void);
static void inline P_SOCKET_UnLockMcastTable(void);

// *** Multicast internal functions definitions ***

static void inline P_SOCKET_LockMcastTable(void)
{
    Mutex_Lock(&priv_lockL2);
}

static void inline P_SOCKET_UnLockMcastTable(void)
{
    Mutex_Unlock(&priv_lockL2);
}

// *** Socket internal functions definitions

static inline SOPC_ReturnStatus P_SOCKET_Configure(Socket sock, bool setNonBlocking);

// *** Private api functions definitions used by P_SOCKET and P_SOCKET_UDP ***

// Get configuration status. True if configured (ip, gw, mask...)
bool P_SOCKET_NETWORK_IsConfigured(void)
{
    return (NETWORK_CONFIG_STATUS_INITIALIZED == priv_P_SOCKET_networkConfigStatus);
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
#if P_SOCKET_DEBUG == 1
    printk("\r\nP_SOCKET =====> o socket reservation exit - new value = %d\r\n", newValue);
#endif
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
#if P_SOCKET_DEBUG == 1
    printk("\r\nP_SOCKET =====> + socket release exit - new value = %d\r\n", newValue);
#endif
    return newValue;
}

// *** Public SOCKET API functions definitions ***

// Initialize ethernet and loopback interface
// Returns true if well configured.
bool SOPC_Socket_Network_Initialize()
{
    uint32_t nwStatus = 0;

    do
    {
        nwStatus = __sync_val_compare_and_swap(&priv_P_SOCKET_networkConfigStatus,    //
                                               NETWORK_CONFIG_STATUS_NOT_INITIALIZED, // Not initialized
                                               NETWORK_CONFIG_STATUS_INITIALIZING);   // Initializing

        if (0 == nwStatus)
        {
            assert(SOPC_STATUS_OK == Mutex_Initialization(&priv_lockL2));

            struct net_if* ptrNetIf = NULL;
            struct in_addr addressLoopBack;
            struct in_addr addressLoopBackNetMask;
            struct in_addr addressInterfaceEth;
            struct in_addr addressInterfaceEthMask;
            struct in_addr addressInterfaceEthGtw;
            net_addr_pton(AF_INET, MY_IP_LB, (void*) &addressLoopBack);
            net_addr_pton(AF_INET, MY_IP_MASK, (void*) &addressLoopBackNetMask);

            net_addr_pton(AF_INET, MY_IP_ETH0, (void*) &addressInterfaceEth);
            net_addr_pton(AF_INET, MY_IP_MASK, (void*) &addressInterfaceEthMask);
            net_addr_pton(AF_INET, MY_IP_GW, (void*) &addressInterfaceEthGtw);
            if (NULL == net_if_ipv4_addr_lookup(&addressLoopBack, &ptrNetIf))
            {
#if defined(CONFIG_NET_L2_DUMMY)
                ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(DUMMY));
#endif
                assert(NULL != ptrNetIf);
                assert(NULL != net_if_ipv4_addr_add(ptrNetIf, &addressLoopBack, NET_ADDR_MANUAL, 0));
                net_if_ipv4_set_netmask(ptrNetIf, &addressLoopBackNetMask);
#if P_SOCKET_DEBUG == 1
                printk("\r\nP_SOCKET: Loopback initialized\r\n");
#endif
            }
            else
            {
#if P_SOCKET_DEBUG == 1
                printk("\r\nP_SOCKET: Loopback already initialized\r\n");
#endif
            }

            if (NULL == net_if_ipv4_addr_lookup(&addressInterfaceEth, &ptrNetIf))
            {
#if defined(CONFIG_NET_L2_ETHERNET)
                ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
                assert(NULL != ptrNetIf);
                assert(NULL != net_if_ipv4_addr_add(ptrNetIf, &addressInterfaceEth, NET_ADDR_MANUAL, 0));
                net_if_ipv4_set_gw(ptrNetIf, &addressInterfaceEthGtw);
                net_if_ipv4_set_netmask(ptrNetIf, &addressInterfaceEthMask);
#if P_SOCKET_DEBUG == 1
                printk("\r\nP_SOCKET: Eth0 initialized\r\n");
#endif
            }
            else
            {
#if P_SOCKET_DEBUG == 1
                printk("\r\nP_SOCKET: Eth0 already initialized\r\n");
#endif
            }

            priv_P_SOCKET_networkConfigStatus = NETWORK_CONFIG_STATUS_INITIALIZED;
            nwStatus = NETWORK_CONFIG_STATUS_INITIALIZED;
        }
        else
        {
            // Initializing on going, yield and retry
            if (NETWORK_CONFIG_STATUS_INITIALIZING == nwStatus)
            {
                k_yield();
            }
        }
    } while (NETWORK_CONFIG_STATUS_INITIALIZING == nwStatus);

    // Initialized or not initialized
    if (NETWORK_CONFIG_STATUS_INITIALIZED == nwStatus)
    {
        return true;
    }
    else
    {
        return false;
    }
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
#if P_SOCKET_DEBUG == 1
        printk("\r\nP_SOCKET: Maximum sock reached, create refused !!!\r\n");
#endif
        P_SOCKET_decrement_nb_sockets();
    }
    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Socket_Listen(Socket sock, SOPC_Socket_AddressInfo* addr)
{
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
#if P_SOCKET_DEBUG == 1
                    printk("\r\nP_SOCKET: Sopc Accept to close can't be performed !!! Yield !!!\r\n");
#endif
                    P_SOCKET_decrement_nb_sockets();
                    k_yield();
                }

            } while (valAuthorization > (MAX_ZEPHYR_SOCKET + 2));

            P_SOCKET_decrement_nb_sockets();
#if P_SOCKET_DEBUG == 1
            printk("\r\nP_SOCKET: Max socket reach, accept failed !!!\r\n");
#endif
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Connect(Socket sock, SOPC_Socket_AddressInfo* addr)
{
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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

#if P_SOCKET_DEBUG == 1
    printk("\r\nP_SOCKET: Sopc select result = %d\r\n", nbReady);
#endif
    return (int32_t) nbReady;
}

SOPC_ReturnStatus SOPC_Socket_Write(Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || count > INT32_MAX || NULL == sentBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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

    if (priv_P_SOCKET_networkConfigStatus != NETWORK_CONFIG_STATUS_INITIALIZED)
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
    if (NULL != sock && SOPC_INVALID_SOCKET != *sock &&
        NETWORK_CONFIG_STATUS_INITIALIZED == priv_P_SOCKET_networkConfigStatus)
    {
        zsock_shutdown(*sock, ZSOCK_SHUT_RDWR);
        if (0 == zsock_close(*sock))
        {
            *sock = SOPC_INVALID_SOCKET;
            P_SOCKET_decrement_nb_sockets();
        }
    }
}

// *** Multicast private function definitions

#if P_SOCKET_MCAST_DEBUG == 1
static inline void P_SOCKET_MCAST_print_sock_from_mcast(void);
#endif

// Register multicast address to L1 and associate socket to this one.
// Multicast address is added, if not already added, to ethernet interface (L2)
SOPC_ReturnStatus P_SOCKET_MCAST_join_mcast_group(int32_t sock, struct in_addr* pAddr)
{
    uint32_t indexMacast = 0;
    uint32_t indexMasock = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Enter join_mcast_group\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_INVALID_SOCKET == sock || NULL == pAddr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!net_ipv4_is_addr_mcast(pAddr))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    P_SOCKET_LockMcastTable();

    // Search for already registered mcast
    for (int i = 0; i < MAX_MCAST; i++)
    {
        if ((tabMCast[i].addr.s_addr == pAddr->s_addr) && (tabMCast[i].addr.s_addr != 0))
        {
            bIsFound = true;
            indexMacast = i;
            break;
        }
    }

    // If not registered mcast, add it
    if (!bIsFound)
    {
        status = P_SOCKET_MCAST_add_sock_to_mcast(sock, pAddr);
        if (SOPC_STATUS_OK == status)
        {
            // Search mcast
            for (int i = 0; i < MAX_MCAST; i++)
            {
                if ((tabMCast[i].addr.s_addr == pAddr->s_addr) && (tabMCast[i].addr.s_addr != 0))
                {
                    bIsFound = true;
                    indexMacast = i;
                    break;
                }
            }
        }
    }

    // Search for already registered socket
    if (bIsFound)
    {
        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if ((tabMCast[indexMacast].sock[j] == sock) && (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET))
            {
                bSockIsFound = true;
                indexMasock = j;
                break;
            }
        }
    }

    // If socket not found, add it
    if (!bSockIsFound)
    {
        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if (SOPC_INVALID_SOCKET == tabMCast[indexMacast].sock[j])
            {
                tabMCast[indexMacast].sock[j] = sock;
                bSockIsFound = true;
                indexMasock = j;
#if P_SOCKET_MCAST_DEBUG == 1
                {
                    char addr_str[32];
                    inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                    printk("\r\nP_SOCKET_UDP: register socket = %d for mcast ip = %s\r\n", //
                           sock,                                                           //
                           addr_str);                                                      //
                }
#endif
                break;
            }
        }
    }

    // Verify mcast is already registered by L1
    if (bIsFound && bSockIsFound)
    {
#if P_SOCKET_MCAST_DEBUG == 1
        {
            char addr_str[32];
            inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
            printk("\r\nP_SOCKET_UDP: verify for socket = %d if mcast ip = %s is already joined\r\n", //
                   sock,                                                                              //
                   addr_str);                                                                         //
        }
#endif

        if (!tabMCast[indexMacast].bIsJoined)
        {
#if P_SOCKET_MCAST_DEBUG == 1
            {
                char addr_str[32];
                inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                printk("\r\nP_SOCKET_UDP: register mcast ip = %s to L1 (first register) for socket = %d\r\n", //
                       addr_str,                                                                              //
                       sock);                                                                                 //
            }
#endif
            // Register L1 multicast
            struct net_if* ptrNetIf = NULL;
#if defined(CONFIG_NET_L2_ETHERNET)
            ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
            if (ptrNetIf != NULL)
            {
                ethernet_add_mcast(ptrNetIf, pAddr);
#if P_SOCKET_MCAST_DEBUG == 1
                printk("\r\nP_SOCKET_UDP: Call ethernet mcast\r\n");
#endif
            }
            tabMCast[indexMacast].bIsJoined = true;
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    P_SOCKET_UnLockMcastTable();

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Exit join_mcast_group\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif
    return status;
}

// Check if address is a knowned multicast address registered with socket in parameter
bool P_SOCKET_MCAST_soft_filter(int sock, struct in_addr* pAddr)
{
    uint32_t indexMacast = 0;
    uint32_t indexMasock = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;
    bool bResult = false;

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Enter verify mcast group\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif

    if (SOPC_INVALID_SOCKET == sock || NULL == pAddr)
    {
        return false;
    }

    if (!net_ipv4_is_addr_mcast(pAddr))
    {
        return false;
    }

    P_SOCKET_LockMcastTable();
    // Search mcast
    for (int i = 0; i < MAX_MCAST; i++)
    {
        if ((tabMCast[i].addr.s_addr == pAddr->s_addr) && (tabMCast[i].addr.s_addr != 0))
        {
            bIsFound = true;
            indexMacast = i;
            break;
        }
    }

    // Search sock
    if (bIsFound)
    {
        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if ((tabMCast[indexMacast].sock[j] == sock) && (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET))
            {
                bSockIsFound = true;
                indexMasock = j;
                break;
            }
        }
    }

    // If mcast and sock found, remove sock
    // Remove mcast from L2 and L1 is not further used
    bResult = (bIsFound && bSockIsFound);

    P_SOCKET_UnLockMcastTable();

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Exit verify mcast group\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif
    return bResult;
}

// Unregister socket from multicast address.
// Remove mcast from L1 if not used by any socket.
SOPC_ReturnStatus P_SOCKET_MCAST_leave_mcast_group(int sock, struct in_addr* add)
{
    uint32_t indexMacast = 0;
    uint32_t indexMasock = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Enter leave mcast group\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_INVALID_SOCKET == sock || NULL == add)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!net_ipv4_is_addr_mcast(add))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    P_SOCKET_LockMcastTable();

    // Search mcast
    for (int i = 0; i < MAX_MCAST; i++)
    {
        if ((tabMCast[i].addr.s_addr == add->s_addr) && (tabMCast[i].addr.s_addr != 0))
        {
            bIsFound = true;
            indexMacast = i;
            break;
        }
    }

    // Search sock
    if (bIsFound)
    {
        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if ((tabMCast[indexMacast].sock[j] == sock) && (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET))
            {
                bSockIsFound = true;
                indexMasock = j;
                break;
            }
        }
    }

    // If mcast and sock found, remove sock
    // Remove mcast from L2 and L1 is not further used
    if (bIsFound && bSockIsFound)
    {
        tabMCast[indexMacast].sock[indexMasock] = SOPC_INVALID_SOCKET;
        bSockIsFound = false;

        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET)
            {
                bSockIsFound = true;
                indexMasock = j;
                break;
            }
        }
#if P_SOCKET_MCAST_DEBUG == 1
        {
            char addr_str[32];
            inet_ntop(AF_INET, add, addr_str, sizeof(addr_str));
            printk("\r\nP_SOCKET_UDP: remove socket = %d for mcast ip = %s\r\n", //
                   sock,                                                         //
                   addr_str                                                      //
            );                                                                   //
        }
#endif
        if (!bSockIsFound)
        {
#if P_SOCKET_MCAST_DEBUG == 1
            {
                char addr_str[32];
                inet_ntop(AF_INET, add, addr_str, sizeof(addr_str));
                printk(
                    "\r\nP_SOCKET_UDP: remove mcast ip = %s (L1 and L2) for socket = %d because no more joined \r\n", //
                    addr_str,                                                                                         //
                    sock                                                                                              //
                );                                                                                                    //
            }
#endif
            // Unregister L1 and L2 multicast
            struct net_if* ptrNetIf = NULL;
#if defined(CONFIG_NET_L2_ETHERNET)
            ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
            if (ptrNetIf != NULL)
            {
                net_if_ipv4_maddr_rm(ptrNetIf, add);
                ethernet_rm_mcast(ptrNetIf, add);
            }

            tabMCast[indexMacast].bIsJoined = false;
            tabMCast[indexMacast].addr.s_addr = 0;
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    P_SOCKET_UnLockMcastTable();

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Exit leave mcast group\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif
    return status;
}

#if P_SOCKET_MCAST_DEBUG == 1
void P_SOCKET_MCAST_print_sock_from_mcast(void)
{
    // Search mcast
    for (int i = 0; i < 1; i++)
    {
        printk("\r\nP_SOCKET_UDP: tabMCast[%d].addr.s_addr=%d\r\n", i, tabMCast[i].addr.s_addr);
        for (int j = 0; j < 2; j++)
        {
            printk("\r\nP_SOCKET_UDP: tabMCast[%d].sock[%d]=%d\r\n", i, j, tabMCast[i].sock[j]);
        }
    }
}
#endif

// Remove socket from all mcast.
void P_SOCKET_MCAST_remove_sock_from_mcast(int sock)
{
#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Enter remove_sock_from_mcast\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif
    if (SOPC_INVALID_SOCKET == sock)
    {
        return;
    }

    // Remove socket from mcast and remove mcast if necessary
    for (int i = 0; i < MAX_MCAST; i++)
    {
        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            (void) P_SOCKET_MCAST_leave_mcast_group(sock, &tabMCast[i].addr);
        }
    }
#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Exit remove_sock_from_mcast\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif
}

// Add socket to mcast
SOPC_ReturnStatus P_SOCKET_MCAST_add_sock_to_mcast(int sock, struct in_addr* add)
{
#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Enter add_sock_to_mcast\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif

    if (SOPC_INVALID_SOCKET == sock || NULL == add)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t indexMacast = 0;
    uint32_t indexMasock = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!net_ipv4_is_addr_mcast(add))
    {
        return SOPC_STATUS_OK;
    }

    P_SOCKET_LockMcastTable();

    // Search for mcast slot already registered
    for (int i = 0; i < MAX_MCAST; i++)
    {
        if ((tabMCast[i].addr.s_addr == add->s_addr) && (tabMCast[i].addr.s_addr != 0))
        {
            bIsFound = true;
            indexMacast = i;
            break;
        }
    }

    if (!bIsFound)
    {
        for (int i = 0; i < MAX_MCAST; i++)
        {
            if (tabMCast[i].addr.s_addr == 0)
            {
#if P_SOCKET_MCAST_DEBUG == 1
                {
                    char addr_str[32];
                    inet_ntop(AF_INET, add, addr_str, sizeof(addr_str));
                    printk(
                        "\r\nP_SOCKET_UDP: add new mcast ip = %s (L2) for to allow bind socket = %d"
                        "\r\n",   //
                        addr_str, //
                        sock      //
                    );            //
                }
#endif

                // Register L2 multicast to allow bind

                struct net_if* ptrNetIf = NULL;

#if defined(CONFIG_NET_L2_ETHERNET)
                ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
                if (ptrNetIf != NULL)
                {
                    net_if_ipv4_maddr_add(ptrNetIf, add);
                }

                tabMCast[i].addr.s_addr = add->s_addr;
                tabMCast[i].bIsJoined = false;

                for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
                {
                    tabMCast[i].sock[j] = SOPC_INVALID_SOCKET;
                }

                bIsFound = true;
                indexMacast = i;
                indexMasock = 0;
                break;
            }
        }
    }
    else
    {
#if P_SOCKET_MCAST_DEBUG == 1
        printk("\r\nP_SOCKET_UDP: add_sock_to_mcast / mcast already registered \r\n");
#endif
        // Search if already registered
        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if (tabMCast[indexMacast].sock[j] == sock)
            {
#if P_SOCKET_MCAST_DEBUG == 1
                printk("\r\nP_SOCKET_UDP: add_sock_to_mcast / sock already registered \r\n");
#endif
                bSockIsFound = true;
                indexMasock = j;
                break;
            }
        }

        if (!bSockIsFound)
        {
            for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
            {
                if (tabMCast[indexMacast].sock[j] == SOPC_INVALID_SOCKET)
                {
                    indexMasock = j;
                    break;
                }
            }
        }
    }

    if (bIsFound && !bSockIsFound)
    {
#if P_SOCKET_MCAST_DEBUG == 1
        {
            char addr_str[32];

            inet_ntop(AF_INET, add, addr_str, sizeof(addr_str));

            printk("\r\nP_SOCKET_UDP: add new socket = %d to mcast = %s \r\n", //
                   sock,                                                       //
                   addr_str                                                    //
            );                                                                 //
        }
#endif
        tabMCast[indexMacast].sock[indexMasock] = sock;
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    P_SOCKET_UnLockMcastTable();

#if P_SOCKET_MCAST_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: Exit add_sock_to_mcast\r\n");
    P_SOCKET_MCAST_print_sock_from_mcast();
#endif

    return status;
}
