/*
 * secure_channel_client_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include <wrappers.h>

#include <assert.h>
#include <stdlib.h>

#include <ua_encoder.h>
#include <secure_channel_client_connection.h>

SC_Channel_Client_Connection* Create_Client_Channel (Namespace*      namespac,
                                                     EncodeableType* encodeableTypes){
    SC_Channel_Client_Connection* scClientConnection = UA_NULL;
    SecureChannel_Connection* sConnection = Create_Secure_Connection();

    if(sConnection != UA_NULL){
        scClientConnection = (SC_Channel_Client_Connection *) malloc (sizeof(SC_Channel_Client_Connection));

        if(scClientConnection != UA_NULL){
            memset (scClientConnection, 0, sizeof(SC_Channel_Client_Connection));
            sConnection->state = SC_Connection_Disconnected;
            scClientConnection->instance = sConnection;
            scClientConnection->namespaces = namespac;
            scClientConnection->encodeableTypes = encodeableTypes;
            scClientConnection->securityMode = Msg_Security_Mode_Invalid;
        }
    }else{
        Delete_Secure_Connection(sConnection);
    }
    return scClientConnection;
}

void Delete_Client_Channel(SC_Channel_Client_Connection* scConnection){
    if(scConnection != UA_NULL){
        if(scConnection->serverCertificate != UA_NULL){
            Delete_Byte_String(scConnection->serverCertificate);
        }
        if(scConnection->clientCertificate != UA_NULL){
            Delete_Byte_String(scConnection->clientCertificate);
        }
        if(scConnection->pendingRequests != UA_NULL){
            free(scConnection->pendingRequests);
        }
        if(scConnection->securityPolicy != UA_NULL){
            Delete_String(scConnection->securityPolicy);
        }
        if(scConnection->instance != UA_NULL){
            Delete_Secure_Connection(scConnection->instance);
        }
        Delete_Timer(&scConnection->watchdogTimer);
    }
}

StatusCode On_Transport_Event_CB(void*            connection,
                                 void*            callbackData,
                                 Connection_Event event,
                                 UA_Msg_Buffer*   msgBuffer,
                                 StatusCode       status){
    SC_Channel_Client_Connection* cConnection = (SC_Channel_Client_Connection*) callbackData;
    TCP_UA_Connection* tcpConnection = (TCP_UA_Connection*) connection;
    StatusCode retStatus = STATUS_OK;
    assert(cConnection->instance->transportConnection == tcpConnection);
    switch(event){
        case ConnectionEvent_Connected:
            assert(status == STATUS_OK);
            assert(cConnection->instance->state == SC_Connection_Connecting_Transport);
            retStatus = Initiate_Applications_Identities
                         (cConnection->instance,
                          cConnection->clientCertificate,
                          cConnection->clientKey,
                          cConnection->serverCertificate);
            // Configure secure connection for encoding / decoding messages
            if(status == STATUS_OK){
                status = Initiate_Receive_Secure_Buffers(cConnection->instance);
            }
            if(status == STATUS_OK){
                status = Initiate_Send_Secure_Buffer(cConnection->instance);
            }
            // Send Open Secure channel request
            if(status == STATUS_OK){
                status = Send_Open_Secure_Channel_Request();
            }
            break;

        case ConnectionEvent_Disconnected:
            //log ?
            Disconnect_Transport(tcpConnection);
            cConnection->instance->state = SC_Connection_Disconnected;
            break;

        case ConnectionEvent_Message:
            assert(status == STATUS_OK);
            switch(msgBuffer->secureType){
                case UA_OpenSecureChannel:
                    if(cConnection->instance->state == SC_Connection_Connecting_Secure){
                        // Receive Open Secure Channel response
                        Receive_Open_Secure_Channel_Response();
                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
                case UA_CloseSecureChannel:
                    if(cConnection->instance->state == SC_Connection_Connected){

                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
                case UA_SecureMessage:
                    if(cConnection->instance->state == SC_Connection_Connected){

                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
            }
            break;
        case ConnectionEvent_Error:
            //log ?
            Disconnect_Transport(tcpConnection);
            cConnection->instance->state = SC_Connection_Disconnected;
            //scConnection->callback: TODO: incompatible types to modify in foundation code
            break;
        default:
            assert(UA_FALSE);
    }
    return retStatus;
}

StatusCode Connect_Client_Channel(SC_Channel_Client_Connection* connection,
                                  char*                         uri,
                                  void*                         pkiConfig,
                                  UA_Byte_String*               clientCertificate,
                                  UA_Byte_String*               clientKey,
                                  UA_Byte_String*               serverCertificate,
                                  Msg_Security_Mode             securityMode,
                                  char*                         securityPolicy,
                                  uint32_t                      requestedLifetime,
                                  SC_Connection_Event_CB*       callback,
                                  void*                         callbackData
                                  )
{
    StatusCode status = STATUS_NOK;

    if(uri != UA_NULL &&
       pkiConfig != UA_NULL &&
       clientCertificate != UA_NULL &&
       clientKey != UA_NULL &&
       serverCertificate != UA_NULL &&
       securityMode != Msg_Security_Mode_Invalid &&
       securityPolicy != UA_NULL &&
       requestedLifetime > 0)
    {
        if(connection->clientCertificate == UA_NULL &&
           connection->clientKey == UA_NULL &&
           connection->serverCertificate == UA_NULL &&
           connection->securityMode == Msg_Security_Mode_Invalid &&
           connection->securityPolicy == UA_NULL &&
           connection->callback == UA_NULL &&
           connection->callbackData == UA_NULL)
        {
            // Create PKI provider
            connection->clientCertificate = Create_Byte_String_Copy(clientCertificate);
            connection->clientKey = Create_Private_Key(clientKey);
            connection->serverCertificate = Create_Byte_String_Copy(serverCertificate);
            connection->securityMode = securityMode;
            connection->securityPolicy = Create_String_From_CString(securityPolicy);
            connection->requestedLifetime = requestedLifetime;
            connection->callback = callback;
            connection->callbackData = callbackData;

            if(connection->clientCertificate == UA_NULL ||
               connection->clientKey == UA_NULL ||
               connection->serverCertificate == UA_NULL ||
               connection->securityMode == Msg_Security_Mode_Invalid ||
               connection->securityPolicy == UA_NULL)
            {
                status = STATUS_NOK;
            }else{
                connection->instance->state = SC_Connection_Connecting_Transport;
                status = Connect_Transport(connection->instance->transportConnection,
                                           uri,
                                           On_Transport_Event_CB,
                                           (void*) connection);

                if(status != STATUS_OK){
                    connection->instance->state = SC_Connection_Disconnected;
                }
            }

        }else{
            status = STATUS_INVALID_STATE;
        }
    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    return status;
}

StatusCode Send_Open_Secure_Channel_Request(){
    // TODO: Set max body size in connection in order to could compute flush on writing
    return STATUS_OK;
}

StatusCode Receive_Open_Secure_Channel_Response(){
    return STATUS_OK;
}

