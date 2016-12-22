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

#include "sopc_secure_channel_client_connection.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "crypto_provider.h"
#include "sopc_encoder.h"
#include "sopc_types.h"
#include "sopc_secure_channel_low_level.h"

typedef struct PendingRequest
{
    uint32_t             requestId; // 0 is invalid request
    SOPC_EncodeableType* responseType;
    uint32_t             timeoutHint;
    uint32_t             startTime;
    SC_ResponseEvent_CB* callback;
    void*                callbackData;
} PendingRequest;

PendingRequest* SC_PendingRequestCreate(uint32_t             requestId,
                                        SOPC_EncodeableType* responseType,
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
            scClientConnection->securityMode = OpcUa_MessageSecurityMode_Invalid;
            SOPC_String_Initialize(&scClientConnection->securityPolicy);

            sConnection->state = SC_Connection_Disconnected;
            scClientConnection->instance = sConnection;

            // TODO: limit set by configuration insopc_stacks_csts ?
            scClientConnection->pendingRequests = SLinkedList_Create(255);
            if(scClientConnection->pendingRequests == NULL){
                free(scClientConnection);
                scClientConnection = NULL;
            }

            scClientConnection->pkiProvider = NULL;

            if(STATUS_OK != Mutex_Inititalization(&scClientConnection->mutex)){
                SC_Client_Delete(scClientConnection);
                scClientConnection = NULL;
            }
        }
    }else{
        SC_Delete(sConnection);
    }
    return scClientConnection;
}

SOPC_StatusCode SC_Client_Configure(SC_ClientConnection*   cConnection,
                                    SOPC_NamespaceTable*   namespaceTable,
                                    SOPC_EncodeableType**  encodeableTypes){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cConnection != NULL && cConnection->instance != NULL){
        Mutex_Lock(&cConnection->mutex);
        if(namespaceTable != NULL){
            status = Namespace_AttachTable(&cConnection->namespaces, namespaceTable);
        }else{
            status = STATUS_OK;
        }
        cConnection->encodeableTypes = encodeableTypes;
        Mutex_Unlock(&cConnection->mutex);
    }
    return status;
}

void Timer_Delete(P_Timer* timer){
    (void) timer;
}

void SC_Client_Delete(SC_ClientConnection* scConnection)
{
    if(scConnection != NULL){
        Mutex_Lock(&scConnection->mutex);
        scConnection->pkiProvider = NULL;
        scConnection->serverCertificate = NULL;
        scConnection->clientCertificate = NULL;
        scConnection->clientKey = NULL;
        SLinkedList_Delete(scConnection->pendingRequests);
        SOPC_String_Clear(&scConnection->securityPolicy);
        if(scConnection->instance != NULL){
            SC_Delete(scConnection->instance);
        }
        Timer_Delete(&scConnection->watchdogTimer);
        Mutex_Unlock(&scConnection->mutex);
        Mutex_Clear(&scConnection->mutex);
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

SOPC_StatusCode Write_OpenSecureChannelRequest(SC_ClientConnection* cConnection,
                                               uint32_t             requestId)
{
    SOPC_StatusCode status = STATUS_OK;
    OpcUa_OpenSecureChannelRequest openRequest;
    OpcUa_OpenSecureChannelRequest_Initialize(&openRequest);
    const uint32_t uzero = 0;
    const uint32_t uone = 1;

    SOPC_MsgBuffer* sendBuf = cConnection->instance->sendingBuffer;

    //// Encode request header
    // Encode authentication token (omitted opaque identifier ???? => must be a bytestring ?)
    openRequest.RequestHeader.AuthenticationToken.IdentifierType = IdentifierType_Numeric;
    openRequest.RequestHeader.AuthenticationToken.Data.Numeric = SOPC_Null_Id;
    // Encode 64 bits UtcTime => null ok ?
    SOPC_DateTime_Clear(&openRequest.RequestHeader.Timestamp);
    // Encode requestHandler
    openRequest.RequestHeader.RequestHandle = requestId;
    // Encode returnDiagnostic => symbolic id
    openRequest.RequestHeader.ReturnDiagnostics = uone;
    // Encode auditEntryId
    status = SOPC_String_CopyFromCString(&openRequest.RequestHeader.AuditEntryId, "audit1");

    if(status == STATUS_OK){
        // Encode timeoutHint => no timeout (for now)
        openRequest.RequestHeader.TimeoutHint = uzero;

        // Extension object: additional header => null node id => no content
        // !! Extensible parameter indicated in specification but Extension object in XML file !!
        // Encoding body byte:
        openRequest.RequestHeader.AdditionalHeader.Encoding = SOPC_ExtObjBodyEncoding_None;
        // Type Id: Node Id
        openRequest.RequestHeader.AdditionalHeader.TypeId.NodeId.IdentifierType = IdentifierType_Numeric;
        openRequest.RequestHeader.AdditionalHeader.TypeId.NodeId.Data.Numeric = SOPC_Null_Id;

        //// Encode request content
        // Client protocol version
        openRequest.ClientProtocolVersion = scProtocolVersion;
        // Enumeration request type => ISSUE_0
        openRequest.RequestType = OpcUa_SecurityTokenRequestType_Issue;

        // Security mode value check
        if(cConnection->securityMode == OpcUa_MessageSecurityMode_Invalid){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            openRequest.SecurityMode = cConnection->securityMode;
        }
    }

    if(status == STATUS_OK && cConnection->securityMode != OpcUa_MessageSecurityMode_None){
        status = CryptoProvider_GenerateSecureChannelNonce(cConnection->instance->currentCryptoProvider,
                                                     &cConnection->instance->currentNonce);

        if(status == STATUS_OK){
            uint8_t* bytes = NULL;
            bytes = SecretBuffer_Expose(cConnection->instance->currentNonce);
            status = SOPC_ByteString_CopyFromBytes(&openRequest.ClientNonce,
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
                                  &OpcUa_OpenSecureChannelRequest_EncodeableType,
                                  &openRequest);
    }

    SecretBuffer_Unexpose(openRequest.ClientNonce.Data);
    OpcUa_OpenSecureChannelRequest_Clear(&openRequest);

    return status;
}

SOPC_StatusCode Send_OpenSecureChannelRequest(SC_ClientConnection* cConnection)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t requestId = 0;

    if(cConnection != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        // Generate next request id
        requestId = GetNextRequestId(cConnection->instance);

        // Set security configuration for secure channel request
        cConnection->instance->currentSecuMode = cConnection->securityMode;
        status = SOPC_String_AttachFrom(&cConnection->instance->currentSecuPolicy,
                                   &cConnection->securityPolicy);
    }
	
    // TODO: manage prec and current crypto provider here if necessary
    // for now created only on sc_connect once
//    if(status == STATUS_OK){
//        cConnection->instance->currentCryptoProvider =
//                CryptoProvider_Create
//                    (SOPC_String_GetRawCString(&cConnection->securityPolicy));
//
//        if(cConnection->instance->currentCryptoProvider == NULL){
//            status = STATUS_NOK;
//        }
//    }

    // MaxBodySize to be computed prior any write in sending buffer
    if(status == STATUS_OK){
        status = SC_SetMaxBodySize(cConnection->instance, FALSE);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSecureMsgHeader(cConnection->instance->sendingBuffer,
                                          SOPC_OpenSecureChannel,
                                          0);
    }

    if(status == STATUS_OK){
        status = SC_EncodeAsymmSecurityHeader(cConnection->instance,
                                              &cConnection->securityPolicy);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSequenceHeader(cConnection->instance->sendingBuffer, requestId);
    }

    if(status == STATUS_OK){
        status = Write_OpenSecureChannelRequest(cConnection, requestId);
    }

    if(status == STATUS_OK){
        status = SC_FlushSecureMsgBuffer(cConnection->instance->sendingBuffer, SOPC_Msg_Chunk_Final);
    }

    if(status == STATUS_OK){
        // TODO: remove precedent OPN request if existing => before flush ?
        PendingRequest* pRequest = SC_PendingRequestCreate(requestId,
                                                           &OpcUa_OpenSecureChannelResponse_EncodeableType,
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

SOPC_StatusCode Read_OpenSecureChannelReponse(SC_ClientConnection* cConnection,
                                              PendingRequest*      pRequest)
{
    assert(cConnection != NULL &&
           pRequest != NULL && pRequest->responseType != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_OpenSecureChannelResponse* encObj = NULL;
    SOPC_EncodeableType* receivedType = NULL;

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
                cConnection->instance->currentSecuToken.createdAt = SOPC_DateTime_ToInt64(&encObj->SecurityToken.CreatedAt);
                cConnection->instance->currentSecuToken.revisedLifetime = encObj->SecurityToken.RevisedLifetime;
            }
        }else{
            // NULL SC ID !
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    if(status == STATUS_OK && cConnection->securityMode != OpcUa_MessageSecurityMode_None){
        uint32_t encryptKeyLength = 0, signKeyLength = 0, initVectorLength = 0;
        SC_SecurityKeySet *pks = NULL;
        status = CryptoProvider_DeriveGetLengths(cConnection->instance->currentCryptoProvider,
                                                 &encryptKeyLength,
                                                 &signKeyLength,
                                                 &initVectorLength);
        if(status == STATUS_OK && encObj->ServerNonce.Length > 0){
            cConnection->instance->currentSecuKeySets.receiverKeySet = KeySet_Create();
            cConnection->instance->currentSecuKeySets.senderKeySet = KeySet_Create();
            pks = cConnection->instance->currentSecuKeySets.receiverKeySet;
            if(NULL != pks) {
                pks->signKey = SecretBuffer_New(signKeyLength);
                pks->encryptKey = SecretBuffer_New(encryptKeyLength);
                pks->initVector = SecretBuffer_New(initVectorLength);
            }
            pks = cConnection->instance->currentSecuKeySets.senderKeySet;
            if(NULL != pks) {
                pks->signKey = SecretBuffer_New(signKeyLength);
                pks->encryptKey = SecretBuffer_New(encryptKeyLength);
                pks->initVector = SecretBuffer_New(initVectorLength);
            }
            status = CryptoProvider_DeriveKeySetsClient(cConnection->instance->currentCryptoProvider,
                                                        cConnection->instance->currentNonce,
                                                        encObj->ServerNonce.Data,
                                                        encObj->ServerNonce.Length,
                                                        cConnection->instance->currentSecuKeySets.senderKeySet,
                                                        cConnection->instance->currentSecuKeySets.receiverKeySet);
        }
    }

    OpcUa_OpenSecureChannelResponse_Clear(encObj);
    free(encObj);

    return status;
}

SOPC_StatusCode Receive_OpenSecureChannelResponse(SC_ClientConnection* cConnection,
                                                  SOPC_MsgBuffer*      transportMsgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    const uint32_t validateSenderCertificateTrue = 1; // True: always activated as indicated in API
    const uint32_t isSymmetricFalse = FALSE;
    const uint32_t isPrecCryptoDataFalse = FALSE; // TODO: add guarantee we are treating last OPN sent: using pending requests ?
    uint32_t requestId = 0;
    uint32_t snPosition = 0;
    PendingRequest* pRequest = NULL;

    if(cConnection != NULL && transportMsgBuffer != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK &&
       transportMsgBuffer->isFinal != SOPC_Msg_Chunk_Final){
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
        status = SOPC_UInt32_Read(&requestId, cConnection->instance->receptionBuffers);
    }

    if(status == STATUS_OK){
        // Retrieve associated pending request
        pRequest = SLinkedList_Remove(cConnection->pendingRequests, requestId);
        if(pRequest == NULL){
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
    }else{
        // Trace / channel CB
    }

    if(status == STATUS_OK){
        // Compute max body size for next messages (symmetric encryption)
        status = SC_SetMaxBodySize(cConnection->instance, 1);
    }

    // Reset reception buffers for next messages
    MsgBuffers_Reset(cConnection->instance->receptionBuffers);

    return status;
}

SOPC_StatusCode Receive_ServiceResponse(SC_ClientConnection* cConnection,
                                        SOPC_MsgBuffer*      transportMsgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t  abortReqPresence = 0;
    uint32_t abortedRequestId = 0;
    SOPC_StatusCode abortReqStatus = STATUS_NOK;
    uint32_t requestId = 0;
    uint8_t requestToRemove = FALSE;
    SOPC_String reason;
    SOPC_String_Initialize(&reason);

    PendingRequest* pRequest = NULL;
    SOPC_EncodeableType* recEncType = NULL;
    void* encObj = NULL;

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(cConnection != NULL){
        status = SC_CheckAbortChunk(cConnection->instance->receptionBuffers,
                                    &reason);
        // Decoded request id to be aborted
        abortReqPresence = 1;
        abortedRequestId = requestId;

        if(status != STATUS_OK){
            abortReqStatus = status;
            //TODO: report (trace)
            // Reset buffer since next message should be a new message
        }
    }

    if(status == STATUS_OK){
        status = SC_DecryptSecureMessage(cConnection->instance,
                                         transportMsgBuffer,
                                         &requestId);
    }

    if(status == STATUS_OK){
        status = SC_CheckPrecChunk(cConnection->instance->receptionBuffers,
                                   requestId,
                                   &abortReqPresence,
                                   &abortedRequestId);
    }

    if(abortReqPresence != FALSE){
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
        // Reset since we will not decode the message in this case (aborted in last chunk)
        MsgBuffers_Reset(cConnection->instance->receptionBuffers);
        status = STATUS_NOK;
    }

    if(status == STATUS_OK){
        if(cConnection->instance->receptionBuffers->isFinal == SOPC_Msg_Chunk_Final){
            // Retrieve associated pending request for current chunk which is final
            pRequest = SLinkedList_Remove(cConnection->pendingRequests, requestId);
            requestToRemove = 1; // True
            if(pRequest == NULL){
                status = STATUS_NOK;
            }
        }else if(cConnection->instance->receptionBuffers->isFinal == SOPC_Msg_Chunk_Intermediate &&
                cConnection->instance->receptionBuffers->nbChunks == 1){
            // When it is the first chunk and it is intermediate we have to check request id is valid
            //  otherwise request id already validated before and not pending request not necessary
            pRequest = SLinkedList_Find(cConnection->pendingRequests, requestId);
            if(pRequest == NULL){
                // Error: unknown request id !
                // TODO: trace + callback for audit ?

                // Reset since we will not decode the chunk in this case
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
                                    &OpcUa_ServiceFault_EncodeableType,
                                    &recEncType,
                                    &encObj);
        }else{
            status = SC_DecodeChunk(cConnection->instance->receptionBuffers,
                                    requestId,
                                    pRequest->responseType,
                                    &OpcUa_ServiceFault_EncodeableType,
                                    &recEncType,
                                    &encObj);
            // TODO: check status before ?
            if(pRequest->callback != NULL && requestToRemove != FALSE){
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

    SOPC_String_Clear(&reason);
    return status;
}

SOPC_StatusCode OnTransportEvent_CB(void*           connection,
                                    void*           callbackData,
                                    ConnectionEvent event,
                                    SOPC_MsgBuffer* msgBuffer,
                                    SOPC_StatusCode status)
{
    SC_ClientConnection* cConnection = (SC_ClientConnection*) callbackData;
    TCP_UA_Connection* tcpConnection = (TCP_UA_Connection*) connection;
    SOPC_StatusCode retStatus = STATUS_OK;
    assert(cConnection->instance->transportConnection == tcpConnection);
    switch(event){
        case ConnectionEvent_Connected:
            assert(status == STATUS_OK);
            assert(cConnection->instance->state == SC_Connection_Connecting_Transport);
            retStatus = SC_InitApplicationIdentities
                         (cConnection->instance,
                          cConnection->clientCertificate,
                          cConnection->clientKey,
                          cConnection->serverCertificate);
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
            retStatus = cConnection->callback(cConnection,
                                              cConnection->callbackData,
                                              SOPC_ConnectionEvent_Disconnected,
                                              OpcUa_BadSecureChannelClosed);
            break;

        case ConnectionEvent_Message:
            assert(status == STATUS_OK);
            switch(msgBuffer->secureType){
                case SOPC_OpenSecureChannel:
                    if(cConnection->instance->state == SC_Connection_Connecting_Secure){
                        // Receive Open Secure Channel response
                        retStatus = Receive_OpenSecureChannelResponse(cConnection, msgBuffer);
                        if(retStatus == STATUS_OK){
                            cConnection->instance->state = SC_Connection_Connected;
                            // TODO: cases in which retStatus != OK should be sent ?
                            retStatus = cConnection->callback(cConnection,
                                                              cConnection->callbackData,
                                                              SOPC_ConnectionEvent_Connected,
                                                              retStatus);
                        }
                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
                case SOPC_CloseSecureChannel:
                    if(cConnection->instance->state == SC_Connection_Connected){
                        assert(FALSE);
                    }else{
                        retStatus = STATUS_INVALID_RCV_PARAMETER;
                    }
                    break;
                case SOPC_SecureMessage:
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
            assert(FALSE);
    }
    return retStatus;
}

SOPC_StatusCode SC_Client_Connect(SC_ClientConnection*      connection,
                                  const char*               uri,
                                  const PKIProvider*        pki,
                                  const Certificate*        crt_cli,
                                  const AsymmetricKey*      key_priv_cli,
                                  const Certificate*        crt_srv,
                                  OpcUa_MessageSecurityMode securityMode,
                                  const char*               securityPolicy,
                                  uint32_t                  requestedLifetime,
                                  SC_ConnectionEvent_CB*    callback,
                                  void*                     callbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(connection != NULL &&
       connection->instance != NULL &&
       connection->instance->state == SC_Connection_Disconnected &&
       uri != NULL &&
       securityMode != OpcUa_MessageSecurityMode_Invalid &&
       securityPolicy != NULL &&
       requestedLifetime > 0 &&
       ((crt_cli != NULL &&
         key_priv_cli != NULL &&
         crt_srv != NULL &&
         pki != NULL)
       || securityMode == OpcUa_MessageSecurityMode_None))
    {
        Mutex_Lock(&connection->mutex);
        if(connection->clientCertificate == NULL &&
           connection->clientKey == NULL &&
           connection->serverCertificate == NULL &&
           connection->securityMode == OpcUa_MessageSecurityMode_Invalid &&
           connection->securityPolicy.Length <= 0 &&
           connection->callback == NULL &&
           connection->callbackData == NULL)
        {
            connection->pkiProvider = pki;

            status = SOPC_String_InitializeFromCString(&connection->securityPolicy, securityPolicy);

            if(STATUS_OK == status){
                // Create CryptoProvider and KeyManager
                connection->instance->currentCryptoProvider =
                        CryptoProvider_Create
                            (SOPC_String_GetRawCString(&connection->securityPolicy));

                if(connection->instance->currentCryptoProvider == NULL){
                    status = STATUS_NOK;
                }
            }
            
            // FIXME: this should be done elsewhere
            connection->clientKey = (AsymmetricKey *)key_priv_cli; // TODO: const override
            connection->clientCertificate = crt_cli;
            connection->serverCertificate = crt_srv;

            if(status == STATUS_OK){
                if(securityMode != OpcUa_MessageSecurityMode_Invalid){
                    connection->securityMode = securityMode;
                }else{
                    status = STATUS_NOK;
                }
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

        Mutex_Unlock(&connection->mutex);
    }
    return status;
}

SOPC_StatusCode SC_Client_Disconnect(SC_ClientConnection* cConnection)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(cConnection != NULL && cConnection->instance != NULL &&
       cConnection->instance->transportConnection != NULL)
    {
        Mutex_Lock(&cConnection->mutex);
        status = STATUS_OK;
        cConnection->instance->state = SC_Connection_Disconnected;
        cConnection->securityMode = OpcUa_MessageSecurityMode_Invalid;
        cConnection->callback = NULL;
        cConnection->callbackData = NULL;
        cConnection->pkiProvider = NULL;
        cConnection->serverCertificate = NULL;
        cConnection->clientCertificate = NULL;
        cConnection->clientKey = NULL;
        SLinkedList_Clear(cConnection->pendingRequests);
        SOPC_String_Clear(&cConnection->securityPolicy);
        TCP_UA_Connection_Disconnect(cConnection->instance->transportConnection);
        Mutex_Unlock(&cConnection->mutex);
    }
    return status;
}

SOPC_StatusCode SC_Send_Request(SC_ClientConnection* connection,
                                SOPC_EncodeableType* requestType,
                                void*                request,
                                SOPC_EncodeableType* responseType,
                                uint32_t             timeout,
                                SC_ResponseEvent_CB* callback,
                                void*                callbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t requestId = 0;
    if(connection != NULL &&
       requestType != NULL &&
       request != NULL)
    {
        Mutex_Lock(&connection->mutex);

        requestId = GetNextRequestId(connection->instance);
        status = SC_EncodeSecureMessage(connection->instance,
                                        requestType,
                                        request,
                                        requestId);

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
            status = SC_FlushSecureMsgBuffer(connection->instance->sendingBuffer, SOPC_Msg_Chunk_Final);
        }

        Mutex_Unlock(&connection->mutex);
    }

    return status;

}
