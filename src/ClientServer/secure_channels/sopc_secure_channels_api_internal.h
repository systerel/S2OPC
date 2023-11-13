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
    INT_EP_SC_CREATE = 0x400,  /**<
                                  Creates a new SC connection to be associated with the given secure channel listener
                                  configuration index and socket index.<br/>
                                  id = endpoint description configuration index<br/>
                                  auxParam = socket index */
    INT_EP_SC_CLOSE,           /**<
                                  Closes the SC connection with the given connection index associated with this SC listener.<br/>
                                  id = secure channel connection index<br/>
                                  (auxParam = (reverse) endpoint configuration index) */
    INT_EP_SC_REVERSE_CONNECT, /**<
                                  Server side only:
                                  Start reverse connection to a client reverse endpoint.<br/>
                                  This event is triggered on ::EP_OPEN event treatment for each reverse connection
                                  configured, <br/>
                                  when a connection is established (client shall be able to connect again) or <br/>
                                  when a connection failed on retry timeout with event
                                  ::TIMER_SC_SERVER_REVERSE_CONN_RETRY.<br/>
                                  id = endpoint description configuration index<br/>
                                  auxParam = (uint16_t) client to connect configuration index in endpoint config */
    INT_SC_RCV_RHE_TRANSITION, /**<
                                  Client side only:<br/>
                                  RHE associated to a waiting SC connection => transition validated for SC<br/>
                                  id = secure channel connection index
                                */

    /* SC connection manager -> SC listener manager */
    INT_EP_SC_CREATED,             /**<
                                       The new SC connection, requested by INT_EP_SC_CREATE, has been created with
                                       given index and is associated with the given SC listener configuration index.
                                       Registers the connection as associated to the listener.<br/>
                                       id = endpoint description configuration index<br/>
                                       auxParam = (uint32_t) secure channel connection index */
    INT_EP_SC_RHE_DECODED,         /**<
                                          Notify secure listener that connection is accepted.<br/>id = secure channel
                                          connection index.<br/>
                                          param = (char*) serverURI<br/>
                                          auxParam = (char*) serverEndpointURL */
    INT_EP_SC_DISCONNECTED,        /**<
                                          Notify listener that connection has been closed.<br/>
                                          id = (reverse) endpoint description configuration index<br/>
                                          auxParam = (uint32_t) secure channel connection index */
    INT_REVERSE_EP_REQ_CONNECTION, /**<
                                      Client side only:<br/>
                                      Initialization of Reverse connection (client-side), associates the secure
                                      channel connection to the secure listener.<br/>
                                      id = reverse endpoint configuration index<br/>
                                      auxParam = (uint32_t) secure channel connection index
                                    */

    /* OPC UA chunks message manager -> SC connection manager */
    INT_SC_RCV_HEL,             /**<
                                  Hello message has been received on the secure channel with given connection index<br/>
                                   id = secure channel connection index<br/>*/
    INT_SC_RCV_ACK,             /**<
                                  Acknowledge message has been received on the secure channel with given connection
                                  index and byte buffer containing message payload.<br/>
                                   id = secure channel connection index<br/>
                                   params = (SOPC_Buffer*) buffer positioned to message payload */
    INT_SC_RCV_ERR,             /**<
                                  Error message has been received on the secure channel with given connection index<br/>
                                   id = secure channel connection index<br/>
                                   params = (SOPC_Buffer*) buffer positioned to message payload */
    INT_SC_RCV_OPN,             /**<
                                  An OPC UA OpenSecureChannel message has been received on the secure channel with
                                  given connection index and byte buffer containing unencrypted message payload.<br/>
                                   id = secure channel connection index<br/>
                                   params = (SOPC_Buffer*) buffer positioned to message payload<br/>
                                   auxParam = (uint32_t) requestId context if request (server side
                                   / requestHandle if response (client side) */
    INT_SC_RCV_CLO,             /**< Same as ::INT_SC_RCV_OPN for CloseSecureChannel*/
    INT_SC_RCV_RHE,             /**<
                                  ReverseHello message was received on the secure channel with given connection
                                  index and byte buffer containing unencrypted message payload.<br/>
                                   id = secure channel connection index<br/>
                                   params = (SOPC_Buffer*) buffer positioned to message payload<br/>
                                   INT_SC_RCV_RHE_TRANSITION is the final step in listener => connection manager */
    INT_SC_RCV_MSG_CHUNKS,      /**<
                                    An OPC UA message has been received in one or several chunks on the secure
                                    channel with given connection index and byte buffer containing unencrypted
                                    message payload.<br/>
                                    Same parameters as ::INT_SC_RCV_OPN */
    INT_SC_RCV_MSG_CHUNK_ABORT, /**<
                                    Notifies OPC UA message of type ABORT received.<br/>
                                    Same parameters as ::INT_SC_RCV_OPN */
    INT_SC_RCV_FAILURE,         /**<
                                   Reception of a message on the secure channel with given connection index failed.
                                   Closes the secure channel connection.<br/>
                                   id = secure channel connection index<br/>
                                   auxParam = error status */
    INT_SC_SND_FATAL_FAILURE,   /**<
                                   Notifies encoding into chunks of an OPC UA message, provided as bytes buffer,
                                   failed during multi-chunk generation, signature or encryption.<br/>
                                   id = secure channel connection index<br/>
                                   params = requestId (server) / requestHandle (client)<br/>
                                   auxParam = (SOPC_StatusCode) error status in case of client */
    INT_SC_SENT_ABORT_FAILURE,  /**<
                                   Notifies encoding into chunks of an OPC UA ABORT message failed during multi-chunk
                                    generation, signature, encryption on transmission.<br/>
                                    Same parameters as ::INT_SC_SND_FATAL_FAILURE */
    /* SC connection manager -> OPC UA chunks message manager */
    INT_SC_SND_HEL,        /**<
                              Provides an OPC UA TCP "Hello" message payload to send for the given connection index.<br/>
                              id = secure channel connection index<br/>
                              params = (SOPC_Buffer*) buffer positioned to message payload */
    INT_SC_SND_ACK,        /**<
                              Same as ::INT_SC_SND_HEL for "Hello" message*/
    INT_SC_SND_ERR,        /**<
                              Same as ::INT_SC_SND_HEL for "Error" message*/
    INT_SC_SND_RHE,        /**<
                              Same as ::INT_SC_SND_HEL for "Reverse Hello" message*/
    INT_SC_SND_OPN,        /**<
                              Provides OPC UA OpenSecureChannel message payload to send for the given connection
                              index.<br/>
                              id = secure channel connection index<br/>
                              params = (SOPC_Buffer*) buffer positioned to message payload<br/>
                              auxParam = (uint32_t) request Id context if response / request Handle if request when MSG */
    INT_SC_SND_CLO,        /**<
                              Provides OPC UA CloseSecureChannel message payload to send for the given connection
                              index.<br/>
                              id = secure channel connection index<br/>
                              params = (SOPC_Buffer*) buffer positioned to message payload */
    INT_SC_SND_MSG_CHUNKS, /**<
                              Provides OPC UA unencrypted message to send on the secure channel with given connection
                              index.<br/>
                              Same parameters as ::INT_SC_SND_OPN */

    /* SC connection manager -> SC connection manager */
    INT_SC_CLOSE /**<
                    Notifies to close the given connection index by modifying state to closed state.
                    It is used to delay the effective closure just after sending an error message
                    (INT_SC_SND_ERR).<br/>
                    id = secure channel connection index<br/>
                    params = (char*) reason<br/>
                    auxParam = (SOPC_StatusCode) errorStatus */
} SOPC_SecureChannels_InternalEvent;

typedef enum
{
    /* SC connection manager */
    TIMER_SC_CONNECTION_TIMEOUT = 0x500, /**<
                                              Clears the secure channel data (if it is still not connected) and
                                              notifies it.<br/>
                                              id = secure channel connection index */
    TIMER_SC_SERVER_REVERSE_CONN_RETRY,  /**< This event is triggered on connection retry timeout
                                              after a reverse connection failure.<br/>
                                              A new reverse connection event ::INT_EP_SC_REVERSE_CONNECT will be
                                              triggered on event treatment.<br/>
                                              id = endpoint configuration index<br/>
                                              params = reverse connection index in endpoint
                                          */
    TIMER_SC_CLIENT_OPN_RENEW,           /**<
                                              Renews the secure channel security token.<br/>
                                              id = secure channel connection index */
    TIMER_SC_REQUEST_TIMEOUT,            /**<
                                            Clears the message request data and notifies it sending
                                            SC_REQUEST_TIMEOUT event to Services.<br/>
                                            id = secure channel connection index<br/>
                                            params = (uint32_t) requestHandle (Debug purpose only)<br/>
                                            auxParam = (uint32_t) requestId */
    /* SC listener manager */
    TIMER_SC_RHE_RECEPTION_TIMEOUT, /**<
                                       Timeout for reception of RHE message after socket connection from server<br/>
                                       id = secure channel connection index */
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
