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

#include "sopc_udp_sockets.h"

#include "lwipopts.h"

#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

#include "ethernetif.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"

static SOPC_ReturnStatus SOPC_UDP_Socket_AddrInfo_Get(bool IPv6,
                                                      const char* node,
                                                      const char* port,
                                                      SOPC_Socket_AddressInfo** addrs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.ai_family = (IPv6 ? AF_INET6 : AF_INET);
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    if ((NULL != node || NULL != port) && NULL != addrs)
    {
        if (getaddrinfo(node, port, &hints, addrs) != 0)
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

SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service)
{
    SOPC_Socket_AddressInfo* addr = NULL;
    SOPC_ReturnStatus status = SOPC_UDP_Socket_AddrInfo_Get(IPv6, node, service, &addr);
    if (SOPC_STATUS_OK != status)
    {
        addr = NULL;
    }
    return addr;
}

void SOPC_UDP_SocketAddress_Delete(SOPC_Socket_AddressInfo** addr)
{
    SOPC_Socket_AddrInfoDelete(addr);
}

SOPC_ReturnStatus SOPC_UDP_Socket_Set_MulticastTTL(Socket sock, uint8_t TTL_scope)
{
    int setOptStatus = -1;

    if (SOPC_INVALID_SOCKET != sock)
    {
        setOptStatus = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &TTL_scope, sizeof(TTL_scope));

        if (setOptStatus < 0)
        {
            return SOPC_STATUS_NOK;
        }
        else
        {
            return SOPC_STATUS_OK;
        }
    }
    return SOPC_STATUS_INVALID_PARAMETERS;
}

static void* get_ai_addr(const SOPC_Socket_AddressInfo* addr)
{
    return addr->ai_addr;
}

static struct ip_mreq SOPC_Internal_Fill_IP_mreq(const SOPC_Socket_AddressInfo* multiCastAddr,
                                                 const SOPC_Socket_AddressInfo* localAddr)
{
    configASSERT(multiCastAddr != NULL);
    struct ip_mreq membership;

    membership.imr_multiaddr.s_addr = ((struct sockaddr_in*) get_ai_addr(multiCastAddr))->sin_addr.s_addr;

    if (NULL == localAddr)
    {
        membership.imr_interface.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        membership.imr_interface.s_addr = ((struct sockaddr_in*) get_ai_addr(localAddr))->sin_addr.s_addr;
    }
    return membership;
}

SOPC_ReturnStatus SOPC_UDP_Socket_AddMembership(Socket sock,
                                                const char* interfaceName,
                                                const SOPC_Socket_AddressInfo* multicast,
                                                const SOPC_Socket_AddressInfo* local)
{
    int setOptStatus = -1;

    if (NULL == multicast || SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (local != NULL)
    {
        if (multicast->ai_family != local->ai_family)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (AF_INET6 == multicast->ai_family)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    struct ip_mreq membership = SOPC_Internal_Fill_IP_mreq(multicast, local);
    setOptStatus = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &membership, sizeof(struct ip_mreq));

    if (setOptStatus < 0)
    {
        return SOPC_STATUS_NOK;
    }
    else
    {
        return SOPC_STATUS_OK;
    }
}

SOPC_ReturnStatus SOPC_UDP_Socket_DropMembership(Socket sock,
                                                 const char* interfaceName,
                                                 const SOPC_Socket_AddressInfo* multicast,
                                                 const SOPC_Socket_AddressInfo* local)
{
    int setOptStatus = -1;

    if (NULL == multicast || SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (AF_INET6 == multicast->ai_family)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    struct ip_mreq membership = SOPC_Internal_Fill_IP_mreq(multicast, local);
    setOptStatus = setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &membership, sizeof(struct ip_mreq));

    if (setOptStatus < 0)
    {
        return SOPC_STATUS_NOK;
    }
    else
    {
        return SOPC_STATUS_OK;
    }
}

static SOPC_ReturnStatus SOPC_UDP_Socket_CreateNew(const SOPC_Socket_AddressInfo* addr,
                                                   const char* interfaceName,
                                                   bool setReuseAddr,
                                                   bool setNonBlocking,
                                                   Socket* sock)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const int trueInt = true;
    int setOptStatus = -1;
    if (NULL != addr && NULL != sock)
    {
        *sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (SOPC_INVALID_SOCKET == *sock)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_STATUS_OK;
            setOptStatus = 0;
        }

        if (SOPC_STATUS_OK == status && false != setReuseAddr)
        {
            setOptStatus = setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
        }

        if (setOptStatus < 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  Socket* sock)
{
    SOPC_ReturnStatus status =
        SOPC_UDP_Socket_CreateNew(listenAddress, interfaceName, setReuseAddr, setNonBlocking, sock);
    if (SOPC_STATUS_OK == status)
    {
        int res = bind(*sock, listenAddress->ai_addr, listenAddress->ai_addrlen);
        if (res == -1)
        {
            SOPC_UDP_Socket_Close(sock);
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               Socket* sock)
{
    return SOPC_UDP_Socket_CreateNew(destAddress, interfaceName, false, setNonBlocking, sock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer)
{
    configASSERT(buffer->position == 0);
    if (SOPC_INVALID_SOCKET == sock || NULL == destAddr || NULL == buffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ssize_t res = sendto(sock, buffer->data, buffer->length, 0, destAddr->ai_addr, destAddr->ai_addrlen);

    if (-1 == res || (uint32_t) res != buffer->length)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(Socket sock, SOPC_Buffer* buffer)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == buffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct sockaddr_in si_client;
    socklen_t slen = sizeof(si_client);

    ssize_t recv_len = recvfrom(sock, buffer->data, buffer->maximum_size, 0, (struct sockaddr*) &si_client, &slen);
    if (-1 == recv_len)
    {
        return SOPC_STATUS_NOK;
    }

    buffer->length = (uint32_t) recv_len;
    if (buffer->length == buffer->maximum_size)
    {
        // The message could be incomplete
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

void SOPC_UDP_Socket_Close(Socket* sock)
{
    SOPC_Socket_Close(sock);
}
