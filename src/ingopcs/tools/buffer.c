/*
 * buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

Buffer* Buffer_Create(uint32_t size)
{
    Buffer* buf = NULL;
    if(size > 0){
        buf = (Buffer*) malloc(sizeof(Buffer));
        if(buf != NULL){
            StatusCode status = Buffer_Init(buf, size);
            if(status != STATUS_OK){
                Buffer_Delete(buf);
                buf = NULL;
            }
        }
    }
    return buf;
}

StatusCode Buffer_Init(Buffer* buffer, uint32_t size)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer != NULL && size > 0){
        status = STATUS_OK;
    }
    if(status == STATUS_OK){
        buffer->position = 0;
        buffer->length = 0;
        buffer->max_size = size;
        buffer->data = (uint8_t*) malloc(sizeof(uint8_t)*size);
        memset(buffer->data, 0, sizeof(uint8_t)*size);
    }
    return status;
}

void Buffer_Clear(Buffer* buffer)
{
    if(buffer != NULL){
        if(buffer->data != NULL){
            free(buffer->data);
        }
    }
}

void Buffer_Delete(Buffer* buffer)
{
    if(buffer != NULL){
        Buffer_Clear(buffer);
        free(buffer);
    }
}

void Buffer_Reset(Buffer* buffer)
{
    if(buffer != NULL && buffer->data != NULL){
        buffer->position = 0;
        buffer->length = 0;
        memset(buffer->data, 0, buffer->max_size);
    }
}

StatusCode Buffer_ResetAfterPosition(Buffer*  buffer,
                                     uint32_t position)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer != NULL && buffer->data != NULL &&
       position <= buffer->length)
    {
        status = STATUS_OK;
        buffer->position = position;
        buffer->length = position;
        memset(&(buffer->data[buffer->position]), 0, buffer->max_size);
    }
    return status;
}

StatusCode Buffer_SetPosition(Buffer* buffer, uint32_t position){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer != NULL && buffer->data != NULL &&
       buffer->length >= position){
        status = STATUS_OK;
        buffer->position = position;
    }
    return status;
}

StatusCode Buffer_SetDataLength(Buffer* buffer, uint32_t length)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint8_t* data = NULL;
    if(buffer != NULL && buffer->data != NULL &&
       buffer->max_size >= length &&
       buffer->position <= length){
        data = buffer->data;
        status = STATUS_OK;
        if(buffer->length > length){
            data = &(buffer->data[length]);
            // Reset unused bytes to 0
            memset(data, 0, buffer->length - length);
        }
        buffer->length = length;
    }
    return status;
}

StatusCode Buffer_Write(Buffer* buffer, const uint8_t* data_src, uint32_t count)
{
    StatusCode status = STATUS_NOK;
    if(data_src == NULL || buffer == NULL || buffer->data == NULL){
        status = STATUS_INVALID_PARAMETERS;
    }else{
        if(buffer->position + count > buffer->max_size){
            status = STATUS_INVALID_PARAMETERS;
        }else{
            if(memcpy(&(buffer->data[buffer->position]), data_src, count)
               == &(buffer->data[buffer->position]))
            {
                buffer->position = buffer->position + count;
                // In case we write in existing buffer position: does not change length
                if(buffer->position > buffer->length){
                    buffer->length = buffer->position;
                }
                status = STATUS_OK;
            }else{
                status = STATUS_INVALID_STATE;
            }
        }
    }
    return status;
}

StatusCode Buffer_Read(uint8_t* data_dest, Buffer* buffer, uint32_t count)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if( buffer != NULL && buffer->data != NULL &&
        buffer->position + count <= buffer->length)
    {
        if(memcpy(data_dest, &(buffer->data[buffer->position]), count) == data_dest){
            buffer->position = buffer->position + count;
            status = STATUS_OK;
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}

StatusCode Buffer_CopyWithLength(Buffer* dest, Buffer* src, uint32_t limitedLength)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && src != NULL &&
       dest->data != NULL && src->data != NULL &&
       src->length >= limitedLength && src->position <= limitedLength &&
       limitedLength <= dest->max_size)
    {
        assert(src->position <= src->length);
        status = STATUS_OK;

        memcpy(dest->data, src->data, limitedLength);
        status = Buffer_SetPosition(dest, 0);
        if(status == STATUS_OK){
            status = Buffer_SetDataLength(dest, limitedLength);
        }
        if(status == STATUS_OK){
            status = Buffer_SetPosition(dest, src->position);
        }
    }
    return status;
}


StatusCode Buffer_Copy(Buffer* dest, Buffer* src)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(src != NULL)
    {
        status = Buffer_CopyWithLength(dest, src, src->length);
    }

    return status;
}
