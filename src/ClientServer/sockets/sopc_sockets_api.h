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
 * \file sopc_sockets_api.h
 *
 * \brief Event oriented API of the Sockets layer.
 *
 *   This module is in charge of the event dispatcher thread management.
 */

#ifndef SOPC_SOCKETS_API_H_
#define SOPC_SOCKETS_API_H_

#include <stdint.h>

#include "sopc_event_handler.h"

/* Sockets input events */
typedef enum
{
    /* External events */
    SOCKET_CREATE_SERVER = 0x0, /* id = endpoint description config index,
                             params = (const char*) URI,
                             auxParam = (bool) listenAllInterfaces
                          */
    SOCKET_ACCEPTED_CONNECTION, /* id = socket index,
                                   auxParam = secure channel connection index */

    SOCKET_CREATE_CLIENT, /* id = secure channel connection index,
                             params = (const char*) URI
                          */
    SOCKET_CLOSE,         /* id = socket index, auxParam = secure channel connection index  */
    SOCKET_CLOSE_SERVER,  /* id = socket index, auxParam = endpoint description config index */
    SOCKET_WRITE          /* id = socket index,
                             params = (SOPC_Buffer*) message buffer */
} SOPC_Sockets_InputEvent;

typedef enum
{
    SOCKET_LISTENER_OPENED = 0x100, /* id = endpoint description config index,
                               auxParam = (uint32_t) socket index
                           */
    SOCKET_LISTENER_CONNECTION,     /* id = endpoint description config index,
                                       auxParam = (uint32_t) new connection socket index
                                   */
    SOCKET_LISTENER_FAILURE,        /* id = endpoint description config index */

    SOCKET_CONNECTION, /* id = secure channel connection index,
                          auxParam = (uint32_t) socket index */

    SOCKET_FAILURE,   /* id = secure channel connection index,
                         auxParam = (uint32_t) socket index */
    SOCKET_RCV_BYTES, /* id = secure channel connection index,
                         params = (SOPC_Buffer*) received buffer containing complete TCP UA chunk
                       */
} SOPC_Sockets_OutputEvent;

/* Sockets event enqueue function */
void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent socketEvent, uint32_t id, uintptr_t params, uintptr_t auxParam);

void SOPC_Sockets_Initialize(void);

void SOPC_Sockets_SetEventHandler(SOPC_EventHandler* handler);

void SOPC_Sockets_Clear(void);

#endif /* SOPC_SOCKETS_API_H_ */
