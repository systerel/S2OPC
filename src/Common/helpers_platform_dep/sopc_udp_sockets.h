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

/**
 *  \file
 *
 *  \brief A platform independent API to use sockets
 */

// TODO: only Linux implementation provided

#ifndef SOPC_UDP_SOCKETS_H_
#define SOPC_UDP_SOCKETS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sopc_buffer.h"
#include "sopc_enums.h"
#include "sopc_raw_sockets.h"

/**
 *  \brief Create a new UDP socket address using getaddrinfo
 *
 * \param IPv6     Flag to activate IPv6 protocol
 * \param node     An IPv4 or IPv6 address or hostname. For default local address set to NULL.
 * \param service  The port to use.
 *
 *  \return     A instantiated address, NULL otherwise. It must be deleted when not used anymore.
 */
SOPC_Socket_AddressInfo* SOPC_UDP_SocketAddress_Create(bool IPv6, const char* node, const char* service);

void SOPC_UDP_SocketAddress_Delete(SOPC_Socket_AddressInfo** addr);

/**
 *  \brief Create a new UDP socket and bind it
 *
 *  \param listenAddress     Address on which the socket shall listen for input data
 *  \param interfaceName     The name of the interface to use, or null if unspecified.
 *                           The name shall be a null-terminated string if defined.
 *  \param setReuseAddr      If value is not false (0) the socket is configured to be reused
 *  \param setNonBlocking    If set the socket is non-blocking for reception
 *  \param[out] sock         Value pointed is set with the newly created socket
 *
 *  \return                  SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_UDP_Socket_CreateToReceive(SOPC_Socket_AddressInfo* listenAddress,
                                                  const char* interfaceName,
                                                  bool setReuseAddr,
                                                  bool setNonBlocking,
                                                  Socket* sock);

/**
 *  \brief Create a new UDP socket and do not bind it
 *
 *  \param destAddress     Destination IP address, used to determine version of the protocol
 *  \param interfaceName   The name of the interface to use, or null if unspecified.
 *                         The name shall be a null-terminated string if defined.
 *  \param setNonBlocking  If set the socket is non-blocking for sending
 *  \param[out] sock       Value pointed is set with the newly created socket
 *
 *  \return                SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_UDP_Socket_CreateToSend(SOPC_Socket_AddressInfo* destAddress,
                                               const char* interfaceName,
                                               bool setNonBlocking,
                                               Socket* sock);

/**
 *  \brief Send data through the UDP socket to given IP address and port
 *
 *  \param sock     The socket used for sending
 *  \param destAddr The destination IPv4 address
 *  \param buffer   The buffer containing data to be sent. Buffer considered with buffer->position 0 and containing
 * buffer->length bytes.
 *
 *  \return        SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_WOULD_BLOCK if partially sent, SOPC_STATUS_NOK
 * otherwise otherwise.
 */
SOPC_ReturnStatus SOPC_UDP_Socket_SendTo(Socket sock, const SOPC_Socket_AddressInfo* destAddr, SOPC_Buffer* buffer);

/**
 *  \brief Receive data on the UDP socket from given IP address and port
 *
 *  \param sock    The socket used for receiving
 *  \param buffer  The buffer with buffer->current_size bytes
 *
 *  \return        SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_OUT_OF_MEMORY if possible partial reception,
 * SOPC_STATUS_NOK
 */
SOPC_ReturnStatus SOPC_UDP_Socket_ReceiveFrom(Socket sock, SOPC_Buffer* buffer);

/**
 * \brief Join a multicast group with provided local IP address and multicast addess
 *
 *  \param sock           The socket to configure
 *  \param interfaceName  The name of the interface to use, or null if unspecified.
 *                        The name shall be a null-terminated string if defined.
 *  \param multicast      The multicast group IPv4 address (range 224.0.0.0 - 239.255.255.255)
 *  \param local          The local IPv4 address joining the group
 *
 *  \return        SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_UDP_Socket_AddMembership(Socket sock,
                                                const char* interfaceName,
                                                const SOPC_Socket_AddressInfo* multicast,
                                                const SOPC_Socket_AddressInfo* local);

/**
 * \brief Quit a multicast group with provided local IP address and multicast addess
 *
 *  \param sock           The socket to configure
 *  \param interfaceName  The name of the interface to use, or null if unspecified.
 *                        The name shall be a null-terminated string if defined.
 *  \param multicast      The multicast group IPv4 address (range 224.0.0.0 - 239.255.255.255)
 *  \param local          The local IPv4 address leaving the group
 *
 *  \return        SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_UDP_Socket_DropMembership(Socket sock,
                                                 const char* interfaceName,
                                                 const SOPC_Socket_AddressInfo* multicast,
                                                 const SOPC_Socket_AddressInfo* local);

/**
 *  \brief Set the Multicast TTL configuration value (default value is 1)
 *   Controls the live time of datagram (decremented by 1 by each router).
 *   In IPv4 multicasting is also used as threshold:
 *   - 0   : Restricted to current host
 *   - 1   : Restricted to same subnet
 *   - <32 : Restricted to same site
 *   - <64 : Restricted to same region
 *   - <128: Restricted to same continent
 *   - <255: Unrestricted
 *
 *  \param sock        The socket to configure
 *  \param TTL_scope   The TTL value to set
 *
 *  \return        SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_UDP_Socket_Set_MulticastTTL(Socket sock, uint8_t TTL_scope);

/**
 *  \brief Close the socket connection and/or clear the socket
 *
 *  \param sock     The socket to disconnect and/or clear
 */
void SOPC_UDP_Socket_Close(Socket* sock);

#endif /* SOPC_UDP_SOCKETS_H_ */
