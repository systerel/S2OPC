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
#include "sopc_udp_sockets.h"
#include "sopc_mem_alloc.h"

#include <mswsock.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_TTL 32u
#pragma comment(lib, "Ws2_32.lib")

static int gInitialized = 0;
static WSADATA wsaData;

#define PRINT_DEBUG printf // Activation du debug

void SOPC_UDP_Socket_Close(SOPC_Socket* sock);

static inline void Network_Initialize(void)
{
    if (!gInitialized)
    {
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        gInitialized = 1;
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
            PRINT_DEBUG("Invalid address %s, getaddrinfo failed: %s(%d)\n", node, gai_strerrorA(rc), rc);
            return SOPC_STATUS_NOK;
        }
        return SOPC_STATUS_OK;
    }
    return SOPC_STATUS_INVALID_PARAMETERS;
}

SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service)
{
    Network_Initialize();
    SOPC_Socket_AddressInfo* addr = NULL;
    SOPC_ReturnStatus status = SOPC_UDP_Socket_AddrInfo_Get(IPv6, node, service, &addr);
    return (status == SOPC_STATUS_OK) ? addr : NULL;
}

void SOPC_UDP_SocketAddress_Delete(SOPC_Socket_AddressInfo** addr)
{
    if (addr && *addr)
    {
        free(*addr);
        *addr = NULL;
    }
}

SOPC_ReturnStatus SOPC_UDP_Socket_Set_MulticastTTL(SOPC_Socket pSock, uint8_t TTL_scope)
{
    int optlen = sizeof(TTL_scope);

    if (pSock->sock != INVALID_SOCKET)
    {
        PRINT_DEBUG("setsockopt(%d, IPPROTO_IP, IP_MULTICAST_TTL, (%d))\n", (int) pSock->sock, TTL_scope);
        int setOptStatus = setsockopt(pSock->sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &TTL_scope, optlen);
        return (setOptStatus == SOCKET_ERROR) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }
    return SOPC_STATUS_INVALID_PARAMETERS;
}

static SOPC_ReturnStatus SOPC_UDP_Socket_CreateNew(const SOPC_Socket_AddressInfo* addr,
                                                   const char* interfaceName,
                                                   int setReuseAddr,
                                                   int setNonBlocking,
                                                   SOCKET* sock)
{
    if (addr == NULL || sock == NULL)
        return SOPC_STATUS_INVALID_PARAMETERS;

    *sock = socket(addr->addrInfo.ai_family, addr->addrInfo.ai_socktype, addr->addrInfo.ai_protocol);
    PRINT_DEBUG("socket(SND) => %llu \n", (unsigned long long) *sock);

    if (*sock == INVALID_SOCKET)
        return SOPC_STATUS_NOK;

    if (setReuseAddr)
    {
        int trueInt = 1;
        setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &trueInt, sizeof(int));
    }

    return SOPC_STATUS_OK;
}

static void* get_ai_addr(const SOPC_Socket_AddressInfo* addr)
{
    return (void*) addr->addrInfo.ai_addr;
}

static int JoinMulticastGroup(SOCKET s, const struct addrinfo* group, struct addrinfo* iface)
{
    struct ip_mreq mreqv4;
    char* optval = NULL;
    int optlevel, option, optlen, rc;

    rc = NO_ERROR;
    if (group->ai_family == AF_INET)
    {
        optlevel = IPPROTO_IP;
        option = IP_ADD_MEMBERSHIP;
        optval = (char*) &mreqv4;
        optlen = sizeof(mreqv4);

        mreqv4.imr_multiaddr.s_addr = ((SOCKADDR_IN*) group->ai_addr)->sin_addr.s_addr;
        mreqv4.imr_interface.s_addr = ((SOCKADDR_IN*) iface->ai_addr)->sin_addr.s_addr;
    }
    else
    {
        fprintf(stderr, "Attempting to join multicast group for invalid address family!\n");
        rc = SOCKET_ERROR;
    }

    if (rc != SOCKET_ERROR)
    {
        PRINT_DEBUG("setsockopt(%d, IPPROTO_IP, IP_ADD_MEMBERSHIP, ...)\n", (int) s);
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            PRINT_DEBUG("JoinMulticastGroup: setsockopt failed with error code %d\n", WSAGetLastError());
        }
    }
    return rc;
}

int SetSendInterface(SOCKET s, struct addrinfo* iface)
{
    char* optval = NULL;
    int optlevel, option, optlen, rc;
    rc = NO_ERROR;

    if (iface->ai_family == AF_INET)
    {
        optlevel = IPPROTO_IP;
        option = IP_MULTICAST_IF;
        optval = (char*) &((SOCKADDR_IN*) iface->ai_addr)->sin_addr.s_addr;
        optlen = sizeof(((SOCKADDR_IN*) iface->ai_addr)->sin_addr.s_addr);
    }
    else
    {
        fprintf(stderr, "Attempting to set sent interface for invalid address family!\n");
        rc = SOCKET_ERROR;
    }

    if (rc != SOCKET_ERROR)
    {
        const SOCKADDR_IN* inaddr = (SOCKADDR_IN*) (iface->ai_addr);

        PRINT_DEBUG("setsockopt(%d, IPPROTO_IP, IP_MULTICAST_IF, %d.%d.%d.%d:%d)\n", (int) s,
                    inaddr->sin_addr.S_un.S_un_b.s_b1, inaddr->sin_addr.S_un.S_un_b.s_b2,
                    inaddr->sin_addr.S_un.S_un_b.s_b3, inaddr->sin_addr.S_un.S_un_b.s_b4,
                    (int) htons(inaddr->sin_port));
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            PRINT_DEBUG("setsockopt() failed with error code %d\n", WSAGetLastError());
        }
    }
    return rc;
}

struct addrinfo* ResolveAddress(const char* addr, const char* port, int af, int type, int proto)
{
    struct addrinfo hints, *res = NULL;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = af;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    rc = getaddrinfo(addr, port, &hints, &res);
    if (rc != 0)
    {
        PRINT_DEBUG("Invalid address %s, getaddrinfo failed: %d\n", addr, rc);
        return NULL;
    }
    return res;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  SOPC_Socket* pSock)
{
    SOCKET s;
    SOCKADDR_IN localif;
    struct ip_mreq mreq;

    struct sockaddr_in* listenAddr;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    *pSock = malloc(sizeof(SOPC_Socket_Impl));

    if (pSock == NULL)
    {
        closesocket(s);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    PRINT_DEBUG("socket(RCV) => %lld \n", s);

    listenAddr = (struct sockaddr_in*) get_ai_addr(listenAddress);

    localif.sin_family = AF_INET;
    localif.sin_port = listenAddr->sin_port;
    localif.sin_addr.s_addr = htonl(INADDR_ANY);

    int res = bind(s, (SOCKADDR*) &localif, sizeof(localif));
    PRINT_DEBUG("bind(%d, %d.%d.%d.%d:%d)\n", (int) s, localif.sin_addr.S_un.S_un_b.s_b4,
                localif.sin_addr.S_un.S_un_b.s_b3, localif.sin_addr.S_un.S_un_b.s_b2, localif.sin_addr.S_un.S_un_b.s_b1,
                (int) htons(localif.sin_port));

    if (res == SOCKET_ERROR)
    {
        PRINT_DEBUG("Failed to Bind socket %d on port %d (%s)\n", (int) (*pSock)->sock, htons(localif.sin_port),
                    gai_strerrorA(WSAGetLastError()));
        SOPC_UDP_Socket_Close(pSock);

        return SOPC_STATUS_NOK;
    }

    // Note: only add membership if socket is a Multicast address
    bool isMC = listenAddress->addrInfo.ai_family == AF_INET;
    if (isMC)
    {
        // IPV4: first address byte indicates if this is a multicast address
        struct sockaddr_in* sAddr = (struct sockaddr_in*) get_ai_addr(listenAddress);
        const uint32_t ip = htonl(sAddr->sin_addr.s_addr);
        isMC = ((ip >> 28) & 0xF) == 0xE; // Multicast mask on 4 first bytes;
    }

    if (isMC)
    {
        PRINT_DEBUG("Detected a Multicast address. Set option 'IP_ADD_MEMBERSHIP'\n");

        struct in_addr interfaceAddr;
        int ptonResult = inet_pton(AF_INET, interfaceName, &interfaceAddr);
        if (ptonResult != 1)
        {
            PRINT_DEBUG("inet_pton failed for interfaceName: %s\n", interfaceName);
            closesocket(s);
            return SOPC_STATUS_INVALID_PARAMETERS;
        }

        mreq.imr_interface = interfaceAddr;
        mreq.imr_multiaddr = listenAddr->sin_addr;

        PRINT_DEBUG("setsockopt(%d, IPPROTO_IP, IP_ADD_MEMBERSHIP, (%s / %d.%d.%d.%d:%d))\n", (int) s, interfaceName,
                    listenAddr->sin_addr.S_un.S_un_b.s_b4, listenAddr->sin_addr.S_un.S_un_b.s_b3,
                    listenAddr->sin_addr.S_un.S_un_b.s_b2, listenAddr->sin_addr.S_un.S_un_b.s_b1,
                    htons(listenAddr->sin_port));
        res = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
        if (res == SOCKET_ERROR)
        {
            PRINT_DEBUG("Failed to IP_ADD_MEMBERSHIP on socket %d  (%s)\n", (int) (*pSock)->sock,
                        gai_strerrorA(WSAGetLastError()));
            SOPC_UDP_Socket_Close(pSock);
            return SOPC_STATUS_NOK;
        }
    }

    //*sock = s;
    (*pSock)->sock = s;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               SOPC_Socket* pSock)
{
    if (pSock == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *pSock = malloc(sizeof(SOPC_Socket_Impl));

    if (pSock == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    return SOPC_UDP_Socket_CreateNew(destAddress, interfaceName, 0, setNonBlocking, &(*pSock)->sock);
}

SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(SOPC_Socket pSock,
                                         const SOPC_Socket_AddressInfo* destAddr,
                                         SOPC_Buffer* buffer)
{
    if (pSock->sock == INVALID_SOCKET || destAddr == NULL || buffer == NULL || buffer->position != 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOCKADDR_IN* inaddr = (SOCKADDR_IN*) (destAddr->addrInfo.ai_addr);
    PRINT_DEBUG("sendto(%d, (...), %u, to (%d.%d.%d.%d:%d))\n", (int) pSock->sock, (int) buffer->length,
                inaddr->sin_addr.S_un.S_un_b.s_b1, inaddr->sin_addr.S_un.S_un_b.s_b2, inaddr->sin_addr.S_un.S_un_b.s_b3,
                inaddr->sin_addr.S_un.S_un_b.s_b4, htons(inaddr->sin_port));
    int res = sendto(pSock->sock, (const char*) buffer->data, buffer->length, 0, destAddr->addrInfo.ai_addr,
                     (int) destAddr->addrInfo.ai_addrlen);

    if (res == SOCKET_ERROR || (uint32_t) res != buffer->length)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(SOPC_Socket pSock, SOPC_Buffer* buffer)
{
    if (pSock->sock == INVALID_SOCKET || buffer == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    struct sockaddr_in si_client;
    socklen_t slen = sizeof(si_client);

    int recv_len =
        recvfrom(pSock->sock, (char*) buffer->data, buffer->current_size, 0, (struct sockaddr*) &si_client, &slen);

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

void SOPC_UDP_Socket_Close(SOPC_Socket* pSock)
{
    //  if (sock && *sock != INVALID_SOCKET)
    if (pSock != NULL && *pSock != NULL && (*pSock)->sock && (*pSock)->sock != INVALID_SOCKET)
    {
        closesocket((*pSock)->sock);
        (*pSock)->sock = INVALID_SOCKET;
    }
}
