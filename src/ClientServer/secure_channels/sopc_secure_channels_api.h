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
    EP_OPEN = 0x200,     /**< Server side only:<br/>
                            Opens a new endpoint connection listener.<br/>
                            id = endpoint description configuration index */
    EP_CLOSE,            /**< Server side only:<br/>
                            Closes an opened endpoint connection listener.<br/>
                            id = endpoint description configuration index */
    REVERSE_EP_OPEN,     /**< Client side only:<br/>
                            Open a reverse endpoint<br/>
                            id = reverse endpoint configuration index */
    REVERSE_EP_CLOSE,    /**< Client side only:<br/> close a reverse endpoint<br/>
                            id = reverse endpoint configuration index */
    SC_CONNECT,          /**< Client side only:<br/>
                            Establishes connection of a new secure channel.<br/>
                            id = secure channel configuration index */
    SC_REVERSE_CONNECT,  /**< Client side only:<br/>
                            Activate the connection on next available server reverse connection<br/>
                            id = reverse endpoint configuration index<br/>
                            params = (uint32_t) secure channel configuration index */
    SC_DISCONNECT,       /**< Disconnects and clears the secure channel identified by the given secure channel index
                              (provided on connected event).<br/>
                              id = secure channel connection index
                              params = (SOPC_StatusCode) reasonCode */
    SC_SERVICE_SND_MSG,  /**< Sends a message on a secure channel.<br/>
                           id = secure channel connection index<br/>
                           params = (SOPC_Buffer*) buffer to send containing empty space for TCP UA header (24 bytes)
                           followed by encoded OpcUa message<br/>
                           auxParam = (uint32_t) request Id context if response (server) / request Handle context if
                           request (client) */
    SC_SERVICE_SND_ERR,  /**< Sends an error message related to the secure channel so as to close it (if in state
                              Connected or Connected_Renew).<br/>
                              id = secure channel connection index<br/>
                              params = (SOPC_StatusCode) encoding  failure status code or security check failed (user
                                       auth)<br/>
                              auxParam = (uint32_t) request Id context of response  (Debug info only)
                          */
    SC_DISCONNECTED_ACK, /**< Notify secure channel disconnected state was received<br/>
                            id = secure channel connection index<br/>
                            params = (uint32_t) secure channel config index (SERVER SIDE ONLY) or 0 (CLIENT SIDE)
                          */
    SCS_REEVALUATE_SCS,  /**< Re-evaluate the secure channels due to application certificate/key update (force SC
                              re-establishment) or PKI application trust list update (peer certificate re-validation
                              necessary).<br/>
                             params = (bool) flag indicating if it concerns server (true) or client (false)
                             application secure channels.<br/>
                             auxParam = (bool) flag indicating if it concerns application certificate/key update (true),
                             otherwise the PKI trust list update (false).
                          */
} SOPC_SecureChannels_InputEvent;

/**
 *  \brief Secure channel output events to services layer
 */
typedef enum
{
    EP_CONNECTED = 0x300,  /**< (Server only) Notification of new SC connected on SC connection listener.<br/>
                              id = endpoint description config index<br/>
                              params = (uint32_t) secure channel config index<br/>
                              auxParams = (uint32_t) secure channel connection index */
    EP_CLOSED,             /**< Notification of closed endpoint connection listener.<br/>
                              id = endpoint description config index<br/>
                              auxParams = SOPC_ReturnStatus */
    EP_REVERSE_CLOSED,     /**<
                              Notifies the SC reverse listener is closed for the given endpoint configuration index<br/>
                              id = reverse endpoint config index<br/>
                              auxParams = SOPC_ReturnStatus */
    SC_CONNECTED,          /**< (Client only) Notify secure channel in connected state.
                              Treats actions waiting for connection to be established.<br/>
                              id = secure channel connection index<br/>
                              auxParams = (uint32_t) secure channel configuration index */
    SC_REVERSE_CONNECTED,  /**< (Client only) Notify secure channel in connected state for a reverse connection.
                              Treats actions waiting for connection to be established.<br/>
                              id = secure channel connection index<br/>
                              params = (uint32_t) secure channel configuration index<br/>
                              auxParams = (uint32_t) reverse endpoint configuration index
                            */
    SC_CONNECTION_TIMEOUT, /**< Notification that secure channel connection attempt failed for the given SC connection
                              configuration index. Notifies failure for actions waiting for connection to be
                              established.<br/>
                              id = secure channel config index */
    SC_DISCONNECTED,       /**< Notify secure channel previously in connected state is now disconnected.
                              It shall be acknowledged by raising back a SC_DISCONNECTED_ACK event.<br/>
                              id = secure channel connection index<br/>
                              params = (uint32_t) secure channel config index (SERVER SIDE ONLY) or 0 (CLIENT SIDE)<br/>
                              auxParam = SOPC_StatusCode
                            */
    SC_SERVICE_RCV_MSG,    /**< Received message on the secure channel for the given SC connection index and with the
                              provided OPC UA message encoded as bytes buffer. When triggered on Server side, the request
                              Id context is provided in addition to other parameters so it can be returned in the
                              response.<br/>
                              id = secure channel connection index<br/>
                              params = (SOPC_Buffer*) OPC UA message payload buffer<br/>
                              auxParam = (uint32_t) request Id context (server side only, 0 if client)
                            */
    SC_SND_FAILURE,        /**< Notifies a message which was provided through the output event SC_SERVICE_SND_MSG failed
                              to be sent.<br/>
                              id = secure channel connection index<br/>
                              params = (uint32_t) requestHandle for client / requestId for server (unused)<br/>
                              auxParam = SOPC_StatusCode */
    SC_REQUEST_TIMEOUT,    /**< When an OPN or MSG sent message did not receive any answer.<br/>
                              id = secure channel connection index<br/>
                              auxParam = (uint32_t) request handle */
} SOPC_SecureChannels_OutputEvent;

/* Secure channel external event enqueue function
 * IMPORTANT NOTE: internal events use will cause an assertion error */
SOPC_ReturnStatus SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                                   uint32_t id,
                                                   uintptr_t params,
                                                   uintptr_t auxParam);

/** \brief Returns the current pending number of events in Secure Channel queue */
uint32_t SOPC_SecureChannels_Get_QueueSize(void);

void SOPC_SecureChannels_Initialize(SOPC_SetListenerFunc* setSocketsListener);

void SOPC_SecureChannels_SetEventHandler(SOPC_EventHandler* handler);

void SOPC_SecureChannels_Clear(void);

#endif /* SOPC_SECURE_CHANNELS_API_H_ */
