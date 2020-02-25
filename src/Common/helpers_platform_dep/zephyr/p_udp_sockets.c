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

#ifndef K_FOREVER
#define K_FOREVER ((void*) -1)
#endif

#include "sopc_udp_sockets.h"

#include "p_sockets.h"

#include "p_udp_sockets.h"

#define P_SOCKET_UDP_DEBUG (0)

/* Max socket based on max connections allowed by zephyr */

#ifdef CONFIG_NET_MAX_CONN
#define MAX_UDP_ZEPHYR_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_UDP_ZEPHYR_SOCKET 4
#endif

static SOPC_ReturnStatus P_SOCKET_UDP_CreateSocket(const SOPC_Socket_AddressInfo* pAddrInfo,
                                                   bool setReuseAddr,
                                                   bool setNonBlocking,
                                                   Socket* pSock)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL != pAddrInfo && NULL != pSock)
    {
        uint32_t authVal = P_SOCKET_increment_nb_sockets();

        if (authVal <= MAX_UDP_ZEPHYR_SOCKET)
        {
            Socket sock = zsock_socket(pAddrInfo->ai_family, SOCK_DGRAM, IPPROTO_UDP);

            if (SOPC_INVALID_SOCKET == sock)
            {
                status = SOPC_STATUS_NOK;
            }
            else
            {
                status = SOPC_STATUS_OK;
            }

            if (SOPC_STATUS_OK == status && false != setReuseAddr)
            {
                const int trueInt = true;
                int setOptStatus = -1;
                setOptStatus = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
                if (setOptStatus < 0)
                {
                    status = SOPC_STATUS_NOK;
                }
            }

            if (SOPC_STATUS_OK == status && false != setNonBlocking)
            {
                int setOptStatus = -1;
                setOptStatus = zsock_fcntl(sock, F_SETFL, O_NONBLOCK);
                if (0 != setOptStatus)
                {
                    status = SOPC_STATUS_NOK;
                }
            }

            if (SOPC_STATUS_OK != status)
            {
                if (sock != SOPC_INVALID_SOCKET)
                {
                    zsock_shutdown(sock, ZSOCK_SHUT_RDWR);
                    if (zsock_close(sock) == 0)
                    {
                        sock = SOPC_INVALID_SOCKET;
                        P_SOCKET_decrement_nb_sockets();
                    }
                }
                else
                {
                    P_SOCKET_decrement_nb_sockets();
                }
            }

            *pSock = sock;
        }
        else
        {
            *pSock = SOPC_INVALID_SOCKET;
            P_SOCKET_decrement_nb_sockets();
            status = SOPC_STATUS_NOK;
        }
#if P_SOCKET_UDP_DEBUG == 1
        printk("\r\nP_SOCKET_UDP: Creation of socket %d\r\n", *pSock);
#endif
    }
    else
    {
#if P_SOCKET_UDP_DEBUG == 1
        printk("\r\nP_SOCKET_UDP: Creation of socket error, invalid parameters\r\n");
#endif
    }

    return status;
}

SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return NULL;
    }

    SOPC_Socket_AddressInfo* pRetAddrInfo = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.ai_family = (IPv6 ? AF_INET6 : AF_INET);
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    // If not, add it to default interface
    if ((NULL != node || NULL != service))
    {
        int ret = zsock_getaddrinfo(node, service, &hints, &pRetAddrInfo);
        if (ret < 0)
        {
            pRetAddrInfo = NULL;
            status = SOPC_STATUS_NOK;
        }
        else
        {
            if (!pRetAddrInfo)
            {
                status = SOPC_STATUS_OK;
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }

    return pRetAddrInfo;
}

void SOPC_UDP_SocketAddress_Delete(SOPC_Socket_AddressInfo** pptrAddrInfo)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return;
    }

    if (NULL != pptrAddrInfo)
    {
        zsock_freeaddrinfo(*pptrAddrInfo);
        *pptrAddrInfo = NULL;
    }
}

SOPC_ReturnStatus SOPC_UDP_Socket_Set_MulticastTTL(Socket sock, uint8_t TTL_scope)
{
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_UDP_Socket_AddMembership(Socket sock,
                                                const SOPC_Socket_AddressInfo* multicast,
                                                const SOPC_Socket_AddressInfo* local)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (multicast == NULL || (local != NULL && local->ai_family != multicast->ai_family))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (multicast->ai_family)
    {
    case AF_INET:
    {
        struct in_addr* multiAddr = &((struct sockaddr_in*) &multicast->_ai_addr)->sin_addr;

        if (!net_ipv4_is_addr_mcast(multiAddr))
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = P_SOCKET_MCAST_join_mcast_group(sock, multiAddr);
        }
    }
    break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_DropMembership(Socket sock,
                                                 const SOPC_Socket_AddressInfo* multicast,
                                                 const SOPC_Socket_AddressInfo* local)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (multicast == NULL || (local != NULL && local->ai_family != multicast->ai_family))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (multicast->ai_family)
    {
    case AF_INET:
    {
        struct in_addr* multiAddr = &((struct sockaddr_in*) &multicast->_ai_addr)->sin_addr;

        if (!net_ipv4_is_addr_mcast(multiAddr))
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = P_SOCKET_MCAST_leave_mcast_group(sock, multiAddr);
        }
    }
    break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  bool setReuseAddr,
                                                  Socket* pSock)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == pSock || NULL == listenAddress)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    Socket sock = SOPC_INVALID_SOCKET;
    SOPC_ReturnStatus status = P_SOCKET_UDP_CreateSocket(listenAddress, setReuseAddr, true, &sock);
    if (SOPC_STATUS_OK == status)
    {
        if (listenAddress->ai_family == AF_INET)
        {
            // add mcast for bind
            status =
                P_SOCKET_MCAST_add_sock_to_mcast(sock, &((struct sockaddr_in*) &listenAddress->_ai_addr)->sin_addr);
            if (SOPC_STATUS_OK == status)
            {
                int res = zsock_bind(sock, &listenAddress->_ai_addr, listenAddress->ai_addrlen);
                if (res < 0)
                {
                    P_SOCKET_MCAST_remove_sock_from_mcast(sock);
                    SOPC_UDP_Socket_Close(&sock);
                    status = SOPC_STATUS_NOK;
                }
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    *pSock = sock;

    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress, Socket* pSock)
{
    return P_SOCKET_UDP_CreateSocket(destAddress, false, false, pSock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_INVALID_SOCKET == sock || NULL == destAddr || NULL == buffer || buffer->position != 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    ssize_t send_length = zsock_sendto(sock, buffer->data,    //
                                       buffer->length,        //
                                       0,                     //
                                       &destAddr->_ai_addr,   //
                                       destAddr->ai_addrlen); //
#if P_SOCKET_UDP_DEBUG == 1
    printk("\r\nP_SOCKET_UDP: SendTo on %d : sent_length = %d\r\n", //
           sock,                                                    //
           send_length);                                            //
#endif

    if (send_length < 0 || (uint32_t) send_length != buffer->length)
    {
        status = SOPC_STATUS_NOK;
    }

    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(Socket sock, SOPC_Buffer* buffer)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_INVALID_SOCKET == sock || NULL == buffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    struct sockaddr_in saddr;
    socklen_t slen = sizeof(saddr);
    bool shallBeDropped = false;
    struct sockaddr_in saddr2;
    socklen_t slen2 = sizeof(saddr2);
    ssize_t recv_len = 0;
    int ret = -1;
    int fl = -1;
    bool bLoop = false;
    do
    {
        slen = sizeof(saddr);
        memset(&saddr, 0, slen);

        SOPC_Buffer_Reset(buffer);

        recv_len = recvfrom(sock,                      //
                            buffer->data,              //
                            buffer->maximum_size,      //
                            0,                         //
                            (struct sockaddr*) &saddr, //
                            &slen);                    //
        if (recv_len >= 0)
        {
            slen2 = sizeof(saddr2);
            memset(&saddr2, 0, slen2);

            // Check if binding on mcast @.
            ret = zsock_getsockname(sock, (struct sockaddr*) &saddr2, &slen2);
            if ((saddr2.sin_family == AF_INET) && (ret == 0))
            {
                if (net_ipv4_is_addr_mcast(&saddr2.sin_addr))
                {
                    shallBeDropped = !P_SOCKET_MCAST_soft_filter(sock, &saddr2.sin_addr);
#if P_SOCKET_UDP_DEBUG == 1
                    {
                        char addr_str[32];
                        inet_ntop(AF_INET, &saddr.sin_addr, addr_str, sizeof(addr_str));
                        printk(
                            "\r\nP_SOCKET_UDP: receive ip = %s port = %d - dropped : %d - check block mode enabled\r\n", //
                            addr_str,              //
                            ntohs(saddr.sin_port), //
                            shallBeDropped);       //
                    }
#endif
                    if (shallBeDropped)
                    {
                        fl = -1;

                        fl = fcntl(sock, F_GETFL, 0);
                        assert(-1 != fl);

                        if ((fl & O_NONBLOCK) == 0)
                        {
                            bLoop = true;
                        }
                    }
                }
            }
        }
    } while (bLoop);

#if P_SOCKET_UDP_DEBUG == 1
    {
        char addr_str[32];
        inet_ntop(AF_INET, &saddr.sin_addr, addr_str, sizeof(addr_str));
        printk("\r\nP_SOCKET_UDP: receive ip = %s port = %d - dropped : %d\r\n", //
               addr_str,                                                         //
               ntohs(saddr.sin_port),                                            //
               shallBeDropped);                                                  //
    }
#endif

    if (recv_len < 0 || shallBeDropped)
    {
        SOPC_Buffer_Reset(buffer);
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        buffer->length = (uint32_t) recv_len;
        if (buffer->length == buffer->maximum_size)
        {
            // The message could be incomplete
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    return status;
}

void SOPC_UDP_Socket_Close(Socket* pSock)
{
    if (!P_SOCKET_NETWORK_IsConfigured())
    {
        return;
    }

    if (NULL != pSock && SOPC_INVALID_SOCKET != *pSock)
    {
        Socket sock = *pSock;
        zsock_shutdown(sock, ZSOCK_SHUT_RDWR);
        if (zsock_close(sock) == 0)
        {
            P_SOCKET_MCAST_remove_sock_from_mcast(sock);
#if P_SOCKET_UDP_DEBUG == 1
            printk("\r\nP_SOCKET_UDP: Socket closed : %d\r\n", *pSock);
#endif
            *pSock = SOPC_INVALID_SOCKET;
            P_SOCKET_decrement_nb_sockets();
        }
    }
}

// Multicast private function definitions

// Register multicast address to L1 and associate socket to this one.
// Multicast address is added, if not already added, to ethernet interface (L2)
SOPC_ReturnStatus __attribute__((weak)) P_SOCKET_MCAST_join_mcast_group(int32_t sock, struct in_addr* pAddr)
{
    return SOPC_STATUS_NOK;
}

// Check if address is a knowned multicast address registered with socket in parameter
bool __attribute__((weak)) P_SOCKET_MCAST_soft_filter(int32_t sock, struct in_addr* pAddr)
{
    return false;
}

// Unregister socket from multicast address.
// Remove mcast from L1 if not used by any socket.
SOPC_ReturnStatus __attribute__((weak)) P_SOCKET_MCAST_leave_mcast_group(int32_t sock, struct in_addr* add)
{
    return SOPC_STATUS_NOK;
}

// Remove socket from all mcast.
void __attribute__((weak)) P_SOCKET_MCAST_remove_sock_from_mcast(int32_t sock)
{
    return;
}

// Add socket to mcast
SOPC_ReturnStatus __attribute__((weak)) P_SOCKET_MCAST_add_sock_to_mcast(int32_t sock, struct in_addr* add)
{
    return SOPC_STATUS_NOK;
}
