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

#include "sopc_endpoint.h"

#ifdef OPCUA_HAVE_SERVERAPI

#include <assert.h>

#include "sopc_secure_channel_server_endpoint.h"
#include "sopc_stack_config.h"
#include "sopc_serverapi.h"
#include "opcua_statuscodes.h"

typedef struct {
    SOPC_EndpointEvent_CB* callback;
    void*                  callbackData;
} SOPC_InternalEndpoint_CallbackData;

typedef struct SOPC_RequestContext {
    SC_Connection*    scConnection;
    uint32_t          requestId;
    SOPC_ServiceType* service;
} SOPC_RequestContext;

SOPC_RequestContext* SOPC_Create_EndpointRequestContext(SC_Connection*    scConnection,
                                                        uint32_t          requestId,
                                                        SOPC_ServiceType* service)
{
    SOPC_RequestContext* result = NULL;
    if(scConnection != NULL && service != NULL){
        result = malloc(sizeof(SOPC_RequestContext));
        if(result != NULL){
            result->scConnection = scConnection;
            result->requestId = requestId;
            result->service = service;
        }
    }
    return result;
}

void SOPC_Delete_EndpointRequestContext(SOPC_RequestContext* reqContext){
    if(reqContext != NULL){
        free(reqContext);
    }
}

SOPC_InternalEndpoint_CallbackData* SOPC_Create_EndpointCallbackData(SOPC_EndpointEvent_CB* callback,
                                                                     void*                  callbackData)
{
    SOPC_InternalEndpoint_CallbackData* result = malloc(sizeof(SOPC_InternalEndpoint_CallbackData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
    }
    return result;
}

void SOPC_Delete_EndpointCallbackData(SOPC_InternalEndpoint_CallbackData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}

SOPC_ServiceType* SOPC_Endpoint_FindService(SC_ServerEndpoint* sEndpoint,
                                            uint32_t requestTypeId){
    SOPC_ServiceType** serviceTable = NULL;
    SOPC_ServiceType* current = NULL;
    SOPC_ServiceType* result = NULL;
    uint32_t idx = 0;
    if(sEndpoint->servicesTable != NULL){
        serviceTable = (SOPC_ServiceType**) sEndpoint->servicesTable;
    }else{
        serviceTable = SOPC_SupportedServiceTypes;
    }
    do{
        current = serviceTable[idx];
        if(NULL != current){
            if(current->RequestTypeId == requestTypeId){
                result = current;
            }
            idx++;
        }
    }
    while(current != NULL && result == NULL);

    return result;
}


SOPC_StatusCode SecureChannelEndpointEvent_CB(struct SC_ServerEndpoint* sEndpoint,
                                              SC_Connection*            scConnection,
                                              void*                     cbData,
                                              SC_EndpointEvent          event,
                                              SOPC_StatusCode           status,
                                              uint32_t*                 requestId,
                                              SOPC_EncodeableType*      reqEncType,
                                              void*                     reqEncObj){
    SOPC_StatusCode retStatus = STATUS_OK;
    SOPC_ServiceType* service = NULL;
    SOPC_RequestContext* reqContext = NULL;
    SOPC_InternalEndpoint_CallbackData* endpointCBdata = (SOPC_InternalEndpoint_CallbackData*) cbData;
    SOPC_String secuPol; // To avoid giving possibility of security policy modification
    SOPC_String_Initialize(&secuPol);
    switch(event){
        case SC_EndpointListenerEvent_Opened:
            break;
        case SC_EndpointListenerEvent_Closed:
            // Deallocation of the endpoint cb data
            SOPC_Delete_EndpointCallbackData(endpointCBdata);
            break;
        case SC_EndpointConnectionEvent_New:
            if(NULL != endpointCBdata->callback){
                retStatus = SOPC_String_Copy(&secuPol, &scConnection->currentSecuPolicy);
                if(STATUS_OK == retStatus){
                    retStatus = endpointCBdata->callback((SOPC_Endpoint) sEndpoint,
                                                         endpointCBdata->callbackData,
                                                         SOPC_EndpointEvent_SecureChannelOpened,
                                                         status,
                                                         scConnection->secureChannelId,
                                                         scConnection->otherAppPublicKeyCert,
                                                         &secuPol,
                                                         scConnection->currentSecuMode);
                }
                SOPC_String_Clear(&secuPol);
            }
            break;
        case SC_EndpointConnectionEvent_Renewed:
            assert(FALSE); // Not implemented
            break;
        case SC_EndpointConnectionEvent_Disconnected:
            if(NULL != endpointCBdata->callback){
                retStatus = SOPC_String_Copy(&secuPol, &scConnection->currentSecuPolicy);
                if(STATUS_OK == retStatus){
                    retStatus = endpointCBdata->callback((SOPC_Endpoint) sEndpoint,
                                                         endpointCBdata->callbackData,
                                                         SOPC_EndpointEvent_SecureChannelClosed,
                                                         status,
                                                         scConnection->secureChannelId,
                                                         scConnection->otherAppPublicKeyCert,
                                                         &secuPol,
                                                         scConnection->currentSecuMode);
                }
                SOPC_String_Clear(&secuPol);
            }
            break;
        case SC_EndpointConnectionEvent_Request:
            service = SOPC_Endpoint_FindService(sEndpoint, reqEncType->TypeId);
            if(NULL == service){
                if(NULL != endpointCBdata->callback){
                    // TODO: report the encodeable type ?
                    retStatus = endpointCBdata->callback((SOPC_Endpoint) sEndpoint,
                                                         endpointCBdata->callbackData,
                                                         SOPC_EndpointEvent_UnsupportedServiceRequested,
                                                         OpcUa_BadServiceUnsupported,
                                                         scConnection->secureChannelId,
                                                         NULL,
                                                         NULL,
                                                         OpcUa_MessageSecurityMode_Invalid);
                }
            }else{
                if(requestId != NULL && reqEncObj != NULL && reqEncType != NULL){
                    reqContext = SOPC_Create_EndpointRequestContext(scConnection, *requestId, service);
                    if(reqContext != NULL){
                        retStatus = service->BeginInvokeService((SOPC_Endpoint) sEndpoint,
                                                                reqContext,
                                                                &reqEncObj,
                                                                reqEncType);
                    }else{
                        retStatus = STATUS_NOK;
                    }
                }else{
                    retStatus = STATUS_INVALID_PARAMETERS;
                }
            }
            break;
        case SC_EndpointConnectionEvent_PartialRequest:
        case SC_EndpointConnectionEvent_AbortRequest:
        case SC_EndpointConnectionEvent_DecoderError:
            // TODO: do something except for log ?
            break;
    }
    return retStatus;
}

SOPC_StatusCode SOPC_Endpoint_Create(SOPC_Endpoint*               endpoint,
                                     SOPC_Endpoint_SerializerType serialType,
                                     SOPC_ServiceType**           services)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ServerEndpoint* sEndpoint = NULL;
    // TODO: manage services registering
    if(endpoint != NULL && serialType == SOPC_EndpointSerializer_Binary){
        sEndpoint = SC_ServerEndpoint_Create();
        if(sEndpoint != NULL){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
        if(STATUS_OK == status && services != NULL){
            sEndpoint->servicesTable = (void**) services;
        }
        *endpoint = sEndpoint;
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_Open(SOPC_Endpoint          endpoint,
                                   char*                  endpointURL,
                                   SOPC_EndpointEvent_CB* callback,
                                   void*                  callbackData,
                                   Certificate*           serverCertificate,
                                   AsymmetricKey*         serverKey,
                                   PKIProvider*           pki,
                                   uint8_t                nbSecuConfigs,
                                   SOPC_SecurityPolicy*   secuConfigurations)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_InternalEndpoint_CallbackData* endpointCbData = NULL;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    if(sEndpoint != NULL && endpointURL != NULL &&
       callback != NULL &&
       nbSecuConfigs > 0 && secuConfigurations != NULL)
    {
        if(sEndpoint->state != SC_Endpoint_Closed){
            status = STATUS_INVALID_STATE;
        }else{
            StackConfiguration_Locked();
            status = SC_ServerEndpoint_Configure(sEndpoint,
                                                 StackConfiguration_GetNamespaces(),
                                                 StackConfiguration_GetEncodeableTypes());
            if(status == STATUS_OK){
                endpointCbData = SOPC_Create_EndpointCallbackData(callback, callbackData);
                if(endpointCbData == NULL){
                    status = STATUS_NOK;
                }
            }
            if(status == STATUS_OK){
                status = SC_ServerEndpoint_Open(endpoint, endpointURL,
                                                pki,
                                                serverCertificate, serverKey,
                                                nbSecuConfigs, secuConfigurations,
                                                SecureChannelEndpointEvent_CB, endpointCbData);
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_SendResponse(SOPC_Endpoint                endpoint,
                                           SOPC_EncodeableType*         responseType,
                                           void*                        response,
                                           struct SOPC_RequestContext** requestContext){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_RequestContext* context = NULL;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    if(sEndpoint != NULL && requestContext != NULL && *requestContext != NULL){
        context = (SOPC_RequestContext*) *requestContext;
        status = SC_Send_Response(sEndpoint, context->scConnection, context->requestId,
                                  responseType, response);
        // Deallocate request context now send response is sent
        SOPC_Delete_EndpointRequestContext(context);
        *requestContext = NULL;
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_AbortResponse(SOPC_Endpoint                endpoint,
                                            SOPC_StatusCode              errorCode,
                                            SOPC_String*                 reason,
                                            struct SOPC_RequestContext** requestContext){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_RequestContext* context = NULL;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    if(sEndpoint != NULL && requestContext != NULL && *requestContext != NULL && reason != NULL){
        context = (SOPC_RequestContext*) *requestContext;
        status = SC_AbortMsg(context->scConnection->sendingBuffer,
                             errorCode,
                             reason);
        // Deallocate request context now response is aborted
        SOPC_Delete_EndpointRequestContext(context);
        *requestContext = NULL;
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_Close(SOPC_Endpoint endpoint){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    if(sEndpoint != NULL){
        status = SC_ServerEndpoint_Close(sEndpoint);
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_Delete(SOPC_Endpoint* endpoint){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ServerEndpoint* sEndpoint = NULL;
    if(endpoint != NULL && *endpoint != NULL){
        sEndpoint = (SC_ServerEndpoint*) *endpoint;
        SC_ServerEndpoint_Delete(sEndpoint);
        *endpoint = NULL;
    }
    return status;
}

void* SOPC_Endpoint_GetCallbackData(SOPC_Endpoint endpoint){
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    SOPC_InternalEndpoint_CallbackData* iCbData = NULL;
    void* result = NULL;
    if(sEndpoint != NULL){
        iCbData = (SOPC_InternalEndpoint_CallbackData*) SC_ServerEndpoint_GetCallbackData(sEndpoint);
        if(iCbData != NULL)
            result = iCbData->callbackData;
    }
    return result;
}

SOPC_StatusCode SOPC_Endpoint_GetServiceFunction(SOPC_Endpoint               endpoint,
                                                 struct SOPC_RequestContext* requestContext,
                                                 SOPC_InvokeService**        serviceFunction)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    SOPC_RequestContext* reqContext;
    if(sEndpoint != NULL && requestContext != NULL && serviceFunction != NULL){
        reqContext = (SOPC_RequestContext*) requestContext;
        *serviceFunction = reqContext->service->InvokeService;
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_GetContextSecureChannelId(struct SOPC_RequestContext* context,
                                                        uint32_t*                   secureChannelId){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(secureChannelId != NULL && context != NULL && context->scConnection != NULL){
        *secureChannelId = context->scConnection->secureChannelId;
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_GetContextSecureChannelSecurityPolicy(struct SOPC_RequestContext* context,
                                                                    SOPC_String*                securityPolicy,
                                                                    OpcUa_MessageSecurityMode*  securityMode){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
     if(securityPolicy != NULL && securityMode != NULL && context != NULL && context->scConnection != NULL)
     {
         status = SOPC_String_Copy(securityPolicy, &context->scConnection->currentSecuPolicy);
         *securityMode = context->scConnection->currentSecuMode;
     }
     return status;
}

SOPC_StatusCode SOPC_Endpoint_GetContextResponseType(struct SOPC_RequestContext* context,
                                                     SOPC_EncodeableType**       respType){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(context != NULL && respType != NULL){
        *respType = context->service->ResponseEncType;
        status = STATUS_OK;
    }
    return status;
}


#endif // SERVER API
