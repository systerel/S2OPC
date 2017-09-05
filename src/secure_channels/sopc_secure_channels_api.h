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

/* Secure channel input events */
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
    /* Services events */
    EP_OPEN, /* id = endpoint description configuration index */
    EP_CLOSE, /* id = endpoint description configuration index */
    SC_CONNECT, /* id = endpoint connection description configuration index */
    SC_DISCONNECT, /* id = secure channel connection index */
    SC_SERVICE_SND_MSG, /* id = secure channel connection index,
                          params = (SOPC_Buffer*) received buffer,
                          auxParam = request Id context (optional: defined if  >= 0) */

    /* Internal events */
    /* SC listener manager -> SC connection manager */
    INT_EP_SC_CREATE, /* id = endpoint description configuration index,
                         auxParam = socket index */
    INT_EP_SC_CLOSE, /* id = secure channel connection index,
                        auxParam = endpoint description configuration index */
    /* SC connection manager -> SC listener manager */
    INT_EP_SC_CREATED, /* id = endpoint description configuration index,
                          auxParam = secure channel connection index */
    INT_EP_SC_DISCONNECTED, /* id = endpoint description configuration index,
                               auxParam = secure channel connection index */

    /* OPC UA chunks message manager -> SC connection manager */
    INT_SC_RCV_HEL, /* >------------------------- */
    INT_SC_RCV_ACK, // id = secure channel connection index,
                    // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_RCV_ERR, /* -------------------------< */

    INT_SC_RCV_OPN, /* >------------------------- */
                    // id = secure channel connection index,
                    // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_RCV_CLO, // auxParam = request Id context if request
    INT_SC_RCV_MSG_CHUNKS, /* -------------------------< */

    INT_SC_RCV_FAILURE, /* id = secure channel connection index,
                           auxParam = error status */
    INT_SC_SND_FAILURE, /* id = secure channel connection index,
                           params = (SOPC_Buffer*) buffer provided for encoding,
                           auxParam = error status in case of client */

    /* SC connection manager -> OPC UA chunks message manager */
    INT_SC_SND_HEL,        /* >------------------------- */
    INT_SC_SND_ACK,        // id = secure channel connection index,
                           // params = (SOPC_Buffer*) buffer positioned to message payload
                           /* -------------------------< */
    INT_SC_SND_ERR,
    INT_SC_SND_OPN,        /* >------------------------- */
                           // id = secure channel connection index,
                           // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_SND_CLO,        // auxParam = request Id context if response
    INT_SC_SND_MSG_CHUNKS, /* -------------------------< */

    /* SC connection manager -> SC connection manager */
    INT_SC_CLOSE // id = secure channel connection index

} SOPC_SecureChannels_InputEvent;

/* Secure channel external event enqueue function
 * IMPORTANT NOTE: internal events use will cause an assertion error */
void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t                       id,
                                      void*                          params,
                                      int32_t                        auxParam);

void SOPC_SecureChannels_Initialize();

void SOPC_SecureChannels_Clear();

#endif /* _SOPC_SECURE_CHANNELS_API_H_ */
