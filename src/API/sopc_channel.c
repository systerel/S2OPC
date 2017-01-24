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

#include <assert.h>
#include <stdlib.h>

typedef struct {
    SOPC_Channel_PfnConnectionStateChanged* callback;
    void*                                   callbackData;
    uint8_t                                 connectedFlag;
    uint8_t                                 disconnectedFlag;
} SOPC_InternalChannel_CallbackData;

SOPC_InternalChannel_CallbackData* SOPC_Create_ChannelCallbackData(SOPC_Channel_PfnConnectionStateChanged* callback,
                                                                   void*                                   callbackData)
{
    SOPC_InternalChannel_CallbackData* result = malloc(sizeof(SOPC_InternalChannel_CallbackData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
        result->connectedFlag = FALSE;
        result->disconnectedFlag = FALSE;
    }
    return result;
}

void SOPC_Delete_ChannelCallbackData(SOPC_InternalChannel_CallbackData* chCbData){
    if(chCbData != NULL){
        free(chCbData);
    }
}

// TODO: "atomic" callbackData for multithreading
typedef struct {
    void*                response;
    SOPC_EncodeableType* responseType;
    SOPC_StatusCode      status;
} InvokeCallbackData;

InvokeCallbackData* Create_InvokeCallbackData(){
    InvokeCallbackData* result = malloc(sizeof(InvokeCallbackData));
    result->response = NULL;
    result->responseType = NULL;
    result->status = STATUS_OK;
    return result;
}

void Set_InvokeCallbackData(InvokeCallbackData*  invCbData,
                            void*                response,
                            SOPC_EncodeableType* responseType,
                            SOPC_StatusCode      status)
{
    invCbData->response = response;
    invCbData->responseType = responseType;
    invCbData->status = status;
}

SOPC_StatusCode Get_InvokeCallbackData(InvokeCallbackData*   invCbData,
                                       void**                response,
                                       SOPC_EncodeableType** responseType){
    *response = invCbData->response;
    *responseType = invCbData->responseType;
    return invCbData->status;
}

void Delete_InvokeCallbackData(InvokeCallbackData* invCbData){
    if(invCbData != NULL){
        free(invCbData);
    }
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

SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = NULL;
    if(channel != NULL && *channel != NULL){
        // Ensure disconnect called for deallocation
        status = SOPC_Channel_Disconnect(*channel);
        cConnection = (SC_ClientConnection*) *channel;
        Delete_InvokeCallbackData(cConnection->callbackData);
        SC_Client_Delete(cConnection);
        *channel = NULL;
    }
    return status;
}

SOPC_StatusCode ChannelConnectionCB(SC_ClientConnection* cConnection,
                                    void*                cbData,
                                    SC_ConnectionEvent   event,
                                    SOPC_StatusCode      status){
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    SOPC_Channel channel = (SOPC_Channel) cConnection;
    SOPC_InternalChannel_CallbackData* callbackData = cbData;
    SOPC_Channel_Event channelConnectionEvent = SOPC_ChannelEvent_Invalid;

    switch(event){
        case SOPC_ConnectionEvent_Connected:
            channelConnectionEvent = SOPC_ChannelEvent_Connected;
            callbackData->connectedFlag = 1; // TRUE
            callbackData->disconnectedFlag = FALSE;
            break;
        case SOPC_ConnectionEvent_Disconnected:
            channelConnectionEvent = SOPC_ChannelEvent_Disconnected;
            callbackData->connectedFlag = FALSE;
            callbackData->disconnectedFlag = 1; // TRUE
            break;
        case SOPC_ConnectionEvent_SecureMessageComplete:
        case SOPC_ConnectionEvent_SecureMessageChunk:
        case SOPC_ConnectionEvent_SecureMessageAbort:
            // TODO: traces ?
            break;
        case SOPC_ConnectionEvent_Invalid:
        case SOPC_ConnectionEvent_UnexpectedError:
            channelConnectionEvent = SOPC_ChannelEvent_Disconnected;
            callbackData->connectedFlag = FALSE;
            callbackData->disconnectedFlag = 1; // TRUE
            break;
    }

    // Channel event management
    switch(channelConnectionEvent){
        case SOPC_ChannelEvent_Invalid:
            // Nothing to do
            retStatus = STATUS_OK;
            break;
        case SOPC_ChannelEvent_Connected:
        case SOPC_ChannelEvent_Disconnected:
            if(callbackData != NULL && callbackData->callback != NULL)
            {
                retStatus = callbackData->callback(channel,
                                                   callbackData->callbackData,
                                                   channelConnectionEvent,
#ifdef STACK_1_02
                                                   NULL,
#endif
                                                   status);
            }
    }
    return retStatus;
}

SOPC_StatusCode SOPC_Channel_InternalBeginConnect(SOPC_Channel                            channel,
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
                                                  SOPC_InternalChannel_CallbackData**                  channelCbData)
{
    assert(channelCbData != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
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
        if(cConnection->instance->state != SC_Connection_Disconnected){
            status = STATUS_INVALID_STATE;
        }else{
            StackConfiguration_Locked();
            status = SC_Client_Configure(cConnection,
                                         StackConfiguration_GetNamespaces(),
                                         StackConfiguration_GetEncodeableTypes());
            if(status == STATUS_OK){
                *channelCbData = SOPC_Create_ChannelCallbackData(cb, cbData);
                if(*channelCbData == NULL){
                    status = STATUS_NOK;
                }
            }
            if(status == STATUS_OK){
                status = SC_Client_Connect(cConnection, url,
                                           pki,
                                           crt_cli, key_priv_cli,
                                           crt_srv,
                                           msgSecurityMode,
                                           reqSecuPolicyUri,
                                           requestedLifetime,
                                           ChannelConnectionCB, *channelCbData);
            }
        }
    }
    return status;
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
    SOPC_InternalChannel_CallbackData* internalCbData = NULL;
    return SOPC_Channel_InternalBeginConnect(channel, url, crt_cli,
                                             key_priv_cli, crt_srv,
                                             pki, reqSecuPolicyUri,
                                             requestedLifetime, msgSecurityMode,
                                             networkTimeout, cb, cbData, &internalCbData);
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
    SOPC_InternalChannel_CallbackData* internalCbData = NULL;
    uint8_t receivedEvent = FALSE;
    const uint32_t sleepTimeout = 500;
    uint32_t timeout = networkTimeout;
    uint32_t loopCpt = 0;
    SOPC_StatusCode status = SOPC_Channel_InternalBeginConnect(channel, url, crt_cli,
                                                               key_priv_cli, crt_srv,
                                                               pki, reqSecuPolicyUri,
                                                               requestedLifetime, msgSecurityMode,
                                                               networkTimeout, cb, cbData,
                                                               &internalCbData);
    while (status == STATUS_OK &&
           receivedEvent == FALSE &&
           loopCpt * sleepTimeout <= timeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
        // just wait for callback called
        //Sleep (sleepTimeout);
        return OpcUa_BadNotImplemented;
#else
        // TODO: will retrieve any message: is it a problem ?
        // Retrieve received messages on socket
        status = SOPC_SocketManager_Loop (SOPC_SocketManager_GetGlobal(),
                                          sleepTimeout);
#endif //OPCUA_MULTITHREADED
        if(internalCbData->connectedFlag != FALSE){
            receivedEvent = 1; // True
        }else if(internalCbData->disconnectedFlag != FALSE){
            receivedEvent = 1; // True
            status = STATUS_NOK; // Connection failed
        }
    }

    if(loopCpt * sleepTimeout > timeout){
        status = OpcUa_BadTimeout;
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
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    uint32_t timeout = 0;
    (void) debugName;

    if(cConnection != NULL &&
       request != NULL && requestType != NULL &&
       cb != NULL)
    {
        if(responseType == NULL){
            // TODO: warning on efficiency ?
        }

        // There is always a request header as first struct field in a request (safe cast)
        timeout = ((OpcUa_RequestHeader*)request)->TimeoutHint;
        status = SC_Send_Request(cConnection,
                                 requestType,
                                 request,
                                 responseType,
                                 timeout,
                                 (SC_ResponseEvent_CB*) cb,
                                 cbData);
    }
    return status;
}

SOPC_StatusCode InvokeRequestCompleteCallback(SOPC_Channel         channel,
                                              void*                response,
                                              SOPC_EncodeableType* responseType,
                                              void*                cbData,
                                              SOPC_StatusCode      status){
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    (void) channel;
    InvokeCallbackData* invCbData = (InvokeCallbackData*) cbData;
    if(invCbData != NULL){
        retStatus = STATUS_OK;
        Set_InvokeCallbackData(invCbData,
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
    const uint32_t waitTimeoutMilliSecs = 500;
    const uint32_t sleepTimeoutMicroSecs = 500;
    const uint32_t sleepFactor = (waitTimeoutMilliSecs * 1000) / sleepTimeoutMicroSecs;
    uint32_t loopCptWait = 0;
    uint32_t loopCptSleep = 0;
    uint8_t receivedEvent = FALSE;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    uint32_t timeout = 0;
    InvokeCallbackData* invCallbackData = Create_InvokeCallbackData();

    if(cConnection != NULL &&
       request != NULL && requestType != NULL &&
       response != NULL && responseType != NULL){
        if(invCallbackData != NULL){
            // There is always a request header as first struct field in a request (safe cast)
            timeout = ((OpcUa_RequestHeader*)request)->TimeoutHint;
            status = SOPC_Channel_BeginInvokeService(channel,
                                                     debugName,
                                                     request, requestType,
                                                     expResponseType,
                                                     InvokeRequestCompleteCallback,
                                                     invCallbackData);
        }else{
            status = STATUS_NOK;
        }
    }

    while (status == STATUS_OK &&
           receivedEvent == FALSE &&
           loopCptWait * waitTimeoutMilliSecs <= timeout)
    {
#if OPCUA_MULTITHREADED || WRAPPER_RECEPTION_THREAD
        // just wait for callback called
        loopCptSleep++;
        if((loopCptSleep % sleepFactor) == 0){
            loopCptWait++;
        }
        SOPC_Sleep (sleepTimeoutMicroSecs);
#else
        (void) loopCptSleep;
        (void) sleepFactor;
        loopCptWait++;
        // TODO: will retrieve any message: is it a problem ?
        // TODO: time waited is not valid anymore if we receive other messages than expected !
        // Retrieve received messages on socket
        status = SOPC_SocketManager_Loop (SOPC_SocketManager_GetGlobal(),
                                          waitTimeoutMilliSecs);
#endif //OPCUA_MULTITHREADED
        status = Get_InvokeCallbackData(invCallbackData,
                                        response,
                                        responseType);
        if(*response != NULL){
            receivedEvent = 1; // True
        }
    }

    if(loopCptWait * waitTimeoutMilliSecs > timeout){
        status = OpcUa_BadTimeout;
    }

    Delete_InvokeCallbackData(invCallbackData);

    return status;
}

SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    // TODO: call the connection state change callback ? Or not necessary because voluntarily closed ?
    if(cConnection != NULL){
        status = STATUS_NOK;
        SOPC_Delete_ChannelCallbackData(cConnection->callbackData);
        status = SC_Client_Disconnect(cConnection);
        StackConfiguration_Unlocked();
    }
    return status;
}

#endif // CLIENT_API

