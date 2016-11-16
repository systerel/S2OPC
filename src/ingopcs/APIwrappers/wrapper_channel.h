/*
 * wrapper_channel.h
 *
 *  Created on: Nov 17, 2016
 *      Author: vincent
 */

#ifndef SOPC_WRAPPER_CHANNEL_H_
#define SOPC_WRAPPER_CHANNEL_H_

#include "sopc_channel.h"

BEGIN_EXTERN_C

SOPC_StatusCode OpcUa_Channel_Create(SOPC_Channel*               channel,
                                     SOPC_Channel_SerializerType serializerType);

void OpcUa_Channel_Clear(SOPC_Channel channel);

void OpcUa_Channel_Delete(SOPC_Channel* channel);

SOPC_StatusCode OpcUa_Channel_BeginInvokeService(SOPC_Channel                     channel,
                                                 char*                            name,
                                                 void*                            request,
                                                 SOPC_EncodeableType*             requestType,
                                                 SOPC_Channel_PfnRequestComplete* callback,
                                                 void*                            callbackData);

SOPC_StatusCode OpcUa_Channel_InvokeService(SOPC_Channel          channel,
                                            char*                 name,
                                            void*                 request,
                                            SOPC_EncodeableType*  requestType,
                                            void**                response,
                                            SOPC_EncodeableType** responseType);

SOPC_StatusCode OpcUa_Channel_BeginDisconnect(SOPC_Channel                            channel,
                                              SOPC_Channel_PfnConnectionStateChanged* callback,
                                              void*                                   callbackData);

SOPC_StatusCode OpcUa_Channel_Disconnect(SOPC_Channel channel);

SOPC_StatusCode OpcUa_Channel_BeginConnect(SOPC_Channel                            channel,
                                           char*                                   url,
                                           SOPC_ByteString*                        clientCertificate,
                                           SOPC_ByteString*                        clientPrivateKey,
                                           SOPC_ByteString*                        serverCertificate,
                                           void*                                   pkiConfig,
                                           SOPC_String*                            requestedSecurityPolicyUri,
                                           int32_t                                 requestedLifetime,
                                           OpcUa_MessageSecurityMode               messageSecurityMode,
                                           uint32_t                                networkTimeout,
                                           SOPC_Channel_PfnConnectionStateChanged* callback,
                                           void*                                   callbackData);

END_EXTERN_C

#endif /* SOPC_WRAPPER_CHANNEL_H_ */
