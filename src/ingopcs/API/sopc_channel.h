/*
 *  \file sopc_channel.h
 *
 *  \brief High level API to manipulate an OPC-UA Channel and invoke OPC UA services through the channel
 *
 *  Created on: Sep 29, 2016
 *      Author: VMO (Systerel)
 */

#ifndef INGOPCS_SOPC_CHANNEL_H_
#define INGOPCS_SOPC_CHANNEL_H_

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
    ChannelSerializer_Invalid = 0x00,
    ChannelSerializer_Binary = 0x01
} SOPC_Channel_SerializerType;

/**
 *  \brief Channel connection event enumeration type
 */
typedef enum {
    ChannelEvent_Invalid = 0x00,
    ChannelEvent_Connected = 0x01,
    ChannelEvent_Disconnected = 0x02
} SOPC_Channel_Event;

/**
 *  \brief Channel connection state changed function callback type
 */
typedef SOPC_StatusCode (SOPC_Channel_PfnConnectionStateChanged) (SOPC_Channel       channel,
                                                                  void*              cbData,
                                                                  SOPC_Channel_Event cEvent,
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
 *  \brief Create a new channel by initializing the provided channel
 *
 *  \param channel     The channel to disconnect and deallocate
 *  \return            STATUS_OK if channel is correctly deleted, STATUS_NOK otherwise (NULL pointer)
 *
 */
SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel);

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
 *  \return                   STATUS_OK if channel connection step succeeded, STATUS_NOK otherwise
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
 *  \brief Disconnect the given channel connection
 *
 *  \param channel     The channel to disconnect
 *  \return            STATUS_OK if channel is correctly disconnected, STATUS_NOK otherwise (NULL pointer)
 *
 */
SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel);

#endif /* CLIENT_API */

#endif /* INGOPCS_SOPC_CHANNEL_H_ */
