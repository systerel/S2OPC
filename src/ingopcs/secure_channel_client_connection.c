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
#include <secure_channel_low_level.h>

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
                cConnection->instance->state = SC_Connection_Connecting_Secure;
                status = Send_Open_Secure_Channel_Request(cConnection);
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

StatusCode Write_Open_Secure_Channel_Request(SC_Channel_Client_Connection* cConnection){
    const UA_Byte twoByteNodeIdType = 0x00;
    const UA_Byte fourByteNodeIdType = 0x01;
    const UA_Byte namespace = 0;
    const UA_Byte nullTypeId = 0;
    const UA_Byte noBodyEncoded = 0;
    const uint16_t openSecureChannelRequestTypeId = 444;
    UA_String* auditId = Create_String_From_CString("audit1");
    UA_String* nullString = Create_String();

    UA_Msg_Buffer* sendBuf = cConnection->instance->sendingBuffer;

    // Encode request type Node Id (prior to message body)
    Write_Secure_Msg_Buffer(sendBuf, &fourByteNodeIdType, 1);
    Write_Secure_Msg_Buffer(sendBuf, &namespace, 1);
    Write_UInt16(sendBuf, openSecureChannelRequestTypeId);

    //// Encode request header
    // Encode authentication token (omitted opaque identifier ???? => must be a bytestring ?)
    Write_Secure_Msg_Buffer(sendBuf, &twoByteNodeIdType, 1);
    Write_Secure_Msg_Buffer(sendBuf, &nullTypeId, 1);
    // Encode 64 bits UtcTime => null ok ?
    Write_UInt32(sendBuf, 0);
    Write_UInt32(sendBuf, 0);
    // Encode requestHandler
    Write_UInt32(sendBuf, sendBuf->requestId);
    // Encode returnDiagnostic => symbolic id
    Write_UInt32(sendBuf, 1);
    // Encode auditEntryId
    Write_UA_String(sendBuf, auditId);
    // Encode timeoutHint => no timeout (for now)
    Write_UInt32(sendBuf, 0);

    // Extension object: additional header => null node id => no content
    // !! Extensible parameter indicated in specification but Extension object in XML file !!
    // Type Id: Node Id
    Write_Secure_Msg_Buffer(sendBuf, &twoByteNodeIdType, 1);
    Write_Secure_Msg_Buffer(sendBuf, &nullTypeId, 1);
    // Encoding body byte:
    Write_Secure_Msg_Buffer(sendBuf, &noBodyEncoded, 1);

    //// Encode request content
    // Client protocol version => 0
    Write_UInt32(sendBuf, 0);
    // Enumeration request type => ISSUE_0
    Write_Int32(sendBuf, 0);
    // Enumeration security mode => NONE_1
    Write_Int32(sendBuf, 1);
    // Client nonce => null string
    Write_UA_String(sendBuf, nullString);
    // Requested lifetime
    Write_Int32(sendBuf, cConnection->requestedLifetime);

    return STATUS_OK;
}

StatusCode Send_Open_Secure_Channel_Request(SC_Channel_Client_Connection* cConnection){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    Crypto_Provider* cProvider = UA_NULL;
    uint32_t requestId = 0;

    if(cConnection != UA_NULL){
        status = STATUS_OK;
    }

    // Set security configuration for secure channel request
    cConnection->instance->currentSecuMode = cConnection->securityMode;
    cConnection->instance->currentSecuPolicy = cConnection->securityPolicy;

    if(status == STATUS_OK){
        cProvider = Create_Crypto_Provider(cConnection->securityPolicy);
        if(cProvider == UA_NULL){
            status = STATUS_NOK;
        }else{
            cConnection->instance->currentCryptoProvider = cProvider;
        }
    }


    // MaxBodySize to be computed prior any write in sending buffer
    if(status == STATUS_OK){
        status = Set_MaxBodySize(cConnection->instance, 1);
    }

    if(status == STATUS_OK){
        status = Encode_Secure_Message_Header(cConnection->instance->sendingBuffer,
                                              UA_OpenSecureChannel,
                                              0);
    }

    if(status == STATUS_OK){
        status = Encode_Asymmetric_Security_Header(cConnection->instance,
                                                   cProvider,
                                                   cConnection->securityPolicy,
                                                   cConnection->clientCertificate,
                                                   cConnection->serverCertificate);
    }

    if(status == STATUS_OK){
        status = Encode_Sequence_Header(cConnection->instance, &requestId);
    }

    if(status == STATUS_OK){
        status = Write_Open_Secure_Channel_Request(cConnection);
    }

    if(status == STATUS_OK){
        status = Flush_Secure_Msg_Buffer(cConnection->instance->sendingBuffer, UA_Msg_Chunk_Final);
    }

    return status;
}

StatusCode Receive_Open_Secure_Channel_Response(){
    return STATUS_OK;
}

