/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
            SOPC_StatusCode status = Buffer_Init(buf, size);
            if(status != STATUS_OK){
                Buffer_Delete(buf);
                buf = NULL;
            }
        }
    }
    return buf;
}

SOPC_StatusCode Buffer_Init(Buffer* buffer, uint32_t size)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
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
            buffer->data = NULL;
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

SOPC_StatusCode Buffer_ResetAfterPosition(Buffer*  buffer,
                                     uint32_t position)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer != NULL && buffer->data != NULL &&
       position <= buffer->length)
    {
        status = STATUS_OK;
        buffer->position = position;
        buffer->length = position;
        memset(&(buffer->data[buffer->position]), 0, buffer->max_size - buffer->position);
    }
    return status;
}

SOPC_StatusCode Buffer_SetPosition(Buffer* buffer, uint32_t position){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(buffer != NULL && buffer->data != NULL &&
       buffer->length >= position){
        status = STATUS_OK;
        buffer->position = position;
    }
    return status;
}

SOPC_StatusCode Buffer_SetDataLength(Buffer* buffer, uint32_t length)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
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

SOPC_StatusCode Buffer_Write(Buffer* buffer, const uint8_t* data_src, uint32_t count)
{
    SOPC_StatusCode status = STATUS_NOK;
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

SOPC_StatusCode Buffer_Read(uint8_t* data_dest, Buffer* buffer, uint32_t count)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
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

SOPC_StatusCode Buffer_CopyWithLength(Buffer* dest, Buffer* src, uint32_t limitedLength)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
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


SOPC_StatusCode Buffer_Copy(Buffer* dest, Buffer* src)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(src != NULL)
    {
        status = Buffer_CopyWithLength(dest, src, src->length);
    }

    return status;
}
