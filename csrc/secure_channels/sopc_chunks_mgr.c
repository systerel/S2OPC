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

#include "sopc_chunks_mgr.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "sopc_crypto_provider.h"
#include "opcua_statuscodes.h"

#include "sopc_toolkit_constants.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_singly_linked_list.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_sockets_api.h"
#include "sopc_encoder.h"

static const uint8_t SOPC_HEL[3] = {'H','E','L'};
static const uint8_t SOPC_ACK[3] = {'A','C','K'};
static const uint8_t SOPC_ERR[3] = {'E','R','R'};
static const uint8_t SOPC_MSG[3] = {'M','S','G'};
static const uint8_t SOPC_OPN[3] = {'O','P','N'};
static const uint8_t SOPC_CLO[3] = {'C','L','O'};

static SOPC_SecureConnection* SC_GetConnection(uint32_t connectionIdx){
    SOPC_SecureConnection* scConnection = NULL;
    if(connectionIdx < SOPC_MAX_SECURE_CONNECTIONS){
        scConnection = &(secureConnectionsArray[connectionIdx]);
    }
    return scConnection;
}

static bool SC_Chunks_DecodeTcpMsgHeader(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx){
    assert(chunkCtx != NULL);
    assert(chunkCtx->chunkInputBuffer != NULL);
    assert(chunkCtx->chunkInputBuffer->length - chunkCtx->chunkInputBuffer->position >= SOPC_TCP_UA_HEADER_LENGTH);
    assert(chunkCtx->currentMsgType == SOPC_MSG_TYPE_INVALID);
    assert(chunkCtx->currentMsgIsFinal == SOPC_MSG_ISFINAL_INVALID);
    assert(chunkCtx->currentMsgSize == 0);

    SOPC_StatusCode status = STATUS_NOK;
    bool result = false;
    uint8_t msgType[3];
    uint8_t isFinal;

        // READ message type
    status = SOPC_Buffer_Read(msgType, chunkCtx->chunkInputBuffer, 3);
    if(STATUS_OK == status){
        result = true;
        if(memcmp(msgType, SOPC_HEL, 3) == 0){
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_HEL;
        }else if(memcmp(msgType, SOPC_ACK, 3) == 0){
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_ACK;
        }else if(memcmp(msgType, SOPC_ERR, 3) == 0){
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_ERR;
        }else if(memcmp(msgType, SOPC_MSG, 3) == 0){
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_SC_MSG;
        }else if(memcmp(msgType, SOPC_OPN, 3) == 0){
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_SC_OPN;
        }else if(memcmp(msgType, SOPC_CLO, 3) == 0){
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_SC_CLO;
        }else{
            // unchanged current msg type => error case
            result = false;
        }
    }

    // READ IsFinal message chunk
    if(result != false)
    {
        status = SOPC_Buffer_Read(&isFinal, chunkCtx->chunkInputBuffer, 1);
        if(STATUS_OK == status){
            switch(isFinal){
            case 'C':
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_INTERMEDIATE;
                break;
            case 'F':
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_FINAL;
                break;
            case 'A':
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_ABORT;
                break;
            default:
                // unchanged current isFinal value => error case
                result = false;
                break;
            }

            //In TCP UA non secure messages reserved byte shall be set to 'F'
            if(chunkCtx->currentMsgType != SOPC_MSG_TYPE_SC_MSG)
            {
                if(chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL){
                    result = false;
                }
            }
        }
    }

    // READ message size
    if(result != false){
        status = SOPC_UInt32_Read(&chunkCtx->currentMsgSize, chunkCtx->chunkInputBuffer);
        if(STATUS_OK != status || chunkCtx->currentMsgSize <= SOPC_TCP_UA_HEADER_LENGTH){
            // Message size cannot be less or equal to the TCP UA header length
            result = false;
        }
    }

    return result;
}

static bool SC_Chunks_ReadDataFromReceivedBuffer(SOPC_Buffer* inputBuffer,
                                                 SOPC_Buffer* receivedBuffer,
                                                 uint32_t     sizeToRead){
    // received buffer shall have enough data to be read
    assert(sizeToRead <= (receivedBuffer->length - receivedBuffer->position));
    SOPC_StatusCode status = STATUS_OK;
    bool result = false;

    // Retrieve position in which data must be placed in input buffer (end of buffer)
    // Its position will not be updated on purpose (to could be read in the future)
    uint8_t* readDest = &(inputBuffer->data[inputBuffer->length]);

    // Update length of input buffer and check it fits (STATUS_OK returned)
    status = SOPC_Buffer_SetDataLength(inputBuffer, inputBuffer->length + sizeToRead);

    if(STATUS_OK == status){
        status = SOPC_Buffer_Read(readDest, receivedBuffer, sizeToRead);
    }

    if(STATUS_OK == status){
        result = true;
    }

    return result;
}

static SOPC_SecureChannels_InputEvent SC_Chunks_MsgTypeToRcvEvent(SOPC_Msg_Type msgType){
    SOPC_SecureChannels_InputEvent scEvent;
    switch(msgType){
    case SOPC_MSG_TYPE_HEL:
        scEvent = INT_SC_RCV_HEL;
        break;
    case SOPC_MSG_TYPE_ACK:
        scEvent = INT_SC_RCV_ACK;
        break;
    case SOPC_MSG_TYPE_ERR:
        scEvent = INT_SC_RCV_ERR;
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        scEvent = INT_SC_RCV_OPN;
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        scEvent = INT_SC_RCV_CLO;
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        scEvent = INT_SC_RCV_MSG_CHUNKS;
        break;
    default:
        assert(false);
    }
    return scEvent;
}

static bool SC_Chunks_IsMsgEncrypted(OpcUa_MessageSecurityMode securityMode,
                                     bool                      isOPN)
{
    assert(securityMode != OpcUa_MessageSecurityMode_Invalid);
    bool toEncrypt = true;
    // Determine if the message must be encrypted
    if(securityMode == OpcUa_MessageSecurityMode_None ||
       (securityMode == OpcUa_MessageSecurityMode_Sign &&
        isOPN == false))
    {
        toEncrypt = false;
    }

    return toEncrypt;
}

static bool SC_Chunks_IsMsgSigned(OpcUa_MessageSecurityMode securityMode)
{
    bool toSign = true;
    // Determine if the message must be signed
    if(securityMode == OpcUa_MessageSecurityMode_None)
    {
        toSign = false;
    }
    return toSign;
}

static SOPC_StatusCode SC_Chunks_DecodeAsymSecurityHeader_Certificates(SOPC_SecureConnection*     scConnection,
                                                                       SOPC_Endpoint_Config*      epConfig,
                                                                       SOPC_SecureChannel_Config* scConfig,
                                                                       bool*                      senderCertificatePresence,
                                                                       SOPC_Certificate**              clientSenderCertificate,
                                                                       bool*                      receiverCertificatePresence)
{
    assert(scConnection != NULL);
    assert(scConnection->cryptoProvider != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);
    assert(senderCertificatePresence != NULL && receiverCertificatePresence != NULL);
    assert(clientSenderCertificate != NULL);

    SOPC_StatusCode status = STATUS_OK;
    bool enforceSecuMode = false;
    bool toEncrypt = true;
    bool toSign = true;
    SOPC_ByteString otherBsAppCert;
    SOPC_ByteString_Initialize(&otherBsAppCert);
    const SOPC_Certificate* runningAppCert = NULL;
    const SOPC_PKIProvider* pkiProvider = NULL;
    SOPC_ByteString senderCertificate;
    SOPC_ByteString_Initialize(&senderCertificate);
    SOPC_ByteString receiverCertThumb;
    SOPC_ByteString_Initialize(&receiverCertThumb);
    uint32_t tmpLength = 0;

    if(scConnection->isServerConnection == false){
        // CLIENT side: config is mandatory and security mode to be enforced
        assert(scConfig != NULL);
        runningAppCert = scConfig->crt_cli;
        pkiProvider = scConfig->pki;
        enforceSecuMode = true;
        if(scConfig->crt_srv != NULL){
            // retrieve expected sender certificate as a ByteString
            status = SOPC_KeyManager_Certificate_CopyDER(scConfig->crt_srv,
                    &otherBsAppCert.Data,
                    &tmpLength);
            if(tmpLength > 0){
                otherBsAppCert.Length = (int32_t) tmpLength;
            }
        }
    }else{
        // SERVER side: client config could be defined or not (new secure channel opening)
        assert(epConfig != NULL);
        runningAppCert = epConfig->serverCertificate;
        pkiProvider = epConfig->pki;
        if(scConfig != NULL){
            enforceSecuMode = true;
            if(scConfig->crt_cli != NULL){
                // retrieve expected sender certificate as a ByteString
                status = SOPC_KeyManager_Certificate_CopyDER(scConfig->crt_cli,
                                                        &otherBsAppCert.Data,
                                                        &tmpLength);
                if(tmpLength > 0){
                    otherBsAppCert.Length = (int32_t) tmpLength;
                }
            }
        }
    }

    // Retrieve encryption and signature configuration expected if defined
    if(enforceSecuMode != false){
        toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode,
                                             true);
        toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
    }

    // Sender Certificate:
    if(STATUS_OK == status){
        status = SOPC_ByteString_Read(&senderCertificate, scConnection->chunksCtx.chunkInputBuffer);
    }else{
        status = OpcUa_BadTcpInternalError;
    }

    if(STATUS_OK == status){
        if (toSign == false && senderCertificate.Length > 0){
            // Table 27 part 6: "field shall be null if the Message is not signed"
            status = OpcUa_BadCertificateUseNotAllowed;
            *senderCertificatePresence = true;
        }else if(toSign != false && senderCertificate.Length > 0){
            // Sender certificate is present
            *senderCertificatePresence = true;
            if(scConfig != NULL){
                // Check certificate is the same as the one in memory (CLIENT side or SERVER side with already established channel)
                int32_t otherAppCertComparison = 0;
                status = SOPC_ByteString_Compare(&otherBsAppCert,
                                                 &senderCertificate,
                                                 &otherAppCertComparison);

                if(status != STATUS_OK || otherAppCertComparison != 0){
                    status = OpcUa_BadCertificateInvalid;
                }
            }

            if(STATUS_OK == status){
                SOPC_Certificate *cert = NULL;
                status = SOPC_KeyManager_Certificate_CreateFromDER(senderCertificate.Data, senderCertificate.Length,
                                                              &cert);
                if(STATUS_OK == status){
                    status = SOPC_CryptoProvider_Certificate_Validate(scConnection->cryptoProvider,
                                                                 pkiProvider,
                                                                 cert);
                }
                if(STATUS_OK != status){
                    status = OpcUa_BadTcpInternalError;
                }

                if(scConnection->isServerConnection == false){
                    if(NULL != cert)
                        SOPC_KeyManager_Certificate_Free(cert);
                }else{
                    // SERVER SIDE ONLY
                    if(STATUS_OK == status){
                        // Set client application certificate to record
                        *clientSenderCertificate = cert;
                    }else{
                        // Error case
                        if(NULL != cert)
                            SOPC_KeyManager_Certificate_Free(cert);
                    }
                }
            }
        }else if(enforceSecuMode == false || toSign == false){
            // Without security mode to enforce, sender certificate absence could be normal
            *senderCertificatePresence = false;
        }else{
            // Sender certificate was expected
            status = OpcUa_BadCertificateInvalid;
        }
    }else{
        status = OpcUa_BadTcpInternalError;
    }


    // Receiver Certificate Thumbprint:
    if(STATUS_OK == status){
        status = SOPC_ByteString_Read(&receiverCertThumb, scConnection->chunksCtx.chunkInputBuffer);

        if(STATUS_OK == status){
            if(toEncrypt == false && receiverCertThumb.Length > 0){
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                status = OpcUa_BadCertificateUseNotAllowed;
                *receiverCertificatePresence = true;
            }else if(toEncrypt != false && receiverCertThumb.Length > 0){
                // Check thumbprint matches current app certificate thumbprint
                *receiverCertificatePresence = true;
                SOPC_ByteString curAppCertThumbprint;
                uint32_t thumbprintLength = 0;
                int32_t runningAppCertComparison = 0;

                status = SOPC_CryptoProvider_CertificateGetLength_Thumbprint(scConnection->cryptoProvider,
                                                                        &thumbprintLength);

                if(STATUS_OK == status && thumbprintLength > INT32_MAX){
                    status = OpcUa_BadCertificateInvalid;
                }else if(STATUS_OK != status){
                    status = OpcUa_BadTcpInternalError;
                }

                if(STATUS_OK == status){
                    if((int32_t) thumbprintLength == receiverCertThumb.Length){
                        status = SOPC_ByteString_InitializeFixedSize(&curAppCertThumbprint, (int32_t) thumbprintLength);
                        if(STATUS_OK == status){
                            status = SOPC_KeyManager_Certificate_GetThumbprint(scConnection->cryptoProvider,
                                                                          runningAppCert,
                                                                          curAppCertThumbprint.Data,
                                                                          thumbprintLength);

                            if(STATUS_OK == status){
                                status = SOPC_ByteString_Compare(&curAppCertThumbprint,
                                                                 &receiverCertThumb,
                                                                 &runningAppCertComparison);

                                if(status != STATUS_OK || runningAppCertComparison != 0){
                                    status = OpcUa_BadCertificateInvalid;
                                }
                            }else{
                                status = OpcUa_BadTcpInternalError;
                            }
                        }else{
                            status = OpcUa_BadTcpInternalError;
                        }
                    }else{
                        status = OpcUa_BadCertificateInvalid;
                    }
                }// if thumbprint length correctly computed

                SOPC_ByteString_Clear(&curAppCertThumbprint);

            }else if(enforceSecuMode == false || toEncrypt == false){ // if toEncrypt
                // Without security mode to enforce, absence could be normal
                *receiverCertificatePresence = false;
            }else{
                // absence was not expected
                status = OpcUa_BadCertificateInvalid;
            }
        }else{ // if decoded thumbprint
            status = OpcUa_BadTcpInternalError;
        }
    }

    SOPC_ByteString_Clear(&otherBsAppCert);

    SOPC_ByteString_Clear(&senderCertificate);
    SOPC_ByteString_Clear(&receiverCertThumb);

    return status;
}

static SOPC_StatusCode SC_Chunks_CheckAsymmetricSecurityHeader(SOPC_SecureConnection* scConnection,
                                                               bool*                  isSecurityActive){
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    SOPC_StatusCode status = STATUS_OK;
    const char* validSecuPolicy = NULL;
    uint16_t validSecuModes = 0;
    bool isSecureModeActive = true;
    SOPC_String securityPolicy;
    SOPC_String tmpStr;
    SOPC_String_Initialize(&tmpStr);
    SOPC_String_Initialize(&securityPolicy);
    SOPC_SecureChannel_Config* clientConfig = NULL;
    SOPC_Endpoint_Config* serverConfig = NULL;
    bool senderCertifPresence = false;
    bool receiverCertifThumbprintPresence = false;
    SOPC_Certificate* clientCertificate = NULL;
    int32_t compareRes = -1;
    uint32_t idx = 0;

    if(scConnection->isServerConnection == false){
        // CLIENT side
        clientConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        if(clientConfig == NULL){
            status = OpcUa_BadInvalidState;
        }
    }else{
        // SERVER side
        serverConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
        if(serverConfig == NULL){
            status = OpcUa_BadInvalidState;
        }
        if(scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
           scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW){
            // A client connection config shall already be defined and contains security expected
            clientConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            if(clientConfig == NULL){
                status = OpcUa_BadInvalidState;
            }
        }
    }

    if(STATUS_OK == status){
        // Decode security policy
        status = SOPC_String_Read(&securityPolicy, chunkCtx->chunkInputBuffer);
        if(STATUS_OK != status){
            status = OpcUa_BadDecodingError;
        }
    }

    if(STATUS_OK == status){
        if(clientConfig != NULL){
            // CLIENT side (expected same as requested) / SERVER side (expected same as initial one)
            status = SOPC_String_AttachFromCstring(&tmpStr, (char*) clientConfig->reqSecuPolicyUri);
            if(STATUS_OK == status){
                status = SOPC_String_Compare(&tmpStr, &securityPolicy, true, &compareRes);
            }
            if(STATUS_OK != status){
                status = OpcUa_BadDecodingError;
            }else{
                validSecuPolicy = clientConfig->reqSecuPolicyUri;
            }
        }else{
            assert(scConnection->isServerConnection != false);
            // SERVER side (shall comply with one server security configuration)
            compareRes = -1;
            for(idx = 0; idx < serverConfig->nbSecuConfigs && compareRes != 0; idx++){
                SOPC_SecurityPolicy* secuPolicy = &(serverConfig->secuConfigurations[idx]);
                if(STATUS_OK == SOPC_String_Compare(&securityPolicy,
                                                    &secuPolicy->securityPolicy,
                                                    true,
                                                    &compareRes)){
                    if(compareRes == 0){
                        validSecuPolicy = SOPC_String_GetRawCString(&secuPolicy->securityPolicy);
                        validSecuModes = secuPolicy->securityModes;
                    }
                }
            }
        }
        // Rejected if not compatible with security polic-y/ies expected
        if(compareRes != 0){
            status = OpcUa_BadSecurityPolicyRejected;
        }
    }

    if(STATUS_OK == status &&
       scConnection->cryptoProvider == NULL){
        scConnection->cryptoProvider = SOPC_CryptoProvider_Create(validSecuPolicy);
        if(scConnection->cryptoProvider == NULL){
            // Rejected by the cryptographic componenent
            status = OpcUa_BadSecurityPolicyRejected;
        }
    }

    if(STATUS_OK == status){
        status = SC_Chunks_DecodeAsymSecurityHeader_Certificates(scConnection,
                                                                 serverConfig,
                                                                 clientConfig,
                                                                 &senderCertifPresence,
                                                                 &clientCertificate,
                                                                 &receiverCertifThumbprintPresence);
    }

    if(STATUS_OK == status){
        // Since the security mode could be unknown before decoding (and possibly decrypting) the message
        // we have to deduce it from the certificates presence (None or SignAndEncrypt only possible in OPN)
        if(senderCertifPresence == false && receiverCertifThumbprintPresence == false){
            isSecureModeActive = false;
        }else if(senderCertifPresence != false && receiverCertifThumbprintPresence != false){
            isSecureModeActive = true;
        }else{
            status = OpcUa_BadCertificateInvalid;
        }

        // In case secure channel config is already done (RENEW), check it is correct
        if(clientConfig != NULL){
            if(isSecureModeActive == false && clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_None){
                // OK it is compatible
            }else if(isSecureModeActive != false &&
                     (clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_Sign ||
                      clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt)){
                // OK it is compatible
            }else{
                // Incompatible parameters with already configured security mode
                status = OpcUa_BadSecurityChecksFailed;
            }
        }
    }

    SOPC_String_Clear(&tmpStr);
    SOPC_String_Clear(&securityPolicy);


    if(STATUS_OK == status){
        *isSecurityActive = isSecureModeActive;
        if(clientConfig == NULL &&
           scConnection->isServerConnection != false){
           // SERVER side (fill temporary secu data necessary to terminate OPN treatment)

           // TODO: Check if it is the same certificate when connection already established ?
           scConnection->serverAsymmSecuInfo.clientCertificate = clientCertificate;
           scConnection->serverAsymmSecuInfo.securityPolicyUri = validSecuPolicy;
           scConnection->serverAsymmSecuInfo.validSecurityModes = validSecuModes;
           scConnection->serverAsymmSecuInfo.isSecureModeActive = isSecureModeActive;
        }
    }

    if(STATUS_OK != status &&
       scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED &&
       scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW){
        // Replace any error with generic error to be used before connection establishment
        status = OpcUa_BadSecurityChecksFailed;
    }
    return status;
}

static SOPC_StatusCode SC_Chunks_CheckSymmetricSecurityHeader(SOPC_SecureConnection* scConnection,
                                                              bool*                  isPrecCryptoData){
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    uint32_t tokenId = 0;
    SOPC_StatusCode status = STATUS_OK;

    status = SOPC_UInt32_Read(&tokenId, chunkCtx->chunkInputBuffer);

    if(STATUS_OK == status){
        if(scConnection->isServerConnection == false){
            // CLIENT side: should accept expired security token up to 25% of the token lifetime
            // TODO: manage time and OPN RENEW => do not accept precedent token for now since it cannot be renewed by client anyway
            if(scConnection->currentSecurityToken.tokenId != tokenId){
                status = OpcUa_BadSecureChannelTokenUnknown;
            }else{
                *isPrecCryptoData = false;
            }
        }else{
            // SERVER side: should accept precedent security token until expiration
            if(scConnection->currentSecurityToken.tokenId == tokenId){
                // Use of the current (or new) security token => OK
                if(scConnection->serverNewSecuTokenActive == false){
                    scConnection->serverNewSecuTokenActive = true;
                }
                *isPrecCryptoData = false;
            }else if(scConnection->precedentSecurityToken.tokenId == tokenId &&
                     scConnection->precedentSecurityToken.secureChannelId != 0 &&
                     scConnection->precedentSecurityToken.tokenId != 0){
                // Still valid with old security token => OK
                *isPrecCryptoData = true;
                // TODO: control expiration of precedent token
            }else{
                status = OpcUa_BadSecureChannelTokenUnknown;
            }
        }
    }else{
        status = OpcUa_BadTcpInternalError;
    }

    return status;
}

static SOPC_StatusCode SC_Chunks_CheckSeqNumReceived(SOPC_SecureConnection* scConnection,
                                                     bool                   isOPN,
                                                     uint32_t               seqNumber)
{
    assert(scConnection != NULL);
    SOPC_StatusCode status = STATUS_OK;

    if(isOPN == false){
        if(scConnection->tcpSeqProperties.lastSNreceived + 1 != seqNumber){
            // Part 6 §6.7.2 v1.03
            if(scConnection->tcpSeqProperties.lastSNreceived > UINT32_MAX - 1024 &&
                    seqNumber < 1024){
                scConnection->tcpSeqProperties.lastSNreceived = seqNumber;
            }else{
                status = OpcUa_BadSecurityChecksFailed;
            }
        }else{
            // Correct sequence number
            scConnection->tcpSeqProperties.lastSNreceived++;
        }
    }else{
        // reset sequence number since it is an OPN
        scConnection->tcpSeqProperties.lastSNreceived = seqNumber;
    }

    return status;
}

static SOPC_StatusCode SC_Chunks_CheckSequenceHeaderSN(SOPC_SecureConnection* scConnection,
                                                       bool                   isOPN){
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_StatusCode status = STATUS_OK;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    uint32_t seqNumber = 0;

    status = SOPC_UInt32_Read(&seqNumber, chunkCtx->chunkInputBuffer);

    if(STATUS_OK == status){
        status = SC_Chunks_CheckSeqNumReceived(scConnection,
                                               isOPN,
                                               seqNumber);
    }else{
        status = OpcUa_BadTcpInternalError;
    }

    return status;
}

static SOPC_StatusCode SC_Chunks_CheckSequenceHeaderRequestId(SOPC_SecureConnection* scConnection,
                                                              bool                   isClient,
                                                              SOPC_Msg_Type          receivedMsgType,
                                                              uint32_t*              requestId){
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_StatusCode status = STATUS_OK;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    SOPC_Msg_Type* recordedMsgType = NULL;

    // Retrieve request id
    status = SOPC_UInt32_Read(requestId, chunkCtx->chunkInputBuffer);
    if(STATUS_OK == status){
        if(isClient != false){
            // Check received request Id was expected for the received message type
            recordedMsgType = SOPC_SLinkedList_RemoveFromId(scConnection->tcpSeqProperties.sentRequestIds,
                                                            *requestId);
            if(recordedMsgType != NULL){
                if(*recordedMsgType != receivedMsgType){
                    status = OpcUa_BadSecurityChecksFailed;
                }
                free(recordedMsgType);
            }else{
                status = OpcUa_BadSecurityChecksFailed;
            }
        }
    }else{
        status = OpcUa_BadTcpInternalError;
    }

    return status;
}

static bool SC_Chunks_DecryptMsg(SOPC_SecureConnection* scConnection,
                                 bool                   isSymmetric,
                                 uint32_t               isPrecCryptoData)
{
    assert(scConnection != NULL);
    SOPC_Buffer* encryptedBuffer = scConnection->chunksCtx.chunkInputBuffer;
    assert(encryptedBuffer != NULL);
    // Current position is SN position
    uint32_t sequenceNumberPosition = encryptedBuffer->position;

    bool result = false;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t decryptedTextLength = 0;
    SOPC_Buffer* plainBuffer = NULL;

    SOPC_Byte* dataToDecrypt = &(encryptedBuffer->data[sequenceNumberPosition]);
    uint32_t lengthToDecrypt = encryptedBuffer->length - sequenceNumberPosition;

    if(isSymmetric == false){
        const SOPC_AsymmetricKey* runningAppPrivateKey = NULL;
        if(scConnection->isServerConnection == false){
            SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL);
            runningAppPrivateKey = scConfig->key_priv_cli;
        }else{
            SOPC_Endpoint_Config* epConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
            assert(epConfig != NULL);
            runningAppPrivateKey = epConfig->serverKey;
        }

        if(runningAppPrivateKey != NULL){
            status = SOPC_CryptoProvider_AsymmetricGetLength_Decryption(scConnection->cryptoProvider,
                                                                   runningAppPrivateKey,
                                                                   lengthToDecrypt,
                                                                   &decryptedTextLength);
            if(STATUS_OK == status){
                result = true;
            }
        }

        if(result != false && decryptedTextLength <= scConnection->tcpMsgProperties.receiveBufferSize){
            // Allocate a new plain buffer of the size of the non encrypted length + decryptedTextLength
            plainBuffer = SOPC_Buffer_Create(sequenceNumberPosition + decryptedTextLength);
            if(plainBuffer == NULL){
                result = false;
            }else{
                // Copy non encrypted data from original buffer to plain text buffer
                status = SOPC_Buffer_CopyWithLength(plainBuffer,
                                                    encryptedBuffer,
                                                    sequenceNumberPosition);
            }
            if(result != false){
                status = SOPC_CryptoProvider_AsymmetricDecrypt(scConnection->cryptoProvider,
                                                          dataToDecrypt,
                                                          lengthToDecrypt,
                                                          runningAppPrivateKey,
                                                          &(plainBuffer->data[sequenceNumberPosition]),
                                                          decryptedTextLength,
                                                          &decryptedTextLength);
                if(status == STATUS_OK){
                    assert(STATUS_OK == SOPC_Buffer_SetDataLength(plainBuffer, sequenceNumberPosition+decryptedTextLength));
                    // Set position to sequence header
                    assert(STATUS_OK == SOPC_Buffer_SetPosition(plainBuffer, sequenceNumberPosition));
                }else{
                    result = false;
                }
            }
        }
    }else{

        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;
        if(isPrecCryptoData == false){
            receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
        }else{
            receiverKeySet = scConnection->precedentSecuKeySets.receiverKeySet;
        }

        status = SOPC_CryptoProvider_SymmetricGetLength_Decryption(scConnection->cryptoProvider,
                                                              lengthToDecrypt,
                                                              &decryptedTextLength);

        if(STATUS_OK == status){
            result = true;
        }

        if(result != false && decryptedTextLength <= scConnection->tcpMsgProperties.receiveBufferSize){
            // Allocate a new plain buffer of the size of the non encrypted length + decryptedTextLength
            plainBuffer = SOPC_Buffer_Create(sequenceNumberPosition + decryptedTextLength);
            if(plainBuffer == NULL){
                result = false;
            }else{
                // Copy non encrypted data from original buffer to plain text buffer
                status = SOPC_Buffer_CopyWithLength(plainBuffer,
                                                    encryptedBuffer,
                                                    sequenceNumberPosition);
            }
            if(result != false){
                status = SOPC_CryptoProvider_SymmetricDecrypt(scConnection->cryptoProvider,
                                                         dataToDecrypt,
                                                         lengthToDecrypt,
                                                         receiverKeySet->encryptKey,
                                                         receiverKeySet->initVector,
                                                         &(plainBuffer->data[sequenceNumberPosition]),
                                                         decryptedTextLength);
                if(status == STATUS_OK){
                    assert(STATUS_OK == SOPC_Buffer_SetDataLength(plainBuffer, sequenceNumberPosition+decryptedTextLength));
                    // Set position to sequence header
                    assert(STATUS_OK == SOPC_Buffer_SetPosition(plainBuffer, sequenceNumberPosition));
                }else{
                    result = false;
                }
            }
        }
    } // Symmetric algo branch

    if(result == false){
        if(plainBuffer != NULL){
            SOPC_Buffer_Delete(plainBuffer);
        }
    }else{
        SOPC_Buffer_Delete(scConnection->chunksCtx.chunkInputBuffer);
        // Replace input buffer with the plain buffer (position == 0 => SN position)
        scConnection->chunksCtx.chunkInputBuffer = plainBuffer;
    }

    return result;
}

static bool SC_Chunks_VerifyMsgSignature(SOPC_SecureConnection* scConnection,
                                         bool                   isSymmetric,
                                         bool                   isPrecCryptoData)
{
    assert(scConnection != NULL);
    SOPC_Buffer* buffer = scConnection->chunksCtx.chunkInputBuffer;
    assert(buffer != NULL);

    bool result = false;

    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    uint32_t signatureSize = 0;
    uint32_t signaturePosition = 0;

    if(isSymmetric == false){
        SOPC_AsymmetricKey* publicKey = NULL;
        const SOPC_Certificate* otherAppCertificate = NULL;
        if(scConnection->isServerConnection == false){
            // Client side
            SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL);
            otherAppCertificate = scConfig->crt_srv;
        }else{
            // Server side: we have to use the temporary stored certificate value
            otherAppCertificate = scConnection->serverAsymmSecuInfo.clientCertificate;
        }


        status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate,
                                                                &publicKey);

        if(status == STATUS_OK){
            status = SOPC_CryptoProvider_AsymmetricGetLength_Signature(scConnection->cryptoProvider,
                                                                  publicKey,
                                                                  &signatureSize);
        }

        if(status == STATUS_OK){
            signaturePosition = buffer->length - signatureSize;

            status = SOPC_CryptoProvider_AsymmetricVerify(scConnection->cryptoProvider,
                                                     buffer->data,
                                                     signaturePosition,
                                                     publicKey,
                                                     &(buffer->data[signaturePosition]),
                                                     signatureSize);
        }

        SOPC_KeyManager_AsymmetricKey_Free(publicKey);
    }else{
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;
        if(isPrecCryptoData == false){
            receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
        }else{
            receiverKeySet = scConnection->precedentSecuKeySets.receiverKeySet;
        }

        status = SOPC_CryptoProvider_SymmetricGetLength_Signature(scConnection->cryptoProvider,
                                                             &signatureSize);

        if(status == STATUS_OK){
            signaturePosition = buffer->length - signatureSize;
            status = SOPC_CryptoProvider_SymmetricVerify(scConnection->cryptoProvider,
                                                    buffer->data,
                                                    signaturePosition,
                                                    receiverKeySet->signKey,
                                                    &(buffer->data[signaturePosition]),
                                                    signatureSize);
        }
    }

    if(STATUS_OK == status){
        result = true;
    }

    return result;
}

static SOPC_StatusCode SC_Chunks_TreatTcpPayload(SOPC_SecureConnection* scConnection,
                                                 uint32_t*              requestId){
    assert(requestId != NULL);

    SOPC_StatusCode status = STATUS_OK;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    // Note: we do not treat multiple chunks => guaranteed by HEL/ACK exchanged (chunk config)
    assert(chunkCtx->currentMsgIsFinal == SOPC_MSG_ISFINAL_FINAL);

    bool asymmSecuHeader = false;
    bool symmSecuHeader = false;
    bool sequenceHeader = false;
    bool hasSecureChannelId = false;
    bool isOPN = false;

    uint32_t secureChannelId = 0;

    bool toDecrypt = false;
    bool toCheckSignature = false;
    bool isPrecCryptoData = false;

    // Note: for non secure message we already check those messages are expected
    //       regarding the connection type (client/server)
    switch(chunkCtx->currentMsgType){
        case SOPC_MSG_TYPE_HEL:
            if(scConnection->isServerConnection == false ||
               chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL){
                // A client shall not receive a HELLO message
                // or HELLO message chunk shall be final
                status = OpcUa_BadTcpMessageTypeInvalid;
            }
            // Nothing to do: whole payload to transmit to the secure connection state manager
            break;
        case SOPC_MSG_TYPE_ACK:
            if(scConnection->isServerConnection != false ||
               chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL){
                // A server shall not receive an ACK message
                // or ACK message chunk shall be final
                status = OpcUa_BadTcpMessageTypeInvalid;
            }
            // Nothing to do: whole payload to transmit to the secure connection state manager
            break;
        case SOPC_MSG_TYPE_ERR:
            if(scConnection->isServerConnection != false ||
               chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL){
                // A server shall not receive an ERROR message
                // or ERR message chunk shall be final
                status = OpcUa_BadTcpMessageTypeInvalid;
            }
            // Nothing to do: whole payload to transmit to the secure connection state manager
            break;
        case SOPC_MSG_TYPE_SC_OPN:
            if(chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL){
                // OPN message chunk shall be final
                status = OpcUa_BadTcpMessageTypeInvalid;
            }else{
                isOPN = true;
                hasSecureChannelId = true;
                asymmSecuHeader = true;
                sequenceHeader = true;
            }
            break;
        case SOPC_MSG_TYPE_SC_CLO:
            if(scConnection->isServerConnection == false ||
               chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL){
                status = OpcUa_BadTcpMessageTypeInvalid;
            }else{
                hasSecureChannelId = true;
                symmSecuHeader = true;
                sequenceHeader = true;
            }
            break;
        case SOPC_MSG_TYPE_SC_MSG:
            hasSecureChannelId = true;
            symmSecuHeader = true;
            sequenceHeader = true;
            break;
        default:
            assert(false);
    }

    if(STATUS_OK == status && hasSecureChannelId != false){
        // Decode secure channel id
        assert(STATUS_OK == SOPC_UInt32_Read(&secureChannelId, chunkCtx->chunkInputBuffer));
        if(isOPN != false){
            if(scConnection->currentSecurityToken.secureChannelId == 0){
                // OK it is a new secure channel: value can be 0 or other
                if(secureChannelId != 0){
                    // Store the secure channel Id as temporary context to be checked
                    scConnection->clientSecureChannelId = secureChannelId;
                }
            }else if(scConnection->currentSecurityToken.secureChannelId != secureChannelId){
                // Error: it shall be the expected secure channel Id
                status = OpcUa_BadTcpSecureChannelUnknown;
            }
        }else{
            if(scConnection->currentSecurityToken.secureChannelId != secureChannelId){
                // Error: it shall be the expected secure channel Id
                status = OpcUa_BadTcpSecureChannelUnknown;
            }
        }
    }

    if(STATUS_OK == status && asymmSecuHeader != false){
        bool isSecurityActive = false;
        status = SC_Chunks_CheckAsymmetricSecurityHeader(scConnection,
                                                         &isSecurityActive);
        if(STATUS_OK == status){
            toDecrypt = isSecurityActive;
            toCheckSignature = isSecurityActive;
            isPrecCryptoData = false; // asymmetric => unused parameter in decrypt / check sign
        }
    }

    if(STATUS_OK == status && symmSecuHeader != false){
        // TODO: toDecrypt / toCheckSignature
        SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        // If a symmetric message is received, secure channel should be configured
        if(scConfig == NULL){
            status = OpcUa_BadSecurityChecksFailed;
        }else{
            status = SC_Chunks_CheckSymmetricSecurityHeader(scConnection,
                                                            &isPrecCryptoData);
            if(STATUS_OK == status){
                toDecrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, isOPN);
                toCheckSignature = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
            }
        }
    }

    if(STATUS_OK == status && toDecrypt != false){
        // Decrypt the message
        bool result = SC_Chunks_DecryptMsg(scConnection,
                                           isOPN == false, // isSymmetric
                                           isPrecCryptoData);
        if(result == false){
            status = OpcUa_BadSecurityChecksFailed;
        }
    }

    if(STATUS_OK == status && toCheckSignature != false){
        // Check decrypted message signature
        bool result = SC_Chunks_VerifyMsgSignature(scConnection,
                                                   isOPN == false, // isSymmetric
                                                   isPrecCryptoData);
        if(result == false){
            status = OpcUa_BadApplicationSignatureInvalid;
        }
    }

    if(STATUS_OK == status && sequenceHeader != false){

        status = SC_Chunks_CheckSequenceHeaderSN(scConnection,
                                                 isOPN);

        if(STATUS_OK == status){
            status = SC_Chunks_CheckSequenceHeaderRequestId(scConnection,
                                                            scConnection->isServerConnection == false, // isClient
                                                            chunkCtx->currentMsgType,
                                                            requestId);
        }

    }

    return status;
}

static void SC_Chunks_TreatReceivedBuffer(SOPC_SecureConnection* scConnection,
                                          uint32_t               scConnectionIdx,
                                          SOPC_Buffer*           receivedBuffer){
    assert(scConnection != NULL);
    assert(receivedBuffer != NULL);
    assert(receivedBuffer->position == 0);

    uint32_t sizeToRead = 0;
    uint32_t sizeAlreadyRead = 0;
    uint32_t sizeAvailable = 0;
    SOPC_StatusCode errorStatus = STATUS_OK;
    uint32_t requestId = 0;
    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    sizeAvailable = receivedBuffer->length - receivedBuffer->position;
    // Continue until an error occurred OR received buffer is empty (could contain 1 or several messages)
    while(result != false && sizeAvailable > 0){
        if(chunkCtx->chunkInputBuffer == NULL){
            // No incomplete message data: create a new buffer
            chunkCtx->chunkInputBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.receiveBufferSize);
            if(chunkCtx->chunkInputBuffer == NULL){
                errorStatus = OpcUa_BadOutOfMemory;
                result = false;
            }
        }

        if(result != false){
            // OPC UA TCP MESSAGE HEADER TREATMENT
            bool decodeHeader = false;

            if(chunkCtx->chunkInputBuffer->length < SOPC_TCP_UA_HEADER_LENGTH){
                // Message data was already received but not enough to know the message size
                //  => new attempt to retrieve message header containing size

                // Compute size to read to obtain the complete message header
                sizeToRead = SOPC_TCP_UA_HEADER_LENGTH - chunkCtx->chunkInputBuffer->length;
                sizeAvailable = receivedBuffer->length - receivedBuffer->position;


                if(sizeAvailable >= sizeToRead){
                    // Complete header available: retrieve header data from received buffer
                    result = SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer,
                                                                  receivedBuffer,
                                                                  sizeToRead);

                    if(result == false){
                        errorStatus = OpcUa_BadTcpMessageTooLarge;
                    }else{
                        // Enough data to decode header
                        decodeHeader = true;
                    }
                }else{
                    // Complete header not available: retrieve available data from received buffer
                    result = SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer,
                                                                  receivedBuffer,
                                                                  sizeAvailable);

                    if(result == false){
                        errorStatus = OpcUa_BadTcpMessageTooLarge;
                    }
                }
            }

            if(result != false && decodeHeader != false){
                // Decode the received OPC UA TCP message header
                result = SC_Chunks_DecodeTcpMsgHeader(&scConnection->chunksCtx);
                if(result == false){
                    errorStatus = OpcUa_BadTcpMessageTypeInvalid;
                }
            }
        }  /* END OF OPC UA TCP MESSAGE HEADER TREATMENT */

        if(result != false){
            /* OPC UA TCP MESSAGE PAYLOAD TREATMENT */

            bool completePayload = false;
            if(chunkCtx->chunkInputBuffer->length >= SOPC_TCP_UA_HEADER_LENGTH){
                // Message header is decoded but message payload not (completly) retrieved
                //  => attempt to retrieve complete message payload
                assert(chunkCtx->currentMsgSize > 0); // message size was decoded
                assert(chunkCtx->currentMsgType != SOPC_MSG_TYPE_INVALID);
                assert(chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_INVALID);

                sizeAvailable = receivedBuffer->length - receivedBuffer->position;
                // Incomplete message payload data size already retrieved in input buffer
                sizeAlreadyRead = chunkCtx->chunkInputBuffer->length - chunkCtx->chunkInputBuffer->position;

                if(chunkCtx->currentMsgSize > SOPC_TCP_UA_HEADER_LENGTH + sizeAlreadyRead){
                    sizeToRead = chunkCtx->currentMsgSize - SOPC_TCP_UA_HEADER_LENGTH - sizeAlreadyRead;
                }else{
                    // Size provided by message seems invalid
                    result = false;
                    errorStatus = OpcUa_BadTcpInternalError; // not really internal no error corresponding
                }

                if(result != false){
                    if(sizeAvailable >= sizeToRead){
                        // Complete payload available: retrieve payload data from received buffer
                        result = SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer,
                                                                      receivedBuffer,
                                                                      sizeToRead);
                        if(result == false){
                            errorStatus = OpcUa_BadTcpMessageTooLarge;
                        }

                        // Enough data to read complete message
                        completePayload = true;
                    }else{

                        result = SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer,
                                                                      receivedBuffer,
                                                                      sizeAvailable);

                        if(result == false){
                            errorStatus = OpcUa_BadTcpMessageTooLarge;
                        }

                    }
                }

            }

            if(result != false && completePayload != false){
                // Decode OPC UA Secure Conversation MessageChunk specific headers if necessary (not HEL/ACK/ERR)
                errorStatus = SC_Chunks_TreatTcpPayload(scConnection,
                                                        &requestId);
                if(STATUS_OK == errorStatus){
                    // Transmit OPC UA message to secure connection state manager
                    SOPC_SecureChannels_InputEvent scEvent = SC_Chunks_MsgTypeToRcvEvent(chunkCtx->currentMsgType);
                    if(scEvent == INT_SC_RCV_ERR ||
                       scEvent == INT_SC_RCV_CLO){
                        // Treat as prio events
                        SOPC_SecureChannels_EnqueueInternalEventAsNext(scEvent,
                                                                       scConnectionIdx,
                                                                       (void*) chunkCtx->chunkInputBuffer,
                                                                       requestId);
                    }else{
                        SOPC_SecureChannels_EnqueueInternalEvent(scEvent,
                                                                 scConnectionIdx,
                                                                 (void*) chunkCtx->chunkInputBuffer,
                                                                 requestId);
                    }
                    // reset chunk context (buffer not deallocated since provided to secure connection state manager)
                    memset(&scConnection->chunksCtx, 0, sizeof(SOPC_SecureConnection_ChunkMgrCtx));
                }else{
                    result = false;
                }
            }
        } /* END OF OPC UA TCP MESSAGE PAYLOAD TREATMENT */

        if(result == false){
            // Treat as prio events
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_RCV_FAILURE,
                                                           scConnectionIdx,
                                                           NULL,
                                                           errorStatus);
            SOPC_Buffer_Delete(chunkCtx->chunkInputBuffer);
            // reset chunk context
            memset(&scConnection->chunksCtx, 0, sizeof(SOPC_SecureConnection_ChunkMgrCtx));
            receivedBuffer->length = 0;
            receivedBuffer->position = 0;
        }

        // Update available data remaining in received buffer
        sizeAvailable = receivedBuffer->length - receivedBuffer->position;
    }

    SOPC_Buffer_Delete(receivedBuffer);
}

static bool SC_Chunks_EncodeTcpMsgHeader(SOPC_SecureConnection* scConnection,
                                         SOPC_Msg_Type          sendMsgType,
                                         bool                   isFinalChunk,
                                         SOPC_Buffer*           buffer){
    assert(scConnection!= NULL);
    assert(buffer != NULL);
    bool result = false;
    const uint8_t* msgTypeBytes = NULL;
    uint8_t isFinalChunkByte = 'F';
    uint32_t messageSize = 0; // Could be temporary depending on message type / secu parameters

    switch(sendMsgType){
    case SOPC_MSG_TYPE_HEL:
        msgTypeBytes = SOPC_HEL;
        result = (isFinalChunk !=false);
        break;
    case SOPC_MSG_TYPE_ACK:
        msgTypeBytes = SOPC_ACK;
        result = (isFinalChunk !=false);
        break;
    case SOPC_MSG_TYPE_ERR:
        msgTypeBytes = SOPC_ERR;
        result = (isFinalChunk !=false);
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        msgTypeBytes = SOPC_OPN;
        result = (isFinalChunk !=false);
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        msgTypeBytes = SOPC_CLO;
        result = (isFinalChunk !=false);
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        msgTypeBytes = SOPC_MSG;
        result = true;
        break;
    default:
        assert(false);
    }

    if(result != false){
        if(STATUS_OK != SOPC_Buffer_Write(buffer, msgTypeBytes, 3)){
            result = false;
        }
    }
    if(result != false){
        if(isFinalChunk == false){
            // Set intermediate chunk value
            isFinalChunkByte = 'C';
        }
        if(STATUS_OK != SOPC_Buffer_Write(buffer, &isFinalChunkByte, 1)){
            result = false;
        }
    }
    if(result != false){
        if(buffer->length >= SOPC_TCP_UA_HEADER_LENGTH){
            messageSize = buffer->length;
        }else{
            messageSize = SOPC_TCP_UA_HEADER_LENGTH;
        }
        if(STATUS_OK != SOPC_UInt32_Write(&messageSize, buffer)){
            result = false;
        }
    }
    return result;
}

static bool SC_Chunks_EncodeAsymSecurityHeader(SOPC_SecureConnection*     scConnection,
                                               SOPC_SecureChannel_Config* scConfig,
                                               SOPC_Buffer*               buffer,
                                               uint32_t*                  securityPolicyLength,
                                               uint32_t*                  senderCertificateSize,
                                               SOPC_StatusCode*           errorStatus){
    assert(scConnection != NULL);
    assert(scConnection->cryptoProvider != NULL);
    assert(scConfig != NULL);
    assert(scConfig->reqSecuPolicyUri != NULL);
    assert(buffer != NULL);
    assert(senderCertificateSize != NULL);
    bool result = true;
    bool toEncrypt = true;
    bool toSign = true;
    SOPC_String strSecuPolicy;
    SOPC_String_Initialize(&strSecuPolicy);
    SOPC_ByteString bsSenderCert;
    SOPC_ByteString_Initialize(&bsSenderCert);
    const SOPC_Certificate* receiverCertCrypto = NULL;

    if(result != false)
    {
        toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode,
                                             true);
        toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
    }

    // Security Policy:
    if(result != false){
        if(STATUS_OK != SOPC_String_AttachFromCstring(&strSecuPolicy, (char*) scConfig->reqSecuPolicyUri) ||
           strSecuPolicy.Length <= 0){
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }else{
            *securityPolicyLength = (uint32_t) strSecuPolicy.Length;
        }
    }
    if(result != false){
        if(STATUS_OK != SOPC_String_Write(&strSecuPolicy, buffer)){
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    // Sender Certificate:
    if(result != false){
        const SOPC_Certificate* senderCert = NULL;
        uint32_t length = 0;
        if(scConnection->isServerConnection == false){
            // Client side
            senderCert = scConfig->crt_cli;
        }else{
            // Server side
            senderCert = scConfig->crt_srv;
        }
        if(senderCert != NULL){
            if(STATUS_OK == SOPC_KeyManager_Certificate_CopyDER(senderCert, &bsSenderCert.Data, &length)
                    && length <= INT32_MAX){
                bsSenderCert.Length = (int32_t) length;
                *senderCertificateSize = length;
            }else{
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
        }
    }
    if(result != false){
        if(toSign != false && bsSenderCert.Length > 0){
            if(STATUS_OK != SOPC_ByteString_Write(&bsSenderCert, buffer)){
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
        }else if(toSign == false){ // Field shall be null if message not signed
            // Note: foundation stack expects -1 value whereas 0 is also valid:
            const int32_t minusOne = -1;
            if(STATUS_OK != SOPC_Int32_Write(&minusOne, buffer)){
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            // NULL string: nothing to write
        }else{
            // Certificate shall be defined in configuration if necessary (configuration constraint)
            assert(false);
        }
    }

    // Receiver Certificate Thumbprint:
    if(result != false){
        // Retrieve correct certificate
        if(scConnection->isServerConnection == false){
            // Client side
            receiverCertCrypto = scConfig->crt_srv;
        }else{
            // Server side
            receiverCertCrypto = scConfig->crt_cli;
        }

        if(toEncrypt != false && receiverCertCrypto != NULL){
            SOPC_ByteString recCertThumbprint;
            SOPC_ByteString_Initialize(&recCertThumbprint);
            uint32_t thumbprintLength = 0;
            if(STATUS_OK != SOPC_CryptoProvider_CertificateGetLength_Thumbprint(scConnection->cryptoProvider,
                                                                           &thumbprintLength)){
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }

            if(result != false){
                if(thumbprintLength <= INT32_MAX &&
                   STATUS_OK == SOPC_ByteString_InitializeFixedSize(&recCertThumbprint, thumbprintLength)){
                    // OK
                }else{
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            if(result != false){
                if(STATUS_OK != SOPC_KeyManager_Certificate_GetThumbprint(scConnection->cryptoProvider,
                                                                     receiverCertCrypto,
                                                                     recCertThumbprint.Data,
                                                                     thumbprintLength)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            if(result != false){
                if(STATUS_OK != SOPC_ByteString_Write(&recCertThumbprint, buffer)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            SOPC_ByteString_Clear(&recCertThumbprint);
        }else if(toEncrypt == false){
            // Note: foundation stack expects -1 value whereas 0 is also valid:
            const int32_t minusOne = -1;
            if(STATUS_OK != SOPC_Int32_Write(&minusOne, buffer)){
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            // NULL string: nothing to write
        }else{
            // Certificate shall be defined in configuration if necessary (configuration constraint)
            assert(false);
        }
    }

    SOPC_String_Clear(&strSecuPolicy);
    SOPC_ByteString_Clear(&bsSenderCert);
    return result;
}

static bool SC_Chunks_Is_ExtraPaddingSizePresent(uint32_t plainTextBlockSize){
    // Extra-padding necessary if padding could be greater 256 bytes (2048 bits)
    // (1 byte for padding size field + 255 bytes of padding).
    // => padding max value is plainTextBlockSize regarding the formula of padding size
    //    (whereas spec part 6 indicates it depends on the key size which is incorrect)
    if(plainTextBlockSize > 256){
        return true;
    }
    return false;
}

static uint32_t SC_Chunks_ComputeMaxBodySize(uint32_t nonEncryptedHeadersSize,
                                             uint32_t chunkSize,
                                             bool     toEncrypt,
                                             uint32_t cipherBlockSize,
                                             uint32_t plainBlockSize,
                                             bool     toSign,
                                             uint32_t signatureSize)
{
    uint32_t result = 0;
    uint32_t paddingSizeFields = 0;
    if(toEncrypt == false){
        // No encryption => consider same size blocks and no padding size fields
        cipherBlockSize = 1;
        plainBlockSize = 1;
        paddingSizeFields = 0;
    }else{
        // By default only 1 byte for padding size field. +1 if extra padding
        paddingSizeFields = 1;
        if(SC_Chunks_Is_ExtraPaddingSizePresent(plainBlockSize) != false){
            paddingSizeFields += 1;
        }
    }

    if(toSign == false){
        signatureSize = 0;
    }

    // Ensure cipher block size is greater or equal to plain block size:
    //  otherwise the plain size could be greater than the  buffer size regarding computation
    assert(cipherBlockSize >= plainBlockSize);

    // Use formulae of spec 1.03 part 6 §6.7.2 even if controversial (see mantis ticket #2897)
    result = plainBlockSize * ((chunkSize - nonEncryptedHeadersSize - signatureSize - paddingSizeFields) / cipherBlockSize) - SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH;

    // Maximum body size (+headers+signature+padding size fields) cannot be greater than maximum buffer size
    assert(chunkSize >=
           (nonEncryptedHeadersSize + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
            result + signatureSize + paddingSizeFields));

    return result;
}

static bool SC_Chunks_GetCryptoSizes(SOPC_SecureConnection*     scConnection,
                                     SOPC_SecureChannel_Config* scConfig,
                                     bool                       isSymmetricAlgo,
                                     bool*                      toEncrypt,
                                     bool*                      toSign,
                                     uint32_t*                  signatureSize,
                                     uint32_t*                  cipherTextBlockSize,
                                     uint32_t*                  plainTextBlockSize){
    assert(scConnection!= NULL);
    assert(scConfig != NULL);
    assert(toEncrypt != NULL);
    assert(toSign != NULL);
    assert(signatureSize != NULL);
    assert(cipherTextBlockSize != NULL);
    assert(plainTextBlockSize != NULL);
    bool result = true;

    if(isSymmetricAlgo == false){
        // ASYMMETRIC CASE

        if(scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None){
            SOPC_AsymmetricKey* publicKey = NULL;
            const SOPC_Certificate* otherAppCertificate = NULL;

            // Asymmetric case: used only for opening channel, signature AND encryption mandatory in this case
            *toEncrypt = true;
            *toSign = true;

            if(scConnection->isServerConnection == false){
                //Client side
                otherAppCertificate = scConfig->crt_srv;
            }else{
                //Server side
                otherAppCertificate = scConfig->crt_cli;
            }

            if(STATUS_OK != SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate,
                                                                           &publicKey)){
                result = false;
            }

            if(result != false){
                // Compute block sizes
                if(STATUS_OK != SOPC_CryptoProvider_AsymmetricGetLength_Msgs(scConnection->cryptoProvider,
                                                                        publicKey,
                                                                        cipherTextBlockSize,
                                                                        plainTextBlockSize)){
                    result = false;
                }
            }
            if(result != false){
                // Compute signature size
                if(STATUS_OK != SOPC_CryptoProvider_AsymmetricGetLength_Signature(scConnection->cryptoProvider,
                                                                             publicKey,
                                                                             signatureSize)){
                    result = false;
                }
            }

            SOPC_KeyManager_AsymmetricKey_Free(publicKey);

        }else{
            *toEncrypt = false; // No data encryption
            *toSign = false;    // No signature
        }
    }else{
        // ASYMMETRIC CASE

        if(scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None){

            if(scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt){
                // Encryption necessary: compute block sizes
                *toEncrypt = true;

                if(STATUS_OK != SOPC_CryptoProvider_SymmetricGetLength_Blocks(scConnection->cryptoProvider,
                                                                         cipherTextBlockSize,
                                                                         plainTextBlockSize)){
                    result = false;
                }
            }else{
                *toEncrypt = false;
            }

            if(result != false){
                // Signature necessary in both Sign and SignAndEncrypt cases: compute signature size
                *toSign = true;
                if(STATUS_OK != SOPC_CryptoProvider_SymmetricGetLength_Signature(scConnection->cryptoProvider,
                                                                            signatureSize)){
                    result = false;
                }
            }
        }else{
            // No signature or encryption
            *toEncrypt = false;
            *toSign = false;
        }
    }

    return result;
}

static uint32_t SC_Chunks_GetMaxBodySize(SOPC_SecureConnection*     scConnection,
                                         SOPC_SecureChannel_Config* scConfig,
                                         uint32_t                   chunkSize,
                                         uint32_t                   nonEncryptedHeadersSize,
                                         bool                       isSymmetric)
{
    assert(scConnection!= NULL);
    assert(scConfig != NULL);
    uint32_t maxBodySize = 0;
    bool result = true;

    bool  toEncrypt = false;
    uint32_t cipherBlockSize = 0;
    uint32_t plainBlockSize =0;
    bool toSign = false;
    uint32_t signatureSize = 0;

    result = SC_Chunks_GetCryptoSizes(scConnection,
                                      scConfig,
                                      isSymmetric,
                                      &toEncrypt,
                                      &toSign,
                                      &signatureSize,
                                      &cipherBlockSize,
                                      &plainBlockSize);

    // Compute the max body size regarding encryption and signature use
    if(result != false){
        maxBodySize = SC_Chunks_ComputeMaxBodySize(nonEncryptedHeadersSize,
                                                   chunkSize,
                                                   toEncrypt,
                                                   cipherBlockSize,
                                                   plainBlockSize,
                                                   toSign,
                                                   signatureSize);
    }

    return maxBodySize;
}

static uint16_t SC_Chunks_GetPaddingSize(uint32_t bytesToEncrypt, // called bytesToWrite in spec part 6 but it should not since it includes SN header !
                                         uint32_t plainBlockSize,
                                         uint32_t signatureSize){
    // By default only 1 padding size field + 1 if extra padding
    uint8_t paddingSizeFields = 1;
    if(SC_Chunks_Is_ExtraPaddingSizePresent(plainBlockSize)){
        paddingSizeFields += 1;
    }
    return plainBlockSize - ((bytesToEncrypt + signatureSize + paddingSizeFields) % plainBlockSize);
}

static bool SOPC_Chunks_EncodePadding(SOPC_SecureConnection*     scConnection,
                                      SOPC_SecureChannel_Config* scConfig,
                                      SOPC_Buffer*               buffer,
                                      bool                       isSymmetricAlgo,
                                      uint32_t                   sequenceNumberPosition,
                                      uint32_t*                  signatureSize,
                                      uint16_t*                  realPaddingLength, // >= paddingSizeField
                                      bool*                      hasExtraPadding){

    assert(scConnection!= NULL);
    assert(scConfig != NULL);
    assert(buffer != NULL);
    assert(signatureSize != NULL);
    assert(realPaddingLength != NULL);
    assert(hasExtraPadding != NULL);

    bool result = true;

    bool  toEncrypt = false;
    uint32_t cipherBlockSize = 0;
    uint32_t plainTextBlockSize = 0;
    bool toSign = false;

    result = SC_Chunks_GetCryptoSizes(scConnection,
                                      scConfig,
                                      isSymmetricAlgo,
                                      &toEncrypt,
                                      &toSign,
                                      signatureSize,
                                      &cipherBlockSize,
                                      &plainTextBlockSize);

    if(result != false && toEncrypt != false){
        *realPaddingLength = SC_Chunks_GetPaddingSize(buffer->length - sequenceNumberPosition,
                                                      plainTextBlockSize,
                                                      *signatureSize);
        //Little endian conversion of padding:
        SOPC_EncodeDecode_UInt16(realPaddingLength);
        if(STATUS_OK != SOPC_Buffer_Write(buffer, (SOPC_Byte*) realPaddingLength, 1)){
            result = false;
        }

        if(result != false){
            uint8_t paddingSizeField = 0;
            paddingSizeField = 0xFF & *realPaddingLength;
            // The value of each byte of the padding is equal to paddingSize:
            SOPC_Byte* paddingBytes = malloc(sizeof(SOPC_Byte)*(*realPaddingLength));
            if(paddingBytes != NULL){
                memset(paddingBytes, paddingSizeField, *realPaddingLength);
                if(STATUS_OK != SOPC_Buffer_Write(buffer, paddingBytes, *realPaddingLength)){
                    result = false;
                }
                free(paddingBytes);
            }else{
                result = false;
            }
        }

        // Extra-padding necessary if padding could be greater 256 bytes
        if(result != false && SC_Chunks_Is_ExtraPaddingSizePresent(plainTextBlockSize) != false){
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            SOPC_Byte extraPadding = 0x00FF & *realPaddingLength;
            if(STATUS_OK != SOPC_Buffer_Write(buffer, &extraPadding, 1)){
                result = false;
            }
        }
    }else{
        result = false;
    }

    return result;
}

static bool SC_Chunks_CheckMaxSenderCertificateSize(uint32_t senderCertificateSize,
                                                    uint32_t messageChunkSize,
                                                    uint32_t securityPolicyUriLength,
                                                    bool     hasPadding,
                                                    uint32_t realPaddingLength,
                                                    bool     hasExtraPadding,
                                                    uint32_t asymmetricSignatureSize){
    bool result = false;
    int32_t maxSize = // Fit in a single message chunk with at least 1 byte of body
     messageChunkSize -
     SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH -
     4 - // URI length field size
     securityPolicyUriLength -
     4 - // Sender certificate length field
     4 - // Receiver certificate thumbprint length field
     20 - // Receiver certificate thumbprint length
     8; // Sequence header size
    if(hasPadding != false){
        maxSize =
         maxSize -
         1 - // padding length field size
         realPaddingLength;
        if(hasExtraPadding){
            // ExtraPaddingSize field size to remove
            maxSize = maxSize - 1;
        }
    }
    maxSize = maxSize - asymmetricSignatureSize;

    if(senderCertificateSize <= (uint32_t) maxSize){
        result = true;
    }
    return result;
}

static bool SC_Chunks_GetEncryptedDataLength(SOPC_SecureConnection*     scConnection,
                                             SOPC_SecureChannel_Config* scConfig,
                                             uint32_t                   plainDataLength,
                                             bool                       isSymmetricAlgo,
                                             uint32_t*                  cipherDataLength)
{
    assert(scConnection != NULL);
    assert(scConfig != NULL);
    assert(cipherDataLength != NULL);
    bool result = true;

    if(isSymmetricAlgo == false){
        const SOPC_Certificate* otherAppCertificate = NULL;
        if(scConnection->isServerConnection == false){
            // Client side
            otherAppCertificate = scConfig->crt_srv;
        }else{
            // Server side
            otherAppCertificate = scConfig->crt_cli;
        }
        if(otherAppCertificate == NULL){
           result = false;
        }else{

            SOPC_AsymmetricKey* otherAppPublicKey = NULL;

            if(STATUS_OK != SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate,
                                                                           &otherAppPublicKey)){
                result = false;
            }

            // Retrieve cipher length
            if(result != false){
                if(STATUS_OK != SOPC_CryptoProvider_AsymmetricGetLength_Encryption(scConnection->cryptoProvider,
                                                                              otherAppPublicKey,
                                                                              plainDataLength,
                                                                              cipherDataLength)){
                    result = false;
                }
            }

            SOPC_KeyManager_AsymmetricKey_Free(otherAppPublicKey);
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            result = false;
        }else{
            // Retrieve cipher length
            if(STATUS_OK != SOPC_CryptoProvider_SymmetricGetLength_Encryption(scConnection->cryptoProvider,
                                                                         plainDataLength,
                                                                         cipherDataLength)){
                result = false;
            }
        }
    }

    return result;
}

static bool SC_Chunks_EncodeSignature(SOPC_SecureConnection* scConnection,
                                      SOPC_Buffer*           buffer,
                                      bool                   symmetricAlgo,
                                      uint32_t               signatureSize)
{
    bool result = false;
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_ByteString signedData;

    if(symmetricAlgo == false){
            const SOPC_AsymmetricKey* runningAppPrivateKey = NULL;
            if(scConnection->isServerConnection == false){
                SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
                assert(scConfig != NULL);
                runningAppPrivateKey = scConfig->key_priv_cli;
            }else{
                SOPC_Endpoint_Config* epConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
                assert(epConfig != NULL);
                runningAppPrivateKey = epConfig->serverKey;
            }

            if(runningAppPrivateKey != NULL){
                status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
            }

            if(STATUS_OK == status){
                status = SOPC_CryptoProvider_AsymmetricSign(scConnection->cryptoProvider,
                                                       buffer->data,
                                                       buffer->length,
                                                       runningAppPrivateKey,
                                                       signedData.Data,
                                                       signedData.Length);
            }

            if(STATUS_OK == status){
                status = SOPC_Buffer_Write(buffer,
                                           signedData.Data,
                                           signedData.Length);
            }
            if(STATUS_OK == status){
                result = true;
            }
            SOPC_ByteString_Clear(&signedData);
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            result = false;
        }else{
            status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
            if(status == STATUS_OK){
                status = SOPC_CryptoProvider_SymmetricSign
                          (scConnection->cryptoProvider,
                           buffer->data,
                           buffer->length,
                           scConnection->currentSecuKeySets.senderKeySet->signKey,
                           signedData.Data,
                           signedData.Length);
            }

            if(STATUS_OK == status){
                status = SOPC_Buffer_Write(buffer,
                                           signedData.Data,
                                           signedData.Length);
            }
            if(STATUS_OK == status){
                result = true;
            }

            SOPC_ByteString_Clear(&signedData);
        }
    }
    return result;
}

static bool SC_Chunks_EncryptMsg(SOPC_SecureConnection* scConnection,
                                 SOPC_Buffer*           nonEncryptedBuffer,
                                 bool                   symmetricAlgo,
                                 uint32_t               sequenceNumberPosition,
                                 uint32_t               encryptedDataLength,
                                 SOPC_Buffer*           encryptedBuffer)
{
    assert(scConnection != NULL);
    assert(nonEncryptedBuffer != NULL);
    assert(encryptedBuffer != NULL);
    bool result = false;

    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte* dataToEncrypt = &nonEncryptedBuffer->data[sequenceNumberPosition];
    const uint32_t dataToEncryptLength = nonEncryptedBuffer->length - sequenceNumberPosition;

    if(symmetricAlgo == false){
        /* ASYMMETRIC CASE */

        SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        assert(scConfig != NULL); // Even on server side it is guaranteed by secure connection state manager (no sending in wrong state)
        const SOPC_Certificate* otherAppCertificate = NULL;
        if(scConnection->isServerConnection == false){
            // Client side
            otherAppCertificate = scConfig->crt_srv;
        }else{
            // Server side
            otherAppCertificate = scConfig->crt_cli;
        }

        SOPC_AsymmetricKey* otherAppPublicKey = NULL;
        SOPC_Byte* encryptedData = NULL;

        // Retrieve other app public key from certificate
        status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate,
                                                                &otherAppPublicKey);

        // Check size of encrypted data array
        if(status == STATUS_OK)
        {
            result = true;
            if(encryptedBuffer->max_size <
                sequenceNumberPosition + encryptedDataLength)
            {
                result = false;
            }
            encryptedData = encryptedBuffer->data;
            if(encryptedData == NULL)
            {
                result = false;
            }else{
                // Copy non encrypted headers part
                memcpy(encryptedData, nonEncryptedBuffer->data, sequenceNumberPosition);
                // Set correct message size and encrypted buffer length
                assert(STATUS_OK == SOPC_Buffer_SetDataLength(encryptedBuffer,
                                                              sequenceNumberPosition + encryptedDataLength));
            }
        }

        // Encrypt
        if(result != false){
            status = SOPC_CryptoProvider_AsymmetricEncrypt
                      (scConnection->cryptoProvider,
                       dataToEncrypt,
                       dataToEncryptLength,
                       otherAppPublicKey,
                       &encryptedData[sequenceNumberPosition],
                       encryptedDataLength);

            if(STATUS_OK != status){
                result = false;
            }
        }

        SOPC_KeyManager_AsymmetricKey_Free(otherAppPublicKey);

    }else{
        /* SYMMETRIC CASE */

        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            result = false;
        }else{
            result = true;
            SOPC_Byte* encryptedData = NULL;

            // Check size of encrypted data array
            if(encryptedBuffer->max_size < sequenceNumberPosition + encryptedDataLength){
                result = false;
            }
            encryptedData = encryptedBuffer->data;
            if(encryptedData == NULL){
                result = false;
            }else{
                // Copy non encrypted headers part
                memcpy(encryptedData, nonEncryptedBuffer->data, sequenceNumberPosition);
                // Set correct message size and encrypted buffer length
                assert(STATUS_OK == SOPC_Buffer_SetDataLength(encryptedBuffer,
                                                              sequenceNumberPosition + encryptedDataLength));

            }

            // Encrypt
            if(result != false){
                status = SOPC_CryptoProvider_SymmetricEncrypt
                          (scConnection->cryptoProvider,
                           dataToEncrypt,
                           dataToEncryptLength,
                           scConnection->currentSecuKeySets.senderKeySet->encryptKey,
                           scConnection->currentSecuKeySets.senderKeySet->initVector,
                           &encryptedData[sequenceNumberPosition],
                           encryptedDataLength);
                if(STATUS_OK != status){
                    result = false;
                }
            }

        } // End valid key set
    }
    return result;
}

static SOPC_StatusCode SC_Chunks_TreatSendBuffer(SOPC_SecureConnection* scConnection,
                                                 uint32_t               optRequestId,
                                                 SOPC_Msg_Type          sendMsgType,
                                                 bool                   isSendTcpOnly,
                                                 bool                   isOPN,
                                                 SOPC_Buffer*           inputBuffer,
                                                 SOPC_Buffer**          outputBuffer,
                                                 SOPC_StatusCode*       errorStatus){
    assert(scConnection!= NULL);
    assert(inputBuffer != NULL);
    assert(outputBuffer != NULL);
    assert(errorStatus != NULL);
    SOPC_SecureChannel_Config* scConfig = NULL;
    bool result = false;
    SOPC_StatusCode status;
    SOPC_Buffer* nonEncryptedBuffer;
    uint32_t requestId = 0;
    uint32_t bodySize = 0;
    uint32_t sequenceNumberPosition = 0; // Position from which encryption start
    uint32_t tokenId = 0;
    uint32_t senderCertificateSize = 0;
    uint32_t securityPolicyLength = 0;
    uint32_t signatureSize = 0;
    bool     hasPadding = false;
    uint16_t realPaddingLength = 0; // padding + extra total size
    bool     hasExtraPadding = false;
    uint32_t encryptedDataLength = 0;

    /* PRE-CONFIG PHASE */

    // Set the position at the beginning of the buffer (to be read or to could encode header for which space was left)
    assert(STATUS_OK == SOPC_Buffer_SetPosition(inputBuffer, 0));

    if(isOPN != false){
        // In specific case of OPN the input buffer contains only message body
        // (without bytes reserved for headers since it is not static size)
        assert(scConnection->tcpMsgProperties.sendBufferSize > 0);
        nonEncryptedBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    }else{
        // In other cases the input buffer contains the message body
        //  but also the reserved bytes for headers before body bytes
        nonEncryptedBuffer = inputBuffer;
    }

    /* ENCODE OPC UA TCP HEADER PHASE */
    result = SC_Chunks_EncodeTcpMsgHeader(scConnection,
                                          sendMsgType,
                                          true, // isFinal
                                          nonEncryptedBuffer);
    if(result != false){
        if(isSendTcpOnly == false){
            /* ENCODE OPC UA SECURE CONVERSATION MESSAGE PHASE*/

            // Note: when sending a secure conversation message, the secure connection configuration shall be defined
            scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL); // Even on server side guaranteed by the secure connection state manager

            bool toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, isOPN);
            bool toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);

            /* ENCODE OPC UA SECURE CONVERSATION MESSAGE EXTRA FIELD (secure channel Id) */
            status = SOPC_UInt32_Write(&scConnection->currentSecurityToken.secureChannelId, nonEncryptedBuffer);
            if(STATUS_OK != status){
                *errorStatus = OpcUa_BadEncodingError;
                result = false;
            }

            if(isOPN == false){
                // SYMMETRIC SECURITY CASE
                sequenceNumberPosition = SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                         SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH;

                /* CHECK MAX BODY SIZE */
                assert(scConnection->symmSecuMaxBodySize != 0);
                // Note: buffer already contains the message body (buffer length == end of body)
                bodySize = nonEncryptedBuffer->length - // symm headers
                           (SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                            SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                            SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                if(bodySize > scConnection->symmSecuMaxBodySize){
                    // Note: we do not manage several chunks for now (expected place to manage it)
                    result = false;
                    if(scConnection->isServerConnection == false){
                        *errorStatus = OpcUa_BadRequestTooLarge;
                    }else{
                        *errorStatus = OpcUa_BadResponseTooLarge;
                    }
                }

                if(result != false){
                    /* ENCODE SYMMETRIC SECURITY HEADER */
                    //  retrieve tokenId
                    if(scConnection->isServerConnection != false &&
                       scConnection->serverNewSecuTokenActive == false){
                        // Server side only (SC renew): new token is not active yet, use the precedent token
                        // TODO: timeout on precedent token validity to be implemented
                        assert(scConnection->precedentSecurityToken.tokenId != 0 &&
                                scConnection->precedentSecurityToken.secureChannelId != 0);
                        tokenId = scConnection->precedentSecurityToken.tokenId;
                    }else{
                        // Use current token
                        tokenId = scConnection->currentSecurityToken.tokenId;
                    }
                    //  encode tokenId
                    if(STATUS_OK != SOPC_UInt32_Write(&tokenId, nonEncryptedBuffer)){
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }

                // Set position to end of body
                if(STATUS_OK != SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }else{
                // ASYMMETRIC SECURITY CASE

                /* ENCODE ASYMMETRIC SECURITY HEADER */
                if(result != false){
                    result = SC_Chunks_EncodeAsymSecurityHeader(scConnection,
                                                                scConfig,
                                                                nonEncryptedBuffer,
                                                                &senderCertificateSize,
                                                                &securityPolicyLength,
                                                                errorStatus);
                    if(result == false){
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }

                if(result != false){
                    // Compute max body sizes (asymm. and symm.) and check asymmetric max body size

                    // Next position is the sequence number position
                    sequenceNumberPosition = nonEncryptedBuffer->position;

                    if(scConnection->asymmSecuMaxBodySize == 0 &&
                            scConnection->symmSecuMaxBodySize == 0){
                        scConnection->asymmSecuMaxBodySize = SC_Chunks_GetMaxBodySize(scConnection,
                                                                                      scConfig,
                                                                                      nonEncryptedBuffer->max_size,
                                                                                      sequenceNumberPosition,
                                                                                      false);
                        scConnection->symmSecuMaxBodySize = SC_Chunks_GetMaxBodySize(scConnection,
                                                                                     scConfig,
                                                                                     nonEncryptedBuffer->max_size,
                                                                                     // sequenceNumber position for symmetric case:
                                                                                     (SOPC_UA_HEADER_LENGTH_POSITION +
                                                                                      SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH),
                                                                                     true);
                    }
                    if(scConnection->asymmSecuMaxBodySize == 0 ||
                       scConnection->symmSecuMaxBodySize == 0){
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }else{
                        // Check asymmetric max body size compliant
                        if(inputBuffer->length > scConnection->asymmSecuMaxBodySize){
                            result = false;
                            if(scConnection->isServerConnection == false){
                                // Client side
                                *errorStatus = OpcUa_BadRequestTooLarge;
                            }else{
                                // Server side
                                *errorStatus = OpcUa_BadResponseTooLarge;
                            }
                        }
                    }
                }

                // Add necessary bytes for encoding sequence header later
                if(result != false){
                    assert(nonEncryptedBuffer->length < nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                    if(STATUS_OK != SOPC_Buffer_SetDataLength(nonEncryptedBuffer, nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH) ||
                       STATUS_OK != SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH)){
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }

                // Copy body bytes from input buffer
                if(STATUS_OK != SOPC_Buffer_Write(nonEncryptedBuffer, inputBuffer->data, inputBuffer->length)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            } // End of Symmetric/Asymmetric security header encoding (+body content bytes)

            if(result != false){
                /* ENCODE PADDING */
                if(toEncrypt != false){
                    hasPadding = true;
                    result = SOPC_Chunks_EncodePadding(scConnection,
                                                       scConfig,
                                                       nonEncryptedBuffer,
                                                       isOPN == false, // isSymmetricAlgo
                                                       sequenceNumberPosition,
                                                       &signatureSize,
                                                       &realPaddingLength,
                                                       &hasExtraPadding);
                    if(result == false){
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }else if(toSign != false){
                    /* SIGN ONLY: ONLY DEFINE SIGNATURE SIZE */
                    bool tmpBool;
                    uint32_t tmpInt;
                    // TODO: avoid to compute non necessary crypto values
                    SC_Chunks_GetCryptoSizes(scConnection,
                                             scConfig,
                                             isOPN == false,
                                             &tmpBool,
                                             &tmpBool,
                                             &signatureSize,
                                             &tmpInt,
                                             &tmpInt);
                }else{
                    signatureSize = 0;
                    hasPadding = false;
                    realPaddingLength = 0;
                    hasExtraPadding = false;
                }
            }

            if(result != false && isOPN != false){
                // ASYMMETRIC SECURITY SPECIFIC CASE: check MaxSenderCertificate side (padding necessary)
                // TODO: since we already encoded everything except signature, is it really necessary ?
                result = SC_Chunks_CheckMaxSenderCertificateSize(senderCertificateSize,
                                                                 nonEncryptedBuffer->max_size,
                                                                 securityPolicyLength,
                                                                 hasPadding,
                                                                 realPaddingLength,
                                                                 hasExtraPadding,
                                                                 signatureSize);
                if(result == false){
                    if(scConnection->isServerConnection == false){
                        // Client side
                        *errorStatus = OpcUa_BadRequestTooLarge;
                    }else{
                        // Server side
                        *errorStatus = OpcUa_BadResponseTooLarge;
                    }
                }
            }


            if(result != false){
                /* ENCODE (ENCRYPTED) MESSAGE SIZE */

                // Set position to message size field
                if(STATUS_OK != SOPC_Buffer_SetPosition(nonEncryptedBuffer, SOPC_UA_HEADER_LENGTH_POSITION)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }else{
                    uint32_t messageSize = 0;
                    if(toEncrypt == false){
                        // Size = current buffer length + signature if signed
                        messageSize = nonEncryptedBuffer->length + signatureSize;
                        if(STATUS_OK != SOPC_UInt32_Write(&messageSize, nonEncryptedBuffer)){
                            result = false;
                            *errorStatus = OpcUa_BadTcpInternalError;
                        }
                    }else{
                        // Compute final encrypted message length:
                        // Data to encrypt = already encoded message from encryption start + signature size
                        const uint32_t plainDataToEncryptLength =
                                nonEncryptedBuffer->length - sequenceNumberPosition + signatureSize;

                        result = SC_Chunks_GetEncryptedDataLength(scConnection,
                                scConfig,
                                plainDataToEncryptLength,
                                isOPN == false, // isSymmetricAlgo
                                &encryptedDataLength);

                        if(result != false){
                            messageSize = sequenceNumberPosition + encryptedDataLength; // non encrypted length + encrypted length
                            if(STATUS_OK != SOPC_UInt32_Write(&messageSize, nonEncryptedBuffer)){
                                result = false;
                                *errorStatus = OpcUa_BadTcpInternalError;
                            }
                        }
                    }
                }
            }

            if(result != false){
                /* ENCODE SEQUENCE NUMBER */

                // Set position to sequence number
                if(STATUS_OK != SOPC_Buffer_SetPosition(nonEncryptedBuffer, sequenceNumberPosition)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }else{
                    if(scConnection->tcpSeqProperties.lastSNsent > UINT32_MAX - 1024){ // Part 6 §6.7.2 v1.03
                        scConnection->tcpSeqProperties.lastSNsent = 1;
                    }else{
                        scConnection->tcpSeqProperties.lastSNsent = scConnection->tcpSeqProperties.lastSNsent + 1;
                    }
                    if(STATUS_OK != SOPC_UInt32_Write(&scConnection->tcpSeqProperties.lastSNsent, nonEncryptedBuffer)){
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }
            }

            if(result != false){
                /* ENCODE REQUEST ID */

                if(scConnection->isServerConnection == false){
                    requestId = (scConnection->clientLastReqId + 1) % UINT32_MAX;
                    if(requestId == 0){
                        requestId = 1;
                    }
                    scConnection->clientLastReqId = requestId;
                }else{
                    requestId = optRequestId;
                }

                if(STATUS_OK != SOPC_UInt32_Write(&requestId, nonEncryptedBuffer)){
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }

            if(result != false){
                // Set the buffer at the end for next write
                assert(STATUS_OK == SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length));
            }

            if(result != false && toSign != false){
                /* SIGN MESSAGE */
                result = SC_Chunks_EncodeSignature(scConnection,
                                                   nonEncryptedBuffer,
                                                   isOPN == false, // = isSymmetric
                                                   signatureSize);
            }

            if(result != false){
                /* ENCRYTP MESSAGE */
                if(toEncrypt != false){

                    SOPC_Buffer* encryptedBuffer = NULL;

                    if(scConnection->tcpMsgProperties.sendBufferSize >= sequenceNumberPosition + encryptedDataLength){
                        encryptedBuffer = SOPC_Buffer_Create(sequenceNumberPosition + encryptedDataLength);
                    }else{
                        // TODO: return status message too large ? => is there any guarantee due to plain buffer size ?
                    }

                    if(encryptedBuffer != NULL){
                        result = SC_Chunks_EncryptMsg(scConnection,
                                                      nonEncryptedBuffer,
                                                      isOPN == false, // = isSymmetric
                                                      sequenceNumberPosition,
                                                      encryptedDataLength,
                                                      encryptedBuffer);
                    }else{
                        result = false;
                    }

                    if(result != false){
                        *outputBuffer = encryptedBuffer;
                    }

                    if(inputBuffer != nonEncryptedBuffer){
                        // If it is only an internal buffer, it shall be freed here
                        // otherwise it is the input buffer freed by caller
                        SOPC_Buffer_Delete(nonEncryptedBuffer);
                        nonEncryptedBuffer = NULL;
                    }
                }else{
                    // No encryption output buffer is non encrypted buffer
                    *outputBuffer = nonEncryptedBuffer;
                }
            }

            if(result != false && scConnection->isServerConnection == false){
                SOPC_Msg_Type* msgType = NULL;
                switch(sendMsgType){
                case SOPC_MSG_TYPE_SC_OPN:
                case SOPC_MSG_TYPE_SC_MSG:
                    /* CLIENT SIDE: RECORD REQUEST SENT (response expected)*/
                    msgType = calloc(1, sizeof(SOPC_Msg_Type));
                    if(msgType != NULL){
                        *msgType = sendMsgType;
                        if(msgType != SOPC_SLinkedList_Append(scConnection->tcpSeqProperties.sentRequestIds, requestId, (void*) msgType)){
                            result = false;
                        }
                    }else{
                        result = false;
                    }
                    break;
                case SOPC_MSG_TYPE_SC_CLO:
                    // No response expected
                    break;
                case SOPC_MSG_TYPE_HEL:
                case SOPC_MSG_TYPE_ACK:
                case SOPC_MSG_TYPE_ERR:
                case SOPC_MSG_TYPE_INVALID:
                default:
                    assert(false);
                }
            }

        }else{
            // simple OPC UA TCP message: just set the output buffer
            *outputBuffer = nonEncryptedBuffer;
        }
    }else{
        *errorStatus = OpcUa_BadEncodingError;
    }

    return result;
}


void SOPC_ChunksMgr_Dispatcher(SOPC_SecureChannels_InputEvent event,
                               uint32_t                       eltId,
                               void*                          params,
                               int32_t                        auxParam){
    SOPC_Msg_Type sendMsgType = SOPC_MSG_TYPE_INVALID;
    SOPC_Buffer* buffer = (SOPC_Buffer*) params;
    SOPC_Buffer* outputBuffer = NULL;
    SOPC_StatusCode errorStatus = STATUS_OK;
    bool isSendCase = false;
    bool isSendTcpOnly = false;
    bool isOPN = false;
    bool result = false;
    // True if socket will be closed after sending this message (ERR, CLO)
    bool socketWillClose = false;
    SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);

    assert(buffer != NULL);

    if(scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED){
        switch(event){
        /* Sockets events: */
        case SOCKET_RCV_BYTES:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: SOCKET_RCV_BYTES\n");
            }
            /* id = secure channel connection index,
           params = (SOPC_Buffer*) received buffer */
            if(scConnection != NULL){
                SC_Chunks_TreatReceivedBuffer(scConnection,
                        eltId,
                        buffer);
            } // else: socket should already receive close request
            break;
            /* SC connection manager -> OPC UA chunks message manager */
            // id = secure channel connection index,
            // params = (SOPC_Buffer*) buffer positioned to message payload
            // auxParam = request Id context if response
        case INT_SC_SND_HEL:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: INT_SC_SND_HEL\n");
            }
            isSendCase = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_HEL;
            break;
        case INT_SC_SND_ACK:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: INT_SC_SND_ACK\n");
            }
            isSendCase = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_ACK;
            break;
        case INT_SC_SND_ERR:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: INT_SC_SND_ERR\n");
            }
            socketWillClose = true;
            isSendCase = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_ERR;
            break;
        case INT_SC_SND_OPN:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: INT_SC_SND_OPN\n");
            }
            isSendCase = true;
            isOPN = true;
            sendMsgType = SOPC_MSG_TYPE_SC_OPN;
            // Note: only message to be provided without size of header reserved (variable size for asymmetric secu header)
            break;
        case INT_SC_SND_CLO:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: INT_SC_SND_CLO\n");
            }
            socketWillClose = true;
            isSendCase = true;
            sendMsgType = SOPC_MSG_TYPE_SC_CLO;
            break;
        case INT_SC_SND_MSG_CHUNKS:
            if(SOPC_DEBUG_PRINTING != false){
                printf("ScChunksMgr: INT_SC_SND_MSG_CHUNKS\n");
            }
            isSendCase = true;
            sendMsgType = SOPC_MSG_TYPE_SC_MSG;
            break;
        default:
            // Already filtered by secure channels API module
            assert(false);
        }

        if(isSendCase != false){
            result = SC_Chunks_TreatSendBuffer(scConnection,
                    auxParam,
                    sendMsgType,
                    isSendTcpOnly,
                    isOPN,
                    buffer,
                    &outputBuffer,
                    &errorStatus);
            if(result == false){
                if(socketWillClose == false){
                    // Treat as prio events
                    SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_FAILURE,
                            eltId,
                            params,
                            errorStatus);
                }else{
                    if(SOPC_DEBUG_PRINTING){
                        printf("Failed sending message type '%d' before socket closed\n", sendMsgType);
                    }
                }
            }else{
                // Require write of output buffer on socket
                SOPC_Sockets_EnqueueEvent(SOCKET_WRITE,
                        scConnection->socketIndex,
                        (void*) outputBuffer,
                        0);

                if(buffer != outputBuffer){
                    // If input buffer not reused for sending on socket: delete it
                    SOPC_Buffer_Delete(buffer);
                }
            }
        }
    }else{ // SC not connected: ignore event and delete buffer data
        SOPC_Buffer_Delete(buffer);
    }
}

