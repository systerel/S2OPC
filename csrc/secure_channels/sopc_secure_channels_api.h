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

#include "sopc_event_handler.h"

/**
 *  \brief Secure channel input events
 *  */
typedef enum
{
    EP_OPEN,            /* id = endpoint description configuration index */
    EP_CLOSE,           /* id = endpoint description configuration index */
    SC_CONNECT,         /* id = secure channel connection configuration index */
    SC_DISCONNECT,      /* id = secure channel connection index */
    SC_SERVICE_SND_MSG, /* id = secure channel connection index,
                          params = (SOPC_Buffer*) buffer to send containing empty space for TCP UA header (24 bytes)
                          followed by encoded OpcUa message,
                          auxParam = (uint32_t) request Id context if response (server) / request Handle context if
                          request (client) */
} SOPC_SecureChannels_InputEvent;

/* Secure channel external event enqueue function
 * IMPORTANT NOTE: internal events use will cause an assertion error */
void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t id,
                                      void* params,
                                      uintptr_t auxParam);

void SOPC_SecureChannels_Initialize(SOPC_SetListenerFunc setSocketsListener);

void SOPC_SecureChannels_Clear(void);

#endif /* SOPC_SECURE_CHANNELS_API_H_ */
