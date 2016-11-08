/*
 * ua_channel.c
 *
 *  Created on: Sep 29, 2016
 *      Author: vincent
 */

#include <ua_channel.h>

#ifdef OPCUA_HAVE_CLIENTAPI

#include <opcua_statuscodes.h>

#include <ua_builtintypes.h>
#include <ua_secure_channel_client_connection.h>
#include <ua_stack_config.h>

#include <stdlib.h>

typedef struct {
    SOPC_Channel_PfnConnectionStateChanged* callback;
    void*                                 callbackData;
} Channel_CallbackData;

Channel_CallbackData* Create_CallbackData(SOPC_Channel_PfnConnectionStateChanged* callback,
                                          void*                                 callbackData)
{
    Channel_CallbackData* result = malloc(sizeof(Channel_CallbackData));
    if(result != NULL){
        result->callback = callback;
        result->callbackData = callbackData;
    }
    return result;
}

void Delete_CallbackData(Channel_CallbackData* chCbData){
    free(chCbData);
}

// TODO: "atomic" callbackData for multithreading
typedef struct {
    void*              response;
    SOPC_EncodeableType* responseType;
    SOPC_StatusCode         status;
} InvokeCallbackData;

InvokeCallbackData* Create_InvokeCallbackData(){
    InvokeCallbackData* result = malloc(sizeof(InvokeCallbackData));
    result->response = NULL;
    result->responseType = NULL;
    result->status = STATUS_OK;
    return result;
}

void Set_InvokeCallbackData(InvokeCallbackData* invCbData,
                            void*               response,
                            SOPC_EncodeableType*  responseType,
                            SOPC_StatusCode          status)
{
    invCbData->response = response;
    invCbData->responseType = responseType;
    invCbData->status = status;
}

SOPC_StatusCode Get_InvokeCallbackData(InvokeCallbackData* invCbData,
                                  void**              response,
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
    if(channel != NULL && serialType == ChannelSerializer_Binary){
        *channel = SC_Client_Create();
        if(channel != NULL){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Channel_Clear(SOPC_Channel channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    // Ensure disconnect called for deallocation
    status = SOPC_Channel_Disconnect(channel);
    Delete_InvokeCallbackData(cConnection->callbackData);
    SC_Client_Delete(cConnection);
    return status;
}

SOPC_StatusCode SOPC_Channel_Delete(SOPC_Channel* channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != NULL){
        status = SOPC_Channel_Clear(*channel);
    }
    return status;
}

SOPC_StatusCode ChannelConnectionCB(SC_ClientConnection* cConnection,
                               void*                cbData,
                               SC_ConnectionEvent   event,
                               SOPC_StatusCode           status){
    SOPC_StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    SOPC_Channel channel = (SOPC_Channel) cConnection;
    Channel_CallbackData* callbackData = cbData;
    SOPC_Channel_Event channelConnectionEvent = ChannelEvent_Invalid;

    switch(event){
        case SOPC_ConnectionEvent_Connected:
            channelConnectionEvent = ChannelEvent_Connected;
            break;
        case SOPC_ConnectionEvent_Disconnected:
            channelConnectionEvent = ChannelEvent_Disconnected;
            break;
        case SOPC_ConnectionEvent_SecureMessageComplete:
        case SOPC_ConnectionEvent_SecureMessageChunk:
        case SOPC_ConnectionEvent_SecureMessageAbort:
            // TODO: traces ?
            break;
        case SOPC_ConnectionEvent_Invalid:
        case SOPC_ConnectionEvent_UnexpectedError:
            channelConnectionEvent = ChannelEvent_Disconnected;
            break;
    }

    // Channel event management
    switch(channelConnectionEvent){
        case ChannelEvent_Invalid:
            // Nothing to do
            retStatus = STATUS_OK;
            break;
        case ChannelEvent_Connected:
        case ChannelEvent_Disconnected:
            if(callbackData != NULL && callbackData->callback != NULL)
            {
                retStatus = callbackData->callback(channel,
                                                   callbackData->callbackData,
                                                   channelConnectionEvent,
                                                   status);
            }
    }
    return retStatus;
}

SOPC_StatusCode SOPC_Channel_BeginConnect(SOPC_Channel                            channel,
                                   const char*                           url,
                                   const Certificate*                    crt_cli,
                                   const AsymmetricKey*                  key_priv_cli,
                                   const Certificate*                    crt_srv,
                                   const PKIProvider*                    pki,
                                   const char*                           reqSecuPolicyUri,
                                   int32_t                               requestedLifetime,
                                   OpcUa_MessageSecurityMode             msgSecurityMode,
                                   uint32_t                              networkTimeout,
                                   SOPC_Channel_PfnConnectionStateChanged* cb,
                                   void*                                 cbData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    Channel_CallbackData* internalCbData = NULL;
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
                internalCbData = Create_CallbackData(cb, cbData);
                if(internalCbData == NULL){
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
                                           ChannelConnectionCB, internalCbData);
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Channel_BeginInvokeService(SOPC_Channel                     channel,
                                         char*                          debugName,
                                         void*                          request,
                                         SOPC_EncodeableType*             requestType,
                                         SOPC_EncodeableType*             responseType,
                                         SOPC_Channel_PfnRequestComplete* cb,
                                         void*                          cbData)
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
        SC_Send_Request(cConnection,
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
                                         void*              response,
                                         SOPC_EncodeableType* responseType,
                                         void*              cbData,
                                         SOPC_StatusCode         status){
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
                                    char*               debugName,
                                    void*               request,
                                    SOPC_EncodeableType*  requestType,
                                    SOPC_EncodeableType*  expResponseType,
                                    void**              response,
                                    SOPC_EncodeableType** responseType){
    const uint32_t sleepTimeout = 500;
    uint32_t loopCpt = 0;
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
           loopCpt * sleepTimeout <= timeout)
    {
        loopCpt++;
#if SOPC_MULTITHREADED
        // just wait for callback called
        //Sleep (sleepTimeout);
        return OpcUa_BadNotImplemented;
#else
        // TODO: will retrieve any message: is it a problem ?
        // Retrieve received messages on socket
        status = SOPC_SocketManager_Loop (SOPC_SocketManager_GetGlobal(),
                                        sleepTimeout);
#endif //SOPC_MULTITHREADED
        status = Get_InvokeCallbackData(invCallbackData,
                                        response,
                                        responseType);
        if(*response != NULL){
            receivedEvent = 1; // True
        }
    }

    if(loopCpt * sleepTimeout > timeout){
        status = OpcUa_BadTimeout;
    }

    return status;
}

SOPC_StatusCode SOPC_Channel_Disconnect(SOPC_Channel channel){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != NULL){
        status = STATUS_NOK;
        Delete_CallbackData(((SC_ClientConnection*) channel)->callbackData);
        status = SC_Client_Disconnect((SC_ClientConnection*) channel);
        StackConfiguration_Unlocked();
    }
    return status;
}

#endif // CLIENT_API

