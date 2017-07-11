/*
 * sopc_sc_events.c
 *
 *  Created on: Jul 7, 2017
 *      Author: vincent
 */
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

#include "sopc_base_types.h"
#include "sopc_sc_events.h"
#include "singly_linked_list.h"
#include "sopc_channel.h"
#include "sopc_secure_channel_client_connection.h"
#include "opcua_identifiers.h"

static SOPC_EventDispatcherManager* tmpToolkitMgr = NULL;

typedef enum SOPC_TMP_Services_Event {
  /* SC to Services events */
  EP_SC_CONNECTED,
  EP_CLOSED,
  SC_CONNECTED,
  SC_CONNECTION_TIMEOUT,
  SC_DISCONNECTED,
  SC_SERVICE_RCV_MSG,
  /* App to Services events */
  SE_OPEN_ENDPOINT,
  SE_CLOSE_ENDPOINT,
  SE_ACTIVATE_SESSION, /* Connect SC + Create Session + Activate session */
  SE_SEND_SESSION_REQUEST, // TODO: manage buffer when session with channel lost ? Or return a send failure in this case
  SE_CLOSE_SESSION,
  //  SE_SEND_PUBLIC_REQUEST, => discovery services /* Connect SC */
} SOPC_TMP_Services_Event;

static SLinkedList* endpointInstList = NULL;
static SLinkedList* secureChConConfigList = NULL;
static SLinkedList* secureChInstList = NULL;


SOPC_EventDispatcherManager* scEventDispatcherMgr = NULL;

void SOPC_TEMP_InitEventDispMgr(SOPC_EventDispatcherManager* toolkitMgr){
    endpointInstList = SLinkedList_Create(0);
    secureChConConfigList = SLinkedList_Create(0);
    secureChInstList = SLinkedList_Create(0);
    tmpToolkitMgr = toolkitMgr;
    scEventDispatcherMgr =
            SOPC_EventDispatcherManager_CreateAndStart(SOPC_SecureChannelEventDispatcher,
                                                       "(Services) Application event dispatcher manager");
}


/* Function provided to the communication Stack to redirect a received service message to the Toolkit */
SOPC_StatusCode TMP_BeginService(SOPC_Endpoint               a_hEndpoint,
                                 struct SOPC_RequestContext* a_hContext,
                                 void**                      a_ppRequest,
                                 SOPC_EncodeableType*        a_pRequestType){
  SOPC_Toolkit_Msg* tMsg = calloc(1, sizeof(SOPC_Toolkit_Msg));
  uint32_t* pConfigIdx = SOPC_Endpoint_GetCallbackData(a_hEndpoint);
  assert(NULL != tMsg && NULL != pConfigIdx);

  tMsg->msg = *a_ppRequest;
  tMsg->encType = a_pRequestType;
  tMsg->respEncType = NULL; // used only on client side for invoking service
  tMsg->isRequest = (!FALSE);
  tMsg->optContext = a_hContext;

  SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                       SC_SERVICE_RCV_MSG,
                                       *pConfigIdx, // TMP: to be replaced by connection Id
                                       tMsg,
                                       0, // set status ? (if not used for context on server side)
                                       "Secure channel (server) received request");

  // => deallocation to be done by toolkit
  // SOPC_Encodeable_Delete(a_pRequestType, a_ppRequest);

  return STATUS_OK;
}

/* Create session service provided to the Stack */
struct SOPC_ServiceType SOPC_Toolkit_CreateSession_ServiceType =
{
    OpcUaId_CreateSessionRequest,
    &OpcUa_CreateSessionResponse_EncodeableType,
    TMP_BeginService,
    NULL
};

/* Activate session service provided to the Stack */
struct SOPC_ServiceType SOPC_Toolkit_ActivateSession_ServiceType =
{
    OpcUaId_ActivateSessionRequest,
    &OpcUa_ActivateSessionResponse_EncodeableType,
   TMP_BeginService,
    NULL
};

/* Close session service provided to the Stack */
struct SOPC_ServiceType SOPC_Toolkit_CloseSession_ServiceType =
{
    OpcUaId_CloseSessionRequest,
    &OpcUa_CloseSessionResponse_EncodeableType,
   TMP_BeginService,
    NULL
};

/* Read service provided to the Stack */
struct SOPC_ServiceType SOPC_Toolkit_Read_ServiceType =
{
    OpcUaId_ReadRequest,
    &OpcUa_ReadResponse_EncodeableType,
    TMP_BeginService,
    NULL
};

/* Write service provided to the Stack */
struct SOPC_ServiceType SOPC_Toolkit_Write_ServiceType =
{
    OpcUaId_WriteRequest,
    &OpcUa_WriteResponse_EncodeableType,
    TMP_BeginService,
    NULL
};

/* List of services provided to the Stack */
SOPC_ServiceType* SOPC_Toolkit_SupportedServiceTypes[] =
{
    &SOPC_Toolkit_CreateSession_ServiceType,
    &SOPC_Toolkit_ActivateSession_ServiceType,
    &SOPC_Toolkit_CloseSession_ServiceType,
    &SOPC_Toolkit_Read_ServiceType,
    &SOPC_Toolkit_Write_ServiceType,
    NULL
};

SOPC_StatusCode TMP_EndpointEvent_CB(SOPC_Endpoint             endpoint,
                                     void*                     cbData,
                                     SOPC_EndpointEvent        event,
                                     SOPC_StatusCode           status,
                                     uint32_t                  secureChannelId,
                                     const Certificate*        clientCertificate,
                                     const SOPC_String*        securityPolicy,
                                     OpcUa_MessageSecurityMode securityMode){
    uint32_t* epConfigIdx = (uint32_t*) cbData;
    SOPC_SecureChannel_ConnectedConfig* scConConfig = NULL;
    (void) endpoint;
    switch(event){
    case SOPC_EndpointEvent_Invalid:
    case SOPC_EndpointEvent_Renewed:
    case SOPC_EndpointEvent_DecoderError:
    case SOPC_EndpointEvent_EndpointClosed:
        assert(epConfigIdx != NULL);
        SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                             EP_CLOSE,
                                             *epConfigIdx,
                                             NULL,
                                             (int32_t) status,
                                             "Endpoint closed for given reason");
        break;
    case SOPC_EndpointEvent_SecureChannelOpened:
        // Create new secure channel config
        scConConfig = malloc(sizeof(SOPC_SecureChannel_ConnectedConfig));
        assert(NULL != scConConfig);
        scConConfig->config = malloc(sizeof(SOPC_SecureChannel_Config));
        scConConfig->config->crt_cli = clientCertificate;
        scConConfig->config->msgSecurityMode = securityMode;
        scConConfig->config->reqSecuPolicyUri = SOPC_String_GetRawCString(securityPolicy);
        scConConfig->connectionId = secureChannelId; // For now we can just use scId but in the future it is better to be a shared index with Sockets !
        scConConfig->configIdx = 0; // TODO: add to toolkit config with index ?

        assert(SLinkedList_FindFromId(secureChConConfigList, scConConfig->connectionId) == NULL); // must be unique id !
        SLinkedList_Prepend(secureChConConfigList, scConConfig->connectionId, (void*) scConConfig);
        // TODO: add to toolkit config ?
        SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                             EP_SC_CONNECTED,
                                             *epConfigIdx,
                                             (void*) scConConfig,
                                             secureChannelId,
                                             "Secure channel connected on Endpoint");
        break;
    case SOPC_EndpointEvent_SecureChannelClosed:
        scConConfig = SLinkedList_RemoveFromId(secureChConConfigList, secureChannelId);
        if(NULL != scConConfig){
            if(NULL != scConConfig->config){
                free(scConConfig->config);
            }
            free(scConConfig);
        }
        SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                             SC_DISCONNECTED,
                                             secureChannelId,
                                             NULL,
                                             (int32_t) status,
                                             "Secure channel disconnected from Endpoint");
        break;
    case SOPC_EndpointEvent_UnsupportedServiceRequested:
    default:
        assert(FALSE);
    }
    return OpcUa_BadNotImplemented;
}

SOPC_StatusCode TMP_SecureChannelEvent_CB (SOPC_Channel       channel,
                                           void*              cbData,
                                           SOPC_Channel_Event cEvent,
                                           SOPC_StatusCode    status){
    SOPC_SecureChannel_ConnectedConfig* scConConfig = NULL;
    SOPC_Channel ch = NULL;
    assert(NULL != channel && NULL != cbData);
    uint32_t* configIdx = (uint32_t*) cbData;
    SC_ClientConnection* clientCon = (SC_ClientConnection*) SOPC_Channel_GetConnection(channel);
    switch(cEvent){
    case SOPC_ChannelEvent_Invalid:
        assert(FALSE);
        break;
    case SOPC_ChannelEvent_Connected:
        // Create new secure channel config
        scConConfig = malloc(sizeof(SOPC_SecureChannel_ConnectedConfig));
        assert(NULL != scConConfig);
        scConConfig->config = malloc(sizeof(SOPC_SecureChannel_Config));
        scConConfig->config->crt_cli = clientCon->clientCertificate;
        scConConfig->config->msgSecurityMode = clientCon->securityMode;
        scConConfig->config->reqSecuPolicyUri = SOPC_String_GetRawCString(&clientCon->securityPolicy);
        scConConfig->connectionId = clientCon->instance->secureChannelId; // For now we can just use scId but in the future it is better to be a shared index with Sockets !
        scConConfig->configIdx = *configIdx; // TODO: add to toolkit config with index ?

        // CAUTION: due to API adaptation we do not use same idx when toolkit is client and server which
        //  could lead to Id conflicts !!! => DO NOT USE TOOLKIT IN SAME TIME AS CLIENT AND SERVER FOR NOW !
        assert(SLinkedList_FindFromId(secureChConConfigList, scConConfig->configIdx) == NULL); // must be unique id !
        SLinkedList_Prepend(secureChConConfigList, scConConfig->configIdx, (void*) scConConfig);
        // TODO: add to toolkit config ?
        SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                             SC_CONNECTED,
                                             scConConfig->secureChannelId,
                                             (void*) scConConfig,
                                             clientCon->instance->secureChannelId,
                                             "Client secure channel connected");
        break;
    case SOPC_ChannelEvent_Disconnected:
        ch = (SOPC_Channel) SLinkedList_RemoveFromId(secureChInstList, *configIdx);
        assert(channel == ch);
        SOPC_Channel_Delete(&channel);
        assert(NULL != tmpToolkitMgr);
        SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                             SC_CONNECTION_TIMEOUT,
                                             *configIdx,
                                             NULL,
                                             (int32_t) status,
                                             "Secure channel closed for given reason");
        break;
    }
    return STATUS_OK;
}

SOPC_StatusCode TMP_SecureChannelResponse_CB(SOPC_Channel         channel,
                                             void*                response,
                                             SOPC_EncodeableType* responseType,
                                             void*                cbData,
                                             SOPC_StatusCode      status){
    (void) status;
    SOPC_Toolkit_Msg* tMsg = calloc(1, sizeof(SOPC_Toolkit_Msg));
    uint32_t id = ((SC_ClientConnection*) SOPC_Channel_GetConnection(channel))->instance->secureChannelId;
    tMsg->isRequest = FALSE;
    tMsg->msg = response;
    tMsg->encType = responseType;
    tMsg->optContext = cbData;
    SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                         SC_SERVICE_RCV_MSG,
                                         id, // TMP: to be replaced by connection Id
                                         tMsg,
                                         0, // set status ? (if not used for context on server side)
                                         "Secure channel (client) received response");
    return STATUS_OK;
}

void SOPC_SecureChannelEventDispatcher(int32_t  scEvent,
                                       uint32_t id,
                                       void*    params,
                                       int32_t  auxParam){
    assert(NULL != tmpToolkitMgr);
    SOPC_Endpoint ep = NULL;
    SOPC_Endpoint_Config* epConfig = NULL;
    uint32_t* idContext = NULL;
    SOPC_Channel ch = NULL;
    SOPC_SecureChannel_Config* scConfig = NULL;
    SOPC_StatusCode status = STATUS_OK;
    SOPC_SecureChannel_ConnectedConfig* scConConfig = NULL; // TMP: needed to store config due to stack/toolkit separation (no access to "shared" context)
    SOPC_Toolkit_Msg* tMsg = NULL;
    switch((SOPC_SC_Event) scEvent){
    /** SC external events */
    /* Services to SC events */
    case EP_OPEN:
        // id ==  endpoint configuration index
        // params = endpoint configuration pointer => TMP due to separation toolkit / stack
        // auxParam == ?
        epConfig = (SOPC_Endpoint_Config*) params;
        idContext = malloc(sizeof(uint32_t)); // TMP memory leak necessary for scope of id
        if(NULL != idContext && NULL != epConfig){
            status = SOPC_Endpoint_Create(&ep, SOPC_EndpointSerializer_Binary, SOPC_Toolkit_SupportedServiceTypes);
        }else{
            status = STATUS_NOK;
        }
        if(STATUS_OK == status){
            *idContext = id;
            status = SOPC_Endpoint_Open(ep,
                    epConfig->endpointURL,
                    TMP_EndpointEvent_CB,
                    (void*) idContext,
                    epConfig->serverCertificate,
                    epConfig->serverKey,
                    epConfig->pki,
                    epConfig->nbSecuConfigs,
                    epConfig->secuConfigurations);
            if(STATUS_OK == status){
                if(ep != SLinkedList_Prepend(endpointInstList, id, (void*) ep)){
                    status = STATUS_NOK;
                }
            }
            if(STATUS_OK != status){
                SOPC_Endpoint_Delete(&ep);
            }
        }
        if(STATUS_OK != status){
            SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                    EP_CLOSED,
                    id,
                    NULL,
                    (int32_t) status,
                    "Endpoint opening failed");
        }
        break;

    case EP_CLOSE:
        // id ==  endpoint configuration index
        ep = (SOPC_Endpoint) SLinkedList_RemoveFromId(endpointInstList, id);
        SOPC_Endpoint_Delete(&ep);
        assert(NULL != tmpToolkitMgr);
        SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                EP_CLOSED,
                id,
                NULL,
                auxParam, // auxParam == status
                "Endpoint closed on demand");
        break;

    case SC_CONNECT:
        // id ==  sc configuration index
        // params = sc configuration pointer => TMP due to separation toolkit / stack
        // auxParam == ?
        scConfig = (SOPC_SecureChannel_Config*) params;
        idContext = malloc(sizeof(uint32_t)); // TMP memory leak necessary for scope of id
        if(NULL != idContext && NULL != scConfig){
            status = SOPC_Channel_Create(&ch, SOPC_EndpointSerializer_Binary);
        }else{
            status = STATUS_NOK;
        }
        if(STATUS_OK == status){
            *idContext = id;
            status = SOPC_Channel_BeginConnect(ch,
                                               scConfig->url,
                                               scConfig->crt_cli,
                                               scConfig->key_priv_cli,
                                               scConfig->crt_srv,
                                               scConfig->pki,
                                               scConfig->reqSecuPolicyUri,
                                               scConfig->requestedLifetime,
                                               scConfig->msgSecurityMode,
                                               1000,
                                               TMP_SecureChannelEvent_CB,
                                               (void*) idContext);
            if(STATUS_OK == status){
                if(ch != SLinkedList_Prepend(secureChInstList, id, (void*) ch)){
                    status = STATUS_NOK;
                }
            }
            if(STATUS_OK != status){
                SOPC_Channel_Delete(&ch);
            }
        }
        if(STATUS_OK != status){
            SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                    SC_CONNECTION_TIMEOUT,
                    id,
                    NULL,
                    (int32_t) status,
                    "Secure channel connection failed");
        }
        break;

    case SC_DISCONNECT:
        assert(NULL != scConConfig);
        ch = (SOPC_Channel) SLinkedList_RemoveFromId(secureChInstList, id);
        scConConfig = SLinkedList_RemoveFromId(secureChConConfigList, id);
        if(NULL != scConConfig){
            SOPC_Channel_Delete(&ch);
            assert(NULL != tmpToolkitMgr);
            SOPC_EventDispatcherManager_AddEvent(tmpToolkitMgr,
                                                 SC_DISCONNECTED,
                                                 scConConfig->secureChannelId,
                                                 NULL,
                                                 scConConfig->connectionId,
                                                 "Secure channel closed on demand");
            if(NULL != scConConfig->config){
                free(scConConfig->config);
            }
            free(scConConfig);
        }
        break;

    case SC_SERVICE_SND_MSG:
        // id ==  connection id
        // params = byte buffer (node Id + OPC UA message)
        // auxParam == secure channel config id ? => tmp since connection defined correctly by stack
        tMsg = params;
        ch = (SOPC_Channel) SLinkedList_FindFromId(secureChInstList, auxParam);
        assert(ch != NULL && tMsg != NULL);
        SOPC_Channel_BeginInvokeService(ch, NULL,
                                        tMsg->msg,
                                        tMsg->encType,
                                        tMsg->respEncType,
                                        TMP_SecureChannelResponse_CB,
                                        NULL //request context ? => type params ?
                                        );
        break;

    case EP_SC_SERVICE_SND_MSG:
        // id ==  endpoint Id id
        // params => message + context ???
        // auxParam => ???
        tMsg = params;
        ep = (SOPC_Endpoint) SLinkedList_FindFromId(endpointInstList, id);
        assert(ep != NULL && tMsg != NULL);
        SOPC_Endpoint_SendResponse(ep,
                                   tMsg->encType,
                                   tMsg->msg,
                                   tMsg->optContext);
        break;

        /* Sockets to SC events */
    case SOCKET_CONNECTION:
        // id ==  endpoint configuration index
        // params = NULL ?
        // auxParam == fresh connection id
        assert(FALSE);
        break;
    case SOCKET_FAILURE:
        // id ==  endpoint config index or SC configuration index or connection Id
        // params = NULL ?
        // auxParam == error status
        assert(FALSE);
        break;
    case SOCKET_RCV_BYTES:
        // id ==  connection Id
        // params = bytes buffer
        // auxParam == ?
        assert(FALSE);
        break;
        /** SC internal events */
        /* SC mgr to EP mgr */
    case EP_SC_DISCONNECTED:
        // id ==  endpoint configuration index
        // params = NULL ?
        // auxParam == connection Id
        assert(FALSE);
        break;
        /* EP mgr to SC mgr */
    case EP_SC_CREATE:
        // id ==  endpoint configuration index
        // params = NULL ?
        // auxParam == connection Id
        assert(FALSE);
        break;
        /* SC mgr to Chunks mgr */
    case ENCODE_HEL:
        // id ==  connection Id
        // params = params MAX TCP ?
        // auxParam == ?
        assert(FALSE);
        break;
    case ENCODE_ACK:
        // id ==  connection Id
        // params = params revised TCP ?
        // auxParam == ?
        assert(FALSE);
        break;
    case ENCODE_OPN_REQ:
        // id ==  connection Id
        // params == NULL ? => connection Id for SC config sufficient
        // auxParam == ?
        assert(FALSE);
        break;
    case ENCODE_OPN_RESP:
        // id ==  connection Id
        // params == NULL ? => connection Id for SC config sufficient
        // auxParam == status ?
        assert(FALSE);
        break;
    case ENCODE_CLO_REQ:
        // id ==  connection Id
        // params == NULL ?
        // auxParam == ?
        assert(FALSE);
        break;
    case ENCODE_CLO_RESP:
        // id ==  connection Id
        // params == NULL ?
        // auxParam == status ?
        assert(FALSE);
        break;
    case ENCODE_MSG_CHUNKS:
        // id ==  connection Id
        // params == Bytes buffer (OPC UA message: node Id + message)
        // auxParam == ?
        assert(FALSE);
        break;
        /* Chunks mgr to SC mgr */
    case SC_RCV_HEL:
        // id ==  connection Id
        // params == bytes buffer with TCP HEL payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_ACK:
        // id ==  connection Id
        // params == bytes buffer with TCP HEL payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_OPN_REQ:
        // id ==  connection Id
        // params == bytes buffer with OPN REQ payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_OPN_RESP:
        // id ==  connection Id
        // params == bytes buffer with OPN RESP payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_CLO_REQ:
        // id ==  connection Id
        // params == bytes buffer with CLO REQ payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_CLO_RESP:
        // id ==  connection Id
        // params == bytes buffer with CLO RESP payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_MSG:
        // id ==  connection Id
        // params == bytes buffer with MSG payload
        // auxParam == ?
        assert(FALSE);
        break;
    case SC_RCV_FAILURE:
        // id ==  connection Id
        // params == NULL ?
        // auxParam == status
        assert(FALSE);
        break;
    case SC_ENCODE_FAILURE:
        // id ==  connection Id
        // params == byte buffer ?
        // auxParam == status
        assert(FALSE);
        break;
    default:
        assert(FALSE);
    }
}
