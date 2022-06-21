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

#ifndef SOPC_SECURE_CHANNELS_API_INTERNAL_H_
#define SOPC_SECURE_CHANNELS_API_INTERNAL_H_

#include <stdint.h>
#include "sopc_secure_channels_api.h"

typedef enum
{
    /* SC listener manager -> SC connection manager */
    INT_EP_SC_CREATE = 0x400,  /* id = endpoint description configuration index,
                                  auxParam = socket index */
    INT_EP_SC_CLOSE,           /* id = secure channel connection index,
                                  auxParam = (uint32_t) endpoint description configuration index */
    INT_EP_SC_REVERSE_CONNECT, /* id = endpoint description configuration index,
                                  auxParam = (uint16_t) client to connect configuration index in endpoint config */
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

    INT_SC_RCV_OPN, /* >------------------------- */
                    // id = secure channel connection index,
                    // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_RCV_CLO, // auxParam = (uint32_t) requestId context if request (server side)
                    //                       / requestHandle if response (client side)
    INT_SC_RCV_MSG_CHUNKS,
    INT_SC_RCV_MSG_CHUNK_ABORT, /* -------------------------< */

    INT_SC_RCV_FAILURE,        /* id = secure channel connection index,
                                  auxParam = error status */
    INT_SC_SND_FATAL_FAILURE,  /* >------------------------- */
                               // id = secure channel connection index,
                               // params = requestId (server) / requestHandle (client)
                               // auxParam = (SOPC_StatusCode) error status in case of client */
    INT_SC_SENT_ABORT_FAILURE, /* -------------------------< */
    /* SC connection manager -> OPC UA chunks message manager */
    INT_SC_SND_HEL, /* >------------------------- */
    INT_SC_SND_ACK, // id = secure channel connection index,
                    // params = (SOPC_Buffer*) buffer positioned to message payload
                    /* -------------------------< */
    INT_SC_SND_ERR,
    INT_SC_SND_RHE,
    INT_SC_SND_OPN,        /* >------------------------- */
                           // id = secure channel connection index,
                           // params = (SOPC_Buffer*) buffer positioned to message payload,
    INT_SC_SND_CLO,        // auxParam = (uint32_t) request Id context if response / request Handle if request when MSG
    INT_SC_SND_MSG_CHUNKS, /* -------------------------< */

    /* SC connection manager -> SC connection manager */
    INT_SC_CLOSE // id = secure channel connection index,
                 // params = (char*) reason,
                 // auxParam = (SOPC_StatusCode) errorStatus
} SOPC_SecureChannels_InternalEvent;

typedef enum
{
    TIMER_SC_CONNECTION_TIMEOUT = 0x500, /* id = secure channel connection index */
    TIMER_SC_SERVER_REVERSE_CONN_RETRY,  /* id = endpoint configuration index
                                          * params = reverse connection index in endpoint
                                          */
    TIMER_SC_CLIENT_OPN_RENEW,           /* id = secure channel connection index */
    TIMER_SC_REQUEST_TIMEOUT,            /* id = secure channel connection index
                                            params = (uint32_t) requestHandle // Debug purpose only
                                            auxParam = (uint32_t) requestId */
} SOPC_SecureChannels_TimerEvent;

// Secure channel internal event enqueue function
void SOPC_SecureChannels_EnqueueInternalEvent(SOPC_SecureChannels_InternalEvent event,
                                              uint32_t id,
                                              uintptr_t params,
                                              uintptr_t auxParam);

/* Secure channel internal event enqueue function: event will be enqueued as next to be treated (only for close SC
 * situation) Note: it is important to close the SC as soon as possible in order to avoid any treatment of new messages
 * on a SC to be closed. */
void SOPC_SecureChannels_EnqueueInternalEventAsNext(SOPC_SecureChannels_InternalEvent event,
                                                    uint32_t id,
                                                    uintptr_t params,
                                                    uintptr_t auxParam);

#endif /* SOPC_SECURE_CHANNELS_API_INTERNAL_H_ */
