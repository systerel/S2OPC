/*
 *  Copyright (C) 2017 Systerel and others.
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

#ifndef SOPC_WRAPPER_ENDPOINT_H_
#define SOPC_WRAPPER_ENDPOINT_H_

#include "sopc_endpoint.h"


typedef SOPC_StatusCode (SOPC_Endpoint_PfnEndpointCallback)(SOPC_Endpoint      endpoint,
                                                            void*              callbackData,
                                                            SOPC_EndpointEvent event,
                                                            SOPC_StatusCode    status,
                                                            uint32_t           secureChannelId,
                                                            SOPC_ByteString*   clientCertificate,
                                                            SOPC_String*       securityPolicy,
                                                            uint16_t           securityMode);

typedef struct SOPC_WrapperKey {
    unsigned int    padding;
    SOPC_ByteString key;
    void*           padding2;
} SOPC_WrapperKey;

SOPC_StatusCode OpcUa_Endpoint_Create(SOPC_Endpoint*               endpoint,
                                      SOPC_Endpoint_SerializerType serializerType,
                                      SOPC_ServiceType**           supportedServices);

void OpcUa_Endpoint_Delete(SOPC_Endpoint* endpoint);

SOPC_StatusCode OpcUa_Endpoint_GetMessageSecureChannelId(SOPC_Endpoint        endpoint,
                                                         SOPC_RequestContext* context,
                                                         uint32_t*            secureChannelId);

SOPC_StatusCode OpcUa_Endpoint_GetMessageSecureChannelSecurityPolicy(SOPC_Endpoint        endpoint,
                                                                     SOPC_RequestContext* context,
                                                                     SOPC_SecurityPolicy* securityPolicy);

SOPC_StatusCode OpcUa_Endpoint_Open(SOPC_Endpoint                      endpoint,
                                    char*                              url,
#ifdef STACK_1_03
                                    uint8_t                            listenOnAllInterfaces,
#endif
                                    SOPC_Endpoint_PfnEndpointCallback* endpointCallback,
                                    void*                              endpointCallbackData,
                                    SOPC_ByteString*                   serverCertificate,
                                    SOPC_WrapperKey*                   serverPrivateKey,
                                    void*                              pkiConfig,
                                    uint32_t                           noOfSecurityPolicies,
                                    SOPC_SecurityPolicy*               securityPolicies);

SOPC_StatusCode OpcUa_Endpoint_Close(SOPC_Endpoint endpoint);

SOPC_StatusCode OpcUa_Endpoint_BeginSendResponse(SOPC_Endpoint         endpoint,
                                                 void*                 context,
                                                 void**                response,
                                                 SOPC_EncodeableType** responseType);

SOPC_StatusCode OpcUa_Endpoint_EndSendResponse(SOPC_Endpoint        endpoint,
                                               void**               context,
                                               SOPC_StatusCode      statusCode,
                                               void*                response,
                                               SOPC_EncodeableType* responseType);

SOPC_StatusCode OpcUa_Endpoint_CancelSendResponse(SOPC_Endpoint        endpoint,
                                                  SOPC_StatusCode      statusCode,
                                                  SOPC_String*         reason,
                                                  void**               context);

// NOT IMPLEMENTED ?
//void OpcUa_Endpoint_SendErrorResponse(SOPC_Endpoint   endpoint,
//                                      void*           context,
//                                      SOPC_StatusCode status);

SOPC_StatusCode OpcUa_Endpoint_GetServiceFunction(SOPC_Endpoint        endpoint,
                                                  void*                context,
                                                  SOPC_InvokeService** invokeService);

SOPC_StatusCode OpcUa_Endpoint_UpdateServiceFunctions(SOPC_Endpoint            endpoint,
                                                      uint32_t                 requestTypeId,
                                                      SOPC_BeginInvokeService* beginInvokeService,
                                                      SOPC_InvokeService*      invokeService);

SOPC_StatusCode OpcUa_Endpoint_GetCallbackData(SOPC_Endpoint  endpoint,
                                               void**         callbackData);

// TODO: implement ? Not sure it is implemented nor used.
//SOPC_StatusCode OpcUa_Endpoint_GetPeerInfo(SOPC_Endpoint endpoint,
//                                           void*         context,
//                                           SOPC_String*  peerInfo);

#endif /* SOPC_WRAPPER_ENDPOINT_H_ */
