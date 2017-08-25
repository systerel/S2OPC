/*
 *  \file sopc_secure_channels_api.h
 *
 *  \brief Event oriented API of the Secure Channle layer.
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

#ifndef _SOPC_SECURE_CHANNELS_API_H_
#define _SOPC_SECURE_CHANNELS_API_H_

#include <stdint.h>

/* Sockets input events */
typedef enum {
    /* External events */
    /*  Socket events */
    SOCKET_LISTENER_OPENED,/* id = endpoint description config index,
                              auxParam = socket index
                          */
    SOCKET_LISTENER_CONNECTION,/* id = endpoint description config index,
                                  auxParam = new connection socket index
                              */
    SOCKET_LISTENER_FAILURE,/* id = endpoint description config index */

    SOCKET_CONNECTION, /* id = secure channel connection index,
                          auxParam = socket index */

    SOCKET_FAILURE, /* id = secure channel connection index,
                       auxParam = socket index */
    SOCKET_RCV_BYTES, /* id = secure channel connection index,
                         params = (SOPC_Buffer*) received buffer
                       */
    /* Internal events */
} SOPC_SecureChannels_InputEvent;

/* Sockets event enqueue function */
void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t                       id,
                                      void*                          params,
                                      int32_t                        auxParam);

void SOPC_SecureChannels_Initialize();

void SOPC_SecureChannels_Clear();

#endif /* _SOPC_SECURE_CHANNELS_API_H_ */
