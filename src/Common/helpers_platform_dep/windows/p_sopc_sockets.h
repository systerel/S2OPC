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

#ifndef SOPC_P_SOCKETS_H_
#define SOPC_P_SOCKETS_H_

#include <winsock2.h>
#include <ws2tcpip.h>

#define MAX_SEND_ATTEMPTS 20
#define SLEEP_NEXT_SEND_ATTEMP 50 // milliseconds

/**
 *  \brief Socket base type
 */
struct SOPC_Socket_Impl
{
    SOCKET sock;
};

/**
 *  \brief Socket addressing information for listening or connecting operation type
 *  \note Internal treatment use the fact it is the first field as property
 */
struct SOPC_Socket_AddressInfo
{
    struct addrinfo addrInfo;
};

/**
 *  \brief Socket address information on a connected socket
 *  \note Internal treatment use the fact it is the first field as property
 *
 */
struct SOPC_Socket_Address
{
    struct addrinfo address;
};

/**
 *  \brief Set of sockets type
 */
struct SOPC_SocketSet
{
    WSAPOLLFD* fds;
    size_t size;  /**< Actual size of fds */
    size_t count; /**< Current number of used items in fds */
};

#endif /* SOPC_P_SOCKETS_H_ */
