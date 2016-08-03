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
            buf->max_size = size;
            buf->data = (UA_Byte*) malloc(sizeof(UA_Byte)*size);
            memset(buf->data, 0, sizeof(UA_Byte)*size);
        }
    }
    return buf;
}

Buffer* Set_Buffer(UA_Byte* data, uint32_t size){
    Buffer* buf = UA_NULL;
    if(data != UA_NULL && size > 0){
        buf = (Buffer*) malloc(sizeof(Buffer));
        if(buf != UA_NULL){
            buf->position = size;
            buf->data = data;
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

StatusCode Set_Position_Buffer(Buffer* buffer, uint32_t position){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer->max_size >= position){
        status = STATUS_OK;
        buffer->position = position;
    }
    return status;
}

StatusCode Write_Buffer(Buffer* buffer, UA_Byte* data_src, uint32_t count){
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || buffer == UA_NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        if(buffer->position + count > buffer->max_size){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            if(memcpy(&(buffer->data[buffer->position]), data_src, count) == &(buffer->data[buffer->position])){
                buffer->position = buffer->position + count;
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
    if(buffer->position + count > buffer->max_size){
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
