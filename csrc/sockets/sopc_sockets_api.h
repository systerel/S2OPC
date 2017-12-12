/*
 *  \file sopc_sockets_api.h
 *
 *  \brief Event oriented API of the Sockets layer.
 *         This module is in charge of the event dispatcher thread management.
 */
/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_SOCKETS_API_H_
#define SOPC_SOCKETS_API_H_

#include <stdint.h>

/* Sockets input events */
typedef enum {
    /* External events */
    SOCKET_CREATE_SERVER,       /* id = endpoint description config index,
                                   params = (const char*) URI,
                                   auxParam = (bool) listenAllInterfaces
                                */
    SOCKET_ACCEPTED_CONNECTION, /* id = socket index,
                                   auxParam = secure channel connection index */

    SOCKET_CREATE_CLIENT, /* id = secure channel connection index,
                             params = (const char*) URI
                          */
    SOCKET_CLOSE,         /* id = socket index */
    SOCKET_WRITE,         /* id = socket index,
                             params = (SOPC_Buffer*) message buffer
                          */

    /* Internal events (network event manager to event manager) */
    INT_SOCKET_LISTENER_CONNECTION_ATTEMPT, /* idx of listening socket */
    INT_SOCKET_CONNECTION_ATTEMPT_FAILED,   /* idx of socket for the rest */
    INT_SOCKET_CONNECTED,
    INT_SOCKET_CLOSE,
    INT_SOCKET_READY_TO_READ,
    INT_SOCKET_READY_TO_WRITE
} SOPC_Sockets_InputEvent;

/* Sockets event enqueue function */
void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent socketEvent, uint32_t id, void* params, int32_t auxParam);

void SOPC_Sockets_Initialize(void);

void SOPC_Sockets_Clear(void);

#endif /* SOPC_SOCKETS_API_H_ */
