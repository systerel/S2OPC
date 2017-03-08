/**
 *  \file sopc_channel.h
 *
 *  \brief High level API to manipulate an OPC-UA Channel and invoke OPC UA services through the channel
 */
/*
 *  Copyright (C) 2016 Systerel and others.
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

#ifndef SOPC_CHANNEL_H_
#define SOPC_CHANNEL_H_

#include "sopc_stack_csts.h"

#ifdef OPCUA_HAVE_CLIENTAPI

#include "sopc_types.h"
#include "pki.h"
#include "key_manager.h"

/**
 *  \brief Channel type definition
 */
typedef void* SOPC_Channel;

/**
 *  \brief Channel serialization enumeration type
 *  Note: only binary serialization is available in INGPOCS
 */
typedef enum {
    SOPC_ChannelSerializer_Invalid = 0x00,
    SOPC_ChannelSerializer_Binary = 0x01
} SOPC_Channel_SerializerType;

/**
 *  \brief Channel connection event enumeration type
 */
typedef enum {
    SOPC_ChannelEvent_Invalid = 0x00,
    SOPC_ChannelEvent_Connected = 0x01, // Note: only in async mode
    SOPC_ChannelEvent_Disconnected = 0x02,
} SOPC_Channel_Event;

/**
 *  \brief Channel connection state changed function callback type
 */
typedef SOPC_StatusCode (SOPC_Channel_PfnConnectionStateChanged) (SOPC_Channel       channel,
                                                                  void*              cbData,
                                                                  SOPC_Channel_Event cEvent,
#ifdef STACK_1_02
                                                                  void*              securityToken,
#endif
                                                                  SOPC_StatusCode    status);

/**
 *  \brief Channel request completed function callback type
 */
typedef SOPC_StatusCode (SOPC_Channel_PfnRequestComplete)(SOPC_Channel         channel,
                                                          void*                response,
                                                          SOPC_EncodeableType* responseType,
                                                          void*                cbData,
                                                          SOPC_StatusCode      status);

/**
 *  \brief Channel asynchronous operation result event enumeration type
 */
typedef enum SOPC_ChannelEvent_AsyncOperationResult {
    SOPC_ChannelAsync_ConnectResult, // Only in case of failure during connect for now
    SOPC_ChannelAsync_InvokeSendRequestResult,
    SOPC_ChannelAsync_DisconnectResult
} SOPC_ChannelEvent_AsyncOperationResult;

/**
 *  \brief Channel asynchronous operation result callback type
 */
typedef void (SOPC_Channel_AsyncResult_CB) (SOPC_Channel                           channel,
                                            void*                                  cbData,
                                            SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                            SOPC_StatusCode                        status);

/**
 *  \brief Create a new channel by initializing the provided channel
 *
 *  \param channel     The channel to initialize
 *  \param serialType  The channel serialization type to send data
 *                     (only ChannelSerializer_Binary is valid)
 *
 *  \return            STATUS_OK if channel is correctly initialized, STATUS_NOK otherwise
 *                     (NULL pointer, invalid serialization value)
 */
SOPC_StatusCode SOPC_Channel_Create(SOPC_Channel*               channel,
                                    SOPC_Channel_SerializerType serialType);

/**
 *  \brief Start the channel connection establishment (TCP connection, TCP UA Hello message)
 *         and the secure channel establishement (TCP UA OpenSecureChannel request) on server acknowledgment
 *         (TCP UA Ack message).
 *         Note: in single threaded mode, it is necessary to call the socket manager loop to receive messages.
 *
 *  \param channel            The channel to connect
 *  \param url                Endpoint address for establishing the connection
 *  \param crt_cli            Client certificate to use for establishing the connection (or NULL for None security mode)
 *  \param key_priv           Client private key to use for establishing the connection (or NULL for None security mode)
 *  \param crt_srv            Server certificate of the endpoint server to connect (or NULL for None security mode)
 *  \param pki                The Public Key Infrastructure to use for validating certificates (or NULL for None security mode)
 *  \param reqSecuPolicyUri   URI of the requested security policy
 *  \param requestedLifetime  Lifetime requested for the channel connection
 *  \param msgSecurityMode    Security mode to use for the messages exchanged through the connection
 *  \param networkTimeout     Network timeout to establish the connection (in milliseconds)
 *  \param cb                 Connection state changed callback function to be called
 *  \param cbData             Data to be provided to the connection state changed callback function on call
 *
 *  \return                   STATUS_OK if channel connection start step succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Channel_BeginConnect(SOPC_Channel                            channel,
                                          const char*                             url,
                                          const Certificate*                      crt_cli,
                                          const AsymmetricKey*                    key_priv,
                                          const Certificate*                      crt_srv,
                                          const PKIProvider*                      pki,
                                          const char*                             reqSecuPolicyUri,
                                          int32_t                                 requestedLifetime,
                                          OpcUa_MessageSecurityMode               msgSecurityMode,
                                          uint32_t                                networkTimeout,
                                          SOPC_Channel_PfnConnectionStateChanged* cb,
                                          void*                                   cbData);

/**
 *  \brief Register action to start the channel connection establishment (TCP connection, TCP UA Hello message)
 *         and the secure channel establishement (TCP UA OpenSecureChannel request) on server acknowledgment
 *         (TCP UA Ack message).
 *
 *  \param channel            The channel to connect
 *  \param url                Endpoint address for establishing the connection
 *  \param crt_cli            Client certificate to use for establishing the connection (or NULL for None security mode)
 *  \param key_priv           Client private key to use for establishing the connection (or NULL for None security mode)
 *  \param crt_srv            Server certificate of the endpoint server to connect (or NULL for None security mode)
 *  \param pki                The Public Key Infrastructure to use for validating certificates (or NULL for None security mode)
 *  \param reqSecuPolicyUri   URI of the requested security policy
 *  \param requestedLifetime  Lifetime requested for the channel connection
 *  \param msgSecurityMode    Security mode to use for the messages exchanged through the connection
 *  \param networkTimeout     Network timeout to establish the connection (in milliseconds)
 *  \param eventCb            Connection state changed callback function to be called (connection established, disconnection)
 *  \param eventCbData        Data to be provided to the connection state changed callback function on call
 *  \param connectCb          Asynchronous connect result callback function called on connection failure or success
 *  \param connectCbData      Asynchronous connect result callback data provided to callback function on call
 *
 *  \return                   STATUS_OK if channel connection start step succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Channel_AsyncConnect(SOPC_Channel                            channel,
                                          const char*                             url,
                                          const Certificate*                      crt_cli,
                                          const AsymmetricKey*                    key_priv,
                                          const Certificate*                      crt_srv,
                                          const PKIProvider*                      pki,
                                          const char*                             reqSecuPolicyUri,
                                          int32_t                                 requestedLifetime,
                                          OpcUa_MessageSecurityMode               msgSecurityMode,
                                          uint32_t                                networkTimeout,
                                          SOPC_Channel_PfnConnectionStateChanged* eventCb,
                                          void*                                   eventCbData,
                                          SOPC_Channel_AsyncResult_CB*            connectCb,
                                          void*                                   connectCbData);

/**
 *  \brief Start the channel connection establishment (TCP connection, TCP UA Hello message)
 *         and the secure channel establishement (TCP UA OpenSecureChannel request) on server acknowledgment
 *         (TCP UA Ack message). Then waits for connection success or failure before returning.
 *
 *  \param channel            The channel to connect
 *  \param url                Endpoint address for establishing the connection
 *  \param crt_cli            Client certificate to use for establishing the connection (or NULL for None security mode)
 *  \param key_priv_cli       Client private key to use for establishing the connection (or NULL for None security mode)
 *  \param crt_srv            Server certificate of the endpoint server to connect (or NULL for None security mode)
 *  \param pki                The Public Key Infrastructure to use for validating certificates (or NULL for None security mode)
 *  \param reqSecuPolicyUri   URI of the requested security policy
 *  \param requestedLifetime  Lifetime requested for the channel connection
 *  \param msgSecurityMode    Security mode to use for the messages exchanged through the connection
 *  \param networkTimeout     Network timeout to establish the connection (in milliseconds)
 *  \param cb                 Connection state changed callback function to be called
 *  \param cbData             Data to be provided to the connection state changed callback function on call
 *
 *  \return                   STATUS_OK if channel connection succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Channel_Connect(SOPC_Channel                            channel,
                                     const char*                             url,
                                     const Certificate*                      crt_cli,
                                     const AsymmetricKey*                    key_priv_cli,
                                     const Certificate*                      crt_srv,
                                     const PKIProvider*                      pki,
                                     const char*                             reqSecuPolicyUri,
                                     int32_t                                 requestedLifetime,
                                     OpcUa_MessageSecurityMode               msgSecurityMode,
                                     uint32_t                                networkTimeout,
                                     SOPC_Channel_PfnConnectionStateChanged* cb,
                                     void*                                   cbData);

/**
 *  \brief Send the given service request message and set the callback to call on service response reception.
 *         Note: in single threaded mode, it is necessary to call the socket manager loop to receive messages.
 *
 *  \param channel      The channel to connect
 *  \param debugName    Name of the invoked service (debug information only)
 *  \param request      Service request message instance of the given request type
 *  \param requestType  Service request type of the service request to invoke
 *  \param responseType Service response type expected for the given request (optional: efficiency improved when provided)
 *  \param cb           Service request completed function callback to call on response reception
 *  \param cbData       Data to provide to the given callback on response reception
 *
 *  \return             STATUS_OK if invocation (request sending) of the service succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Channel_BeginInvokeService(SOPC_Channel                     channel,
                                                char*                            debugName,
                                                void*                            request,
                                                SOPC_EncodeableType*             requestType,
                                                SOPC_EncodeableType*             responseType,
                                                SOPC_Channel_PfnRequestComplete* cb,
                                                void*                            cbData);

/**
 *  \brief Send the given service request message and return after setting the received service response.
 *
 *  \param channel         The channel to connect
 *  \param debugName       Name of the invoked service (debug information only)
 *  \param request         Service request message instance of the given request type
 *  \param requestType     Service request type of the service request to invoke
 *  \param expResponseType Service response type expected for the given request (optional: efficiency improved when provided)
 *  \param response        Service response message of the given type received for the service invoked
 *  \param responseType    Service response type received for the service invoked
 *
 *  \return                STATUS_OK if invocation of the service succeeded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Channel_InvokeService(SOPC_Channel          channel,
                                           char*                 debugName,
                                           void*                 request,
                                           SOPC_EncodeableType*  requestType,
                                           SOPC_EncodeableType*  expResponseType,
                                           void**                response,
                                           SOPC_EncodeableType** responseType);

/**
 *  \brief Start action of disconnecting the given channel connection
 *
 *  \param channel           The channel to disconnect
 *  \param disconnectCb      Asynchronous disconnect result callback function called on connection failure or success
 *  \param disconnectCbData  Asynchronous disconnect result callback data provided to callback function on call
 *
 *  \return            STATUS_OK if channel disconnection action is recorded, STATUS_NOK otherwise
 *
 */
SOPC_StatusCode SOPC_Channel_AsyncDisconnect(SOPC_Channel                 channel,
                                             SOPC_Channel_AsyncResult_CB* disconnect,
                                             void*                        disconnectData);

/**
 *  \brief Disconnect the given channel connection
 *
 *  \param channel     The channel to disconnect
 *  \return            STATUS_OK if channel is correctly disconnected, STATUS_NOK otherwise (NULL pointer)
 *
 */

SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel);

/**
*  \brief Start action of deleting the given channel connection
*
*  \param channel     The channel to delete. Note: channel must be considered freed after return (it will be done asynchronously).
*  \return            STATUS_OK if channel delete action is recorded, STATUS_NOK otherwise
*
*/
SOPC_StatusCode SOPC_Channel_AsyncDelete(SOPC_Channel channel);

/**
 *  \brief Disconnect and deallocate the channel
 *
 *  \param channel     The channel to disconnect and deallocate
 *  \return            STATUS_OK if channel is correctly deleted, STATUS_NOK otherwise (NULL pointer)
 *
 */
SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel);

#endif /* CLIENT_API */

#endif /* SOPC_CHANNEL_H_ */
