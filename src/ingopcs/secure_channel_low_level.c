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

SecureChannel_Connection* Create_Secure_Connection (){
    SecureChannel_Connection* sConnection = UA_NULL;
    TCP_UA_Connection* connection = Create_Connection();

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
    if(scConnection != UA_NULL){
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
                                      scConnection->transportConnection);
        if(msgBuffer != UA_NULL){
            scConnection->sendingBuffer = msgBuffer;
            status = STATUS_OK;
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

StatusCode Check_Max_Sender_Certificate_Size(OpcUa_CryptoProvider* cryptoProvider,
                                             UA_Byte_String        senderCertificate,
                                             uint32_t              messageChunkSize,
                                             UA_String*            securityPolicyUri,
                                             uint32_t              padding,
                                             uint32_t              extraPadding,
                                             uint32_t              asymmetricSignatureSize){
    StatusCode status = STATUS_NOK;
    //TODO: can be checked only when generating padding since
    return status;
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
                                             OpcUa_CryptoProvider*     cryptoProvider,
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
    const uint32_t headersSize = scConnection->sendingBufferSNPosition - 1;
    const uint32_t bodyChunkSize = scConnection->sendingBuffer->buffers->max_size - headersSize;

    // Computed maxBlock and then maxBodySize based on revised formula of mantis ticket #2897
    // Spec 1.03 part 6 incoherent
    const uint32_t maxBlocks = bodyChunkSize / cipherBlockSize;
    // MaxBodySize = unCiphered block size * max blocs - sequence header -1 for PaddingSize field
    return plainBlockSize * maxBlocks - UA_SECURE_MESSAGE_SEQUENCE_LENGTH - signatureSize - 1;
}

uint32_t Get_Padding_Size(uint32_t bytesToWrite,
                          uint32_t plainBlockSize,
                          uint32_t signatureSize){
    return plainBlockSize - ((bytesToWrite + signatureSize + 1) % plainBlockSize);
}

StatusCode Encode_Secure_Message_Footer(SecureChannel_Connection* scConnection,
                                        OpcUa_CryptoProvider      cryptoProvider,
                                        UA_Byte_String            senderCertificate,
                                        Private_Key               pKey){
    //assert(bytesToWrite <= GetMaxBodySize(...))
    return STATUS_OK;
}

StatusCode Write_Secure_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                                   UA_Byte*       data_src,
                                   uint32_t       count){
    return STATUS_OK;
}

StatusCode Flush_Secure_Msg_Buffer(UA_Msg_Buffer* msgBuffer){
    return STATUS_OK;
}

