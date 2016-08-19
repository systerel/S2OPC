/*
 * tcp_ua_connection.h
 *
 *  Created on: Aug 2, 2016
 *      Author: vincent
 */

#include <assert.h>
#include <tcp_ua_low_level.h>
#include <ua_encoder.h>

StatusCode Encode_TCP_UA_Header(UA_Msg_Buffer* msgBuffer,
                                TCP_UA_Message_Type type){
    StatusCode status = STATUS_OK;
    UA_Byte fByte = 'F';
    assert(msgBuffer->buffers->max_size > UA_HEADER_LENGTH);
    switch(type){
        case TCP_UA_Message_Hello:
            status = Write_Buffer(msgBuffer->buffers, HEL, 3);
            break;
        case TCP_UA_Message_Acknowledge:
            status = Write_Buffer(msgBuffer->buffers, ACK, 3);
            break;
        case TCP_UA_Message_Error:
            status = Write_Buffer(msgBuffer->buffers, ERR, 3);
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
        status = Write_UInt32(msgBuffer, UA_HEADER_LENGTH);
        if(status == STATUS_OK){
            msgBuffer->type = type;
            msgBuffer->msgSize = UA_HEADER_LENGTH;
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
    const uint32_t currentPosition = msgBuffer->buffers->position;
    status = Set_Position_Buffer(msgBuffer->buffers, UA_HEADER_LENGTH_POSITION);
    if(status == STATUS_OK){
        status = Write_UInt32(msgBuffer, currentPosition);
    }
    if(status == STATUS_OK){
        status = Set_Position_Buffer(msgBuffer->buffers, currentPosition);
        msgBuffer->msgSize = currentPosition;
    }
    return status;
}


StatusCode Read_TCP_UA_Data(Socket socket,
                            UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    uint32_t readBytes;

    if(msgBuffer->buffers->length >= UA_HEADER_LENGTH){
        assert(msgBuffer->msgSize > 0);

        if(msgBuffer->buffers->length < msgBuffer->msgSize){
            //incomplete message, continue to read it
            status = STATUS_OK;
        }else if(msgBuffer->buffers->length == msgBuffer->msgSize){
            if(msgBuffer->isFinal == UA_Msg_Chunk_Intermediate){
                status = Reset_Msg_Buffer_Next_Chunk(msgBuffer, 0);
            }else{
                Reset_Msg_Buffer(msgBuffer);
                // status will be set reading the header
            }
        }else{
            // constraint error: header length <= buffer length <= message size
            status = STATUS_INVALID_PARAMETERS;
        }
    }

    if(msgBuffer->buffers->length < UA_HEADER_LENGTH){

        // Attempt to read header
        if(msgBuffer->buffers->max_size > UA_HEADER_LENGTH){
            readBytes = UA_HEADER_LENGTH - msgBuffer->buffers->length;
            status = Socket_Read(socket, msgBuffer->buffers->data, readBytes, &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Set_Data_Length_Buffer(msgBuffer->buffers, msgBuffer->buffers->length + readBytes);
            }
            if(msgBuffer->buffers->length == UA_HEADER_LENGTH){
                status = Read_TCP_UA_Header(msgBuffer);
            }else if(msgBuffer->buffers->length > UA_HEADER_LENGTH){
                status = STATUS_INVALID_STATE;
            }else{
                // Incomplete header: Wait for new read event !
                status = STATUS_OK_INCOMPLETE;
            }
        }
    }

    if(status == STATUS_OK){
        if(msgBuffer->buffers->max_size >= msgBuffer->msgSize){

            readBytes = msgBuffer->msgSize - msgBuffer->buffers->length;
            status = Socket_Read(socket,
                                 &(msgBuffer->buffers->data[msgBuffer->buffers->length]),
                                 readBytes,
                                 &readBytes);
            if(status == STATUS_OK && readBytes > 0){
                Set_Data_Length_Buffer(msgBuffer->buffers, msgBuffer->buffers->length + readBytes);
            }
            if(msgBuffer->buffers->length == msgBuffer->msgSize){
                // Message complete just return
                assert(status == STATUS_OK);
            }else if(msgBuffer->buffers->length == msgBuffer->msgSize){
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
       && msgBuffer->buffers->length == UA_HEADER_LENGTH
       && msgBuffer->type == TCP_UA_Message_Unknown)
    {
        // READ message type
        status = Read_Buffer(msgType, msgBuffer->buffers, 3);
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
            status = Read_Buffer(&isFinal, msgBuffer->buffers, 1);
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
            status = Read_UInt32(msgBuffer, &msgBuffer->msgSize);
        }

    }else{
        status = STATUS_INVALID_PARAMETERS;
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
       || msgBuffer->buffers->length - msgBuffer->buffers->position < count)
    {
            status = STATUS_INVALID_PARAMETERS;
    }else{
        status = Read_Buffer(data_dest, msgBuffer->buffers, count);
    }
    return status;
}

StatusCode Flush_Msg_Buffer(UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    int32_t writtenBytes = 0;
    writtenBytes = Socket_Write((Socket) msgBuffer->flushData,
                                msgBuffer->buffers->data,
                                msgBuffer->buffers->length);
    if(writtenBytes == msgBuffer->buffers->length){
        status = STATUS_OK;
    }
    // Manage different cases ? (socket error, blocked sending)
    return status;
}


StatusCode Write_Msg_Buffer(UA_Msg_Buffer* msgBuffer,
                            UA_Byte*       data_src,
                            uint32_t       count)
{
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || msgBuffer == UA_NULL)
    {
            status = STATUS_INVALID_PARAMETERS;
    }else{
        if(msgBuffer->buffers->position + count > msgBuffer->buffers->max_size){
            // Error message should be managed at secure channel level:
            //  no possible message chunks except for MSG type !
            status = STATUS_INVALID_STATE;
        }else{
            status = Write_Buffer(msgBuffer->buffers, data_src, count);
        }
    }
    return status;
}
