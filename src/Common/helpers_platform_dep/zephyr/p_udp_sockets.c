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
#include <fcntl.h>
#include <stdlib.h>

#include "kernel.h"
#include "net/ethernet.h"
#include "net/net_if.h"
#include "net/socket.h"

#include "sopc_udp_sockets.h"

#include "p_multicast.h"

#include "p_sockets.h"

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
    }

    return status;
}

SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service)
{
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
            status = P_MULTICAST_join_or_leave_mcast_group(sock, multiAddr, true);
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
            status = P_MULTICAST_join_or_leave_mcast_group(sock, multiAddr, false);
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
            status = P_MULTICAST_join_or_leave_mcast_group(
                sock, &((struct sockaddr_in*) &listenAddress->_ai_addr)->sin_addr, true);
            if (SOPC_STATUS_OK == status)
            {
                int res = zsock_bind(sock, &listenAddress->_ai_addr, listenAddress->ai_addrlen);
                if (res < 0)
                {
                    printf("\r\nBIND ERROR\r\n");
                    P_MULTICAST_remove_sock_from_mcast(sock);
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

    if (send_length < 0 || (uint32_t) send_length != buffer->length)
    {
        status = SOPC_STATUS_NOK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(Socket sock, SOPC_Buffer* buffer)
{
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
                    shallBeDropped = !P_MULTICAST_soft_filter(sock, &saddr2.sin_addr);

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
    if (NULL != pSock && SOPC_INVALID_SOCKET != *pSock)
    {
        Socket sock = *pSock;
        zsock_shutdown(sock, ZSOCK_SHUT_RDWR);
        if (zsock_close(sock) == 0)
        {
            P_MULTICAST_remove_sock_from_mcast(sock);
            *pSock = SOPC_INVALID_SOCKET;
            P_SOCKET_decrement_nb_sockets();
        }
    }
}
