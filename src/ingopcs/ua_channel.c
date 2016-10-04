/*
 * ua_channel.c
 *
 *  Created on: Sep 29, 2016
 *      Author: vincent
 */

#include <ua_channel.h>

#ifdef OPCUA_HAVE_CLIENTAPI

#include <ua_secure_channel_client_connection.h>

#include <wrappers.h>

#include <stdlib.h>

extern UA_NamespaceTable* g_namespaceTable;

typedef struct {
    UA_Channel_PfnConnectionStateChanged* callback;
    void*                                 callbackData;
} Channel_CallbackData;

Channel_CallbackData* Create_CallbackData(UA_Channel_PfnConnectionStateChanged* callback,
                                          void*                                 callbackData)
{
    Channel_CallbackData* result = malloc(sizeof(Channel_CallbackData));
    if(result != UA_NULL){
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
    UA_EncodeableType* responseType;
    StatusCode         status;
} InvokeCallbackData;

InvokeCallbackData* Create_InvokeCallbackData(){
    InvokeCallbackData* result = malloc(sizeof(InvokeCallbackData));
    result->response = UA_NULL;
    result->responseType = UA_NULL;
    result->status = STATUS_OK;
    return result;
}

void Set_InvokeCallbackData(InvokeCallbackData* invCbData,
                            void*               response,
                            UA_EncodeableType*  responseType,
                            StatusCode          status)
{
    invCbData->response = response;
    invCbData->responseType = responseType;
    invCbData->status = status;
}

StatusCode Get_InvokeCallbackData(InvokeCallbackData* invCbData,
                                  void**              response,
                                  UA_EncodeableType** responseType){
    *response = invCbData->response;
    *responseType = invCbData->responseType;
    return invCbData->status;
}

void Delete_InvokeCallbackData(InvokeCallbackData* invCbData){
    if(invCbData != UA_NULL){
        free(invCbData);
    }
}


StatusCode UA_Channel_Create(UA_Channel*               channel,
                             UA_Channel_SerializerType serialType){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != UA_NULL && serialType == ChannelSerializer_Binary){
        *channel = SC_Client_Create();
        if(channel != UA_NULL){
            status = STATUS_OK;
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

StatusCode UA_Channel_Delete(UA_Channel* channel){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != UA_NULL){
        // Ensure disconnect called for deallocation
        UA_Channel_Disconnect(*channel);
        SC_Client_Delete((SC_ClientConnection*) *channel);
        *channel = UA_NULL;
        status = STATUS_OK;
    }
    return status;
}

//StatusCode ChannelCB (SC_ClientConnection* connection,
//                      void*                response,
//                      UA_EncodeableType*   responseType,
//                      void*                callbackData,
//                      StatusCode           status)
//{
//    UA_Channel channel = (UA_Channel) connection;
//    Channel_CallbackData cbData = callbackData;
//}

StatusCode ChannelConnectionCB(SC_ClientConnection* cConnection,
                               void*                cbData,
                               SC_ConnectionEvent   event,
                               StatusCode           status){
    StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    UA_Channel channel = (UA_Channel) cConnection;
    Channel_CallbackData* callbackData = cbData;
    UA_Channel_Event channelConnectionEvent = ChannelEvent_Invalid;

    switch(event){
        case UA_ConnectionEvent_Connected:
            channelConnectionEvent = ChannelEvent_Connected;
            break;
        case UA_ConnectionEvent_Disconnected:
            channelConnectionEvent = ChannelEvent_Disconnected;
            break;
        case UA_ConnectionEvent_SecureMessageComplete:
        case UA_ConnectionEvent_SecureMessageChunk:
        case UA_ConnectionEvent_SecureMessageAbort:
            // TODO: traces ?
            break;
        case UA_ConnectionEvent_Invalid:
        case UA_ConnectionEvent_UnexpectedError:
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
            if(callbackData != UA_NULL && callbackData->callback != UA_NULL)
            {
                retStatus = callbackData->callback(channel,
                                                   callbackData->callback,
                                                   channelConnectionEvent,
                                                   status);
            }
    }

    return retStatus;
}

StatusCode UA_Channel_BeginConnect(UA_Channel                            channel,
                                   char*                                 url,
                                   UA_ByteString*                        clientCertificate,
                                   UA_ByteString*                        clientPrivateKey,
                                   UA_ByteString*                        serverCertificate,
                                   void*                                 pkiConfig,
                                   char*                                 reqSecuPolicyUri,
                                   int32_t                               requestedLifetime,
                                   UA_MessageSecurityMode                msgSecurityMode,
                                   uint32_t                              networkTimeout,
                                   UA_Channel_PfnConnectionStateChanged* cb,
                                   void*                                 cbData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    Channel_CallbackData* internalCbData = UA_NULL;
    (void) networkTimeout;

    if(cConnection != UA_NULL && cConnection->instance != UA_NULL &&
       url != UA_NULL &&
       clientCertificate != UA_NULL && clientPrivateKey != UA_NULL &&
       serverCertificate != UA_NULL && pkiConfig != UA_NULL &&
       reqSecuPolicyUri != UA_NULL &&
       msgSecurityMode != UA_MessageSecurityMode_Invalid &&
       cb != UA_NULL)
    {
        if(cConnection->instance->state != SC_Connection_Disconnected){
            status = STATUS_INVALID_STATE;
        }else{
            status = SC_Client_Configure(cConnection,
                                         g_namespaceTable,
                                         UA_KnownEncodeableTypes);
            if(status == STATUS_OK){
                internalCbData = Create_CallbackData(cb, cbData);
                if(internalCbData == UA_NULL){
                    status = STATUS_NOK;
                }
            }
            if(status == STATUS_OK){
                status = SC_Client_Connect(cConnection, url,
                                           pkiConfig,
                                           clientCertificate, clientPrivateKey,
                                           serverCertificate,
                                           msgSecurityMode,
                                           reqSecuPolicyUri,
                                           requestedLifetime,
                                           ChannelConnectionCB, internalCbData);
            }
        }
    }
    return status;
}

StatusCode UA_Channel_BeginInvokeService(UA_Channel                     channel,
                                         char*                          debugName,
                                         void*                          request,
                                         UA_EncodeableType*             requestType,
                                         UA_EncodeableType*             responseType,
                                         UA_Channel_PfnRequestComplete* cb,
                                         void*                          cbData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    uint32_t timeout = 0;
    (void) debugName;

    if(cConnection != UA_NULL &&
       request != UA_NULL && requestType != UA_NULL &&
       cb != UA_NULL)
    {
        if(responseType == UA_NULL){
            // TODO: warning on efficiency ?
        }

        // There is always a request header as first struct field in a request (safe cast)
        timeout = ((UA_RequestHeader*)request)->TimeoutHint;
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

StatusCode InvokeRequestCompleteCallback(UA_Channel         channel,
                                         void*              response,
                                         UA_EncodeableType* responseType,
                                         void*              cbData,
                                         StatusCode         status){
    StatusCode retStatus = STATUS_INVALID_PARAMETERS;
    (void) channel;
    InvokeCallbackData* invCbData = (InvokeCallbackData*) cbData;
    if(invCbData != UA_NULL){
        retStatus = STATUS_OK;
        Set_InvokeCallbackData(invCbData,
                               response, responseType,
                               status);
    }
    return retStatus;
}

StatusCode UA_Channel_InvokeService(UA_Channel          channel,
                                    char*               debugName,
                                    void*               request,
                                    UA_EncodeableType*  requestType,
                                    UA_EncodeableType*  expResponseType,
                                    void**              response,
                                    UA_EncodeableType** responseType){
    const uint32_t sleepTimeout = 500;
    uint32_t loopCpt = 0;
    uint8_t receivedEvent = UA_FALSE;
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_ClientConnection* cConnection = (SC_ClientConnection*) channel;
    uint32_t timeout = 0;
    InvokeCallbackData* invCallbackData = Create_InvokeCallbackData();

    if(cConnection != UA_NULL &&
       request != UA_NULL && requestType != UA_NULL &&
       response != UA_NULL && responseType != UA_NULL){
        if(invCallbackData != UA_NULL){
            // There is always a request header as first struct field in a request (safe cast)
            timeout = ((UA_RequestHeader*)request)->TimeoutHint;
            status = UA_Channel_BeginInvokeService(channel,
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
           receivedEvent == UA_FALSE &&
           loopCpt * sleepTimeout <= timeout)
    {
        loopCpt++;
#if UA_MULTITHREADED
        // just wait for callback called
        //Sleep (sleepTimeout);
        return OpcUa_BadNotImplemented;
#else
        // TODO: will retrieve any message: is it a problem ?
        // Retrieve received messages on socket
        status = UA_SocketManager_Loop (UA_NULL, // global socket manager
                                        sleepTimeout,
                                        1);
#endif //UA_MULTITHREADED
        status = Get_InvokeCallbackData(invCallbackData,
                                        response,
                                        responseType);
        if(*response != UA_NULL){
            receivedEvent = 1; // True
        }
    }

    if(loopCpt * sleepTimeout > timeout){
        status = OpcUa_BadTimeout;
    }

    return status;
}

StatusCode UA_Channel_Disconnect(UA_Channel channel){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(channel != UA_NULL){
        status = STATUS_NOK;
        //status = SC_Client_Disconnect((SC_ClientConnection*) channel);
    }
    return status;
}

#endif // CLIENT_API

