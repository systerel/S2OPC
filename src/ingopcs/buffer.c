/*
 * buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <stdlib.h>
#include <string.h>
#include <buffer.h>

Buffer* Create_Buffer(uint32_t size){
    Buffer* buf = UA_NULL;
    if(size > 0){
        buf = (Buffer*) malloc(sizeof(Buffer));
        if(buf != UA_NULL){
            buf->position = 0;
            buf->length = 0;
            buf->max_size = size;
            buf->data = (UA_Byte*) malloc(sizeof(UA_Byte)*size);
            memset(buf->data, 0, sizeof(UA_Byte)*size);
        }
    }
    return buf;
}

Buffer* Set_Buffer(UA_Byte* data, uint32_t position, uint32_t length, uint32_t maxsize){
    Buffer* buf = UA_NULL;
    if(data != UA_NULL && length > 0){
        buf = (Buffer*) malloc(sizeof(Buffer));
        if(buf != UA_NULL){
            buf->length = length;
            buf->data = data;
            buf->position = position;
            buf->max_size = maxsize;
        }
    }
    return buf;
}

void Delete_Buffer(Buffer* buffer){
    if(buffer != UA_NULL){
        if(buffer->data != UA_NULL){
            free(buffer->data);
        }
        free(buffer);
    }
}

void Reset_Buffer(Buffer* buffer){
    buffer->position = 0;
    buffer->length = 0;
    memset(buffer->data, 0, buffer->max_size);
}

void Reset_Buffer_After_Position(Buffer*  buffer,
                                 uint32_t position){
    buffer->position = position;
    buffer->length = position;
    memset(&(buffer->data[buffer->position]), 0, buffer->max_size);
}

StatusCode Set_Position_Buffer(Buffer* buffer, uint32_t position){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer->max_size >= position){
        status = STATUS_OK;
        buffer->position = position;
    }
    return status;
}

StatusCode Set_Data_Length_Buffer(Buffer* buffer, uint32_t length){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer->max_size >= length){
        status = STATUS_OK;
        buffer->length = length;
    }
    return status;
}

StatusCode Write_Buffer(Buffer* buffer, const UA_Byte* data_src, uint32_t count){
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || buffer == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        if(buffer->position + count > buffer->max_size){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            if(memcpy(&(buffer->data[buffer->position]), data_src, count) == &(buffer->data[buffer->position])){
                buffer->position = buffer->position + count;
                buffer->length = buffer->position;
                status = STATUS_OK;
            }else{
                status = STATUS_INVALID_STATE;
            }
        }
    }
    return status;
}

StatusCode Read_Buffer(UA_Byte* data_dest, Buffer* buffer, uint32_t count){
    StatusCode status = STATUS_NOK;
    if(buffer->position + count > buffer->length){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        if(memcpy(data_dest, &(buffer->data[buffer->position]), count) == data_dest){
            buffer->position = buffer->position + count;
            status = STATUS_OK;
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}
