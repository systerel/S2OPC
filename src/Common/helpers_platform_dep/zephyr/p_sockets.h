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
 * @brief
 *  Implementation of SOPC sockets in scope of ZEPHYR OS
 */
#ifndef SOPC_ZEPHYR_P_SOCKETS_HEADER_
#define SOPC_ZEPHYR_P_SOCKETS_HEADER_

#include <stdbool.h>
#include <stdint.h>

#include "net/socket.h"

#include "sopc_enums.h"

#define SOPC_INVALID_SOCKET (-1)

/* Socket definition */

typedef int Socket;

/* Addr info redefinition */

typedef struct zsock_addrinfo SOPC_Socket_AddressInfo;

/* Socket set definition*/

typedef struct
{
    int32_t fdmax;    /**< max of the set */
    zsock_fd_set set; /**< set */
} SOPC_SocketSet;

#endif /* SOPC_ZEPHYR_P_SOCKETS_HEADER_ */
