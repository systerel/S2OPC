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

#include "p_sockets.h"

/* Multicast platform dep functions used by platform dep pub sub UDP */

/**
 * @brief
 *      Create a multicast group and sets sock in that group
 * @param sock The socket to register in multicast group
 * @param multicast The Multicast IPV4 address
 * @return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus P_MULTICAST_AddIpV4Membership(Socket sock, const SOPC_Socket_AddressInfo* multicast);

/**
 * @brief
 *      Removes a multicast group
 * @param sock The socket to remove from a multicast group
 * @param multicast The Multicast IPV4 address, or NULL if unknown
 * @return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus P_MULTICAST_DropIpV4Membership(Socket sock, const SOPC_Socket_AddressInfo* multicast);

#endif /* P_MCAST_H_ */
