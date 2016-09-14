/*
 * secure_channel_client_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include "ua_secure_channel_client_connection.h"

#include <wrappers.h>

#include <assert.h>
#include <stdlib.h>

#include <ua_encoder.h>
#include "ua_secure_channel_low_level.h"
#include <ua_types.h>

PendingRequest* SC_PendingRequestCreate(uint32_t           requestId,
                                        UA_EncodeableType* responseType,
                                        uint32_t           timeoutHint,
                                        uint32_t           startTime,
                                        ResponseEvent_CB*  callback,
                                        void*              callbackData){
    PendingRequest* result = UA_NULL;
    if(requestId != 0){
        result = malloc(sizeof(PendingRequest));
    }
    if(result != UA_NULL){
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
    if(pRequest != UA_NULL){
        free(pRequest);
    }
}

SC_ClientConnection* SC_Client_Create(Namespace*      namespac,
                                      EncodeableType* encodeableTypes)
{
    SC_ClientConnection* scClientConnection = UA_NULL;
    SC_Connection* sConnection = SC_Create();

    if(sConnection != UA_NULL){
        scClientConnection = (SC_ClientConnection *) malloc (sizeof(SC_ClientConnection));

        if(scClientConnection != UA_NULL){
            memset (scClientConnection, 0, sizeof(SC_ClientConnection));
            sConnection->state = SC_Connection_Disconnected;
            scClientConnection->instance = sConnection;
            scClientConnection->namespaces = namespac;
            scClientConnection->encodeableTypes = encodeableTypes;
            scClientConnection->securityMode = UA_MessageSecurityMode_Invalid;
            scClientConnection->pendingRequests = SLinkedList_Create();
            if(scClientConnection->pendingRequests == UA_NULL){
                free(scClientConnection);
                scClientConnection = UA_NULL;
            }
        }
    }else{
        SC_Delete(sConnection);
    }
    return scClientConnection;
}

void SC_Client_Delete(SC_ClientConnection* scConnection)
{
    if(scConnection != UA_NULL){
        if(scConnection->pkiProvider != UA_NULL){
            PKIProvider_Delete(scConnection->pkiProvider);
        }
        if(scConnection->serverCertificate != UA_NULL){
            ByteString_Clear(scConnection->serverCertificate);
        }
        if(scConnection->clientCertificate != UA_NULL){
            ByteString_Clear(scConnection->clientCertificate);
        }
        if(scConnection->pendingRequests != UA_NULL){
            free(scConnection->pendingRequests);
        }
        if(scConnection->securityPolicy != UA_NULL){
            String_Clear(scConnection->securityPolicy);
        }
        if(scConnection->instance != UA_NULL){
            SC_Delete(scConnection->instance);
        }
        Timer_Delete(&scConnection->watchdogTimer);
    }
}

StatusCode Write_OpenSecureChannelRequest(SC_ClientConnection* cConnection)
{
    StatusCode status = STATUS_OK;
    UA_OpenSecureChannelRequest openRequest;
    UA_OpenSecureChannelRequest_Initialize(&openRequest);
    const uint32_t uzero = 0;
    const uint32_t uone = 1;

    UA_ByteString* bsKey = UA_NULL;

    UA_MsgBuffer* sendBuf = cConnection->instance->sendingBuffer;
    PrivateKey* pkey = UA_NULL;
    uint32_t pkeyLength = 0;

    //// Encode request header
    // Encode authentication token (omitted opaque identifier ???? => must be a bytestring ?)
    openRequest.RequestHeader.AuthenticationToken.identifierType = IdentifierType_Numeric;
    openRequest.RequestHeader.AuthenticationToken.numeric = UA_Null_Id;
    // Encode 64 bits UtcTime => null ok ?
    openRequest.RequestHeader.Timestamp = 0;
    // Encode requestHandler
    openRequest.RequestHeader.RequestHandle = sendBuf->requestId;
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

    if(status == STATUS_OK){
        // Security mode
        status = CryptoProvider_SymmetricGenerateKeyLength(cConnection->instance->currentCryptoProvider, &pkeyLength);
    }

    if(status == STATUS_OK){
        pkey = CryptoProvider_SymmetricGenereateKey(cConnection->instance->currentCryptoProvider,
                                                    pkeyLength);
        if(pkey != UA_NULL){
            UA_ByteString* bsKey = PrivateKey_BeginUse(pkey);
            status = ByteString_AttachFrom(&openRequest.ClientNonce, bsKey);
            if(status == STATUS_OK){
                cConnection->instance->currentNonce = pkey;
            }
        }else{
            status = STATUS_NOK;
        }
    }

    if(status == STATUS_OK){
        openRequest.RequestedLifetime = cConnection->requestedLifetime;
    }

    status = SC_EncodeMsgBody(sendBuf,
                              UA_OpenSecureChannelRequest_EncodeableType,
                              &openRequest);

    PrivateKey_EndUse(bsKey);

    return status;
}

StatusCode Send_OpenSecureChannelRequest(SC_ClientConnection* cConnection)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    CryptoProvider* cProvider = UA_NULL;
    uint32_t requestId = 0;

    if(cConnection != UA_NULL){
        status = STATUS_OK;
    }

    // Set security configuration for secure channel request
    cConnection->instance->currentSecuMode = cConnection->securityMode;
    cConnection->instance->currentSecuPolicy = cConnection->securityPolicy;

    if(status == STATUS_OK){
        cProvider = CryptoProvider_Create(cConnection->securityPolicy);
        if(cProvider == UA_NULL){
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
                                              cConnection->securityPolicy,
                                              cConnection->clientCertificate,
                                              cConnection->serverCertificate);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSequenceHeader(cConnection->instance, &requestId);
    }

    if(status == STATUS_OK){
        status = Write_OpenSecureChannelRequest(cConnection);
    }

    if(status == STATUS_OK){
        status = SC_FlushSecureMsgBuffer(cConnection->instance->sendingBuffer, UA_Msg_Chunk_Final);
    }

    if(status == STATUS_OK){
        PendingRequest* pRequest = SC_PendingRequestCreate(requestId,
                                                           &UA_OpenSecureChannelResponse_EncodeableType,
                                                           0, // Not managed now
                                                           0, // Not managed now
                                                           UA_NULL, // No callback, specifc message header used (OPN)
                                                           UA_NULL);
        if(pRequest != SLinkedList_Add(cConnection->pendingRequests, requestId, pRequest)){
            status = STATUS_NOK;
        }else{
            cConnection->nbPendingRequests += 1;
        }
    }

    return status;
}

StatusCode Read_OpenSecureChannelReponse(SC_ClientConnection* cConnection,
                                         PendingRequest*      pRequest)
{
    assert(cConnection != UA_NULL &&
           pRequest != UA_NULL && pRequest->responseType != UA_NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_OpenSecureChannelResponse* encObj = UA_NULL;

    status = SC_DecodeMsgBody(cConnection->instance,
                              pRequest->responseType,
                              (void**) &encObj);
    if(status == STATUS_OK){
        status = SC_CheckReceivedProtocolVersion(cConnection->instance, encObj->ServerProtocolVersion);
    }

    return status;
}

StatusCode Receive_OpenSecureChannelResponse(SC_ClientConnection* cConnection,
                                             UA_MsgBuffer*        transportMsgBuffer)
{
    StatusCode status = STATUS_INVALID_RCV_PARAMETER;
    const uint32_t validateSenderCertificateTrue = 1; // True: always activated as indicated in API
    const uint32_t isSymmetricFalse = UA_FALSE; // TRUE
    const uint32_t isPrecCryptoDataFalse = UA_FALSE; // TODO: add guarantee we are treating last OPN sent: using pending requests ?
    uint32_t requestId = 0;
    uint32_t snPosition = 0;
    PendingRequest* pRequest = UA_NULL;

    if(transportMsgBuffer->isFinal == UA_Msg_Chunk_Final){
        // OPN request/response must be in one chunk only
        status = STATUS_OK;
    }

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(status == STATUS_OK){
        SC_DecodeSecureMsgSCid(cConnection->instance,
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
        // TODO: Check request id valid
        status = UInt32_Read(cConnection->instance->receptionBuffers, &requestId);
    }

    if(status == STATUS_OK){
        pRequest = SLinkedList_Remove(cConnection->pendingRequests, requestId);
        if(pRequest == UA_NULL){
            status = STATUS_NOK;
        }
    }

    if(status == STATUS_OK){
        // Decode message body content
        status = Read_OpenSecureChannelReponse(cConnection, pRequest);
    }

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
                          cConnection->clientCertificate,
                          cConnection->clientKey,
                          cConnection->serverCertificate);
            // Configure secure connection for encoding / decoding messages
            if(status == STATUS_OK){
                status = SC_InitReceiveSecureBuffers(cConnection->instance);
            }
            if(status == STATUS_OK){
                status = SC_InitSendSecureBuffer(cConnection->instance);
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
                        Receive_OpenSecureChannelResponse(cConnection, msgBuffer);
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
                             char*                  uri,
                             void*                  pkiConfig,
                             UA_ByteString*         clientCertificate,
                             UA_ByteString*         clientKey,
                             UA_ByteString*         serverCertificate,
                             UA_MessageSecurityMode securityMode,
                             char*                  securityPolicy,
                             uint32_t               requestedLifetime,
                             SC_ConnectionEvent_CB* callback,
                             void*                  callbackData)
{
    StatusCode status = STATUS_NOK;

    if(uri != UA_NULL &&
       pkiConfig != UA_NULL &&
       clientCertificate != UA_NULL &&
       clientKey != UA_NULL &&
       serverCertificate != UA_NULL &&
       securityMode != UA_MessageSecurityMode_Invalid &&
       securityPolicy != UA_NULL &&
       requestedLifetime > 0)
    {
        if(connection->clientCertificate == UA_NULL &&
           connection->clientKey == UA_NULL &&
           connection->serverCertificate == UA_NULL &&
           connection->securityMode == UA_MessageSecurityMode_Invalid &&
           connection->securityPolicy == UA_NULL &&
           connection->callback == UA_NULL &&
           connection->callbackData == UA_NULL)
        {
            // Create PKI provider
            connection->pkiProvider = PKIProvider_Create(pkiConfig);
            connection->clientCertificate = ByteString_Copy(clientCertificate);
            connection->clientKey = PrivateKey_Create(clientKey);
            connection->serverCertificate = ByteString_Copy(serverCertificate);
            connection->securityMode = securityMode;
            connection->securityPolicy = String_CreateFromCString(securityPolicy);
            connection->requestedLifetime = requestedLifetime;
            connection->callback = callback;
            connection->callbackData = callbackData;

            if(connection->clientCertificate == UA_NULL ||
               connection->clientKey == UA_NULL ||
               connection->serverCertificate == UA_NULL ||
               connection->securityMode == UA_MessageSecurityMode_Invalid ||
               connection->securityPolicy == UA_NULL)
            {
                status = STATUS_NOK;
            }else{
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
