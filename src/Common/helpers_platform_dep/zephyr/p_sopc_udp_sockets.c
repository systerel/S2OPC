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

#include <zephyr/kernel.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>

#include "sopc_udp_sockets.h"

#include "p_sopc_multicast.h"
#include "p_sopc_sockets.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

static void* get_ai_addr(const SOPC_Socket_AddressInfo* addr)
{
    return addr->addrInfo.ai_addr;
}

static SOPC_ReturnStatus P_SOCKET_UDP_CreateSocket(const SOPC_Socket_AddressInfo* pAddrInfo,
                                                   const char* interfaceName,
                                                   bool setReuseAddr,
                                                   bool setNonBlocking,
                                                   SOPC_Socket* pSock)
{
    if (NULL == pAddrInfo || NULL == pSock)
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
    *pSock = SOPC_INVALID_SOCKET;

    socketImpl->sock = zsock_socket(pAddrInfo->addrInfo.ai_family, SOCK_DGRAM, IPPROTO_UDP);

    if (-1 != socketImpl->sock)
    {
        status = SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status && setReuseAddr)
    {
        const int trueInt = true;
        setOptStatus = setsockopt(socketImpl->sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
        if (setOptStatus < 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status && setNonBlocking)
    {
        S2OPC_TEMP_FAILURE_RETRY(setOptStatus, zsock_fcntl(socketImpl->sock, F_SETFL, O_NONBLOCK));
        if (0 != setOptStatus)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status && NULL != interfaceName)
    {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(struct ifreq));
        size_t ifr_name_len = sizeof(ifr.ifr_name);
        strncpy(ifr.ifr_name, interfaceName, ifr_name_len);
        ifr.ifr_name[ifr_name_len - 1] = '\0';
        setOptStatus = setsockopt(socketImpl->sock, SOL_SOCKET, SO_BINDTODEVICE, (void*) &ifr, sizeof(struct ifreq));
        if (setOptStatus < 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status && socketImpl != SOPC_INVALID_SOCKET)
    {
        SOPC_UDP_Socket_Close(&socketImpl);
    }

    *pSock = socketImpl;

    return status;
}

SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service)
{
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.addrInfo.ai_family = (IPv6 ? AF_INET6 : AF_INET);
    hints.addrInfo.ai_socktype = SOCK_DGRAM;
    hints.addrInfo.ai_flags = AI_PASSIVE;
    hints.addrInfo.ai_protocol = IPPROTO_UDP;

    struct zsock_addrinfo* getAddrInfoRes = NULL;

    // If not, add it to default interface
    if ((NULL != node || NULL != service))
    {
        int ret = zsock_getaddrinfo(node, service, &hints.addrInfo, &getAddrInfoRes);
        if (ret < 0)
        {
            getAddrInfoRes = NULL;
        }
    }

    return (SOPC_Socket_AddressInfo*) getAddrInfoRes;
}

void SOPC_UDP_SocketAddress_Delete(SOPC_Socket_AddressInfo** pptrAddrInfo)
{
    if (NULL != pptrAddrInfo)
    {
        zsock_freeaddrinfo(&(*pptrAddrInfo)->addrInfo);
        *pptrAddrInfo = NULL;
    }
}

SOPC_ReturnStatus SOPC_UDP_Socket_AddMembership(SOPC_Socket sock,
                                                const char* interfaceName,
                                                const SOPC_Socket_AddressInfo* multicast)
{
    if (NULL != interfaceName)
    {
        // Not supported in ZEPHYR
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    if (multicast == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_STATUS_OK; // Already done in SOPC_UDP_Socket_CreateToReceive
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  SOPC_Socket* pSock)
{
    if (NULL == pSock || NULL == listenAddress)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Socket sock = SOPC_INVALID_SOCKET;
    SOPC_ReturnStatus status =
        P_SOCKET_UDP_CreateSocket(listenAddress, interfaceName, setReuseAddr, setNonBlocking, &sock);
    if (SOPC_STATUS_OK == status)
    {
        bool isMC = false;
        if (listenAddress->addrInfo.ai_family == AF_INET)
        {
            // IPV4: first address byte indicates if this is a multicast address
            struct sockaddr_in* sAddr = (struct sockaddr_in*) get_ai_addr(listenAddress);
            const uint32_t ip = htonl(sAddr->sin_addr.s_addr);
            isMC = ((ip >> 28) & 0xF) == 0xE; // Multicast mask on 4 first bytes;
            // If address is multicast, then add membership
            if (isMC)
            {
                // Multicast must be set before Bind for ZEPHYR
                status = P_MULTICAST_AddIpV4Membership(sock, listenAddress);
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status)
        {
            int res = zsock_bind(sock->sock, &listenAddress->addrInfo._ai_addr, listenAddress->addrInfo.ai_addrlen);
            if (res < 0)
            {
                if (isMC)
                {
                    P_MULTICAST_DropIpV4Membership(sock, listenAddress);
                }
                SOPC_UDP_Socket_Close(&sock);
                status = SOPC_STATUS_NOK;
            }
        }
    }

    *pSock = sock;

    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               SOPC_Socket* pSock)
{
    return P_SOCKET_UDP_CreateSocket(destAddress, interfaceName, false, setNonBlocking, pSock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(SOPC_Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == destAddr || NULL == buffer || buffer->position != 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    ssize_t sent_length = 0;

    S2OPC_TEMP_FAILURE_RETRY(sent_length, zsock_sendto(sock->sock, buffer->data, buffer->length, 0,
                                                       &destAddr->addrInfo._ai_addr, destAddr->addrInfo.ai_addrlen));

    if (sent_length < 0 || (uint32_t) sent_length != buffer->length)
    {
        status = SOPC_STATUS_NOK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(SOPC_Socket sock, SOPC_Buffer* buffer)
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

    S2OPC_TEMP_FAILURE_RETRY(
        recv_len, recvfrom(sock->sock, buffer->data, buffer->maximum_size, 0, (struct sockaddr*) &saddr, &slen));

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

void SOPC_UDP_Socket_Close(SOPC_Socket* pSock)
{
    if (NULL != pSock && SOPC_INVALID_SOCKET != *pSock)
    {
        SOPC_Socket sock = *pSock;
        zsock_shutdown(sock->sock, ZSOCK_SHUT_RDWR);
        int res = 0;

        S2OPC_TEMP_FAILURE_RETRY(res, zsock_close(sock->sock));

        if (res == 0)
        {
            P_MULTICAST_DropIpV4Membership(sock, NULL);
            SOPC_Free(*pSock);
            *pSock = SOPC_INVALID_SOCKET;
        }
    }
}
