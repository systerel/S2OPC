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
#include <string.h>

typedef struct SOPC_IntChannel {
    // Connection instance
    SC_ClientConnection*  cConnection;

    // Configuration to store on connection event for message encoding
    uint32_t              secureChannelId;
    uint32_t              maxChunksSendingCfg;
    uint32_t              maxMsgSizeSendingCfg;
    uint32_t              bufferSizeSendingCfg;
    SOPC_NamespaceTable*  nsTableCfg;
    SOPC_EncodeableType** encTypesTableCfg;
} SOPC_IntChannel;

typedef struct SOPC_IntChannel_CallbackData {
    SOPC_Channel_PfnConnectionStateChanged* callback;
    void*                                   callbackData;
    SOPC_IntChannel*                        channel;
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
    SOPC_IntChannel*                        channel;
    SOPC_Channel_Event                      event;
    SOPC_StatusCode                         status;
} SOPC_IntChannel_AppChannelEventCbData;

typedef struct SOPC_IntChannel_AppChannelAsyncResultCbData {
    SOPC_Channel_AsyncResult_CB*           callback;
    void*                                  callbackData;
    SOPC_IntChannel*                       channel;
    SOPC_ChannelEvent_AsyncOperationResult event;
    SOPC_StatusCode                        status;
} SOPC_IntChannel_AppChannelAsyncResultCbData;

// Could be used as callbackData in SOPC_IntChannel_AppChannelAsyncResultCbData
// when using a (timed) condition waiting on async result is needed
typedef struct SOPC_IntChannel_AsyncResultCondData {
    Mutex           mutex;
    Condition       cond;
    uint8_t         flag;
    uint8_t         freeInCallback;
    SOPC_StatusCode status;
} SOPC_IntChannel_AsyncResultCondData;

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
    SOPC_IntChannel*              intChannel;
    SOPC_Channel_AsyncResult_CB*  disconnectCb;
    void*                         disconnectCbData;
} SOPC_IntChannel_DisconnectData;

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

SOPC_StatusCode SOPC_IntChannel_AsyncResultCondData_Init(SOPC_IntChannel_AsyncResultCondData* asyncData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != asyncData){
        asyncData->flag = FALSE;
        asyncData->freeInCallback = FALSE;
        asyncData->status = STATUS_NOK;
        status = Mutex_Initialization(&asyncData->mutex);
        if(STATUS_OK == status){
            status = Condition_Init(&asyncData->cond);
        }
    }
    return status;
}

SOPC_IntChannel_AsyncResultCondData* SOPC_IntChannel_AsyncResultCondData_Create()
{
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_IntChannel_AsyncResultCondData* result = malloc(sizeof(SOPC_IntChannel_AsyncResultCondData));
    if(NULL != result){
        status = SOPC_IntChannel_AsyncResultCondData_Init(result);
        if(STATUS_OK != status){
            free(result);
            result = NULL;
        }
    }
    return result;
}

void SOPC_IntChannel_AsyncResultCondData_Clear(SOPC_IntChannel_AsyncResultCondData* data){
    Mutex_Clear(&data->mutex);
    Condition_Clear(&data->cond);
}


void SOPC_IntChannel_AsyncResultCondData_Delete(SOPC_IntChannel_AsyncResultCondData* data)
{
    if(NULL != data){
        SOPC_IntChannel_AsyncResultCondData_Clear(data);
        free(data);
    }
}

// Manage the async result flag for notifying the sync function waiting for it
void SOPC_IntChannel_AsyncResultCondDataCB(SOPC_Channel                           channel,
                                           void*                                  cbData,
                                           SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                           SOPC_StatusCode                        status)
{
    (void) cEvent;
    (void) channel;
    uint8_t freeDataHere = FALSE;
    SOPC_IntChannel_AsyncResultCondData* asynResultData = (SOPC_IntChannel_AsyncResultCondData*) cbData;
    Mutex_Lock(&asynResultData->mutex);
    asynResultData->flag = 1;
    asynResultData->status = status;
    freeDataHere = asynResultData->freeInCallback;
    Condition_SignalAll(&asynResultData->cond);
    Mutex_Unlock(&asynResultData->mutex);

    if(FALSE != freeDataHere){
        // Sync function using the flag has stopped before callback
        // data must be freed here
        SOPC_IntChannel_AsyncResultCondData_Delete(asynResultData);
    }
}

SOPC_StatusCode SOPC_IntChannel_CreateAction_AppChannelAsyncResult(void*                asyncResultCBdata,
                                                                   const char*          actionTxt)
{
    assert(NULL != asyncResultCBdata);
    SOPC_IntChannel_AppChannelAsyncResultCbData* cbData = (SOPC_IntChannel_AppChannelAsyncResultCbData*) asyncResultCBdata;
    SOPC_ActionQueueManager* qManager = NULL;
    // Depending on the fact if the final callback is internal and responsible
    //  to signal end of sync operation we use the "application callback queue manager" or
    //  the "stack action queue manager".
    // It is necessary since a sync call could be done through the "application callback queue manager"
    //  and will provoke a dead lock if we enqueue the condition signal callback in the same queue.
    if(SOPC_IntChannel_AsyncResultCondDataCB == cbData->callback){ // SOPC_IntChannel_AsyncResultCondDataCB unique function to manage condition signal
        qManager = stackActionQueueMgr;
    }else{
        qManager = appCallbackQueueMgr;
    }
    return SOPC_ActionQueueManager_AddAction(qManager,
                                             SOPC_Channel_Action_AppChannelAsyncResutlCallback,
                                             asyncResultCBdata,
                                             actionTxt);
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

SOPC_IntChannel* SOPC_IntChannel_Create(){
    SOPC_IntChannel* result = malloc(sizeof(SOPC_IntChannel));
    if(NULL != result){
        memset(result, 0, sizeof(SOPC_IntChannel));
    }
    return result;
}

SOPC_StatusCode SOPC_Channel_Create(SOPC_Channel*               channel,
                                    SOPC_Channel_SerializerType serialType){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel* intChannel = NULL;
    if(channel != NULL && serialType == SOPC_ChannelSerializer_Binary){
        intChannel = SOPC_IntChannel_Create();
        if(intChannel != NULL){
            intChannel->cConnection = SC_Client_Create();
            if(NULL != intChannel->cConnection){
                *channel = intChannel;
                status = STATUS_OK;
            }else{
                free(intChannel);
                status = STATUS_NOK;
            }
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

void SOPC_IntChannel_AsyncDeleteDiscoResultCB(SOPC_Channel                           channel,
                                              void*                                  cbData,
                                              SOPC_ChannelEvent_AsyncOperationResult cEvent,
                                              SOPC_StatusCode                        status)
{
    SOPC_IntChannel* intChannel = NULL;
    assert(cEvent == SOPC_ChannelAsync_DisconnectResult);
    (void) cbData;
    if(STATUS_OK == status){
        intChannel = (SOPC_IntChannel*) channel;
        SC_Client_Delete(intChannel->cConnection);
        free(intChannel);
    }
}

SOPC_StatusCode SOPC_Channel_AsyncDelete(SOPC_Channel channel){
    return SOPC_Channel_AsyncDisconnect(channel,
                                        SOPC_IntChannel_AsyncDeleteDiscoResultCB,
                                        NULL);
}

SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel* intChannel = NULL;
    if(channel != NULL && *channel != NULL){
        intChannel = (SOPC_IntChannel*) *channel;
        // Ensure disconnect called for deallocation
        status = SOPC_Channel_Disconnect(*channel);
        if(STATUS_OK == status || STATUS_INVALID_STATE == status){
            SC_Client_Delete(intChannel->cConnection);
            free(intChannel);
            // No need to wait delete action terminated to set pointer to null
            *channel = NULL;
        }
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
    SOPC_IntChannel* intChannel = (SOPC_IntChannel*) callbackData->channel;

    switch(event){
        case SOPC_ConnectionEvent_Connected:
            channelConnectionEvent = SOPC_ChannelEvent_Connected;
            intChannel->secureChannelId = cConnection->instance->secureChannelId;
            intChannel->maxChunksSendingCfg = cConnection->instance->transportConnection->maxChunkCountSnd;
            intChannel->maxMsgSizeSendingCfg = cConnection->instance->transportConnection->maxMessageSizeSnd;
            intChannel->bufferSizeSendingCfg = cConnection->instance->transportConnection->sendBufferSize;
            intChannel->nsTableCfg = &cConnection->namespaces;
            intChannel->encTypesTableCfg = cConnection->encodeableTypes;
            // TODO: retrieve msgBuffer parameters
            break;
        case SOPC_ConnectionEvent_Disconnected:
        case SOPC_ConnectionEvent_ConnectionFailed:
            channelConnectionEvent = SOPC_ChannelEvent_Disconnected;
            intChannel->secureChannelId = 0;
            intChannel->maxChunksSendingCfg = 0;
            intChannel->maxMsgSizeSendingCfg = 0;
            intChannel->bufferSizeSendingCfg = 0;
            intChannel->nsTableCfg = NULL;
            intChannel->encTypesTableCfg = NULL;
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
                                                                      "Channel event applicative callback: connected");
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
                    SOPC_IntChannel_CreateAction_AppChannelAsyncResult((void*) asyncCbData,
                                                                       "Channel connected async result event applicative callback");
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
                    SOPC_IntChannel_CreateAction_AppChannelAsyncResult((void*) asyncCbData,
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
                                                                      "Channel event applicative callback: disconnected");
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
            SOPC_IntChannel_CreateAction_AppChannelAsyncResult((void*) appCallbackData,
                                                               "Channel begin connect async result event applicative callback");
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
    SOPC_IntChannel* intChannel = (SOPC_IntChannel*) channel;
    SOPC_IntChannel_ConnectData* connectData = NULL;
    (void) networkTimeout;

    if(intChannel != NULL && intChannel->cConnection != NULL &&
       intChannel->cConnection->instance != NULL &&
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
            connectData->cConnection = intChannel->cConnection;
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
    SOPC_IntChannel* intChannel = (SOPC_IntChannel*) channel;
    SOPC_IntChannel_CallbackData* icbData = (SOPC_IntChannel_CallbackData*) intChannel->cConnection->callbackData;
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
    uint32_t timeout = networkTimeout;

    SOPC_StatusCode status = STATUS_NOK;
    SOPC_IntChannel_AsyncResultCondData* asynResultData = SOPC_IntChannel_AsyncResultCondData_Create();
    uint8_t freeAsyncDataHere = 1;

    if(NULL != asynResultData){
        status = SOPC_IntChannel_BeginConnect(1, // True for sync connect
                                              channel, url, crt_cli,
                                              key_priv_cli, crt_srv,
                                              pki, reqSecuPolicyUri,
                                              requestedLifetime, msgSecurityMode,
                                              networkTimeout, cb, cbData,
                                              SOPC_IntChannel_AsyncResultCondDataCB,
                                              (void*) asynResultData,
                                              &internalCbData);
    }

    Mutex_Lock(&asynResultData->mutex);
    if(asynResultData->flag == FALSE){
        Mutex_UnlockAndTimedWaitCond(&asynResultData->cond, &asynResultData->mutex, timeout);
    }
    if(asynResultData->flag == FALSE){
        status = OpcUa_BadTimeout;
        // We will not free the asyncResult here since call will terminate
        asynResultData->freeInCallback = 1;
    }else{
        status = asynResultData->status;
    }
    freeAsyncDataHere = asynResultData->freeInCallback == FALSE;
    Mutex_Unlock(&asynResultData->mutex);

    if(FALSE != freeAsyncDataHere){
        SOPC_IntChannel_AsyncResultCondData_Delete(asynResultData);
    }

    return status;
}

void SOPC_IntChannel_InvokeSendRequestResultCB(void*           callbackData,
                                               SOPC_StatusCode status)
{
    assert(callbackData != NULL);
    SOPC_IntChannel_AppChannelAsyncResultCbData* operationEndData = (SOPC_IntChannel_AppChannelAsyncResultCbData*) callbackData;
    operationEndData->status = status;
    SOPC_IntChannel_CreateAction_AppChannelAsyncResult((void*) callbackData,
                                                       "Channel async invoke send request result event applicative callback");
}

SOPC_StatusCode SOPC_Channel_AsyncInvokeService(SOPC_Channel                     channel,
                                                void*                            request,
                                                SOPC_EncodeableType*             requestType,
                                                SOPC_EncodeableType*             responseType,
                                                SOPC_Channel_PfnRequestComplete* cb,// Must create an action if applicative callback to call
                                                void*                            cbData,
                                                SOPC_Channel_AsyncResult_CB*     sendReqResultCb,
                                                void*                            sendReqResultCbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel* intChannel = (SOPC_IntChannel*) channel;
    uint32_t timeout = 0;
    SOPC_IntChannel_InvokeCallbackData* appCallbackData = NULL;
    SOPC_IntChannel_AppChannelAsyncResultCbData* appSendReqResultCbData = NULL;
    SOPC_Socket_EndOperation_CB* endSendCallback = NULL;
    SOPC_MsgBuffers* msgBuffers = NULL;

    if(intChannel != NULL && intChannel->cConnection != NULL &&
       request != NULL && // requestType != NULL &&
       cb != NULL)
    {
        if(responseType == NULL){
            // TODO: warning on efficiency ?
        }

        appCallbackData = SOPC_IntChannel_InvokeCallbackData_Create(channel,
                                                                    cb,
                                                                    cbData);

        // TODO: do not use end send request callback at this level
        //       anymore since error will be triggered in SOPC_Channel_PfnRequestComplete callback
        // => wait for encoding to be done by applicative code
        if(NULL != sendReqResultCb){
            appSendReqResultCbData = SOPC_IntChannel_AppChannelAsyncResutlCbData_Create(sendReqResultCb,
                                                                                        sendReqResultCbData,
                                                                                        channel,
                                                                                        SOPC_ChannelAsync_InvokeSendRequestResult,
                                                                                        STATUS_NOK);
            if(NULL != appSendReqResultCbData){
                endSendCallback = SOPC_IntChannel_InvokeSendRequestResultCB;
            }
        }

        if(NULL != appCallbackData && (NULL != appSendReqResultCbData || NULL == sendReqResultCb)){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
            if(NULL != appCallbackData){
                SOPC_IntChannel_InvokeCallbackData_Delete(appCallbackData);
                appCallbackData = NULL;
            }
            if(NULL != appSendReqResultCbData){
                SOPC_IntChannel_AppChannelAsyncResutlCbData_Delete(appSendReqResultCbData);
                appSendReqResultCbData = NULL;
            }
        }

        if(STATUS_OK == status){
            // There is always a request header as first struct field in a request (safe cast)
            //timeout = ((OpcUa_RequestHeader*)request)->TimeoutHint;

            // Note: request message limited to 1 chunk maximum for this intermediary version
            msgBuffers = MsgBuffer_Create((SOPC_Buffer*) request,
                                          1,
                                          intChannel->cConnection->instance,
                                          intChannel->nsTableCfg,
                                          intChannel->encTypesTableCfg);

            if(NULL != msgBuffers){
                status = SC_Client_EncodeRequest(intChannel->secureChannelId,
                                                 requestType,
                                                 request,
                                                 msgBuffers);
            }else{
                status = STATUS_NOK;
            }

            if(STATUS_OK == status){
                // TODO: do not use end send request callback at this level
                //       anymore since error will be triggered in SOPC_Channel_PfnRequestComplete callback
                status = SC_CreateAction_Send_Request(intChannel->cConnection,
                        requestType,
                        msgBuffers,
                        responseType,
                        timeout,
                        (SC_ResponseEvent_CB*) SOPC_Channel_CreateAction_AppInvokeCallback,
                        appCallbackData,
                        endSendCallback,
                        (void*) appSendReqResultCbData);
            }
        }
    }

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
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel_AsyncResultCondData asyncData;
    (void) debugName;

    if(NULL != channel){
        status = SOPC_IntChannel_AsyncResultCondData_Init(&asyncData);

        if(STATUS_OK == status){
            status = SOPC_Channel_AsyncInvokeService(channel,
                                                     request,
                                                     requestType,
                                                     responseType,
                                                     cb,
                                                     cbData,
                                                     SOPC_IntChannel_AsyncResultCondDataCB,
                                                     (void*) &asyncData);
        }

        if(STATUS_OK == status){
            status = Mutex_Lock(&asyncData.mutex);

            while(STATUS_OK == status && asyncData.flag == FALSE){
                status = Mutex_UnlockAndWaitCond(&asyncData.cond, &asyncData.mutex);
            }

            if(STATUS_OK == status){
                status = asyncData.status;
            }
            Mutex_Unlock(&asyncData.mutex);
        }

        SOPC_IntChannel_AsyncResultCondData_Clear(&asyncData);
    }

    return status;
}

SOPC_StatusCode SOPC_IntChannel_InvokeResponseCallback(SOPC_Channel         channel,
                                                       void*                response,
                                                       SOPC_EncodeableType* responseType,
                                                       void*                cbData,
                                                       SOPC_StatusCode      status){
    assert(NULL != cbData);
    (void) channel;
    uint8_t freeDataHere = FALSE;
    SOPC_IntChannel_InvokeCallbackData* invCbData = (SOPC_IntChannel_InvokeCallbackData*) cbData;
    SOPC_IntChannel_AsyncResultCondData* asynResultData = (SOPC_IntChannel_AsyncResultCondData*) invCbData->cbData;
    assert(NULL != asynResultData);

    Mutex_Lock(&asynResultData->mutex);
    // Set results in invoke callback data in order to be retrieved by Invoke sync function
    SOPC_IntChannel_InvokeCallbackData_Set(invCbData,
                                           response, responseType,
                                           status);
    asynResultData->flag = 1;
    asynResultData->status = status;
    freeDataHere = asynResultData->freeInCallback;
    Condition_SignalAll(&asynResultData->cond);
    Mutex_Unlock(&asynResultData->mutex);

    if(FALSE != freeDataHere){
        // Sync function using the flag has stopped before callback
        // data must be freed here
        SOPC_IntChannel_AsyncResultCondData_Delete(asynResultData);
        SOPC_IntChannel_InvokeCallbackData_Delete(invCbData);
    }
    return STATUS_OK; // not used by caller
}

SOPC_StatusCode SOPC_Channel_InvokeService(SOPC_Channel          channel,
                                           char*                 debugName,
                                           void*                 request,
                                           SOPC_EncodeableType*  requestType,
                                           SOPC_EncodeableType*  expResponseType,
                                           void**                response,
                                           SOPC_EncodeableType** responseType){
    (void) debugName;
    uint8_t freeAsyncDataHere = FALSE;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_StatusCode responseStatus = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel* intChannel = (SOPC_IntChannel*) channel;
    uint32_t timeout = 0;
    SOPC_IntChannel_AsyncResultCondData* asyncResultCond = NULL;
    SOPC_IntChannel_InvokeCallbackData* asyncResultContainer = NULL;

    if(intChannel != NULL && intChannel->cConnection != NULL &&
       request != NULL && requestType != NULL &&
       response != NULL && responseType != NULL){

        asyncResultCond = SOPC_IntChannel_AsyncResultCondData_Create();
        asyncResultContainer = SOPC_IntChannel_InvokeCallbackData_Create(channel,
                                                                         NULL,
                                                                         asyncResultCond);

        if(NULL != asyncResultCond && NULL != asyncResultContainer){
            // TODO: manage timeout in stack only (pending request timeout) and not in sync call
            //       => use a Cond instead of TimedCond once done
            // There is always a request header as first struct field in a request (safe cast)
            timeout = ((OpcUa_RequestHeader*)request)->TimeoutHint;

            status = SOPC_Channel_AsyncInvokeService
                        (channel,
                         request,
                         requestType,
                         expResponseType,
                         SOPC_IntChannel_InvokeResponseCallback, // will be called in any case of error (no need of async result function)
                         asyncResultContainer,
                         NULL,
                         NULL);
        }else{
            status = STATUS_NOK;
            if(NULL != asyncResultCond){
                SOPC_IntChannel_AsyncResultCondData_Delete(asyncResultCond);
            }
            if(NULL != asyncResultContainer){
                SOPC_IntChannel_InvokeCallbackData_Delete(asyncResultContainer);
            }
        }

        if(STATUS_OK == status){
            Mutex_Lock(&asyncResultCond->mutex);
            if(asyncResultCond->flag == FALSE){
                Mutex_UnlockAndTimedWaitCond(&asyncResultCond->cond, &asyncResultCond->mutex, timeout);
            }
            if(asyncResultCond->flag == FALSE){
                status = OpcUa_BadTimeout;
                // We will not free the asyncResult here since call will terminate
                asyncResultCond->freeInCallback = 1;
            }else{
                status = asyncResultCond->status;
                responseStatus = SOPC_IntChannel_InvokeCallbackData_Get(asyncResultContainer,
                                                                        response, responseType);
                assert(status == responseStatus);
            }
            freeAsyncDataHere = asyncResultCond->freeInCallback == FALSE;
            Mutex_Unlock(&asyncResultCond->mutex);

            if(FALSE != freeAsyncDataHere){
                SOPC_IntChannel_AsyncResultCondData_Delete(asyncResultCond);
                SOPC_IntChannel_InvokeCallbackData_Delete(asyncResultContainer);
            }
        }
    }

    return status;
}

void SOPC_Channel_Action_AsyncDisconnect(void* arg){
    assert(NULL != arg);
    SOPC_IntChannel_DisconnectData* discoData = (SOPC_IntChannel_DisconnectData*) arg;
    SOPC_IntChannel_AppChannelAsyncResultCbData* appCallbackData = NULL;
    SOPC_IntChannel_CallbackData_Delete(discoData->intChannel->cConnection->callbackData);
    SC_Client_Disconnect(discoData->intChannel->cConnection);
    SOPC_StackConfiguration_Unlocked();
    appCallbackData = SOPC_IntChannel_AppChannelAsyncResutlCbData_Create(discoData->disconnectCb,
                                                                         discoData->disconnectCbData,
                                                                         discoData->intChannel,
                                                                         SOPC_ChannelAsync_DisconnectResult,
                                                                         STATUS_OK);
    if(NULL != appCallbackData){
        // If asynchronous call to connect, we need to notify applicative layer of error
        // otherwise it is reported internally using the errorFlag
        SOPC_IntChannel_CreateAction_AppChannelAsyncResult((void*) appCallbackData,
                                                           "Channel disconnect async result event applicative callback");
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
        discoData->intChannel = (SOPC_IntChannel*) channel;
        discoData->disconnectCb = disconnect;
        discoData->disconnectCbData = disconnectData;
        status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                   SOPC_Channel_Action_AsyncDisconnect,
                                                   (void*) discoData,
                                                   "Begin disconnect channel");
    }
    return status;
}

SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_IntChannel_AsyncResultCondData asyncData;

    if(NULL != channel){
        status = SOPC_IntChannel_AsyncResultCondData_Init(&asyncData);

        if(STATUS_OK == status){
            status = SOPC_Channel_AsyncDisconnect(channel,
                                                  SOPC_IntChannel_AsyncResultCondDataCB,
                                                  (void*) &asyncData);
        }

        if(STATUS_OK == status){
            status = Mutex_Lock(&asyncData.mutex);

            while(STATUS_OK == status && asyncData.flag == FALSE){
                status = Mutex_UnlockAndWaitCond(&asyncData.cond, &asyncData.mutex);
            }

            if(STATUS_OK == status){
                status = asyncData.status;
            }
            Mutex_Unlock(&asyncData.mutex);
        }

        SOPC_IntChannel_AsyncResultCondData_Clear(&asyncData);
    }
    return status;
}

void* SOPC_Channel_GetConnection(SOPC_Channel channel){
    return (void*) ((SOPC_IntChannel*) channel)->cConnection;
}

#endif // CLIENT_API

