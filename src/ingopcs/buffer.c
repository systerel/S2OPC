/*
 * buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <buffer.h>

Buffer* Buffer_Create(uint32_t size)
{
    Buffer* buf = UA_NULL;
    if(size > 0){
        buf = (Buffer*) malloc(sizeof(Buffer));
        if(buf != UA_NULL){
            StatusCode status = Buffer_Init(buf, size);
            if(status != STATUS_OK){
                Buffer_Delete(buf);
                buf = UA_NULL;
            }
        }
    }
    return buf;
}

StatusCode Buffer_Init(Buffer* buffer, uint32_t size)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer != UA_NULL && size > 0){
        status = STATUS_OK;
    }
    if(status == STATUS_OK){
        buffer->position = 0;
        buffer->length = 0;
        buffer->max_size = size;
        buffer->data = (UA_Byte*) malloc(sizeof(UA_Byte)*size);
        memset(buffer->data, 0, sizeof(UA_Byte)*size);
    }
    return status;
}

Buffer* Buffer_Set_Data(UA_Byte* data, uint32_t position, uint32_t length, uint32_t maxsize)
{
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

void Buffer_Delete(Buffer* buffer)
{
    if(buffer != UA_NULL){
        if(buffer->data != UA_NULL){
            free(buffer->data);
        }
        free(buffer);
    }
}

void Buffer_Reset(Buffer* buffer)
{
    buffer->position = 0;
    buffer->length = 0;
    memset(buffer->data, 0, buffer->max_size);
}

void Buffer_ResetAfterPosition(Buffer*  buffer,
                               uint32_t position)
{
    buffer->position = position;
    buffer->length = position;
    memset(&(buffer->data[buffer->position]), 0, buffer->max_size);
}

StatusCode Buffer_SetPosition(Buffer* buffer, uint32_t position){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer->max_size >= position){
        status = STATUS_OK;
        buffer->position = position;
    }
    return status;
}

StatusCode Buffer_SetDataLength(Buffer* buffer, uint32_t length)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    UA_Byte* data = buffer->data;
    if(buffer != UA_NULL && buffer->max_size >= length){
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

StatusCode Buffer_Write(Buffer* buffer, const UA_Byte* data_src, uint32_t count)
{
    StatusCode status = STATUS_NOK;
    if(data_src == UA_NULL || buffer == UA_NULL){
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

StatusCode Buffer_Read(UA_Byte* data_dest, Buffer* buffer, uint32_t count)
{
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

StatusCode Buffer_CopyWithLength(Buffer* dest, Buffer* src, uint32_t limitedLength)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != UA_NULL && src != UA_NULL &&
       src->length >= limitedLength &&
       src->length <= dest->max_size)
    {
        assert(src->position <= src->length);
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        memcpy(dest->data, src->data, limitedLength);
        status = Buffer_SetDataLength(dest, limitedLength);
        if(status == STATUS_OK){
            status = Buffer_SetPosition(dest, src->position);
        }
    }
    return status;
}


StatusCode Buffer_Copy(Buffer* dest, Buffer* src)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(src != UA_NULL)
    {
        status = Buffer_CopyWithLength(dest, src, src->length);
    }

    return status;
}
