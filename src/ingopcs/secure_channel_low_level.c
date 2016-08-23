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
#include <secure_channel_low_level.h>
#include <tcp_ua_connection.h>
#include <tcp_ua_low_level.h>

UA_String* UA_String_Security_Policy_None;
UA_String* UA_String_Security_Policy_Basic128Rsa15;
UA_String* UA_String_Security_Policy_Basic256;
UA_String* UA_String_Security_Policy_Basic256Sha256;

SecureChannel_Connection* Create_Secure_Connection (){
    SecureChannel_Connection* sConnection = UA_NULL;
    TCP_UA_Connection* connection = Create_Connection();
    UA_String_Security_Policy_None =
     Create_String_From_CString(SECURITY_POLICY_NONE);
    UA_String_Security_Policy_Basic128Rsa15 =
     Create_String_From_CString(SECURITY_POLICY_BASIC128RSA15);
    UA_String_Security_Policy_Basic256 =
     Create_String_From_CString(SECURITY_POLICY_BASIC256);
    UA_String_Security_Policy_Basic256Sha256 =
     Create_String_From_CString(SECURITY_POLICY_BASIC256SHA256);


    if(connection != UA_NULL){
        sConnection = (SecureChannel_Connection *) malloc(sizeof(SecureChannel_Connection));

        if(sConnection != 0){
            memset (sConnection, 0, sizeof(SecureChannel_Connection));
            sConnection->state = SC_Connection_Error;
            sConnection->transportConnection = connection;

        }else{
            Delete_Connection(connection);
        }
    }
    return sConnection;
}

void Delete_Secure_Connection (SecureChannel_Connection* scConnection){
    Delete_String(UA_String_Security_Policy_None);
    Delete_String(UA_String_Security_Policy_Basic128Rsa15);
    Delete_String(UA_String_Security_Policy_Basic256);
    Delete_String(UA_String_Security_Policy_Basic256Sha256);
    if(scConnection != UA_NULL){
        // Do not delete runningAppCertificate, runnigAppPrivateKey and otherAppCertificate:
        //  managed by upper level
        if(scConnection->runningAppPublicKey != UA_NULL){
            Delete_Byte_String(scConnection->runningAppPublicKey);
        }
        if(scConnection->otherAppPublicKey != UA_NULL){
            Delete_Byte_String(scConnection->otherAppPublicKey);
        }
        if(scConnection->sendingBuffer != UA_NULL){
            Delete_Msg_Buffer(&scConnection->sendingBuffer);
        }
        if(scConnection->receptionBuffers != UA_NULL){
            Delete_Msg_Buffers(&scConnection->receptionBuffers);
        }
        if(scConnection->transportConnection != UA_NULL){
            Delete_Connection(scConnection->transportConnection);
        }
        if(scConnection->currentNonce != UA_NULL){
            Delete_Private_Key(scConnection->currentNonce);
        }
        Delete_String(scConnection->currentSecuPolicy);
        Delete_Crypto_Provider(scConnection->currentCryptoProvider);
        Delete_Crypto_Provider(scConnection->precCryptoProvider);
        free(scConnection);
    }
}

StatusCode Initiate_Applications_Identities(SecureChannel_Connection* scConnection,
                                            UA_Byte_String* runningAppCertificate,
                                            Private_Key*    runningAppPrivateKey,
                                            UA_Byte_String* otherAppCertificate){
    StatusCode status = STATUS_OK;
    if(scConnection->runningAppCertificate == UA_NULL &&
       scConnection->runningAppPrivateKey == UA_NULL &&
       scConnection->otherAppCertificate == UA_NULL){
        scConnection->runningAppCertificate = runningAppCertificate;
        Delete_Byte_String(scConnection->runningAppPublicKey);
        scConnection->runningAppPublicKey = UA_NULL;
        scConnection->runningAppPrivateKey = runningAppPrivateKey;
        scConnection->otherAppCertificate = otherAppCertificate;
        Delete_Byte_String(scConnection->otherAppPublicKey);
        scConnection->otherAppPublicKey = UA_NULL;
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

StatusCode Initiate_Receive_Secure_Buffers(SecureChannel_Connection* scConnection){
    StatusCode status = STATUS_INVALID_STATE;
    if(scConnection->receptionBuffers == UA_NULL){
        if(scConnection->transportConnection->maxChunkCountRcv != 0){
            scConnection->receptionBuffers = Create_Msg_Buffers
             (scConnection->transportConnection->maxChunkCountRcv,
              scConnection->transportConnection->receiveBufferSize);

        }else if(scConnection->transportConnection->maxMessageSizeRcv != 0){
            // Is message size including whole message or only body as it is the case in part 4 §5.3 last § ???
            scConnection->receptionBuffers = Create_Msg_Buffers
              (scConnection->transportConnection->maxMessageSizeRcv
               /scConnection->transportConnection->receiveBufferSize,
               scConnection->transportConnection->receiveBufferSize);
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

StatusCode Initiate_Send_Secure_Buffer(SecureChannel_Connection* scConnection){
    StatusCode status = STATUS_NOK;
    UA_Msg_Buffer* msgBuffer;
    if(scConnection->sendingBuffer == UA_NULL){
        Buffer* buf = Create_Buffer(scConnection->transportConnection->sendBufferSize);
        msgBuffer = Create_Msg_Buffer(buf,
                                      scConnection->transportConnection->maxChunkCountSnd,
                                      scConnection);
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
StatusCode Retrieve_Public_Key_From_Cert(SecureChannel_Connection* scConnection,
                                         uint32_t                  runningApp,
                                         UA_Byte_String**          publicKey)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte_String* certificate = UA_NULL;
    uint32_t publicKeyLength = 0;

    if(scConnection != UA_NULL && *publicKey == UA_NULL){
        status = STATUS_OK;
    }

    if(runningApp == UA_FALSE){
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

    if(*publicKey == UA_NULL && certificate != UA_NULL){
        status = Asymmetric_Get_Public_Key_Length(scConnection->currentCryptoProvider,
                                                  certificate,
                                                  &publicKeyLength);
        if(status == STATUS_OK){
            *publicKey = Create_Byte_String_Fixed_Size(publicKeyLength);
        }

        if(*publicKey != UA_NULL){
            status = GetPublicKeyFromCert_Crypto_Provider(scConnection->currentCryptoProvider,
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

StatusCode Get_Asymmetric_Algorithm_Blocks_Sizes(UA_String* securityPolicyUri,
                                                 uint32_t   keySize,
                                                 uint32_t*  cipherTextBlockSize,
                                                 uint32_t*  plainTextBlockSize){
    StatusCode status = STATUS_OK;
    const uint32_t sha1outputLength = 20; // bytes (160 bits)

    if(securityPolicyUri->length == UA_String_Security_Policy_None->length &&
       memcmp(securityPolicyUri->characters,
              UA_String_Security_Policy_None->characters,
              securityPolicyUri->length)
       == 0)
    {
        *cipherTextBlockSize = 1;
        *plainTextBlockSize = 1;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic128Rsa15->length &&
             memcmp(securityPolicyUri->characters,
                     UA_String_Security_Policy_Basic128Rsa15->characters,
                    securityPolicyUri->length)
             == 0)
    {
        // RSA 1.5: RSA spec 7.2.1 (https://www.ietf.org/rfc/rfc2437.txt)
        *cipherTextBlockSize = keySize;
        *plainTextBlockSize = keySize - 11;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
            memcmp(securityPolicyUri->characters,
                    UA_String_Security_Policy_Basic256->characters,
                   securityPolicyUri->length)
            == 0)
   {
        // RSA OAEP: RSA spec 7.1.1 (https://www.ietf.org/rfc/rfc2437.txt)
        // + RSA spec 10.1 : "For the EME-OAEP encoding method, only SHA-1 is recommended."
        *cipherTextBlockSize = keySize;
        *plainTextBlockSize = keySize - 2 -2*sha1outputLength;
   }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
           memcmp(securityPolicyUri->characters,
                   UA_String_Security_Policy_Basic256->characters,
                  securityPolicyUri->length)
           == 0)
   {
       // RSA OAEP: RSA spec 7.1.1 (https://www.ietf.org/rfc/rfc2437.txt)
       // + RSA spec 10.1 : "For the EME-OAEP encoding method, only SHA-1 is recommended."
       *cipherTextBlockSize = keySize;
       *plainTextBlockSize = keySize - 2 -2*sha1outputLength;
   }else{
       status = STATUS_INVALID_PARAMETERS;
   }
   return status;
}

uint32_t Get_Asymmetric_Signature_Size(UA_String* securityPolicyUri,
                                       uint32_t   privateKeySize){
    uint32_t signatureSize = 0;
    if(securityPolicyUri->length == UA_String_Security_Policy_None->length &&
       memcmp(securityPolicyUri->characters,
              UA_String_Security_Policy_None->characters,
              securityPolicyUri->length)
       == 0)
    {
        signatureSize = 0;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic128Rsa15->length &&
             memcmp(securityPolicyUri->characters,
                     UA_String_Security_Policy_Basic128Rsa15->characters,
                    securityPolicyUri->length)
             == 0)
    {
        // Regarding RSA spec 5.2 (https://www.ietf.org/rfc/rfc2437.txt), signature size = key size
        signatureSize = privateKeySize;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
            memcmp(securityPolicyUri->characters,
                    UA_String_Security_Policy_Basic256->characters,
                   securityPolicyUri->length)
            == 0)
    {
        // Regarding RSA spec 5.2 (https://www.ietf.org/rfc/rfc2437.txt), signature size = key size
        signatureSize = privateKeySize;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
           memcmp(securityPolicyUri->characters,
                   UA_String_Security_Policy_Basic256->characters,
                  securityPolicyUri->length)
           == 0)
    {
        // Regarding RSA spec 5.2 (https://www.ietf.org/rfc/rfc2437.txt), signature size = key size
        signatureSize = privateKeySize;
    }
    return signatureSize;
}

StatusCode Get_Symmetric_Algorithm_Blocks_Sizes(UA_String* securityPolicyUri,
                                                uint32_t*  cipherTextBlockSize,
                                                uint32_t*  plainTextBlockSize){
    StatusCode status = STATUS_NOK;

    if(securityPolicyUri->length == UA_String_Security_Policy_None->length &&
       memcmp(securityPolicyUri->characters,
              UA_String_Security_Policy_None->characters,
              securityPolicyUri->length)
       == 0)
    {
        *cipherTextBlockSize = 1;
        *plainTextBlockSize = 1;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic128Rsa15->length &&
             memcmp(securityPolicyUri->characters,
                     UA_String_Security_Policy_Basic128Rsa15->characters,
                    securityPolicyUri->length)
             == 0)
    {
        // AES: 128 bits block size
        *cipherTextBlockSize = 16;
        *plainTextBlockSize = 16;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
            memcmp(securityPolicyUri->characters,
                    UA_String_Security_Policy_Basic256->characters,
                   securityPolicyUri->length)
            == 0)
   {
        // AES: 128 bits block size
        *cipherTextBlockSize = 16;
        *plainTextBlockSize = 16;
   }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
           memcmp(securityPolicyUri->characters,
                   UA_String_Security_Policy_Basic256->characters,
                  securityPolicyUri->length)
           == 0)
   {
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
    if(securityPolicyUri->length == UA_String_Security_Policy_None->length &&
       memcmp(securityPolicyUri->characters,
              UA_String_Security_Policy_None->characters,
              securityPolicyUri->length)
       == 0)
    {
        signatureSize = 0;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic128Rsa15->length &&
             memcmp(securityPolicyUri->characters,
                     UA_String_Security_Policy_Basic128Rsa15->characters,
                    securityPolicyUri->length)
             == 0)
    {
        // Symmetric signature algo is Sha1 => output size is 160 bits = 20 bytes
        signatureSize = 20;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
            memcmp(securityPolicyUri->characters,
                    UA_String_Security_Policy_Basic256->characters,
                   securityPolicyUri->length)
            == 0)
    {
        // Symmetric signature algo is Sha1 => output size is 160 bits = 20 bytes
        signatureSize = 20;
    }else if(securityPolicyUri->length == UA_String_Security_Policy_Basic256->length &&
           memcmp(securityPolicyUri->characters,
                   UA_String_Security_Policy_Basic256->characters,
                  securityPolicyUri->length)
           == 0)
    {
        // Symmetric signature algo is Sha256 => output size is 256 bits = 32 bytes
        signatureSize = 32;
    }
    return signatureSize;
}

uint32_t Get_MaxBodySize(UA_Msg_Buffer* msgBuffer,
                         uint32_t       cipherBlockSize,
                         uint32_t       plainBlockSize,
                         uint32_t       signatureSize){
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

StatusCode Get_Encrypted_Data_Length(SecureChannel_Connection* scConnection,
                                     uint32_t                  plainDataLength,
                                     uint32_t                  symmetricAlgo,
                                     uint32_t*                 cipherDataLength)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;

    if(scConnection != UA_NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK && symmetricAlgo == UA_FALSE){
        if(scConnection->otherAppCertificate == UA_NULL){
           status = STATUS_INVALID_STATE;
        }else{

            UA_Byte_String* otherAppPublicKey = UA_NULL;

            // Retrieve other app public key from certificate
            status = Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &otherAppPublicKey);

            // Retrieve cipher length
            if(status == STATUS_OK){
                status = AsymmetricEncrypt_Length_Crypto_Provider
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
            status = SymmetricEncrypt_Length_Crypto_Provider
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

uint32_t Is_Msg_Encrypted(Msg_Security_Mode securityMode,
                          UA_Msg_Buffer*    msgBuffer)
{
    assert(securityMode != Msg_Security_Mode_Invalid);
    uint32_t toEncrypt = 1; // True
    // Determine if the message must be encrypted
    if(securityMode == Msg_Security_Mode_None ||
       (securityMode == Msg_Security_Mode_Sign &&
        msgBuffer->secureType != UA_OpenSecureChannel))
    {
        toEncrypt = UA_FALSE;
    }

    return toEncrypt;
}

uint32_t Is_Msg_Signed(Msg_Security_Mode securityMode)
{
    uint32_t toSign = 1; // True
    // Determine if the message must be signed
    if(securityMode == Msg_Security_Mode_None)
    {
        toSign = UA_FALSE;
    }
    return toSign;
}

StatusCode Check_Max_Sender_Certificate_Size(Crypto_Provider* cryptoProvider,
                                             UA_Byte_String*  senderCertificate,
                                             uint32_t         messageChunkSize,
                                             UA_String*       securityPolicyUri,
                                             uint8_t          hasPadding,
                                             uint32_t         padding,
                                             uint32_t         extraPadding,
                                             uint32_t         asymmetricSignatureSize){
    StatusCode status = STATUS_NOK;
    uint32_t maxSize = // Fit in a single message chunk with at least 1 byte of body
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

uint16_t Get_Padding_Size(uint32_t bytesToWrite,
                          uint32_t plainBlockSize,
                          uint32_t signatureSize){
    return plainBlockSize - ((bytesToWrite + signatureSize + 1) % plainBlockSize);
}

// Set internal properties

StatusCode Set_MaxBodySize(SecureChannel_Connection* scConnection,
                           uint32_t                  isSymmetric){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != UA_NULL){
        uint32_t cipherBlockSize = 0;
        uint32_t plainBlockSize =0;
        if(isSymmetric == UA_FALSE){
            uint32_t publicKeyModulusLength = 0;
            UA_Byte_String* publicKey = UA_NULL;
            Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &publicKey);
            status = Asymmetric_Get_Public_Key_Modulus_Length(scConnection->currentCryptoProvider,
                                                              publicKey,
                                                              &publicKeyModulusLength);

            if(status == STATUS_OK){
                status = Get_Asymmetric_Algorithm_Blocks_Sizes(scConnection->currentSecuPolicy,
                                                               publicKeyModulusLength,
                                                               &cipherBlockSize,
                                                               &plainBlockSize);
                 if(status == STATUS_OK){
                     scConnection->sendingMaxBodySize = Get_MaxBodySize(scConnection->sendingBuffer,
                                                                        cipherBlockSize,
                                                                        plainBlockSize,
                                                                        Get_Asymmetric_Signature_Size
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
                 scConnection->sendingMaxBodySize = Get_MaxBodySize(scConnection->sendingBuffer,
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

StatusCode Encode_Secure_Message_Header(UA_Msg_Buffer* msgBuffer,
                                        UA_Secure_Message_Type smType,
                                        uint32_t secureChannelId)
{
    StatusCode status = STATUS_NOK;
    UA_Byte fByte = 'F';
    if(msgBuffer != UA_NULL){
        status = STATUS_OK;
        switch(smType){
                case UA_SecureMessage:
                    status = Write_Buffer(msgBuffer->buffers, MSG, 3);
                    break;
                case UA_OpenSecureChannel:
                    status = Write_Buffer(msgBuffer->buffers, OPN, 3);
                    break;
                case UA_CloseSecureChannel:
                    status = Write_Buffer(msgBuffer->buffers, CLO, 3);
                    break;
        }
        status = Set_Secure_Message_Type(msgBuffer, smType);
        if(status == STATUS_OK){
            // Default behavior: final except if too long for UA_SecureMessage only !
            status = Write_Buffer(msgBuffer->buffers, &fByte, 1);
        }
        if(status == STATUS_OK){
            msgBuffer->isFinal = UA_Msg_Chunk_Final;
            // Temporary message size
            status = Write_UInt32(msgBuffer, UA_SECURE_MESSAGE_HEADER_LENGTH);
        }
        if(status == STATUS_OK){
            // Secure channel Id
            status = Write_UInt32(msgBuffer, secureChannelId);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
    }

    return status;
}

StatusCode Encode_Sequence_Header(SecureChannel_Connection* scConnection,
                                  uint32_t*                 requestId){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != UA_NULL &&
       scConnection->sendingBuffer != UA_NULL &&
       requestId != UA_NULL){
        //TODO: check constraints on sequence number ? Modified in next spec ?
        status = STATUS_OK;
    }

    // Set temporary SN value: to be set on message sending (ensure contiguous SNs)
    if(status == STATUS_OK){
        scConnection->sendingBuffer->sequenceNumberPosition = scConnection->sendingBuffer->buffers->position;
        Write_UInt32(scConnection->sendingBuffer, 0);
    }

    *requestId = scConnection->lastRequestIdSent + 1;
    if(*requestId == 0){
        (*requestId)++;
    }
    scConnection->sendingBuffer->requestId = *requestId;
    if(status == STATUS_OK){
        Write_UInt32(scConnection->sendingBuffer, *requestId);
    }
    scConnection->lastRequestIdSent = *requestId;

    return status;
}

StatusCode Encode_Asymmetric_Security_Header(SecureChannel_Connection* scConnection,
                                             UA_String*                securityPolicy,
                                             UA_Byte_String*           senderCertificate,
                                             UA_Byte_String*           receiverCertificate){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True
    if(scConnection != UA_NULL && scConnection->sendingBuffer != UA_NULL &&
       //cryptoProvider != UA_NULL &&
       securityPolicy != UA_NULL &&
       senderCertificate != UA_NULL &&
       receiverCertificate != UA_NULL)
    {
        toEncrypt = Is_Msg_Encrypted(scConnection->currentSecuMode, scConnection->sendingBuffer);
        toSign = Is_Msg_Signed(scConnection->currentSecuMode);
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        if(securityPolicy->length>0){
            status = Write_UA_String(scConnection->sendingBuffer, securityPolicy);
        }else{
            // Null security policy is invalid parameter since unspecified
            status = STATUS_INVALID_PARAMETERS;
        }
    }
    // Sender Certificate:
    if(status == STATUS_OK){
        if(toSign != UA_FALSE && senderCertificate->length>0){ // Field shall be null if message not signed
            status = Write_UA_String(scConnection->sendingBuffer, senderCertificate);
        }else{
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            // status = Write_Int32(scConnection->sendingBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            status = Write_Int32(scConnection->sendingBuffer, -1);
            // NULL string: nothing to write
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){

        UA_Byte_String* recCertThumbprint = UA_NULL;
        if(toEncrypt != UA_FALSE){
            uint32_t thumbprintLength = 0;
            status = Asymmetric_Get_Certificate_Thumbprint_Length(scConnection->currentCryptoProvider,
                                                                  receiverCertificate,
                                                                  &thumbprintLength);
            if(status == STATUS_OK){
                recCertThumbprint = Create_Byte_String_Fixed_Size(thumbprintLength);
                if(recCertThumbprint != UA_NULL){
                    status = Asymmetric_Get_Certificate_Thumbprint(scConnection->currentCryptoProvider,
                                                                   receiverCertificate,
                                                                   recCertThumbprint);
                }else{
                    status = STATUS_NOK;
                }
            }

            status = Write_UA_String(scConnection->sendingBuffer, recCertThumbprint);
        }else{
            // TODO:
            // regarding mantis #3335 negative values are not valid anymore
            //status = Write_Int32(scConnection->sendingBuffer, 0);
            // BUT FOUNDATION STACK IS EXPECTING -1 !!!
            status = Write_Int32(scConnection->sendingBuffer, -1);
            // NULL string: nothing to write
        }

        Delete_Byte_String(recCertThumbprint);
    }else{
        status = STATUS_NOK;
    }

    return status;
}

StatusCode Write_Secure_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                                   const UA_Byte* data_src,
                                   uint32_t       count){
    SecureChannel_Connection* scConnection = UA_NULL;
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || msgBuffer == UA_NULL || msgBuffer->flushData == UA_NULL)
    {
        status = STATUS_INVALID_PARAMETERS;
    }else{
        status = STATUS_OK;
        scConnection = (SecureChannel_Connection*) msgBuffer->flushData;
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
                status = Write_Buffer(msgBuffer->buffers, data_src, tmpCount);

                // Flush it !
                if(status == STATUS_OK){
                    status = Flush_Secure_Msg_Buffer(msgBuffer,
                                                     UA_Msg_Chunk_Intermediate);
                }

                if(status == STATUS_OK){
                    status = Reset_Msg_Buffer_Next_Chunk
                              (msgBuffer,
                               msgBuffer->sequenceNumberPosition +
                                UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                }
            }
        }
        if(status == STATUS_OK){
            status = Write_Buffer(msgBuffer->buffers, data_src, count);
        }
    }
    return status;
}

StatusCode Set_Message_Length(UA_Msg_Buffer* msgBuffer,
                              uint32_t       msgLength){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;
    if(msgBuffer != UA_NULL && msgLength < msgBuffer->buffers->max_size){
        originPosition = msgBuffer->buffers->position;
        status = Set_Position_Buffer(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(msgBuffer, msgLength);
    }
    if(status == STATUS_OK){
        status = Set_Position_Buffer(msgBuffer->buffers, originPosition);
        msgBuffer->msgSize = msgLength;
    }
    return status;
}

StatusCode Set_Message_Chunk_Type(UA_Msg_Buffer*     msgBuffer,
                                  UA_Msg_Final_Chunk chunkType){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t originPosition = 0;

    if(msgBuffer != UA_NULL){
        originPosition = msgBuffer->buffers->position;
        status = Set_Position_Buffer(msgBuffer->buffers, UA_HEADER_ISFINAL_POSITION);
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
        status = Write_Msg_Buffer(msgBuffer, &chunkTypeByte, 1);
    }

    if(status == STATUS_OK){
        status = Set_Position_Buffer(msgBuffer->buffers, originPosition);
    }

    return status;
}

StatusCode Set_Sequence_Number(UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SecureChannel_Connection* scConnection = UA_NULL;
    uint32_t originPosition = 0;

    if(msgBuffer != UA_NULL && msgBuffer->flushData != UA_NULL){
       status = STATUS_OK;
       scConnection = (SecureChannel_Connection*) msgBuffer->flushData;
       if(scConnection-> lastSeqNumSent > UINT32_MAX - 1024){ // Part 6 §6.7.2 v1.03
           scConnection->lastSeqNumSent = 1;
       }else{
           scConnection->lastSeqNumSent = scConnection-> lastSeqNumSent + 1;
       }
       originPosition = msgBuffer->buffers->position;
       status = Set_Position_Buffer(msgBuffer->buffers, msgBuffer->sequenceNumberPosition);
       if(status == STATUS_OK){
           Write_UInt32(msgBuffer, scConnection->lastSeqNumSent);
       }

       if(status == STATUS_OK){
               status = Set_Position_Buffer(msgBuffer->buffers, originPosition);
       }

    }

    return status;
}

StatusCode Set_Request_Id(UA_Msg_Buffer* msgBuffer,
                          uint32_t*      requestId){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SecureChannel_Connection* scConnection = UA_NULL;
    if(msgBuffer != UA_NULL && msgBuffer->flushData != UA_NULL){
       status = STATUS_OK;
       scConnection = (SecureChannel_Connection*) msgBuffer->flushData;
       status = Set_Position_Buffer
                 (msgBuffer->buffers, msgBuffer->sequenceNumberPosition + UA_HEADER_SN_LENGTH);
       // TODO: request Id must be randomized ?
       scConnection->lastRequestIdSent++;
       if(scConnection->lastRequestIdSent + 1 == 0){
           scConnection->lastRequestIdSent++;
       }
       msgBuffer->requestId = scConnection->lastRequestIdSent;
       if(status == STATUS_OK){
           Write_UInt32(msgBuffer, scConnection->lastRequestIdSent);
       }
    }

    return status;
}

StatusCode Encode_Padding(SecureChannel_Connection* scConnection,
                          UA_Msg_Buffer*            msgBuffer,
                          uint8_t                   symmetricAlgo,
                          uint8_t*                  hasPadding,
                          uint8_t*                  paddingSize,
                          uint8_t*                  hasExtraPadding,
                          uint32_t*                 signatureSize)
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
            UA_Byte_String* publicKey = UA_NULL;
            Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &publicKey);

            status = Asymmetric_Get_Public_Key_Modulus_Length(scConnection->currentCryptoProvider,
                                                              publicKey,
                                                              &publicKeyModulusLength);

            if(status == STATUS_OK){
                *signatureSize = Get_Asymmetric_Signature_Size
                                  (scConnection->currentSecuPolicy,
                                   publicKeyModulusLength);
                status = Get_Asymmetric_Algorithm_Blocks_Sizes
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
             Get_Private_Key_Size
              (scConnection->currentSecuKeySets.senderKeySet->encryptKey);
            status = Get_Asymmetric_Algorithm_Blocks_Sizes
                      (scConnection->currentSecuPolicy,
                       encryptKeySize,
                       &cipherBlockSize,
                       &plainBlockSize);
        }
    }

    if(status == STATUS_OK){
        uint16_t padding = Get_Padding_Size
                            (msgBuffer->buffers->length -
                              msgBuffer->sequenceNumberPosition +
                              UA_SECURE_MESSAGE_SEQUENCE_LENGTH,
                             plainBlockSize,
                             *signatureSize);
        //Little endian conversion of padding:
        uint16_t littleEndianPadding = EncodeDecode_UInt16(padding);
        status = Write_Buffer(msgBuffer->buffers, (UA_Byte*) &littleEndianPadding, 1);
        *paddingSize = 0xFF & littleEndianPadding;

        if(status == STATUS_OK){
            // The value of each byte of the padding is equal to paddingSize:
            UA_Byte paddingBytes[*paddingSize];
            memset(paddingBytes, *paddingSize, *paddingSize);
            status = Write_Buffer(msgBuffer->buffers, paddingBytes, *paddingSize);
        }

        // Extra-padding necessary if
        if(status == STATUS_OK && encryptKeySize > 256){
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            UA_Byte extraPadding = 0x00FF & littleEndianPadding;
            Write_Buffer(msgBuffer->buffers, &extraPadding, 1);
        }
    }

    return status;
}

StatusCode Encode_Signature(SecureChannel_Connection* scConnection,
                            UA_Msg_Buffer*            msgBuffer,
                            uint8_t                   symmetricAlgo,
                            uint32_t                  signatureSize)
{
    StatusCode status = STATUS_OK;
    if(symmetricAlgo == UA_FALSE){
        if(scConnection->runningAppCertificate == UA_NULL ||
           scConnection->runningAppPrivateKey == UA_NULL ||
           scConnection->otherAppCertificate == UA_NULL){
           status = STATUS_INVALID_STATE;
        }else{
            UA_Byte_String* signedData = Create_Byte_String_Fixed_Size(signatureSize);
            if(signedData != UA_NULL){
                status = AsymmetricSign_Crypto_Provider
                          (scConnection->currentCryptoProvider,
                           msgBuffer->buffers->data,
                           msgBuffer->buffers->length,
                           scConnection->runningAppPrivateKey,
                           signedData);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Write_Buffer(msgBuffer->buffers,
                                      signedData->characters,
                                      signedData->length);
            }
            Delete_Byte_String(signedData);
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == UA_NULL){
            status = STATUS_INVALID_STATE;
        }else{
            UA_Byte_String* signedData = Create_Byte_String_Fixed_Size(signatureSize);
            if(signedData != UA_NULL){
                status = SymmetricSign_Crypto_Provider
                          (scConnection->currentCryptoProvider,
                           msgBuffer->buffers->data,
                           msgBuffer->buffers->length,
                           scConnection->currentSecuKeySets.senderKeySet->signKey,
                           signedData);
            }else{
                status = STATUS_NOK;
            }

            if(status == STATUS_OK){
                status = Write_Buffer(msgBuffer->buffers,
                                      signedData->characters,
                                      signedData->length);
            }
            Delete_Byte_String(signedData);
        }
    }
    return status;
}

StatusCode Encrypt_Message(SecureChannel_Connection* scConnection,
                           UA_Msg_Buffer*            msgBuffer,
                           uint8_t                   symmetricAlgo,
                           uint32_t                  encryptedDataLength,
                           UA_Msg_Buffer*            encryptedMsgBuffer)
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
           scConnection->otherAppCertificate == UA_NULL){
           status = STATUS_INVALID_STATE;
        }else{
            UA_Byte_String* otherAppPublicKey = UA_NULL;
            UA_Byte* encryptedData = UA_NULL;

            if(status == STATUS_OK){
                // Retrieve other app public key from certificate
                status = Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &otherAppPublicKey);
            }

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
                    Set_Data_Length_Buffer(encryptedMsgBuffer->buffers,
                                           msgBuffer->sequenceNumberPosition + encryptedDataLength);
                    encryptedMsgBuffer->msgSize = msgBuffer->msgSize;
                    // Message size was already encrypted message length, it must be the same now
                    assert(encryptedMsgBuffer->buffers->length == encryptedMsgBuffer->msgSize);
                    // Ensure internal properties coherency (even if not used)
                    encryptedMsgBuffer->isFinal = msgBuffer->isFinal;
                    encryptedMsgBuffer->type = msgBuffer->type;
                    encryptedMsgBuffer->requestId = msgBuffer->requestId;
                    encryptedMsgBuffer->secureType = msgBuffer->secureType;
                    encryptedMsgBuffer->nbChunks = msgBuffer->nbChunks;
                }
            }

            // Encrypt
            if(status == STATUS_OK){
                status = AsymmetricEncrypt_Crypto_Provider
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
                    Set_Data_Length_Buffer(encryptedMsgBuffer->buffers,
                                           msgBuffer->sequenceNumberPosition + encryptedDataLength);

                }
            }

            // Encrypt
            if(status == STATUS_OK){
                status = SymmetricEncrypt_Crypto_Provider
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

StatusCode Flush_Secure_Msg_Buffer(UA_Msg_Buffer*     msgBuffer,
                                   UA_Msg_Final_Chunk chunkType){
    SecureChannel_Connection* scConnection = UA_NULL;
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
        scConnection = (SecureChannel_Connection*) msgBuffer->flushData;

        toEncrypt = Is_Msg_Encrypted(scConnection->currentSecuMode, msgBuffer);

        toSign = Is_Msg_Signed(scConnection->currentSecuMode);

        // Determine if the asymmetric algorithms must be used
        if(msgBuffer->secureType == UA_OpenSecureChannel){
            symmetricAlgo = UA_FALSE;
        }

        //// Encode padding if encrypted message
        if(toEncrypt == UA_FALSE){
            // No padding fields
        }else{
            status = Encode_Padding(scConnection,
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
            status = Get_Encrypted_Data_Length(scConnection, plainDataToEncryptLength, symmetricAlgo, &encryptedLength);
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
            status = Encode_Signature(scConnection,
                                      msgBuffer,
                                      symmetricAlgo,
                                      signatureSize);

        }

        //// Check sender certificate size is not bigger than maximum size to be sent
        if(status == STATUS_OK &&
           msgBuffer->secureType == UA_OpenSecureChannel &&
           toSign != UA_FALSE){
            status = Check_Max_Sender_Certificate_Size(scConnection->currentCryptoProvider,
                                                       scConnection->runningAppCertificate,
                                                       msgBuffer->buffers->max_size,
                                                       scConnection->currentSecuPolicy,
                                                       hasPadding,
                                                       paddingSize,
                                                       hasExtraPadding,
                                                       signatureSize);
        }

        if(status == STATUS_OK && toEncrypt == UA_FALSE){
            // No encryption necessary but we need to attach buffer as transport buffer (done during encryption otherwise)
            status = Copy_Buffer_To_Msg_Buffer(scConnection->transportConnection->outputMsgBuffer,
                                               scConnection->sendingBuffer);
        }else{
            // TODO: use detach / attach to control references on the transport msg buffer ?
            status = Encrypt_Message(scConnection,
                                     msgBuffer,
                                     symmetricAlgo,
                                     encryptedLength,
                                     scConnection->transportConnection->outputMsgBuffer);
        }

        if(status == STATUS_OK){
            // TODO: detach transport buffer ?
            status = Flush_Msg_Buffer(scConnection->transportConnection->outputMsgBuffer);
        }
    }
    return status;
}

StatusCode Decode_Secure_Message_SecureChannelId(SecureChannel_Connection* scConnection,
                                                 UA_Msg_Buffer*            transportBuffer)
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
        status = Read_UInt32(transportBuffer, &secureChannelId);
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

StatusCode Decode_Asymmetric_Security_Header(SecureChannel_Connection* scConnection,
                                             PKIProvider*              pkiProvider,
                                             UA_Msg_Buffer*            transportBuffer,
                                             uint32_t                  validateSenderCert)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toEncrypt = 1; // True
    uint32_t toSign = 1; // True

    UA_Byte_String* securityPolicy = Create_Byte_String();
    UA_Byte_String* senderCertificate = Create_Byte_String();
    UA_Byte_String* receiverCertThumb = Create_Byte_String();

    uint32_t validationStatusCode = 0;

    if(scConnection != UA_NULL &&
       transportBuffer != UA_NULL)
    {
        // Asymmetric security header must use current security parameters
        // (TODO: add guarantee we are treating last OPN sent: using pending requests ?)
        toEncrypt = Is_Msg_Encrypted(scConnection->currentSecuMode,
                                     transportBuffer);
        toSign = Is_Msg_Signed(scConnection->currentSecuMode);
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        status = Read_UA_Byte_String(transportBuffer, securityPolicy);

        if(status == STATUS_OK){
            uint32_t secuPolicyComparison = 0;
            status = Compare_Byte_Strings(scConnection->currentSecuPolicy, securityPolicy, &secuPolicyComparison);

            if(status != STATUS_OK || secuPolicyComparison != 0){
                status = STATUS_INVALID_RCV_PARAMETER;
            }
        }
    }


    // Sender Certificate:
    if(status == STATUS_OK){
        status = Read_UA_Byte_String(transportBuffer, senderCertificate);
        if(status == STATUS_OK){
            if (toSign == UA_FALSE && senderCertificate->length > 0){
                // Table 27 part 6: "field shall be null if the Message is not signed"
                status = STATUS_INVALID_RCV_PARAMETER;
            }else if(toSign != UA_FALSE){
                // Check certificate is the same as the one in memory
                uint32_t otherAppCertComparison = 0;
                status = Compare_Byte_Strings(scConnection->otherAppCertificate,
                                              senderCertificate,
                                              &otherAppCertComparison);

                if(status != STATUS_OK || otherAppCertComparison != 0){
                    status = STATUS_INVALID_RCV_PARAMETER;
                }

                if(status == STATUS_OK && validateSenderCert != UA_FALSE){
                    status = PKIProvider_Validate_Certificate(pkiProvider,
                                                              senderCertificate,
                                                              &validationStatusCode);
                    if(status != STATUS_OK){
                        // TODO: report validation status code
                    }
                }
            }
        }
    }

    // Receiver Certificate Thumbprint:
    if(status == STATUS_OK){
        status = Read_UA_Byte_String(transportBuffer, receiverCertThumb);

        if(status == STATUS_OK){
            if(toEncrypt == UA_FALSE && receiverCertThumb->length > 0){
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                status =STATUS_INVALID_RCV_PARAMETER;
            }else if(toEncrypt != UA_FALSE){
                // Check thumbprint matches current app certificate thumbprint

                UA_Byte_String* curAppCertThumbprint = UA_NULL;
                uint32_t thumbprintLength = 0;
                uint32_t runningAppCertComparison = 0;

                status = Asymmetric_Get_Certificate_Thumbprint_Length(scConnection->currentCryptoProvider,
                                                                      scConnection->runningAppCertificate,
                                                                      &thumbprintLength);
                if(status == STATUS_OK){
                    if(thumbprintLength == receiverCertThumb->length){
                        curAppCertThumbprint = Create_Byte_String_Fixed_Size(thumbprintLength);
                        if(curAppCertThumbprint != UA_NULL){
                            status = Asymmetric_Get_Certificate_Thumbprint(scConnection->currentCryptoProvider,
                                                                           scConnection->runningAppCertificate,
                                                                           curAppCertThumbprint);

                            if(status == STATUS_OK){
                                status = Compare_Byte_Strings(curAppCertThumbprint,
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
            } // if toEncrypt
        } // if decoded thumbprint
    }

    Delete_Byte_String(securityPolicy);
    Delete_Byte_String(senderCertificate);
    Delete_Byte_String(receiverCertThumb);

    return status;
}

StatusCode Is_Precedent_Crypto_Data(SecureChannel_Connection* scConnection,
                                   uint32_t                  receivedTokenId,
                                   uint32_t*                 isPrecCryptoData){
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

StatusCode Decrypt_Message_Content(SecureChannel_Connection* scConnection,
                                   UA_Msg_Buffer*            transportBuffer,
                                   uint32_t                  isSymmetric,
                                   uint32_t                  isPrecCryptoData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toDecrypt = 1;
    uint32_t decryptedTextLength = 0;
    Crypto_Provider* cryptoProvider = UA_NULL;
    Msg_Security_Mode securityMode = Msg_Security_Mode_Invalid;
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

        toDecrypt = Is_Msg_Encrypted(securityMode, transportBuffer);
    }

    if(toDecrypt != UA_FALSE){
        // Message is encrypted

        if(status == STATUS_OK){
            if(isSymmetric == UA_FALSE){
                status = AsymmetricDecrypt_Length_Crypto_Provider(cryptoProvider,
                                                                  transportBuffer->buffers->data,
                                                                  transportBuffer->buffers->length,
                                                                  scConnection->runningAppPrivateKey,
                                                                  &decryptedTextLength);

                if(status == STATUS_OK && decryptedTextLength <= scConnection->receptionBuffers->buffers->max_size){
                    plainBuffer = Next_Chunk_From_Msg_Buffers(scConnection->receptionBuffers, &bufferIdx);
                    if(plainBuffer != UA_NULL){
                        assert(plainBuffer->max_size >= decryptedTextLength);
                        status = AsymmetricDecrypt_Crypto_Provider(cryptoProvider,
                                                                   transportBuffer->buffers->data,
                                                                   transportBuffer->buffers->length,
                                                                   scConnection->runningAppPrivateKey,
                                                                   plainBuffer->data,
                                                                   &decryptedTextLength);
                    }else{
                        status = STATUS_NOK;
                    }
                }else if(status == STATUS_OK){
                    status = STATUS_INVALID_STATE;
                }
            }else{

                SC_Security_Key_Set* receiverKeySet = UA_NULL;
                if(isPrecCryptoData == UA_FALSE){
                    receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
                }else{
                    receiverKeySet = scConnection->precSecuKeySets.receiverKeySet;
                }
                status = SymmetricDecrypt_Length_Crypto_Provider(cryptoProvider,
                                                                 transportBuffer->buffers->data,
                                                                 transportBuffer->buffers->length,
                                                                 receiverKeySet->encryptKey,
                                                                 receiverKeySet->initVector,
                                                                 &decryptedTextLength);

                if(status == STATUS_OK && decryptedTextLength <= scConnection->receptionBuffers->buffers->max_size){
                    plainBuffer = Next_Chunk_From_Msg_Buffers(scConnection->receptionBuffers, &bufferIdx);
                    if(plainBuffer != UA_NULL){
                        assert(plainBuffer->max_size >= decryptedTextLength);
                        status = SymmetricDecrypt_Crypto_Provider(cryptoProvider,
                                                                  transportBuffer->buffers->data,
                                                                  transportBuffer->buffers->length,
                                                                  receiverKeySet->encryptKey,
                                                                  receiverKeySet->initVector,
                                                                  plainBuffer->data,
                                                                  &decryptedTextLength);
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
        plainBuffer = Next_Chunk_From_Msg_Buffers(scConnection->receptionBuffers, &bufferIdx);
        assert(transportBuffer->nbChunks == scConnection->receptionBuffers->nbChunks);
        status = Copy_Buffer_To_Msg_Buffers(scConnection->receptionBuffers, bufferIdx, transportBuffer);
    }

    return status;
}

StatusCode Verify_Message_Signature(SecureChannel_Connection* scConnection,
                                    uint32_t                  isSymmetric,
                                    uint32_t                  isPrecCryptoData)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t toVerify = 1;

    Crypto_Provider* cryptoProvider = UA_NULL;
    Msg_Security_Mode securityMode = Msg_Security_Mode_Invalid;
    UA_String* securityPolicy = UA_NULL;
    Buffer* receptionBuffer = Get_Current_Chunk_From_Msg_Buffers(scConnection->receptionBuffers);

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

        toVerify = Is_Msg_Signed(securityMode);
    }

    if(toVerify != UA_FALSE){
        if(isSymmetric == UA_FALSE){
            uint32_t publicKeyModulusLength = 0;
            UA_Byte_String* publicKey = UA_NULL;
            Retrieve_Public_Key_From_Cert(scConnection, UA_FALSE, &publicKey);

            status = Asymmetric_Get_Public_Key_Modulus_Length(cryptoProvider,
                                                              publicKey,
                                                              &publicKeyModulusLength);

            if(status == STATUS_OK){
                signatureSize = Get_Asymmetric_Signature_Size
                                 (securityPolicy,
                                  publicKeyModulusLength);
                signaturePosition = receptionBuffer->length - signatureSize;

                status = AsymmetricVerify_Crypto_Provider(cryptoProvider,
                                                          receptionBuffer->data,
                                                          signaturePosition,
                                                          publicKey,
                                                          &(receptionBuffer->data[signaturePosition]),
                                                          signatureSize);
            }
        }else{
            SC_Security_Key_Set* receiverKeySet = UA_NULL;
            if(isPrecCryptoData == UA_FALSE){
                receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
            }else{
                receiverKeySet = scConnection->precSecuKeySets.receiverKeySet;
            }

            signatureSize = Get_Symmetric_Signature_Size(securityPolicy);
            signaturePosition = receptionBuffer->length - signatureSize;

            status = SymmetricVerify_Crypto_Provider(cryptoProvider,
                                                     receptionBuffer->data,
                                                     signaturePosition,
                                                     receiverKeySet->signKey,
                                                     &(receptionBuffer->data[signaturePosition]),
                                                     signatureSize);
        }
    }

    return status;
}

StatusCode Check_Sequence_Number_Received(SecureChannel_Connection* scConnection)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t seqNumber = 0;

    if(scConnection != UA_NULL){
        status = STATUS_OK;
        status = Read_UInt32(scConnection->receptionBuffers, &seqNumber);
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
