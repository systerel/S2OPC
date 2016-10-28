/*
 * secure_channel_low_level.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */

#include <wrappers.h>

#include <assert.h>
#include <stdlib.h>

#include "ua_secure_channel_low_level.h"

#include <ua_builtintypes.h>
#include <ua_encoder.h>
#include <ua_tcp_ua_low_level.h>
#include <secret_buffer.h>
#include <crypto_provider.h>

const uint32_t scProtocolVersion = 0;

static const UA_String UA_String_Security_Policy_None = {
        .length = 47,
        .characters = (UA_Byte*) SECURITY_POLICY_NONE,
        .clearBytes = FALSE
};

static const UA_String UA_String_Security_Policy_Basic128Rsa15 = {
        .length = 56,
        .characters = (UA_Byte*) SECURITY_POLICY_BASIC128RSA15,
        .clearBytes = FALSE
};

static const UA_String UA_String_Security_Policy_Basic256 = {
        .length = 51,
        .characters = (UA_Byte*) SECURITY_POLICY_BASIC256,
        .clearBytes = FALSE
};

static const UA_String UA_String_Security_Policy_Basic256Sha256 = {
        .length = 57,
        .characters = (UA_Byte*) SECURITY_POLICY_BASIC256SHA256,
        .clearBytes = FALSE
};

SC_Connection* SC_Create (){
    SC_Connection* sConnection = NULL;
    TCP_UA_Connection* connection = TCP_UA_Connection_Create(scProtocolVersion);

    if(connection != NULL){
        sConnection = (SC_Connection *) malloc(sizeof(SC_Connection));

        if(sConnection != 0){
            memset (sConnection, 0, sizeof(SC_Connection));
            sConnection->transportConnection = connection;
            sConnection->state = SC_Connection_Error;
            ByteString_Initialize(&sConnection->runningAppCertificate);
            ByteString_Initialize(&sConnection->runningAppPublicKey);
            ByteString_Initialize(&sConnection->otherAppCertificate);
            ByteString_Initialize(&sConnection->otherAppPublicKey);
            ByteString_Initialize(&sConnection->currentSecuPolicy);
            ByteString_Initialize(&sConnection->precSecuPolicy);

        }else{
            TCP_UA_Connection_Delete(connection);
        }
    }
    return sConnection;
}

void SC_Delete (SC_Connection* scConnection){
    if(scConnection != NULL){
        // Do not delete runningAppCertificate, runnigAppPrivateKey and otherAppCertificate:
        //  managed by upper level
        ByteString_Clear(&scConnection->runningAppCertificate);
        ByteString_Clear(&scConnection->runningAppPublicKey);
        // Never deallocate private key, provided by upper level (client/server connection)
        scConnection->runningAppPrivateKey = NULL;
        ByteString_Clear(&scConnection->otherAppCertificate);
        ByteString_Clear(&scConnection->otherAppPublicKey);
        if(scConnection->sendingBuffer != NULL)
        {
            MsgBuffer_Delete(&scConnection->sendingBuffer);
        }
        if(scConnection->receptionBuffers != NULL)
        {
            MsgBuffers_Delete(&scConnection->receptionBuffers);
        }
        if(scConnection->transportConnection != NULL)
        {
            TCP_UA_Connection_Delete(scConnection->transportConnection);
        }
        String_Clear(&scConnection->currentSecuPolicy);
        ByteString_Clear(&scConnection->precSecuPolicy);
        SecretBuffer_DeleteClear(scConnection->currentNonce);
        KeySet_Delete(scConnection->currentSecuKeySets.receiverKeySet);
        KeySet_Delete(scConnection->currentSecuKeySets.senderKeySet);
        KeySet_Delete(scConnection->precSecuKeySets.receiverKeySet);
        KeySet_Delete(scConnection->precSecuKeySets.senderKeySet);
        CryptoProvider_Delete(scConnection->currentCryptoProvider);
        CryptoProvider_Delete(scConnection->precCryptoProvider);
        free(scConnection);
    }
}

StatusCode SC_InitApplicationIdentities(SC_Connection* scConnection,
                                        UA_ByteString* runningAppCertificate,
                                        SecretBuffer*  runningAppPrivateKey,
                                        UA_ByteString* otherAppCertificate){
    StatusCode status = STATUS_OK;
    if(scConnection->runningAppCertificate.length <= 0 &&
       scConnection->runningAppPrivateKey == NULL &&
       scConnection->otherAppCertificate.length <= 0)
    {
        if(runningAppCertificate != NULL){
            status = ByteString_AttachFrom(&scConnection->runningAppCertificate,
                                           runningAppCertificate);
        }
        ByteString_Clear(&scConnection->runningAppPublicKey);
        scConnection->runningAppPrivateKey = runningAppPrivateKey;

        if(otherAppCertificate != NULL){
            status = ByteString_AttachFrom(&scConnection->otherAppCertificate,
                                           otherAppCertificate);
        }
        ByteString_Clear(&scConnection->otherAppPublicKey);
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

StatusCode SC_InitReceiveSecureBuffers(SC_Connection* scConnection,
                                       UA_NamespaceTable*  namespaceTable,
                                       UA_EncodeableType** encodeableTypes)
{
    StatusCode status = STATUS_INVALID_STATE;
    if(scConnection->receptionBuffers == NULL){
        if(scConnection->transportConnection->maxChunkCountRcv != 0)
        {
            scConnection->receptionBuffers = MsgBuffers_Create
             (scConnection->transportConnection->maxChunkCountRcv,
              scConnection->transportConnection->receiveBufferSize,
              namespaceTable,
              encodeableTypes);

        }else if(scConnection->transportConnection->maxMessageSizeRcv != 0){
            // Is message size including whole message or only body as it is the case in part 4 §5.3 last § ???
            scConnection->receptionBuffers = MsgBuffers_Create
              (scConnection->transportConnection->maxMessageSizeRcv
               /scConnection->transportConnection->receiveBufferSize,
               scConnection->transportConnection->receiveBufferSize,
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

StatusCode SC_InitSendSecureBuffer(SC_Connection* scConnection,
                                   UA_NamespaceTable*  namespaceTable,
                                   UA_EncodeableType** encodeableTypes)
{
    StatusCode status = STATUS_NOK;
    UA_MsgBuffer* msgBuffer;
    if(scConnection->sendingBuffer == NULL){
        Buffer* buf = Buffer_Create(scConnection->transportConnection->sendBufferSize);
        msgBuffer = MsgBuffer_Create(buf,
                                     scConnection->transportConnection->maxChunkCountSnd,
                                     scConnection,
                                     namespaceTable,
                                     encodeableTypes);
        if(msgBuffer != NULL){
            scConnection->sendingBuffer = msgBuffer;
            status = STATUS_OK;
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

// TODO: delete this one.
StatusCode RetrievePublicKeyFromCert(CryptoProvider* cryptoProvider,
                                     UA_ByteString*  certificate,
                                     UA_ByteString*  publicKey)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t publicKeyLength = 0;
    if(cryptoProvider != NULL &&
       publicKey != NULL &&
       certificate != NULL)
    {
        status = CryptoProvider_GetPublicKeyLengthFromCert(cryptoProvider,
                                                           certificate,
                                                           &publicKeyLength);
        if(status == STATUS_OK){
            status = ByteString_InitializeFixedSize(publicKey, publicKeyLength);
        }

        if(status == STATUS_OK){
            status = CryptoProvider_GetPublicKeyFromCert(cryptoProvider,
                                                         certificate,
                                                         publicKey);
        }else{
            status = STATUS_NOK;
        }

    }else if(publicKey == NULL){
        status = STATUS_NOK;
    }

    return status;
}

// Retrieve public key from certificate and set it in internal properties
StatusCode SC_RetrieveAndSetPublicKeyFromCert(SC_Connection*  scConnection,
                                              uint32_t        runningApp,
                                              UA_ByteString** publicKey)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ByteString* certificate = NULL;

    if(scConnection != NULL &&
       publicKey != NULL && *publicKey == NULL)
    {
        if(runningApp == FALSE)
        {
            certificate = &scConnection->otherAppCertificate;
            *publicKey = &scConnection->otherAppPublicKey;
        }else{
            certificate = &scConnection->runningAppCertificate;
            *publicKey = &scConnection->runningAppPublicKey;
        }

        if((*publicKey)->length <= 0){
            // Key never extracted from cert. before:
            // TODO: use KeyManager_Certificate_GetPublicKey
            status = RetrievePublicKeyFromCert(scConnection->currentCryptoProvider,
                                               certificate,
                                               *publicKey);
        }else{
            status = STATUS_OK;
        }
    }

    return status;
}

//// Cryptographic properties helpers

uint8_t Is_ExtraPaddingSizePresent(uint32_t plainBlockSize){
    // Extra-padding necessary if padding could be greater 256 bytes (2048 bits)
    // (1 byte for padding size field + 255 bytes of padding).
    // => padding max value is plainBlockSize regarding the formula of padding size
    //    (whereas spec part 6 indicates it depends on the key size which is incorrect)
    return plainBlockSize > 256;
}

StatusCode GetAsymmBlocksSizes(UA_String* securityPolicyUri,
                               uint32_t   keySize,
                               uint32_t*  cipherTextBlockSize,
                               uint32_t*  plainTextBlockSize){
    StatusCode status = STATUS_OK;
    const uint32_t sha1outputLength = 20; // bytes (160 bits)

    if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_None) != FALSE)
    {
        *cipherTextBlockSize = 1;
        *plainTextBlockSize = 1;
    }else if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_Basic128Rsa15) != FALSE){
        // RSA 1.5: RSA spec 7.2.1 (https://tools.ietf.org/html/rfc3447)
        *cipherTextBlockSize = keySize;
        *plainTextBlockSize = keySize - 11;
    }else if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_Basic256) != FALSE){
        // RSA OAEP: RSA spec 7.1.1 (https://tools.ietf.org/html/rfc3447)
        // + RSA spec 10.1 : "For the EME-OAEP encoding method, only SHA-1 is recommended."
        *cipherTextBlockSize = keySize;
        *plainTextBlockSize = keySize - 2 -2*sha1outputLength;
   }else if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_Basic256Sha256) != FALSE){
       // RSA OAEP: RSA spec 7.1.1 (https://tools.ietf.org/html/rfc3447)
       // + RSA spec 10.1 : "For the EME-OAEP encoding method, only SHA-1 is recommended."
       *cipherTextBlockSize = keySize;
       *plainTextBlockSize = keySize - 2 -2*sha1outputLength;
   }else{
       status = STATUS_INVALID_PARAMETERS;
   }
   return status;
}

uint32_t GetAsymmSignatureSize(UA_String* securityPolicyUri,
                               uint32_t   privateKeySize)
{
    uint32_t signatureSize = 0;
    if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_None) != FALSE)
    {
        signatureSize = 0;
    }else if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_Basic128Rsa15) != FALSE){
        // Regarding RSA spec 5.2.1 (https://tools.ietf.org/html/rfc3447), signature size = key size
        signatureSize = privateKeySize;
    }else if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_Basic256) != FALSE){
        // Regarding RSA spec 5.2.1 (https://tools.ietf.org/html/rfc3447), signature size = key size
        signatureSize = privateKeySize;
    }else if(String_Equal(securityPolicyUri, &UA_String_Security_Policy_Basic256Sha256) != FALSE){
        // Regarding RSA spec 5.2.1 (https://tools.ietf.org/html/rfc3447), signature size = key size
        signatureSize = privateKeySize;
    }
    return signatureSize;
}

uint32_t GetMaxBodySize(UA_MsgBuffer* msgBuffer,
                        uint32_t     cipherBlockSize,
                        uint32_t     plainBlockSize,
                        uint32_t     signatureSize)
{
    // Ensure cipher block size is greater or equal to plain block size:
    //  otherwise the plain size could be greater than the  buffer size regarding computation
    assert(cipherBlockSize >= plainBlockSize);
    // By default only 1 byte for padding size field. +1 if extra padding
    uint32_t paddingSizeFields = 1;
    const uint32_t headersSize = msgBuffer->sequenceNumberPosition - 1;
    const uint32_t bodyChunkSize = msgBuffer->buffers->max_size - headersSize;

    // Computed maxBlock and then maxBodySize based on revised formula of mantis ticket #2897
    // Spec 1.03 part 6 incoherent
    const uint32_t maxBlocks = bodyChunkSize / cipherBlockSize;

    if(Is_ExtraPaddingSizePresent(plainBlockSize) != FALSE){
        paddingSizeFields += 1;
    }

    // MaxBodySize = unCiphered block size * max blocs - sequence header -1 for PaddingSize field(s)
    return plainBlockSize * maxBlocks - UA_SECURE_MESSAGE_SEQUENCE_LENGTH - signatureSize - paddingSizeFields;
}

// Get information from internal properties
StatusCode GetEncryptedDataLength(SC_Connection* scConnection,
                                  uint32_t       plainDataLength,
                                  uint32_t       symmetricAlgo,
                                  uint32_t*      cipherDataLength)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;

    if(scConnection != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == FALSE){
        if(scConnection->otherAppCertificate.length <= 0){
           status = STATUS_INVALID_STATE;
        }else{

            UA_ByteString* otherAppPublicKey = NULL;

            // Retrieve other app public key from certificate
            // TODO integration: see if and how it was avoided previously
            status = SC_RetrieveAndSetPublicKeyFromCert(scConnection, FALSE, &otherAppPublicKey);

            // Retrieve cipher length
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricGetLength_Encryption(scConnection->currentCryptoProvider,
                                                                       otherAppPublicKey,
                                                                       plainDataLength,
                                                                       cipherDataLength);
            }
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

uint32_t IsMsgEncrypted(UA_MessageSecurityMode securityMode,
                        UA_MsgBuffer*          msgBuffer)
{
    assert(securityMode != UA_MessageSecurityMode_Invalid);
    uint32_t toEncrypt = 1; // True
    // Determine if the message must be encrypted
    if(securityMode == UA_MessageSecurityMode_None ||
       (securityMode == UA_MessageSecurityMode_Sign &&
        msgBuffer->secureType != UA_OpenSecureChannel))
    {
        toEncrypt = FALSE;
    }

    return toEncrypt;
}

uint32_t IsMsgSigned(UA_MessageSecurityMode securityMode)
{
    uint32_t toSign = 1; // True
    // Determine if the message must be signed
    if(securityMode == UA_MessageSecurityMode_None)
    {
        toSign = FALSE;
    }
    return toSign;
}

StatusCode CheckMaxSenderCertificateSize(UA_ByteString*  senderCertificate,
                                         uint32_t        messageChunkSize,
                                         UA_String*      securityPolicyUri,
                                         uint8_t         hasPadding,
                                         uint32_t        padding,
                                         uint32_t        extraPadding,
                                         uint32_t        asymmetricSignatureSize){
    StatusCode status = STATUS_NOK;
    int32_t maxSize = // Fit in a single message chunk with at least 1 byte of body
     messageChunkSize -
     UA_SECURE_MESSAGE_HEADER_LENGTH -
     4 - // URI length field size
     securityPolicyUri->length -
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

    if(senderCertificate->length <= maxSize){
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

StatusCode SC_SetMaxBodySize(SC_Connection* scConnection,
                             uint32_t       isSymmetric){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != NULL){
        uint32_t cipherBlockSize = 0;
        uint32_t plainBlockSize =0;
        if(isSymmetric == FALSE){
            uint32_t publicKeyModulusLength = 0;
            UA_ByteString* publicKey = NULL;
            // TODO: isn't this already done?
            SC_RetrieveAndSetPublicKeyFromCert(scConnection, FALSE, &publicKey);
            // TODO: use CryptoProvider_AsymmetricGetLength_Msgs or CryptoProvider_AsymmetricGetLength_MsgPlainText instead, if possible.
            status = CryptoProvider_GetAsymmPublicKeyModulusLength(scConnection->currentCryptoProvider,
                                                                   publicKey,
                                                                   &publicKeyModulusLength);

            if(status == STATUS_OK){
                // TODO: use CryptoProvider_AsymmetricGetLength_Msgs
                status = GetAsymmBlocksSizes(&scConnection->currentSecuPolicy,
                                             publicKeyModulusLength,
                                             &cipherBlockSize,
                                             &plainBlockSize);
                 if(status == STATUS_OK){
                     //TODO: CryptoProvider_AsymmetricGetLength_Signature
                     scConnection->sendingMaxBodySize = GetMaxBodySize(scConnection->sendingBuffer,
                                                                       cipherBlockSize,
                                                                       plainBlockSize,
                                                                       GetAsymmSignatureSize
                                                                        (&scConnection->currentSecuPolicy,
                                                                         publicKeyModulusLength));
                 }
            }else{
                status = STATUS_NOK;
            }
        }else{
            status = CryptoProvider_SymmetricGetLength_Blocks(scConnection->currentCryptoProvider,
                                                              &cipherBlockSize,
                                                              &plainBlockSize);
             if(status == STATUS_OK){
                 uint32_t signatureSize = 0;
                 status = CryptoProvider_SymmetricGetLength_Signature(scConnection->currentCryptoProvider,
                                                                      &signatureSize);
                 if(status == STATUS_OK){
                     scConnection->sendingMaxBodySize = GetMaxBodySize(scConnection->sendingBuffer,
                                                                       cipherBlockSize,
                                                                       plainBlockSize,
                                                                       signatureSize);
                 }
             }
        }
    }
    return status;
}

//// End cryptographic properties helpers

StatusCode SC_EncodeSecureMsgHeader(UA_MsgBuffer*        msgBuffer,
                                    UA_SecureMessageType smType,
                                    uint32_t             secureChannelId)
{
    StatusCode status = STATUS_NOK;
    UA_Byte fByte = 'F';
    if(msgBuffer != NULL){
        status = STATUS_OK;
        switch(smType){
                case UA_SecureMessage:
                    status = Buffer_Write(msgBuffer->buffers, MSG, 3);
                    break;
                case UA_OpenSecureChannel:
                    status = Buffer_Write(msgBuffer->buffers, OPN, 3);
                    break;
                case UA_CloseSecureChannel:
                    status = Buffer_Write(msgBuffer->buffers, CLO, 3);
                    break;
        }
        status = MsgBuffer_SetSecureMsgType(msgBuffer, smType);
        if(status == STATUS_OK){
            // Default behavior: final except if too long for UA_SecureMessage only !
            status = Buffer_Write(msgBuffer->buffers, &fByte, 1);
        }
        if(status == STATUS_OK){
            msgBuffer->isFinal = UA_Msg_Chunk_Final;
            // Temporary message size
            const uint32_t msgHeaderLength = UA_SECURE_MESSAGE_HEADER_LENGTH;
            status = UInt32_Write(&msgHeaderLength, msgBuffer);
        }
        if(status == STATUS_OK){
            // Secure channel Id
            status = UInt32_Write(&secureChannelId, msgBuffer);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
    }

    return status;
}

StatusCode SC_EncodeSequenceHeader(UA_MsgBuffer* msgBuffer,
                                   uint32_t      requestId){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    const uint32_t zero = 0;
    if(msgBuffer != NULL){
        status = STATUS_OK;
    }

    // Set temporary SN value: to be set on message sending (ensure contiguous SNs)
    if(status == STATUS_OK){
        msgBuffer->sequenceNumberPosition = msgBuffer->buffers->position;
        UInt32_Write(&zero, msgBuffer);
    }

    if(status == STATUS_OK){
        UInt32_Write(&requestId, msgBuffer);
    }

    return status;
}

StatusCode EncodeAsymmSecurityHeader(CryptoProvider*        cryptoProvider,
                                     UA_MsgBuffer*          msgBuffer,
                                     UA_MessageSecurityMode secuMode,
                                     UA_String*             securityPolicy,
                                     UA_ByteString*         senderCertificate,
                                     UA_ByteString*         receiverCertificate){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True
    if(cryptoProvider != NULL && msgBuffer != NULL &&
       securityPolicy != NULL &&
       senderCertificate != NULL &&
       receiverCertificate != NULL)
    {
        toEncrypt = IsMsgEncrypted(secuMode, msgBuffer);
        toSign = IsMsgSigned(secuMode);
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        if(securityPolicy->length>0){
            status = String_Write(securityPolicy, msgBuffer);
        }else{
            // Null security policy is invalid parameter since unspecified
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    // Sender Certificate:
    if(status == STATUS_OK){
        if(toSign != FALSE && senderCertificate->length>0){ // Field shall be null if message not signed
            status = String_Write(senderCertificate, msgBuffer);
        }else{
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            // status = Write_Int32(msgBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            const int32_t minusOne = -1;
            status = Int32_Write(&minusOne, msgBuffer);
            // NULL string: nothing to write
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){

        UA_ByteString recCertThumbprint;
        if(toEncrypt != FALSE){
            int32_t thumbprintLength = 0;
            status = CryptoProvider_GetCertThumbprintLength(cryptoProvider,
                                                            receiverCertificate,
                                                            &thumbprintLength);
            if(status == STATUS_OK){
                status = ByteString_InitializeFixedSize(&recCertThumbprint, thumbprintLength);
                if(status == STATUS_OK){
                    status = CryptoProvider_GetCertThumbprint(cryptoProvider,
                                                              receiverCertificate,
                                                              &recCertThumbprint);
                }else{
                    status = STATUS_NOK;
                }
            }

            status = String_Write(&recCertThumbprint, msgBuffer);
        }else{
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            //status = Write_Int32(msgBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            const int32_t minusOne = -1;
            status = Int32_Write(&minusOne, msgBuffer);
            // NULL string: nothing to write
        }

        ByteString_Clear(&recCertThumbprint);
    }else{
        status = STATUS_NOK;
    }

    return status;
}

StatusCode SC_EncodeAsymmSecurityHeader(SC_Connection* scConnection,
                                        UA_String*     securityPolicy,
                                        UA_ByteString* senderCertificate,
                                        UA_ByteString* receiverCertificate){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != NULL)
    {
        status = EncodeAsymmSecurityHeader(scConnection->currentCryptoProvider,
                                           scConnection->sendingBuffer,
                                           scConnection->currentSecuMode,
                                           securityPolicy,
                                           senderCertificate,
                                           receiverCertificate);
    }

    return status;
}

StatusCode SC_EncodeMsgBody(UA_MsgBuffer*      msgBuffer,
                            UA_EncodeableType* encType,
                            void*              msgBody)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_NodeId nodeId;
    NodeId_Initialize(&nodeId);

    if(msgBuffer != NULL && msgBody != NULL &&
       encType != NULL){
        nodeId.identifierType = IdentifierType_Numeric;
        if(encType->namespace == NULL){
            nodeId.namespace = 0;
        }else{
            // TODO: find namespace Id
        }
        nodeId.numeric = encType->binaryTypeId;

        status = NodeId_Write(&nodeId, msgBuffer);
    }
    if(status == STATUS_OK){
        status = encType->encodeFunction(msgBody, msgBuffer);
    }
    return status;
}

StatusCode SC_WriteSecureMsgBuffer(UA_MsgBuffer*  msgBuffer,
                                   const UA_Byte* data_src,
                                   uint32_t       count){
    SC_Connection* scConnection = NULL;
    StatusCode status = STATUS_NOK;
    if(data_src == NULL || msgBuffer == NULL || msgBuffer->flushData == NULL)
    {
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = STATUS_OK;
        scConnection = (SC_Connection*) msgBuffer->flushData;
        if(msgBuffer->buffers->position + count >
            msgBuffer->sequenceNumberPosition +
            UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
            scConnection->sendingMaxBodySize)
        {
            // Precedent position cannot be greater than message size:
            //  otherwise it means size has not been checked precedent time (it could occurs only when writing headers)
            assert(msgBuffer->sequenceNumberPosition +
                    UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                    scConnection->sendingMaxBodySize >= msgBuffer->buffers->position);
            if(msgBuffer->maxChunks != 0 && msgBuffer->nbChunks + 1 > msgBuffer->maxChunks){
                // TODO: send an abort message instead of message !!!
                status = STATUS_INVALID_STATE;
            }else{
                // Fulfill buffer with maximum amount of bytes
                uint32_t tmpCount = // Maximum Count - Precedent Count => Count to write
                 (msgBuffer->sequenceNumberPosition + UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                  scConnection->sendingMaxBodySize) - msgBuffer->buffers->position;
                count = count - tmpCount;
                status = Buffer_Write(msgBuffer->buffers, data_src, tmpCount);

                // Flush it !
                if(status == STATUS_OK){
                    status = SC_FlushSecureMsgBuffer(msgBuffer,
                                                     UA_Msg_Chunk_Intermediate);
                }

                if(status == STATUS_OK){
                    status = MsgBuffer_ResetNextChunk
                              (msgBuffer,
                               msgBuffer->sequenceNumberPosition +
                                UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                }
            }
        }
        if(status == STATUS_OK){
            status = Buffer_Write(msgBuffer->buffers, data_src, count);
        }
    }
    return status;
}

StatusCode Set_Message_Length(UA_MsgBuffer* msgBuffer,
                              uint32_t      msgLength){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;
    if(msgBuffer != NULL && msgLength < msgBuffer->buffers->max_size){
        originPosition = msgBuffer->buffers->position;
        status = Buffer_SetPosition(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(&msgLength, msgBuffer);
    }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(msgBuffer->buffers, originPosition);
        msgBuffer->currentChunkSize = msgLength;
    }
    return status;
}

StatusCode Set_Message_Chunk_Type(UA_MsgBuffer*    msgBuffer,
                                  UA_MsgFinalChunk chunkType){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;

    if(msgBuffer != NULL){
        originPosition = msgBuffer->buffers->position;
        status = Buffer_SetPosition(msgBuffer->buffers, UA_HEADER_ISFINAL_POSITION);
    }

    UA_Byte chunkTypeByte;
    switch(chunkType){
        case UA_Msg_Chunk_Final:
            chunkTypeByte = 'F';
            break;
        case UA_Msg_Chunk_Intermediate:
            chunkTypeByte = 'C';
            break;
        case UA_Msg_Chunk_Abort:
            chunkTypeByte = 'A';
            break;
        default:
            status = STATUS_INVALID_PARAMETERS;
    }

    if(status == STATUS_OK){
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &chunkTypeByte, 1);
    }

    if(status == STATUS_OK){
        status = Buffer_SetPosition(msgBuffer->buffers, originPosition);
    }

    return status;
}

StatusCode Set_Sequence_Number(UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SC_Connection* scConnection = NULL;
    uint32_t originPosition = 0;

    if(msgBuffer != NULL && msgBuffer->flushData != NULL){
       status = STATUS_OK;
       scConnection = (SC_Connection*) msgBuffer->flushData;
       if(scConnection-> lastSeqNumSent > UINT32_MAX - 1024){ // Part 6 §6.7.2 v1.03
           scConnection->lastSeqNumSent = 1;
       }else{
           scConnection->lastSeqNumSent = scConnection-> lastSeqNumSent + 1;
       }
       originPosition = msgBuffer->buffers->position;
       status = Buffer_SetPosition(msgBuffer->buffers, msgBuffer->sequenceNumberPosition);
       if(status == STATUS_OK){
           UInt32_Write(&scConnection->lastSeqNumSent, msgBuffer);
       }

       if(status == STATUS_OK){
               status = Buffer_SetPosition(msgBuffer->buffers, originPosition);
       }

    }

    return status;
}

StatusCode EncodePadding(SC_Connection* scConnection,
                          UA_MsgBuffer* msgBuffer,
                          uint8_t       symmetricAlgo,
                          uint8_t*      hasPadding,
                          uint16_t*     realPaddingLength, // >= paddingSizeField
                          uint8_t*      hasExtraPadding,
                          uint32_t*     signatureSize)
{
    StatusCode status = STATUS_OK;
    uint32_t plainBlockSize = 0;
    uint32_t cipherBlockSize = 0;
    *hasPadding = 1; // True

    if(symmetricAlgo == FALSE){
        if(scConnection->runningAppCertificate.length <= 0 ||
           scConnection->runningAppPrivateKey == NULL ||
           scConnection->otherAppCertificate.length <= 0){
           status = STATUS_INVALID_STATE;
        }else{

            uint32_t publicKeyModulusLength = 0;
            UA_ByteString* publicKey = NULL;
            SC_RetrieveAndSetPublicKeyFromCert(scConnection, FALSE, &publicKey);

            // TODO: use CryptoProvider_AsymmetricGetLength_Msgs or CryptoProvider_AsymmetricGetLength_MsgPlainText instead, if possible.
            status = CryptoProvider_GetAsymmPublicKeyModulusLength(scConnection->currentCryptoProvider,
                                                                   publicKey,
                                                                   &publicKeyModulusLength);

            if(status == STATUS_OK){
                // TODO: use CryptoProvider_AsymmetricGetLength_Signature
                *signatureSize = GetAsymmSignatureSize
                                  (&scConnection->currentSecuPolicy,
                                   publicKeyModulusLength);
                // TODO: use CryptoProvider_AsymmetricGetLength_Msgs
                status = GetAsymmBlocksSizes
                          (&scConnection->currentSecuPolicy,
                           publicKeyModulusLength,
                           &cipherBlockSize,
                           &plainBlockSize);
            }

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
        EncodeDecode_UInt16(realPaddingLength);
        status = Buffer_Write(msgBuffer->buffers, (UA_Byte*) realPaddingLength, 1);
        paddingSizeField = 0xFF & *realPaddingLength;

        if(status == STATUS_OK){
            // The value of each byte of the padding is equal to paddingSize:
            UA_Byte paddingBytes[*realPaddingLength];
            memset(paddingBytes, paddingSizeField, *realPaddingLength);
            status = Buffer_Write(msgBuffer->buffers, paddingBytes, *realPaddingLength);
        }

        // Extra-padding necessary if padding could be greater 256 bytes
        if(status == STATUS_OK && Is_ExtraPaddingSizePresent(plainBlockSize) != FALSE){
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            UA_Byte extraPadding = 0x00FF & *realPaddingLength;
            Buffer_Write(msgBuffer->buffers, &extraPadding, 1);
        }
    }

    return status;
}

StatusCode EncodeSignature(SC_Connection* scConnection,
                           UA_MsgBuffer* msgBuffer,
                           uint8_t       symmetricAlgo,
                           uint32_t      signatureSize)
{
    StatusCode status = STATUS_OK;
    UA_ByteString signedData;
    if(symmetricAlgo == FALSE){
        if(scConnection->runningAppCertificate.length <= 0 ||
           scConnection->runningAppPrivateKey == NULL ||
           scConnection->otherAppCertificate.length <= 0){
           status = STATUS_INVALID_STATE;
        }else{
            status = ByteString_InitializeFixedSize(&signedData, signatureSize);
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricSign(scConnection->currentCryptoProvider,
                                                           msgBuffer->buffers->data,
                                                           msgBuffer->buffers->length,
                                                           scConnection->runningAppPrivateKey,
                                                           signedData.characters,
                                                           signedData.length);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers,
                                      signedData.characters,
                                      signedData.length);
            }
            ByteString_Clear(&signedData);
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            status = STATUS_INVALID_STATE;
        }else{
            status = ByteString_InitializeFixedSize(&signedData, signatureSize);
            if(status == STATUS_OK){
                status = CryptoProvider_SymmetricSign
                          (scConnection->currentCryptoProvider,
                           msgBuffer->buffers->data,
                           msgBuffer->buffers->length,
                           scConnection->currentSecuKeySets.senderKeySet->signKey,
                           signedData.characters,
                           signedData.length);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers,
                                      signedData.characters,
                                      signedData.length);
            }
            ByteString_Clear(&signedData);
        }
    }
    return status;
}

StatusCode EncryptMsg(SC_Connection* scConnection,
                      UA_MsgBuffer*  msgBuffer,
                      uint8_t        symmetricAlgo,
                      uint32_t       encryptedDataLength,
                      UA_MsgBuffer*  encryptedMsgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte* dataToEncrypt = &msgBuffer->buffers->data[msgBuffer->sequenceNumberPosition];
    const uint32_t dataToEncryptLength = msgBuffer->buffers->length - msgBuffer->sequenceNumberPosition;

    if(scConnection != NULL && msgBuffer != NULL &&
       encryptedMsgBuffer != NULL && encryptedMsgBuffer->buffers != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == FALSE){
        if(scConnection->runningAppCertificate.length <= 0 ||
           scConnection->runningAppPrivateKey == NULL ||
           scConnection->otherAppCertificate.length <= 0){
           status = STATUS_INVALID_STATE;
        }else{
            UA_ByteString* otherAppPublicKey = NULL;
            UA_Byte* encryptedData = NULL;

            if(status == STATUS_OK){
                // Retrieve other app public key from certificate
                status = SC_RetrieveAndSetPublicKeyFromCert(scConnection, FALSE, &otherAppPublicKey);
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
                           &encryptedDataLength);
            }

        } // End valid asymmetric encryption data
    }else if (status == STATUS_OK){
        if(scConnection->currentSecuKeySets.senderKeySet == NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == NULL){
            status = STATUS_INVALID_STATE;
        }else{
            UA_Byte* encryptedData = NULL;

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

StatusCode SC_FlushSecureMsgBuffer(UA_MsgBuffer*     msgBuffer,
                                   UA_MsgFinalChunk  chunkType){
    SC_Connection* scConnection = NULL;
    StatusCode status = STATUS_NOK;
    uint8_t toEncrypt = 1; // True
    uint8_t toSign = 1; // True
    uint8_t symmetricAlgo = 1; // True;
    uint8_t hasPadding = 0;
    uint16_t paddingLength = 0;
    uint8_t hasExtraPadding = 0;
    uint32_t signatureSize = 0;
    uint32_t encryptedLength = 0;

    if(msgBuffer == NULL || msgBuffer->flushData == NULL ||
       (chunkType != UA_Msg_Chunk_Final && msgBuffer->secureType != UA_SecureMessage))
    {
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = STATUS_OK;
        scConnection = (SC_Connection*) msgBuffer->flushData;

        toEncrypt = IsMsgEncrypted(scConnection->currentSecuMode, msgBuffer);

        toSign = IsMsgSigned(scConnection->currentSecuMode);

        // Determine if the asymmetric algorithms must be used
        if(msgBuffer->secureType == UA_OpenSecureChannel){
            symmetricAlgo = FALSE;
        }

        //// Encode padding if encrypted message
        if(toEncrypt == FALSE){
            // No padding fields
        }else{
            status = EncodePadding(scConnection,
                                   msgBuffer,
                                   symmetricAlgo,
                                   &hasPadding,
                                   &paddingLength,
                                   &hasExtraPadding,
                                   &signatureSize);
        }

        //// Set the chunk type for the given message
        if(status == STATUS_OK){
            status = Set_Message_Chunk_Type(msgBuffer, chunkType);
        }

        //// Encode message length:
        // Compute final encrypted message length:
        if(status == STATUS_OK){
            const uint32_t plainDataToEncryptLength = // Already encoded message from encryption start + signature size
             msgBuffer->buffers->length - msgBuffer->sequenceNumberPosition + signatureSize;
            status = GetEncryptedDataLength(scConnection, plainDataToEncryptLength, symmetricAlgo, &encryptedLength);
        }
        // Set final message length
        if(status == STATUS_OK){
            status = Set_Message_Length(scConnection->sendingBuffer,
                                        msgBuffer->sequenceNumberPosition + encryptedLength);
        }

        //// Encode sequence number
        if(status == STATUS_OK){
            status = Set_Sequence_Number(msgBuffer);
        }

        //// Encode signature if message signed
        if(toSign == FALSE){
            // No signature field
        }else if(status == STATUS_OK){
            status = EncodeSignature(scConnection,
                                     msgBuffer,
                                     symmetricAlgo,
                                     signatureSize);

            if(symmetricAlgo != FALSE){
                status = CryptoProvider_SymmetricVerify(scConnection->currentCryptoProvider,
                                                        scConnection->sendingBuffer->buffers->data,
                                                        scConnection->sendingBuffer->buffers->length - signatureSize,
                                                        scConnection->currentSecuKeySets.senderKeySet->signKey,
                                                        &scConnection->sendingBuffer->buffers->data[scConnection->sendingBuffer->buffers->length - signatureSize],
                                                        signatureSize);
            }
            // TODO integration: AsymmetricVerify ???
        }

        //// Check sender certificate size is not bigger than maximum size to be sent
        if(status == STATUS_OK &&
           msgBuffer->secureType == UA_OpenSecureChannel &&
           toSign != FALSE)
        {
            status = CheckMaxSenderCertificateSize(&scConnection->runningAppCertificate,
                                                   msgBuffer->buffers->max_size,
                                                   &scConnection->currentSecuPolicy,
                                                   hasPadding,
                                                   paddingLength,
                                                   hasExtraPadding,
                                                   signatureSize);
        }

        if(status == STATUS_OK){
            if(toEncrypt == FALSE){
                // No encryption necessary but we need to attach buffer as transport buffer (done during encryption otherwise)
                status = MsgBuffer_CopyBuffer(scConnection->transportConnection->outputMsgBuffer,
                                              scConnection->sendingBuffer);
            }else{
                // TODO: use detach / attach to control references on the transport msg buffer ?
                status = EncryptMsg(scConnection,
                                    msgBuffer,
                                    symmetricAlgo,
                                    encryptedLength,
                                    scConnection->transportConnection->outputMsgBuffer);
            }
        }

        if(status == STATUS_OK){
            // TODO: detach transport buffer ?
            status = TCP_UA_FlushMsgBuffer(scConnection->transportConnection->outputMsgBuffer);
        }

        if(status == STATUS_OK && chunkType == UA_Msg_Chunk_Final){
            // Reset buffer for next sending
            MsgBuffer_Reset(scConnection->sendingBuffer);
        }
    }
    return status;
}

StatusCode SC_DecodeSecureMsgSCid(SC_Connection* scConnection,
                                  UA_MsgBuffer*  transportBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t secureChannelId = 0;
    if(scConnection != NULL &&
       transportBuffer != NULL)
    {
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        //TODO: on server side, randomize secure channel ids (table 26 part 6)!
        status = UInt32_Read(&secureChannelId, transportBuffer);
    }

    if(status == STATUS_OK){
        if(secureChannelId == 0){
            // A server cannot attribute 0 as secure channel id:
            //  not so clear but implied by 6.7.6 part 6: "may be 0 if the Message is an OPN"
            status = STATUS_INVALID_RCV_PARAMETER;
        }else if(scConnection->secureChannelId == 0){
            // Assign Id provided by server
            scConnection->secureChannelId = secureChannelId;
        }else if(scConnection->secureChannelId != secureChannelId){
            // Different Id assigned by server: invalid case (id never changes on same connection instance)
            status = STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

StatusCode SC_DecodeAsymmSecurityHeader(SC_Connection* scConnection,
                                        PKIProvider*   pkiProvider,
                                        UA_MsgBuffer*  transportBuffer,
                                        uint32_t       validateSenderCert,
                                        uint32_t*      sequenceNumberPosition)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True

    UA_ByteString securityPolicy;
    ByteString_Initialize(&securityPolicy);
    UA_ByteString senderCertificate;
    ByteString_Initialize(&senderCertificate);
    UA_ByteString receiverCertThumb;
    ByteString_Initialize(&receiverCertThumb);

    uint32_t validationStatusCode = 0;

    if(scConnection != NULL &&
       transportBuffer != NULL)
    {
        // Asymmetric security header must use current security parameters
        // (TODO: add guarantee we are treating last OPN sent: using pending requests ?)
        toEncrypt = IsMsgEncrypted(scConnection->currentSecuMode,
                                   transportBuffer);
        toSign = IsMsgSigned(scConnection->currentSecuMode);
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        status = ByteString_Read(&securityPolicy, transportBuffer);

        if(status == STATUS_OK){
            uint32_t secuPolicyComparison = 0;
            status = ByteString_Compare(&scConnection->currentSecuPolicy,
                                        &securityPolicy, &secuPolicyComparison);

            if(status != STATUS_OK || secuPolicyComparison != 0){
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }

    // Sender Certificate:
    if(status == STATUS_OK){
        status = ByteString_Read(&senderCertificate, transportBuffer);
        if(status == STATUS_OK){
            if (toSign == FALSE && senderCertificate.length > 0){
                // Table 27 part 6: "field shall be null if the Message is not signed"
                status = STATUS_INVALID_RCV_PARAMETER;
            }else if(toSign != FALSE){
                // Check certificate is the same as the one in memory
                uint32_t otherAppCertComparison = 0;
                status = ByteString_Compare(&scConnection->otherAppCertificate,
                                            &senderCertificate,
                                            &otherAppCertComparison);

                if(status != STATUS_OK || otherAppCertComparison != 0){
                    status = STATUS_INVALID_RCV_PARAMETER;
                }

                if(status == STATUS_OK && validateSenderCert != FALSE){
                    void* certStore = NULL;
                    certStore = PKIProvider_Open_Cert_Store(pkiProvider);
                    if(certStore != NULL){
                        status = PKIProvider_Validate_Certificate(pkiProvider,
                                                                  &senderCertificate,
                                                                  certStore,
                                                                  &validationStatusCode);
                        PKIProvider_Close_Cert_Store(pkiProvider, &certStore);
                    }
                    if(status != STATUS_OK){
                        // TODO: report validation status code
                    }
                }
            }
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){
        status = ByteString_Read(&receiverCertThumb, transportBuffer);

        if(status == STATUS_OK){
            if(toEncrypt == FALSE && receiverCertThumb.length > 0){
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                status =STATUS_INVALID_RCV_PARAMETER;
            }else if(toEncrypt != FALSE){
                // Check thumbprint matches current app certificate thumbprint

                UA_ByteString curAppCertThumbprint;
                int32_t thumbprintLength = 0;
                uint32_t runningAppCertComparison = 0;

                status = CryptoProvider_GetCertThumbprintLength(scConnection->currentCryptoProvider,
                                                                &scConnection->runningAppCertificate,
                                                                &thumbprintLength);
                if(status == STATUS_OK){
                    if(thumbprintLength == receiverCertThumb.length){
                        status = ByteString_InitializeFixedSize(&curAppCertThumbprint, thumbprintLength);
                        if(status == STATUS_OK){
                            status = CryptoProvider_GetCertThumbprint(scConnection->currentCryptoProvider,
                                                                      &scConnection->runningAppCertificate,
                                                                      &curAppCertThumbprint);

                            if(status == STATUS_OK){
                                status = ByteString_Compare(&curAppCertThumbprint,
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

                ByteString_Clear(&curAppCertThumbprint);

            } // if toEncrypt
            // Set the sequence number position which is the next position to read
            //  since whole asymmetric security header was read
            *sequenceNumberPosition = transportBuffer->buffers->position;
        } // if decoded thumbprint
    }

    ByteString_Clear(&securityPolicy);
    ByteString_Clear(&senderCertificate);
    ByteString_Clear(&receiverCertThumb);

    return status;
}

StatusCode SC_IsPrecedentCryptoData(SC_Connection* scConnection,
                                    uint32_t       receivedTokenId,
                                    uint32_t*      isPrecCryptoData){
    StatusCode status = STATUS_INVALID_PARAMETERS;
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

StatusCode SC_DecryptMsg(SC_Connection* scConnection,
                         UA_MsgBuffer*  transportBuffer,
                         uint32_t       sequenceNumberPosition,
                         uint32_t       isSymmetric,
                         uint32_t       isPrecCryptoData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toDecrypt = 1;
    uint32_t decryptedTextLength = 0;
    CryptoProvider* cryptoProvider = NULL;
    UA_MessageSecurityMode securityMode = UA_MessageSecurityMode_Invalid;
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
        UA_Byte* dataToDecrypt = &(transportBuffer->buffers->data[sequenceNumberPosition]);
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

StatusCode SC_DecodeMsgBody(UA_MsgBuffer*       receptionBuffer,
                            UA_NamespaceTable*  namespaceTable,
                            UA_EncodeableType** knownTypes,
                            UA_EncodeableType*  respEncType,
                            UA_EncodeableType*  errEncType,
                            UA_EncodeableType** receivedEncType,
                            void**              encodeableObj)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_EncodeableType* recEncType;
    UA_NodeId nodeId;
    const char* nsName;
    uint16_t nsIndex = 0;
    NodeId_Initialize(&nodeId);
    if(receptionBuffer != NULL && namespaceTable != NULL && encodeableObj != NULL &&
       (knownTypes != NULL || respEncType != NULL))
    {
        status = NodeId_Read(&nodeId, receptionBuffer);
    }

    if(status == STATUS_OK && nodeId.identifierType == UA_IdType_Numeric){

        if(respEncType != NULL){
            // Case in which we know the expected type from the request Id
            if (nodeId.numeric == respEncType->typeId || nodeId.numeric == respEncType->binaryTypeId){
    //          || nodeId.numeric == respEncType->xmlTypeId){ => what is the point to accept this type ?
                *receivedEncType = respEncType;
            }else if(errEncType != NULL &&
                     (nodeId.numeric == errEncType->typeId || nodeId.numeric == errEncType->binaryTypeId)){
    //               || nodeId.numeric == errEncType->xmlTypeId){ => what is the point to accept this type ?
                *receivedEncType = errEncType;
            }else{
                status = STATUS_INVALID_RCV_PARAMETER;
            }

            // Check namespace of received type using index
            if(status == STATUS_OK){
                    recEncType = *receivedEncType;
                    if(recEncType->namespace == NULL && nodeId.namespace != OPCUA_NAMESPACE_INDEX){
                        status = STATUS_INVALID_RCV_PARAMETER;
                    }else if(recEncType->namespace != NULL){
                        status = Namespace_GetIndex(namespaceTable,
                                                    recEncType->namespace,
                                                    &nsIndex);
                        if(status == STATUS_OK){
                            if(nodeId.namespace != nsIndex){
                                status = STATUS_INVALID_RCV_PARAMETER;
                            }
                        }
                    }
                }

        }else{
            // Must be the case in which we cannot know the type before decoding it
            if(nodeId.namespace == OPCUA_NAMESPACE_INDEX){
                EncodeableType_GetEncodeableType(knownTypes, OPCUA_NAMESPACE_NAME, nodeId.numeric);
            }else{
                nsName = Namespace_GetName(namespaceTable, nodeId.namespace);
                if(nsName != NULL){
                    recEncType = EncodeableType_GetEncodeableType(knownTypes, nsName, nodeId.numeric);
                }
                if(recEncType == NULL){
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            }
        }


    }else if(status == STATUS_OK){
        status = STATUS_INVALID_RCV_PARAMETER;
    }

    if(status == STATUS_OK){
        *encodeableObj = malloc(recEncType->allocSize);
        if(*encodeableObj != NULL){
            status = recEncType->decodeFunction(*encodeableObj, receptionBuffer);
        }else{
            status = STATUS_NOK;
        }
    }
    NodeId_Clear(&nodeId);
    return status;
}

StatusCode SC_VerifyMsgSignature(SC_Connection* scConnection,
                                 uint32_t       isSymmetric,
                                 uint32_t       isPrecCryptoData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toVerify = 1;

    CryptoProvider* cryptoProvider = NULL;
    UA_MessageSecurityMode securityMode = UA_MessageSecurityMode_Invalid;
    UA_String* securityPolicy = NULL;
    Buffer* receptionBuffer = MsgBuffers_GetCurrentChunk(scConnection->receptionBuffers);

    uint32_t signatureSize = 0;
    uint32_t signaturePosition = 0;

    if(scConnection != NULL){
        status = STATUS_OK;

        if(isPrecCryptoData == FALSE){
            cryptoProvider = scConnection->currentCryptoProvider;
            securityMode = scConnection->currentSecuMode;
            securityPolicy = &scConnection->currentSecuPolicy;
        }else{
            cryptoProvider = scConnection->precCryptoProvider;
            securityMode = scConnection->precSecuMode;
            securityPolicy = &scConnection->precSecuPolicy;
        }

        toVerify = IsMsgSigned(securityMode);
    }

    if(toVerify != FALSE){
        if(isSymmetric == FALSE){
            uint32_t publicKeyModulusLength = 0;
            UA_ByteString* publicKey = NULL;
            // TODO: isn't this already done?
            SC_RetrieveAndSetPublicKeyFromCert(scConnection, FALSE, &publicKey);

            // TODO: use CryptoProvider_AsymmetricGetLength_Msgs or CryptoProvider_AsymmetricGetLength_MsgPlainText instead, if possible.
            status = CryptoProvider_GetAsymmPublicKeyModulusLength(cryptoProvider,
                                                                   publicKey,
                                                                   &publicKeyModulusLength);

            if(status == STATUS_OK){
                // TODO: use CryptoProvider_AsymmetricGetLength_Signature
                signatureSize = GetAsymmSignatureSize
                                 (securityPolicy,
                                  publicKeyModulusLength);
                signaturePosition = receptionBuffer->length - signatureSize;

                status = CryptoProvider_AsymmetricVerify(cryptoProvider,
                                                         receptionBuffer->data,
                                                         signaturePosition,
                                                         publicKey,
                                                         &(receptionBuffer->data[signaturePosition]),
                                                         signatureSize);
            }
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

StatusCode SC_CheckSeqNumReceived(SC_Connection* scConnection)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t seqNumber = 0;

    if(scConnection != NULL){
        status = STATUS_OK;
        status = UInt32_Read(&seqNumber, scConnection->receptionBuffers);
    }

    if(status == STATUS_OK){
        if(scConnection->receptionBuffers->secureType == UA_OpenSecureChannel){
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
            }
        }
    }

    return status;
}

StatusCode SC_CheckReceivedProtocolVersion(SC_Connection* scConnection,
                                           uint32_t       scProtocolVersion)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
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

StatusCode SC_EncodeSecureMessage(SC_Connection*     scConnection,
                                  UA_EncodeableType* encType,
                                  void*              value,
                                  uint32_t           requestId)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_MsgBuffer* msgBuffer = scConnection->sendingBuffer;
    if(scConnection != NULL &&
       encType != NULL &&
       value != NULL)
    {
        // Encode secure message header:
        status = SC_EncodeSecureMsgHeader(msgBuffer,
                                          UA_SecureMessage,
                                          scConnection->currentSecuToken.channelId);
    }

    if(status == STATUS_OK){
        // Encode symmetric security header
        status = UInt32_Write(&scConnection->currentSecuToken.tokenId, msgBuffer);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSequenceHeader(msgBuffer, requestId);
    }

    if(status == STATUS_OK){
        status = SC_EncodeMsgBody(msgBuffer, encType, value);
    }

    return status;

}

StatusCode SC_DecodeSymmSecurityHeader(UA_MsgBuffer* transportBuffer,
                                       uint32_t*     tokenId,
                                       uint32_t*     snPosition)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(transportBuffer != NULL)
    {
        status = UInt32_Read(tokenId, transportBuffer);
    }
    if(status == STATUS_OK){
        *snPosition = transportBuffer->buffers->position;
    }
    return status;
}

StatusCode SC_RemovePaddingAndSig(SC_Connection* scConnection,
                                  uint32_t       isPrecCryptoData)
{
    // Only valid for symmetric encryption ! No need for asymm (1 chunk maximum)
    StatusCode status = STATUS_INVALID_PARAMETERS;
    CryptoProvider* cProvider = NULL;
    uint32_t sigSize = 0;
    uint32_t cipherBlockSize = 0;
    uint32_t plainBlockSize = 0;
    uint16_t padding = 0;
    Buffer* curChunk = MsgBuffers_GetCurrentChunk(scConnection->receptionBuffers);
    uint32_t newBufferLength = curChunk->length;
    if(scConnection != NULL){
        if(isPrecCryptoData == FALSE){
            cProvider = scConnection->currentCryptoProvider;
        }else{
            cProvider = scConnection->precCryptoProvider;
        }
        // Compute signature size and remove from buffer length
        status = CryptoProvider_SymmetricGetLength_Signature(cProvider,
                                                             &sigSize);
        if(status == STATUS_OK){
            newBufferLength = newBufferLength - sigSize;
            status = CryptoProvider_SymmetricGetLength_Blocks(cProvider,
                                                              &cipherBlockSize,
                                                              &plainBlockSize);
        }
    }

    if(status == STATUS_OK){
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

        // Set new buffer length without padding and signature
        status = Buffer_SetDataLength(curChunk, newBufferLength);
    }


    return status;
}

StatusCode SC_DecryptSecureMessage(SC_Connection* scConnection,
                                   UA_MsgBuffer*  transportMsgBuffer,
                                   uint32_t*      requestId)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
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
        status = UInt32_Read(requestId, scConnection->receptionBuffers);
    }

    if(status == STATUS_OK){
        status = SC_RemovePaddingAndSig(scConnection,
                                        isPrecCryptoDataFalse);
    }

    return status;
}

StatusCode SC_CheckPrecChunk(UA_MsgBuffers* msgBuffer,
                             uint32_t       requestId,
                             uint8_t*       abortReqPresence,
                             uint32_t*      abortReqId)
{
    assert(msgBuffer != NULL);
    StatusCode status = STATUS_INVALID_PARAMETERS;

    if(msgBuffer != NULL &&
       abortReqPresence != NULL && abortReqId != NULL)
    {
        // Check if we received a new request id related message before
        //  precedent treatment end
        if(msgBuffer->nbChunks > 1 &&
           msgBuffer->receivedReqId != requestId)
        {
            *abortReqPresence = 1;
            *abortReqId = msgBuffer->receivedReqId;
            status = MsgBuffers_SetCurrentChunkFirst(msgBuffer);
        }else{
            *abortReqPresence = FALSE;
            status = STATUS_OK;
        }
    }
    return status;
}

StatusCode SC_CheckAbortChunk(UA_MsgBuffers* msgBuffer,
                              UA_String*     reason){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t errorCode = 0;
    if(msgBuffer != NULL && reason != NULL){
        if(msgBuffer->isFinal == UA_Msg_Chunk_Abort){
            status = UInt32_Read(&errorCode, msgBuffer);
            if(status == STATUS_OK){
                status = errorCode;
                String_Read(reason, msgBuffer);
            }

        }else{
            status = STATUS_OK;
        }
    }
    return status;
}

StatusCode SC_DecodeChunk(UA_MsgBuffers*      msgBuffers,
                          uint32_t            requestId,
                          UA_EncodeableType*  expEncType,
                          UA_EncodeableType*  errEncType,
                          UA_EncodeableType** recEncType,
                          void**              encObj){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t msgSize = 0;
    Buffer* tmpBuffer = NULL;
    uint32_t idx = 0;
    UA_MsgBuffer* tmpMsgBuffer = NULL;
    Buffer* buffer = NULL;

    if(msgBuffers != NULL &&
       recEncType != NULL && encObj != NULL)
    {

        switch(msgBuffers->isFinal){
            case UA_Msg_Chunk_Final:
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
            case UA_Msg_Chunk_Intermediate:
                assert(msgBuffers->nbChunks >= 1);
                if(msgBuffers->nbChunks == 1){
                    msgBuffers->receivedReqId = requestId;
                }else if(msgBuffers->receivedReqId != requestId){
                    status = STATUS_NOK;
                    assert(FALSE);
                }
                break;
            case UA_Msg_Chunk_Abort:
                status = STATUS_NOK;
                assert(FALSE);
                break;
            case UA_Msg_Chunk_Unknown:
            case UA_Msg_Chunk_Invalid:
                status = STATUS_NOK;
                assert(FALSE);
        }
    }

    return status;
}

