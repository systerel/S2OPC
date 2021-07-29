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
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __INT32_MAX__
#include <toolchain/xcc_missing_defs.h>
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

#include <fcntl.h>
#include <kernel.h>
#include <net/socket.h>

#include "sopc_mutexes.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"

#include "p_sockets.h"

#define P_SOCKET_DEBUG (0)

static volatile uint32_t priv_nbSockets = 0;

uint32_t P_SOCKET_increment_nb_socket(void)
{
    uint32_t currentValue = 0;
    uint32_t newValue = 0;
    bool bTransition = false;
    do
    {
        currentValue = priv_nbSockets;

        newValue = currentValue + 1;

        bTransition = __sync_bool_compare_and_swap(&priv_nbSockets, currentValue, newValue);

    } while (!bTransition);
#if P_SOCKET_DEBUG == 1
    printk("\r\nP_SOCKET =====> o socket reservation exit - new value = %d\r\n", newValue);
#endif
    return newValue;
}

uint32_t P_SOCKET_decrement_nb_socket(void)
{
    uint32_t currentValue = 0;
    uint32_t newValue = 0;
    bool bTransition = false;
    do
    {
        currentValue = priv_nbSockets;

        if (currentValue > 0)
        {
            newValue = currentValue - 1;
        }
        else
        {
            newValue = currentValue;
        }

        bTransition = __sync_bool_compare_and_swap(&priv_nbSockets, currentValue, newValue);

    } while (!bTransition);
#if P_SOCKET_DEBUG == 1
    printk("\r\nP_SOCKET =====> + socket release exit - new value = %d\r\n", newValue);
#endif
    return newValue;
}

bool SOPC_Socket_Network_Initialize()
{
    return true;
}

bool SOPC_Socket_Network_Clear()
{
    return true;
}

SOPC_ReturnStatus SOPC_Socket_AddrInfo_Get(char* hostname, char* port, SOPC_Socket_AddressInfo** addrs)
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
    return addr->ai_family == PF_INET6;
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

static SOPC_ReturnStatus Socket_Configure(Socket sock, bool setNonBlocking)
{
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
    if (NULL == addr || NULL == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t valAuthorization = P_SOCKET_increment_nb_socket();
    if (valAuthorization <= MAX_SOCKET)
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
            status = Socket_Configure(*sock, setNonBlocking);
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
            P_SOCKET_decrement_nb_socket();
        }

        return status;
    }
    else
    {
#if P_SOCKET_DEBUG == 1
        printk("\r\nP_SOCKET: Maximum sock reached, create refused !!!\r\n");
#endif
        P_SOCKET_decrement_nb_socket();
    }
    return SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Socket_Listen(Socket sock, SOPC_Socket_AddressInfo* addr)
{
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
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    struct sockaddr remoteAddr;
    socklen_t addrLen = 0;
    if (SOPC_INVALID_SOCKET != listeningSock && NULL != acceptedSock)
    {
        uint32_t valAuthorization = P_SOCKET_increment_nb_socket();
        if (valAuthorization <= MAX_SOCKET)
        {
            *acceptedSock = zsock_accept(listeningSock, &remoteAddr, &addrLen);
            if (SOPC_INVALID_SOCKET != *acceptedSock)
            {
                status = Socket_Configure(*acceptedSock, setNonBlocking);
            }
            else
            {
                P_SOCKET_decrement_nb_socket();
            }
        }
        else
        {
            do
            {
                valAuthorization = P_SOCKET_increment_nb_socket();
                if ((MAX_SOCKET + 2) >= valAuthorization)
                {
                    *acceptedSock = zsock_accept(listeningSock, &remoteAddr, &addrLen);
                    if (*acceptedSock >= 0)
                    {
                        zsock_close(*acceptedSock);
                    }
                    *acceptedSock = -1;
                    P_SOCKET_decrement_nb_socket();
                }
                else
                {
#if P_SOCKET_DEBUG == 1
                    printk("\r\nsopc Accept to close can't be performed !!! Yield !!!\r\n");
#endif
                    P_SOCKET_decrement_nb_socket();
                    k_yield();
                }

            } while (valAuthorization > (MAX_SOCKET + 2));

            P_SOCKET_decrement_nb_socket();
#if P_SOCKET_DEBUG == 1
            printk("\r\nP_SOCKET: Max socket reach, accept failed !!!\r\n");
#endif
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Socket_Connect(Socket sock, SOPC_Socket_AddressInfo* addr)
{
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
    if (connectStatus == 0)
    {
        return SOPC_STATUS_OK;
    }

    return SOPC_STATUS_NOK;
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

    if (readSet == NULL && writeSet == NULL)
    {
        return -1;
    }

    if ((readSet != NULL) && (writeSet == NULL || readSet->fdmax > writeSet->fdmax))
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

    if (waitMs == 0)
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
    printk("\r\nselect result = %d\r\n", nbReady);
    return (int32_t) nbReady;
}

SOPC_ReturnStatus SOPC_Socket_Write(Socket sock, const uint8_t* data, uint32_t count, uint32_t* sentBytes)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == data || count > INT32_MAX || sentBytes == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
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

    ssize_t sReadCount = 0;
    sReadCount = zsock_recv(sock, data, dataSize, 0);

    if (sReadCount > 0)
    {
        if (readCount != NULL)
            *readCount = (uint32_t) sReadCount;
        return SOPC_STATUS_OK;
    }

    if (sReadCount == 0)
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
    if (NULL != sock && SOPC_INVALID_SOCKET != *sock)
    {
        zsock_shutdown(*sock, ZSOCK_SHUT_RDWR);
        if (zsock_close(*sock) == 0)
        {
            *sock = SOPC_INVALID_SOCKET;
            P_SOCKET_decrement_nb_socket();
        }
    }
}
