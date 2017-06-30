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

#include "sopc_secure_channel_low_level.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "secret_buffer.h"
#include "crypto_provider.h"
#include "sopc_encoder.h"
#include "sopc_builtintypes.h"
#include "sopc_tcp_ua_low_level.h"

const uint32_t scProtocolVersion = 0;

typedef struct SOPC_OperationEnd_FlushSecureMsgData {
    SOPC_MsgBuffer*              msgBuffers;
    uint32_t                     nextFlushChunkIdx;
    uint32_t                     tokenId;
    uint32_t                     requestId;
    uint32_t                     precSeqNum;
    SOPC_Socket_EndOperation_CB* callback;
    void*                        callbackData;
    SOPC_StatusCode              errorStatus;
} SOPC_OperationEnd_FlushSecureMsgData;

SC_Connection* SC_Create (TCP_UA_Connection* connection){
    SC_Connection* sConnection = NULL;

    if(connection != NULL){
        sConnection = (SC_Connection *) malloc(sizeof(SC_Connection));

        if(sConnection != 0){
            memset (sConnection, 0, sizeof(SC_Connection));
            sConnection->transportConnection = connection;
            sConnection->state = SC_Connection_Disconnected;
            SOPC_ByteString_Initialize(&sConnection->runningAppCertificate);
            SOPC_ByteString_Initialize(&sConnection->otherAppCertificate);
            SOPC_String_Initialize(&sConnection->currentSecuPolicy);
            SOPC_String_Initialize(&sConnection->precSecuPolicy);

            SOPC_ActionQueue_Init(&sConnection->msgQueue, "Connection msgs to send");
            sConnection->msgQueueToken = 1;
        }else{
            TCP_UA_Connection_Delete(connection);
        }
    }
    return sConnection;
}

void SC_Disconnect(SC_Connection* scConnection){
    if(scConnection != NULL && scConnection->state != SC_Connection_Disconnected)
    {
        scConnection->state = SC_Connection_Disconnected;
        TCP_UA_Connection_Disconnect(scConnection->transportConnection);
    }
}

void SC_Delete (SC_Connection* scConnection){
    if(scConnection != NULL){
        // Do not delete runningAppCertificate, runnigAppPrivateKey and otherAppCertificate:
        //  managed by upper level
        SOPC_ByteString_Clear(&scConnection->runningAppCertificate);
        scConnection->runningAppPublicKeyCert = NULL;
        scConnection->runningAppPrivateKey = NULL; // DO NOT DEALLOCATE: manage by upper level
        SOPC_ByteString_Clear(&scConnection->otherAppCertificate);
        if(scConnection->otherAppPublicKeyCert != NULL &&
           scConnection->transportConnection->serverSideConnection != FALSE){
            // On server side only
            KeyManager_Certificate_Free((Certificate*) scConnection->otherAppPublicKeyCert);
        }
        scConnection->otherAppPublicKeyCert = NULL;
        if(scConnection->receptionBuffers != NULL)
        {
            MsgBuffers_Delete(&scConnection->receptionBuffers);
        }
        if(scConnection->transportConnection != NULL)
        {
            TCP_UA_Connection_Delete(scConnection->transportConnection);
            scConnection->transportConnection = NULL;
        }
        SOPC_String_Clear(&scConnection->currentSecuPolicy);
        SOPC_String_Clear(&scConnection->precSecuPolicy);
        SecretBuffer_DeleteClear(scConnection->currentNonce);
        KeySet_Delete(scConnection->currentSecuKeySets.receiverKeySet);
        KeySet_Delete(scConnection->currentSecuKeySets.senderKeySet);
        KeySet_Delete(scConnection->precSecuKeySets.receiverKeySet);
        KeySet_Delete(scConnection->precSecuKeySets.senderKeySet);
        CryptoProvider_Free(scConnection->currentCryptoProvider);
        CryptoProvider_Free(scConnection->precCryptoProvider);
        SOPC_ActionQueue_Free(&scConnection->msgQueue);
        free(scConnection);
    }
}

// SINGLE FUNCTION TO TAKE TOKEN
void SC_Action_TreateMsgQueue(void* arg){
    SC_Connection* scConnection = (SC_Connection*) arg;
    SOPC_StatusCode status = STATUS_NOK;
    SOPC_ActionFunction* fctPointer = NULL;
    void*                fctArgument = NULL;
    const char*          actionText = NULL;
    assert(NULL != scConnection);
    if(FALSE != scConnection->msgQueueToken){
        status = SOPC_Action_NonBlockingDequeue(scConnection->msgQueue,
                                                &fctPointer,
                                                &fctArgument,
                                                &actionText);
        if(STATUS_OK == status){
            scConnection->msgQueueToken = FALSE; // Take the token
            fctPointer(fctArgument); // Call the message treatment => treatment must call
        }else if(OpcUa_BadWouldBlock == status){
            status = STATUS_OK; // Nothing to treat, SC_TreatMsgQueue will be called once a new action is enqueued
        }else{
            status = STATUS_NOK;
        }
    } // else nothing to do, SC_TreatMsgQueue will be called once token is returned
}

// SINGLE FUNCTION TO RELEASE TOKEN
void SC_CreateAction_ReleaseToken(SC_Connection* scConnection){
    assert(NULL != scConnection);
    assert(scConnection->msgQueueToken == FALSE);
    scConnection->msgQueueToken = 1;
    SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                      SC_Action_TreateMsgQueue,
                                      (void*) scConnection,
                                      "Releasing message queue token");
}

SOPC_StatusCode SC_InitApplicationIdentities(SC_Connection*       scConnection,
                                             uint8_t              noneSecurityMode,
                                             const Certificate*   runningAppCertificate,
                                             const AsymmetricKey* runningAppPrivateKey,
                                             const Certificate*   otherAppCertificate){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t certLength = 0;
    if(scConnection->runningAppCertificate.Length <= 0 &&
       scConnection->runningAppPrivateKey == NULL &&
       scConnection->otherAppPublicKeyCert == NULL &&
       scConnection->otherAppCertificate.Length <= 0)
    {
        if(runningAppCertificate == NULL && runningAppPrivateKey == NULL &&
           otherAppCertificate == NULL &&
           noneSecurityMode != FALSE)
        {
            status = STATUS_OK;; // None security mode: no certificate to use
        }else if(runningAppCertificate != NULL && runningAppPrivateKey != NULL &&
                 (otherAppCertificate != NULL || // For a client side connection other app certificate is mandatory
                  scConnection->transportConnection->serverSideConnection != FALSE))
        {
            scConnection->runningAppPublicKeyCert = runningAppCertificate;
            scConnection->runningAppPrivateKey = runningAppPrivateKey;
            status = KeyManager_Certificate_CopyDER(runningAppCertificate,
                                                    &scConnection->runningAppCertificate.Data,
                                                    &certLength);
            if(certLength > INT32_MAX){
                status = STATUS_NOK;
            }else{
                scConnection->runningAppCertificate.Length = (int32_t) certLength;
            }

            if(otherAppCertificate != NULL){
                scConnection->otherAppPublicKeyCert = otherAppCertificate;
                if(STATUS_OK == status){
                    certLength = 0;
                    status = KeyManager_Certificate_CopyDER(otherAppCertificate,
                                                            &scConnection->otherAppCertificate.Data,
                                                            &certLength);
                }

                if(certLength > INT32_MAX){
                    status = STATUS_NOK;
                }else{
                    scConnection->otherAppCertificate.Length = (int32_t) certLength;
                }
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

SOPC_StatusCode SC_InitOtherAppIdentity(SC_Connection*       scConnection,
                                        Certificate*         otherAppCertificate,
                                        SOPC_ByteString*     otherAppCertificateBs){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection->otherAppPublicKeyCert == NULL &&
       scConnection->otherAppCertificate.Length <= 0)
    {
        status = SOPC_ByteString_Copy(&scConnection->otherAppCertificate, otherAppCertificateBs);
        if(STATUS_OK == status){
            scConnection->otherAppPublicKeyCert = otherAppCertificate;
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

SOPC_StatusCode SC_InitReceiveSecureBuffers(SC_Connection* scConnection,
                                            SOPC_NamespaceTable*  namespaceTable,
                                            SOPC_EncodeableType** encodeableTypes)
{
    SOPC_StatusCode status = STATUS_INVALID_STATE;
    if(scConnection->receptionBuffers == NULL){
        if(scConnection->transportConnection->maxChunkCountRcv != 0)
        {
            scConnection->receptionBuffers = MsgBuffers_Create
             (scConnection->transportConnection->maxChunkCountRcv,
              scConnection->transportConnection->receiveBufferSize,
              NULL,
              namespaceTable,
              encodeableTypes);

        }else if(scConnection->transportConnection->maxMessageSizeRcv != 0){
            // Is message size including whole message or only body as it is the case in part 4 §5.3 last § ???
            scConnection->receptionBuffers = MsgBuffers_Create
              (scConnection->transportConnection->maxMessageSizeRcv
               /scConnection->transportConnection->receiveBufferSize,
               scConnection->transportConnection->receiveBufferSize,
               NULL,
               namespaceTable,
               encodeableTypes);
            // Using receive buffer size is correct over-approximation of un-encrypted size (blocks) ?
        }else{
            // We need to know either nbChunks or max message size to could limit the number of buffers:
            // Check in init ?
            assert(FALSE);
        }
        if(scConnection->receptionBuffers == NULL){
            status = STATUS_NOK;
        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_MsgBuffers* SC_CreateSendSecureBuffers(uint32_t              maxChunksSendingCfg,
                                            uint32_t              maxMsgSizeSendingCfg,
                                            uint32_t              bufferSizeSendingCfg,
                                            SC_Connection*        flushData,
                                            SOPC_NamespaceTable*  nsTableCfg,
                                            SOPC_EncodeableType** encTypesTableCfg)
{
    uint32_t idx;
    SOPC_MsgBuffers* msgBuffers = NULL;
    if(maxChunksSendingCfg != 0)
    {
        msgBuffers = MsgBuffers_Create(maxChunksSendingCfg,
                                       bufferSizeSendingCfg,
                                       (void*) flushData,
                                       nsTableCfg,
                                       encTypesTableCfg);

    }else if(maxMsgSizeSendingCfg != 0){
        msgBuffers = MsgBuffers_Create(maxMsgSizeSendingCfg / bufferSizeSendingCfg,
                                       bufferSizeSendingCfg,
                                       (void*) flushData,
                                       nsTableCfg,
                                       encTypesTableCfg);
    }
    // Set a current chunk
    MsgBuffers_NextChunk(msgBuffers, &idx);
    return msgBuffers;
}

//// Cryptographic properties helpers

uint8_t Is_ExtraPaddingSizePresent(uint32_t plainBlockSize){
    // Extra-padding necessary if padding could be greater 256 bytes (2048 bits)
    // (1 byte for padding size field + 255 bytes of padding).
    // => padding max value is plainBlockSize regarding the formula of padding size
    //    (whereas spec part 6 indicates it depends on the key size which is incorrect)
    return plainBlockSize > 256;
}

uint32_t GetMaxBodySize(uint32_t nonEncryptedHeadersSize,
                        uint32_t chunkSize,
                        uint8_t  toEncrypt,
                        uint32_t cipherBlockSize,
                        uint32_t plainBlockSize,
                        uint8_t  toSign,
                        uint32_t signatureSize)
{
    uint32_t result = 0;
    uint32_t paddingSizeFields = 0;
    if(toEncrypt == FALSE){
        // No encryption => consider same size blocks and no padding size fields
        cipherBlockSize = 1;
        plainBlockSize = 1;
        paddingSizeFields = 0;
    }else{
        // By default only 1 byte for padding size field. +1 if extra padding
        paddingSizeFields = 1;
        if(Is_ExtraPaddingSizePresent(plainBlockSize) != FALSE){
            paddingSizeFields += 1;
        }
    }

    if(toSign == FALSE){
        signatureSize = 0;
    }

    // Ensure cipher block size is greater or equal to plain block size:
    //  otherwise the plain size could be greater than the  buffer size regarding computation
    assert(cipherBlockSize >= plainBlockSize);

    const uint32_t bodyChunkSize = chunkSize - nonEncryptedHeadersSize; // Body includes sequence header (encrypted)

    // Computed maxBlock and then maxBodySize based on revised formula of mantis ticket #2897
    // Spec 1.03 part 6 incoherent
    const uint32_t maxBlocks = bodyChunkSize / cipherBlockSize;


    // MaxBodySize = unCiphered block size * max blocs - sequence header -1 for PaddingSize field(s) - signature size
    // <=> Maximum body size after adding the sequence header, padding size fields (padding could be 0) and signature in plain text available size
    result = plainBlockSize * maxBlocks - UA_SECURE_MESSAGE_SEQUENCE_LENGTH - signatureSize - paddingSizeFields;
    // Maximum body size (+headers+signature+padding size fields) cannot be greater than maximum buffer size
    assert(chunkSize >=
           (nonEncryptedHeadersSize + UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
            result + signatureSize + paddingSizeFields));
    return result;
}

// Get information from internal properties
SOPC_StatusCode GetEncryptedDataLength(SC_Connection* scConnection,
                                       uint32_t       plainDataLength,
                                       uint32_t       symmetricAlgo,
                                       uint32_t*      cipherDataLength)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(scConnection != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == FALSE){
        if(scConnection->otherAppCertificate.Length <= 0){
           status = STATUS_INVALID_STATE;
        }else{

            AsymmetricKey* otherAppPublicKey = NULL;

            status = KeyManager_AsymmetricKey_CreateFromCertificate(scConnection->otherAppPublicKeyCert,
                                                                    &otherAppPublicKey);

            // Retrieve cipher length
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricGetLength_Encryption(scConnection->currentCryptoProvider,
                                                                       otherAppPublicKey,
                                                                       plainDataLength,
                                                                       cipherDataLength);
            }

            KeyManager_AsymmetricKey_Free(otherAppPublicKey);
        }
    }else if (status == STATUS_OK){
        if(scConnection->currentSecuKeySets.senderKeySet == NULL){
            status = STATUS_INVALID_STATE;
        }else{
            // Retrieve cipher length
            status = CryptoProvider_SymmetricGetLength_Encryption
                      (scConnection->currentCryptoProvider,
                      plainDataLength,
                      cipherDataLength);
        }
    }

    return status;
}

// Check OPCUA cryptographic properties

uint32_t IsMsgEncrypted(OpcUa_MessageSecurityMode securityMode,
                        SOPC_MsgBuffer*           msgBuffer)
{
    assert(securityMode != OpcUa_MessageSecurityMode_Invalid);
    uint32_t toEncrypt = 1; // True
    // Determine if the message must be encrypted
    if(securityMode == OpcUa_MessageSecurityMode_None ||
       (securityMode == OpcUa_MessageSecurityMode_Sign &&
        msgBuffer->secureType != SOPC_OpenSecureChannel))
    {
        toEncrypt = FALSE;
    }

    return toEncrypt;
}

uint32_t IsMsgSigned(OpcUa_MessageSecurityMode securityMode)
{
    uint32_t toSign = 1; // True
    // Determine if the message must be signed
    if(securityMode == OpcUa_MessageSecurityMode_None)
    {
        toSign = FALSE;
    }
    return toSign;
}

SOPC_StatusCode CheckMaxSenderCertificateSize(SOPC_ByteString* senderCertificate,
                                              uint32_t         messageChunkSize,
                                              SOPC_String*     securityPolicyUri,
                                              uint8_t          hasPadding,
                                              uint32_t         padding,
                                              uint32_t         extraPadding,
                                              uint32_t         asymmetricSignatureSize){
    SOPC_StatusCode status = STATUS_NOK;
    int32_t maxSize = // Fit in a single message chunk with at least 1 byte of body
     messageChunkSize -
     UA_SECURE_MESSAGE_HEADER_LENGTH -
     4 - // URI length field size
     securityPolicyUri->Length -
     4 - // Sender certificate length field
     4 - // Receiver certificate thumbprint length field
     20 - // Receiver certificate thumbprint length
     8; // Sequence header size
    if(hasPadding != FALSE){
        maxSize =
         maxSize -
         1 - // padding length field size
         padding;
    }
    maxSize = maxSize - extraPadding - asymmetricSignatureSize;

    if(senderCertificate->Length <= maxSize){
        status = STATUS_OK;
    }
    return status;
}

uint16_t GetPaddingSize(uint32_t bytesToEncrypt, // called bytesToWrite in spec part6 but it should not since it includes SN header !
                        uint32_t plainBlockSize,
                        uint32_t signatureSize){
    // By default only 1 padding size field + 1 if extra padding
    uint8_t paddingSizeFields = 1;
    if(Is_ExtraPaddingSizePresent(plainBlockSize)){
        paddingSizeFields += 1;
    }
    return plainBlockSize - ((bytesToEncrypt + signatureSize + paddingSizeFields) % plainBlockSize);
}

// Set internal properties

SOPC_StatusCode SC_SetMaxBodySize(SC_Connection* scConnection,
                                  uint32_t       nonEncryptedHeadersSize,
                                  uint32_t       chunkSize,
                                  uint32_t       isSymmetric)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != NULL){
        status = STATUS_OK;
        uint8_t  toEncrypt = FALSE;
        uint32_t cipherBlockSize = 0;
        uint32_t plainBlockSize =0;
        uint8_t  toSign = FALSE;
        uint32_t signatureSize = 0;
        if(isSymmetric == FALSE){

            if(scConnection->currentSecuMode != OpcUa_MessageSecurityMode_None){
                AsymmetricKey* publicKey = NULL;


                // Asymmetric case: used only for opening channel, signature AND encryption mandatory in this case
                toEncrypt = 1; // TRUE
                toSign = 1; // TRUE

                status = KeyManager_AsymmetricKey_CreateFromCertificate(scConnection->otherAppPublicKeyCert,
                                                                        &publicKey);

                if(status == STATUS_OK){
                    // Compute block sizes
                    status = CryptoProvider_AsymmetricGetLength_Msgs(scConnection->currentCryptoProvider,
                                                                     publicKey,
                                                                     &cipherBlockSize,
                                                                     &plainBlockSize);
                }
                if(status == STATUS_OK){
                    // Compute signature size
                    status = CryptoProvider_AsymmetricGetLength_Signature(scConnection->currentCryptoProvider,
                                                                           publicKey,
                                                                           &signatureSize);
                }

                KeyManager_AsymmetricKey_Free(publicKey);

            }else{
                toEncrypt = FALSE; // No data encryption
                toSign = FALSE;    // No signature
            }
        }else{
            if(scConnection->currentSecuMode != OpcUa_MessageSecurityMode_None){

                if(scConnection->currentSecuMode == OpcUa_MessageSecurityMode_SignAndEncrypt){
                    // Encryption necessary: compute block sizes
                    toEncrypt = 1; // TRUE
                    status = CryptoProvider_SymmetricGetLength_Blocks(scConnection->currentCryptoProvider,
                                                                      &cipherBlockSize,
                                                                      &plainBlockSize);
                }else{
                    toEncrypt = FALSE;
                }

                if(status == STATUS_OK){
                    // Signature necessary in both Sign and SignAndEncrypt cases: compute signature size
                    toSign = 1; // TRUE
                    status = CryptoProvider_SymmetricGetLength_Signature(scConnection->currentCryptoProvider,
                                                                          &signatureSize);
                }
            }else{
                // No signature or encryption
                toEncrypt = FALSE;
                toSign = FALSE;
            }
        }

        // Compute the max body size regarding encryption and signature use
        if(status == STATUS_OK){
             scConnection->sendingMaxBodySize = GetMaxBodySize(nonEncryptedHeadersSize,
                                                               chunkSize,
                                                               toEncrypt,
                                                               cipherBlockSize,
                                                               plainBlockSize,
                                                               toSign,
                                                               signatureSize);
         }

    }
    return status;
}

//// End cryptographic properties helpers

SOPC_StatusCode SC_EncodeSecureMsgHeader(SOPC_MsgBuffers*       msgBuffers,
                                         SOPC_SecureMessageType smType,
                                         uint32_t               secureChannelId)
{
    // Important note: it is safe to consider we always write in first chunk (msgBuffers->buffers)
    //  of message buffers since headers are encoded once for and
    //  will then be copied for next chunks
    SOPC_StatusCode status = STATUS_NOK;
    assert(msgBuffers->buffers->position == 0);
    SOPC_Byte fByte = 'F';
    if(msgBuffers != NULL){
        status = STATUS_OK;
        switch(smType){
                case SOPC_SecureMessage:
                    status = Buffer_Write(msgBuffers->buffers, SOPC_MSG, 3);
                    break;
                case SOPC_OpenSecureChannel:
                    status = Buffer_Write(msgBuffers->buffers, SOPC_OPN, 3);
                    break;
                case SOPC_CloseSecureChannel:
                    status = Buffer_Write(msgBuffers->buffers, SOPC_CLO, 3);
                    break;
        }
        status = MsgBuffer_SetSecureMsgType(msgBuffers, smType);
        if(status == STATUS_OK){
            // Default behavior: final except if too long for SOPC_SecureMessage only !
            status = Buffer_Write(msgBuffers->buffers, &fByte, 1);
        }
        if(status == STATUS_OK){
            msgBuffers->isFinal = SOPC_Msg_Chunk_Final;
            // Temporary message size
            const uint32_t msgHeaderLength = UA_SECURE_MESSAGE_HEADER_LENGTH;
            status = SOPC_UInt32_Write(&msgHeaderLength, msgBuffers);
        }
        if(status == STATUS_OK){
            // Secure channel Id
            status = SOPC_UInt32_Write(&secureChannelId, msgBuffers);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
    }

    return status;
}

SOPC_StatusCode SC_EncodeSequenceHeader(SOPC_MsgBuffers* msgBuffers,
                                        uint32_t         requestId){
    // Important note: it is safe to consider we always write in first chunk (msgBuffers->buffers->...)
    //  of message buffers since headers are encoded once for and
    //  will then be copied for next chunks
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    const uint32_t zero = 0;
    if(msgBuffers != NULL){
        status = STATUS_OK;
    }

    // Set temporary SN value: to be set on message sending (ensure contiguous SNs)
    if(status == STATUS_OK){
        msgBuffers->sequenceNumberPosition = msgBuffers->buffers->position;
        SOPC_UInt32_Write(&zero, msgBuffers);
    }

    if(status == STATUS_OK){
        SOPC_UInt32_Write(&requestId, msgBuffers);
    }

    return status;
}

SOPC_StatusCode EncodeAsymmSecurityHeader(CryptoProvider*           cryptoProvider,
                                          SOPC_MsgBuffer*           msgBuffer,
                                          OpcUa_MessageSecurityMode secuMode,
                                          SOPC_String*              securityPolicy,
                                          SOPC_ByteString*          senderCertificate,
                                          const Certificate*        receiverCertCrypto){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    assert(msgBuffer->nbBuffers == 1);
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True
    if(cryptoProvider != NULL && msgBuffer != NULL &&
       securityPolicy != NULL &&
       senderCertificate != NULL)
    {
        toEncrypt = IsMsgEncrypted(secuMode, msgBuffer);
        toSign = IsMsgSigned(secuMode);
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        if(securityPolicy->Length>0){
            status = SOPC_String_Write(securityPolicy, msgBuffer);
        }else{
            // Null security policy is invalid parameter since unspecified
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    // Sender Certificate:
    if(status == STATUS_OK){
        if(toSign != FALSE && senderCertificate->Length>0){ // Field shall be null if message not signed
            status = SOPC_ByteString_Write(senderCertificate, msgBuffer);
        }else if(toSign == FALSE){
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            // status = Write_Int32(msgBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            const int32_t minusOne = -1;
            status = SOPC_Int32_Write(&minusOne, msgBuffer);
            // NULL string: nothing to write
        }else{
            status = STATUS_INVALID_PARAMETERS;
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){
        if(toEncrypt != FALSE && receiverCertCrypto != NULL){
            SOPC_ByteString recCertThumbprint;
            uint32_t thumbprintLength = 0;
            status = CryptoProvider_CertificateGetLength_Thumbprint(cryptoProvider, &thumbprintLength);

            if(STATUS_OK == status){
                if(thumbprintLength <= INT32_MAX){
                    status = SOPC_ByteString_InitializeFixedSize(&recCertThumbprint, (int32_t) thumbprintLength);
                }else{
                    status = STATUS_NOK;
                }
            }
            if(STATUS_OK == status){
                status = KeyManager_Certificate_GetThumbprint(cryptoProvider,
                                                              receiverCertCrypto,
                                                              recCertThumbprint.Data,
                                                              thumbprintLength);
            }

            if(STATUS_OK == status){
                status = SOPC_ByteString_Write(&recCertThumbprint, msgBuffer);
            }
            SOPC_ByteString_Clear(&recCertThumbprint);
        }else if(toEncrypt == FALSE){
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            //status = Write_Int32(msgBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            const int32_t minusOne = -1;
            status = SOPC_Int32_Write(&minusOne, msgBuffer);
            // NULL string: nothing to write
        }else{
            status = STATUS_INVALID_PARAMETERS;
        }
    }else{
        status = STATUS_NOK;
    }

    return status;
}

SOPC_StatusCode SC_EncodeAsymmSecurityHeader(SC_Connection*  scConnection,
                                             SOPC_MsgBuffer* msgBuffer,
                                             SOPC_String*    securityPolicy){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != NULL)
    {
        status = EncodeAsymmSecurityHeader(scConnection->currentCryptoProvider,
                                           msgBuffer,
                                           scConnection->currentSecuMode,
                                           securityPolicy,
                                           &scConnection->runningAppCertificate,
                                           scConnection->otherAppPublicKeyCert);
    }

    return status;
}

SOPC_StatusCode SC_EncodeMsgBody(SOPC_MsgBuffers*     msgBuffers,
                                 SOPC_EncodeableType* encType,
                                 void*                msgBody)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_NodeId nodeId;
    SOPC_NodeId_Initialize(&nodeId);

    if(msgBuffers != NULL && msgBody != NULL &&
       encType != NULL){
        nodeId.IdentifierType = IdentifierType_Numeric;
        if(encType->NamespaceUri == NULL){
            nodeId.Namespace = 0;
        }else{
            // TODO: find namespace Id
        }
        nodeId.Data.Numeric = encType->BinaryEncodingTypeId;

        status = SOPC_NodeId_Write(&nodeId, msgBuffers);
    }
    if(status == STATUS_OK){
        status = encType->Encode(msgBody, msgBuffers);
    }
    return status;
}

SOPC_StatusCode Set_Message_Length(SOPC_MsgBuffers* msgBuffers,
                                   uint32_t         nbChunk, // Number of chunk in which length must be set
                                   uint32_t         msgLength){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;
    uint32_t originNbChunk = 0;
    Buffer* buffer = NULL;
    if(msgBuffers != NULL && msgLength < msgBuffers->buffers->max_size){
        originPosition = msgBuffers->buffers->position;
        originNbChunk = msgBuffers->nbChunks;
        msgBuffers->nbChunks = nbChunk; // Mimic nbChunk is the current chunk to write into using SOPC_<Type>_Write functions
        buffer = MsgBuffers_GetCurrentChunk(msgBuffers);
        if(NULL != buffer && msgLength < buffer->max_size){
            status = Buffer_SetPosition(buffer, UA_HEADER_LENGTH_POSITION);
        }
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&msgLength, msgBuffers);
    }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(buffer, originPosition);
        msgBuffers->nbChunks = originNbChunk;
        msgBuffers->currentChunkSize = msgLength;
    }
    return status;
}

SOPC_StatusCode Set_Message_SymmetricSecuTokenId(SOPC_MsgBuffers* msgBuffers,
                                                 uint32_t         nbChunk, // Number of chunk in which length must be set
                                                 uint32_t         tokenId)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;
    uint32_t originNbChunk = 0;
    Buffer* buffer = NULL;
    if(msgBuffers != NULL){
        originPosition = msgBuffers->buffers->position;
        originNbChunk = msgBuffers->nbChunks;
        msgBuffers->nbChunks = nbChunk; // Mimic nbChunk is the current chunk to write into using SOPC_<Type>_Write functions
        buffer = MsgBuffers_GetCurrentChunk(msgBuffers);
        if(NULL != buffer){
            status = Buffer_SetPosition(buffer, UA_SECURE_MESSAGE_HEADER_LENGTH);
        }
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&tokenId, msgBuffers);
    }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(buffer, originPosition);
        msgBuffers->nbChunks = originNbChunk;
    }
    return status;
}

SOPC_StatusCode Set_Message_Chunk_Type(SOPC_MsgBuffers*   msgBuffers,
                                       uint32_t           nbChunk, // Number of chunk in which length must be set
                                       SOPC_MsgFinalChunk chunkType){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    Buffer* buffer = NULL;
    uint32_t originPosition = 0;

    if(msgBuffers != NULL){
        buffer = &msgBuffers->buffers[nbChunk-1];
        originPosition = buffer->position;
        status = Buffer_SetPosition(buffer, UA_HEADER_ISFINAL_POSITION);
    }

    SOPC_Byte chunkTypeByte;
    switch(chunkType){
        case SOPC_Msg_Chunk_Final:
            chunkTypeByte = 'F';
            break;
        case SOPC_Msg_Chunk_Intermediate:
            chunkTypeByte = 'C';
            break;
        case SOPC_Msg_Chunk_Abort:
            chunkTypeByte = 'A';
            break;
        default:
            status = STATUS_INVALID_PARAMETERS;
    }

    if(status == STATUS_OK){
        status = TCP_UA_WriteMsgBuffer(buffer, &chunkTypeByte, 1);
    }

    if(status == STATUS_OK){
        status = Buffer_SetPosition(buffer, originPosition);
    }

    return status;
}

SOPC_StatusCode Set_Sequence_NumberHeader(SOPC_MsgBuffers* msgBuffers,
                                          uint32_t         nbChunk, // Number of chunk in which length must be set
                                          uint32_t         requestId)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_Connection* scConnection = NULL;

    uint32_t originPosition = 0;
    uint32_t originNbChunk = 0;
    Buffer* buffer = NULL;
    if(msgBuffers != NULL && msgBuffers->flushData != NULL){
        scConnection = (SC_Connection*) msgBuffers->flushData;
        if(scConnection-> lastSeqNumSent > UINT32_MAX - 1024){ // Part 6 §6.7.2 v1.03
            scConnection->lastSeqNumSent = 1;
        }else{
            scConnection->lastSeqNumSent = scConnection-> lastSeqNumSent + 1;
        }

        originPosition = msgBuffers->buffers->position;
        originNbChunk = msgBuffers->nbChunks;

        msgBuffers->nbChunks = nbChunk; // Mimic nbChunk is the current chunk to write into using SOPC_<Type>_Write functions
        buffer = MsgBuffers_GetCurrentChunk(msgBuffers);

        if(NULL != buffer){
            // Sequence number position is the same in any chunk (same header for all chunks of same msg)
            status = Buffer_SetPosition(buffer, msgBuffers->sequenceNumberPosition);
        }
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&scConnection->lastSeqNumSent, msgBuffers);
    }
    if(status == STATUS_OK){
            status = SOPC_UInt32_Write(&requestId, msgBuffers);
        }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(buffer, originPosition);
        msgBuffers->nbChunks = originNbChunk;
    }
    return status;
}

SOPC_StatusCode EncodePadding(SC_Connection* scConnection,
                              SOPC_MsgBuffer* msgBuffer,
                              uint8_t       symmetricAlgo,
                              uint8_t*      hasPadding,
                              uint16_t*     realPaddingLength, // >= paddingSizeField
                              uint8_t*      hasExtraPadding,
                              uint32_t*     signatureSize)
{
    SOPC_StatusCode status = STATUS_OK;
    uint32_t plainBlockSize = 0;
    uint32_t cipherBlockSize = 0;
    *hasPadding = 1; // True

    if(symmetricAlgo == FALSE){
        if(scConnection->runningAppCertificate.Length <= 0 ||
           scConnection->runningAppPrivateKey == NULL ||
           scConnection->otherAppCertificate.Length <= 0){
           status = STATUS_INVALID_STATE;
        }else{
            AsymmetricKey* publicKey = NULL;

            status = KeyManager_AsymmetricKey_CreateFromCertificate(scConnection->otherAppPublicKeyCert,
                                                                    &publicKey);

            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricGetLength_Signature(scConnection->currentCryptoProvider,
                                                                      publicKey,
                                                                      signatureSize);
            }
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricGetLength_Msgs(scConnection->currentCryptoProvider,
                                                                 publicKey,
                                                                 &cipherBlockSize,
                                                                 &plainBlockSize);
            }

            KeyManager_AsymmetricKey_Free(publicKey);
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            status = STATUS_INVALID_STATE;
        }else{
            status = CryptoProvider_SymmetricGetLength_Signature(scConnection->currentCryptoProvider,
                                                                 signatureSize);
            if(status == STATUS_OK){
                status = CryptoProvider_SymmetricGetLength_Blocks(scConnection->currentCryptoProvider,
                                                                  &cipherBlockSize,
                                                                  &plainBlockSize);
            }
        }
    }

    if(status == STATUS_OK){
        uint8_t paddingSizeField = 0;
        *realPaddingLength = GetPaddingSize(msgBuffer->buffers->length -
                                              msgBuffer->sequenceNumberPosition,
                                            plainBlockSize,
                                            *signatureSize);
        //Little endian conversion of padding:
        SOPC_EncodeDecode_UInt16(realPaddingLength);
        status = Buffer_Write(msgBuffer->buffers, (SOPC_Byte*) realPaddingLength, 1);
        paddingSizeField = 0xFF & *realPaddingLength;

        if(status == STATUS_OK){
            // The value of each byte of the padding is equal to paddingSize:
            SOPC_Byte* paddingBytes = malloc(sizeof(SOPC_Byte)*(*realPaddingLength));
            if(paddingBytes != NULL){
                memset(paddingBytes, paddingSizeField, *realPaddingLength);
                status = Buffer_Write(msgBuffer->buffers, paddingBytes, *realPaddingLength);
                free(paddingBytes);
                paddingBytes = NULL;
            }else{
                status = STATUS_NOK;
            }
        }

        // Extra-padding necessary if padding could be greater 256 bytes
        if(status == STATUS_OK && Is_ExtraPaddingSizePresent(plainBlockSize) != FALSE){
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            SOPC_Byte extraPadding = 0x00FF & *realPaddingLength;
            Buffer_Write(msgBuffer->buffers, &extraPadding, 1);
        }
    }

    return status;
}

SOPC_StatusCode EncodeSignature(SC_Connection*  scConnection,
                                SOPC_MsgBuffer* msgBuffer,
                                uint8_t         symmetricAlgo,
                                uint32_t        signatureSize)
{
    SOPC_StatusCode status = STATUS_OK;
    SOPC_ByteString signedData;
    if(symmetricAlgo == FALSE){
        if(scConnection->runningAppCertificate.Length <= 0 ||
           scConnection->runningAppPrivateKey == NULL ||
           scConnection->otherAppCertificate.Length <= 0){
           status = STATUS_INVALID_STATE;
        }else{
            status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricSign(scConnection->currentCryptoProvider,
                                                       msgBuffer->buffers->data,
                                                       msgBuffer->buffers->length,
                                                       scConnection->runningAppPrivateKey,
                                                       signedData.Data,
                                                       signedData.Length);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers,
                                      signedData.Data,
                                      signedData.Length);
            }
            SOPC_ByteString_Clear(&signedData);
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            status = STATUS_INVALID_STATE;
        }else{
            status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
            if(status == STATUS_OK){
                status = CryptoProvider_SymmetricSign
                          (scConnection->currentCryptoProvider,
                           msgBuffer->buffers->data,
                           msgBuffer->buffers->length,
                           scConnection->currentSecuKeySets.senderKeySet->signKey,
                           signedData.Data,
                           signedData.Length);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers,
                                      signedData.Data,
                                      signedData.Length);
            }
            SOPC_ByteString_Clear(&signedData);
        }
    }
    return status;
}

SOPC_StatusCode EncryptMsg(SC_Connection* scConnection,
                      SOPC_MsgBuffer*     msgBuffer,
                      uint8_t             symmetricAlgo,
                      uint32_t            encryptedDataLength,
                      SOPC_MsgBuffer*     encryptedMsgBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_Byte* dataToEncrypt = &msgBuffer->buffers->data[msgBuffer->sequenceNumberPosition];
    const uint32_t dataToEncryptLength = msgBuffer->buffers->length - msgBuffer->sequenceNumberPosition;

    if(scConnection != NULL && msgBuffer != NULL &&
       encryptedMsgBuffer != NULL && encryptedMsgBuffer->buffers != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == FALSE){
        if(scConnection->runningAppCertificate.Length <= 0 ||
           scConnection->runningAppPrivateKey == NULL ||
           scConnection->otherAppCertificate.Length <= 0){
           status = STATUS_INVALID_STATE;
        }else{
            AsymmetricKey* otherAppPublicKey = NULL;
            SOPC_Byte* encryptedData = NULL;

            if(status == STATUS_OK){
                // Retrieve other app public key from certificate
                status = KeyManager_AsymmetricKey_CreateFromCertificate(scConnection->otherAppPublicKeyCert,
                                                                        &otherAppPublicKey);
            }

            // Check size of encrypted data array
            if(status == STATUS_OK)
            {
                if(encryptedMsgBuffer->buffers->max_size <
                    msgBuffer->sequenceNumberPosition + encryptedDataLength)
                {
                    status = STATUS_NOK;
                }
                encryptedData = encryptedMsgBuffer->buffers->data;
                if(encryptedData == NULL)
                {
                    status = STATUS_NOK;
                }else{
                    // Copy non encrypted headers part
                    memcpy(encryptedData, msgBuffer->buffers->data, msgBuffer->sequenceNumberPosition);
                    // Set correct message size and encrypted buffer length
                    Buffer_SetDataLength(encryptedMsgBuffer->buffers,
                                           msgBuffer->sequenceNumberPosition + encryptedDataLength);
                    encryptedMsgBuffer->currentChunkSize = msgBuffer->currentChunkSize;
                    // Message size was already encrypted message length, it must be the same now
                    assert(encryptedMsgBuffer->buffers->length == encryptedMsgBuffer->currentChunkSize);
                    // Ensure internal properties coherency (even if not used)
                    encryptedMsgBuffer->isFinal = msgBuffer->isFinal;
                    encryptedMsgBuffer->type = msgBuffer->type;
                    encryptedMsgBuffer->secureType = msgBuffer->secureType;
                    encryptedMsgBuffer->nbChunks = msgBuffer->nbChunks;
                }
            }

            // Encrypt
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricEncrypt
                          (scConnection->currentCryptoProvider,
                           dataToEncrypt,
                           dataToEncryptLength,
                           otherAppPublicKey,
                           &encryptedData[msgBuffer->sequenceNumberPosition],
                           encryptedDataLength);
            }

            KeyManager_AsymmetricKey_Free(otherAppPublicKey);

        } // End valid asymmetric encryption data
    }else if (status == STATUS_OK){
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            status = STATUS_INVALID_STATE;
        }else{
            SOPC_Byte* encryptedData = NULL;

            // Check size of encrypted data array
            if(status == STATUS_OK){
                if(encryptedMsgBuffer->buffers->max_size <
                    msgBuffer->sequenceNumberPosition + encryptedDataLength){
                    status = STATUS_NOK;
                }
                encryptedData = encryptedMsgBuffer->buffers->data;
                if(encryptedData == NULL){
                    status = STATUS_NOK;
                }else{
                    // Copy non encrypted headers part
                    memcpy(encryptedData, msgBuffer->buffers->data, msgBuffer->sequenceNumberPosition);
                    // Set correct message size and encrypted buffer length
                    Buffer_SetDataLength(encryptedMsgBuffer->buffers,
                                         msgBuffer->sequenceNumberPosition + encryptedDataLength);

                }
            }

            // Encrypt
            if(status == STATUS_OK){
                status = CryptoProvider_SymmetricEncrypt
                          (scConnection->currentCryptoProvider,
                           dataToEncrypt,
                           dataToEncryptLength,
                           scConnection->currentSecuKeySets.senderKeySet->encryptKey,
                           scConnection->currentSecuKeySets.senderKeySet->initVector,
                           &encryptedData[msgBuffer->sequenceNumberPosition],
                           encryptedDataLength);
            }
        } // End valid key set
    }
    return status;
}

void SOPC_OperationEnd_AbortMsg_CB(void*           arg,
                                   SOPC_StatusCode sendingStatus)
{
    assert(NULL != arg);
    (void) sendingStatus;
    SOPC_OperationEnd_FlushSecureMsgData* flushSecureMsgData = (SOPC_OperationEnd_FlushSecureMsgData*) arg;
    assert(NULL != arg);
    // In any case (success or failure of abort msg sending), call the end operation callback to indicate error that cause abort
    flushSecureMsgData->callback(flushSecureMsgData->callbackData,
                                 flushSecureMsgData->errorStatus);
    MsgBuffers_Delete(&flushSecureMsgData->msgBuffers);
}

// Always delete message buffers when abort sent or failed to be send
SOPC_StatusCode SC_AbortMsg(SOPC_MsgBuffers*                      msgBuffers,
                            uint32_t                              tokenId,
                            uint32_t                              requestId,
                            SOPC_StatusCode                       errorCode,
                            SOPC_String*                          reason,
                            SOPC_OperationEnd_FlushSecureMsgData* flushSecureMsgData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(msgBuffers != NULL){
        if(msgBuffers->nbChunks > 1 && reason != NULL){
            // At least one chunk has already been sent => abort message necessary
            // Set only one chunk for abort message
            msgBuffers->nbChunks = 1;
            // Set buffer position to body start (reuse header part already filled)
            status = Buffer_SetPosition(msgBuffers->buffers,
                                        (msgBuffers->sequenceNumberPosition +
                                         UA_SECURE_MESSAGE_SEQUENCE_LENGTH));
            if(STATUS_OK == status){
                status = SOPC_StatusCode_Write(&errorCode, msgBuffers);
            }
            if(STATUS_OK == status){
                status = SOPC_String_Write(reason, msgBuffers);
            }
            if(STATUS_OK == status){
                flushSecureMsgData->errorStatus = errorCode; // To call the end operation function with error code
                msgBuffers->isFinal = SOPC_Msg_Chunk_Abort;
                status = SC_FlushSecureMsgBuffers(msgBuffers,
                                                  0,
                                                  tokenId,
                                                  requestId,
                                                  SOPC_OperationEnd_AbortMsg_CB,
                                                  (void*) flushSecureMsgData);
            }
            if(STATUS_OK != status){
                MsgBuffers_Delete(&msgBuffers);
            }
        }else{
            // If no chunk sent, no need to send an abort message
            status = STATUS_OK;
            // Reset buffer for next sending
            MsgBuffers_Delete(&msgBuffers);
        }
    }
    return status;
}

void SOPC_Action_AbortMsg(void* arg){
    assert(NULL != arg);
    SOPC_String reason;
    SOPC_String_Initialize(&reason);
    // TODO: manage encoding chunk number and request Id ? in reason without using a VLA (for sprintf)
    char* genericReason = "Error encoding an intermediate chunk sending a message";
    if(genericReason != NULL){
        if(STATUS_OK != SOPC_String_AttachFromCstring(&reason, genericReason)){
            SOPC_String_Clear(&reason);
        }
    }

    SOPC_OperationEnd_FlushSecureMsgData* flushSecureMsgData = (SOPC_OperationEnd_FlushSecureMsgData*) arg;
    SC_AbortMsg(flushSecureMsgData->msgBuffers,
                flushSecureMsgData->tokenId,
                flushSecureMsgData->requestId,
                flushSecureMsgData->errorStatus,
                &reason,
                flushSecureMsgData);
}

void SOPC_OperationEnd_FlushSecureMsg_CB(void*           arg,
                                         SOPC_StatusCode sendingStatus);

void SOPC_Action_FlushSecureMsgNextChunk(void* arg){
    assert(NULL != arg);
    SOPC_OperationEnd_FlushSecureMsgData* data = (SOPC_OperationEnd_FlushSecureMsgData*) arg;
    SOPC_StatusCode status = STATUS_NOK;
    status = SC_FlushSecureMsgBuffers(data->msgBuffers,
                                      data->nextFlushChunkIdx,
                                      data->tokenId,
                                      data->requestId,
                                      data->callback,
                                      data->callbackData);
    if(STATUS_OK != status){
        // In case of failure, reuse the flush callback:
        //  only issue to do that is that we cannot differentiate sign/encrypt failure of socket write one
        SOPC_OperationEnd_FlushSecureMsg_CB(arg,
                                            status);
    }else{
        free(data);
    }
}

void SOPC_OperationEnd_FlushSecureMsg_CB(void*           arg,
                                         SOPC_StatusCode sendingStatus)
{
    assert(NULL != arg);
    SOPC_OperationEnd_FlushSecureMsgData* data = (SOPC_OperationEnd_FlushSecureMsgData*) arg;
    if(sendingStatus == STATUS_OK){
        if(data->nextFlushChunkIdx != 0){
            assert(data->nextFlushChunkIdx + 1 <= data->msgBuffers->nbChunks);
            // Creation of flush action for next chunk
            sendingStatus = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                              SOPC_Action_FlushSecureMsgNextChunk,
                                                              arg,
                                                              "Flush secure message for next chunk");
            // In case sending status is Bad here, we will not try to send an abort message
            // since it will use the same action queue manager which already failed
        }
    }else{
        SC_Connection* scConnection = (SC_Connection*) data->msgBuffers->flushData;
        if(NULL != scConnection){
            // Restore last sequence number sent
            scConnection->lastSeqNumSent = data->precSeqNum;
        }
        if(data->msgBuffers->nbChunks && data->nextFlushChunkIdx > 1){
            // A chunk was already sent, abort message is necessary
            sendingStatus = SOPC_ActionQueueManager_AddAction(stackActionQueueMgr,
                                                              SOPC_Action_AbortMsg,
                                                              arg,
                                                              "Abort chunk message due to failure when sending a chunk (> first chunk)");
        }
    }

    if(data->nextFlushChunkIdx == 0 ||
       STATUS_OK != sendingStatus){
        // Only when new action is not created or failed to be added:
        //  then we have to call end operation callback, free the msg buffers and callback data

        if(NULL != data->callback){
            data->callback(data->callbackData, sendingStatus);
        }
        MsgBuffers_Delete(&data->msgBuffers);


        free(data);
    }

}

// Caution: in case of failure returned, caller must call SC_AbortMsg to
// manage abort with multi-chunk + reset the message buffer
SOPC_StatusCode SC_FlushSecureMsgBuffers(SOPC_MsgBuffers*             msgBuffers,
                                         uint32_t                     flushChunkIdx,
                                         uint32_t                     tokenId,
                                         uint32_t                     requestId,
                                         SOPC_Socket_EndOperation_CB* callback,
                                         void*                        callbackData){
    SC_Connection* scConnection = NULL;
    SOPC_Socket_Transaction_Event transactionEvent = SOCKET_TRANSACTION_START; // For 1 chunk
    SOPC_MsgFinalChunk chunkType = SOPC_Msg_Chunk_Invalid;
    SOPC_OperationEnd_FlushSecureMsgData* data = NULL;
    SOPC_StatusCode status = STATUS_NOK;
    uint8_t toEncrypt = 1; // True
    uint8_t toSign = 1; // True
    uint8_t symmetricAlgo = 1; // True;
    uint8_t hasPadding = 0;
    uint16_t paddingLength = 0;
    uint8_t hasExtraPadding = 0;
    uint32_t signatureSize = 0;
    uint32_t encryptedLength = 0;
    uint32_t precSeqNum = 0;

    uint32_t nextFlushChunkIdx = 0;


    if(msgBuffers == NULL || msgBuffers->flushData == NULL || msgBuffers->nbChunks <= flushChunkIdx ||
       (msgBuffers->secureType != SOPC_SecureMessage && msgBuffers->nbChunks > 1)) // Multi chunk only possible in secure message
    {
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = STATUS_OK;

        // Determine chunk type and transaction event for socket layer
        if(msgBuffers->nbChunks > 1){
            // Abort message is only one chunk
            assert(chunkType != SOPC_Msg_Chunk_Abort);
            // Multi-chunk message
            if(flushChunkIdx + 1 == msgBuffers->nbChunks){
                // Last chunk
                chunkType = SOPC_Msg_Chunk_Final;
                transactionEvent = SOCKET_TRANSACTION_END;
                nextFlushChunkIdx = 0; // 0 means no next chunk since it is last one
            }else{
                if(flushChunkIdx == 0){
                    transactionEvent = SOCKET_TRANSACTION_START; // First chunk
                }
                // Intermediate
                chunkType = SOPC_Msg_Chunk_Intermediate;
                nextFlushChunkIdx = flushChunkIdx + 1;
            }
        }else{
            // One chunk only message
            if(chunkType != SOPC_Msg_Chunk_Abort){
                chunkType = SOPC_Msg_Chunk_Final;
            }
            transactionEvent = SOCKET_TRANSACTION_START_END;
            nextFlushChunkIdx = 0; // 0 means no next chunk
        }

        scConnection = (SC_Connection*) msgBuffers->flushData;
        precSeqNum = scConnection->lastSeqNumSent;

        toEncrypt = IsMsgEncrypted(scConnection->currentSecuMode, msgBuffers);

        toSign = IsMsgSigned(scConnection->currentSecuMode);

        // Determine if the asymmetric algorithms must be used
        if(msgBuffers->secureType == SOPC_OpenSecureChannel){
            symmetricAlgo = FALSE;
        }

        //// Encode padding if encrypted message
        if(toEncrypt == FALSE){
            // No padding fields
        }else{
            status = EncodePadding(scConnection,
                                   msgBuffers,
                                   symmetricAlgo,
                                   &hasPadding,
                                   &paddingLength,
                                   &hasExtraPadding,
                                   &signatureSize);
        }

        //// Set the chunk type for the given message
        if(status == STATUS_OK){
            status = Set_Message_Chunk_Type(msgBuffers,
                                            flushChunkIdx+1,
                                            chunkType);
        }

        //// Encode message length:
        // Compute final encrypted message length:
        if(status == STATUS_OK){
            const uint32_t plainDataToEncryptLength = // Already encoded message from encryption start + signature size
             msgBuffers->buffers->length - msgBuffers->sequenceNumberPosition + signatureSize;
            if(toEncrypt == FALSE){
                // No encryption same data length
                encryptedLength = plainDataToEncryptLength;
            }else{
                status = GetEncryptedDataLength(scConnection, plainDataToEncryptLength,
                                                symmetricAlgo, &encryptedLength);
            }
        }
        // Set final message length
        if(status == STATUS_OK){
            status = Set_Message_Length(msgBuffers,
                                        flushChunkIdx+1,
                                        msgBuffers->sequenceNumberPosition + encryptedLength);
        }

        if(status == STATUS_OK && msgBuffers->secureType == SOPC_SecureMessage){
            // Only if it is a secure MSG (=> not OPN or CLO)
            status = Set_Message_SymmetricSecuTokenId(msgBuffers,
                                                      flushChunkIdx+1,
                                                      tokenId);
        }

        //// Encode sequence number
        if(status == STATUS_OK){
            status = Set_Sequence_NumberHeader(msgBuffers,
                                               flushChunkIdx+1,
                                               requestId);
        }

        //// Encode signature if message signed
        if(toSign == FALSE){
            // No signature field
        }else if(status == STATUS_OK){
            status = EncodeSignature(scConnection,
                                     msgBuffers,
                                     symmetricAlgo,
                                     signatureSize);
        }

        //// Check sender certificate size is not bigger than maximum size to be sent
        if(status == STATUS_OK &&
           msgBuffers->secureType == SOPC_OpenSecureChannel &&
           toSign != FALSE)
        {
            status = CheckMaxSenderCertificateSize(&scConnection->runningAppCertificate,
                                                   msgBuffers->buffers->max_size,
                                                   &scConnection->currentSecuPolicy,
                                                   hasPadding,
                                                   paddingLength,
                                                   hasExtraPadding,
                                                   signatureSize);
        }

        if(status == STATUS_OK){
            if(toEncrypt == FALSE){
                // No encryption necessary but we need to attach buffer as transport buffer (done during encryption otherwise)
                status = MsgBuffers_CopyBufferIdx(scConnection->transportConnection->outputMsgBuffer,
                                                  msgBuffers,
                                                  flushChunkIdx);
            }else{
                // TODO: use detach / attach to control references on the transport msg buffer ?
                status = EncryptMsg(scConnection,
                                    msgBuffers,
                                    symmetricAlgo,
                                    encryptedLength,
                                    scConnection->transportConnection->outputMsgBuffer);
            }
        }

        if(status == STATUS_OK){
            data = malloc(sizeof(SOPC_OperationEnd_FlushSecureMsgData));
            if(NULL != data){
                data->msgBuffers = msgBuffers;
                data->nextFlushChunkIdx = nextFlushChunkIdx;
                data->tokenId = tokenId;
                data->requestId = requestId;
                data->precSeqNum = precSeqNum;
                data->callback = callback;
                data->callbackData = callbackData;
                data->errorStatus = STATUS_OK;

                status = TCP_UA_FlushMsgBuffer(scConnection->transportConnection->outputMsgBuffer,
                                               transactionEvent,
                                               requestId,
                                               SOPC_OperationEnd_FlushSecureMsg_CB,
                                               (void*) data);

                if(STATUS_OK != status){
                    free(data);
                }
            }else{
                status = STATUS_NOK;
            }
        }
    }
    return status;
}

SOPC_StatusCode SC_DecodeSecureMsgSCid(SC_Connection*  scConnection,
                                       SOPC_MsgBuffer* transportBuffer)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t secureChannelId = 0;
    if(scConnection != NULL &&
       transportBuffer != NULL)
    {
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        //TODO: on server side, randomize secure channel ids (table 26 part 6)!
        status = SOPC_UInt32_Read(&secureChannelId, transportBuffer);
    }

    if(status == STATUS_OK){
        if(scConnection->secureChannelId != secureChannelId){
            // Different Id assigned by server: invalid case (id never changes on same connection instance)
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    return status;
}

SOPC_StatusCode SC_DecodeAsymSecurityHeader_SecurityPolicy(SC_Connection*  scConnection,
                                                           SOPC_MsgBuffer* transportBuffer,
                                                           SOPC_String*    securityPolicy)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != NULL &&
       transportBuffer != NULL &&
       securityPolicy != NULL)
        {
            status = SOPC_String_Read(securityPolicy, transportBuffer);
        }
        return status;
}

SOPC_StatusCode SC_DecodeAsymSecurityHeader_Certificates(SC_Connection*     scConnection,
                                                         SOPC_MsgBuffer*    transportBuffer,
                                                         const PKIProvider* pkiProvider,
                                                         uint32_t           validateSenderCert,
                                                         uint8_t            enforceSecuMode,
                                                         uint32_t*          sequenceNumberPosition,
                                                         uint8_t*           senderCertificatePresence,
                                                         uint8_t*           receiverCertificatePresense)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True
    SOPC_ByteString senderCertificate;
    SOPC_ByteString_Initialize(&senderCertificate);
    SOPC_ByteString receiverCertThumb;
    SOPC_ByteString_Initialize(&receiverCertThumb);
    if(scConnection != NULL &&
       transportBuffer != NULL &&
       sequenceNumberPosition != NULL &&
       senderCertificatePresence != NULL &&
       receiverCertificatePresense != NULL)
    {
        if(enforceSecuMode != FALSE){
            // Asymmetric security header must use current security parameters
            // (TODO: add guarantee we are treating last OPN sent: using pending requests ?)
            toEncrypt = IsMsgEncrypted(scConnection->currentSecuMode,
                                       transportBuffer);
            toSign = IsMsgSigned(scConnection->currentSecuMode);
        }
        status = STATUS_OK;
    }

    // Sender Certificate:
    if(status == STATUS_OK){
        status = SOPC_ByteString_Read(&senderCertificate, transportBuffer);
        if(status == STATUS_OK){
            if (toSign == FALSE && senderCertificate.Length > 0){
                // Table 27 part 6: "field shall be null if the Message is not signed"
                status = STATUS_INVALID_RCV_PARAMETER;
                *senderCertificatePresence = 1; // TRUE
            }else if(toSign != FALSE && senderCertificate.Length > 0){
                *senderCertificatePresence = 1; // TRUE
                if(scConnection->transportConnection->serverSideConnection == FALSE){
                    // Check certificate is the same as the one in memory (CLIENT SIDE ONLY)
                    int32_t otherAppCertComparison = 0;
                    status = SOPC_ByteString_Compare(&scConnection->otherAppCertificate,
                                                     &senderCertificate,
                                                     &otherAppCertComparison);

                    if(status != STATUS_OK || otherAppCertComparison != 0){
                        status = STATUS_INVALID_RCV_PARAMETER;
                    }
                }

                if(status == STATUS_OK && validateSenderCert != FALSE){
                    Certificate *cert = NULL;
                    status = KeyManager_Certificate_CreateFromDER(senderCertificate.Data, senderCertificate.Length,
                                                                  &cert);
                    if(status == STATUS_OK){
                        status = CryptoProvider_Certificate_Validate(scConnection->currentCryptoProvider,
                                                                     pkiProvider,
                                                                     cert);
                    }

                    if(scConnection->transportConnection->serverSideConnection == FALSE){
                        if(NULL != cert)
                            KeyManager_Certificate_Free(cert);
                    }else{
                        if(status == STATUS_OK){
                            // Set as valid other application certificate (SERVER SIDE ONLY)
                            status = SC_InitOtherAppIdentity(scConnection,
                                                             cert,
                                                             &senderCertificate);
                        }else{
                            // Error case (SERVER SIDE ONLY)
                            if(NULL != cert)
                                KeyManager_Certificate_Free(cert);
                        }
                    }
                }
            }else if(enforceSecuMode == FALSE || toSign == FALSE){
                // Without security mode to enforce, absence could be normal
                *senderCertificatePresence = FALSE;
            }else{
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){
        status = SOPC_ByteString_Read(&receiverCertThumb, transportBuffer);

        if(status == STATUS_OK){
            if(toEncrypt == FALSE && receiverCertThumb.Length > 0){
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                status =STATUS_INVALID_RCV_PARAMETER;
                *receiverCertificatePresense = 1; // TRUE
            }else if(toEncrypt != FALSE && receiverCertThumb.Length > 0){
                // Check thumbprint matches current app certificate thumbprint
                *receiverCertificatePresense = 1; // TRUE
                SOPC_ByteString curAppCertThumbprint;
                uint32_t thumbprintLength = 0;
                int32_t runningAppCertComparison = 0;

                status = CryptoProvider_CertificateGetLength_Thumbprint(scConnection->currentCryptoProvider,
                                                                        &thumbprintLength);

                if(STATUS_OK == status && thumbprintLength > INT32_MAX){
                    status = STATUS_NOK;
                }
                if(STATUS_OK == status){
                    if((int32_t) thumbprintLength == receiverCertThumb.Length){
                        status = SOPC_ByteString_InitializeFixedSize(&curAppCertThumbprint, (int32_t) thumbprintLength);
                        if(status == STATUS_OK){
                            status = KeyManager_Certificate_GetThumbprint(scConnection->currentCryptoProvider,
                                                                          scConnection->runningAppPublicKeyCert,
                                                                          curAppCertThumbprint.Data,
                                                                          thumbprintLength);

                            if(status == STATUS_OK){
                                status = SOPC_ByteString_Compare(&curAppCertThumbprint,
                                                            &receiverCertThumb,
                                                            &runningAppCertComparison);

                                if(status != STATUS_OK || runningAppCertComparison != 0){
                                    status = STATUS_INVALID_RCV_PARAMETER; // TODO: BadCertificateUnknown error ? part 6 6.7.6 p40
                                }
                            }

                        }else{
                            status = STATUS_NOK;
                        }
                    }else{
                        status = STATUS_INVALID_RCV_PARAMETER; // TODO: BadCertificateUnknown error ? part 6 6.7.6 p40
                    }
                } // if thumbprint length correctly computed

                SOPC_ByteString_Clear(&curAppCertThumbprint);

            }else if(enforceSecuMode == FALSE || toEncrypt == FALSE){ // if toEncrypt
                // Without security mode to enforce, absence could be normal
                *receiverCertificatePresense = FALSE;
            }else{
                status = STATUS_INVALID_RCV_PARAMETER;
            }

            // Set the sequence number position which is the next position to read
            //  since whole asymmetric security header was read
            *sequenceNumberPosition = transportBuffer->buffers->position;
        } // if decoded thumbprint
    }

    SOPC_ByteString_Clear(&senderCertificate);
    SOPC_ByteString_Clear(&receiverCertThumb);

    return status;
}

SOPC_StatusCode SC_DecodeAsymmSecurityHeader(SC_Connection*     scConnection,
                                             const PKIProvider* pkiProvider,
                                             SOPC_MsgBuffer*    transportBuffer,
                                             uint32_t           validateSenderCert,
                                             uint32_t*          sequenceNumberPosition)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t senderCertificatePresence, receiverCertificatePresence;

    SOPC_String securityPolicy; // It's a byte string but it's same UA binary representation (use String for String_Compare)
    SOPC_String_Initialize(&securityPolicy);

    status = SC_DecodeAsymSecurityHeader_SecurityPolicy(scConnection, transportBuffer, &securityPolicy);

    // Security Policy:
    if(status == STATUS_OK){
        int32_t secuPolicyComparison = 0;
        status = SOPC_String_Compare(&scConnection->currentSecuPolicy,
                                     &securityPolicy,
                                     1,
                                     &secuPolicyComparison);

        if(status != STATUS_OK || secuPolicyComparison != 0){
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }

    // Sender and Receiver Certificate:
    if(status == STATUS_OK){
        status = SC_DecodeAsymSecurityHeader_Certificates(scConnection, transportBuffer, pkiProvider,
                                                          validateSenderCert,
                                                          1, // enforceSecuMode == TRUE
                                                          sequenceNumberPosition,
                                                          &senderCertificatePresence,
                                                          &receiverCertificatePresence);
    }

    SOPC_String_Clear(&securityPolicy);

    return status;
}

SOPC_StatusCode SC_IsPrecedentCryptoData(SC_Connection* scConnection,
                                         uint32_t       receivedTokenId,
                                         uint32_t*      isPrecCryptoData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != NULL){
        status = STATUS_OK;
        if(scConnection->currentSecuToken.tokenId == receivedTokenId){
            *isPrecCryptoData = FALSE;
        }else if(scConnection->precSecuToken.tokenId == receivedTokenId){
           *isPrecCryptoData = 1; // TRUE
            // TODO: check validity timeout of prec crypto data
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }
    return status;
}

SOPC_StatusCode SC_DecryptMsg(SC_Connection*  scConnection,
                              SOPC_MsgBuffer* transportBuffer,
                              uint32_t        sequenceNumberPosition,
                              uint32_t        isSymmetric,
                              uint32_t        isPrecCryptoData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toDecrypt = 1;
    uint32_t decryptedTextLength = 0;
    CryptoProvider* cryptoProvider = NULL;
    OpcUa_MessageSecurityMode securityMode = OpcUa_MessageSecurityMode_Invalid;
    Buffer* plainBuffer = NULL;
    uint32_t bufferIdx = 0;

    if(scConnection != NULL && transportBuffer != NULL){
        status = STATUS_OK;

        if(isPrecCryptoData == FALSE){
            cryptoProvider = scConnection->currentCryptoProvider;
            securityMode = scConnection->currentSecuMode;
        }else{
            cryptoProvider = scConnection->precCryptoProvider;
            securityMode = scConnection->precSecuMode;
            isPrecCryptoData = 1; // TRUE
            // TODO: check validity timeout of prec crypto data
        }

        toDecrypt = IsMsgEncrypted(securityMode, transportBuffer);
    }

    // TODO: factorize common code between length retrieved and decrypt ...
    if(toDecrypt != FALSE){
        // Message is encrypted
        SOPC_Byte* dataToDecrypt = &(transportBuffer->buffers->data[sequenceNumberPosition]);
        uint32_t lengthToDecrypt = transportBuffer->buffers->length - sequenceNumberPosition;

        if(status == STATUS_OK){
            if(isSymmetric == FALSE){
                status = CryptoProvider_AsymmetricGetLength_Decryption(cryptoProvider,
                                                                       scConnection->runningAppPrivateKey,
                                                                       lengthToDecrypt,
                                                                       &decryptedTextLength);

                if(status == STATUS_OK && decryptedTextLength <= scConnection->receptionBuffers->buffers->max_size){
                    // Retrieve next chunk empty buffer
                    plainBuffer = MsgBuffers_NextChunk(scConnection->receptionBuffers, &bufferIdx);
                    if(plainBuffer != NULL){
                        // Copy non encrypted data from original buffer to plain text buffer
                        // and set position to current read position
                        status = MsgBuffers_CopyBuffer(scConnection->receptionBuffers,
                                                       bufferIdx,
                                                       transportBuffer,
                                                       sequenceNumberPosition);
                    }
                    if(status == STATUS_OK){
                        assert(plainBuffer->max_size >= decryptedTextLength);
                        status = CryptoProvider_AsymmetricDecrypt(cryptoProvider,
                                                                  dataToDecrypt,
                                                                  lengthToDecrypt,
                                                                  scConnection->runningAppPrivateKey,
                                                                  &(plainBuffer->data[sequenceNumberPosition]),
                                                                  decryptedTextLength, // TODO integration: pas très beau d'avoir la variable et son pointeur juste après (devrait marcher ceci dit, mais ne permet pas de vérifier qu'on s'est pas planté)
                                                                  &decryptedTextLength);
                        if(status == STATUS_OK){
                            Buffer_SetDataLength(plainBuffer, sequenceNumberPosition + decryptedTextLength);
                        }
                    }else{
                        status = STATUS_NOK;
                    }
                }else if(status == STATUS_OK){
                    status = STATUS_INVALID_STATE;
                }
            }else{

                SC_SecurityKeySet* receiverKeySet = NULL;
                if(isPrecCryptoData == FALSE){
                    receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
                }else{
                    receiverKeySet = scConnection->precSecuKeySets.receiverKeySet;
                }
                status = CryptoProvider_SymmetricGetLength_Decryption(cryptoProvider,
                                                                      lengthToDecrypt,
                                                                      &decryptedTextLength);

                if(status == STATUS_OK && decryptedTextLength <= scConnection->receptionBuffers->buffers->max_size){
                    // Retrieve next chunk empty buffer
                    plainBuffer = MsgBuffers_NextChunk(scConnection->receptionBuffers, &bufferIdx);

                    if(plainBuffer != NULL){
                        // Copy non encrypted data from original buffer to plain text buffer
                        // and set position to current read position
                        status = MsgBuffers_CopyBuffer(scConnection->receptionBuffers,
                                                       bufferIdx,
                                                       transportBuffer,
                                                       sequenceNumberPosition);
                    }
                    if(status == STATUS_OK){
                        assert(plainBuffer->max_size >= decryptedTextLength);
                        status = CryptoProvider_SymmetricDecrypt(cryptoProvider,
                                                                 dataToDecrypt,
                                                                 lengthToDecrypt,
                                                                 receiverKeySet->encryptKey,
                                                                 receiverKeySet->initVector,
                                                                 &(plainBuffer->data[sequenceNumberPosition]),
                                                                 decryptedTextLength);
                        if(status == STATUS_OK){
                            Buffer_SetDataLength(plainBuffer, sequenceNumberPosition + decryptedTextLength);
                        }
                    }else{
                        status = STATUS_NOK;
                    }
                }else if(status == STATUS_OK){
                    status = STATUS_INVALID_STATE;
                }
            } // Symmetric algo branch
        }
    }else{
        // Message is not encrypted
        plainBuffer = MsgBuffers_NextChunk(scConnection->receptionBuffers, &bufferIdx);
        assert(transportBuffer->nbChunks == scConnection->receptionBuffers->nbChunks);
        status = MsgBuffers_CopyBuffer(scConnection->receptionBuffers,
                                       bufferIdx,
                                       transportBuffer,
                                       transportBuffer->buffers->length);
    }

    return status;
}

SOPC_StatusCode SC_DecodeMsgBody(SOPC_MsgBuffer*       receptionBuffer,
                                 SOPC_NamespaceTable*  namespaceTable,
                                 SOPC_EncodeableType** knownTypes,
                                 SOPC_EncodeableType*  recvEncType,
                                 SOPC_EncodeableType*  errEncType,
                                 SOPC_EncodeableType** receivedEncType,
                                 void**                encodeableObj)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_EncodeableType* recEncType = NULL;
    SOPC_NodeId nodeId;
    const char* nsName;
    uint16_t nsIndex = 0;
    SOPC_NodeId_Initialize(&nodeId);
    if(receptionBuffer != NULL && namespaceTable != NULL && encodeableObj != NULL &&
       (knownTypes != NULL || recvEncType != NULL))
    {
        status = SOPC_NodeId_Read(&nodeId, receptionBuffer);
    }

    if(status == STATUS_OK && nodeId.IdentifierType == OpcUa_IdType_Numeric){

        if(recvEncType != NULL){
            // Case in which we know the expected type from the request Id
            if (nodeId.Data.Numeric == recvEncType->TypeId || nodeId.Data.Numeric == recvEncType->BinaryEncodingTypeId){
    //          || nodeId.data.numeric == recvEncType->xmlTypeId){ => what is the point to accept this type ?
                *receivedEncType = recvEncType;
            }else if(errEncType != NULL &&
                     (nodeId.Data.Numeric == errEncType->TypeId || nodeId.Data.Numeric == errEncType->BinaryEncodingTypeId)){
    //               || nodeId.data.numeric == errEncType->xmlTypeId){ => what is the point to accept this type ?
                *receivedEncType = errEncType;
            }else{
                status = STATUS_INVALID_RCV_PARAMETER;
            }

            // Check namespace of received type using index
            if(status == STATUS_OK){
                recEncType = *receivedEncType;
                if(recEncType->NamespaceUri == NULL && nodeId.Namespace != OPCUA_NAMESPACE_INDEX){
                    status = STATUS_INVALID_RCV_PARAMETER;
                }else if(recEncType->NamespaceUri != NULL){
                    status = Namespace_GetIndex(namespaceTable,
                                                recEncType->NamespaceUri,
                                                &nsIndex);
                    if(status == STATUS_OK){
                        if(nodeId.Namespace != nsIndex){
                            status = STATUS_INVALID_RCV_PARAMETER;
                        }
                    }
                }
            }

        }else{
            // Must be the case in which we cannot know the type before decoding it
            if(nodeId.Namespace == OPCUA_NAMESPACE_INDEX){
                recEncType = SOPC_EncodeableType_GetEncodeableType(knownTypes, OPCUA_NAMESPACE_NAME, nodeId.Data.Numeric);
            }else{
                nsName = Namespace_GetName(namespaceTable, nodeId.Namespace);
                if(nsName != NULL){
                    recEncType = SOPC_EncodeableType_GetEncodeableType(knownTypes, nsName, nodeId.Data.Numeric);
                }
                if(recEncType == NULL){
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            }
            *receivedEncType = recEncType;
        }


    }else if(status == STATUS_OK){
        status = STATUS_INVALID_RCV_PARAMETER;
    }

    if(status == STATUS_OK){
        *encodeableObj = malloc(recEncType->AllocationSize);
        if(*encodeableObj != NULL){
            status = recEncType->Decode(*encodeableObj, receptionBuffer);
        }else{
            status = STATUS_NOK;
        }
    }
    SOPC_NodeId_Clear(&nodeId);
    return status;
}

SOPC_StatusCode SC_VerifyMsgSignature(SC_Connection* scConnection,
                                      uint32_t       isSymmetric,
                                      uint32_t       isPrecCryptoData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toVerify = 1;

    CryptoProvider* cryptoProvider = NULL;
    OpcUa_MessageSecurityMode securityMode = OpcUa_MessageSecurityMode_Invalid;
    Buffer* receptionBuffer = MsgBuffers_GetCurrentChunk(scConnection->receptionBuffers);

    uint32_t signatureSize = 0;
    uint32_t signaturePosition = 0;

    if(scConnection != NULL){
        status = STATUS_OK;

        if(isPrecCryptoData == FALSE){
            cryptoProvider = scConnection->currentCryptoProvider;
            securityMode = scConnection->currentSecuMode;
        }else{
            cryptoProvider = scConnection->precCryptoProvider;
            securityMode = scConnection->precSecuMode;
        }

        toVerify = IsMsgSigned(securityMode);
    }

    if(toVerify != FALSE){
        if(isSymmetric == FALSE){
            AsymmetricKey* publicKey = NULL;


            status = KeyManager_AsymmetricKey_CreateFromCertificate(scConnection->otherAppPublicKeyCert,
                                                                    &publicKey);

            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricGetLength_Signature(scConnection->currentCryptoProvider,
                                                                    publicKey,
                                                                    &signatureSize);
            }

            if(status == STATUS_OK){
                signaturePosition = receptionBuffer->length - signatureSize;

                status = CryptoProvider_AsymmetricVerify(cryptoProvider,
                                                         receptionBuffer->data,
                                                         signaturePosition,
                                                         publicKey,
                                                         &(receptionBuffer->data[signaturePosition]),
                                                         signatureSize);
            }

            KeyManager_AsymmetricKey_Free(publicKey);
        }else{
            SC_SecurityKeySet* receiverKeySet = NULL;
            if(isPrecCryptoData == FALSE){
                receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
            }else{
                receiverKeySet = scConnection->precSecuKeySets.receiverKeySet;
            }

            status = CryptoProvider_SymmetricGetLength_Signature(cryptoProvider,
                                                                 &signatureSize);

            if(status == STATUS_OK){
                signaturePosition = receptionBuffer->length - signatureSize;
                status = CryptoProvider_SymmetricVerify(cryptoProvider,
                                                        receptionBuffer->data,
                                                        signaturePosition,
                                                        receiverKeySet->signKey,
                                                        &(receptionBuffer->data[signaturePosition]),
                                                        signatureSize);
            }
        }
    }

    return status;
}

SOPC_StatusCode SC_CheckSeqNumReceived(SC_Connection* scConnection)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t seqNumber = 0;

    if(scConnection != NULL){
        status = STATUS_OK;
        status = SOPC_UInt32_Read(&seqNumber, scConnection->receptionBuffers);
    }

    if(status == STATUS_OK){
        if(scConnection->receptionBuffers->secureType == SOPC_OpenSecureChannel){
            scConnection->lastSeqNumReceived = seqNumber;
        }else{
            if(scConnection->lastSeqNumReceived + 1 != seqNumber){
                // Part 6 §6.7.2 v1.03
                if(scConnection->lastSeqNumReceived > UINT32_MAX - 1024 &&
                   seqNumber < 1024){
                    scConnection->lastSeqNumReceived = seqNumber;
                }else{
                    status = STATUS_NOK;
                }
            }else{
                // Correct sequence number
                scConnection->lastSeqNumReceived++;
            }
        }
    }

    return status;
}

SOPC_StatusCode SC_CheckReceivedProtocolVersion(SC_Connection* scConnection,
                                                uint32_t       scProtocolVersion)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t transportProtocolVersion = 0;

    if(scConnection != NULL){
        status = STATUS_OK;
        // use Get_Rcv_Protocol_Version and check it is the same as the one received in SC
        if(TCP_UA_Connection_GetReceiveProtocolVersion(scConnection->transportConnection,
                                                       &transportProtocolVersion)
           != FALSE)
        {
            if(scProtocolVersion != transportProtocolVersion){
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }
    return status;
}

SOPC_StatusCode SC_EncodeSecureMessage(SOPC_MsgBuffers*     msgBuffers,
                                       SOPC_EncodeableType* encType,
                                       void*                value,
                                       uint32_t             secureChannelId,
                                       uint32_t             tokenId,
                                       uint32_t             requestId)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(msgBuffers != NULL &&
       encType != NULL &&
       value != NULL)
    {
        // Encode secure message header:
        status = SC_EncodeSecureMsgHeader(msgBuffers,
                                          SOPC_SecureMessage,
                                          secureChannelId);
    }

    if(STATUS_OK == status){
        // Encode symmetric security header
        status = SOPC_UInt32_Write(&tokenId, msgBuffers);
    }

    if(STATUS_OK == status){
        status = SC_EncodeSequenceHeader(msgBuffers, requestId);
    }

    if(STATUS_OK == status){
        status = SC_EncodeMsgBody(msgBuffers, encType, value);
    }

    return status;
}

SOPC_StatusCode SC_DecodeSymmSecurityHeader(SOPC_MsgBuffer* transportBuffer,
                                            uint32_t*      tokenId,
                                            uint32_t*      snPosition)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(transportBuffer != NULL)
    {
        status = SOPC_UInt32_Read(tokenId, transportBuffer);
    }
    if(status == STATUS_OK){
        *snPosition = transportBuffer->buffers->position;
    }
    return status;
}

SOPC_StatusCode SC_RemovePaddingAndSig(SC_Connection* scConnection,
                                       uint32_t       isPrecCryptoData)
{
    // Only valid for symmetric encryption ! No need for asymm (1 chunk maximum)
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    CryptoProvider* cProvider = NULL;
    OpcUa_MessageSecurityMode securityMode = OpcUa_MessageSecurityMode_Invalid;
    uint32_t sigSize = 0;
    uint32_t cipherBlockSize = 0;
    uint32_t plainBlockSize = 0;
    uint16_t padding = 0;
    Buffer* curChunk = MsgBuffers_GetCurrentChunk(scConnection->receptionBuffers);
    uint32_t newBufferLength = curChunk->length;
    uint8_t isEncrypted = 1;
    uint8_t isSigned = 1;

    if(scConnection != NULL){
        status = STATUS_OK;
        if(isPrecCryptoData == FALSE){
            cProvider = scConnection->currentCryptoProvider;
            securityMode = scConnection->currentSecuMode;
        }else{
            cProvider = scConnection->precCryptoProvider;
            securityMode = scConnection->precSecuMode;
        }

        isEncrypted = IsMsgEncrypted(securityMode, scConnection->receptionBuffers);
        isSigned = IsMsgSigned(securityMode);
    }

    if(status == STATUS_OK){

        if(isSigned == FALSE){
            sigSize = 0;
        }else{
            // Compute signature size and remove from buffer length
            status = CryptoProvider_SymmetricGetLength_Signature(cProvider,
                                                                 &sigSize);
        }

        if(status == STATUS_OK){
            newBufferLength = newBufferLength - sigSize;
            if(isEncrypted != FALSE){
                status = CryptoProvider_SymmetricGetLength_Blocks(cProvider,
                                                                  &cipherBlockSize,
                                                                  &plainBlockSize);
            }
        }
    }

    if(status == STATUS_OK){
        // No padding to handle if there is no encryption
        if(isEncrypted != FALSE){
            // Extra padding management: possible if block size greater than:
            // UINT8_MAX (1 byte representation max)+ 1 (padding size field byte))
            if(Is_ExtraPaddingSizePresent(plainBlockSize) != FALSE){
                // compute most significant byte value
                padding = curChunk->data[newBufferLength-1] << 8;
                // remove extra padding size field
                newBufferLength -= 1;
            }

            // Pading bytes are containing the (low byte) of padding length (<=> paddingSize value)
            padding += curChunk->data[newBufferLength-1];
            // Remove padding bytes (including extra if existing)
            newBufferLength -= padding;
            // Remove padding field
            newBufferLength -= 1;
        }
        // Set new buffer length without padding and signature
        status = Buffer_SetDataLength(curChunk, newBufferLength);
    }


    return status;
}

SOPC_StatusCode SC_DecryptSecureMessage(SC_Connection*  scConnection,
                                        SOPC_MsgBuffer* transportMsgBuffer,
                                        uint32_t*       requestId)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    const uint32_t isSymmetricTrue = 1; // TRUE
    uint32_t isPrecCryptoDataFalse = FALSE;
    uint32_t tokenId = 0;
    uint32_t snPosition = 0;

    // Message header already managed by transport layer
    // (except secure channel Id)
    if(scConnection != NULL){
        status = SC_DecodeSecureMsgSCid(scConnection,
                                        transportMsgBuffer);
    }

    if(status == STATUS_OK){
        // Check security policy
        // Validate other app certificate
        // Check current app certificate thumbprint
        status = SC_DecodeSymmSecurityHeader(transportMsgBuffer,
                                             &tokenId,
                                             &snPosition);
    }

    if(status == STATUS_OK){
        // Determine keyset to use regarding token id provided
        status = SC_IsPrecedentCryptoData(scConnection,
                                          tokenId,
                                          &isPrecCryptoDataFalse);
    }

    if(status == STATUS_OK){
        // Decrypt message content and store complete message in secure connection buffer
        status = SC_DecryptMsg(scConnection,
                               transportMsgBuffer,
                               snPosition,
                               isSymmetricTrue,
                               isPrecCryptoDataFalse);
    }

    if(status == STATUS_OK){
        // Check decrypted message signature
        status = SC_VerifyMsgSignature(scConnection,
                                       isSymmetricTrue,
                                       isPrecCryptoDataFalse);
    }

    if(status == STATUS_OK){
        status = SC_CheckSeqNumReceived(scConnection);
    }

    if(status == STATUS_OK){
        // Retrieve request id
        status = SOPC_UInt32_Read(requestId, scConnection->receptionBuffers);
    }

    if(status == STATUS_OK){
        status = SC_RemovePaddingAndSig(scConnection,
                                        isPrecCryptoDataFalse);
    }

    return status;
}

SOPC_StatusCode SC_CheckPrecChunk(SOPC_MsgBuffers* msgBuffer,
                                  uint32_t         requestId,
                                  uint8_t*         abortReqPresence,
                                  uint32_t*        abortReqId)
{
    assert(msgBuffer != NULL);
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(msgBuffer != NULL &&
       abortReqPresence != NULL && abortReqId != NULL)
    {
        // Check if we received a new request id related message before
        //  precedent treatment end
        if(msgBuffer->nbChunks > 1 &&
           msgBuffer->msgRequestId != requestId)
        {
            *abortReqPresence = 1;
            *abortReqId = msgBuffer->msgRequestId;
            status = MsgBuffers_SetCurrentChunkFirst(msgBuffer);
        }else{
            *abortReqPresence = FALSE;
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_StatusCode SC_CheckAbortChunk(SOPC_MsgBuffers* msgBuffer,
                                   SOPC_String*     reason){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t errorCode = 0;
    if(msgBuffer != NULL && reason != NULL){
        if(msgBuffer->isFinal == SOPC_Msg_Chunk_Abort){
            status = SOPC_UInt32_Read(&errorCode, msgBuffer);
            if(status == STATUS_OK){
                status = errorCode;
                SOPC_String_Read(reason, msgBuffer);
            }

        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_StatusCode SC_DecodeChunk(SOPC_MsgBuffers*      msgBuffers,
                               uint32_t              requestId,
                               SOPC_EncodeableType*  expEncType,
                               SOPC_EncodeableType*  errEncType,
                               SOPC_EncodeableType** recEncType,
                               void**                encObj){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t msgSize = 0;
    Buffer* tmpBuffer = NULL;
    uint32_t idx = 0;
    SOPC_MsgBuffer* tmpMsgBuffer = NULL;
    Buffer* buffer = NULL;

    if(msgBuffers != NULL &&
       recEncType != NULL && encObj != NULL)
    {

        switch(msgBuffers->isFinal){
            case SOPC_Msg_Chunk_Final:
                // Treat case with only 1 chunk for response
                if(msgBuffers->nbChunks == 1){
                    status = SC_DecodeMsgBody(msgBuffers,
                                              &msgBuffers->nsTable,
                                              msgBuffers->encTypesTable,
                                              expEncType,
                                              errEncType,
                                              recEncType,
                                              encObj);
                }else{
                    //Store all buffers in 1 to could decode msg body !
                    for(idx = 0; idx < msgBuffers->nbChunks; idx++){
                        msgSize += msgBuffers->buffers[idx].length;
                    }
                    buffer = Buffer_Create(msgSize);

                    if(buffer == NULL){
                        status = STATUS_NOK;
                    }

                    // Copy all buffers content into temporary buffer for decoding
                    for(idx = 0; idx < msgBuffers->nbChunks && status == STATUS_OK; idx++){
                        tmpBuffer = &msgBuffers->buffers[idx];
                        status = Buffer_Write(buffer, tmpBuffer->data, tmpBuffer->length);
                    }

                    if(status == STATUS_OK){
                        tmpMsgBuffer = MsgBuffer_Create(buffer,
                                                        msgBuffers->maxChunks,
                                                        NULL,
                                                        &msgBuffers->nsTable,
                                                        msgBuffers->encTypesTable);
                        if(tmpMsgBuffer != NULL){
                            status = SC_DecodeMsgBody(tmpMsgBuffer,
                                                      &msgBuffers->nsTable,
                                                      msgBuffers->encTypesTable,
                                                      expEncType,
                                                      errEncType,
                                                      recEncType,
                                                      encObj);
                        }
                    }

                    if(tmpMsgBuffer == NULL){
                        // In any case buffer deallocation guarantee
                        Buffer_Delete(buffer);
                    }else{
                        // In case msg buffer is allocated, buffer also deallocated here
                        MsgBuffers_Delete(&tmpMsgBuffer);
                    }
                }

                MsgBuffers_Reset(msgBuffers);
                break;
            case SOPC_Msg_Chunk_Intermediate:
                assert(msgBuffers->nbChunks >= 1);
                if(msgBuffers->nbChunks == 1){
                    msgBuffers->msgRequestId = requestId;
                }else if(msgBuffers->msgRequestId != requestId){
                    status = STATUS_NOK;
                    assert(FALSE);
                }
                break;
            case SOPC_Msg_Chunk_Abort:
                status = STATUS_NOK;
                assert(FALSE);
                break;
            case SOPC_Msg_Chunk_Unknown:
            case SOPC_Msg_Chunk_Invalid:
                status = STATUS_NOK;
                assert(FALSE);
        }
    }

    return status;
}

