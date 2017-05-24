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
#ifdef STACK_1_02
                                           char*                                   sTransportProfileUri,
#endif
                                           SOPC_ByteString*                        clientCertificate,
                                           SOPC_ByteString*                        clientPrivateKey,
                                           SOPC_ByteString*                        serverCertificate,
                                           void*                                   pkiConfig,
                                           SOPC_String*                            requestedSecurityPolicyUri,
                                           int32_t                                 requestedLifetime,
                                           OpcUa_MessageSecurityMode               messageSecurityMode,
#ifdef STACK_1_01
                                           void*                                   securityToken,
#endif
                                           uint32_t                                networkTimeout,
                                           SOPC_Channel_PfnConnectionStateChanged* callback,
                                           void*                                   callbackData);

SOPC_StatusCode OpcUa_Channel_Connect(SOPC_Channel                            channel,
                                      char*                                   url,
#ifdef STACK_1_02
                                      char*                                   sTransportProfileUri,
#endif
                                      SOPC_Channel_PfnConnectionStateChanged* callback,
                                      void*                                   callbackData,
                                      SOPC_ByteString*                        clientCertificate,
                                      SOPC_ByteString*                        clientPrivateKey,
                                      SOPC_ByteString*                        serverCertificate,
                                      void*                                   pkiConfig,
                                      SOPC_String*                            requestedSecurityPolicyUri,
                                      int32_t                                 requestedLifetime,
                                      OpcUa_MessageSecurityMode               messageSecurityMode,
#if defined STACK_1_01 || defined STACK_1_02
                                      void*                                   securityToken,
#endif
                                      uint32_t                                networkTimeout);

END_EXTERN_C

#endif /* SOPC_WRAPPER_CHANNEL_H_ */
