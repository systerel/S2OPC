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

#include "sopc_common_constants.h"
#include "sopc_macros.h"
#include "sopc_udp_sockets.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

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

static struct ip_mreqn SOPC_Internal_Fill_IP_mreq(const SOPC_Socket_AddressInfo* multiCastAddr,
                                                  const SOPC_Socket_AddressInfo* localAddr,
                                                  unsigned int if_index)
{
    assert(multiCastAddr != NULL);
    struct ip_mreqn membership;

    membership.imr_multiaddr.s_addr = ((struct sockaddr_in*) get_ai_addr(multiCastAddr))->sin_addr.s_addr;
    assert(if_index > 0);
    membership.imr_ifindex = (int) if_index;
    if (NULL == localAddr)
    {
        membership.imr_address.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        membership.imr_address.s_addr = ((struct sockaddr_in*) get_ai_addr(localAddr))->sin_addr.s_addr;
    }
    return membership;
}

static struct ipv6_mreq SOPC_Internal_Fill_IP6_mreq(const SOPC_Socket_AddressInfo* multiCastAddr,
                                                    const SOPC_Socket_AddressInfo* localAddr,
                                                    unsigned int if_index)
{
    SOPC_UNUSED_ARG(localAddr);
    assert(if_index > 0);
    assert(multiCastAddr != NULL);
    struct ipv6_mreq membership;

    membership.ipv6mr_multiaddr = ((struct sockaddr_in6*) get_ai_addr(multiCastAddr))->sin6_addr;
    membership.ipv6mr_interface = if_index;

    return membership;
}

static bool setMembershipOption(Socket sock,
                                const SOPC_Socket_AddressInfo* multicast,
                                const SOPC_Socket_AddressInfo* local,
                                unsigned int ifindex,
                                int level,
                                int optname)
{
    int setOptStatus = -1;
    if (IPPROTO_IPV6 == level)
    {
        struct ipv6_mreq membershipV6 = SOPC_Internal_Fill_IP6_mreq(multicast, local, ifindex);
        setOptStatus = setsockopt(sock, level, optname, &membershipV6, sizeof(membershipV6));
    }
    else if (IPPROTO_IP == level)
    {
        struct ip_mreqn membership = SOPC_Internal_Fill_IP_mreq(multicast, local, ifindex);
        setOptStatus = setsockopt(sock, level, optname, &membership, sizeof(membership));
    }
    else
    {
        assert(false);
    }
    return (0 == setOptStatus);
}

static SOPC_ReturnStatus applyMembershipToAllInterfaces(Socket sock,
                                                        const SOPC_Socket_AddressInfo* multicast,
                                                        const SOPC_Socket_AddressInfo* local,
                                                        int optnameIPv4,
                                                        int optnameIPv6)
{
    // Without interfaceName provided: drop on all possible interfaces
    struct ifaddrs* ifap = NULL;
    int result = getifaddrs(&ifap);

    if (0 != result)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    uint32_t counter = 0;
    bool atLeastOneItfSuccess = false;

    for (struct ifaddrs* ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr)
        {
            if (AF_INET6 == multicast->ai_family)
            {
                if (AF_INET6 == ifa->ifa_addr->sa_family)
                {
                    counter++;
                    atLeastOneItfSuccess |= setMembershipOption(sock, multicast, local, if_nametoindex(ifa->ifa_name),
                                                                IPPROTO_IPV6, optnameIPv6);
                }
            }
            else
            {
                if (AF_INET == ifa->ifa_addr->sa_family)
                {
                    counter++;
                    atLeastOneItfSuccess |= setMembershipOption(sock, multicast, local, if_nametoindex(ifa->ifa_name),
                                                                IPPROTO_IP, optnameIPv4);
                }
            }
        }
    }
    freeifaddrs(ifap);

    if (0 == counter)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }
    else
    {
        if (atLeastOneItfSuccess)
        {
            return SOPC_STATUS_OK;
        }
        else
        {
            return SOPC_STATUS_NOK;
        }
    }
}

SOPC_ReturnStatus SOPC_UDP_Socket_AddMembership(Socket sock,
                                                const char* interfaceName,
                                                const SOPC_Socket_AddressInfo* multicast,
                                                const SOPC_Socket_AddressInfo* local)
{
    if (NULL == multicast || NULL == local || SOPC_INVALID_SOCKET == sock || multicast->ai_family != local->ai_family)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Using interfaceName provided
    if (NULL != interfaceName)
    {
        unsigned int ifindex = if_nametoindex(interfaceName);
        bool ipv4success = false;
        bool ipv6success = false;
        ipv6success = setMembershipOption(sock, multicast, local, ifindex, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP);
        ipv4success = setMembershipOption(sock, multicast, local, ifindex, IPPROTO_IP, IP_ADD_MEMBERSHIP);
        if (!ipv6success)
        {
            SOPC_CONSOLE_PRINTF("AddMembership failure (error='%s') on interface for IPv6: %s\n", strerror(errno),
                                interfaceName);
        }
        if (!ipv4success)
        {
            SOPC_CONSOLE_PRINTF("AddMembership failure (error='%s') on interface for IPv4: %s\n", strerror(errno),
                                interfaceName);
        }
        if (ipv4success || ipv6success)
        {
            return SOPC_STATUS_OK;
        }
        else
        {
            return SOPC_STATUS_NOK;
        }
    }

    return applyMembershipToAllInterfaces(sock, multicast, local, IP_ADD_MEMBERSHIP, IPV6_ADD_MEMBERSHIP);
}

SOPC_ReturnStatus SOPC_UDP_Socket_DropMembership(Socket sock,
                                                 const char* interfaceName,
                                                 const SOPC_Socket_AddressInfo* multicast,
                                                 const SOPC_Socket_AddressInfo* local)
{
    if (NULL == multicast || SOPC_INVALID_SOCKET == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Using interfaceName provided
    if (NULL != interfaceName)
    {
        bool success = false;
        unsigned int ifindex = if_nametoindex(interfaceName);
        success |= setMembershipOption(sock, multicast, local, ifindex, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP);
        success |= setMembershipOption(sock, multicast, local, ifindex, IPPROTO_IP, IP_DROP_MEMBERSHIP);

        if (success)
        {
            return SOPC_STATUS_OK;
        }
        else
        {
            return SOPC_STATUS_NOK;
        }
    }

    return applyMembershipToAllInterfaces(sock, multicast, local, IP_DROP_MEMBERSHIP, IPV6_DROP_MEMBERSHIP);
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
        }

        if (SOPC_STATUS_OK == status && setReuseAddr)
        {
            setOptStatus = setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        /*
        // Enforce IPV6 sockets can be used for IPV4 connections (if socket is IPV6)
        if (setOptStatus != -1 && addr->addrin.sin_family == AF_INET6)
        {
            const int falseInt = false;
            setOptStatus = setsockopt(*sock, IPPROTO_IPV6, IPV6_V6ONLY, (const void*) &falseInt, sizeof(int));
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }
        */

        if (SOPC_STATUS_OK == status && setNonBlocking)
        {
            setOptStatus = fcntl(*sock, F_SETFL, O_NONBLOCK);
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status && NULL != interfaceName)
        {
            setOptStatus =
                setsockopt(*sock, SOL_SOCKET, SO_BINDTODEVICE, interfaceName, (socklen_t) strlen(interfaceName));
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
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
    if (SOPC_INVALID_SOCKET == sock || NULL == destAddr || NULL == buffer || buffer->position != 0)
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

    ssize_t recv_len = recvfrom(sock, buffer->data, buffer->current_size, 0, (struct sockaddr*) &si_client, &slen);
    if (recv_len == -1)
    {
        return SOPC_STATUS_NOK;
    }

    buffer->length = (uint32_t) recv_len;
    if (buffer->length == buffer->current_size)
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
