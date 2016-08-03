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
    assert(msgBuffer->buffer->max_size > TCP_UA_HEADER_LENGTH);
    switch(type){
        case TCP_UA_Message_Hello:
            status = Write_Buffer(msgBuffer->buffer, (UA_Byte*) "HEL", 3);
            break;
        case TCP_UA_Message_Acknowledge:
            status = Write_Buffer(msgBuffer->buffer, (UA_Byte*) "ACK", 3);
            break;
        case TCP_UA_Message_Error:
            status = Write_Buffer(msgBuffer->buffer, (UA_Byte*) "ERR", 3);
            break;
        case TCP_UA_Message_SecureChannel:
            // Managed by secure channel layer
            break;
        default:
            // Error case (Invalid or Unknown)
            status = STATUS_NOK;
    }
    if(status == STATUS_OK){
        // reserver byte
        status = Write_Msg_Buffer(msgBuffer, &fByte, 1);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(msgBuffer, TCP_UA_HEADER_LENGTH);
        // Encode header length
        if(status == STATUS_OK){
            msgBuffer->type = type;
            msgBuffer->length = TCP_UA_HEADER_LENGTH;
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
        msgBuffer->length = currentPosition;
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
            // if(msgBuffer->nbChunks + 1 > maxChunks) ???
            // Flush and increase nbChunks
        }
        status = Write_Buffer(msgBuffer->buffer, data_src, count);
    }
    return status;
}

StatusCode Flush_Msg_Buffer(Socket socket,
                            UA_Msg_Buffer* msgBuffer){
    StatusCode status = STATUS_NOK;
    uint32_t writtenBytes = 0;
    writtenBytes = Socket_Write(socket, msgBuffer->buffer->data, msgBuffer->buffer->position);
    if(writtenBytes == msgBuffer->buffer->position){
        status = STATUS_OK;
    }
    return status;
}
