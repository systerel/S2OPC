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
 *  \brief A platform independent API to use packet sockets
 *
 *  \warning: packet socket require administrative rights
 */

// TODO: only Linux implementation provided

#ifndef SOPC_ETH_SOCKETS_H_
#define SOPC_ETH_SOCKETS_H_

#include <stdbool.h>

// Platform dependent types
#include "p_sockets.h"

#include "sopc_buffer.h"
#include "sopc_enums.h"

// Ethernet header size includes DMAC (6) + SMAC (6) +  TYPE (2)
#define ETHERNET_HEADER_SIZE 14

/**
 *  \brief Socket addressing information for sending operation type
 */
typedef struct SOPC_ETH_Socket_SendAddressInfo SOPC_ETH_Socket_SendAddressInfo;
/**
 *  \brief Socket addressing information for listening operation type
 */
typedef struct SOPC_ETH_Socket_ReceiveAddressInfo SOPC_ETH_Socket_ReceiveAddressInfo;

/**
 *  \brief Create a new address information for packet sending
 *
 *  \param interfaceName     Name of the network interface to use to send packet (mandatory)
 *  \param destMACaddr       Destination MAC address to send packet as a string representation (mandatory).
 *                           It shall be hexadecimal values separated by character ':' and zero terminated.
 *
 *                           E.g.: "01-00-00-01-04-00"
 *  \param[out] sendAddInfo  Value pointed is set with a newly allocated and fulfilled send address information.
 *                           Caller shall call ::SOPC_Free when not used anymore.
 *
 *  \return                SOPC_STATUS_OK if operation succeeded,
 *                         SOPC_OUT_OF_MEMORY in case of allocation failure,
 *                         SOPC_STATUS_INVALID_PARAMETERS in case of incorrect parameters leading to failure.
 */
SOPC_ReturnStatus SOPC_ETH_Socket_CreateSendAddressInfo(const char* interfaceName,
                                                        const char* destMACaddr,
                                                        SOPC_ETH_Socket_SendAddressInfo** sendAddInfo);

/**
 *  \brief Create a new address information for packet reception.
 *
 *
 *  \param interfaceName          Name of the network interface to use to receive packets (optional)
 *  \param recvMulticast          If set, multicast packet reception is active.
 *                                A multicast address should be specified in \p destMACaddr,
 *                                otherwise packet reception is active for all multicast addresses.
 *  \param destMACaddr            Destination MAC address of packets accepted as a string representation (optional).
 *                                It shall be hexadecimal values separated by character ':' and zero terminated.
 *                                It shall be set only if \p destMACaddr is set.
 *                                E.g.: "01-00-00-01-04-00"
 *  \param sourceMACaddr          Source MAC address of packets accepted as a string representation (optional).
 *                                It shall be hexadecimal values separated by character ':' and zero terminated.
 *                                E.g.: "0A:00:00:01:04:00"
 *  \param[out] recvAddInfo       Value pointed is set with a newly allocated and fulfilled receive address information.
 *                                Caller shall call ::SOPC_Free when not used anymore.
 *
 *  \return                       SOPC_STATUS_OK if operation succeeded,
 *                                SOPC_OUT_OF_MEMORY in case of allocation failure,
 *                                SOPC_STATUS_INVALID_PARAMETERS in case of incorrect parameters leading to failure.
 */
SOPC_ReturnStatus SOPC_ETH_Socket_CreateReceiveAddressInfo(const char* interfaceName,
                                                           bool recvMulticast,
                                                           const char* destMACaddr,
                                                           const char* sourceMACaddr,
                                                           SOPC_ETH_Socket_ReceiveAddressInfo** recvAddInfo);

/**
 *  \brief Create a new ETH socket, bind it using and add membership for multicast if active in \p receiveAddrInfo.
 *         Membership for multicast is added on specified interface in \p receiveAddrInfo
 *         or all compatible interfaces otherwise.
 *
 *  \param receiveAddrInfo Receive address information. It is used to bind socket to particular interface if defined
 *                         and to add membership for multicast if defined.
 *  \param setNonBlocking  Set the socket as non blocking
 *  \param[out] sock       Value pointed is set with the newly created socket.
 *
 *  \return                SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS in case of incorrect
 *                         parameters, SOPC_STATUS_NOT_SUPPORTED in case of multicast activation impossibility
 *                         and SOPC_STATUS_NOK in case of socket error.
 */
SOPC_ReturnStatus SOPC_ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                                  bool setNonBlocking,
                                                  Socket* sock);

/**
 *  \brief Create a new ETH socket using \p sendAddrInfo properties.
 *
 *  \param sendAddrInfo    Send address information
 *  \param setNonBlocking  Set the socket as non blocking
 *  \param[out] sock       Value pointed is set with the newly created socket.
 *
 *  \return                SOPC_STATUS_OK if operation succeeded, SOPC_STATUS_INVALID_PARAMETERS in case of incorrect
 *                         parameters and SOPC_STATUS_NOK in case of socket error.
 */
SOPC_ReturnStatus SOPC_ETH_Socket_CreateToSend(SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                               bool setNonBlocking,
                                               Socket* sock);

/**
 *  \brief Send data through the ETH socket to given IP address and port
 *
 *  \param sock              The socket used for sending
 *  \param sendAddrInfo      The address information for sending (source interface and destination MAC address)
 *  \param etherType         The EtherType value to indicate in Ethernet header
 *  \param buffer            The buffer containing data to be sent. Buffer containing buffer->length bytes.
 *
 *  \return        SOPC_STATUS_OK if operation succeeded,
 *                 SOPC_STATUS_OUT_OF_MEMORY in case of allocation failure,
 *                 SOPC_STATUS_WOULD_BLOCK if not all bytes were sent,
 *                 SOPC_STATUS_NOK otherwise otherwise.
 */
SOPC_ReturnStatus SOPC_ETH_Socket_SendTo(Socket sock,
                                         const SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                         uint16_t etherType,
                                         SOPC_Buffer* buffer);

/**
 *  \brief Receive data on the ETH socket from given address info
 *
 *  \param sock               The socket used for receiving
 *  \param receiveAddrInfo    The address information to use for reception.
 *                            Check source address, dest address and protocol version if configured.
 *  \param checkEtherType     If set, check Ethernet header protocol value of packet is \p etherType.
 *  \param etherType          The expected EtherType value in Ethernet header, packets are ignored if different
 *                            when \p checkEtherType is set.
 *  \param[in,out] buffer     The buffer with buffer->current_size bytes available.
 *                            buffer->length is updated on reception.
 *                            The content on reception includes the ethernet header content of 14 bytes:
 *                            - Destination MAC (6 bytes)
 *                            - Source MAC (6 bytes)
 *                            - EtherType
 *
 *  \return                   SOPC_STATUS_OK if operation succeeded,
 *                            SOPC_STATUS_INVALID_PARAMETERS in case of parameters issue,
 *                            SOPC_STATUS_OUT_OF_MEMORY in case buffer size might have been insufficient,
 *                            SOPC_STATUS_NOK otherwise.
 */
SOPC_ReturnStatus SOPC_ETH_Socket_ReceiveFrom(Socket sock,
                                              const SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                              bool checkEtherType,
                                              uint16_t etherType,
                                              SOPC_Buffer* buffer);

/**
 *  \brief Close the socket connection and/or clear the socket
 *
 *  \param sock     The socket to disconnect and/or clear
 */
void SOPC_ETH_Socket_Close(Socket* sock);

#endif /* SOPC_ETH_SOCKETS_H_ */
