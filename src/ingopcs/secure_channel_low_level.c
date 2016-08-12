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
        if(scConnection->sendingBuffer != UA_NULL){
            Delete_Msg_Buffer(scConnection->sendingBuffer);
        }
        if(scConnection->receptionBuffers != UA_NULL){
                    Delete_Msg_Buffers(scConnection->receptionBuffers);
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
        scConnection->runningAppPrivateKey = runningAppPrivateKey;
        scConnection->otherAppCertificate = otherAppCertificate;
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
             (scConnection->transportConnection->maxChunkCountRcv);

        }else if(scConnection->transportConnection->maxMessageSizeRcv != 0){
            // Is message size including whole message or only body as it is the case in part 4 §5.3 last § ???
            scConnection->receptionBuffers = Create_Msg_Buffers
              (scConnection->transportConnection->maxMessageSizeRcv
               /scConnection->transportConnection->receiveBufferSize);
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
    if(scConnection->sendingBuffer == NULL){
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

StatusCode Check_Max_Sender_Certificate_Size(Crypto_Provider* cryptoProvider,
                                             UA_Byte_String*       senderCertificate,
                                             uint32_t              messageChunkSize,
                                             UA_String*            securityPolicyUri,
                                             uint8_t               hasPadding,
                                             uint32_t              padding,
                                             uint32_t              extraPadding,
                                             uint32_t              asymmetricSignatureSize){
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

StatusCode Get_Asymmetric_Algorithm_Blocks_Sizes(UA_String* securityPolicyUri,
                                                 uint32_t   keySize,
                                                 uint32_t*  cipherTextBlockSize,
                                                 uint32_t*  plainTextBlockSize){
    StatusCode status = STATUS_OK;
    // TODO: implement with correct sizes
    return status;
}

uint32_t Get_Asymmetric_Signature_Size(UA_String*       securityPolicyUri,
                                       UA_Byte_String*  pubKeySize){
    // TODO: implement with correct sizes
    return 0;
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
    // TODO: implement with correct sizes
    return 0;
}

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
                                  uint32_t                  sequenceNumber,
                                  uint32_t                  requestId){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != UA_NULL && scConnection->sendingBuffer != UA_NULL){
        //TODO: check constraints on sequence number ? Modified in next spec ?
        status = STATUS_OK;
    }
    if(status == STATUS_OK){
        scConnection->sendingBufferSNPosition = scConnection->sendingBuffer->buffers->position;
        Write_UInt32(scConnection->sendingBuffer, sequenceNumber);
    }
    if(status == STATUS_OK){
        Write_UInt32(scConnection->sendingBuffer, requestId);
    }
    return status;
}

StatusCode Encode_Asymmetric_Security_Header(SecureChannel_Connection* scConnection,
                                             Crypto_Provider*     cryptoProvider,
                                             UA_String*                securityPolicy,
                                             UA_Byte_String*           clientCertificate,
                                             UA_Byte_String*           serverCertificate){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(scConnection != UA_NULL && scConnection->sendingBuffer != UA_NULL &&
       //cryptoProvider != UA_NULL &&
       securityPolicy != UA_NULL &&
       clientCertificate != UA_NULL &&
       serverCertificate != UA_NULL){
        status = STATUS_OK;
    }

    // Security Policy:
    if(status == STATUS_OK){
        if(securityPolicy->length>0){
            status = Write_Int32(scConnection->sendingBuffer, securityPolicy->length);
            if(status == STATUS_OK){
                status = Write_Secure_Msg_Buffer(scConnection->sendingBuffer,
                                                 securityPolicy->characters,
                                                 securityPolicy->length);
            }
        }else{
            // regarding mantis #3335 negative values are not valid anymore
            status = Write_Int32(scConnection->sendingBuffer, 0);
            // NULL string: nothing to write
        }
    }
    // Sender Certificate:
    if(status == STATUS_OK){
        if(clientCertificate->length>0){
            status = Write_Int32(scConnection->sendingBuffer, clientCertificate->length);
            if(status == STATUS_OK){
                status = Write_Secure_Msg_Buffer(scConnection->sendingBuffer,
                                                 clientCertificate->characters,
                                                 clientCertificate->length);
            }
        }else{
            // regarding mantis #3335 negative values are not valid anymore
            status = Write_Int32(scConnection->sendingBuffer, 0);
            // NULL string: nothing to write
        }
    }

    // Receiver Certificate Thumbprint:
    // TODO: get certificate thumbprint from crypto provider
    UA_Byte_String* recCertThumbprint = Create_Byte_String();
    //OpcUa_Crypto_GetCertificateThumbprint(cryptoProvider, serverCertificate, recCertThumbprint);
    if(status == STATUS_OK){
        if(recCertThumbprint->length>0){
            status = Write_Int32(scConnection->sendingBuffer, recCertThumbprint->length);
            if(status == STATUS_OK){
                status = Write_Secure_Msg_Buffer(scConnection->sendingBuffer,
                                                 recCertThumbprint->characters,
                                                 recCertThumbprint->length);
            }
        }else{
            // regarding mantis #3335 negative values are not valid anymore
            status = Write_Int32(scConnection->sendingBuffer, 0);
            // NULL string: nothing to write
        }
    }

    return status;
}

uint32_t Get_MaxBodySize(SecureChannel_Connection* scConnection,
                         uint32_t                  cipherBlockSize,
                         uint32_t                  plainBlockSize,
                         uint32_t                  signatureSize){
    // Ensure cipher block size is greater or equal to plain block size:
    //  otherwise the plain size could be greater than the  buffer size regarding computation
    assert(cipherBlockSize >= plainBlockSize);
    const uint32_t headersSize = scConnection->sendingBufferSNPosition - 1;
    const uint32_t bodyChunkSize = scConnection->sendingBuffer->buffers->max_size - headersSize;

    // Computed maxBlock and then maxBodySize based on revised formula of mantis ticket #2897
    // Spec 1.03 part 6 incoherent
    const uint32_t maxBlocks = bodyChunkSize / cipherBlockSize;
    // MaxBodySize = unCiphered block size * max blocs - sequence header -1 for PaddingSize field
    return plainBlockSize * maxBlocks - UA_SECURE_MESSAGE_SEQUENCE_LENGTH - signatureSize - 1;
}

uint16_t Get_Padding_Size(uint32_t bytesToWrite,
                          uint32_t plainBlockSize,
                          uint32_t signatureSize){
    return plainBlockSize - ((bytesToWrite + signatureSize + 1) % plainBlockSize);
}

StatusCode Encode_Secure_Message_Footer(SecureChannel_Connection* scConnection,
                                        Crypto_Provider           cryptoProvider,
                                        UA_Byte_String            senderCertificate,
                                        Private_Key               pKey){
    //assert(bytesToWrite <= GetMaxBodySize(...))
    return STATUS_OK;
}

StatusCode Write_Secure_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                                   UA_Byte*       data_src,
                                   uint32_t       count){
    SecureChannel_Connection* scConnection = UA_NULL;
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || msgBuffer == UA_NULL || msgBuffer->flushData == UA_NULL)
    {
        status = STATUS_INVALID_PARAMETERS;
    }else{
        scConnection = (SecureChannel_Connection*) msgBuffer->flushData;
        if(msgBuffer->buffers->position + count >
            scConnection->sendingBufferSNPosition +
            UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
            scConnection->sendingMaxBodySize)
        {
            if(msgBuffer->nbChunks + 1 > msgBuffer->maxChunks){
                // TODO: send an abort message instead of message !!!
                status = STATUS_INVALID_STATE;
            }else{
                // Fulfill buffer with maximum amount of bytes
                uint32_t tmpCount =
                 scConnection->sendingBufferSNPosition +
                 UA_SECURE_MESSAGE_SEQUENCE_LENGTH +
                 scConnection->sendingMaxBodySize - msgBuffer->buffers->position;
                count = count - tmpCount;
                status = Write_Buffer(msgBuffer->buffers, data_src,tmpCount);

                // Flush it !
                if(status == STATUS_OK){
                    status = Flush_Secure_Msg_Buffer(msgBuffer,
                                                     UA_Msg_Chunk_Intermediate);
                }

                if(status == STATUS_OK){
                    status = Reset_Msg_Buffer_Next_Chunk
                              (msgBuffer,
                               scConnection->sendingBufferSNPosition +
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

StatusCode Set_Message_Length(UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(msgBuffer != UA_NULL){
        status = Set_Position_Buffer(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(msgBuffer, msgBuffer->buffers->length);
    }
    if(status == STATUS_OK){
        msgBuffer->msgSize = msgBuffer->buffers->length;
    }
    return status;
}

StatusCode Set_Sequence_Number(UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    SecureChannel_Connection* scConnection = UA_NULL;
    if(msgBuffer != UA_NULL && msgBuffer->flushData != UA_NULL){
       status = STATUS_OK;
       scConnection = (SecureChannel_Connection*) msgBuffer->flushData;
       if(scConnection-> lastSeqNumSent > UINT32_MAX - 1024){ // Part 6 §6.7.2 v1.03
           scConnection->lastSeqNumSent = 1;
       }else{
           scConnection->lastSeqNumSent = scConnection-> lastSeqNumSent + 1;
       }
       status = Set_Position_Buffer(msgBuffer->buffers, scConnection->sendingBufferSNPosition);
       if(status == STATUS_OK){
           Write_UInt32(msgBuffer, scConnection->lastSeqNumSent);
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
            UA_Byte_String* publicKey = Create_Byte_String();
            status = GetPublicKeyFromCert_Crypto_Provider
                      (scConnection->currentCryptoProvider,
                       scConnection->runningAppCertificate,
                       UA_NULL,
                       publicKey);
            encryptKeySize = Get_Private_Key_Size(scConnection->runningAppPrivateKey);
            if(status == STATUS_OK){
                *signatureSize = Get_Asymmetric_Signature_Size
                                  (scConnection->currentSecuPolicy,
                                   publicKey);
                status = Get_Asymmetric_Algorithm_Blocks_Sizes
                          (scConnection->currentSecuPolicy,
                           encryptKeySize,
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
                              scConnection->sendingBufferSNPosition +
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
            UA_Byte_String* signedData = Create_Byte_String();
            signedData->length = msgBuffer->buffers->length;
            signedData->characters = msgBuffer->buffers->data;
            status = AsymmetricSign_Crypto_Provider
                      (scConnection->currentCryptoProvider,
                       msgBuffer->buffers->data,
                       msgBuffer->buffers->length,
                       scConnection->runningAppPrivateKey,
                       signedData);
            if(status == STATUS_OK){
                assert(signedData->length == signatureSize);
                status = Write_Buffer(msgBuffer->buffers,
                                      signedData->characters,
                                      signedData->length);
            }
        }
    }else{
        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL ||
           scConnection->currentSecuKeySets.receiverKeySet == UA_NULL){
            status = STATUS_INVALID_STATE;
        }else{
            UA_Byte_String* signedData = Create_Byte_String();
            signedData->length = msgBuffer->buffers->length;
            signedData->characters = msgBuffer->buffers->data;
            status = SymmetricSign_Crypto_Provider
                      (scConnection->currentCryptoProvider,
                       msgBuffer->buffers->data,
                       msgBuffer->buffers->length,
                       scConnection->currentSecuKeySets.senderKeySet->signKey,
                       signedData);
            if(status == STATUS_OK){
                assert(signedData->length == signatureSize);
                status = Write_Buffer(msgBuffer->buffers,
                                      signedData->characters,
                                      signedData->length);
            }
        }
    }
    return status;
}

StatusCode Encrypt_Message(SecureChannel_Connection* scConnection,
                           UA_Msg_Buffer*            msgBuffer,
                           uint8_t                   symmetricAlgo,
                           UA_Msg_Buffer*            encryptedMsgBuffer)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte* dataToEncrypt = &msgBuffer->buffers->data[scConnection->sendingBufferSNPosition];
    const uint32_t dataToEncryptLength = msgBuffer->buffers->length - scConnection->sendingBufferSNPosition;

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
            UA_Byte* encryptedData = UA_NULL;
            UA_Byte_String* otherAppPublicKey = Create_Byte_String();
            uint32_t encryptedDataLength = 0;
            status = GetPublicKeyFromCert_Crypto_Provider(scConnection->currentCryptoProvider,
                                                          scConnection->otherAppCertificate,
                                                          UA_NULL,
                                                          otherAppPublicKey);
            // Retrieve cipher length
            if(status == STATUS_OK){
                status = AsymmetricEncrypt_Length_Crypto_Provider
                          (scConnection->currentCryptoProvider,
                           dataToEncrypt,
                           dataToEncryptLength,
                           otherAppPublicKey,
                           &encryptedDataLength);
            }

            // Check size of encrypted data array
            if(status == STATUS_OK){
                if(encryptedMsgBuffer->buffers->max_size <
                    scConnection->sendingBufferSNPosition + encryptedDataLength){
                    status = STATUS_NOK;
                }
                encryptedData = encryptedMsgBuffer->buffers->data;
                if(encryptedData == UA_NULL){
                    status = STATUS_NOK;
                }else{
                    // Copy non encrypted headers part
                    memcpy(encryptedData, msgBuffer->buffers, scConnection->sendingBufferSNPosition);
                }
            }

            // Encrypt
            if(status == STATUS_OK){
                status = AsymmetricEncrypt_Crypto_Provider
                          (scConnection->currentCryptoProvider,
                           dataToEncrypt,
                           dataToEncryptLength,
                           otherAppPublicKey,
                           encryptedData,
                           &encryptedDataLength);
            }
        }
    }else if (status == STATUS_OK){
//        if(scConnection->currentSecuKeySets.senderKeySet == UA_NULL ||
//           scConnection->currentSecuKeySets.receiverKeySet == UA_NULL){
//            status = STATUS_INVALID_STATE;
//        }else{
//            UA_Byte_String* signedData = Create_Byte_String();
//            signedData->length = msgBuffer->buffers->length;
//            signedData->characters = msgBuffer->buffers->data;
//            status = SymmetricSign_Crypto_Provider
//                      (scConnection->currentCryptoProvider,
//                       msgBuffer->buffers->data,
//                       msgBuffer->buffers->length,
//                       scConnection->currentSecuKeySets.senderKeySet->signKey,
//                       signedData);
//            if(status == STATUS_OK){
//                assert(signedData->length == signatureSize);
//                status = Write_Buffer(msgBuffer->buffers,
//                                      signedData->characters,
//                                      signedData->length);
//            }
//        }
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

    if(msgBuffer == UA_NULL || msgBuffer->flushData == UA_NULL ||
       (chunkType != UA_Msg_Chunk_Final && msgBuffer->secureType != UA_SecureMessage))
    {
        status = STATUS_INVALID_PARAMETERS;
    }else{
        scConnection = (SecureChannel_Connection*) msgBuffer->flushData;
        // Determine if the message must be encrypted
        if(scConnection->currentSecuMode == Msg_Security_Mode_None ||
           (scConnection->currentSecuMode == Msg_Security_Mode_Sign &&
            msgBuffer->secureType != UA_OpenSecureChannel))
        {
            toEncrypt = UA_FALSE;
        }
        // Determine if the message must be signed
        if(scConnection->currentSecuMode == Msg_Security_Mode_Sign ||
           scConnection->currentSecuMode == Msg_Security_Mode_SignAndEncrypt)
        {
            toSign = UA_FALSE;
        }
        // Determine if the asymmetric algorithms must be used
        if(msgBuffer->secureType == UA_OpenSecureChannel){
            symmetricAlgo = UA_FALSE;
        }

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

        if(toSign == UA_FALSE){
            // No signature field
        }else if(status == STATUS_OK){
            status = Encode_Signature(scConnection,
                                      msgBuffer,
                                      symmetricAlgo,
                                      signatureSize);

        }

        if(status == STATUS_OK && msgBuffer->secureType == UA_OpenSecureChannel){
            status = Check_Max_Sender_Certificate_Size(scConnection->currentCryptoProvider,
                                                       scConnection->runningAppCertificate,
                                                       msgBuffer->buffers->length,
                                                       scConnection->currentSecuPolicy,
                                                       hasPadding,
                                                       paddingSize,
                                                       hasExtraPadding,
                                                       signatureSize);
        }

        if(status == STATUS_OK){
            status = Set_Message_Length(msgBuffer);
        }

        if(status == STATUS_OK){
            status = Set_Sequence_Number(msgBuffer);
        }

        if(status == STATUS_OK && toEncrypt == UA_FALSE){
            // No encryption necessary
        }else{
            // TODO: use detach / attach to control references on the transport msg buffer ?
            status = Encrypt_Message(scConnection,
                                     msgBuffer,
                                     symmetricAlgo,
                                     scConnection->transportConnection->outputMsgBuffer);
        }

        if(status == STATUS_OK){
            // TODO: detach transport buffer ?
            Flush_Msg_Buffer(scConnection->transportConnection->outputMsgBuffer);
        }
    }
    return status;
}

