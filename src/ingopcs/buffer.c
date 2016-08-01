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
            buf->size = size;
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
            buf->size = size;
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
