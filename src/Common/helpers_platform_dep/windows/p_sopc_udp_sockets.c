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

#include "p_sopc_sockets.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_udp_sockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mswsock.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

static bool gInitialized = false;
static WSADATA wsaData;

void SOPC_UDP_Socket_Close(SOPC_Socket* sock);

static inline void Network_Initialize(void)
{
    if (!gInitialized)
    {
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        gInitialized = true;
    }
}

static inline void Network_Clear(void)
{
    WSACleanup();
}

static SOPC_ReturnStatus SOPC_UDP_Socket_AddrInfo_Get(int IPv6,
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

    if ((node != NULL || port != NULL) && addrs != NULL)
    {
        int rc = getaddrinfo(node, port, (struct addrinfo*) &hints, (struct addrinfo**) addrs);
        if (rc != 0)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "UDP sock: Invalid address %s, getaddrinfo failed: %s(%d)",
                                   node, gai_strerrorA(rc), rc);
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
    Network_Initialize();
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
    if (sock != NULL && sock->sock != INVALID_SOCKET)
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "UDP sock: setsockopt(%d, IPPROTO_IP, IP_MULTICAST_TTL, (%d))",
                               (int) sock->sock, TTL_scope);
        int setOptStatus =
            setsockopt(sock->sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &TTL_scope, (int) sizeof(TTL_scope));
        return (setOptStatus == SOCKET_ERROR) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }
    return SOPC_STATUS_INVALID_PARAMETERS;
}

static void* get_ai_addr(const SOPC_Socket_AddressInfo* addr)
{
    return (void*) addr->addrInfo.ai_addr;
}

static SOPC_ReturnStatus SOPC_UDP_Socket_CreateNew(const SOPC_Socket_AddressInfo* addr,
                                                   const char* interfaceName,
                                                   bool setReuseAddr,
                                                   bool setNonBlocking,
                                                   SOPC_Socket* sock)
{
    SOPC_UNUSED_ARG(interfaceName);
    if (NULL == addr || NULL == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Socket_Impl* socketImpl = SOPC_Calloc(1, sizeof(*socketImpl));
    if (NULL == socketImpl)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const int trueInt = true;
    int setOptStatus = -1;

    socketImpl->sock = socket(addr->addrInfo.ai_family, addr->addrInfo.ai_socktype, addr->addrInfo.ai_protocol);
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "UDP sock: socket => %llu ", (unsigned long long) socketImpl->sock);

    if (INVALID_SOCKET == socketImpl->sock)
    {
        status = SOPC_STATUS_NOK;
    }
    else
    {
        status = SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status && setReuseAddr)
    {
        setOptStatus = setsockopt(socketImpl->sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &trueInt, sizeof(int));
        if (setOptStatus < 0)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "UDP sock: setsockopt (SO_REUSEADDR) failed with error: %ld",
                                   setOptStatus);
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status && setNonBlocking)
    {
        u_long iMode = 1; // iMode = 0 => blocking is enabled. iMode != 0 => non-blocking mode is enabled.
        setOptStatus = ioctlsocket(socketImpl->sock, FIONBIO, &iMode);
        if (setOptStatus != NO_ERROR)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "UDP sock: ioctlsocket failed with error: %ld",
                                   setOptStatus);
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *sock = socketImpl;
    }
    else
    {
        SOPC_Socket_Close(&socketImpl);
    }

    return status;
}

/* WARNING: interfaceName must be and ipv4 address or NULL */
SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  SOPC_Socket* sock)
{
    SOPC_ReturnStatus status =
        SOPC_UDP_Socket_CreateNew(listenAddress, interfaceName, setReuseAddr, setNonBlocking, sock);

    if (SOPC_STATUS_OK == status)
    {
        int res = SOCKET_ERROR;
        struct sockaddr_in* listenAddr = (struct sockaddr_in*) get_ai_addr(listenAddress);

        // Note: only add membership if socket is a Multicast address
        bool isMC = false;
        if (listenAddress->addrInfo.ai_family == AF_INET)
        {
            // IPV4: first address byte indicates if this is a multicast address
            const uint32_t ip = htonl(listenAddr->sin_addr.s_addr);
            isMC = ((ip >> 28) & 0xF) == 0xE; // Multicast mask on 4 first bytes;
        }

        if (isMC)
        {
            // Bind with INADDR_ANY and the right port, then specify the multicast address
            // and the name of the interface on which you want to listen.
            SOPC_Logger_TraceDebug(
                SOPC_LOG_MODULE_COMMON,
                "UDP sock: Detected a Multicast address. Bind with INADDR_ANY, then Set option 'IP_ADD_MEMBERSHIP'");
            // Bind socket with INADDR_ANY and listen port
            SOCKADDR_IN localif;
            localif.sin_family = AF_INET;
            localif.sin_port = listenAddr->sin_port;
            localif.sin_addr.s_addr = htonl(INADDR_ANY);
            res = bind((*sock)->sock, (SOCKADDR*) &localif, sizeof(localif));
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "UDP sock: bind(%d, %d.%d.%d.%d:%d)", (int) (*sock)->sock,
                                   localif.sin_addr.S_un.S_un_b.s_b4, localif.sin_addr.S_un.S_un_b.s_b3,
                                   localif.sin_addr.S_un.S_un_b.s_b2, localif.sin_addr.S_un.S_un_b.s_b1,
                                   (int) htons(localif.sin_port));

            // Add membership (listen addr) and interfaceName addr, if != NULL
            struct ip_mreq mreq;
            if (interfaceName != NULL)
            {
                struct in_addr interfaceAddr;
                int resInet = inet_pton(AF_INET, interfaceName, &interfaceAddr);
                if (resInet <= 0)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "UDP sock: Failed to inet_pton on socket %d  (%s)",
                                           (int) (*sock)->sock, gai_strerrorA(WSAGetLastError()));
                }
                else
                {
                    mreq.imr_interface = interfaceAddr;
                }
            }
            else
            {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            }

            mreq.imr_multiaddr.s_addr = listenAddr->sin_addr.s_addr;
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON,
                                   "UDP sock: setsockopt(%d, IPPROTO_IP, IP_ADD_MEMBERSHIP, (%s / %d.%d.%d.%d:%d))",
                                   (int) (*sock)->sock, interfaceName, listenAddr->sin_addr.S_un.S_un_b.s_b1,
                                   listenAddr->sin_addr.S_un.S_un_b.s_b2, listenAddr->sin_addr.S_un.S_un_b.s_b3,
                                   listenAddr->sin_addr.S_un.S_un_b.s_b4, htons(listenAddr->sin_port));
            res = setsockopt((*sock)->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
        }
        else // Bind directly with unicast addr
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "UDP sock: bind(%d, %d.%d.%d.%d:%d)", (int) (*sock)->sock,
                                   listenAddr->sin_addr.S_un.S_un_b.s_b1, listenAddr->sin_addr.S_un.S_un_b.s_b2,
                                   listenAddr->sin_addr.S_un.S_un_b.s_b3, listenAddr->sin_addr.S_un.S_un_b.s_b4,
                                   (int) htons(listenAddr->sin_port));
            res = bind((*sock)->sock, listenAddress->addrInfo.ai_addr, (int) listenAddress->addrInfo.ai_addrlen);
        }
        if (res == SOCKET_ERROR)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "UDP sock: Failed to Bind socket %d on port %d (%s)",
                                   (int) (*sock)->sock, htons(listenAddr->sin_port), gai_strerrorA(WSAGetLastError()));
            SOPC_UDP_Socket_Close(sock);
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

/* WARNING: interfaceName is NOT USED */
SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               SOPC_Socket* sock)
{
    // TODO : Enable interfaceName for sending (set IP_MULTICAST_IF or IP_UNICAST_IF, depending uni/multicast)
    return SOPC_UDP_Socket_CreateNew(destAddress, interfaceName, false, setNonBlocking, sock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(SOPC_Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer)
{
    if (INVALID_SOCKET == sock->sock || NULL == destAddr || NULL == buffer || buffer->position != 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOCKADDR_IN* inaddr = (SOCKADDR_IN*) (destAddr->addrInfo.ai_addr);
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "UDP sock: sendto(%d, (...), %u, to (%d.%d.%d.%d:%d))",
                           (int) sock->sock, (int) buffer->length, inaddr->sin_addr.S_un.S_un_b.s_b1,
                           inaddr->sin_addr.S_un.S_un_b.s_b2, inaddr->sin_addr.S_un.S_un_b.s_b3,
                           inaddr->sin_addr.S_un.S_un_b.s_b4, htons(inaddr->sin_port));
    int res = sendto(sock->sock, (const char*) buffer->data, (int) buffer->length, 0, destAddr->addrInfo.ai_addr,
                     (int) destAddr->addrInfo.ai_addrlen);

    if (SOCKET_ERROR == res || (uint32_t) res != buffer->length)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(SOPC_Socket sock, SOPC_Buffer* buffer)
{
    if (INVALID_SOCKET == sock->sock || NULL == buffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct sockaddr_in si_client;
    socklen_t slen = sizeof(si_client);

    int recv_len =
        recvfrom(sock->sock, (char*) buffer->data, (int) buffer->current_size, 0, (struct sockaddr*) &si_client, &slen);

    if (recv_len == SOCKET_ERROR)
    {
        return SOPC_STATUS_NOK;
    }

    buffer->length = (uint32_t) recv_len;
    if (buffer->length == buffer->current_size)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

void SOPC_UDP_Socket_Close(SOPC_Socket* sock)
{
    if (sock != NULL && *sock != NULL && (*sock)->sock && (*sock)->sock != INVALID_SOCKET)
    {
        closesocket((*sock)->sock);
        (*sock)->sock = INVALID_SOCKET;
    }
}
