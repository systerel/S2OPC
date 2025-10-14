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

#include <windows.h>
#include <winsock2.h>
#include "p_sopc_sockets.h"
#include "sopc_common_constants.h"
#include "sopc_eth_sockets.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

struct SOPC_ETH_Socket_ReceiveAddressInfo
{
    // struct sockaddr_ll addr;
    bool recvMulticast;
    bool recvForDest;
    unsigned char recvDestAddr[6];
    bool recvFromSource;
    unsigned char recvSourceAddr[6];
};

SOPC_ReturnStatus SOPC_ETH_Socket_CreateSendAddressInfo(const char* interfaceName,
                                                        const char* destMACaddr,
                                                        SOPC_ETH_Socket_SendAddressInfo** sendAddInfo)
{
    SOPC_UNUSED_ARG(interfaceName);
    SOPC_UNUSED_ARG(destMACaddr);
    SOPC_UNUSED_ARG(sendAddInfo);

    // Ethernet raw socket not supported on Windows
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToSend(SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                               bool setNonBlocking,
                                               SOPC_Socket* sock)
{
    SOPC_UNUSED_ARG(sendAddrInfo);
    SOPC_UNUSED_ARG(setNonBlocking);
    SOPC_UNUSED_ARG(sock);

    // Ethernet raw socket not supported on Windows
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_SendTo(SOPC_Socket sock,
                                         const SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                         uint16_t etherType,
                                         SOPC_Buffer* buffer)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(sendAddrInfo);
    SOPC_UNUSED_ARG(etherType);
    SOPC_UNUSED_ARG(buffer);

    // Ethernet raw socket not supported on Windows
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_ETH_Socket_Close(SOPC_Socket* sock)
{
    if (NULL != sock && SOPC_INVALID_SOCKET != *sock)
    {
        // There is no concept of EINTR on Windows, so we don't need to retry
        closesocket((*sock)->sock);
        SOPC_Free(*sock);
        *sock = SOPC_INVALID_SOCKET;
    }
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateReceiveAddressInfo(const char* interfaceName,
                                                           bool recvMulticast,
                                                           const char* destMACaddr,
                                                           const char* sourceMACaddr,
                                                           SOPC_ETH_Socket_ReceiveAddressInfo** recvAddInfo)
{
    SOPC_UNUSED_ARG(interfaceName);
    SOPC_UNUSED_ARG(recvMulticast);
    SOPC_UNUSED_ARG(destMACaddr);
    SOPC_UNUSED_ARG(sourceMACaddr);

    if (NULL == recvAddInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Dump allocation for API
    *recvAddInfo = SOPC_Calloc(1, sizeof(SOPC_ETH_Socket_ReceiveAddressInfo));
    if (NULL == *recvAddInfo)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Marque le stub comme "non fonctionnel" si jamais appelé.
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                                  bool setNonBlocking,
                                                  SOPC_Socket* sock)
{
    SOPC_UNUSED_ARG(receiveAddrInfo);
    SOPC_UNUSED_ARG(setNonBlocking);

    if (NULL == sock)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Not supported on Windows
    *sock = SOPC_INVALID_SOCKET;

    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_ReceiveFrom(SOPC_Socket sock,
                                              const SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                              bool checkEtherType,
                                              uint16_t etherType,
                                              SOPC_Buffer* buffer)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(receiveAddrInfo);
    SOPC_UNUSED_ARG(checkEtherType);
    SOPC_UNUSED_ARG(etherType);
    SOPC_UNUSED_ARG(buffer);

    // Ethernet raw socket not supported on Windows
    return SOPC_STATUS_NOT_SUPPORTED;
}
