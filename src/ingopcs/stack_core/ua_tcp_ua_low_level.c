/*
 * tcp_ua_connection.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#include "ua_tcp_ua_low_level.h"

#include <assert.h>
#include <string.h>

#include <ua_encoder.h>
#include <opcua_statuscodes.h>

const uint32_t tcpProtocolVersion = 0;

StatusCode TCP_UA_EncodeHeader(UA_MsgBuffer*  msgBuffer,
                               TCP_UA_MsgType type){
    StatusCode status = STATUS_OK;
    UA_Byte fByte = 'F';
    assert(msgBuffer->buffers->max_size > TCP_UA_HEADER_LENGTH);
    switch(type){
        case TCP_UA_Message_Hello:
            status = Buffer_Write(msgBuffer->buffers, HEL, 3);
            break;
        case TCP_UA_Message_Acknowledge:
            status = Buffer_Write(msgBuffer->buffers, ACK, 3);
            break;
        case TCP_UA_Message_Error:
            status = Buffer_Write(msgBuffer->buffers, ERR, 3);
            break;
        case TCP_UA_Message_SecureMessage:
            // Managed by secure channel layer
            break;
        default:
            // Error case (Invalid or Unknown)
            status = STATUS_NOK;
    }
    if(status == STATUS_OK){
        // reserved byte
        status = TCP_UA_WriteMsgBuffer(msgBuffer, &fByte, 1);
    }
    if(status == STATUS_OK){
        const uint32_t headerLength = TCP_UA_HEADER_LENGTH;
        status = UInt32_Write(&headerLength, msgBuffer);
        if(status == STATUS_OK){
            msgBuffer->type = type;
            msgBuffer->currentChunkSize = TCP_UA_HEADER_LENGTH;
            msgBuffer->nbChunks = 1;
        }
    }
    return status;
}

StatusCode TCP_UA_FinalizeHeader(UA_MsgBuffer* msgBuffer){
    assert(msgBuffer->type == TCP_UA_Message_Hello
           || msgBuffer->type == TCP_UA_Message_Acknowledge
           || msgBuffer->type == TCP_UA_Message_Error);
    StatusCode status = STATUS_NOK;
    const uint32_t currentPosition = msgBuffer->buffers->position;

    status = Buffer_SetPosition(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);

    if(status == STATUS_OK){
        status = UInt32_Write(&currentPosition, msgBuffer);
    }
    if(status == STATUS_OK){
        status = Buffer_SetPosition(msgBuffer->buffers, currentPosition);
        msgBuffer->currentChunkSize = currentPosition;
    }
    return status;
}


StatusCode TCP_UA_ReadData(UA_Socket*    socket,
                           UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    uint32_t readBytes;

    if(msgBuffer->buffers->length >= TCP_UA_HEADER_LENGTH){
        assert(msgBuffer->currentChunkSize > 0);

        if(msgBuffer->buffers->length < msgBuffer->currentChunkSize){
            //incomplete message, continue to read it
            status = STATUS_OK;
        }else if(msgBuffer->buffers->length == msgBuffer->currentChunkSize){
            if(msgBuffer->isFinal == UA_Msg_Chunk_Intermediate){
                status = MsgBuffer_ResetNextChunk(msgBuffer, 0);
            }else{
                MsgBuffer_Reset(msgBuffer);
                // status will be set reading the header
            }
        }else{
            // constraint error: header length <= buffer length <= message size
            status = STATUS_INVALID_PARAMETERS;
        }
    }

    if(msgBuffer->buffers->length < TCP_UA_HEADER_LENGTH){

        // Attempt to read header
        if(msgBuffer->buffers->max_size > TCP_UA_HEADER_LENGTH){
            readBytes = TCP_UA_HEADER_LENGTH - msgBuffer->buffers->length;
            status = UA_Socket_Read(socket, msgBuffer->buffers->data, readBytes, &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Buffer_SetDataLength(msgBuffer->buffers, msgBuffer->buffers->length + readBytes);

                if(msgBuffer->buffers->length == TCP_UA_HEADER_LENGTH){
                    status = TCP_UA_ReadHeader(msgBuffer);
                }else if(msgBuffer->buffers->length > TCP_UA_HEADER_LENGTH){
                    status = STATUS_INVALID_STATE;
                }else{
                    // Incomplete header: Wait for new read event !
                    status = STATUS_OK_INCOMPLETE;
                }
            }else{
                // TODO: manage other statuses disconnect, etc.
            }
        }
    }

    if(status == STATUS_OK){
        if(msgBuffer->buffers->max_size >= msgBuffer->currentChunkSize){

            readBytes = msgBuffer->currentChunkSize - msgBuffer->buffers->length;
            status = UA_Socket_Read(socket,
                                    &(msgBuffer->buffers->data[msgBuffer->buffers->length]),
                                    readBytes,
                                    &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Buffer_SetDataLength(msgBuffer->buffers, msgBuffer->buffers->length + readBytes);
            }
            if(msgBuffer->buffers->length == msgBuffer->currentChunkSize){
                // Message complete just return
                assert(status == STATUS_OK);
            }else if(msgBuffer->buffers->length == msgBuffer->currentChunkSize){
                status = STATUS_INVALID_STATE;
            }else{
                status = STATUS_OK_INCOMPLETE;
            }

        }else{
            status = OpcUa_BadTcpMessageTooLarge;
        }
    }

    return status;
}

StatusCode TCP_UA_ReadHeader(UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_OK;
    UA_Byte msgType[3];
    UA_Byte isFinal;

    if(msgBuffer != NULL
       && msgBuffer->buffers->length == TCP_UA_HEADER_LENGTH
       && msgBuffer->type == TCP_UA_Message_Unknown)
    {
        // READ message type
        status = Buffer_Read(msgType, msgBuffer->buffers, 3);
        if(status == STATUS_OK){
            if(memcmp(msgType, HEL, 3) == 0){
                msgBuffer->type = TCP_UA_Message_Hello;
            }else if(memcmp(msgType, ACK, 3) == 0){
                msgBuffer->type = TCP_UA_Message_Acknowledge;
            }else if(memcmp(msgType, ERR, 3) == 0){
                msgBuffer->type = TCP_UA_Message_Error;
            }

            if(msgBuffer->type == TCP_UA_Message_Unknown){
                // should be a secure message
                if(memcmp(msgType, MSG, 3) == 0){
                    msgBuffer->type = TCP_UA_Message_SecureMessage;
                    msgBuffer->secureType = UA_SecureMessage;
                }else if(memcmp(msgType, OPN, 3) == 0){
                    msgBuffer->type = TCP_UA_Message_SecureMessage;
                    msgBuffer->secureType = UA_OpenSecureChannel;
                }else if(memcmp(msgType, CLO, 3) == 0){
                    msgBuffer->type = TCP_UA_Message_SecureMessage;
                    msgBuffer->secureType = UA_CloseSecureChannel;
                }else{
                    msgBuffer->type = TCP_UA_Message_Invalid;
                    status = OpcUa_BadTcpMessageTypeInvalid;
                }
            }

            assert(msgBuffer->type != TCP_UA_Message_Unknown);
            assert(msgBuffer->type != TCP_UA_Message_Invalid || status != STATUS_OK);
        }

        // READ IsFinal message chunk
        if(status == STATUS_OK)
        {
            status = Buffer_Read(&isFinal, msgBuffer->buffers, 1);
            if(status == STATUS_OK){
                switch(isFinal){
                    case 'C':
                        msgBuffer->isFinal = UA_Msg_Chunk_Intermediate;
                        break;
                    case 'F':
                        msgBuffer->isFinal = UA_Msg_Chunk_Final;
                        break;
                    case 'A':
                        msgBuffer->isFinal = UA_Msg_Chunk_Abort;
                        break;
                    default:
                        msgBuffer->isFinal = UA_Msg_Chunk_Invalid;
                        status = OpcUa_BadTcpMessageTypeInvalid;
                        break;
                }

                //In TCP UA non secure messages reserved byte shall be set to 'F'
                if(msgBuffer->type != TCP_UA_Message_SecureMessage
                   || msgBuffer->secureType == UA_OpenSecureChannel // As indicated by mantis #3378
                   || msgBuffer->secureType == UA_CloseSecureChannel ) // As indicated by mantis #3378
                {
                    if(msgBuffer->isFinal != UA_Msg_Chunk_Final){
                        status = OpcUa_BadTcpMessageTypeInvalid;
                    }
                }
            } // read isFinal chunk
        }

        // READ message size
        if(status == STATUS_OK){
            status = UInt32_Read(&msgBuffer->currentChunkSize, msgBuffer);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    return status;
}

StatusCode TCP_UA_ReadMsgBuffer(UA_Byte*      data_dest,
                                uint32_t      size,
                                UA_MsgBuffer* msgBuffer,
                                uint32_t      count){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    Buffer* buffer = NULL;
    if(data_dest != NULL && size > 0 && msgBuffer != NULL && count > 0){
        // Treat single buffer messages and multiple buffers message (UA_MsgBuffers)
        if(msgBuffer->nbBuffers == 1){
            buffer = msgBuffer->buffers;
        }else if(msgBuffer->nbBuffers > 1 && msgBuffer->nbChunks > 0){
            // TCP UA layer never treat chunks
            assert(msgBuffer->nbChunks == 1);
            buffer = MsgBuffers_GetCurrentChunk(msgBuffer);
            if(buffer == NULL){
                status = STATUS_NOK;
            }
        }

        // Check for size and count values
        if(data_dest == NULL || msgBuffer == NULL
           || size < count
           || buffer->length - buffer->position < count)
        {
            status = STATUS_INVALID_PARAMETERS;
        }else{
            status = Buffer_Read(data_dest, buffer, count);
        }
    }

    return status;
}

StatusCode TCP_UA_FlushMsgBuffer(UA_MsgBuffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    uint32_t writtenBytes = 0;
    writtenBytes = UA_Socket_Write((UA_Socket*) msgBuffer->flushData,
                                   msgBuffer->buffers->data,
                                   msgBuffer->buffers->length);
    if(writtenBytes == msgBuffer->buffers->length){
        status = STATUS_OK;
    }
    // Manage different cases ? (socket error, blocked sending)
    return status;
}


StatusCode TCP_UA_WriteMsgBuffer(UA_MsgBuffer*  msgBuffer,
                                 const UA_Byte* data_src,
                                 uint32_t       count)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(data_src != NULL && msgBuffer != NULL && count > 0)
    {
        if(msgBuffer->buffers->position + count > msgBuffer->buffers->max_size){
            // Error message should be managed at secure channel level:
            //  no possible message chunks except for MSG type !
            status = STATUS_INVALID_STATE;
        }else{
            status = Buffer_Write(msgBuffer->buffers, data_src, count);
        }
    }
    return status;
}
