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

#ifndef P_MCAST_H
#define P_MCAST_H

#include <stdint.h>

#include "sopc_enums.h"

/* Multicast platform dep functions used by platform dep pub sub UDP */

SOPC_ReturnStatus P_MULTICAST_join_or_leave_mcast_group(int32_t sock, struct in_addr* add, bool bJoinRequest);
bool P_MULTICAST_soft_filter(int32_t sock, struct in_addr* add);
void P_MULTICAST_remove_sock_from_mcast(int32_t sock);
void P_MULTICAST_Initialize(void);
void P_MULTICAST_enet_add_mcast(struct net_if* ptrNetIf, struct in_addr* pAddr);
void P_MULTICAST_enet_rm_mcast(struct net_if* ptrNetIf, struct in_addr* pAddr);

#endif /* P_MCAST_H_ */
