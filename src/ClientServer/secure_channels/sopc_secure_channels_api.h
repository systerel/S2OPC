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
 *  \brief Secure channel input events
 *  */
typedef enum
{
    EP_OPEN = 0x200,    /* id = endpoint description configuration index */
    EP_CLOSE,           /* id = endpoint description configuration index */
    SC_CONNECT,         /* id = secure channel configuration index */
    SC_DISCONNECT,      /* id = secure channel connection index */
    SC_SERVICE_SND_MSG, /* id = secure channel connection index,
                          params = (SOPC_Buffer*) buffer to send containing empty space for TCP UA header (24 bytes)
                          followed by encoded OpcUa message,
                          auxParam = (uint32_t) request Id context if response (server) / request Handle context if
                          request (client) */
    SC_SERVICE_SND_ERR, /* id = secure channel connection index,
                           params = (SOPC_StatusCode) encoding failure status code
                           auxParam = (uint32_t) request Id context of response (Debug info only)
                         */
} SOPC_SecureChannels_InputEvent;

typedef enum
{
    EP_CONNECTED = 0x300,  /* id = endpoint description config index,
                              params = (uint32_t) endpoint connection config index,
                              auxParams = (uint32_t) secure channel connection index */
    EP_CLOSED,             /* id = endpoint description config index,
                                 auxParams = SOPC_ReturnStatus */
    SC_CONNECTED,          /* id = secure channel connection index,
                              auxParams = (uint32_t) secure channel configuration index */
    SC_CONNECTION_TIMEOUT, /* id = endpoint connection config index */
    SC_DISCONNECTED,       /* id = secure channel connection index
                              auxParam = SOPC_StatusCode
                            */
    SC_SERVICE_RCV_MSG,    /* id = secure channel connection index,
                              params = (SOPC_Buffer*) OPC UA message payload buffer,
                              auxParam = (uint32_t) request Id context (server side only, 0 if client) */
    SC_SND_FAILURE,        /* id = secure channel connection index,
                              params = (uint32_t) requestHandle for client / requestId for server (unused)
                              auxParam = SOPC_StatusCode */
    SC_REQUEST_TIMEOUT,    /* id = secure channel connection index,
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
