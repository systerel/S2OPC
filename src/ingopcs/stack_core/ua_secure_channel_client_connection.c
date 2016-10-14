/*
 * secure_channel_client_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include <assert.h>
#include <stdlib.h>

#include <wrappers.h>

#include "ua_secure_channel_client_connection.h"

#include <crypto_provider.h>

#include <ua_encoder.h>
#include <ua_secure_channel_low_level.h>
#include <ua_types.h>

PendingRequest* SC_PendingRequestCreate(uint32_t             requestId,
                                        UA_EncodeableType*   responseType,
                                        uint32_t             timeoutHint,
                                        uint32_t             startTime,
                                        SC_ResponseEvent_CB* callback,
                                        void*                callbackData){
    PendingRequest* result = NULL;
    if(requestId != 0){
        result = malloc(sizeof(PendingRequest));
    }
    if(result != NULL){
        result->requestId = requestId;
        result->responseType = responseType;
        result->timeoutHint = timeoutHint;
        result->startTime = startTime;
        result->callback = callback;
        result->callbackData = callbackData;
    }
    return result;
}

void SC_PendingRequestDelete(PendingRequest* pRequest){
    if(pRequest != NULL){
        free(pRequest);
    }
}

SC_ClientConnection* SC_Client_Create(){
    SC_ClientConnection* scClientConnection = NULL;
    SC_Connection* sConnection = SC_Create();

    if(sConnection != NULL){
        scClientConnection = (SC_ClientConnection *) malloc (sizeof(SC_ClientConnection));

        if(scClientConnection != NULL){
            memset (scClientConnection, 0, sizeof(SC_ClientConnection));
            Namespace_Initialize(&scClientConnection->namespaces);
            ByteString_Initialize(&scClientConnection->serverCertificate);
            ByteString_Initialize(&scClientConnection->clientCertificate);
            scClientConnection->securityMode = UA_MessageSecurityMode_Invalid;
            ByteString_Initialize(&scClientConnection->securityPolicy);

            sConnection->state = SC_Connection_Disconnected;
            scClientConnection->instance = sConnection;

            // TODO: limit set by configuration in ua_stacks_csts ?
            scClientConnection->pendingRequests = SLinkedList_Create(255);
            if(scClientConnection->pendingRequests == NULL){
                free(scClientConnection);
                scClientConnection = NULL;
            }
        }
    }else{
        SC_Delete(sConnection);
    }
    return scClientConnection;
}

StatusCode SC_Client_Configure(SC_ClientConnection* cConnection,
                               UA_NamespaceTable*   namespaceTable,
                               UA_EncodeableType**  encodeableTypes){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cConnection != NULL && cConnection->instance != NULL){
        status = Namespace_AttachTable(&cConnection->namespaces, namespaceTable);
        cConnection->encodeableTypes = encodeableTypes;
    }
    return status;
}

SC_ClientConnection* SC_Client_CreateAndConfigure(UA_NamespaceTable*   namespaceTable,
                                                  UA_EncodeableType**  encodeableTypes)
{
    SC_ClientConnection* scClientConnection = NULL;
    StatusCode status = STATUS_OK;
    scClientConnection = SC_Client_Create();
    status = SC_Client_Configure(scClientConnection,
                                 namespaceTable,
                                 encodeableTypes);
    if(status != STATUS_OK){
        SC_Client_Delete(scClientConnection);
        scClientConnection = NULL;
    }
    return scClientConnection;
}

void SC_Client_Delete(SC_ClientConnection* scConnection)
{
    if(scConnection != NULL){
        if(scConnection->pkiProvider != NULL){
            PKIProvider_Delete(scConnection->pkiProvider);
        }
        ByteString_Clear(&scConnection->serverCertificate);
        ByteString_Clear(&scConnection->clientCertificate);
        SLinkedList_Delete(scConnection->pendingRequests);
        String_Clear(&scConnection->securityPolicy);
        if(scConnection->instance != NULL){
            SC_Delete(scConnection->instance);
        }
        Timer_Delete(&scConnection->watchdogTimer);
        free(scConnection);
    }
}

uint32_t GetNextRequestId(SC_Connection* scConnection){
    uint32_t requestId = 0;
    assert(scConnection != NULL);

    requestId = scConnection->lastRequestIdSent + 1;
    if(requestId == 0){
        (requestId)++;
    }
    scConnection->lastRequestIdSent = requestId;

    return requestId;
}

StatusCode Write_OpenSecureChannelRequest(SC_ClientConnection* cConnection,
                                          uint32_t             requestId)
{
    StatusCode status = STATUS_OK;
    UA_OpenSecureChannelRequest openRequest;
    UA_OpenSecureChannelRequest_Initialize(&openRequest);
    const uint32_t uzero = 0;
    const uint32_t uone = 1;

    UA_MsgBuffer* sendBuf = cConnection->instance->sendingBuffer;

    //// Encode request header
    // Encode authentication token (omitted opaque identifier ???? => must be a bytestring ?)
    openRequest.RequestHeader.AuthenticationToken.identifierType = IdentifierType_Numeric;
    openRequest.RequestHeader.AuthenticationToken.numeric = UA_Null_Id;
    // Encode 64 bits UtcTime => null ok ?
    openRequest.RequestHeader.Timestamp = 0;
    // Encode requestHandler
    openRequest.RequestHeader.RequestHandle = requestId;
    // Encode returnDiagnostic => symbolic id
    openRequest.RequestHeader.ReturnDiagnostics = uone;
    // Encode auditEntryId
    status = String_CopyFromCString(&openRequest.RequestHeader.AuditEntryId, "audit1");

    if(status == STATUS_OK){
        // Encode timeoutHint => no timeout (for now)
        openRequest.RequestHeader.TimeoutHint = uzero;

        // Extension object: additional header => null node id => no content
        // !! Extensible parameter indicated in specification but Extension object in XML file !!
        // Encoding body byte:
        openRequest.RequestHeader.AdditionalHeader.encoding = UA_ExtObjBodyEncoding_None;
        // Type Id: Node Id
        openRequest.RequestHeader.AdditionalHeader.typeId.identifierType = IdentifierType_Numeric;
        openRequest.RequestHeader.AdditionalHeader.typeId.numeric = UA_Null_Id;

        //// Encode request content
        // Client protocol version
        openRequest.ClientProtocolVersion = scProtocolVersion;
        // Enumeration request type => ISSUE_0
        openRequest.RequestType = UA_SecurityTokenRequestType_Issue;

        // Security mode value check
        if(cConnection->securityMode == UA_MessageSecurityMode_Invalid){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            openRequest.SecurityMode = cConnection->securityMode;
        }
    }

    // TODO: to remove after complete integration of INGOPCS crypto provider
    CryptoProvider* cProvider = CryptoProvider_Create(String_GetRawCString(&cConnection->securityPolicy));

    if(status == STATUS_OK && cProvider != NULL){
        status = CryptoProvider_SymmetricGenerateKey(cProvider, //cConnection->instance->currentCryptoProvider,
                                                     &cConnection->instance->currentNonce);
        CryptoProvider_Delete(cProvider);
        cProvider = NULL;
        if(status == STATUS_OK){
            uint8_t* bytes = NULL;
            bytes = SecretBuffer_Expose(cConnection->instance->currentNonce);
            status = ByteString_AttachFromBytes(&openRequest.ClientNonce,
                                                bytes,
                                                SecretBuffer_GetLength(cConnection->instance->currentNonce));
        }else{
            status = STATUS_NOK;
        }
    }

    if(status == STATUS_OK){
        openRequest.RequestedLifetime = cConnection->requestedLifetime;
    }

    if(status == STATUS_OK){
        status = SC_EncodeMsgBody(sendBuf,
                                  &UA_OpenSecureChannelRequest_EncodeableType,
                                  &openRequest);
    }

    SecretBuffer_Unexpose(openRequest.ClientNonce.characters);
    UA_OpenSecureChannelRequest_Clear(&openRequest);

    return status;
}

StatusCode Send_OpenSecureChannelRequest(SC_ClientConnection* cConnection)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    CryptoProvider* cProvider = NULL;
    uint32_t requestId = 0;

    if(cConnection != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        // Generate next request id
        requestId = GetNextRequestId(cConnection->instance);

        // Set security configuration for secure channel request
        cConnection->instance->currentSecuMode = cConnection->securityMode;
        status = String_AttachFrom(&cConnection->instance->currentSecuPolicy,
                                   &cConnection->securityPolicy);
    }

    if(status == STATUS_OK){
        // TODO: temporary replacement of code to gen a foundation crypto provider
        //cProvider = CryptoProvider_Create(&cConnection->securityPolicy);
        char* secuPolC = String_GetCString(&cConnection->securityPolicy);
        cProvider = malloc(sizeof(OpcUa_CryptoProvider));
        status = OpcUa_CreateCryptoProvider(secuPolC, (OpcUa_CryptoProvider*) cProvider);
        free(secuPolC);
        if(cProvider == NULL){
            status = STATUS_NOK;
        }else{
            cConnection->instance->currentCryptoProvider = cProvider;
        }
    }


    // MaxBodySize to be computed prior any write in sending buffer
    if(status == STATUS_OK){
        status = SC_SetMaxBodySize(cConnection->instance, UA_FALSE);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSecureMsgHeader(cConnection->instance->sendingBuffer,
                                          UA_OpenSecureChannel,
                                          0);
    }

    if(status == STATUS_OK){
        status = SC_EncodeAsymmSecurityHeader(cConnection->instance,
                                              &cConnection->securityPolicy,
                                              &cConnection->clientCertificate,
                                              &cConnection->serverCertificate);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSequenceHeader(cConnection->instance->sendingBuffer, requestId);
    }

    if(status == STATUS_OK){
        status = Write_OpenSecureChannelRequest(cConnection, requestId);
    }

    if(status == STATUS_OK){
        status = SC_FlushSecureMsgBuffer(cConnection->instance->sendingBuffer, UA_Msg_Chunk_Final);
    }

    if(status == STATUS_OK){
        // TODO: remove precedent OPN request if existing => before flush ?
        PendingRequest* pRequest = SC_PendingRequestCreate(requestId,
                                                           &UA_OpenSecureChannelResponse_EncodeableType,
                                                           0, // Not managed now
                                                           0, // Not managed now
                                                           NULL, // No callback, specifc message header used (OPN)
                                                           NULL);
        if(pRequest != SLinkedList_Add(cConnection->pendingRequests, requestId, pRequest)){
            status = STATUS_NOK;
        }
    }

    return status;
}

StatusCode Read_OpenSecureChannelReponse(SC_ClientConnection* cConnection,
                                         PendingRequest*      pRequest)
{
    assert(cConnection != NULL &&
           pRequest != NULL && pRequest->responseType != NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_OpenSecureChannelResponse* encObj = NULL;
    UA_EncodeableType* receivedType = NULL;

    status = SC_DecodeMsgBody(cConnection->instance->receptionBuffers,
                              &cConnection->instance->receptionBuffers->nsTable,
                              NULL,
                              pRequest->responseType,
                              NULL,
                              &receivedType,
                              (void**) &encObj);
    if(status == STATUS_OK){
        status = SC_CheckReceivedProtocolVersion(cConnection->instance, encObj->ServerProtocolVersion);
    }

    if(status == STATUS_OK){
        // TODO: in case of renew: when moving from current to prec ?
        // TODO: is the sc ID the same after a renew ? => It should => to be checked

        if(encObj->SecurityToken.ChannelId != 0){
            // Check same secure channel id was used in secure message header
            if(encObj->SecurityToken.ChannelId != cConnection->instance->secureChannelId){
                status = STATUS_INVALID_RCV_PARAMETER;
            }else{
                cConnection->instance->currentSecuToken.channelId = encObj->SecurityToken.ChannelId;
                cConnection->instance->currentSecuToken.tokenId = encObj->SecurityToken.TokenId;
                cConnection->instance->currentSecuToken.createdAt = encObj->SecurityToken.CreatedAt;
                cConnection->instance->currentSecuToken.revisedLifetime = encObj->SecurityToken.RevisedLifetime;
            }
        }else{
            // NULL SC ID !
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    if(status == STATUS_OK){
        int32_t encryptKeyLength, signKeyLength, initVectorLength;
        status = CryptoProvider_DeriveKeyLengths(cConnection->instance->currentCryptoProvider,
                                                 &encryptKeyLength,
                                                 &signKeyLength,
                                                 &initVectorLength);
        if(status == STATUS_OK && encObj->ServerNonce.length > 0){
            cConnection->instance->currentSecuKeySets.receiverKeySet = KeySet_Create();
            cConnection->instance->currentSecuKeySets.senderKeySet = KeySet_Create();
            status = CryptoProvider_ClientDeriveKeySets(cConnection->instance->currentCryptoProvider,
                                                        cConnection->instance->currentNonce,
                                                        &encObj->ServerNonce,
                                                        encryptKeyLength,
                                                        signKeyLength,
                                                        initVectorLength,
                                                        cConnection->instance->currentSecuKeySets.senderKeySet,
                                                        cConnection->instance->currentSecuKeySets.receiverKeySet);
        }
    }

    UA_OpenSecureChannelResponse_Clear(encObj);
    free(encObj);

    return status;
}

StatusCode Receive_OpenSecureChannelResponse(SC_ClientConnection* cConnection,
                                             UA_MsgBuffer*        transportMsgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    const uint32_t validateSenderCertificateTrue = 1; // True: always activated as indicated in API
    const uint32_t isSymmetricFalse = UA_FALSE;
    const uint32_t isPrecCryptoDataFalse = UA_FALSE; // TODO: add guarantee we are treating last OPN sent: using pending requests ?
    uint32_t requestId = 0;
    uint32_t snPosition = 0;
    PendingRequest* pRequest = NULL;

    if(cConnection != NULL && transportMsgBuffer != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK &&
       transportMsgBuffer->isFinal != UA_Msg_Chunk_Final){
        // OPN request/response must be in one chunk only
        status = STATUS_INVALID_RCV_PARAMETER;
    }

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(status == STATUS_OK){
        status = SC_DecodeSecureMsgSCid(cConnection->instance,
                                        transportMsgBuffer);
    }

    if(status == STATUS_OK){
        // Check security policy
        // Validate other app certificate
        // Check current app certificate thumbprint
        status = SC_DecodeAsymmSecurityHeader(cConnection->instance,
                                              cConnection->pkiProvider,
                                              transportMsgBuffer,
                                              validateSenderCertificateTrue,
                                              &snPosition);
    }

    if(status == STATUS_OK){
        // Decrypt message content and store complete message in secure connection buffer
        status = SC_DecryptMsg(cConnection->instance,
                               transportMsgBuffer,
                               snPosition,
                               isSymmetricFalse,
                               isPrecCryptoDataFalse);
    }

    if(status == STATUS_OK){
        // Check decrypted message signature
        status = SC_VerifyMsgSignature(cConnection->instance,
                                       isSymmetricFalse,
                                       isPrecCryptoDataFalse); // IsAsymmetric = TRUE
    }

    if(status == STATUS_OK){
        status = SC_CheckSeqNumReceived(cConnection->instance);
    }

    if(status == STATUS_OK){
        // Retrieve request id
        status = UInt32_Read(&requestId, cConnection->instance->receptionBuffers);
    }

    if(status == STATUS_OK){
        // Retrieve associated pending request
        pRequest = SLinkedList_Remove(cConnection->pendingRequests, requestId);
        if(pRequest == NULL){
            status = STATUS_NOK;
        }
    }

    // TODO: remove only because foundation and ingopcs crypto provider are coexisting
    if(status == STATUS_OK){
        if(cConnection->instance->currentCryptoProvider != NULL){
            OpcUa_DeleteCryptoProvider((OpcUa_CryptoProvider*)cConnection->instance->currentCryptoProvider);
            free(cConnection->instance->currentCryptoProvider);
            cConnection->instance->currentCryptoProvider = NULL;
        }
        cConnection->instance->currentCryptoProvider = CryptoProvider_Create
                                                        (String_GetRawCString(&cConnection->securityPolicy));
        if(cConnection->instance->currentCryptoProvider == NULL){
            status = STATUS_NOK;
        }
    }

    if(status == STATUS_OK){
        // Decode message body content
        status = Read_OpenSecureChannelReponse(cConnection, pRequest);
    }

    if(status == STATUS_OK){
        SC_PendingRequestDelete(pRequest);
        pRequest = NULL;
        // Reset reception buffers for next messages
        MsgBuffers_Reset(cConnection->instance->receptionBuffers);
    }else{
        // Trace / channel CB
    }

    return status;
}

StatusCode Receive_ServiceResponse(SC_ClientConnection* cConnection,
                                   UA_MsgBuffer*        transportMsgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t  abortReqPresence = 0;
    uint32_t abortedRequestId = 0;
    StatusCode abortReqStatus = STATUS_NOK;
    uint32_t requestId = 0;
    uint8_t requestToRemove = UA_FALSE;
    UA_String reason;
    String_Initialize(&reason);

    PendingRequest* pRequest = NULL;
    UA_EncodeableType* recEncType = NULL;
    void* encObj = NULL;

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(cConnection != NULL){
        status = SC_DecryptSecureMessage(cConnection->instance,
                                         transportMsgBuffer,
                                         &requestId);
    }

    if(status == STATUS_OK){
        status = SC_CheckAbortChunk(cConnection->instance->receptionBuffers,
                                    &reason);
        // Decoded request id to be aborted
        abortReqPresence = 1;
        abortedRequestId = requestId;

        if(status != STATUS_OK){
            abortReqStatus = status;
            //TODO: report (trace)
        }
    }

    if(status == STATUS_OK){
        status = SC_CheckPrecChunk(cConnection->instance->receptionBuffers,
                                   requestId,
                                   &abortReqPresence,
                                   &abortedRequestId);
    }

    if(abortReqPresence != UA_FALSE){
        // Note: status is OK if from a prec chunk or NOK if current chunk is abort chunk
        // Retrieve request id to be aborted and call callback if any
        pRequest = SLinkedList_Remove(cConnection->pendingRequests, abortedRequestId);
        if(pRequest != NULL){
            if(pRequest->callback != NULL){
                pRequest->callback(cConnection,
                                   encObj,
                                   recEncType,
                                   pRequest->callbackData,
                                   abortReqStatus);
            }
            SC_PendingRequestDelete(pRequest);
        }
    }

    if(status == STATUS_OK){
        if(cConnection->instance->receptionBuffers->isFinal == UA_Msg_Chunk_Final){
            // Retrieve associated pending request for current chunk which is final
            pRequest = SLinkedList_Remove(cConnection->pendingRequests, requestId);
            requestToRemove = 1; // True
            if(pRequest == NULL){
                status = STATUS_NOK;
            }
        }else if(cConnection->instance->receptionBuffers->isFinal == UA_Msg_Chunk_Intermediate &&
                cConnection->instance->receptionBuffers->nbChunks == 1){
            // When it is the first chunk and it is intermediate we have to check request id is valid
            //  otherwise request id already validated before and not pending request not necessary
            pRequest = SLinkedList_Find(cConnection->pendingRequests, requestId);
            if(pRequest == NULL){
                // Error: unknown request id !
                // TODO: trace + callback for audit ?
                MsgBuffers_Reset(cConnection->instance->receptionBuffers);
                status = STATUS_NOK;
            }
        }
    }

    if(status == STATUS_OK){
        if(pRequest == NULL){
            status = SC_DecodeChunk(cConnection->instance->receptionBuffers,
                                    requestId,
                                    NULL,
                                    &UA_ServiceFault_EncodeableType,
                                    &recEncType,
                                    &encObj);
        }else{
            status = SC_DecodeChunk(cConnection->instance->receptionBuffers,
                                    requestId,
                                    pRequest->responseType,
                                    &UA_ServiceFault_EncodeableType,
                                    &recEncType,
                                    &encObj);
            // TODO: check status before ?
            if(pRequest->callback != NULL && requestToRemove != UA_FALSE){
                pRequest->callback(cConnection,
                                   encObj,
                                   recEncType,
                                   pRequest->callbackData,
                                   status);

                // Deallocate pending request
                SC_PendingRequestDelete(pRequest);
            }
        }
    }

    String_Clear(&reason);
    return status;
}

StatusCode OnTransportEvent_CB(void*           connection,
                               void*           callbackData,
                               ConnectionEvent event,
                               UA_MsgBuffer*   msgBuffer,
                               StatusCode      status)
{
    SC_ClientConnection* cConnection = (SC_ClientConnection*) callbackData;
    TCP_UA_Connection* tcpConnection = (TCP_UA_Connection*) connection;
    StatusCode retStatus = STATUS_OK;
    assert(cConnection->instance->transportConnection == tcpConnection);
    switch(event){
        case ConnectionEvent_Connected:
            assert(status == STATUS_OK);
            assert(cConnection->instance->state == SC_Connection_Connecting_Transport);
            retStatus = SC_InitApplicationIdentities
                         (cConnection->instance,
                          &cConnection->clientCertificate,
                          cConnection->clientKey,
                          &cConnection->serverCertificate);
            // Configure secure connection for encoding / decoding messages
            if(status == STATUS_OK){
                status = SC_InitReceiveSecureBuffers(cConnection->instance,
                                                     &cConnection->namespaces,
                                                     cConnection->encodeableTypes);
            }
            if(status == STATUS_OK){
                status = SC_InitSendSecureBuffer(cConnection->instance,
                                                 &cConnection->namespaces,
                                                 cConnection->encodeableTypes);
            }
            // Send Open Secure channel request
            if(status == STATUS_OK){
                cConnection->instance->state = SC_Connection_Connecting_Secure;
                status = Send_OpenSecureChannelRequest(cConnection);
            }
            break;

        case ConnectionEvent_Disconnected:
            //log ?
            TCP_UA_Connection_Disconnect(tcpConnection);
            cConnection->instance->state = SC_Connection_Disconnected;
            break;

        case ConnectionEvent_Message:
            assert(status == STATUS_OK);
            switch(msgBuffer->secureType){
                case UA_OpenSecureChannel:
                    if(cConnection->instance->state == SC_Connection_Connecting_Secure){
                        // Receive Open Secure Channel response
                        retStatus = Receive_OpenSecureChannelResponse(cConnection, msgBuffer);
                        if(retStatus == STATUS_OK){
                            cConnection->instance->state = SC_Connection_Connected;
                            // TODO: cases in which retStatus != OK should be sent ?
                            retStatus = cConnection->callback(cConnection,
                                                              cConnection->callbackData,
                                                              OpcUa_ConnectionEvent_Connect,
                                                              retStatus);
                        }
                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
                case UA_CloseSecureChannel:
                    if(cConnection->instance->state == SC_Connection_Connected){
                        assert(UA_FALSE);
                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
                case UA_SecureMessage:
                    if(cConnection->instance->state == SC_Connection_Connected){
                        retStatus = Receive_ServiceResponse(cConnection, msgBuffer);
                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
            }
            break;
        case ConnectionEvent_Error:
            //log ?
            TCP_UA_Connection_Disconnect(tcpConnection);
            cConnection->instance->state = SC_Connection_Disconnected;
            //scConnection->callback: TODO: incompatible types to modify in foundation code
            break;
        default:
            assert(UA_FALSE);
    }
    return retStatus;
}

StatusCode SC_Client_Connect(SC_ClientConnection*   connection,
                             const char*            uri,
                             void*                  pkiConfig,
                             const UA_ByteString*   clientCertificate,
                             const UA_ByteString*   clientKey,
                             const UA_ByteString*   serverCertificate,
                             UA_MessageSecurityMode securityMode,
                             const char*            securityPolicy,
                             uint32_t               requestedLifetime,
                             SC_ConnectionEvent_CB* callback,
                             void*                  callbackData)
{
    StatusCode status = STATUS_NOK;

    if(uri != NULL &&
       pkiConfig != NULL &&
       clientCertificate != NULL &&
       clientKey != NULL &&
       serverCertificate != NULL &&
       securityMode != UA_MessageSecurityMode_Invalid &&
       securityPolicy != NULL &&
       requestedLifetime > 0)
    {
        if(connection->clientCertificate.length <= 0 &&
           SecretBuffer_GetLength(connection->clientKey) == 0 &&
           connection->serverCertificate.length <= 0 &&
           connection->securityMode == UA_MessageSecurityMode_Invalid &&
           connection->securityPolicy.length <= 0 &&
           connection->callback == NULL &&
           connection->callbackData == NULL)
        {
            // Create PKI provider
            connection->pkiProvider = PKIProvider_Create(pkiConfig);
            status = ByteString_Copy(&connection->clientCertificate, clientCertificate);

            if(status == STATUS_OK){
                connection->clientKey = SecretBuffer_NewFromExposedBuffer
                                         (clientKey->characters, clientKey->length);
                status = ByteString_Copy(&connection->serverCertificate, serverCertificate);
            }

            if(status == STATUS_OK){
                if(securityMode != UA_MessageSecurityMode_Invalid){
                    connection->securityMode = securityMode;
                }else{
                    status = STATUS_NOK;
                }
            }

            if(status == STATUS_OK){
                status = String_InitializeFromCString(&connection->securityPolicy, securityPolicy);
            }

            connection->requestedLifetime = requestedLifetime;
            connection->callback = callback;
            connection->callbackData = callbackData;

            if(status == STATUS_OK){
                // TODO: check security mode = None if securityPolicy != None ??? => see http://opcfoundation.org/UA-Profile/UA/SecurityPolicy%23Basic128Rsa15
                connection->instance->state = SC_Connection_Connecting_Transport;
                status = TCP_UA_Connection_Connect(connection->instance->transportConnection,
                                                   uri,
                                                   OnTransportEvent_CB,
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

StatusCode SC_Send_Request(SC_ClientConnection* connection,
                           UA_EncodeableType*   requestType,
                           void*                request,
                           UA_EncodeableType*   responseType,
                           uint32_t             timeout,
                           SC_ResponseEvent_CB* callback,
                           void*                callbackData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t requestId = 0;
    if(connection != NULL &&
       requestType != NULL &&
       request != NULL)
    {
        requestId = GetNextRequestId(connection->instance);
        status = SC_EncodeSecureMessage(connection->instance,
                                        requestType,
                                        request,
                                        requestId);
    }

    if(status == STATUS_OK){
        // Create associated pending request
        PendingRequest* pRequest = SC_PendingRequestCreate(requestId,
                                                           responseType,
                                                           timeout,
                                                           0, // Not managed now
                                                           callback,
                                                           callbackData);
        if(pRequest != SLinkedList_Add(connection->pendingRequests, requestId, pRequest)){
            status = STATUS_NOK;
        }
    }

    if(status == STATUS_OK){
        status = SC_FlushSecureMsgBuffer(connection->instance->sendingBuffer, UA_Msg_Chunk_Final);
    }

    return status;

}
