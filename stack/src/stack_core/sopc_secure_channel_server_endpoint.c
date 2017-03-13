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

#include "sopc_secure_channel_server_endpoint.h"
#include "sopc_endpoint.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sopc_tcp_ua_listener.h"
#include "sopc_types.h"
#include "sopc_encoder.h"
#include "crypto_profiles.h"
#include "sopc_action_queue_manager.h"

typedef struct TransportEvent_CallbackData {
    SC_ServerEndpoint* endpoint;
    SC_Connection*     scConnection;
    uint32_t           scConnectionId;
} TransportEvent_CallbackData;

typedef struct SOPC_Action_ServiceResponseSendData {
    SC_Connection*               scConnection;
    uint32_t                     requestId;
    SOPC_EncodeableType*         responseType;
    SOPC_MsgBuffers*             respBuffers;
    SOPC_Socket_EndOperation_CB* endSendCallback;
    void*                        endSendCallbackData;
} SOPC_Action_ServiceResponseSendData;

TransportEvent_CallbackData* Create_TransportEventCbData(SC_ServerEndpoint* endpoint,
                                                         SC_Connection*     scConnection,
                                                         uint32_t           scConnectionId)
{
    TransportEvent_CallbackData* result = malloc(sizeof(TransportEvent_CallbackData));
    if(NULL != result){
        result->endpoint = endpoint;
        result->scConnection = scConnection;
        result->scConnectionId = scConnectionId;
    }
    return result;
}

void Delete_TransportEventCbData(TransportEvent_CallbackData* cbData){
    if(NULL != cbData){
        free(cbData);
    }
}

SOPC_StatusCode Read_OpenSecureChannelRequest(SC_Connection*       scConnection,
                                              SOPC_SecurityPolicy* secuPolicy,
                                              uint8_t              senderCertPresence,
                                              uint8_t              receiverCertPresence,
                                              uint32_t*            requestHandle)
{
    assert(scConnection != NULL && secuPolicy != NULL && requestHandle != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    OpcUa_OpenSecureChannelRequest* encObj = NULL;
    SOPC_EncodeableType* receivedType = NULL;

    status = SC_DecodeMsgBody(scConnection->receptionBuffers,
                              &scConnection->receptionBuffers->nsTable,
                              NULL,
                              &OpcUa_OpenSecureChannelRequest_EncodeableType,
                              NULL,
                              &receivedType,
                              (void**) &encObj);
    if(STATUS_OK == status){
        status = SC_CheckReceivedProtocolVersion(scConnection, encObj->ClientProtocolVersion);
    }

    if(STATUS_OK == status){
        *requestHandle = encObj->RequestHeader.RequestHandle;
        // TODO: in case of renew: when moving from current to prec ?
        if(encObj->RequestType == OpcUa_SecurityTokenRequestType_Issue){
            switch(encObj->SecurityMode){
                case OpcUa_MessageSecurityMode_Invalid:
                    status = STATUS_INVALID_RCV_PARAMETER;
                    break;
                case OpcUa_MessageSecurityMode_None:
                    if((SECURITY_MODE_NONE_MASK & secuPolicy->securityModes) != 0){
                        if(senderCertPresence == FALSE && receiverCertPresence == FALSE){
                            status = STATUS_OK;
                        }else{
                            // Certificates shall be null when Secu mode = None (part 6. table 27)
                            status = STATUS_NOK;
                        }
                    }
                    break;
                case OpcUa_MessageSecurityMode_Sign:
                    if((SECURITY_MODE_SIGN_MASK & secuPolicy->securityModes) != 0){
                        if(senderCertPresence != FALSE && receiverCertPresence != FALSE){
                            status = STATUS_OK;
                        }else{
                            // Certificates shall not be null when Secu mode = Sign (part 6. table 27)
                            // Note: message is always signed and encrypted in asymmetric security headers
                            // since it is necessary for establishment of symmetric encryption
                            status = STATUS_NOK;
                        }
                    }
                    break;
                case OpcUa_MessageSecurityMode_SignAndEncrypt:
                    if((SECURITY_MODE_SIGNANDENCRYPT_MASK & secuPolicy->securityModes) != 0){
                        if(senderCertPresence != FALSE && receiverCertPresence != FALSE){
                            status = STATUS_OK;
                        }else{
                            // Certificates shall not be null when Secu mode = Sign (part 6. table 27)
                            // Note: message is always signed and encrypted in asymmetric security headers
                            // since it is necessary for establishment of symmetric encryption
                            status = STATUS_NOK;
                        }
                    }
                    break;
            }
            if(STATUS_OK == status){
                scConnection->currentSecuMode = encObj->SecurityMode;
            }

            if(STATUS_OK == status){
                if(encObj->RequestedLifetime > OPCUA_SECURITYTOKEN_LIFETIME_MAX){
                    scConnection->currentSecuToken.revisedLifetime = OPCUA_SECURITYTOKEN_LIFETIME_MAX;
                }else if(encObj->RequestedLifetime < OPCUA_SECURITYTOKEN_LIFETIME_MIN){
                    scConnection->currentSecuToken.revisedLifetime = OPCUA_SECURITYTOKEN_LIFETIME_MIN;
                }else{
                    scConnection->currentSecuToken.revisedLifetime = encObj->RequestedLifetime;
                }
            }


            if(STATUS_OK == status && scConnection->currentSecuMode != OpcUa_MessageSecurityMode_None){
                uint32_t encryptKeyLength = 0, signKeyLength = 0, initVectorLength = 0;
                SC_SecurityKeySet *pks = NULL;


                status = CryptoProvider_GenerateSecureChannelNonce(scConnection->currentCryptoProvider,
                                                                   &scConnection->currentNonce);

                if(STATUS_OK == status){
                    status = CryptoProvider_DeriveGetLengths(scConnection->currentCryptoProvider,
                                                             &encryptKeyLength,
                                                             &signKeyLength,
                                                             &initVectorLength);
                }

                if(STATUS_OK == status && encObj->ClientNonce.Length > 0){
                    scConnection->currentSecuKeySets.receiverKeySet = KeySet_Create();
                    scConnection->currentSecuKeySets.senderKeySet = KeySet_Create();
                    pks = scConnection->currentSecuKeySets.receiverKeySet;
                    if(NULL != pks) {
                        pks->signKey = SecretBuffer_New(signKeyLength);
                        pks->encryptKey = SecretBuffer_New(encryptKeyLength);
                        pks->initVector = SecretBuffer_New(initVectorLength);
                    }
                    pks = scConnection->currentSecuKeySets.senderKeySet;
                    if(NULL != pks) {
                        pks->signKey = SecretBuffer_New(signKeyLength);
                        pks->encryptKey = SecretBuffer_New(encryptKeyLength);
                        pks->initVector = SecretBuffer_New(initVectorLength);
                    }
                    status = CryptoProvider_DeriveKeySetsServer(scConnection->currentCryptoProvider,
                                                                encObj->ClientNonce.Data,
                                                                encObj->ClientNonce.Length,
                                                                scConnection->currentNonce,
                                                                scConnection->currentSecuKeySets.receiverKeySet,
                                                                scConnection->currentSecuKeySets.senderKeySet);
                }
            }

        }else{
            // TODO: Renew: not managed for now
            assert(FALSE);
        }
    }

    OpcUa_OpenSecureChannelRequest_Clear(encObj);
    free(encObj);

    return status;
}

SOPC_StatusCode Receive_OpenSecureChannelRequest(SC_ServerEndpoint* sEndpoint,
                                                 SC_Connection*     scConnection,
                                                 SOPC_MsgBuffer*    transportMsgBuffer,
                                                 uint32_t*          requestId,
                                                 uint32_t*          requestHandle)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    const uint32_t validateSenderCertificateTrue = 1; // True: mandatory
    const uint32_t isSymmetricFalse = FALSE;
    const uint32_t isPrecCryptoDataFalse = FALSE; // TODO: add guarantee we are treating last OPN sent: using pending requests ?
    uint32_t idx = 0;
    int32_t secuPolicyComparison = 0;
    uint8_t senderCertPresence = FALSE;
    uint8_t receiverCertPresence = FALSE;
    SOPC_String strSecuPolicy;
    SOPC_String_Initialize(&strSecuPolicy);
    SOPC_SecurityPolicy* secuPolicy = NULL;
    uint32_t secureChannelId = 0;
    uint32_t snPosition = 0;

    if(scConnection != NULL && transportMsgBuffer != NULL){
        status = STATUS_OK;
    }

    if(STATUS_OK == status &&
       transportMsgBuffer->isFinal != SOPC_Msg_Chunk_Final){
        // OPN request/response must be in one chunk only
        status = STATUS_INVALID_RCV_PARAMETER;
    }

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(STATUS_OK == status){
        status = SOPC_UInt32_Read(&secureChannelId, transportMsgBuffer);
    }

    if(STATUS_OK == status){
        // Decode asymmetric security header:
        // - Check security policy
        // - Validate other app certificate
        // - Check current app certificate thumbprint
        status = SC_DecodeAsymSecurityHeader_SecurityPolicy(scConnection, transportMsgBuffer, &strSecuPolicy);

        // Check security policy correct
        // TODO: check that in case of renew the security policy could change (=> if not just check same as old one)
        secuPolicyComparison = -1;
        for(idx = 0; idx < sEndpoint->nbSecurityPolicies && secuPolicyComparison != 0; idx++){
            secuPolicy = &sEndpoint->securityPolicies[idx];
            if(STATUS_OK == SOPC_String_Compare(&strSecuPolicy,
                                                &secuPolicy->securityPolicy,
                                                1,
                                                &secuPolicyComparison)){
            }else{
                // Do not consider result
                secuPolicyComparison = -1;
            }
        }

        // TODO: in case of renew: when moving from current to prec ?

        if(secuPolicyComparison == 0 && secuPolicy != NULL && scConnection->currentSecuPolicy.Length <= 0){
            status = SOPC_String_AttachFrom(&scConnection->currentSecuPolicy, &secuPolicy->securityPolicy);
        }else{
            status = STATUS_NOK;
        }
    }

    // TODO: in case of renew: when moving from current to prec ?
    if(STATUS_OK == status && scConnection->currentCryptoProvider == NULL){
        scConnection->currentCryptoProvider = CryptoProvider_Create(SOPC_String_GetRawCString(&secuPolicy->securityPolicy));
        if(scConnection->currentCryptoProvider == NULL){
            status = STATUS_NOK;
        }
    }else{
        status = STATUS_NOK;
    }

    if(STATUS_OK == status){
        status = SC_DecodeAsymSecurityHeader_Certificates(scConnection, transportMsgBuffer, sEndpoint->pkiProvider,
                                                          validateSenderCertificateTrue, // always validate server cert
                                                          FALSE, // enforceSecuMode == FALSE => unknown secu mode for now
                                                          &snPosition,
                                                          &senderCertPresence,
                                                          &receiverCertPresence); // Sender and receiver certificates presence depend on secu mode
    }

    if(STATUS_OK == status){
        // Since the security mode is not known before decoding (and possibly decrypting) the message
        // we have to deduce it from the certificates presence (None or SignAndEncrypt only possible in OPN)
        if(senderCertPresence == FALSE && receiverCertPresence == FALSE){
            scConnection->currentSecuMode = OpcUa_MessageSecurityMode_None;
        }else if(senderCertPresence != FALSE && receiverCertPresence != FALSE){
            scConnection->currentSecuMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    if(STATUS_OK == status){
        // Decrypt message content and store complete message in secure connection buffer
        status = SC_DecryptMsg(scConnection,
                               transportMsgBuffer,
                               snPosition,
                               isSymmetricFalse,
                               isPrecCryptoDataFalse);
    }

    if(STATUS_OK == status){
        // Check decrypted message signature
        status = SC_VerifyMsgSignature(scConnection,
                                       isSymmetricFalse,
                                       isPrecCryptoDataFalse); // IsAsymmetric = TRUE
    }

    // TODO: differentiate errors after this point must generate a ServiceFaultMessage whereas precedent errors
    //       must generate a transport error and close the connection

    if(STATUS_OK == status){
        status = SC_CheckSeqNumReceived(scConnection);
    }

    if(STATUS_OK == status){
        // Retrieve request id
        status = SOPC_UInt32_Read(requestId, scConnection->receptionBuffers);
    }

    if(STATUS_OK == status){
        // Decode message body content
        status = Read_OpenSecureChannelRequest(scConnection,
                                               secuPolicy,
                                               senderCertPresence,
                                               receiverCertPresence,
                                               requestHandle);
    }

    // Set the secure channel id
    if(STATUS_OK == status){
        if(secureChannelId == 0 && scConnection->secureChannelId == 0){
            // Randomize secure channel ids (table 26 part 6)
            uint32_t newSecureChannelId = 0;
            uint8_t occupiedId = FALSE;
            uint8_t attempts = 5; // attempts to find a non conflicting secure Id
            SLinkedListIterator it = NULL;
            SC_Connection* otherConnection = NULL;
            while(scConnection->secureChannelId == 0 && attempts > 0){
                attempts--;
                CryptoProvider_GenerateRandomID(scConnection->currentCryptoProvider, &newSecureChannelId);
                occupiedId = FALSE;
                // A server cannot attribute 0 as secure channel id:
                //  not so clear but implied by 6.7.6 part 6: "may be 0 if the Message is an OPN"
                if(newSecureChannelId != 0){
                    // Check if other channels already use the random id in existing connections
                    it = SLinkedList_GetIterator(sEndpoint->secureChannelConnections);
                    otherConnection = SLinkedList_Next(&it);
                    while(occupiedId == FALSE && otherConnection != NULL){
                        if(otherConnection->secureChannelId == newSecureChannelId){
                            occupiedId = 1; // TRUE
                        }
                        otherConnection = SLinkedList_Next(&it);
                    }
                    if(occupiedId == FALSE){
                        // Id is not used by another channel in the endpoint:
                        scConnection->secureChannelId = newSecureChannelId;
                    }
                }
            }
            if(scConnection->secureChannelId == 0){
                status = STATUS_NOK;
            }
        }else if(scConnection->secureChannelId != secureChannelId){
            // Different Id between client and server: invalid case (id never changes on same connection instance)
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    // TODO: in case of renew: when moving from current to prec ?
    if(STATUS_OK == status && scConnection->currentSecuToken.channelId == 0){
        scConnection->currentSecuToken.channelId = scConnection->secureChannelId;
    }else if(scConnection->currentSecuToken.channelId != scConnection->secureChannelId){
        status = STATUS_NOK;
    }

    // TODO: in case of renew: when moving from current to prec ?
    if(STATUS_OK == status && scConnection->currentSecuToken.tokenId == 0){
        // Generate token Id
        uint32_t newTokenId = 0;
        CryptoProvider_GenerateRandomID(scConnection->currentCryptoProvider, &newTokenId);
        if(newTokenId != scConnection->precSecuToken.tokenId){
            scConnection->currentSecuToken.tokenId = newTokenId;
        }else{
            // Second attempt and last !
            CryptoProvider_GenerateRandomID(scConnection->currentCryptoProvider, &newTokenId);
            if(newTokenId != scConnection->precSecuToken.tokenId){
                scConnection->currentSecuToken.tokenId = newTokenId;
            }
        }

        if(scConnection->currentSecuToken.tokenId == 0){
            status = STATUS_NOK;
        }
    }else{
        status = STATUS_NOK;
    }

    if(STATUS_OK == status && scConnection->currentSecuToken.createdAt == 0){
        scConnection->currentSecuToken.createdAt = 0; // TODO: use current date
    }else{
        status = STATUS_NOK;
    }

    // Reset reception buffers for next messages
    MsgBuffers_Reset(scConnection->receptionBuffers);

    SOPC_String_Clear(&strSecuPolicy);

    return status;
}


SOPC_StatusCode Write_OpenSecureChannelResponse(SC_Connection*   scConnection,
                                                SOPC_MsgBuffers* msgBuffers,
                                                uint32_t         requestHandle)
{
    SOPC_StatusCode status = STATUS_OK;
    uint8_t* bytes = NULL;
    OpcUa_OpenSecureChannelResponse openResponse;
    OpcUa_OpenSecureChannelResponse_Initialize(&openResponse);

    //// Encode response header
    // Encode 64 bits UtcTime => null ok ?
    SOPC_DateTime_Clear(&openResponse.ResponseHeader.Timestamp);
    // Encode requestHandler
    openResponse.ResponseHeader.RequestHandle = requestHandle;
    // Encode service result code: always OK since we should send tranport error or service fault in other cases
    openResponse.ResponseHeader.ServiceResult = STATUS_OK;
    // No service diagnostic (default)
    // No of string table = 0 (default)
    // String table = NULL (default)

    // Extension object: additional header => null node id => no content
    // !! Extensible parameter indicated in specification but Extension object in XML file !!
    // Encoding body byte:
    openResponse.ResponseHeader.AdditionalHeader.Encoding = SOPC_ExtObjBodyEncoding_None;
    // Type Id: Node Id
    openResponse.ResponseHeader.AdditionalHeader.TypeId.NodeId.IdentifierType = IdentifierType_Numeric;
    openResponse.ResponseHeader.AdditionalHeader.TypeId.NodeId.Data.Numeric = SOPC_Null_Id;

    //// Encode response content
    // Server protocol version
    openResponse.ServerProtocolVersion = scProtocolVersion;
    // Security token
    openResponse.SecurityToken.ChannelId = scConnection->currentSecuToken.channelId;
    openResponse.SecurityToken.TokenId = scConnection->currentSecuToken.tokenId;
    SOPC_DateTime_FromInt64(&openResponse.SecurityToken.CreatedAt, scConnection->currentSecuToken.createdAt);
    openResponse.SecurityToken.RevisedLifetime = scConnection->currentSecuToken.revisedLifetime;
    // Server nonce
    if(scConnection->currentSecuMode != OpcUa_MessageSecurityMode_None){
        bytes = SecretBuffer_Expose(scConnection->currentNonce);
        status = SOPC_ByteString_CopyFromBytes(&openResponse.ServerNonce,
                                               bytes,
                                               SecretBuffer_GetLength(scConnection->currentNonce));
    }
    if(status == STATUS_OK){
        status = SC_EncodeMsgBody(msgBuffers,
                                  &OpcUa_OpenSecureChannelResponse_EncodeableType,
                                  &openResponse);
    }

    if(scConnection->currentSecuMode != OpcUa_MessageSecurityMode_None){
        SecretBuffer_Unexpose(openResponse.ServerNonce.Data);
    }
    OpcUa_OpenSecureChannelResponse_Clear(&openResponse);

    return status;
}

SOPC_StatusCode Send_OpenSecureChannelResponse(SC_ServerEndpoint*           sEndpoint,
                                               SC_Connection*               scConnection,
                                               uint32_t                     requestId,
                                               uint32_t                     requestHandle,
                                               SOPC_Socket_EndOperation_CB* callback,
                                               void*                        callbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_MsgBuffers* msgBuffers = NULL;

    if(scConnection != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        msgBuffers = SC_CreateSendSecureBuffers(1, // OPN is not authorized to use more than one chunk
                                                scConnection->transportConnection->maxMessageSizeSnd,
                                                scConnection->transportConnection->sendBufferSize,
                                                scConnection,
                                                &sEndpoint->namespaces,
                                                sEndpoint->encodeableTypes);
        assert(msgBuffers->nbBuffers == 1); // <=> SOPC_MsgBuffer* type
    }

    // MaxBodySize to be computed prior any write in sending buffer
    //  => temporary value set before we know the asymmetric security header size (not encrypted headers part)
    if(status == STATUS_OK){
        status = SC_SetMaxBodySize(scConnection,
                                   UA_SECURE_MESSAGE_HEADER_LENGTH,
                                   msgBuffers->buffers->max_size,
                                   FALSE);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSecureMsgHeader(msgBuffers,
                                          SOPC_OpenSecureChannel,
                                          scConnection->secureChannelId);
    }

    if(status == STATUS_OK){
        status = SC_EncodeAsymmSecurityHeader(scConnection,
                                              msgBuffers,
                                              &scConnection->currentSecuPolicy);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSequenceHeader(msgBuffers, requestId);
    }

    // MaxBodySize to be recomputed now the non encrypted headers size is known
    if(status == STATUS_OK){
        status = SC_SetMaxBodySize(scConnection,
                                   msgBuffers->sequenceNumberPosition,
                                   msgBuffers->buffers->max_size,
                                   FALSE);
    }

    if(status == STATUS_OK){
        status = Write_OpenSecureChannelResponse(scConnection,
                                                 msgBuffers,
                                                 requestHandle);
    }


    if(STATUS_OK == status){
        status = SC_FlushSecureMsgBuffers(msgBuffers,
                                          0,
                                          0, // not used
                                          requestId,
                                          callback,
                                          (void*) callbackData);
    }

    if(status != STATUS_OK){
        MsgBuffers_Delete(&msgBuffers);
    }

    // Compute max body size for next messages (symmetric encryption)
    if(status == STATUS_OK){
        status = SC_SetMaxBodySize(scConnection,
                                   UA_SECURE_MESSAGE_HEADER_LENGTH + UA_SYMMETRIC_SECURITY_HEADER_LENGTH,
                                   msgBuffers->buffers->max_size,
                                   FALSE);
    }

    return status;
}

SOPC_StatusCode Receive_ServiceRequest(SC_Connection*        scConnection,
                                       SOPC_MsgBuffer*       transportMsgBuffer,
                                       uint32_t*             requestId,
                                       SOPC_EncodeableType** receivedEncType,
                                       void**                receivedEncObj)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t  abortReqPresence = 0;
    uint32_t abortedRequestId = 0;
    SOPC_String reason;
    SOPC_String_Initialize(&reason);

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(scConnection != NULL && transportMsgBuffer != NULL &&
       receivedEncType != NULL && receivedEncObj != NULL)
    {
        status = SC_CheckAbortChunk(scConnection->receptionBuffers,
                                            &reason);
        // Decoded request id to be aborted
        abortReqPresence = 1;
        abortedRequestId = *requestId;

        if(status != STATUS_OK){
            //TODO: report (trace with status)
        }
    }

    if(STATUS_OK == status){
        status = SC_DecryptSecureMessage(scConnection,
                                         transportMsgBuffer,
                                         requestId);
    }

    if(STATUS_OK == status){
        // Check if the request Id is still the same that precedent chunks for the message.
        // If it is not, abort and reset buffers.
        status = SC_CheckPrecChunk(scConnection->receptionBuffers,
                                   *requestId,
                                   &abortReqPresence,
                                   &abortedRequestId);
    }

    if(abortReqPresence != FALSE){
        // Note: status is OK if from a prec chunk or NOK if current chunk is abort chunk
        // Reset since we will not decode the message in this case (aborted in last chunk or invalid request id)
        MsgBuffers_Reset(scConnection->receptionBuffers);
    }

    if(STATUS_OK == status){
        status = SC_DecodeChunk(scConnection->receptionBuffers,
                                *requestId,
                                NULL,
                                NULL,
                                receivedEncType,
                                receivedEncObj);
    }

    SOPC_String_Clear(&reason);

    return status;
}

void SOPC_OpenSecureChannelResponse_Sent_CB(void*           arg,
                                            SOPC_StatusCode sendingStatus)
{
    TransportEvent_CallbackData* teventCbData = (TransportEvent_CallbackData*) arg;
    SC_ServerEndpoint* sEndpoint = NULL;
    SC_Connection* scConnection = NULL;
    if(STATUS_OK == sendingStatus){
        if(NULL != teventCbData &&
           NULL != teventCbData->endpoint && NULL != teventCbData->scConnection)
        {
            sEndpoint = teventCbData->endpoint;
            scConnection = teventCbData->scConnection;
            scConnection->state = SC_Connection_Connected;

            // TODO: differentiate renew from new ...
            if(NULL != sEndpoint->callback){
                sEndpoint->callback(sEndpoint, scConnection,
                                    sEndpoint->callbackData,
                                    SC_EndpointConnectionEvent_New,
                                    STATUS_OK, // TODO: manage other cases in same ActionFunction ? Other ?
                                    NULL, NULL, NULL);
            }
        }
    }else{
        OnConnectionTransportEvent_CB(arg,
                                      ConnectionEvent_Error,
                                      NULL,
                                      sendingStatus);
    }
}

SOPC_StatusCode OnConnectionTransportEvent_CB(void*           callbackData,
                                              ConnectionEvent event,
                                              SOPC_MsgBuffer* msgBuffer,
                                              SOPC_StatusCode status)
{
    TransportEvent_CallbackData* teventCbData = (TransportEvent_CallbackData*) callbackData;
    SC_ServerEndpoint* sEndpoint = NULL;
    SC_Connection* scConnection = NULL;
    uint32_t requestId = 0;
    SOPC_EncodeableType* receivedEncType = NULL;
    void* receivedEncObj = NULL;
    SOPC_StatusCode retStatus = STATUS_OK;
    uint8_t noneSecurityMode = !FALSE;
    uint32_t idx = 0;
    if(NULL != teventCbData &&
       NULL != teventCbData->endpoint && NULL != teventCbData->scConnection)
    {
        sEndpoint = teventCbData->endpoint;
        scConnection = teventCbData->scConnection;
        retStatus = status;
        // Record if there are only None security modes (no certificates needed in this case only)
        for(idx = 0; idx < sEndpoint->nbSecurityPolicies; idx++){
            if(sEndpoint->securityPolicies[idx].securityModes != SECURITY_MODE_NONE_MASK){
                noneSecurityMode = FALSE;
            }
        }

        switch(event){
            case ConnectionEvent_Connected:
                if(SC_Connection_Disconnected == scConnection->state){
                    // Configure secure connection for encoding / decoding messages
					if(status == STATUS_OK){

						// Set only server side identity for now
                    	status = SC_InitApplicationIdentities(scConnection,
                    	                                      noneSecurityMode,
                        	                                  sEndpoint->serverCertificate,
                            	                              sEndpoint->serverKey,
                                	                          NULL);
                	}
                    if(STATUS_OK == status){
                        status = SC_InitReceiveSecureBuffers(scConnection,
                                                             &sEndpoint->namespaces,
                                                             sEndpoint->encodeableTypes);
                    }
                    if(STATUS_OK == status){
                        scConnection->state = SC_Connection_Connecting_Secure;
                    }
                }
                break;
            case ConnectionEvent_Message:
                switch(msgBuffer->secureType){
                    case SOPC_OpenSecureChannel:
                        if(SC_Connection_Connecting_Secure == scConnection->state){
                            uint32_t requestId = 0;
                            uint32_t requestHandle = 0;

                            // Receive Open Secure Channel request
                            retStatus = Receive_OpenSecureChannelRequest(sEndpoint, scConnection, msgBuffer,
                                                                         &requestId, &requestHandle);

                            if(STATUS_OK == retStatus){
                                retStatus = Send_OpenSecureChannelResponse(sEndpoint,
                                                                           scConnection,
                                                                           requestId, requestHandle,
                                                                           SOPC_OpenSecureChannelResponse_Sent_CB,
                                                                           teventCbData);
                            }

                            if(STATUS_OK != retStatus){
                                // TODO: Regarding status and if it is before secu verif or after
                                //       a transport error (before) or a service fault (after) must be sent
                                OnConnectionTransportEvent_CB(callbackData,
                                                              ConnectionEvent_Error,
                                                              NULL,
                                                              status);
                            }

                        }else{
                            retStatus = STATUS_INVALID_RCV_PARAMETER;
                        }
                        break;
                    case SOPC_CloseSecureChannel:
                        if(SC_Connection_Connected == scConnection->state){
                            OnConnectionTransportEvent_CB(callbackData,
                                                          ConnectionEvent_Error,
                                                          NULL,
                                                          status);
                        }
                        break;
                    case SOPC_SecureMessage:
                        if(SC_Connection_Connected == scConnection->state){

                            retStatus = Receive_ServiceRequest(scConnection, msgBuffer, &requestId,
                                                               &receivedEncType, &receivedEncObj);

                            // TODO: Manage partial request / abort
                            if(NULL != sEndpoint->callback){
                                sEndpoint->callback(sEndpoint, scConnection,
                                                    sEndpoint->callbackData,
                                                    SC_EndpointConnectionEvent_Request,
                                                    retStatus,
                                                    &requestId, receivedEncType, receivedEncObj);
                            }
                            // Note: receivedEncObj to be cleared by application (SDK)
                        }else{
                            retStatus = STATUS_INVALID_RCV_PARAMETER;
                        }

                        break;
                }
                break;
            case ConnectionEvent_Error:
            case ConnectionEvent_Disconnected:
                if(scConnection->state != SC_Connection_Disconnected){
                    if(NULL != sEndpoint->callback){
                        sEndpoint->callback(sEndpoint, scConnection,
                                            sEndpoint->callbackData,
                                            SC_EndpointConnectionEvent_Disconnected,
                                            OpcUa_BadSecureChannelClosed,
                                            NULL, NULL, NULL);
                    }
                }
                scConnection->state = SC_Connection_Disconnected;
                // Remove secure connection from the endpoint
                if(scConnection ==  SLinkedList_RemoveFromId(sEndpoint->secureChannelConnections,
                                                       teventCbData->scConnectionId)){
                    SC_Delete(scConnection);
                    scConnection = NULL;
                }else{
                    // Connection not found !
                    assert(FALSE);
                }

                Delete_TransportEventCbData(teventCbData);
                break;
        }
    }
    return retStatus;
}

void SOPC_OperationEnd_ServiceResponse_CB(void*           arg,
                                          SOPC_StatusCode sendingRespStatus)
{
    assert(NULL != arg);
    SOPC_Action_ServiceResponseSendData* data = (SOPC_Action_ServiceResponseSendData*) arg;

    SC_CreateAction_ReleaseToken((void*)data->scConnection);

    data->endSendCallback(data->endSendCallbackData,
                          sendingRespStatus);
    free(data);
}

void SC_Send_Response(SOPC_Action_ServiceResponseSendData* sendData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t willReleaseToken = FALSE;

    SC_Connection*       scConnection = NULL;
    uint32_t             requestId = NULL;
    SOPC_EncodeableType* responseType = NULL;
    SOPC_MsgBuffers*     respBuffers = NULL;
    SOPC_Socket_EndOperation_CB*  endSendCallback = NULL;
    void*                         endSendCallbackData = NULL;

    if(NULL != sendData){
        scConnection = sendData->scConnection;
        requestId = sendData->requestId;
        responseType = sendData->responseType;
        respBuffers = sendData->respBuffers;
        endSendCallback = sendData->endSendCallback;
        endSendCallbackData = sendData->endSendCallbackData;
    }

    if(scConnection != NULL &&
       responseType != NULL &&
       respBuffers != NULL)
    {
        if(SC_Connection_Connected == scConnection->state){

            // Set request Id, used for socket transaction
            respBuffers->msgRequestId = requestId;

            status = SC_FlushSecureMsgBuffers(respBuffers,
                                              0,
                                              scConnection->currentSecuToken.tokenId,
                                              requestId,
                                              SOPC_OperationEnd_ServiceResponse_CB,
                                              (void*) sendData);

            // Token will be released by SOPC_OperationEnd_ServiceResponse_CB
            willReleaseToken = 1;

            if(STATUS_OK != status){
                SOPC_OperationEnd_ServiceResponse_CB((void*) sendData,
                                                     status);
            }

        }else{
            // != Connected
            status = OpcUa_BadServerNotConnected;
            endSendCallback(endSendCallbackData,
                            status);
        }

        if(FALSE == willReleaseToken){
            SC_CreateAction_ReleaseToken((void*)scConnection);
        }

    }else{
        status = STATUS_INVALID_STATE;
        endSendCallback(endSendCallbackData,
                                    status);
    }
}

void SC_Action_Send_Response(void* arg){
    SOPC_Action_ServiceResponseSendData* data = (SOPC_Action_ServiceResponseSendData*) arg;
    assert(NULL != data);
    SC_Send_Response(data);
}

SOPC_StatusCode SC_CreateAction_Send_Response(SC_Connection*               scConnection,
                                              uint32_t                     requestId,
                                              SOPC_EncodeableType*         responseType,
                                              SOPC_MsgBuffers*             respBuffers,
                                              SOPC_Socket_EndOperation_CB* endSendCallback,
                                              void*                        endSendCallbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Action_ServiceResponseSendData* data = NULL;
    if(scConnection != NULL &&
       responseType != NULL &&
       respBuffers != NULL)
    {
        status = STATUS_NOK;
        data = malloc(sizeof(SOPC_Action_ServiceResponseSendData));
        if(data != NULL){
            data->scConnection = scConnection;
            data->requestId = requestId;
            data->responseType = responseType;
            data->respBuffers = respBuffers;
            data->endSendCallback = endSendCallback;
            data->endSendCallbackData = endSendCallbackData;
            status = SOPC_Action_BlockingEnqueue(scConnection->msgQueue,
                                                 SC_Action_Send_Response,
                                                 (void*) data,
                                                 "Send response");
            if(STATUS_OK == status){
                status = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                           SC_Action_TreateMsgQueue,
                                                           (void*) scConnection,
                                                           "Treat message queue due to new 'Send repsonse'");
            }else{
                free(data);
            }
        }
    }
    return status;
}

SOPC_StatusCode SC_ServerEndpoint_EncodeResponse(uint32_t             secureChannelId,
                                                 uint32_t             requestId,
                                                 SOPC_EncodeableType* responseType,
                                                 void*                response,
                                                 SOPC_MsgBuffers*     msgBuffers)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != responseType && NULL != response && NULL != msgBuffers){
        status = SC_EncodeSecureMessage(msgBuffers,
                                        responseType,
                                        response,
                                        secureChannelId,
                                        0, // To be set before signature/encryption: token Id
                                        requestId);
    }
    return status;
}


SOPC_StatusCode AcceptedNewConnection(SC_ServerEndpoint* sEndpoint,
                                      TCP_UA_Connection* newTcpConnection){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_Connection* scConnection = NULL;
    TransportEvent_CallbackData* connectionCbData = NULL;
    if(sEndpoint != NULL && newTcpConnection != NULL){
        scConnection = SC_Create(newTcpConnection);
        if(NULL == scConnection){
            status = STATUS_NOK;
        }else{
            sEndpoint->lastSecureConnectionId++;
            connectionCbData = Create_TransportEventCbData(sEndpoint,
                                                           scConnection,
                                                           sEndpoint->lastSecureConnectionId);
            status = TCP_UA_Connection_AcceptedSetCallback(newTcpConnection,
                                                           OnConnectionTransportEvent_CB,
                                                           connectionCbData);
            // TODO: activate a timer for end of SC connection timeout (in SC_Connection)
        }

        if(STATUS_OK == status){
            // Add as a new secure connection to the endpoint
            SLinkedList_Prepend(sEndpoint->secureChannelConnections,
                            sEndpoint->lastSecureConnectionId,
                            scConnection);
        }else{
            Delete_TransportEventCbData(connectionCbData);
            connectionCbData = NULL;
            scConnection->transportConnection = NULL; // connection provided as parameter cannot be freed here
            SC_Delete(scConnection);
            scConnection = NULL;
        }
    }
    return status;
}

SOPC_StatusCode OnListenerEvent_CB(SC_ServerEndpoint* sEndpoint,
                                   TCP_ListenerEvent  event,
                                   SOPC_StatusCode    status,
                                   TCP_UA_Connection* newTcpConnection){
    SOPC_StatusCode retStatus = STATUS_OK;
    switch(event){
        case TCP_ListenerEvent_Error:
            SC_ServerEndpoint_Close(sEndpoint);
            if(NULL != sEndpoint->callback){
                sEndpoint->callback(sEndpoint, NULL,
                                    sEndpoint->callbackData,
                                    SC_EndpointListenerEvent_Closed,
                                    status,
                                    NULL, NULL, NULL);
            }
            break;
        case TCP_ListenerEvent_Closed:
            if(NULL != sEndpoint->callback){
                sEndpoint->callback(sEndpoint, NULL,
                                    sEndpoint->callbackData,
                                    SC_EndpointListenerEvent_Closed,
                                    status,
                                    NULL, NULL, NULL);
            }
            break;
        case TCP_ListenerEvent_Opened:
            if(NULL != sEndpoint->callback){
                sEndpoint->callback(sEndpoint, NULL,
                                    sEndpoint->callbackData,
                                    SC_EndpointListenerEvent_Opened,
                                    status,
                                    NULL, NULL, NULL);
            }
            break;
        case TCP_ListenerEvent_Connect:
            if(NULL != newTcpConnection){
                retStatus = AcceptedNewConnection(sEndpoint, newTcpConnection);
            }else{
                retStatus = STATUS_INVALID_PARAMETERS;
            }
            if(STATUS_OK != retStatus){
                TCP_UA_Connection_Delete(newTcpConnection);
            }
            break;
    }
    return retStatus;
}

SC_ServerEndpoint* SC_ServerEndpoint_Create(){
    SC_ServerEndpoint* result = NULL;
    TCP_UA_Listener* listener = TCP_UA_Listener_Create(scProtocolVersion);
    if(NULL != listener){
        result = malloc(sizeof(SC_ServerEndpoint));
        if(NULL != result){
            memset(result, 0, sizeof(SC_ServerEndpoint));
            result->transportListener = listener;
            result->state = SC_Endpoint_Closed;
            result->secureChannelConnections = SLinkedList_Create(OPCUA_ENDPOINT_MAXCONNECTIONS);

            if(NULL == result->secureChannelConnections){
                free(result);
                result= NULL;
            }
        }
    }
    return result;
}

SOPC_StatusCode SC_ServerEndpoint_Configure(SC_ServerEndpoint*     endpoint,
                                            SOPC_NamespaceTable*   namespaceTable,
                                            SOPC_EncodeableType**  encodeableTypes){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(endpoint != NULL){
        if(namespaceTable != NULL){
            status = Namespace_AttachTable(&endpoint->namespaces, namespaceTable);
        }else{
            status = STATUS_OK;
        }
        endpoint->encodeableTypes = encodeableTypes;
    }
    return status;
}

SOPC_StatusCode SC_ServerEndpoint_Open(SC_ServerEndpoint*   endpoint,
                                       const char*          endpointURL,
                                       const PKIProvider*   pki,
                                       const Certificate*   serverCertificate,
                                       const AsymmetricKey* serverKey,
                                       uint8_t              nbSecurityPolicies,
                                       SOPC_SecurityPolicy* securityPolicies,
                                       SC_EndpointEvent_CB* callback,
                                       void*                callbackData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t cryptoNeeded = FALSE;
    uint16_t idx = 0;
    SOPC_SecurityPolicy* secuPolicy = NULL;

    if(endpoint != NULL && endpoint->transportListener != NULL &&
       endpointURL != NULL &&
       nbSecurityPolicies > 0 && securityPolicies != NULL){
        status = STATUS_OK;
        for(idx = 0; idx < nbSecurityPolicies && STATUS_OK == status; idx++){
            secuPolicy = &securityPolicies[idx];
            if(secuPolicy != NULL &&
               (secuPolicy->securityModes & SECURITY_MODE_ANY_MASK) != 0x00)
            {
                if(NULL == CryptoProfile_Get(SOPC_String_GetRawCString(&secuPolicy->securityPolicy)))
                {
                    // Security policy is not recognized
                    status = STATUS_INVALID_PARAMETERS;
                }else if((secuPolicy->securityModes & SECURITY_MODE_SIGN_MASK) != 0 ||
                         (secuPolicy->securityModes & SECURITY_MODE_SIGNANDENCRYPT_MASK) != 0){
                    cryptoNeeded = 1;
                }
            }else{
                status = STATUS_INVALID_PARAMETERS;
            }
        }

        if(STATUS_OK == status && cryptoNeeded != FALSE){
            if(serverCertificate == NULL || serverKey == NULL){
                // Certificate and keys needed for cryptographic operations
                status = STATUS_INVALID_PARAMETERS;
            }
        }

        if(STATUS_OK == status){
            endpoint->nbSecurityPolicies = nbSecurityPolicies;
            endpoint->securityPolicies = malloc(sizeof(SOPC_SecurityPolicy) * nbSecurityPolicies);
            if(NULL != endpoint->securityPolicies){
                for(idx = 0; idx < nbSecurityPolicies && STATUS_OK == status; idx++){
                    SOPC_String_Initialize(&endpoint->securityPolicies[idx].securityPolicy);
                    status = SOPC_String_Copy(&endpoint->securityPolicies[idx].securityPolicy, &securityPolicies[idx].securityPolicy);
                    endpoint->securityPolicies[idx].securityModes = securityPolicies[idx].securityModes;
                    endpoint->securityPolicies[idx].padding = NULL;
                }
            }else{
                status = STATUS_NOK;
            }
        }

        if(STATUS_OK == status){
            endpoint->pkiProvider = pki;
            endpoint->serverCertificate = serverCertificate;
            endpoint->serverKey = serverKey;
            endpoint->callback = callback;
            endpoint->callbackData = callbackData;
            status = TCP_UA_Listener_Open(endpoint->transportListener,
                                          endpointURL,
                                          OnListenerEvent_CB,
                                          endpoint);
        }
        if(STATUS_OK == status){
            endpoint->state = SC_Endpoint_Opened;
        }
    }
    return status;
}

SOPC_StatusCode SC_ServerEndpoint_Close(SC_ServerEndpoint* endpoint){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t idx = 0;
    if(endpoint != NULL){
        if(endpoint->state != SC_Endpoint_Opened){
            status = STATUS_INVALID_STATE;
        }else{
            status = STATUS_OK;
            TCP_UA_Listener_Close(endpoint->transportListener);
            endpoint->state = SC_Endpoint_Closed;
        }
        if(endpoint->securityPolicies != NULL){
            for(idx = 0; idx < endpoint->nbSecurityPolicies; idx++){
                SOPC_String_Clear(&endpoint->securityPolicies[idx].securityPolicy);
                endpoint->securityPolicies[idx].securityModes = 0;
            }
            endpoint->nbSecurityPolicies = 0;
        }
        endpoint->pkiProvider = NULL;
        endpoint->serverCertificate = NULL;
        endpoint->serverKey = NULL;
        endpoint->callback = NULL;
        endpoint->callbackData = NULL;
    }
    return status;
}

void Internal_SLinkedList_Delete_Connection(uint32_t id, void *val){
    (void) id;
    SC_Delete((SC_Connection*) val);
}

void SC_ServerEndpoint_Delete(SC_ServerEndpoint* endpoint){
    if(endpoint != NULL){
        if(endpoint->transportListener != NULL){
            TCP_UA_Listener_Delete(endpoint->transportListener);
            endpoint->transportListener = NULL;
        }
        if(endpoint->secureChannelConnections != NULL){
            SLinkedList_Apply(endpoint->secureChannelConnections,
                              Internal_SLinkedList_Delete_Connection);
            SLinkedList_Delete(endpoint->secureChannelConnections);
        }
        if(endpoint->securityPolicies != NULL){
            free(endpoint->securityPolicies);
        }
        free(endpoint);
    }
}

void* SC_ServerEndpoint_GetCallbackData(SC_ServerEndpoint* endpoint){
    return endpoint->callbackData;
}
