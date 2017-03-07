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

#include "sopc_channel.h"

#ifdef OPCUA_HAVE_CLIENTAPI

#include "opcua_statuscodes.h"

#include "sopc_builtintypes.h"
#include "sopc_secure_channel_client_connection.h"
#include "sopc_stack_config.h"
#include "sopc_threads.h"
#include "sopc_action_queue_manager.h"

#include <assert.h>
#include <stdlib.h>

typedef struct SOPC_IntChannel_CallbackData {
    SOPC_Channel_PfnConnectionStateChanged* callback;
    void*                                   callbackData;
    SOPC_Channel                            channel;
    // Use the channel event callback data to store the connect async result callback
    //  since it is used to notify end of connect operation
    SOPC_Channel_AsyncResult_CB*            connectCb;
    void*                                   connectCbData;

    // Internal use for synchronous connect
    uint8_t                                 isSyncConnectFlag;
} SOPC_IntChannel_CallbackData;

typedef struct SOPC_IntChannel_AppChannelEventCbData {
    SOPC_Channel_PfnConnectionStateChanged* callback;
    void*                                   callbackData;
    SOPC_Channel                            channel;
    SOPC_Channel_Event                      event;
    SOPC_StatusCode                         status;
} SOPC_IntChannel_AppChannelEventCbData;

typedef struct SOPC_IntChannel_AppChannelAsyncResultCbData {
    SOPC_Channel_AsyncResult_CB*           callback;
    void*                                  callbackData;
    SOPC_Channel                           channel;
    SOPC_ChannelEvent_AsyncOperationResult event;
    SOPC_StatusCode                        status;
} SOPC_IntChannel_AppChannelAsyncResultCbData;

// Could be used as callbackData in SOPC_IntChannel_AppChannelAsyncResultCbData
// when a flag change on async result event is needed
typedef struct SOPC_IntChannel_AsyncResultFlagData {
    Mutex           dataMutex;
    uint8_t         freeInCallback;
    uint8_t         flag;
    SOPC_StatusCode status;
} SOPC_IntChannel_AsyncResultFlagData;

typedef struct SOPC_IntChannel_EndOperationData {
    SOPC_Channel    channel;
    uint8_t         endOperationFlag;
    Mutex           endOperationMutex;
    SOPC_StatusCode operationStatus;
} SOPC_IntChannel_EndOperationData;

typedef struct SOPC_IntChannel_ConnectData {
    SC_ClientConnection*                    cConnection;
    const char*                             url;
    const Certificate*                      crt_cli;
    const AsymmetricKey*                    key_priv_cli;
    const Certificate*                      crt_srv;
    const PKIProvider*                      pki;
    const char*                             reqSecuPolicyUri;
    int32_t                                 requestedLifetime;
    OpcUa_MessageSecurityMode               msgSecurityMode;
    uint32_t                                networkTimeout;
    SOPC_IntChannel_CallbackData*           channelCbData;
    SOPC_Channel_AsyncResult_CB*            connectCb;
    void*                                   connectCbData;
} SOPC_IntChannel_ConnectData;

typedef struct SOPC_IntChannel_DisconnectData {
    SC_ClientConnection*                    cConnection;
    SOPC_Channel_AsyncResult_CB*            disconnectCb;
    void*                                   disconnectCbData;
} SOPC_IntChannel_DisconnectData;

// TODO: "atomic" callbackData for multithreading
typedef struct SOPC_IntChannel_InvokeCallbackData {
    SOPC_Channel                     channel;
    void*                            response;
    SOPC_EncodeableType*             responseType;
    SOPC_Channel_PfnRequestComplete* cb;
    void*                            cbData;
    SOPC_StatusCode                  status;
} SOPC_IntChannel_InvokeCallbackData;

SOPC_IntChannel_CallbackData* SOPC_IntChannel_CallbackData_Create(uint8_t                                 isSyncConnect,
                                                                  SOPC_Channel_PfnConnectionStateChanged* callback,
                                                                  void*                                   callbackData,
                                                                  SOPC_Channel                            channel,
                                                                  SOPC_Channel_AsyncResult_CB*            connectCb,
                                                                  void*                                   connectCbData)
{
    SOPC_IntChannel_CallbackData* result = malloc(sizeof(SOPC_IntChannel_CallbackData));
    if(result != NULL){
        result->isSyncConnectFlag = isSyncConnect;
        result->callback = callback;
        result->callbackData = callbackData;
        result->channel = channel;
        result->connectCb = connectCb;
        result->connectCbData = connectCbData;
    }
    return result;
}

void SOPC_IntChannel_CallbackData_Delete(SOPC_IntChannel_CallbackData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}

SOPC_IntChannel_AppChannelEventCbData* SOPC_IntChannel_AppChannelEventCbData_Create(SOPC_Channel_PfnConnectionStateChanged* callback,
                                                                                    void*                                   callbackData,
                                                                                    SOPC_Channel                            channel,
                                                                                    SOPC_Channel_Event                      event,
                                                                                    SOPC_StatusCode                         status)
{
    SOPC_IntChannel_AppChannelEventCbData* result = malloc(sizeof(SOPC_IntChannel_AppChannelEventCbData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
        result->channel = channel;
        result->event = event;
        result->status = status;
    }
    return result;
}

void SOPC_IntChannel_AppChannelEventCbData_Delete(SOPC_IntChannel_AppChannelEventCbData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}

void SOPC_Channel_Action_AppChannelEventCallback(void* arg){
    assert(NULL != arg);
    SOPC_IntChannel_AppChannelEventCbData* cbData = (SOPC_IntChannel_AppChannelEventCbData*) arg;
    cbData->callback(cbData->channel,
                     cbData->callbackData,
                     cbData->event,
#ifdef STACK_1_02
                           NULL,
#endif
                     cbData->status);
    SOPC_IntChannel_AppChannelEventCbData_Delete(cbData);
}

SOPC_IntChannel_AppChannelAsyncResultCbData* SOPC_IntChannel_AppChannelAsyncResutlCbData_Create
    (SOPC_Channel_AsyncResult_CB*           callback,
     void*                                  callbackData,
     SOPC_Channel                           channel,
     SOPC_ChannelEvent_AsyncOperationResult event,
     SOPC_StatusCode                        status)
{
    SOPC_IntChannel_AppChannelAsyncResultCbData* result = malloc(sizeof(SOPC_IntChannel_AppChannelAsyncResultCbData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
        result->channel = channel;
        result->event = event;
        result->status = status;
    }
    return result;
}

void SOPC_IntChannel_AppChannelAsyncResutlCbData_Delete(SOPC_IntChannel_AppChannelAsyncResultCbData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}

void SOPC_Channel_Action_AppChannelAsyncResutlCallback(void* arg){
    assert(NULL != arg);
    SOPC_IntChannel_AppChannelAsyncResultCbData* cbData = (SOPC_IntChannel_AppChannelAsyncResultCbData*) arg;
    cbData->callback(cbData->channel,
                     cbData->callbackData,
                     cbData->event,
                     cbData->status);
    SOPC_IntChannel_AppChannelAsyncResutlCbData_Delete(cbData);
}

SOPC_IntChannel_AsyncResultFlagData* SOPC_IntChannel_AsyncResultFlagData_Create()
{
    SOPC_IntChannel_AsyncResultFlagData* result = malloc(sizeof(SOPC_IntChannel_AsyncResultFlagData));
    if(NULL != result){
        Mutex_Initialization(&result->dataMutex);
        result->freeInCallback = FALSE;
        result->flag = FALSE;
        result->status = STATUS_NOK;
    }
    return result;
}

void SOPC_IntChannel_AsyncResultFlagData_Delete(SOPC_IntChannel_AsyncResultFlagData* data)
{
    if(NULL != data){
        Mutex_Clear(&data->dataMutex);
        free(data);
    }
}

// Manage the async result flag for notifying the sync function waiting for it
void SOPC_IntChannel_AsyncResultFlagCB(SOPC_Channel                           channel,
                                       void*                                  cbData,
                                       SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                       SOPC_StatusCode                        status)
{
    (void) cEvent;
    assert(NULL != channel);
    uint8_t freeDataHere = FALSE;
    SOPC_IntChannel_AsyncResultFlagData* asynResultData = (SOPC_IntChannel_AsyncResultFlagData*) cbData;
    Mutex_Lock(&asynResultData->dataMutex);
    asynResultData->status = status;
    asynResultData->flag = 1;
    freeDataHere = asynResultData->freeInCallback;
    Mutex_Unlock(&asynResultData->dataMutex);

    if(FALSE != freeDataHere){
        // Sync function using the flag has stopped before callback
        // data must be freed here
        SOPC_IntChannel_AsyncResultFlagData_Delete(asynResultData);
    }
}

SOPC_IntChannel_InvokeCallbackData* SOPC_IntChannel_InvokeCallbackData_Create(SOPC_Channel                     channel,
                                                                              SOPC_Channel_PfnRequestComplete* cb,
                                                                              void*                            cbData){
    SOPC_IntChannel_InvokeCallbackData* result = malloc(sizeof(SOPC_IntChannel_InvokeCallbackData));
    if(NULL != result){
        result->channel = channel;
        result->cb = cb;
        result->cbData =cbData;
        result->response = NULL;
        result->responseType = NULL;
        result->status = STATUS_NOK;
    }
    return result;
}

void SOPC_IntChannel_InvokeCallbackData_Set(SOPC_IntChannel_InvokeCallbackData*  invCbData,
                                            void*                response,
                                            SOPC_EncodeableType* responseType,
                                            SOPC_StatusCode      status)
{
    invCbData->response = response;
    invCbData->responseType = responseType;
    invCbData->status = status;
}

SOPC_StatusCode SOPC_IntChannel_InvokeCallbackData_Get(SOPC_IntChannel_InvokeCallbackData*   invCbData,
                                                       void**                 response,
                                                       SOPC_EncodeableType**  responseType)
{
    *response = invCbData->response;
    *responseType = invCbData->responseType;
    return invCbData->status;
}

void SOPC_IntChannel_InvokeCallbackData_Delete(SOPC_IntChannel_InvokeCallbackData* invCbData){
    if(invCbData != NULL){
        free(invCbData);
    }
}


void SOPC_Channel_Action_AppCallback(void* arg){
    assert(NULL != arg);
    SOPC_IntChannel_InvokeCallbackData* appCbData = (SOPC_IntChannel_InvokeCallbackData*) arg;
    appCbData->cb(appCbData->channel,
                  appCbData->response,
                  appCbData->responseType,
                  appCbData->cbData,
                  appCbData->status);
    SOPC_IntChannel_InvokeCallbackData_Delete(appCbData);
}

SOPC_StatusCode SOPC_Channel_CreateAction_AppInvokeCallback(SOPC_Channel         channel,
                                                            void*                response,
                                                            SOPC_EncodeableType* responseType,
                                                            void*                cbData,
                                                            SOPC_StatusCode      status){
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    (void) channel;
    SOPC_IntChannel_InvokeCallbackData* appCbData = (SOPC_IntChannel_InvokeCallbackData*) cbData;
    if(appCbData != NULL){
        retStatus = STATUS_OK;
        SOPC_IntChannel_InvokeCallbackData_Set(appCbData,
                               response, responseType,
                               status);
        retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                      SOPC_Channel_Action_AppCallback,
                                                      (void*) appCbData,
                                                      "Channel invoke service response applicative callback");
    }
    return retStatus;
}

SOPC_StatusCode SOPC_Channel_Create(SOPC_Channel*               channel,
                                    SOPC_Channel_SerializerType serialType){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != NULL && serialType == SOPC_ChannelSerializer_Binary){
        *channel = SC_Client_Create();
        if(channel != NULL){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

void SOPC_IntChannel_BeginDeleteDiscoResultCB(SOPC_Channel                           channel,
                                              void*                                  cbData,
                                              SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                              SOPC_StatusCode                        status)
{
    assert(cEvent == SOPC_ChannelAsync_DisconnectResult);
    (void) cbData;
    if(STATUS_OK == status){
        SC_Client_Delete(channel);
    }
}

SOPC_StatusCode SOPC_Channel_AsyncDelete(SOPC_Channel channel){
    return SOPC_Channel_AsyncDisconnect(channel,
                                        SOPC_IntChannel_BeginDeleteDiscoResultCB,
                                        NULL);
}

SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != NULL && *channel != NULL){

        // Ensure disconnect called for deallocation
        status = SOPC_Channel_Disconnect(*channel);
        if(STATUS_OK == status){
            SC_Client_Delete(*channel);
        }
        // No need to wait delete action terminated to set pointer to null
        *channel = NULL;
    }
    return status;
}

SOPC_StatusCode SOPC_IntChannel_ChannelEventCB(SC_ClientConnection* cConnection,
                                               void*                cbData,
                                               SC_ConnectionEvent   event,
                                               SOPC_StatusCode      status){
    (void) cConnection;
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel_CallbackData* callbackData = cbData;
    SOPC_IntChannel_AppChannelEventCbData* appCallbackData = NULL;
    SOPC_Channel_Event channelConnectionEvent = SOPC_ChannelEvent_Invalid;

    switch(event){
        case SOPC_ConnectionEvent_Connected:
            channelConnectionEvent = SOPC_ChannelEvent_Connected;
            break;
        case SOPC_ConnectionEvent_Disconnected:
        case SOPC_ConnectionEvent_ConnectionFailed:
            channelConnectionEvent = SOPC_ChannelEvent_Disconnected;
            break;
        case SOPC_ConnectionEvent_SecureMessageComplete:
        case SOPC_ConnectionEvent_SecureMessageChunk:
        case SOPC_ConnectionEvent_SecureMessageAbort:
            // TODO: traces ?
            break;
        case SOPC_ConnectionEvent_Invalid:
        case SOPC_ConnectionEvent_UnexpectedError:
            channelConnectionEvent = SOPC_ChannelEvent_Disconnected;
            break;
    }

    // Channel event management
    switch(channelConnectionEvent){
        case SOPC_ChannelEvent_Invalid:
            // Nothing to do
            retStatus = STATUS_OK;
            break;
        case SOPC_ChannelEvent_Connected:
            if(callbackData != NULL && callbackData->callback != NULL) // Note: in sync mode we do not notify application of connection
            {
                // Inhibit connect event if we are in sync mode
                if(callbackData->isSyncConnectFlag == FALSE){
                    appCallbackData = SOPC_IntChannel_AppChannelEventCbData_Create(callbackData->callback,
                                                                                   callbackData->callbackData,
                                                                                   callbackData->channel,
                                                                                   channelConnectionEvent,
                                                                                   status);
                    if(NULL != appCallbackData){
                        retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                      SOPC_Channel_Action_AppChannelEventCallback,
                                                                      (void*) appCallbackData,
                                                                      "Channel event applicative callback");
                    }else{
                        retStatus = STATUS_NOK;
                    }
                }

                // Also notify as async result
                SOPC_IntChannel_AppChannelAsyncResultCbData* asyncCbData =
                        SOPC_IntChannel_AppChannelAsyncResutlCbData_Create(callbackData->connectCb,
                                                                           callbackData->connectCbData,
                                                                           callbackData->channel,
                                                                           SOPC_ChannelAsync_ConnectResult,
                                                                           status);
                if(NULL != asyncCbData){
                    // If asynchronous call to connect, we need to notify applicative layer of error
                    // otherwise it is reported internally using the errorFlag
                    SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                      SOPC_Channel_Action_AppChannelAsyncResutlCallback,
                                                      (void*) asyncCbData,
                                                      "Channel async result event applicative callback");
                }
            }
            break;
        case SOPC_ChannelEvent_Disconnected:
            if(event == SOPC_ConnectionEvent_ConnectionFailed){
                // Connection failure:
                // Only notify as async result
                SOPC_IntChannel_AppChannelAsyncResultCbData* asyncCbData =
                        SOPC_IntChannel_AppChannelAsyncResutlCbData_Create(callbackData->connectCb,
                                                                           callbackData->connectCbData,
                                                                           callbackData->channel,
                                                                           SOPC_ChannelAsync_ConnectResult,
                                                                           status);
                if(NULL != asyncCbData){
                    // If asynchronous call to connect, we need to notify applicative layer of error
                    // otherwise it is reported internally using the errorFlag
                    SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                      SOPC_Channel_Action_AppChannelAsyncResutlCallback,
                                                      (void*) asyncCbData,
                                                      "Channel async connection failure event applicative callback");
                }
            }else{
                // Unexpected disconnection
                if(callbackData != NULL && callbackData->callback != NULL)
                {
                    appCallbackData = SOPC_IntChannel_AppChannelEventCbData_Create(callbackData->callback,
                                                                                   callbackData->callbackData,
                                                                                   callbackData->channel,
                                                                                   channelConnectionEvent,
                                                                                   status);
                    if(NULL != appCallbackData){
                        retStatus = SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                                                      SOPC_Channel_Action_AppChannelEventCallback,
                                                                      (void*) appCallbackData,
                                                                      "Channel event applicative callback");
                    }else{
                        retStatus = STATUS_NOK;
                    }
                }
            }
            break;
    }
    return retStatus;
}

void SOPC_IntChannel_BeginConnectStatus(SOPC_IntChannel_ConnectData* connectData,
                                        SOPC_StatusCode              beginConnectStatus){
    SOPC_IntChannel_AppChannelAsyncResultCbData* appCallbackData = NULL;
    // TODO: report good connect result (then it will report a connected channel event if connection established ?)
    if(STATUS_OK != beginConnectStatus){
        appCallbackData = SOPC_IntChannel_AppChannelAsyncResutlCbData_Create(connectData->connectCb,
                                                                             connectData->connectCbData,
                                                                             connectData->cConnection,
                                                                             SOPC_ChannelAsync_ConnectResult,
                                                                             beginConnectStatus);
        if(NULL != appCallbackData){
            // If asynchronous call to connect, we need to notify applicative layer of error
            // otherwise it is reported internally using the errorFlag
            SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                              SOPC_Channel_Action_AppChannelAsyncResutlCallback,
                                              (void*) appCallbackData,
                                              "Channel async result event applicative callback");
        }
    }
}

void SOPC_Channel_Action_BeginConnect(void* arg){
    assert(arg != NULL);
    SOPC_IntChannel_ConnectData* connectData = (SOPC_IntChannel_ConnectData*) arg;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(connectData->cConnection->instance->state != SC_Connection_Disconnected){
        status = STATUS_INVALID_STATE;
    }else{
        SOPC_StackConfiguration_Locked();
        status = SC_Client_Configure(connectData->cConnection,
                                     SOPC_StackConfiguration_GetNamespaces(),
                                     SOPC_StackConfiguration_GetEncodeableTypes());

        if(status == STATUS_OK){
            status = SC_Client_Connect(connectData->cConnection,
                                       connectData->url,
                                       connectData->pki,
                                       connectData->crt_cli, connectData->key_priv_cli,
                                       connectData->crt_srv,
                                       connectData->msgSecurityMode,
                                       connectData->reqSecuPolicyUri,
                                       connectData->requestedLifetime,
                                       SOPC_IntChannel_ChannelEventCB,
                                       connectData->channelCbData);
        }
    }

    SOPC_IntChannel_BeginConnectStatus(connectData, status);

    free(connectData);
}

SOPC_StatusCode SOPC_IntChannel_BeginConnect(uint8_t                                 isSyncConnectFlag,
                                             SOPC_Channel                            channel,
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
                                             void*                                   cbData,
                                             SOPC_Channel_AsyncResult_CB*            connectCb,
                                             void*                                   connectCbData,
                                             SOPC_IntChannel_CallbackData**          channelCbData)
{
    assert(channelCbData != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    SOPC_IntChannel_ConnectData* connectData = NULL;
    (void) networkTimeout;

    if(cConnection != NULL && cConnection->instance != NULL &&
       url != NULL &&
       ((crt_cli != NULL && key_priv_cli != NULL &&
         crt_srv != NULL && pki != NULL)
        || msgSecurityMode == OpcUa_MessageSecurityMode_None) &&
       reqSecuPolicyUri != NULL &&
       msgSecurityMode != OpcUa_MessageSecurityMode_Invalid &&
       cb != NULL)
    {
        status = STATUS_NOK;

        connectData = malloc(sizeof(SOPC_IntChannel_ConnectData));

        if(NULL != connectData){
            *channelCbData = SOPC_IntChannel_CallbackData_Create(isSyncConnectFlag,
                                                                 cb,
                                                                 cbData,
                                                                 channel,
                                                                 connectCb,
                                                                 connectCbData);
        }

        if(NULL != connectData && NULL != *channelCbData){
            status = STATUS_OK;
            connectData->cConnection = cConnection;
            connectData->url = url;
            connectData->crt_cli = crt_cli;
            connectData->key_priv_cli = key_priv_cli;
            connectData->crt_srv = crt_srv;
            connectData->pki = pki;
            connectData->reqSecuPolicyUri = reqSecuPolicyUri;
            connectData->requestedLifetime = requestedLifetime;
            connectData->msgSecurityMode = msgSecurityMode;
            connectData->networkTimeout = networkTimeout;
            connectData->channelCbData = *channelCbData;
            // Stored also in connect data in addition to channelCbData for simple access
            connectData->connectCb = connectCb;
            connectData->connectCbData = connectCbData;
        }

        if(STATUS_OK == status){
            status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                       SOPC_Channel_Action_BeginConnect,
                                                       (void*) connectData,
                                                       "Begin connect channel");
            if(STATUS_OK != status){
                SOPC_IntChannel_CallbackData_Delete(*channelCbData);
                *channelCbData = NULL;
                free(connectData);
            }
        }
    }

    return status;
}

// Mimics disconnected event callback in case of failure to comply with Foundation API
void SOPC_IntChannel_BeginConnect_ConnectCB(SOPC_Channel                           channel,
                                            void*                                  cbData,
                                            SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                            SOPC_StatusCode                        status)
{
    assert(NULL != channel);
    (void) cbData;
    SC_ClientConnection* scConnection = (SC_ClientConnection*) channel;
    SOPC_IntChannel_CallbackData* icbData = (SOPC_IntChannel_CallbackData*) scConnection->callbackData;
    if(cEvent == SOPC_ChannelAsync_ConnectResult && STATUS_OK != status){
        icbData->callback(icbData->channel,
                          icbData->callbackData,
                          SOPC_ChannelEvent_Disconnected,
    #ifdef STACK_1_02
                               NULL,
    #endif
                          status);
    }

}

SOPC_StatusCode SOPC_Channel_BeginConnect(SOPC_Channel                            channel,
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
                                          void*                                   cbData)
{
    SOPC_IntChannel_CallbackData* internalCbData = NULL;
    return SOPC_IntChannel_BeginConnect(FALSE, channel, url, crt_cli,
                                        key_priv_cli, crt_srv,
                                        pki, reqSecuPolicyUri,
                                        requestedLifetime, msgSecurityMode,
                                        networkTimeout, cb, cbData,
                                        SOPC_IntChannel_BeginConnect_ConnectCB, NULL,
                                        &internalCbData);
}

SOPC_StatusCode SOPC_Channel_AsyncConnect(SOPC_Channel                            channel,
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
                                          void*                                   cbData,
                                          SOPC_Channel_AsyncResult_CB*            connectCb,
                                          void*                                   connectCbData)
{
    SOPC_IntChannel_CallbackData* internalCbData = NULL;
    return SOPC_IntChannel_BeginConnect(FALSE, channel, url, crt_cli,
                                        key_priv_cli, crt_srv,
                                        pki, reqSecuPolicyUri,
                                        requestedLifetime, msgSecurityMode,
                                        networkTimeout, cb, cbData,
                                        connectCb, connectCbData,
                                        &internalCbData);
}

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
                                     void*                                   cbData)
{
    SOPC_IntChannel_CallbackData* internalCbData = NULL;
    uint8_t receivedEvent = FALSE;
    const uint32_t sleepTimeout = 10;
    uint32_t timeout = networkTimeout;
    uint32_t loopCpt = 0;

    SOPC_StatusCode status = STATUS_NOK;
    SOPC_IntChannel_AsyncResultFlagData* asynResultData = SOPC_IntChannel_AsyncResultFlagData_Create();
    uint8_t freeAsyncDataHere = 1;

    if(NULL != asynResultData){
        status = SOPC_IntChannel_BeginConnect(1, // True for sync connect
                                              channel, url, crt_cli,
                                              key_priv_cli, crt_srv,
                                              pki, reqSecuPolicyUri,
                                              requestedLifetime, msgSecurityMode,
                                              networkTimeout, cb, cbData,
                                              SOPC_IntChannel_AsyncResultFlagCB,
                                              (void*) asynResultData,
                                              &internalCbData);
    }
    while (status == STATUS_OK &&
           receivedEvent == FALSE &&
           loopCpt * sleepTimeout <= timeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
        Mutex_Lock(&asynResultData->dataMutex);
        if(asynResultData->flag != FALSE){
            receivedEvent = 1; // True
            status = asynResultData->status;
        }
        if(loopCpt * sleepTimeout > timeout){
            // We will not free the asyncResult here since call will terminate
            asynResultData->freeInCallback = 1;
        }
        freeAsyncDataHere = asynResultData->freeInCallback == FALSE;
        Mutex_Unlock(&asynResultData->dataMutex);
    }

    if(loopCpt * sleepTimeout > timeout){
        status = OpcUa_BadTimeout;
    }

    if(FALSE != freeAsyncDataHere){
        SOPC_IntChannel_AsyncResultFlagData_Delete(asynResultData);
    }

    return status;
}

void SOPC_IntChannel_EndOperation_CB(void*           callbackData,
                                     SOPC_StatusCode status){
    assert(callbackData != NULL);
    SOPC_IntChannel_EndOperationData* operationEndData = (SOPC_IntChannel_EndOperationData*) callbackData;
    operationEndData->operationStatus = status;
    operationEndData->endOperationFlag = 1;
    Mutex_Unlock(&operationEndData->endOperationMutex);
}

SOPC_StatusCode SOPC_IntChannel_BeginInvokeService(SOPC_Channel                     channel,
                                                        char*                            debugName,
                                                        void*                            request,
                                                        SOPC_EncodeableType*             requestType,
                                                        SOPC_EncodeableType*             responseType,
                                                        SOPC_Channel_PfnRequestComplete* cb,// Must create an action if applicative callback to call
                                                        void*                            cbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    uint32_t timeout = 0;
    (void) debugName;
    SOPC_IntChannel_EndOperationData sentEndData;
    sentEndData.operationStatus = STATUS_NOK;
    sentEndData.channel = channel;
    sentEndData.endOperationFlag = FALSE;

    if(cConnection != NULL &&
       request != NULL && requestType != NULL &&
       cb != NULL)
    {
        if(responseType == NULL){
            // TODO: warning on efficiency ?
        }

        status = Mutex_Initialization(&sentEndData.endOperationMutex);
        if(STATUS_OK == status){
            status = Mutex_Lock(&sentEndData.endOperationMutex);
        }
        if(STATUS_OK == status){
            // There is always a request header as first struct field in a request (safe cast)
            timeout = ((OpcUa_RequestHeader*)request)->TimeoutHint;
            status = SC_CreateAction_Send_Request(cConnection,
                                                  requestType,
                                                  request,
                                                  responseType,
                                                  timeout,
                                                  (SC_ResponseEvent_CB*) cb,
                                                  cbData,
                                                  SOPC_IntChannel_EndOperation_CB,
                                                  (void*) &sentEndData);
        }
    }

    if(STATUS_OK == status){
        if(sentEndData.endOperationFlag == FALSE){
            Mutex_Lock(&sentEndData.endOperationMutex);
        }
        status = sentEndData.operationStatus;
    }
    Mutex_Unlock(&sentEndData.endOperationMutex);
    Mutex_Clear(&sentEndData.endOperationMutex);

    return status;
}


SOPC_StatusCode SOPC_Channel_BeginInvokeService(SOPC_Channel                     channel,
                                                char*                            debugName,
                                                void*                            request,
                                                SOPC_EncodeableType*             requestType,
                                                SOPC_EncodeableType*             responseType,
                                                SOPC_Channel_PfnRequestComplete* cb,
                                                void*                            cbData)
{
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_IntChannel_InvokeCallbackData* appCallbackData = SOPC_IntChannel_InvokeCallbackData_Create(channel,
                                                                    cb,
                                                                    cbData);
    if(NULL != appCallbackData){
        status = SOPC_IntChannel_BeginInvokeService(channel,
                                                         debugName,
                                                         request,
                                                         requestType,
                                                         responseType,
                                                         SOPC_Channel_CreateAction_AppInvokeCallback,
                                                         appCallbackData);
    }

    return status;
}

SOPC_StatusCode SOPC_IntChannel_InvokeRequestCompleteCallback(SOPC_Channel         channel,
                                                              void*                response,
                                                              SOPC_EncodeableType* responseType,
                                                              void*                cbData,
                                                              SOPC_StatusCode      status){
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    (void) channel;
    SOPC_IntChannel_InvokeCallbackData* invCbData = (SOPC_IntChannel_InvokeCallbackData*) cbData;
    if(invCbData != NULL){
        retStatus = STATUS_OK;
        SOPC_IntChannel_InvokeCallbackData_Set(invCbData,
                               response, responseType,
                               status);
    }
    return retStatus;
}

SOPC_StatusCode SOPC_Channel_InvokeService(SOPC_Channel          channel,
                                           char*                 debugName,
                                           void*                 request,
                                           SOPC_EncodeableType*  requestType,
                                           SOPC_EncodeableType*  expResponseType,
                                           void**                response,
                                           SOPC_EncodeableType** responseType){
    const uint32_t waitTimeoutMilliSecs = 1;
    uint32_t loopCptWait = 0;
    uint8_t receivedEvent = FALSE;
    SOPC_StatusCode localStatus = STATUS_NOK;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    uint32_t timeout = 0;
    SOPC_IntChannel_InvokeCallbackData* invCallbackData = SOPC_IntChannel_InvokeCallbackData_Create(channel,
                                                                    NULL, // No application callback here, only need receive "flag" to stop
                                                                    NULL);

    if(cConnection != NULL &&
       request != NULL && requestType != NULL &&
       response != NULL && responseType != NULL){
        if(invCallbackData != NULL){
            // There is always a request header as first struct field in a request (safe cast)
            timeout = ((OpcUa_RequestHeader*)request)->TimeoutHint;
            status = SOPC_IntChannel_BeginInvokeService(channel,
                                                             debugName,
                                                             request, requestType,
                                                             expResponseType,
                                                             SOPC_IntChannel_InvokeRequestCompleteCallback,
                                                             invCallbackData);
        }else{
            status = STATUS_NOK;
        }
    }

    while (status == STATUS_OK &&
           receivedEvent == FALSE &&
           loopCptWait * waitTimeoutMilliSecs <= timeout)
    {
        loopCptWait++;
        // TODO: time waited is not valid anymore if we receive other messages than expected !
        // Retrieve received messages on socket
        SOPC_Sleep(waitTimeoutMilliSecs);
        localStatus = SOPC_IntChannel_InvokeCallbackData_Get(invCallbackData,
                                             response,
                                             responseType);
        if(*response != NULL){
            receivedEvent = 1; // True
            status = localStatus;
        }
    }

    if(loopCptWait * waitTimeoutMilliSecs > timeout){
        status = OpcUa_BadTimeout;
    }

    SOPC_IntChannel_InvokeCallbackData_Delete(invCallbackData);

    return status;
}

void SOPC_Channel_Action_AsyncDisconnect(void* arg){
    assert(NULL != arg);
    SOPC_IntChannel_DisconnectData* discoData = (SOPC_IntChannel_DisconnectData*) arg;
    SOPC_IntChannel_AppChannelAsyncResultCbData* appCallbackData = NULL;
    SOPC_IntChannel_CallbackData_Delete(discoData->cConnection->callbackData);
    SC_Client_Disconnect(discoData->cConnection);
    SOPC_StackConfiguration_Unlocked();
    appCallbackData = SOPC_IntChannel_AppChannelAsyncResutlCbData_Create(discoData->disconnectCb,
                                                                         discoData->disconnectCbData,
                                                                         discoData->cConnection,
                                                                         SOPC_ChannelAsync_DisconnectResult,
                                                                         STATUS_OK);
    if(NULL != appCallbackData){
        // If asynchronous call to connect, we need to notify applicative layer of error
        // otherwise it is reported internally using the errorFlag
        SOPC_ActionQueueManager_AddAction(appCallbackQueueMgr,
                                          SOPC_Channel_Action_AppChannelAsyncResutlCallback,
                                          (void*) appCallbackData,
                                          "Channel async result event applicative callback");
    }
    free(discoData);
}

SOPC_StatusCode SOPC_Channel_AsyncDisconnect(SOPC_Channel                 channel,
                                             SOPC_Channel_AsyncResult_CB* disconnect,
                                             void*                        disconnectData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel_DisconnectData* discoData = NULL;
    // TODO: call the connection state change callback ? Or not necessary because voluntarily closed ?
    if(channel != NULL){
        discoData = malloc(sizeof(SOPC_IntChannel_DisconnectData));
        discoData->cConnection = channel;
        discoData->disconnectCb = disconnect;
        discoData->disconnectCbData = disconnectData;
        status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                   SOPC_Channel_Action_AsyncDisconnect,
                                                   (void*) discoData,
                                                   "Begin disconnect channel");
    }
    return status;
}

void SOPC_IntChannel_DisconnectResultCB(SOPC_Channel                           channel,
                                        void*                                  cbData,
                                        SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                        SOPC_StatusCode                        status)
{
    assert(cEvent == SOPC_ChannelAsync_DisconnectResult);
    (void) channel;
    SOPC_IntChannel_EndOperation_CB(cbData,
                                    status);
}

SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel_EndOperationData endDisconnectData;
    endDisconnectData.operationStatus = STATUS_NOK;
    endDisconnectData.channel = channel;
    endDisconnectData.endOperationFlag = FALSE;

    if(NULL != channel){

        status = Mutex_Initialization(&endDisconnectData.endOperationMutex);

        if(STATUS_OK == status){
            status = Mutex_Lock(&endDisconnectData.endOperationMutex);
        }

        if(STATUS_OK == status){
            SOPC_Channel_AsyncDisconnect(channel,
                                         SOPC_IntChannel_DisconnectResultCB,
                                         (void*) &endDisconnectData);
        }

        if(STATUS_OK == status && FALSE == endDisconnectData.endOperationFlag){
            status = Mutex_Lock(&endDisconnectData.endOperationMutex);
        }

        if(STATUS_OK == status){
            status = endDisconnectData.operationStatus;
        }

        Mutex_Unlock(&endDisconnectData.endOperationMutex);
        Mutex_Clear(&endDisconnectData.endOperationMutex);
    }
    return status;
}

#endif // CLIENT_API

