/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _OpcUa_Channel_H_
#define _OpcUa_Channel_H_ 1

#include "sopc_channel.h"

#ifdef OPCUA_HAVE_CLIENTAPI

#include "sopc_types_wrapper.h"

OPCUA_BEGIN_EXTERN_C

/**
 * @brief The Channel Handle.
 */
typedef SOPC_Channel OpcUa_Channel;

/**
 * @brief Types of events that can occur at an channel and get reported to the application.
 */
#define eOpcUa_Channel_Event_Invalid SOPC_ChannelEvent_Invalid
#define eOpcUa_Channel_Event_Connected SOPC_ChannelEvent_Connected
#define eOpcUa_Channel_Event_Disconnected SOPC_ChannelEvent_Disconnected
typedef SOPC_Channel_Event OpcUa_Channel_Event;
//typedef enum eOpcUa_Channel_Event
//{
//    /** @brief Reserved/Invalid/Ignore */
//    eOpcUa_Channel_Event_Invalid = ChannelEvent_Invalid,
//    /** @brief A secure channel has been connected. (ignore for sync api) */
//    eOpcUa_Channel_Event_Connected = ChannelEvent_Connected,
//    /** @brief A secure channel has been disconnected. */
//    eOpcUa_Channel_Event_Disconnected  = ChannelEvent_Disconnected
//} OpcUa_Channel_Event;

/**
 * @brief Types of serializers supported by the enpoint.
 *
 * @see OpcUa_Channel_Create
 */
#define OpcUa_Channel_SerializerType_Invalid SOPC_ChannelSerializer_Invalid
#define OpcUa_Channel_SerializerType_Binary SOPC_ChannelSerializer_Binary
#define OpcUa_Channel_SerializerType_Xml SOPC_ChannelSerializer_Invalid
typedef SOPC_Channel_SerializerType OpcUa_Channel_SerializerType;
//enum _OpcUa_Channel_SerializerType
//{
//    OpcUa_Channel_SerializerType_Invalid = ChannelSerializer_Invalid,
//    OpcUa_Channel_SerializerType_Binary = ChannelSerializer_Binary,
//    OpcUa_Channel_SerializerType_Xml
//};
//typedef enum _OpcUa_Channel_SerializerType OpcUa_Channel_SerializerType;

/**
 * @brief Called by the stack to report an asynchronous connection event.
 *
 * @param pChannel         [in] The channel used to send the request.
 * @param pCallbackData    [in] The callback data specifed when the request was sent.
 * @param eEvent           [in] The event that occured.
 * @param uStatus          [in] The status code, with which the operation completed.
 */
typedef SOPC_Channel_PfnConnectionStateChanged OpcUa_Channel_PfnConnectionStateChanged;
//typedef OpcUa_StatusCode (OpcUa_Channel_PfnConnectionStateChanged)(
//    OpcUa_Channel       hChannel,
//    OpcUa_Void*         pCallbackData,
//    OpcUa_Channel_Event eEvent,
//    OpcUa_StatusCode    uStatus);

/**
 * @brief Creates a new channel.
 *
 * @param ppChannel    [out] The new channel.
 * @param eEncoderType [in]  The type of encoder to use for messages.
 */
OpcUa_StatusCode OpcUa_Channel_Create(
    OpcUa_Channel*               phChannel,
    OpcUa_Channel_SerializerType eSerializerType);

/**
 * @brief Deletes a channel.
 *
 * @param ppChannel [in/out] The channel to delete.
 */
OpcUa_Void OpcUa_Channel_Delete(
    OpcUa_Channel* phChannel);

/**
 * @brief Establishes a network connection with the server but does not create the session.
 *
 * @param pChannel                      [in] The channel to connect.
 * @param sUrl                          [in] The url of the server to connect to.
 * @param pfCallback                    [in] Function to call for channel event notification.
 * @param pvCallbackData                [in] Gets passed back to the application in pfCallback.
 * @param pClientCertificate            [in] The clients certificate.
 * @param pClientPrivateKey             [in] The clients private key matching the public key in the certificate.
 * @param pServerCertificate            [in] The certificate of the server.
 * @param pPKIConfig                    [in] Implementation dependend configuration for the PKI.
 * @param pRequestedSecurityPolicyUri   [in] URI defining the security parameter set applied to the connection.
 * @param nRequestedLifetime            [in] The requested lifetime for the security token.
 * @param messageSecurityMode           [in] The message security mode requested for the communication.
 * @param nNetworkTimeout               [in] The network timeout. Also used for disconnect.
 */
OpcUa_StatusCode OpcUa_Channel_Connect(
    OpcUa_Channel                               hChannel,
    OpcUa_StringA                               sUrl,
    OpcUa_Channel_PfnConnectionStateChanged*    pfCallback,
    OpcUa_Void*                                 pvCallbackData,
    OpcUa_ByteString*                           pClientCertificate,
    OpcUa_ByteString*                           pClientPrivateKey,
    OpcUa_ByteString*                           pServerCertificate,
    OpcUa_Void*                                 pPKIConfig,
    OpcUa_String*                               pRequestedSecurityPolicyUri,
    OpcUa_Int32                                 nRequestedLifetime,
    OpcUa_MessageSecurityMode                   messageSecurityMode,
    OpcUa_UInt32                                nNetworkTimeout);

/**
 * @brief Establishes a network connection with the server including the secure conversation but does not create the session.
 *
 * @param pChannel                      [in] The channel to connect.
 * @param sUrl                          [in] The url of the server to connect to.
 * @param pClientCertificate            [in] The certificate of the client.
 * @param pClientPrivateKey             [in] The private key for the certificate.
 * @param pServerCertificate            [in] The certificate of the server.
 * @param pPKIConfig                    [in] The platform dependend pki configuration.
 * @param pRequestedSecurityPolicyUri   [in] The URI of the OPC UA security policy to use for this connection.
 * @param nRequestedLifetime            [in] The Lifetime of the connection.
 * @param messageSecurityMode           [in] The constant for None, Sign or SignAndEncrypt mode.
 * @param nNetworkTimeout               [in] The network timeout. Also used for disconnect.
 * @param pfCallback                    [in] Function to call when connection is established.
 * @param pCallbackData                 [in] Data to pass to pfCallback.
 */
OpcUa_StatusCode OpcUa_Channel_BeginConnect(
    OpcUa_Channel                               pChannel,
    OpcUa_StringA                               sUrl,
    OpcUa_ByteString*                           pClientCertificate,
    OpcUa_ByteString*                           pClientPrivateKey,
    OpcUa_ByteString*                           pServerCertificate,
    OpcUa_Void*                                 pPKIConfig,
    OpcUa_String*                               pRequestedSecurityPolicyUri,
    OpcUa_Int32                                 nRequestedLifetime,
    OpcUa_MessageSecurityMode                   messageSecurityMode,
    OpcUa_UInt32                                nNetworkTimeout,
    OpcUa_Channel_PfnConnectionStateChanged*    pfCallback,
    OpcUa_Void*                                 pCallbackData);

/**
 * @brief Closes the network connection with the server.
 *
 * @param pChannel [in] The session to disconnect.
 */
OpcUa_StatusCode OpcUa_Channel_Disconnect(OpcUa_Channel pChannel);

/**
 * @brief Closes the network connection with the server asnchronously.
 *
 * @param pChannel      [in] The session to disconnect.
 * @param pfCallback    [in] Function to call when connection is closed.
 * @param pCallbackData [in] Data to pass to pfCallback.
 */
OpcUa_StatusCode OpcUa_Channel_BeginDisconnect(
    OpcUa_Channel                               pChannel,
    OpcUa_Channel_PfnConnectionStateChanged*    pfCallback,
    OpcUa_Void*                                 pCallbackData);

/**
 * @brief Called by the stack to report an asynchronous request that completed.
 *
 * @param pChannel         [in] The session used to send the request.
 * @param hAsyncState      [in] The async call state object.
 * @param pCallbackData    [in] The callback data specifed when the request was sent.
 */
typedef SOPC_Channel_PfnRequestComplete OpcUa_Channel_PfnRequestComplete;
//typedef OpcUa_StatusCode (OpcUa_Channel_PfnRequestComplete)(
//    OpcUa_Channel         hChannel,
//    OpcUa_Void*           pResponse,
//    OpcUa_EncodeableType* pResponseType,
//    OpcUa_Void*           pCallbackData,
//    OpcUa_StatusCode      uStatus);

/**
 * @brief Invokes a service.
 *
 * @param pChannel       [in]  The session to use.
 * @param sName          [in]  The name of the service being invoked.
 * @param pRequest       [in]  The request body.
 * @param pRequestType   [in]  The type of request.
 * @param ppResponse     [out] The response body.
 * @param ppResponseType [out] The type of response.
 */
OpcUa_StatusCode OpcUa_Channel_InvokeService(
    OpcUa_Channel          hChannel,
    OpcUa_StringA          sName,
    OpcUa_Void*            pRequest,
    OpcUa_EncodeableType*  pRequestType,
    OpcUa_Void**           ppResponse,
    OpcUa_EncodeableType** ppResponseType);

/**
 * @brief Invokes a service asynchronously.
 *
 * @param pChannel       [in]  The session to use.
 * @param sName          [in]  The name of the service being invoked.
 * @param pRequest       [in]  The request body.
 * @param pRequestType   [in]  The type of request.
 * @param pCallback      [in]  The callback to use when the response arrives.
 * @param pCallbackData  [in]  The data to pass to the callback.
 */
OpcUa_StatusCode OpcUa_Channel_BeginInvokeService(
    OpcUa_Channel                     hChannel,
    OpcUa_StringA                     sName,
    OpcUa_Void*                       pRequest,
    OpcUa_EncodeableType*             a_pRequestType,
    OpcUa_Channel_PfnRequestComplete* pCallback,
    OpcUa_Void*                       pCallbackData);

OPCUA_END_EXTERN_C

#endif /* OPCUA_HAVE_CLIENTAPI */
#endif /* _OpcUa_Channel_H_ */
