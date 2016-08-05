/*
 * tcp_ua_connection.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#include <assert.h>
#include <tcp_ua_low_level.h>
#include <ua_encoder.h>

const UA_Byte HEL[3] = {'H','E','L'};
const UA_Byte ACK[3] = {'A','C','K'};
const UA_Byte ERR[3] = {'E','R','R'};
const UA_Byte MSG[3] = {'M','S','G'};
const UA_Byte OPN[3] = {'O','P','N'};
const UA_Byte CLO[3] = {'C','L','O'};

StatusCode Encode_TCP_UA_Header(UA_Msg_Buffer* msgBuffer,
                                TCP_UA_Message_Type type){
    StatusCode status = STATUS_OK;
    UA_Byte fByte = 'F';
    assert(msgBuffer->buffer->max_size > TCP_UA_HEADER_LENGTH);
    switch(type){
        case TCP_UA_Message_Hello:
            status = Write_Buffer(msgBuffer->buffer, HEL, 3);
            break;
        case TCP_UA_Message_Acknowledge:
            status = Write_Buffer(msgBuffer->buffer, ACK, 3);
            break;
        case TCP_UA_Message_Error:
            status = Write_Buffer(msgBuffer->buffer, ERR, 3);
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
        status = Write_Msg_Buffer(msgBuffer, &fByte, 1);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(msgBuffer, TCP_UA_HEADER_LENGTH);
        if(status == STATUS_OK){
            msgBuffer->type = type;
            msgBuffer->msgSize = TCP_UA_HEADER_LENGTH;
            msgBuffer->nbChunks = 1;
        }
    }
    return status;
}

StatusCode Finalize_TCP_UA_Header(UA_Msg_Buffer* msgBuffer){
    assert(msgBuffer->type == TCP_UA_Message_Hello
           || msgBuffer->type == TCP_UA_Message_Acknowledge
           || msgBuffer->type == TCP_UA_Message_Error);
    StatusCode status = STATUS_NOK;
    const uint32_t currentPosition = msgBuffer->buffer->position;
    status = Set_Position_Buffer(msgBuffer->buffer, TCP_UA_HEADER_LENGTH_POSITION);
    if(status == STATUS_OK){
        status = Write_UInt32(msgBuffer, currentPosition);
    }
    if(status == STATUS_OK){
        status = Set_Position_Buffer(msgBuffer->buffer, currentPosition);
        msgBuffer->msgSize = currentPosition;
    }
    return status;
}


StatusCode Read_TCP_UA_Data(Socket socket,
                            UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    uint32_t readBytes;

    if(msgBuffer->buffer->length >= TCP_UA_HEADER_LENGTH){
        assert(msgBuffer->msgSize > 0);

        if(msgBuffer->buffer->length < msgBuffer->msgSize){
            //incomplete message, continue to read it
            status = STATUS_OK;
        }else if(msgBuffer->buffer->length == msgBuffer->msgSize){
            if(msgBuffer->isFinal == UA_Msg_Chunk_Intermediate){
                status = Reset_Msg_Buffer_Next_Chunk(msgBuffer);
            }else{
                Reset_Msg_Buffer(msgBuffer);
                // status will be set reading the header
            }
        }else{
            // constraint error: header length <= buffer length <= message size
            status = STATUS_INVALID_PARAMETERS;
        }
    }

    if(msgBuffer->buffer->length < TCP_UA_HEADER_LENGTH){

        // Attempt to read header
        if(msgBuffer->buffer->max_size > TCP_UA_HEADER_LENGTH){
            readBytes = TCP_UA_HEADER_LENGTH - msgBuffer->buffer->length;
            status = Socket_Read(socket, msgBuffer->buffer->data, readBytes, &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Set_Data_Length_Buffer(msgBuffer->buffer, msgBuffer->buffer->length + readBytes);
            }
            if(msgBuffer->buffer->length == TCP_UA_HEADER_LENGTH){
                status = Read_TCP_UA_Header(msgBuffer);
            }else if(msgBuffer->buffer->length > TCP_UA_HEADER_LENGTH){
                status = STATUS_INVALID_STATE;
            }else{
                // Incomplete header: Wait for new read event !
                status = STATUS_OK_INCOMPLETE;
            }
        }
    }

    if(status == STATUS_OK){
        if(msgBuffer->buffer->max_size >= msgBuffer->msgSize){

            readBytes = msgBuffer->msgSize - msgBuffer->buffer->length;
            status = Socket_Read(socket,
                                 &(msgBuffer->buffer->data[msgBuffer->buffer->length]),
                                 readBytes,
                                 &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Set_Data_Length_Buffer(msgBuffer->buffer, msgBuffer->buffer->length + readBytes);
            }
            if(msgBuffer->buffer->length == msgBuffer->msgSize){
                // Message complete just return
                assert(status == STATUS_OK);
            }else if(msgBuffer->buffer->length == msgBuffer->msgSize){
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

StatusCode Read_TCP_UA_Header(UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_OK;
    UA_Byte msgType[3];
    UA_Byte isFinal;

    if(msgBuffer != UA_NULL
       && msgBuffer->buffer->length == TCP_UA_HEADER_LENGTH
       && msgBuffer->type == TCP_UA_Message_Unknown)
    {
        // READ message type
        status = Read_Buffer(msgType, msgBuffer->buffer, 3);
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
                }else if(memcmp(msgType, ERR, 3) == 0){
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
            status = Read_Buffer(&isFinal, msgBuffer->buffer, 1);
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
                   && msgBuffer->isFinal != UA_Msg_Chunk_Final)
                {
                    status = STATUS_INVALID_RCV_PARAMETER;
                }
            } // read isFinal chunk
        }

        // READ message size
        if(status == STATUS_OK){
            status = Read_UInt32(msgBuffer, &msgBuffer->msgSize);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    return status;
}

StatusCode Write_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                            UA_Byte* data_src,
                            uint32_t count){
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || msgBuffer == UA_NULL){
            status = STATUS_INVALID_PARAMETERS;
    }else{
        if(msgBuffer->buffer->position + count > msgBuffer->buffer->max_size){
            if(msgBuffer->nbChunks + 1 > msgBuffer->maxChunks){
                // Error message should be managed at secure channel level
                status = STATUS_INVALID_STATE;
            }else{
                Flush_Msg_Buffer(msgBuffer);
                Reset_Msg_Buffer_Next_Chunk(msgBuffer);
            }
        }
        status = Write_Buffer(msgBuffer->buffer, data_src, count);
    }
    return status;
}

StatusCode Read_Msg_Buffer(UA_Byte* data_dest,
                           uint32_t size,
                           UA_Msg_Buffer* msgBuffer,
                           uint32_t count){
    StatusCode status = STATUS_NOK;
    if(data_dest == UA_NULL || msgBuffer == UA_NULL
       || size < count
       || msgBuffer->buffer->length - msgBuffer->buffer->position < count)
    {
            status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Read_Buffer(data_dest, msgBuffer->buffer, count);
    }
    return status;
}

StatusCode Flush_Msg_Buffer(UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    int32_t writtenBytes = 0;
    writtenBytes = Socket_Write(msgBuffer->socket,
                                msgBuffer->buffer->data,
                                msgBuffer->buffer->position);
    if(writtenBytes == msgBuffer->buffer->position){
        status = STATUS_OK;
    }
    // Manage different cases ? (socket error, blocked sending)
    return status;
}
