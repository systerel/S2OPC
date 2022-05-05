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

#include "sopc_eth_sockets.h"

#include "p_sockets.h"
#include "sopc_assert.h"
#include "sopc_macros.h"

SOPC_ReturnStatus SOPC_ETH_Socket_CreateSendAddressInfo(const char* interfaceName,
                                                        const char* destMACaddr,
                                                        SOPC_ETH_Socket_SendAddressInfo** sendAddInfo)
{
    SOPC_UNUSED_ARG(interfaceName);
    SOPC_UNUSED_ARG(destMACaddr);
    SOPC_UNUSED_ARG(sendAddInfo);
    return SOPC_STATUS_NOT_SUPPORTED;
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
    SOPC_UNUSED_ARG(recvAddInfo);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                                  bool setNonBlocking,
                                                  Socket* sock)
{
    SOPC_UNUSED_ARG(receiveAddrInfo);
    SOPC_UNUSED_ARG(setNonBlocking);
    SOPC_UNUSED_ARG(sock);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToSend(SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                               bool setNonBlocking,
                                               Socket* sock)
{
    SOPC_UNUSED_ARG(sendAddrInfo);
    SOPC_UNUSED_ARG(setNonBlocking);
    SOPC_UNUSED_ARG(sock);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_SendTo(Socket sock,
                                         const SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                         uint16_t etherType,
                                         SOPC_Buffer* buffer)
{
    SOPC_UNUSED_ARG(sendAddrInfo);
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(etherType);
    SOPC_UNUSED_ARG(buffer);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ETH_Socket_ReceiveFrom(Socket sock,
                                              const SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                              bool checkEtherType,
                                              uint16_t etherType,
                                              SOPC_Buffer* buffer)
{
    SOPC_UNUSED_ARG(receiveAddrInfo);
    SOPC_UNUSED_ARG(sock);
    SOPC_UNUSED_ARG(checkEtherType);
    SOPC_UNUSED_ARG(etherType);
    SOPC_UNUSED_ARG(buffer);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_ETH_Socket_Close(Socket* sock)
{
    SOPC_UNUSED_ARG(sock);
    SOPC_ASSERT(false);
}
