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

#ifndef PUBSUB_S2OPC_PUBSUB_SRC_DEP_PLATFORM_ZEPHYR_P_UDP_SOCKETS_H_
#define PUBSUB_S2OPC_PUBSUB_SRC_DEP_PLATFORM_ZEPHYR_P_UDP_SOCKETS_H_

#include <stdint.h>

#include "sopc_enums.h"

/* Multicast platform dep functions used by platform dep pub sub UDP */

__weak SOPC_ReturnStatus P_SOCKET_MCAST_add_sock_to_mcast(int32_t sock, struct in_addr* add);
__weak SOPC_ReturnStatus P_SOCKET_MCAST_join_mcast_group(int32_t sock, struct in_addr* add);
__weak SOPC_ReturnStatus P_SOCKET_MCAST_leave_mcast_group(int32_t sock, struct in_addr* add);
__weak bool P_SOCKET_MCAST_soft_filter(int32_t sock, struct in_addr* add);
__weak void P_SOCKET_MCAST_remove_sock_from_mcast(int32_t sock);

#endif /* PUBSUB_S2OPC_PUBSUB_SRC_DEP_PLATFORM_ZEPHYR_P_UDP_SOCKETS_H_ */
