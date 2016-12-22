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

#ifndef SOPC_ENDPOINT_H_
#define SOPC_ENDPOINT_H_

#include "sopc_stack_csts.h"

#ifdef OPCUA_HAVE_SERVERAPI

#include "sopc_secure_channel_server_endpoint.h"
#include "sopc_encodeabletype.h"

/**
 *  \brief Endpoint serialization enumeration type
 *  Note: only binary serialization is available in INGPOCS
 */
typedef enum {
    SOPC_EndpointSerializer_Invalid = 0x00,
    SOPC_EndpointSerializer_Binary = 0x01
} SOPC_Endpoint_SerializerType;

typedef void* SOPC_Endpoint;

struct SOPC_RequestContext;

typedef enum SOPC_EndpointEvent
{
    SOPC_EndpointEvent_Invalid,
    SOPC_EndpointEvent_SecureChannelOpened,
    SOPC_EndpointEvent_SecureChannelClosed,
    SOPC_EndpointEvent_Renewed,
    SOPC_EndpointEvent_UnsupportedServiceRequested,
    SOPC_EndpointEvent_DecoderError,
    SOPC_EndpointEvent_EndpointClosed
} SOPC_EndpointEvent;

typedef SOPC_StatusCode (SOPC_EndpointEvent_CB) (SOPC_Endpoint      endpoint,
                                                 void*              cbData,
                                                 SOPC_EndpointEvent event,
                                                 SOPC_StatusCode    status,
                                                 uint32_t           secureChannelId,
                                                 const Certificate* clientCertificate,
                                                 const SOPC_String* securityPolicy);

typedef SOPC_StatusCode (SOPC_InvokeService) (SOPC_Endpoint endpoint, ...);

typedef SOPC_StatusCode (SOPC_BeginInvokeService) (SOPC_Endpoint               endpoint,
                                                   struct SOPC_RequestContext* requestContext,
                                                   void**                      a_ppRequest,
                                                   SOPC_EncodeableType*        a_pRequestType);

typedef struct SOPC_ServiceType {
    uint32_t                 RequestTypeId;
    SOPC_EncodeableType*     ResponseEncType;
    SOPC_BeginInvokeService* BeginInvokeService;
    SOPC_InvokeService*      InvokeService;
} SOPC_ServiceType;

SOPC_StatusCode SOPC_Endpoint_Create(SOPC_Endpoint*               endpoint,
                                     SOPC_Endpoint_SerializerType serialType,
                                     SOPC_ServiceType**           services); // Null terminated table

SOPC_StatusCode SOPC_Endpoint_Open(SOPC_Endpoint          endpoint,
                                   char*                  endpointURL,
                                   SOPC_EndpointEvent_CB* callback,
                                   void*                  callbackData,
                                   Certificate*           serverCertificate,
                                   AsymmetricKey*         serverKey,
                                   PKIProvider*           pki,
                                   uint8_t                nbSecuConfigs,
                                   SOPC_SecurityPolicy*   secuConfigurations);

SOPC_StatusCode SOPC_Endpoint_SendResponse(SOPC_Endpoint                endpoint,
                                           SOPC_EncodeableType*         responseType,
                                           void*                        response,
                                           struct SOPC_RequestContext** requestContext);

SOPC_StatusCode SOPC_Endpoint_Close(SOPC_Endpoint endpoint);

SOPC_StatusCode SOPC_Endpoint_Delete(SOPC_Endpoint* endpoint);

SOPC_StatusCode SOPC_Endpoint_GetServiceFunction(SOPC_Endpoint               endpoint,
                                                 struct SOPC_RequestContext* requestContext,
                                                 SOPC_InvokeService**        serviceFunction);

#endif /* SERVER API */

#endif /* SOPC_ENDPOINT_H_ */
