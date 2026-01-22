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

// This file is not built if PubSub is not used
#if !defined(S2OPC_CLIENTSERVER_ONLY) || S2OPC_CLIENTSERVER_ONLY == 0

#include "p_sopc_sockets.h"

#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_udp_sockets.h"

#include "lwipopts.h"

#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"

/****** CONFIGURATION CHECK *******/
#if !LWIP_MULTICAST_TX_OPTIONS
#error "LwIP 'LWIP_MULTICAST_TX_OPTIONS' is required for this sample!"
#endif

static SOPC_ReturnStatus SOPC_UDP_Socket_AddrInfo_Get(bool IPv6,
                                                      const char* node,
                                                      const char* port,
                                                      SOPC_Socket_AddressInfo** addrs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Socket_AddressInfo hints;
    memset(&hints, 0, sizeof(SOPC_Socket_AddressInfo));
    hints.addrInfo.ai_family = (IPv6 ? AF_INET6 : AF_INET);
    hints.addrInfo.ai_socktype = SOCK_DGRAM;
    hints.addrInfo.ai_flags = AI_PASSIVE;
    hints.addrInfo.ai_protocol = IPPROTO_UDP;

    struct addrinfo* getAddrInfoRes = NULL;

    if ((NULL != node || NULL != port) && NULL != addrs)
    {
        if (getaddrinfo(node, port, &hints.addrInfo, &getAddrInfoRes) != 0)
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

SOPC_ReturnStatus SOPC_UDP_Socket_Set_MulticastTTL(SOPC_Socket sock, uint8_t TTL_scope)
{
    int setOptStatus = -1;

    if (NULL != sock)
    {
        setOptStatus = setsockopt(sock->sock, IPPROTO_IP, IP_MULTICAST_TTL, &TTL_scope, sizeof(TTL_scope));

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

static in_addr_t get_ai_addr(const SOPC_Socket_AddressInfo* addr)
{
    SOPC_ASSERT(NULL != addr);
    return ((struct sockaddr_in*) addr->addrInfo.ai_addr)->sin_addr.s_addr;
}

static struct ip_mreq SOPC_Internal_Fill_IP_mreq(const in_addr_t multiCastAddr)
{
    struct ip_mreq membership;

    membership.imr_multiaddr.s_addr = multiCastAddr;
    membership.imr_interface.s_addr = htonl(INADDR_ANY);
    return membership;
}

static SOPC_ReturnStatus socket_AddMembership(SOPC_Socket sock, const SOPC_Socket_AddressInfo* multicast)
{
    int setOptStatus = -1;

    if (NULL == multicast || !SOPC_FREERTOS_SOCKET_IS_VALID(sock))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (AF_INET6 == multicast->addrInfo.ai_family)
    {
        return SOPC_STATUS_NOT_SUPPORTED;
    }

    SOPC_ASSERT(NULL == sock->membership);
    sock->membership = (struct ip_mreq*) SOPC_Malloc(sizeof(struct ip_mreq));
    SOPC_ASSERT(NULL != sock->membership);

    *sock->membership = SOPC_Internal_Fill_IP_mreq(get_ai_addr(multicast));
    setOptStatus = setsockopt(sock->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, sock->membership, sizeof(struct ip_mreq));

    if (setOptStatus < 0)
    {
        SOPC_Free(sock->membership);
        sock->membership = NULL;
        return SOPC_STATUS_NOK;
    }
    else
    {
        return SOPC_STATUS_OK;
    }
}

static SOPC_ReturnStatus socket_DropMembership(SOPC_Socket sock)
{
    int setOptStatus = -1;

    if (!SOPC_FREERTOS_SOCKET_IS_VALID(sock))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL != sock->membership)
    {
        setOptStatus = setsockopt(sock->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, sock->membership, sizeof(struct ip_mreq));

        SOPC_Free(sock->membership);
        sock->membership = NULL;
        if (setOptStatus < 0)
        {
            return SOPC_STATUS_NOK;
        }
    }
    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus SOPC_UDP_Socket_CreateNew(const SOPC_Socket_AddressInfo* addr,
                                                   const char* interfaceName,
                                                   bool setReuseAddr,
                                                   bool setNonBlocking,
                                                   SOPC_Socket* pSock)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const int trueInt = true;
    int setOptStatus = -1;
    if (NULL != addr && NULL != pSock)
    {
        SOPC_Socket result = SOPC_Malloc(sizeof(*result));
        SOPC_ASSERT(NULL != result);
        result->sock = socket(addr->addrInfo.ai_family, addr->addrInfo.ai_socktype, addr->addrInfo.ai_protocol);
        result->membership = NULL;

        if (SOPC_FREERTOS_INVALID_SOCKET_ID == result->sock)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_STATUS_OK;
            setOptStatus = 0;
        }

        if (SOPC_STATUS_OK == status && setReuseAddr)
        {
            setOptStatus = setsockopt(result->sock, SOL_SOCKET, SO_REUSEADDR, (const void*) &trueInt, sizeof(int));
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status && setNonBlocking)
        {
            setOptStatus = fcntl(result->sock, F_SETFL, O_NONBLOCK);
            if (setOptStatus < 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status && NULL != interfaceName)
        {
            struct ifreq ifreq = {0};
            strncpy(ifreq.ifr_name, interfaceName, sizeof(ifreq.ifr_name) - 1);
            errno = 0;
            setOptStatus = setsockopt(result->sock, SOL_SOCKET, SO_BINDTODEVICE, (void*) &ifreq, sizeof(ifreq));
            if (setOptStatus < 0)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "setsockopt SO_BINDTODEVICE failed");
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            *pSock = result;
        }
        else
        {
            SOPC_Free(result);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  SOPC_Socket* pSock)
{
    *pSock = NULL;
    SOPC_ReturnStatus status =
        SOPC_UDP_Socket_CreateNew(listenAddress, interfaceName, setReuseAddr, setNonBlocking, pSock);
    if (SOPC_STATUS_OK == status)
    {
        int res = bind((*pSock)->sock, listenAddress->addrInfo.ai_addr, listenAddress->addrInfo.ai_addrlen);
        if (res == -1)
        {
            status = SOPC_STATUS_NOK;
        }
        else
        {
            bool isMC = false;
            if (listenAddress->addrInfo.ai_family == AF_INET)
            {
                // IPV4: first address byte indicates if this is a multicast address
                struct sockaddr_in* sAddr = (struct sockaddr_in*) listenAddress->addrInfo.ai_addr;
                const uint32_t ip = htonl(sAddr->sin_addr.s_addr);
                isMC = ((ip >> 28) & 0xF) == 0xE; // Multicast mask on 4 first bytes;
            }

            // If address is multicast, then add membership
            if (isMC)
            {
                status = socket_AddMembership(*pSock, listenAddress);
            }
        }
        if (SOPC_STATUS_OK != status && NULL != *pSock)
        {
            SOPC_UDP_Socket_Close(pSock);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               SOPC_Socket* pSock)
{
    return SOPC_UDP_Socket_CreateNew(destAddress, interfaceName, false, setNonBlocking, pSock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(SOPC_Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer)
{
    if (!SOPC_FREERTOS_SOCKET_IS_VALID(sock) || NULL == destAddr || NULL == buffer || buffer->position != 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

#ifdef SOPC_FREERTOS_UDP_RAM_BASE
    /* Note that there is no build-defense that this section is not used by the linker script
     * For any other variable. This is Ok with current demo (NUCLEO_H723ZG) as proof of concept but clearly has
     * to be more securely done for a real project.
     */
    static uint8_t* DTCMR_Buffer = (void*) SOPC_FREERTOS_UDP_RAM_BASE;
    static SOPC_Mutex* mutex = NULL;

    if (NULL == mutex)
    {
        SOPC_Mutex_Initialization(mutex);
    }

    SOPC_Mutex_Lock(mutex);

    memcpy(DTCMR_Buffer, buffer->data, buffer->length);
    ssize_t res =
        sendto(sock->sock, DTCMR_Buffer, buffer->length, 0, destAddr->addrInfo.ai_addr, destAddr->addrInfo.ai_addrlen);

    SOPC_Mutex_Unlock(mutex);
#else
#warning \
    "SOPC_FREERTOS_UDP_RAM_BASE is not defined and UDP sending may probably fail if RAM section is not properly set with DMA! (STM32H series at least)."
    /* See
     * https://community.st.com/t5/stm32-mcus/ethernet-not-working-on-stm32h7x3/ta-p/49479
     * (from https://forums.freertos.org/t/stm32h7-eth-tx-fails-depending-on-buffer-address/18087/2)
     */
    ssize_t res =
        sendto(sock->sock, buffer->data, buffer->length, 0, destAddr->addrInfo.ai_addr, destAddr->addrInfo.ai_addrlen);
#endif

    if (-1 == res || (uint32_t) res != buffer->length)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(SOPC_Socket sock, SOPC_Buffer* buffer)
{
    if (!SOPC_FREERTOS_SOCKET_IS_VALID(sock) || NULL == buffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct sockaddr_in si_client;
    socklen_t slen = sizeof(si_client);

    ssize_t recv_len =
        recvfrom(sock->sock, buffer->data, buffer->maximum_size, 0, (struct sockaddr*) &si_client, &slen);
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

void SOPC_UDP_Socket_Close(SOPC_Socket* pSock)
{
    socket_DropMembership(*pSock);
    SOPC_Socket_Close(pSock);
}

#endif
