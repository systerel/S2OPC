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

#ifndef SOPC_SOCKETS_EVENT_MGR_H_
#define SOPC_SOCKETS_EVENT_MGR_H_

#include <stdint.h>

#include "sopc_sockets_api.h"
#include "sopc_sockets_internal_ctx.h"

typedef enum
{
    /* Internal events (network event manager to event manager) */
    INT_SOCKET_LISTENER_CONNECTION_ATTEMPT = 0x10, /* idx of listening socket */
    INT_SOCKET_CONNECTION_ATTEMPT_FAILED,          /* idx of socket for the rest */
    INT_SOCKET_CONNECTED,
    INT_SOCKET_CLOSE,
    INT_SOCKET_READY_TO_READ,
    INT_SOCKET_READY_TO_WRITE
} SOPC_Sockets_InternalInputEvent;

void SOPC_SocketsEventMgr_Dispatcher(SOPC_Sockets_InputEvent socketEvent,
                                     uint32_t eltId,
                                     uintptr_t params,
                                     uintptr_t auxParam);

void SOPC_SocketsInternalEventMgr_Dispatcher(SOPC_Sockets_InternalInputEvent event, SOPC_Socket* socketElt);

#endif /* SOPC_SOCKETS_EVENT_MGR_H_ */
