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
#include "sopc_encodeable.h"
#include "opcua_statuscodes.h"
#include "sopc_action_queue_manager.h"

typedef struct SOPC_IntEndpoint_EventCallbackData {
    SOPC_EndpointEvent_CB*        callback;
    void*                         callbackData;
    SOPC_Endpoint                 endpoint;
    // Use the endpoint event callback data to store the open async result callback
    //  since it is used to notify end of connect operation
    SOPC_Endpoint_AsyncResult_CB* openResultCb;
    void*                         openResultCbData;
} SOPC_IntEndpoint_EventCallbackData;

typedef struct {
    SOPC_IntEndpoint_EventCallbackData* endpointData;
    SOPC_EndpointEvent                  event;
    SOPC_StatusCode                     status;
    uint32_t                            secureChannelId;
    const Certificate*                  clientCertificate;
    SOPC_String                         securityPolicy;
    OpcUa_MessageSecurityMode           securityMode;
} SOPC_IntEndpoint_ConnectionEventData;

struct SOPC_RequestContext {
    SOPC_Endpoint        endpoint;
    SC_Connection*       scConnection;
    uint32_t             requestId;
    void*                request;
    SOPC_EncodeableType* requestType;
    SOPC_ServiceType*    service;
};

typedef struct {
    SOPC_Endpoint   endpoint;
    Mutex           endOperationMutex;
    SOPC_StatusCode operationStatus;
} SOPC_IntEndpoint_EndOperationData;

typedef struct {
    SOPC_Endpoint                       endpoint;
    char*                               endpointURL;
    Certificate*                        serverCertificate;
    AsymmetricKey*                      serverKey;
    PKIProvider*                        pki;
    uint8_t                             nbSecuConfigs;
    SOPC_SecurityPolicy*                secuConfigurations;
    SOPC_IntEndpoint_EventCallbackData* endpointCbData;
    SOPC_Endpoint_AsyncResult_CB*       asyncResultCb;
    void*                               asyncResultCbData;
} SOPC_IntEndpoint_OpenData;

typedef struct SOPC_IntEndpoint_AppEndpointAsyncResultCbData {
    SOPC_Endpoint_AsyncResult_CB*           callback;
    void*                                   callbackData;
    SOPC_Endpoint                           endpoint;
    SOPC_EndpointEvent_AsyncOperationResult event;
    SOPC_StatusCode                         status;
} SOPC_IntEndpoint_AppEndpointAsyncResultCbData;

SOPC_RequestContext* SOPC_RequestContext_Create(SOPC_Endpoint        endpoint,
                                                SC_Connection*       scConnection,
                                                uint32_t             requestId,
                                                void*                pRequest,
                                                SOPC_EncodeableType* pRequestType,
                                                SOPC_ServiceType*    service)
{
    SOPC_RequestContext* result = NULL;
    if(scConnection != NULL && service != NULL){
        result = malloc(sizeof(SOPC_RequestContext));
        if(result != NULL){
            result->endpoint = endpoint;
            result->scConnection = scConnection;
            result->requestId = requestId;
            result->request = pRequest;
            result->requestType = pRequestType;
            result->service = service;
        }
    }
    return result;
}

void SOPC_RequestContext_Delete(SOPC_RequestContext* reqContext){
    if(reqContext != NULL){
        free(reqContext);
    }
}

void SOPC_Endpoint_Action_BeginInvokeCallback(void* arg){
    assert(NULL != arg);
    SOPC_RequestContext* requestContext = (SOPC_RequestContext*) arg;
    void* localRequest = requestContext->request; // Necessary to comply with parameter type (void **) and validity scope (exec of BeginInvoke)
    // TODO: action in case of failure ?
    requestContext->service->BeginInvokeService(requestContext->endpoint,
                                                requestContext,
                                                &localRequest,
                                                requestContext->requestType);
}

SOPC_IntEndpoint_EventCallbackData* SOPC_IntEndpoint_EventCallbackData_Create(SOPC_EndpointEvent_CB*        callback,
                                                                     void*                         callbackData,
                                                                     SOPC_Endpoint                 endpoint,
                                                                     SOPC_Endpoint_AsyncResult_CB* openResultCb,
                                                                     void*                         openResultCbData)
{
    SOPC_IntEndpoint_EventCallbackData* result = malloc(sizeof(SOPC_IntEndpoint_EventCallbackData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
        result->endpoint = endpoint;
        result->openResultCb = openResultCb;
        result->openResultCbData = openResultCbData;
    }
    return result;
}

void SOPC_Delete_EndpointCallbackData(SOPC_IntEndpoint_EventCallbackData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}


SOPC_IntEndpoint_ConnectionEventData* SOPC_IntEndpoint_ConnectionEventData_Create(SOPC_IntEndpoint_EventCallbackData* endpointCbData,
                                                                                  SOPC_EndpointEvent                  event,
                                                                                  SOPC_StatusCode                     status,
                                                                                  uint32_t                            secureChannelId,
                                                                                  const Certificate*                  clientCertificate,
                                                                                  SOPC_String*                        securityPolicy,
                                                                                  OpcUa_MessageSecurityMode           securityMode)
{
    SOPC_IntEndpoint_ConnectionEventData* result = malloc(sizeof(SOPC_IntEndpoint_ConnectionEventData));
    if(result != NULL){
        result->endpointData = endpointCbData;
        result->event = event;
        result->status = status;
        result->secureChannelId = secureChannelId;
        result->clientCertificate = clientCertificate;
        SOPC_String_Initialize(&result->securityPolicy);
        if(NULL != securityPolicy){
            SOPC_String_Copy(&result->securityPolicy, securityPolicy);
        }
        result->securityMode = securityMode;
    }
    return result;
}

void SOPC_IntEndpoint_ConnectionEventData_Delete(SOPC_IntEndpoint_ConnectionEventData* chCbData){
    if(chCbData != NULL){
        SOPC_String_Clear(&chCbData->securityPolicy);
        free(chCbData);
    }
}

void SOPC_Endpoint_Action_EndpointEventCallback(void* arg){
    assert(NULL != arg);
    SOPC_IntEndpoint_ConnectionEventData* connectionEventData = (SOPC_IntEndpoint_ConnectionEventData*) arg;
    SOPC_IntEndpoint_EventCallbackData* endpointCBdata = connectionEventData->endpointData;
    endpointCBdata->callback(endpointCBdata->endpoint,
                             endpointCBdata->callbackData,
                             connectionEventData->event,
                             connectionEventData->status,
                             connectionEventData->secureChannelId,
                             connectionEventData->clientCertificate,
                             &connectionEventData->securityPolicy,
                             connectionEventData->securityMode);
    SOPC_IntEndpoint_ConnectionEventData_Delete(connectionEventData);
}

SOPC_IntEndpoint_AppEndpointAsyncResultCbData* SOPC_IntEndpoint_AppEndpointAsyncResutlCbData_Create
    (SOPC_Endpoint_AsyncResult_CB*           callback,
     void*                                   callbackData,
     SOPC_Endpoint                           endpoint,
     SOPC_EndpointEvent_AsyncOperationResult event,
     SOPC_StatusCode                         status)
{
    SOPC_IntEndpoint_AppEndpointAsyncResultCbData* result = malloc(sizeof(SOPC_IntEndpoint_AppEndpointAsyncResultCbData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
        result->endpoint = endpoint;
        result->event = event;
        result->status = status;
    }
    return result;
}

void SOPC_IntEndpoint_AppEndpointAsyncResutlCbData_Delete(SOPC_IntEndpoint_AppEndpointAsyncResultCbData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}

void SOPC_Endpoint_Action_AppEndpointAsyncResutlCallback(void* arg){
    assert(NULL != arg);
    SOPC_IntEndpoint_AppEndpointAsyncResultCbData* cbData = (SOPC_IntEndpoint_AppEndpointAsyncResultCbData*) arg;
    cbData->callback(cbData->endpoint,
                     cbData->callbackData,
                     cbData->event,
                     cbData->status);
    SOPC_IntEndpoint_AppEndpointAsyncResutlCbData_Delete(cbData);
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


SOPC_StatusCode SOPC_IntEndpoint_SecureChannelEvent_CB(SC_ServerEndpoint*        sEndpoint,
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
    SOPC_IntEndpoint_EventCallbackData* endpointCBdata = (SOPC_IntEndpoint_EventCallbackData*) cbData;
    SOPC_IntEndpoint_ConnectionEventData* connectionEventData = NULL;
    SOPC_IntEndpoint_AppEndpointAsyncResultCbData* asynResultData = NULL;
    switch(event){
        case SC_EndpointListenerEvent_Opened:
            if(NULL != endpointCBdata->openResultCb){
                asynResultData = SOPC_IntEndpoint_AppEndpointAsyncResutlCbData_Create(endpointCBdata->openResultCb,
                                                                                      endpointCBdata->openResultCbData,
                                                                                      sEndpoint,
                                                                                      SOPC_EndpointAsync_OpenResult,
                                                                                      status);
                if(NULL != asynResultData){
                    retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                  SOPC_Endpoint_Action_AppEndpointAsyncResutlCallback,
                                                                  (void*) asynResultData,
                                                                  "Endpoint applicative callback open result");
                }
            }
            if(STATUS_OK != status){
                // It means open operation failed => closed state
                SOPC_Delete_EndpointCallbackData(endpointCBdata);
            }
            break;
        case SC_EndpointListenerEvent_Closed:
            // Deallocation of the endpoint cb data
            SOPC_Delete_EndpointCallbackData(endpointCBdata);
            break;
        case SC_EndpointConnectionEvent_New:
            if(NULL != endpointCBdata->callback){
                // TODO: copy certificate and secu policy: no guarantee it will stay allocated
                connectionEventData = SOPC_IntEndpoint_ConnectionEventData_Create(endpointCBdata,
                                                                                  SOPC_EndpointEvent_SecureChannelOpened,
                                                                                  status,
                                                                                  scConnection->secureChannelId,
                                                                                  scConnection->otherAppPublicKeyCert,
                                                                                  &scConnection->currentSecuPolicy,
                                                                                  scConnection->currentSecuMode);
                if(NULL != connectionEventData){
                    retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                  SOPC_Endpoint_Action_EndpointEventCallback,
                                                                  (void*) connectionEventData,
                                                                  "Endpoint applicative callback event 'SecureChannelOpened'");
                }
            }
            break;
        case SC_EndpointConnectionEvent_Renewed:
            assert(FALSE); // Not implemented
            break;
        case SC_EndpointConnectionEvent_Disconnected:
            if(NULL != endpointCBdata->callback){
                connectionEventData = SOPC_IntEndpoint_ConnectionEventData_Create(endpointCBdata,
                                                                                  SOPC_EndpointEvent_SecureChannelClosed,
                                                                                  status,
                                                                                  scConnection->secureChannelId,
                                                                                  NULL,
                                                                                  NULL,
                                                                                  OpcUa_MessageSecurityMode_Invalid);
                if(NULL != connectionEventData){
                    retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                  SOPC_Endpoint_Action_EndpointEventCallback,
                                                                  (void*) connectionEventData,
                                                                  "Endpoint applicative callback event 'SecureChannelClosed'");
                }
            }
            break;
        case SC_EndpointConnectionEvent_Request:
            service = SOPC_Endpoint_FindService(sEndpoint, reqEncType->TypeId);
            if(NULL == service){
                if(NULL != endpointCBdata->callback){
                    // TODO: report the encodeable type ?
                    connectionEventData = SOPC_IntEndpoint_ConnectionEventData_Create(endpointCBdata,
                                                                                      SOPC_EndpointEvent_UnsupportedServiceRequested,
                                                                                      OpcUa_BadServiceUnsupported,
                                                                                      scConnection->secureChannelId,
                                                                                      NULL,
                                                                                      NULL,
                                                                                      OpcUa_MessageSecurityMode_Invalid);
                    if(NULL != connectionEventData){
                        retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                      SOPC_Endpoint_Action_EndpointEventCallback,
                                                                      (void*) connectionEventData,
                                                                      "Endpoint applicative callback event 'UnsupportedServiceRequested'");
                    }
                }
            }else{
                if(requestId != NULL && reqEncObj != NULL && reqEncType != NULL){
                    reqContext = SOPC_RequestContext_Create((SOPC_Endpoint) sEndpoint,
                                                            scConnection,
                                                            *requestId,
                                                            reqEncObj,
                                                            reqEncType,
                                                            service);
                    if(reqContext != NULL){
                        retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                      SOPC_Endpoint_Action_BeginInvokeCallback,
                                                                      (void*) reqContext,
                                                                      "Begin Invoke applicative service");
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

void SOPC_IntEndpoint_Action_BeginOpen(void* arg){
    assert(NULL != arg);
    SOPC_IntEndpoint_OpenData* openData = (SOPC_IntEndpoint_OpenData*) arg;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) openData->endpoint;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(sEndpoint->state != SC_Endpoint_Closed){
        status = STATUS_INVALID_STATE;
    }else{
        SOPC_StackConfiguration_Locked();
        status = SC_ServerEndpoint_Configure(sEndpoint,
                                             SOPC_StackConfiguration_GetNamespaces(),
                                             SOPC_StackConfiguration_GetEncodeableTypes());
        if(status == STATUS_OK){
            status = SC_ServerEndpoint_Open(openData->endpoint, openData->endpointURL,
                                            openData->pki,
                                            openData->serverCertificate, openData->serverKey,
                                            openData->nbSecuConfigs, openData->secuConfigurations,
                                            SOPC_IntEndpoint_SecureChannelEvent_CB, openData->endpointCbData);
        }
    }

    if(STATUS_OK != status){
        // If operation failed, notify it internally
        SOPC_IntEndpoint_SecureChannelEvent_CB(openData->endpoint,
                                      NULL,
                                      (void*) openData->endpointCbData,
                                      SC_EndpointListenerEvent_Opened,
                                      status,
                                      NULL,
                                      NULL,
                                      NULL);
    }

    free(openData);
}

SOPC_StatusCode SOPC_Endpoint_AsyncOpen(SOPC_Endpoint                 endpoint,
                                        char*                         endpointURL,
                                        SOPC_EndpointEvent_CB*        callback,
                                        void*                         callbackData,
                                        Certificate*                  serverCertificate,
                                        AsymmetricKey*                serverKey,
                                        PKIProvider*                  pki,
                                        uint8_t                       nbSecuConfigs,
                                        SOPC_SecurityPolicy*          secuConfigurations,
                                        SOPC_Endpoint_AsyncResult_CB* asyncCb,
                                        void*                         asyncCbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntEndpoint_OpenData* openData = NULL;
    SOPC_IntEndpoint_EventCallbackData* endpointCbData = NULL;
    if(endpoint != NULL && endpointURL != NULL &&
       callback != NULL &&
       nbSecuConfigs > 0 && secuConfigurations != NULL)
    {
        status = STATUS_NOK;

        openData = malloc(sizeof(SOPC_IntEndpoint_OpenData));

        if(NULL != openData){
            endpointCbData = SOPC_IntEndpoint_EventCallbackData_Create(callback,
                                                              callbackData,
                                                              endpoint,
                                                              asyncCb,
                                                              asyncCbData);
        }

        if(NULL != openData && NULL != endpointCbData){
            status = STATUS_OK;
            openData->endpoint = endpoint;
            openData->endpointURL = endpointURL;
            openData->serverCertificate = serverCertificate;
            openData->serverKey = serverKey;
            openData->pki = pki;
            openData->nbSecuConfigs = nbSecuConfigs;
            openData->secuConfigurations = secuConfigurations;
            openData->endpointCbData = endpointCbData;
            openData->asyncResultCb = asyncCb;
            openData->asyncResultCbData = asyncCbData;
        }

        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_IntEndpoint_Action_BeginOpen,
                                                       (void*) openData,
                                                       "Begin open endpoint");
            if(STATUS_OK != status){
                SOPC_Delete_EndpointCallbackData(endpointCbData);
                free(openData);
            }
        }

    }
    return status;
}

void SOPC_IntEndpoint_AsyncOpenCB(SOPC_Endpoint                           endpoint,
                                  void*                                   cbData,
                                  SOPC_EndpointEvent_AsyncOperationResult cEvent,
                                  SOPC_StatusCode                         status)
{
    (void) endpoint;
    assert(cEvent == SOPC_EndpointAsync_OpenResult);
    SOPC_IntEndpoint_EndOperationData* endOpenData = (SOPC_IntEndpoint_EndOperationData*) cbData;
    endOpenData->operationStatus = status;
    Mutex_Unlock(&endOpenData->endOperationMutex);
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
    // Temporarly used of a mutex, must use a barrier to have a timeout value
    SOPC_IntEndpoint_EndOperationData endOpenData;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(endpoint != NULL && endpointURL != NULL &&
       callback != NULL &&
       nbSecuConfigs > 0 && secuConfigurations != NULL)
    {
        status = Mutex_Initialization(&endOpenData.endOperationMutex);
        endOpenData.operationStatus = STATUS_NOK;
        endOpenData.endpoint = endpoint;
        if(STATUS_OK == status){
            status = Mutex_Lock(&endOpenData.endOperationMutex);
        }

        if(STATUS_OK == status){
            status = SOPC_Endpoint_AsyncOpen(endpoint,
                                             endpointURL,
                                             callback,
                                             callbackData,
                                             serverCertificate,
                                             serverKey,
                                             pki,
                                             nbSecuConfigs,
                                             secuConfigurations,
                                             SOPC_IntEndpoint_AsyncOpenCB,
                                             (void*) &endOpenData);
        }

        if(STATUS_OK == status){
            status = Mutex_Lock(&endOpenData.endOperationMutex);
        }

        if(STATUS_OK == status){
            status = endOpenData.operationStatus;
        }

        Mutex_Clear(&endOpenData.endOperationMutex);
    }
    return status;
}

void SOPC_IntEndpoint_EndOperation_CB(void*           callbackData,
                                      SOPC_StatusCode status){
    assert(callbackData != NULL);
    SOPC_IntEndpoint_EndOperationData* operationEndData = (SOPC_IntEndpoint_EndOperationData*) callbackData;
    operationEndData->operationStatus = status;
    Mutex_Unlock(&operationEndData->endOperationMutex);
}

SOPC_StatusCode SOPC_Endpoint_CreateResponse(SOPC_Endpoint         endpoint,
                                             SOPC_RequestContext*  context,
                                             void**                response,
                                             SOPC_EncodeableType** responseType){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    (void) endpoint;
    if(context != NULL && response != NULL && responseType != NULL){
        status = STATUS_OK;
        if(NULL == *responseType){
            status = SOPC_Endpoint_GetContextResponseType(context, responseType);
        }
        if(STATUS_OK == status){
            status = SOPC_Encodeable_Create(*responseType, response);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_SendResponse(SOPC_Endpoint         endpoint,
                                           SOPC_EncodeableType*  responseType,
                                           void*                 response,
                                           SOPC_RequestContext** requestContext)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_RequestContext* context = NULL;
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    SOPC_IntEndpoint_EndOperationData endSendResponse;
    endSendResponse.endpoint = endpoint;
    endSendResponse.operationStatus = STATUS_NOK;
    if(sEndpoint != NULL && requestContext != NULL && *requestContext != NULL){
        context = (SOPC_RequestContext*) *requestContext;

        status = Mutex_Initialization(&endSendResponse.endOperationMutex);

        if(STATUS_OK == status){
            status = Mutex_Lock(&endSendResponse.endOperationMutex);
        }

        if(STATUS_OK == status){
            status = SC_CreateAction_Send_Response(sEndpoint,
                                                   context->scConnection,
                                                   context->requestId,
                                                   responseType,
                                                   response,
                                                   SOPC_IntEndpoint_EndOperation_CB,
                                                   (void*) &endSendResponse);
        }
        // Deallocate request context now send response is sent
        SOPC_RequestContext_Delete(context);
        *requestContext = NULL;
    }

    if(STATUS_OK == status){
        status = Mutex_Lock(&endSendResponse.endOperationMutex);
    }

    if(STATUS_OK == status){
        status = endSendResponse.operationStatus;
    }

    Mutex_Unlock(&endSendResponse.endOperationMutex);
    Mutex_Clear(&endSendResponse.endOperationMutex);

    return status;
}

SOPC_StatusCode SOPC_Endpoint_CancelSendResponse(SOPC_Endpoint                endpoint,
                                                 SOPC_StatusCode              errorCode,
                                                 SOPC_String*                 reason,
                                                 SOPC_RequestContext**        requestContext)
{
    (void) endpoint;
    (void) errorCode;
    (void) reason;
    SOPC_RequestContext_Delete(*requestContext);
    *requestContext = NULL;
    return STATUS_OK;
}



SOPC_StatusCode SOPC_Endpoint_BeginClose(SOPC_Endpoint endpoint){
    return SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                             SC_ServerEndpoint_CloseAux,
                                             (void*) endpoint,
                                             "Begin close endpoint");
}

void SOPC_Endpoint_Action_EndpointClose(void* arg){
    SOPC_IntEndpoint_EndOperationData* closeData = (SOPC_IntEndpoint_EndOperationData*) arg;
    assert(NULL != closeData && NULL != closeData->endpoint);
    SOPC_StatusCode status = SC_ServerEndpoint_Close(closeData->endpoint);
    // Call end operation callback
    SOPC_IntEndpoint_EndOperation_CB(arg,
                                     status);
}

SOPC_StatusCode SOPC_Endpoint_Close(SOPC_Endpoint endpoint){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    SOPC_IntEndpoint_EndOperationData endCloseData;
    endCloseData.operationStatus = STATUS_NOK;

    if(endpoint != NULL){

        endCloseData.endpoint = endpoint;

        status = Mutex_Initialization(&endCloseData.endOperationMutex);

        if(STATUS_OK == status){
            status = Mutex_Lock(&endCloseData.endOperationMutex);
        }

        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Endpoint_Action_EndpointClose,
                                                       (void*) &endCloseData,
                                                       "Delete endpoint");
        }
    }

    if(STATUS_OK == status){
        Mutex_Lock(&endCloseData.endOperationMutex);
        status = endCloseData.operationStatus;
    }

    Mutex_Unlock(&endCloseData.endOperationMutex);
    Mutex_Clear(&endCloseData.endOperationMutex);

    return status;
}

SOPC_StatusCode SOPC_Endpoint_BeginDelete(SOPC_Endpoint endpoint){
    return SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                             SC_ServerEndpoint_DeleteAux,
                                             (void*) endpoint,
                                             "Begin delete endpoint");
}

void SOPC_Endpoint_Action_EndpointDelete(void* arg){
    SOPC_IntEndpoint_EndOperationData* deleteData = (SOPC_IntEndpoint_EndOperationData*) arg;
    assert(NULL != deleteData && NULL != deleteData->endpoint);
    SC_ServerEndpoint_Delete(deleteData->endpoint);
    // Call end operation callback
    SOPC_IntEndpoint_EndOperation_CB(arg,
                                  STATUS_OK);
}

SOPC_StatusCode SOPC_Endpoint_Delete(SOPC_Endpoint* endpoint){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    SOPC_IntEndpoint_EndOperationData endDeleteData;
    endDeleteData.operationStatus = STATUS_NOK;

    if(endpoint != NULL && *endpoint != NULL){

        endDeleteData.endpoint = *endpoint;

        status = Mutex_Initialization(&endDeleteData.endOperationMutex);

        if(STATUS_OK == status){
            status = Mutex_Lock(&endDeleteData.endOperationMutex);
        }

        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Endpoint_Action_EndpointDelete,
                                                       (void*) &endDeleteData,
                                                       "Delete endpoint");
        }
    }

    if(STATUS_OK == status){
        Mutex_Lock(&endDeleteData.endOperationMutex);
        status = endDeleteData.operationStatus;
    }

    if(STATUS_OK == status){
        *endpoint = NULL;
    }

    Mutex_Unlock(&endDeleteData.endOperationMutex);
    Mutex_Clear(&endDeleteData.endOperationMutex);

    return status;
}

void* SOPC_Endpoint_GetCallbackData(SOPC_Endpoint endpoint){
    SC_ServerEndpoint* sEndpoint = (SC_ServerEndpoint*) endpoint;
    SOPC_IntEndpoint_EventCallbackData* iCbData = NULL;
    void* result = NULL;
    if(sEndpoint != NULL){
        iCbData = (SOPC_IntEndpoint_EventCallbackData*) SC_ServerEndpoint_GetCallbackData(sEndpoint);
        if(iCbData != NULL)
            result = iCbData->callbackData;
    }
    return result;
}

SOPC_StatusCode SOPC_Endpoint_GetServiceFunction(SOPC_Endpoint        endpoint,
                                                 SOPC_RequestContext* requestContext,
                                                 SOPC_InvokeService** serviceFunction)
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

SOPC_StatusCode SOPC_Endpoint_GetContextSecureChannelId(SOPC_RequestContext* context,
                                                        uint32_t*            secureChannelId){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(secureChannelId != NULL && context != NULL && context->scConnection != NULL){
        *secureChannelId = context->scConnection->secureChannelId;
        status = STATUS_OK;
    }
    return status;
}

SOPC_StatusCode SOPC_Endpoint_GetContextSecureChannelSecurityPolicy(SOPC_RequestContext*       context,
                                                                    SOPC_String*               securityPolicy,
                                                                    OpcUa_MessageSecurityMode* securityMode){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
     if(securityPolicy != NULL && securityMode != NULL && context != NULL && context->scConnection != NULL)
     {
         status = SOPC_String_Copy(securityPolicy, &context->scConnection->currentSecuPolicy);
         *securityMode = context->scConnection->currentSecuMode;
     }
     return status;
}

SOPC_StatusCode SOPC_Endpoint_GetContextResponseType(SOPC_RequestContext*  context,
                                                     SOPC_EncodeableType** respType){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(context != NULL && respType != NULL){
        *respType = context->service->ResponseEncType;
        status = STATUS_OK;
    }
    return status;
}


#endif // SERVER API
