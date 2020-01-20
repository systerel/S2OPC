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

#ifndef SOPC_P_SOCKETS_HEADER_
#define SOPC_P_SOCKETS_HEADER_

#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __INT32_MAX__
#include <toolchain/xcc_missing_defs.h>
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

#include <fcntl.h>
#include <kernel.h>
#include <net/socket.h>

#define SOPC_INVALID_SOCKET (-1)

#ifdef CONFIG_NET_SOCKETS_POLL_MAX
#define MAX_PENDING_CONNECTION CONFIG_NET_SOCKETS_POLL_MAX
#else
#define MAX_PENDING_CONNECTION 4
#endif

#ifdef CONFIG_NET_MAX_CONN
#define MAX_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_SOCKET 4
#endif

#ifndef CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT
#define CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT 16
#endif

#define MAX_MCAST CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT

/**
 *  \brief Socket base type
 */
typedef int Socket;

/**
 *  \brief Socket addressing information for listening or connecting operation type
 */
typedef struct zsock_addrinfo SOPC_Socket_AddressInfo;

/**
 *  \brief Set of sockets type
 */
typedef struct
{
    int32_t fdmax;    /**< max of the set */
    zsock_fd_set set; /**< set */
} SOPC_SocketSet;

bool P_SOCKET_NETWORK_IsInitialized(void);

SOPC_ReturnStatus P_SOCKET_MCAST_add_sock_to_mcast(int sock, struct in_addr* add);
SOPC_ReturnStatus P_SOCKET_MCAST_join_mcast_group(int sock, struct in_addr* add);
SOPC_ReturnStatus P_SOCKET_MCAST_leave_mcast_group(int sock, struct in_addr* add);
bool P_SOCKET_MCAST_soft_filter(int sock, struct in_addr* add);
void P_SOCKET_MCAST_remove_sock_from_mcast(int sock);

uint32_t P_SOCKET_increment_nb_socket(void);
uint32_t P_SOCKET_decrement_nb_socket(void);

#endif /* SOPC_P_SOCKETS_H_ */
