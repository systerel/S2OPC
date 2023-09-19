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
 * \file sopc_secure_channels_api.h
 *
 * \brief Event oriented API of the Secure Channel layer.
 *
 *   This module is in charge of the event dispatcher thread management.
 */

#ifndef SOPC_SECURE_CHANNELS_API_H_
#define SOPC_SECURE_CHANNELS_API_H_

#include <stdint.h>

#include "sopc_event_handler.h"

/**
 *  \brief Secure channel input events from services layer
 */
typedef enum
{
    EP_OPEN = 0x200,     /**< id = endpoint description configuration index */
    EP_CLOSE,            /**< id = endpoint description configuration index */
    REVERSE_EP_OPEN,     /**< Client: open a reverse endpoint<BR>
                            id = reverse endpoint configuration index */
    REVERSE_EP_CLOSE,    /**< Client: close a reverse endpoint<BR>
                            id = reverse endpoint configuration index */
    SC_CONNECT,          /**< id = secure channel configuration index */
    SC_REVERSE_CONNECT,  /**< Client: Activate the connection on next available server reverse connection<BR>
                            id = reverse endpoint configuration index<BR>
                            params = (uint32_t) secure channel configuration index */
    SC_DISCONNECT,       /**< id = secure channel connection index */
    SC_SERVICE_SND_MSG,  /**< id = secure channel connection index<BR>
                           params = (SOPC_Buffer*) buffer to send containing empty space for TCP UA header (24 bytes)
                           followed by encoded OpcUa message<BR>
                           auxParam = (uint32_t) request Id context if response (server) / request Handle context if
                           request (client) */
    SC_SERVICE_SND_ERR,  /**< id = secure channel connection index<BR>
                            params = (SOPC_StatusCode) encoding failure status code or security check failed (user
                            auth)<BR>
                            auxParam = (uint32_t) request Id context of response (Debug info only)
                          */
    SC_DISCONNECTED_ACK, /**< Notify secure channel disconnected state was received<BR>
                            id = secure channel connection index<BR>
                            params = (uint32_t) secure channel config index (SERVER SIDE ONLY) or 0 (CLIENT SIDE)
                          */
    SCS_REEVALUATE_SCS,  /**< Re-evaluate the secure channels due to application certificate/key update (force SC
                              re-establishment) or PKI application trust list update (peer certificate re-validation
                              necessary).<BR>
 
                             params = (bool) flag indicating if it concerns server (true) or client (false)
                             application secure channels.<BR>
                             auxParam = (bool) flag indicating if it concerns application certificate/key update (true),
                             otherwise the PKI trust list update (false).
                          */
} SOPC_SecureChannels_InputEvent;

/**
 *  \brief Secure channel output events to services layer
 */
typedef enum
{
    EP_CONNECTED = 0x300,  /**< id = endpoint description config index<BR>
                              params = (uint32_t) secure channel config index<BR>
                              auxParams = (uint32_t) secure channel connection index */
    EP_CLOSED,             /**< id = endpoint description config index<BR>
                              auxParams = SOPC_ReturnStatus */
    EP_REVERSE_CLOSED,     /**< id = reverse endpoint config index<BR>
                              auxParams = SOPC_ReturnStatus */
    SC_CONNECTED,          /**< Notify secure channel in connected state<BR>
                              id = secure channel connection index<BR>
                              auxParams = (uint32_t) secure channel configuration index */
    SC_REVERSE_CONNECTED,  /**< id = secure channel connection index<BR>
                              params = (uint32_t) secure channel configuration index<BR>
                              auxParams = (uint32_t) reverse endpoint configuration index
                            */
    SC_CONNECTION_TIMEOUT, /**< id = secure channel config index */
    SC_DISCONNECTED,       /**< Notify secure channel previously in connected state is now disconnected
                              It shall be acknowledged by raising back a SC_DISCONNECTED_ACK event.
                              id = secure channel connection index<BR>
                              params = (uint32_t) secure channel config index (SERVER SIDE ONLY) or 0 (CLIENT SIDE)<BR>
                              auxParam = SOPC_StatusCode
                            */
    SC_SERVICE_RCV_MSG,    /**< id = secure channel connection index<BR>
                              params = (SOPC_Buffer*) OPC UA message payload buffer<BR>
                              auxParam = (uint32_t) request Id context (server side only, 0 if client) */
    SC_SND_FAILURE,        /**< id = secure channel connection index<BR>
                              params = (uint32_t) requestHandle for client / requestId for server (unused)<BR>
                              auxParam = SOPC_StatusCode */
    SC_REQUEST_TIMEOUT,    /**< id = secure channel connection index<BR>
                              auxParam = (uint32_t) request handle */
} SOPC_SecureChannels_OutputEvent;

/* Secure channel external event enqueue function
 * IMPORTANT NOTE: internal events use will cause an assertion error */
SOPC_ReturnStatus SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                                   uint32_t id,
                                                   uintptr_t params,
                                                   uintptr_t auxParam);

void SOPC_SecureChannels_Initialize(SOPC_SetListenerFunc* setSocketsListener);

void SOPC_SecureChannels_SetEventHandler(SOPC_EventHandler* handler);

void SOPC_SecureChannels_Clear(void);

#endif /* SOPC_SECURE_CHANNELS_API_H_ */
