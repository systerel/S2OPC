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
 *  \file sopc_secure_channels_api.h
 *
 *  \brief Event oriented API of the Secure Channel layer.
 *
 *         This module is in charge of the event dispatcher thread management.
 */

#ifndef SOPC_SECURE_CHANNELS_API_H_
#define SOPC_SECURE_CHANNELS_API_H_

#include <stdint.h>

/**
 *  \brief Secure channel input events
 *  */
typedef enum {
    /* External events */
    /*  Socket events */
    SOCKET_LISTENER_OPENED,     /* id = endpoint description config index,
                                   auxParam = (uint32_t) socket index
                               */
    SOCKET_LISTENER_CONNECTION, /* id = endpoint description config index,
                                   auxParam = (uint32_t) new connection socket index
                               */
    SOCKET_LISTENER_FAILURE,    /* id = endpoint description config index */

    SOCKET_CONNECTION, /* id = secure channel connection index,
                          auxParam = (uint32_t) socket index */

    SOCKET_FAILURE,   /* id = secure channel connection index,
                         auxParam = (uint32_t) socket index */
    SOCKET_RCV_BYTES, /* id = secure channel connection index,
                         params = (SOPC_Buffer*) received buffer containing complete TCP UA chunk
                       */
    /* Services events */
    EP_OPEN,            /* id = endpoint description configuration index */
    EP_CLOSE,           /* id = endpoint description configuration index */
    SC_CONNECT,         /* id = secure channel connection configuration index */
    SC_DISCONNECT,      /* id = secure channel connection index */
    SC_SERVICE_SND_MSG, /* id = secure channel connection index,
                          params = (SOPC_Buffer*) buffer to send containing empty space for TCP UA header (24 bytes)
                          followed by encoded OpcUa message,
                          auxParam = (uint32_t) request Id context if response (server) / request Handle context if
                          request (client) */

    /* Timer events */
    TIMER_SC_CONNECTION_TIMEOUT, /* id = secure channel connection index */
    TIMER_SC_CLIENT_OPN_RENEW,   /* id = secure channel connection index */
    TIMER_SC_REQUEST_TIMEOUT,    /* id = secure channel connection index
                                    auxParam = (uint32_t) requestId
                                  */

    /* Internal events */
    /* SC listener manager -> SC connection manager */
    INT_EP_SC_CREATE, /* id = endpoint description configuration index,
                         auxParam = socket index */
    INT_EP_SC_CLOSE,  /* id = secure channel connection index,
                         auxParam = (uint32_t) endpoint description configuration index */
    /* SC connection manager -> SC listener manager */
    INT_EP_SC_CREATED,      /* id = endpoint description configuration index,
                               auxParam = (uint32_t) secure channel connection index */
    INT_EP_SC_DISCONNECTED, /* id = endpoint description configuration index,
                               auxParam = (uint32_t) secure channel connection index */

    /* OPC UA chunks message manager -> SC connection manager */
    INT_SC_RCV_HEL, /* >------------------------- */
    INT_SC_RCV_ACK, // id = secure channel connection index,
                    // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_RCV_ERR, /* -------------------------< */

    INT_SC_RCV_OPN,        /* >------------------------- */
                           // id = secure channel connection index,
                           // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_RCV_CLO,        // auxParam = (uint32_t) request Id context if request
    INT_SC_RCV_MSG_CHUNKS, /* -------------------------< */

    INT_SC_RCV_FAILURE, /* id = secure channel connection index,
                           auxParam = error status */
    INT_SC_SND_FAILURE, /* id = secure channel connection index,
                           params = (uint32_t *) requestId,
                           auxParam = (SOPC_StatusCode) error status in case of client */
    /* SC connection manager -> OPC UA chunks message manager */
    INT_SC_SND_HEL, /* >------------------------- */
    INT_SC_SND_ACK, // id = secure channel connection index,
                    // params = (SOPC_Buffer*) buffer positioned to message payload
                    /* -------------------------< */
    INT_SC_SND_ERR,
    INT_SC_SND_OPN,        /* >------------------------- */
                           // id = secure channel connection index,
                           // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_SND_CLO,        // auxParam = (uint32_t) request Id context if response / request Handle if request when MSG
    INT_SC_SND_MSG_CHUNKS, /* -------------------------< */

    /* SC connection manager -> SC connection manager */
    INT_SC_CLOSE // id = secure channel connection index,
                 // params = (char*) reason,
                 // auxParam = (SOPC_StatusCode) errorStatus

} SOPC_SecureChannels_InputEvent;

/* Secure channel external event enqueue function
 * IMPORTANT NOTE: internal events use will cause an assertion error */
void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t id,
                                      void* params,
                                      uintptr_t auxParam);

void SOPC_SecureChannels_Initialize(void);

void SOPC_SecureChannels_Clear(void);

#endif /* SOPC_SECURE_CHANNELS_API_H_ */
