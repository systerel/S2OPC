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

#include "lwip/errno.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#define SOPC_INVALID_SOCKET (-1)
#define SOMAXCONN (2)

/**
 *  \brief Socket base type
 */
typedef int Socket;

/**
 *  \brief Socket addressing information for listening or connecting operation type
 */
typedef struct addrinfo SOPC_Socket_AddressInfo;

/**
 *  \brief Set of sockets type
 */
typedef struct
{
    int fdmax;  /**< max of the set */
    fd_set set; /**< set */
    uint8_t rfu[2];
} SOPC_SocketSet;

#endif /* SOPC_P_SOCKETS_H_ */
