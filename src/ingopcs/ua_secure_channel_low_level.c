/*
 * secure_channel_low_level.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */

#include <wrappers.h>

#include <assert.h>
#include <stdlib.h>
#include <ua_encoder.h>
#include <ua_tcp_ua_low_level.h>
#include <ua_secure_channel_low_level.h>

const uint32_t scProtocolVersion = 0;

UA_String* UA_String_Security_Policy_None;
UA_String* UA_String_Security_Policy_Basic128Rsa15;
UA_String* UA_String_Security_Policy_Basic256;
UA_String* UA_String_Security_Policy_Basic256Sha256;

SC_Connection* SC_Create (){
    SC_Connection* sConnection = UA_NULL;
    TCP_UA_Connection* connection = TCP_UA_Connection_Create(scProtocolVersion);
    UA_String_Security_Policy_None =
     String_CreateFromCString(SECURITY_POLICY_NONE);
    UA_String_Security_Policy_Basic128Rsa15 =
     String_CreateFromCString(SECURITY_POLICY_BASIC128RSA15);
    UA_String_Security_Policy_Basic256 =
     String_CreateFromCString(SECURITY_POLICY_BASIC256);
    UA_String_Security_Policy_Basic256Sha256 =
     String_CreateFromCString(SECURITY_POLICY_BASIC256SHA256);


    if(connection != UA_NULL){
        sConnection = (SC_Connection *) malloc(sizeof(SC_Connection));

        if(sConnection != 0){
            memset (sConnection, 0, sizeof(SC_Connection));
            sConnection->state = SC_Connection_Error;
            sConnection->transportConnection = connection;

        }else{
            TCP_UA_Connection_Delete(connection);
        }
    }
    return sConnection;
}

void SC_Delete (SC_Connection* scConnection){
    String_Clear(UA_String_Security_Policy_None);
    String_Clear(UA_String_Security_Policy_Basic128Rsa15);
    String_Clear(UA_String_Security_Policy_Basic256);
    String_Clear(UA_String_Security_Policy_Basic256Sha256);
    if(scConnection != UA_NULL){
        // Do not delete runningAppCertificate, runnigAppPrivateKey and otherAppCertificate:
        //  managed by upper level
        if(scConnection->runningAppPublicKey != UA_NULL)
        {
            ByteString_Clear(scConnection->runningAppPublicKey);
        }
        if(scConnection->otherAppPublicKey != UA_NULL)
        {
            ByteString_Clear(scConnection->otherAppPublicKey);
        }
        if(scConnection->sendingBuffer != UA_NULL)
        {
            MsgBuffer_Delete(&scConnection->sendingBuffer);
        }
        if(scConnection->receptionBuffers != UA_NULL)
        {
            MsgBuffers_Delete(&scConnection->receptionBuffers);
        }
        if(scConnection->transportConnection != UA_NULL)
        {
            TCP_UA_Connection_Delete(scConnection->transportConnection);
        }
        if(scConnection->currentNonce != UA_NULL)
        {
            PrivateKey_Delete(scConnection->currentNonce);
        }
        String_Clear(scConnection->currentSecuPolicy);
        CryptoProvider_Delete(scConnection->currentCryptoProvider);
        CryptoProvider_Delete(scConnection->precCryptoProvider);
        free(scConnection);
    }
}

StatusCode SC_InitApplicationIdentities(SC_Connection* scConnection,
                                            UA_ByteString* runningAppCertificate,
                                            PrivateKey*    runningAppPrivateKey,
                                            UA_ByteString* otherAppCertificate){
    StatusCode status = STATUS_OK;
    if(scConnection->runningAppCertificate == UA_NULL &&
       scConnection->runningAppPrivateKey == UA_NULL &&
       scConnection->otherAppCertificate == UA_NULL)
    {
        scConnection->runningAppCertificate = runningAppCertificate;
        ByteString_Clear(scConnection->runningAppPublicKey);
        scConnection->runningAppPublicKey = UA_NULL;
        scConnection->runningAppPrivateKey = runningAppPrivateKey;
        scConnection->otherAppCertificate = otherAppCertificate;
        ByteString_Clear(scConnection->otherAppPublicKey);
        scConnection->otherAppPublicKey = UA_NULL;
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
    if(scConnection->receptionBuffers == UA_NULL){
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
            assert(UA_FALSE);
        }
        if(scConnection->receptionBuffers == UA_NULL){
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
    if(scConnection->sendingBuffer == UA_NULL){
        Buffer* buf = Buffer_Create(scConnection->transportConnection->sendBufferSize);
        msgBuffer = MsgBuffer_Create(buf,
                                     scConnection->transportConnection->maxChunkCountSnd,
                                     scConnection,
                                     namespaceTable,
                                     encodeableTypes);
        if(msgBuffer != UA_NULL){
            scConnection->sendingBuffer = msgBuffer;
            status = STATUS_OK;
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

// Retrieve public key from certificate and set it in internal properties
StatusCode Retrieve_Public_Key_From_Cert(SC_Connection*  scConnection,
                                         uint32_t        runningApp,
                                         UA_ByteString** publicKey)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_ByteString* certificate = UA_NULL;
    uint32_t publicKeyLength = 0;

    if(scConnection != UA_NULL && *publicKey == UA_NULL){
        status = STATUS_OK;
    }

    if(runningApp == UA_FALSE)
    {
        if(scConnection->otherAppPublicKey == UA_NULL){
            certificate = scConnection->otherAppCertificate;
        }else{
            *publicKey = scConnection->otherAppPublicKey;
        }
    }else{
        if(scConnection->runningAppPublicKey == UA_NULL){
            certificate = scConnection->runningAppCertificate;
        }else{
            *publicKey = scConnection->runningAppPublicKey;
        }
    }

    if(*publicKey == UA_NULL && certificate != UA_NULL)
    {
        status = CryptoProvider_GetPublicKeyLengthFromCert(scConnection->currentCryptoProvider,
                                                  certificate,
                                                  &publicKeyLength);
        if(status == STATUS_OK){
            *publicKey = ByteString_CreateFixedSize(publicKeyLength);
        }

        if(*publicKey != UA_NULL){
            status = CryptoProvider_GetPublicKeyFromCert(scConnection->currentCryptoProvider,
                                                          certificate,
                                                          *publicKey);
        }else{
            status = STATUS_NOK;
        }

        if(status == STATUS_OK && runningApp == UA_FALSE){
            scConnection->otherAppPublicKey = *publicKey;
        }else if (status == STATUS_OK && runningApp != UA_FALSE){
            scConnection->runningAppPublicKey = *publicKey;
        }

    }else if(*publicKey == UA_NULL){
        status = STATUS_NOK;
    }

    return status;
}

//// Cryptographic properties helpers

StatusCode GetAsymmBlocksSizes(UA_String* securityPolicyUri,
                               uint32_t   keySize,
                               uint32_t*  cipherTextBlockSize,
                               uint32_t*  plainTextBlockSize){
    StatusCode status = STATUS_OK;
    const uint32_t sha1outputLength = 20; // bytes (160 bits)

    if(String_Equal(securityPolicyUri, UA_String_Security_Policy_None) != UA_FALSE)
    {
        *cipherTextBlockSize = 1;
        *plainTextBlockSize = 1;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic128Rsa15) != UA_FALSE){
        // RSA 1.5: RSA spec 7.2.1 (https://www.ietf.org/rfc/rfc2437.txt)
        *cipherTextBlockSize = keySize;
        *plainTextBlockSize = keySize - 11;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256) != UA_FALSE){
        // RSA OAEP: RSA spec 7.1.1 (https://www.ietf.org/rfc/rfc2437.txt)
        // + RSA spec 10.1 : "For the EME-OAEP encoding method, only SHA-1 is recommended."
        *cipherTextBlockSize = keySize;
        *plainTextBlockSize = keySize - 2 -2*sha1outputLength;
   }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256Sha256) != UA_FALSE){
       // RSA OAEP: RSA spec 7.1.1 (https://www.ietf.org/rfc/rfc2437.txt)
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
    if(String_Equal(securityPolicyUri, UA_String_Security_Policy_None) != UA_FALSE)
    {
        signatureSize = 0;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic128Rsa15) != UA_FALSE){
        // Regarding RSA spec 5.2 (https://www.ietf.org/rfc/rfc2437.txt), signature size = key size
        signatureSize = privateKeySize;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256) != UA_FALSE){
        // Regarding RSA spec 5.2 (https://www.ietf.org/rfc/rfc2437.txt), signature size = key size
        signatureSize = privateKeySize;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256Sha256) != UA_FALSE){
        // Regarding RSA spec 5.2 (https://www.ietf.org/rfc/rfc2437.txt), signature size = key size
        signatureSize = privateKeySize;
    }
    return signatureSize;
}

StatusCode Get_Symmetric_Algorithm_Blocks_Sizes(UA_String* securityPolicyUri,
                                                uint32_t*  cipherTextBlockSize,
                                                uint32_t*  plainTextBlockSize){
    StatusCode status = STATUS_NOK;

    if(String_Equal(securityPolicyUri, UA_String_Security_Policy_None) != UA_FALSE)
    {
        *cipherTextBlockSize = 1;
        *plainTextBlockSize = 1;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic128Rsa15) != UA_FALSE){
        // AES: 128 bits block size
        *cipherTextBlockSize = 16;
        *plainTextBlockSize = 16;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256) != UA_FALSE){
        // AES: 128 bits block size
        *cipherTextBlockSize = 16;
        *plainTextBlockSize = 16;
   }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256Sha256) != UA_FALSE){
       // AES: 128 bits block size
       *cipherTextBlockSize = 16;
       *plainTextBlockSize = 16;
   }else{
       status = STATUS_INVALID_PARAMETERS;
   }
   return status;
}

uint32_t Get_Symmetric_Signature_Size(UA_String* securityPolicyUri){
    uint32_t signatureSize = 0;
    if(String_Equal(securityPolicyUri, UA_String_Security_Policy_None) != UA_FALSE)
    {
        signatureSize = 0;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic128Rsa15) != UA_FALSE){
        // Symmetric signature algo is Sha1 => output size is 160 bits = 20 bytes
        signatureSize = 20;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256) != UA_FALSE){
        // Symmetric signature algo is Sha1 => output size is 160 bits = 20 bytes
        signatureSize = 20;
    }else if(String_Equal(securityPolicyUri, UA_String_Security_Policy_Basic256Sha256) != UA_FALSE){
        // Symmetric signature algo is Sha256 => output size is 256 bits = 32 bytes
        signatureSize = 32;
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
    const uint32_t headersSize = msgBuffer->sequenceNumberPosition - 1;
    const uint32_t bodyChunkSize = msgBuffer->buffers->max_size - headersSize;

    // Computed maxBlock and then maxBodySize based on revised formula of mantis ticket #2897
    // Spec 1.03 part 6 incoherent
    const uint32_t maxBlocks = bodyChunkSize / cipherBlockSize;
    // MaxBodySize = unCiphered block size * max blocs - sequence header -1 for PaddingSize field
    return plainBlockSize * maxBlocks - UA_SECURE_MESSAGE_SEQUENCE_LENGTH - signatureSize - 1;
}

// Get information from internal properties

StatusCode GetEncryptedDataLength(SC_Connection* scConnection,
                                  uint32_t       plainDataLength,
                                  uint32_t       symmetricAlgo,
                                  uint32_t*      cipherDataLength)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;

    if(scConnection != UA_NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == UA_FALSE){
        if(scConnection->otherAppCertificate == UA_NULL){
           status = STATUS_INVALID_STATE;
        }else{

            UA_ByteString* otherAppPublicKey = UA_NULL;

            // Retrieve other app public key from certificate
            status = Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &otherAppPublicKey);

            // Retrieve cipher length
            if(status == STATUS_OK){
                status = CryptoProvider_AsymmetricEncryptLength
                          (scConnection->currentCryptoProvider,
                           UA_NULL,
                           plainDataLength,
                           otherAppPublicKey,
                           cipherDataLength);
            }
        }
    }else if (status == STATUS_OK){
        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL){
            status = STATUS_INVALID_STATE;
        }else{
            // Retrieve cipher length
            status = CryptoProvider_SymmetricEncryptLength
                      (scConnection->currentCryptoProvider,
                       UA_NULL,
                       plainDataLength,
                       scConnection->currentSecuKeySets.senderKeySet->encryptKey,
                       scConnection->currentSecuKeySets.senderKeySet->initVector,
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
        toEncrypt = UA_FALSE;
    }

    return toEncrypt;
}

uint32_t IsMsgSigned(UA_MessageSecurityMode securityMode)
{
    uint32_t toSign = 1; // True
    // Determine if the message must be signed
    if(securityMode == UA_MessageSecurityMode_None)
    {
        toSign = UA_FALSE;
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
    if(hasPadding != UA_FALSE){
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

uint16_t GetPaddingSize(uint32_t bytesToWrite,
                        uint32_t plainBlockSize,
                        uint32_t signatureSize){
    return plainBlockSize - ((bytesToWrite + signatureSize + 1) % plainBlockSize);
}

// Set internal properties

StatusCode SC_SetMaxBodySize(SC_Connection* scConnection,
                             uint32_t       isSymmetric){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != UA_NULL){
        uint32_t cipherBlockSize = 0;
        uint32_t plainBlockSize =0;
        if(isSymmetric == UA_FALSE){
            uint32_t publicKeyModulusLength = 0;
            UA_ByteString* publicKey = UA_NULL;
            Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &publicKey);
            status = CryptoProvider_GetAsymmPublicKeyModulusLength(scConnection->currentCryptoProvider,
                                                                   publicKey,
                                                                   &publicKeyModulusLength);

            if(status == STATUS_OK){
                status = GetAsymmBlocksSizes(scConnection->currentSecuPolicy,
                                             publicKeyModulusLength,
                                             &cipherBlockSize,
                                             &plainBlockSize);
                 if(status == STATUS_OK){
                     scConnection->sendingMaxBodySize = GetMaxBodySize(scConnection->sendingBuffer,
                                                                       cipherBlockSize,
                                                                       plainBlockSize,
                                                                       GetAsymmSignatureSize
                                                                        (scConnection->currentSecuPolicy,
                                                                         publicKeyModulusLength));
                 }
            }else{
                status = STATUS_NOK;
            }
        }else{
            status = Get_Symmetric_Algorithm_Blocks_Sizes(scConnection->currentSecuPolicy,
                                                          &cipherBlockSize,
                                                          &plainBlockSize);
             if(status == STATUS_OK){
                 scConnection->sendingMaxBodySize = GetMaxBodySize(scConnection->sendingBuffer,
                                                                   cipherBlockSize,
                                                                   plainBlockSize,
                                                                   Get_Symmetric_Signature_Size
                                                                    (scConnection->currentSecuPolicy));
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
    if(msgBuffer != UA_NULL){
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
            status = UInt32_Write(msgBuffer, &msgHeaderLength);
        }
        if(status == STATUS_OK){
            // Secure channel Id
            status = UInt32_Write(msgBuffer, &secureChannelId);
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
    if(msgBuffer != UA_NULL){
        status = STATUS_OK;
    }

    // Set temporary SN value: to be set on message sending (ensure contiguous SNs)
    if(status == STATUS_OK){
        msgBuffer->sequenceNumberPosition = msgBuffer->buffers->position;
        UInt32_Write(msgBuffer, &zero);
    }

    if(status == STATUS_OK){
        UInt32_Write(msgBuffer, &requestId);
    }

    return status;
}

StatusCode SC_EncodeAsymmSecurityHeader(SC_Connection* scConnection,
                                        UA_String*     securityPolicy,
                                        UA_ByteString* senderCertificate,
                                        UA_ByteString* receiverCertificate){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True
    if(scConnection != UA_NULL && scConnection->sendingBuffer != UA_NULL &&
       //cryptoProvider != UA_NULL &&
       securityPolicy != UA_NULL &&
       senderCertificate != UA_NULL &&
       receiverCertificate != UA_NULL)
    {
        toEncrypt = IsMsgEncrypted(scConnection->currentSecuMode, scConnection->sendingBuffer);
        toSign = IsMsgSigned(scConnection->currentSecuMode);
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        if(securityPolicy->length>0){
            status = String_Write(scConnection->sendingBuffer, securityPolicy);
        }else{
            // Null security policy is invalid parameter since unspecified
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    // Sender Certificate:
    if(status == STATUS_OK){
        if(toSign != UA_FALSE && senderCertificate->length>0){ // Field shall be null if message not signed
            status = String_Write(scConnection->sendingBuffer, senderCertificate);
        }else{
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            // status = Write_Int32(scConnection->sendingBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            const int32_t minusOne = -1;
            status = Int32_Write(scConnection->sendingBuffer, &minusOne);
            // NULL string: nothing to write
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){

        UA_ByteString* recCertThumbprint = UA_NULL;
        if(toEncrypt != UA_FALSE){
            int32_t thumbprintLength = 0;
            status = CryptoProvider_GetCertThumbprintLength(scConnection->currentCryptoProvider,
                                                            receiverCertificate,
                                                            &thumbprintLength);
            if(status == STATUS_OK){
                recCertThumbprint = ByteString_CreateFixedSize(thumbprintLength);
                if(recCertThumbprint != UA_NULL){
                    status = CryptoProvider_GetCertThumbprint(scConnection->currentCryptoProvider,
                                                              receiverCertificate,
                                                              recCertThumbprint);
                }else{
                    status = STATUS_NOK;
                }
            }

            status = String_Write(scConnection->sendingBuffer, recCertThumbprint);
        }else{
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            //status = Write_Int32(scConnection->sendingBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            const int32_t minusOne = -1;
            status = Int32_Write(scConnection->sendingBuffer, &minusOne);
            // NULL string: nothing to write
        }

        ByteString_Clear(recCertThumbprint);
    }else{
        status = STATUS_NOK;
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

    if(msgBuffer != UA_NULL && msgBody != UA_NULL &&
       encType != UA_NULL){
        nodeId.identifierType = IdentifierType_Numeric;
        if(encType->namespace == UA_NULL){
            nodeId.namespace = 0;
        }else{
            // TODO: find namespace Id
        }
        nodeId.numeric = encType->binaryTypeId;

        status = NodeId_Write(msgBuffer, &nodeId);
    }
    if(status == STATUS_OK){
        status = encType->encodeFunction(msgBuffer, msgBody);
    }
    return status;
}

StatusCode SC_WriteSecureMsgBuffer(UA_MsgBuffer*  msgBuffer,
                                   const UA_Byte* data_src,
                                   uint32_t       count){
    SC_Connection* scConnection = UA_NULL;
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || msgBuffer == UA_NULL || msgBuffer->flushData == UA_NULL)
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
                uint32_t tmpCount =
                 msgBuffer->sequenceNumberPosition +
                 UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                 scConnection->sendingMaxBodySize - msgBuffer->buffers->position;
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
    if(msgBuffer != UA_NULL && msgLength < msgBuffer->buffers->max_size){
        originPosition = msgBuffer->buffers->position;
        status = Buffer_SetPosition(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(msgBuffer, &msgLength);
    }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(msgBuffer->buffers, originPosition);
        msgBuffer->msgSize = msgLength;
    }
    return status;
}

StatusCode Set_Message_Chunk_Type(UA_MsgBuffer*    msgBuffer,
                                  UA_MsgFinalChunk chunkType){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;

    if(msgBuffer != UA_NULL){
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
    SC_Connection* scConnection = UA_NULL;
    uint32_t originPosition = 0;

    if(msgBuffer != UA_NULL && msgBuffer->flushData != UA_NULL){
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
           UInt32_Write(msgBuffer, &scConnection->lastSeqNumSent);
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
                          uint8_t*      paddingSize,
                          uint8_t*      hasExtraPadding,
                          uint32_t*     signatureSize)
{
    StatusCode status = STATUS_OK;
    uint32_t plainBlockSize = 0;
    uint32_t cipherBlockSize = 0;
    uint32_t encryptKeySize = 0;
    *hasPadding = 1; // True

    if(symmetricAlgo == UA_FALSE){
        if(scConnection->runningAppCertificate == UA_NULL ||
           scConnection->runningAppPrivateKey == UA_NULL ||
           scConnection->otherAppCertificate == UA_NULL){
           status = STATUS_INVALID_STATE;
        }else{

            uint32_t publicKeyModulusLength = 0;
            UA_ByteString* publicKey = UA_NULL;
            Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &publicKey);

            status = CryptoProvider_GetAsymmPublicKeyModulusLength(scConnection->currentCryptoProvider,
                                                                   publicKey,
                                                                   &publicKeyModulusLength);

            if(status == STATUS_OK){
                *signatureSize = GetAsymmSignatureSize
                                  (scConnection->currentSecuPolicy,
                                   publicKeyModulusLength);
                status = GetAsymmBlocksSizes
                          (scConnection->currentSecuPolicy,
                           publicKeyModulusLength,
                           &cipherBlockSize,
                           &plainBlockSize);
            }

        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == UA_NULL){
            status = STATUS_INVALID_STATE;
        }else{
            *signatureSize = Get_Symmetric_Signature_Size(scConnection->currentSecuPolicy);
            // Here public key is the signing key
            encryptKeySize =
             PrivateKey_GetSize
              (scConnection->currentSecuKeySets.senderKeySet->encryptKey);
            status = GetAsymmBlocksSizes
                      (scConnection->currentSecuPolicy,
                       encryptKeySize,
                       &cipherBlockSize,
                       &plainBlockSize);
        }
    }

    if(status == STATUS_OK){
        uint16_t padding = GetPaddingSize
                            (msgBuffer->buffers->length -
                              msgBuffer->sequenceNumberPosition +
                              UA_SECURE_MESSAGE_SEQUENCE_LENGTH,
                             plainBlockSize,
                             *signatureSize);
        //Little endian conversion of padding:
        EncodeDecode_UInt16(&padding);
        status = Buffer_Write(msgBuffer->buffers, (UA_Byte*) &padding, 1);
        *paddingSize = 0xFF & padding;

        if(status == STATUS_OK){
            // The value of each byte of the padding is equal to paddingSize:
            UA_Byte paddingBytes[*paddingSize];
            memset(paddingBytes, *paddingSize, *paddingSize);
            status = Buffer_Write(msgBuffer->buffers, paddingBytes, *paddingSize);
        }

        // Extra-padding necessary if
        if(status == STATUS_OK && encryptKeySize > 256){
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            UA_Byte extraPadding = 0x00FF & padding;
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
    if(symmetricAlgo == UA_FALSE){
        if(scConnection->runningAppCertificate == UA_NULL ||
           scConnection->runningAppPrivateKey == UA_NULL ||
           scConnection->otherAppCertificate == UA_NULL){
           status = STATUS_INVALID_STATE;
        }else{
            UA_ByteString* signedData = ByteString_CreateFixedSize(signatureSize);
            if(signedData != UA_NULL){
                status = CryptoProvider_AsymmetricSign
                          (scConnection->currentCryptoProvider,
                           msgBuffer->buffers->data,
                           msgBuffer->buffers->length,
                           scConnection->runningAppPrivateKey,
                           signedData);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers,
                                      signedData->characters,
                                      signedData->length);
            }
            ByteString_Clear(signedData);
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == UA_NULL){
            status = STATUS_INVALID_STATE;
        }else{
            UA_ByteString* signedData = ByteString_CreateFixedSize(signatureSize);
            if(signedData != UA_NULL){
                status = CryptoProvider_SymmetricSign
                          (scConnection->currentCryptoProvider,
                           msgBuffer->buffers->data,
                           msgBuffer->buffers->length,
                           scConnection->currentSecuKeySets.senderKeySet->signKey,
                           signedData);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Buffer_Write(msgBuffer->buffers,
                                      signedData->characters,
                                      signedData->length);
            }
            ByteString_Clear(signedData);
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

    if(scConnection != UA_NULL && msgBuffer != UA_NULL &&
       encryptedMsgBuffer != UA_NULL && encryptedMsgBuffer->buffers != UA_NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == UA_FALSE){
        if(scConnection->runningAppCertificate == UA_NULL ||
           scConnection->runningAppPrivateKey == UA_NULL ||
           scConnection->otherAppCertificate == UA_NULL)
        {
           status = STATUS_INVALID_STATE;
        }else{
            UA_ByteString* otherAppPublicKey = UA_NULL;
            UA_Byte* encryptedData = UA_NULL;

            if(status == STATUS_OK){
                // Retrieve other app public key from certificate
                status = Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &otherAppPublicKey);
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
                if(encryptedData == UA_NULL)
                {
                    status = STATUS_NOK;
                }else{
                    // Copy non encrypted headers part
                    memcpy(encryptedData, msgBuffer->buffers->data, msgBuffer->sequenceNumberPosition);
                    // Set correct message size and encrypted buffer length
                    Buffer_SetDataLength(encryptedMsgBuffer->buffers,
                                           msgBuffer->sequenceNumberPosition + encryptedDataLength);
                    encryptedMsgBuffer->msgSize = msgBuffer->msgSize;
                    // Message size was already encrypted message length, it must be the same now
                    assert(encryptedMsgBuffer->buffers->length == encryptedMsgBuffer->msgSize);
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
        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == UA_NULL){
            status = STATUS_INVALID_STATE;
        }else{
            UA_Byte* encryptedData = UA_NULL;

            // Check size of encrypted data array
            if(status == STATUS_OK){
                if(encryptedMsgBuffer->buffers->max_size <
                    msgBuffer->sequenceNumberPosition + encryptedDataLength){
                    status = STATUS_NOK;
                }
                encryptedData = encryptedMsgBuffer->buffers->data;
                if(encryptedData == UA_NULL){
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
                           &encryptedDataLength);
            }
        } // End valid key set
    }
    return status;
}

StatusCode SC_FlushSecureMsgBuffer(UA_MsgBuffer*     msgBuffer,
                                   UA_MsgFinalChunk  chunkType){
    SC_Connection* scConnection = UA_NULL;
    StatusCode status = STATUS_NOK;
    uint8_t toEncrypt = 1; // True
    uint8_t toSign = 1; // True
    uint8_t symmetricAlgo = 1; // True;
    uint8_t hasPadding = 0;
    uint8_t paddingSize = 0;
    uint8_t hasExtraPadding = 0;
    uint32_t signatureSize = 0;
    uint32_t encryptedLength = 0;

    if(msgBuffer == UA_NULL || msgBuffer->flushData == UA_NULL ||
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
            symmetricAlgo = UA_FALSE;
        }

        //// Encode padding if encrypted message
        if(toEncrypt == UA_FALSE){
            // No padding fields
        }else{
            status = EncodePadding(scConnection,
                                   msgBuffer,
                                   symmetricAlgo,
                                   &hasPadding,
                                   &paddingSize,
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
        if(toSign == UA_FALSE){
            // No signature field
        }else if(status == STATUS_OK){
            status = EncodeSignature(scConnection,
                                     msgBuffer,
                                     symmetricAlgo,
                                     signatureSize);

        }

        //// Check sender certificate size is not bigger than maximum size to be sent
        if(status == STATUS_OK &&
           msgBuffer->secureType == UA_OpenSecureChannel &&
           toSign != UA_FALSE)
        {
            status = CheckMaxSenderCertificateSize(scConnection->runningAppCertificate,
                                                   msgBuffer->buffers->max_size,
                                                   scConnection->currentSecuPolicy,
                                                   hasPadding,
                                                   paddingSize,
                                                   hasExtraPadding,
                                                   signatureSize);
        }

        if(status == STATUS_OK && toEncrypt == UA_FALSE){
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

        if(status == STATUS_OK){
            // TODO: detach transport buffer ?
            status = TCP_UA_FlushMsgBuffer(scConnection->transportConnection->outputMsgBuffer);
        }
    }
    return status;
}

StatusCode SC_DecodeSecureMsgSCid(SC_Connection* scConnection,
                                  UA_MsgBuffer*  transportBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t secureChannelId = 0;
    if(scConnection != UA_NULL &&
       transportBuffer != UA_NULL)
    {
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        //TODO: on server side, randomize secure channel ids (table 26 part 6)!
        status = UInt32_Read(transportBuffer, &secureChannelId);
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

    UA_ByteString* securityPolicy = ByteString_Create();
    UA_ByteString* senderCertificate = ByteString_Create();
    UA_ByteString* receiverCertThumb = ByteString_Create();

    uint32_t validationStatusCode = 0;

    if(scConnection != UA_NULL &&
       transportBuffer != UA_NULL)
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
        status = ByteString_Read(transportBuffer, securityPolicy);

        if(status == STATUS_OK){
            uint32_t secuPolicyComparison = 0;
            status = ByteString_Compare(scConnection->currentSecuPolicy, securityPolicy, &secuPolicyComparison);

            if(status != STATUS_OK || secuPolicyComparison != 0){
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }

    // Sender Certificate:
    if(status == STATUS_OK){
        status = ByteString_Read(transportBuffer, senderCertificate);
        if(status == STATUS_OK){
            if (toSign == UA_FALSE && senderCertificate->length > 0){
                // Table 27 part 6: "field shall be null if the Message is not signed"
                status = STATUS_INVALID_RCV_PARAMETER;
            }else if(toSign != UA_FALSE){
                // Check certificate is the same as the one in memory
                uint32_t otherAppCertComparison = 0;
                status = ByteString_Compare(scConnection->otherAppCertificate,
                                            senderCertificate,
                                            &otherAppCertComparison);

                if(status != STATUS_OK || otherAppCertComparison != 0){
                    status = STATUS_INVALID_RCV_PARAMETER;
                }

                if(status == STATUS_OK && validateSenderCert != UA_FALSE){
                    void* certStore = UA_NULL;
                    certStore = PKIProvider_Open_Cert_Store(pkiProvider);
                    if(certStore != UA_NULL){
                        status = PKIProvider_Validate_Certificate(pkiProvider,
                                                                  senderCertificate,
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
        status = ByteString_Read(transportBuffer, receiverCertThumb);

        if(status == STATUS_OK){
            if(toEncrypt == UA_FALSE && receiverCertThumb->length > 0){
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                status =STATUS_INVALID_RCV_PARAMETER;
            }else if(toEncrypt != UA_FALSE){
                // Check thumbprint matches current app certificate thumbprint

                UA_ByteString* curAppCertThumbprint = UA_NULL;
                int32_t thumbprintLength = 0;
                uint32_t runningAppCertComparison = 0;

                status = CryptoProvider_GetCertThumbprintLength(scConnection->currentCryptoProvider,
                                                                scConnection->runningAppCertificate,
                                                                &thumbprintLength);
                if(status == STATUS_OK){
                    if(thumbprintLength == receiverCertThumb->length){
                        curAppCertThumbprint = ByteString_CreateFixedSize(thumbprintLength);
                        if(curAppCertThumbprint != UA_NULL){
                            status = CryptoProvider_GetCertThumbprint(scConnection->currentCryptoProvider,
                                                                      scConnection->runningAppCertificate,
                                                                      curAppCertThumbprint);

                            if(status == STATUS_OK){
                                status = ByteString_Compare(curAppCertThumbprint,
                                                            receiverCertThumb,
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

                ByteString_Clear(curAppCertThumbprint);

            } // if toEncrypt
            // Set the sequence number position which is the next position to read
            //  since whole asymmetric security header was read
            *sequenceNumberPosition = transportBuffer->buffers->position;
        } // if decoded thumbprint
    }

    ByteString_Clear(securityPolicy);
    ByteString_Clear(senderCertificate);
    ByteString_Clear(receiverCertThumb);

    return status;
}

StatusCode SC_IsPrecedentCryptoData(SC_Connection* scConnection,
                                    uint32_t       receivedTokenId,
                                    uint32_t*      isPrecCryptoData){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != UA_NULL){
        status = STATUS_OK;
        if(scConnection->currentSecuToken.tokenId == receivedTokenId){
            *isPrecCryptoData = UA_FALSE;
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
    CryptoProvider* cryptoProvider = UA_NULL;
    UA_MessageSecurityMode securityMode = UA_MessageSecurityMode_Invalid;
    Buffer* plainBuffer = UA_NULL;
    uint32_t bufferIdx = 0;

    if(scConnection != UA_NULL && transportBuffer != UA_NULL){
        status = STATUS_OK;

        if(isPrecCryptoData == UA_FALSE){
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
    if(toDecrypt != UA_FALSE){
        // Message is encrypted
        UA_Byte* dataToDecrypt = &(transportBuffer->buffers->data[sequenceNumberPosition]);
        uint32_t lengthToDecrypt = transportBuffer->buffers->length - sequenceNumberPosition;

        if(status == STATUS_OK){
            if(isSymmetric == UA_FALSE){
                status = CryptoProvider_AsymmetricDecryptLength(cryptoProvider,
                                                                dataToDecrypt,
                                                                lengthToDecrypt,
                                                                scConnection->runningAppPrivateKey,
                                                                &decryptedTextLength);

                if(status == STATUS_OK && decryptedTextLength <= scConnection->receptionBuffers->buffers->max_size){
                    // Retrieve next chunk empty buffer
                    plainBuffer = MsgBuffers_NextChunk(scConnection->receptionBuffers, &bufferIdx);
                    if(plainBuffer != UA_NULL){
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

                SC_SecurityKeySet* receiverKeySet = UA_NULL;
                if(isPrecCryptoData == UA_FALSE){
                    receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
                }else{
                    receiverKeySet = scConnection->precSecuKeySets.receiverKeySet;
                }
                status = CryptoProvider_SymmetricDecryptLength(cryptoProvider,
                                                               dataToDecrypt,
                                                               lengthToDecrypt,
                                                               receiverKeySet->encryptKey,
                                                               receiverKeySet->initVector,
                                                               &decryptedTextLength);

                if(status == STATUS_OK && decryptedTextLength <= scConnection->receptionBuffers->buffers->max_size){
                    // Retrieve next chunk empty buffer
                    plainBuffer = MsgBuffers_NextChunk(scConnection->receptionBuffers, &bufferIdx);

                    if(plainBuffer != UA_NULL){
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

StatusCode SC_DecodeMsgBody(SC_Connection*      scConnection,
                            UA_EncodeableType*  respEncType,
                            UA_EncodeableType*  errEncType,
                            UA_EncodeableType** receivedEncType,
                            void**              encodeableObj)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_EncodeableType* recEncType;
    UA_NodeId nodeId;
    uint16_t nsIndex = 0;
    NodeId_Initialize(&nodeId);
    if(scConnection != UA_NULL && respEncType != UA_NULL && encodeableObj != UA_NULL){
        status = NodeId_Read(scConnection->receptionBuffers, &nodeId);
    }
    if(status == STATUS_OK && nodeId.identifierType == UA_IdType_Numeric){

        if (nodeId.numeric == respEncType->typeId || nodeId.numeric == respEncType->binaryTypeId){
//          || nodeId.numeric == respEncType->xmlTypeId){ => what is the point to accept this type ?
            *receivedEncType = respEncType;
        }else if(nodeId.numeric == errEncType->typeId || nodeId.numeric == errEncType->binaryTypeId){
//               || nodeId.numeric == errEncType->xmlTypeId){ => what is the point to accept this type ?
            *receivedEncType = errEncType;
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }

        if(status == STATUS_OK){
            recEncType = *receivedEncType;
            if(recEncType->namespace == UA_NULL && nodeId.namespace != 0){
                status = STATUS_INVALID_RCV_PARAMETER;
            }else if(recEncType->namespace != UA_NULL){
                status = Namespace_GetIndex(scConnection->receptionBuffers->nsTable,
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
        status = STATUS_INVALID_RCV_PARAMETER;
    }

    if(status == STATUS_OK){
        *encodeableObj = malloc(recEncType->allocSize);
        if(*encodeableObj != UA_NULL){
            status = recEncType->decodeFunction(scConnection->receptionBuffers, *encodeableObj);
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

    CryptoProvider* cryptoProvider = UA_NULL;
    UA_MessageSecurityMode securityMode = UA_MessageSecurityMode_Invalid;
    UA_String* securityPolicy = UA_NULL;
    Buffer* receptionBuffer = MsgBuffers_GetCurrentChunk(scConnection->receptionBuffers);

    uint32_t signatureSize = 0;
    uint32_t signaturePosition = 0;

    if(scConnection != UA_NULL){
        status = STATUS_OK;

        if(isPrecCryptoData == UA_FALSE){
            cryptoProvider = scConnection->currentCryptoProvider;
            securityMode = scConnection->currentSecuMode;
            securityPolicy = scConnection->currentSecuPolicy;
        }else{
            cryptoProvider = scConnection->precCryptoProvider;
            securityMode = scConnection->precSecuMode;
            securityPolicy = scConnection->precSecuPolicy;
        }

        toVerify = IsMsgSigned(securityMode);
    }

    if(toVerify != UA_FALSE){
        if(isSymmetric == UA_FALSE){
            uint32_t publicKeyModulusLength = 0;
            UA_ByteString* publicKey = UA_NULL;
            Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &publicKey);

            status = CryptoProvider_GetAsymmPublicKeyModulusLength(cryptoProvider,
                                                                   publicKey,
                                                                   &publicKeyModulusLength);

            if(status == STATUS_OK){
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
            SC_SecurityKeySet* receiverKeySet = UA_NULL;
            if(isPrecCryptoData == UA_FALSE){
                receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
            }else{
                receiverKeySet = scConnection->precSecuKeySets.receiverKeySet;
            }

            signatureSize = Get_Symmetric_Signature_Size(securityPolicy);
            signaturePosition = receptionBuffer->length - signatureSize;

            status = CryptoProvider_SymmetricVerify(cryptoProvider,
                                                    receptionBuffer->data,
                                                    signaturePosition,
                                                    receiverKeySet->signKey,
                                                    &(receptionBuffer->data[signaturePosition]),
                                                    signatureSize);
        }
    }

    return status;
}

StatusCode SC_CheckSeqNumReceived(SC_Connection* scConnection)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t seqNumber = 0;

    if(scConnection != UA_NULL){
        status = STATUS_OK;
        status = UInt32_Read(scConnection->receptionBuffers, &seqNumber);
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

    if(scConnection != UA_NULL){
        status = STATUS_OK;
        // use Get_Rcv_Protocol_Version and check it is the same as the one received in SC
        if(TCP_UA_Connection_GetReceiveProtocolVersion(scConnection->transportConnection,
                                                       &transportProtocolVersion)
                   != UA_FALSE)
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
    if(scConnection != UA_NULL &&
       encType != UA_NULL &&
       value != UA_NULL)
    {
        // Encode secure message header:
        status = SC_EncodeSecureMsgHeader(msgBuffer,
                                          UA_SecureMessage,
                                          scConnection->currentSecuToken.channelId);
    }

    if(status == STATUS_OK){
        // Encode symmetric security header
        status = UInt32_Write(msgBuffer, &scConnection->currentSecuToken.tokenId);
    }

    if(status == STATUS_OK){
        status = SC_EncodeSequenceHeader(msgBuffer, requestId);
    }

    if(status == STATUS_OK){
        status = SC_EncodeMsgBody(msgBuffer, encType, value);
    }

    return status;

}
