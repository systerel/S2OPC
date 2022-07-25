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

#include "sopc_udp_sockets.h"

#include "p_multicast.h"
#include "p_sockets.h"

static SOPC_ReturnStatus P_SOCKET_UDP_CreateSocket(const SOPC_Socket_AddressInfo* pAddrInfo,
                                                   const char* interfaceName,
                                                   bool setReuseAddr,
                                                   bool setNonBlocking,
                                                   Socket* pSock)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    int setOptStatus = -1;

    if (NULL != pAddrInfo && NULL != pSock)
    {
        *pSock = SOPC_INVALID_SOCKET;
        status = SOPC_STATUS_NOK;

        Socket sock = zsock_socket(pAddrInfo->ai_family, SOCK_DGRAM, IPPROTO_UDP);

        if (SOPC_INVALID_SOCKET != sock)
        {
            status = SOPC_STATUS_OK;
        }

        if (SOPC_STATUS_OK == status && setReuseAddr)
        {
            const int trueInt = true;
            setOptStatus = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status && setNonBlocking)
        {
            setOptStatus = zsock_fcntl(sock, F_SETFL, O_NONBLOCK);
            if (0 != setOptStatus)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status && NULL != interfaceName)
        {
            setOptStatus =
                setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, interfaceName, (socklen_t) strlen(interfaceName));
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK != status && sock != SOPC_INVALID_SOCKET)
        {
            SOPC_UDP_Socket_Close(&sock);
        }

        *pSock = sock;
    }

    return status;
}

SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service)
{
    SOPC_Socket_AddressInfo* pRetAddrInfo = NULL;
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

SOPC_ReturnStatus SOPC_UDP_Socket_AddMembership(Socket sock,
                                                const char* interfaceName,
                                                const SOPC_Socket_AddressInfo* multicast,
                                                const SOPC_Socket_AddressInfo* local)
{
    if (NULL != interfaceName)
    {
        // Not supported in ZEPHYR
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    if (multicast == NULL || (local != NULL && local->ai_family != multicast->ai_family))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_STATUS_OK; // Already done in SOPC_UDP_Socket_CreateToReceive
}

SOPC_ReturnStatus SOPC_UDP_Socket_DropMembership(Socket sock,
                                                 const char* interfaceName,
                                                 const SOPC_Socket_AddressInfo* multicast,
                                                 const SOPC_Socket_AddressInfo* local)
{
    if (NULL != interfaceName)
    {
        // Not supported in ZEPHYR
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    if (multicast == NULL || (local != NULL && local->ai_family != multicast->ai_family))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return P_MULTICAST_DropIpV4Membership(sock, multicast);
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  Socket* pSock)
{
    if (NULL == pSock || NULL == listenAddress)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    Socket sock = SOPC_INVALID_SOCKET;
    SOPC_ReturnStatus status =
        P_SOCKET_UDP_CreateSocket(listenAddress, interfaceName, setReuseAddr, setNonBlocking, &sock);
    if (SOPC_STATUS_OK == status)
    {
        if (listenAddress->ai_family == AF_INET)
        {
            // Multicast must be set before Bind for ZEPHYR
            status = P_MULTICAST_AddIpV4Membership(sock, listenAddress);
            if (SOPC_STATUS_OK == status)
            {
                int res = zsock_bind(sock, &listenAddress->_ai_addr, listenAddress->ai_addrlen);
                if (res < 0)
                {
                    P_MULTICAST_DropIpV4Membership(sock, listenAddress);
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

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               Socket* pSock)
{
    return P_SOCKET_UDP_CreateSocket(destAddress, interfaceName, false, setNonBlocking, pSock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == destAddr || NULL == buffer || buffer->position != 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    ssize_t sent_lenght = zsock_sendto(sock, buffer->data,    //
                                       buffer->length,        //
                                       0,                     //
                                       &destAddr->_ai_addr,   //
                                       destAddr->ai_addrlen); //

    if (sent_lenght < 0 || (uint32_t) sent_lenght != buffer->length)
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
    ssize_t recv_len = 0;

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
        buffer->length = (uint32_t) recv_len;
        if (buffer->length == buffer->maximum_size)
        {
            // The message could be incomplete
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    else
    {
        SOPC_Buffer_Reset(buffer);
        status = SOPC_STATUS_NOK;
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
            P_MULTICAST_DropIpV4Membership(sock, NULL);
            *pSock = SOPC_INVALID_SOCKET;
        }
    }
}
