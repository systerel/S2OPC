/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "sopc_buffer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SOPC_Buffer* SOPC_Buffer_Create(uint32_t size)
{
    SOPC_Buffer* buf = NULL;
    if (size > 0)
    {
        buf = (SOPC_Buffer*) malloc(sizeof(SOPC_Buffer));
        if (buf != NULL)
        {
            SOPC_ReturnStatus status = SOPC_Buffer_Init(buf, size);
            if (status != SOPC_STATUS_OK)
            {
                SOPC_Buffer_Delete(buf);
                buf = NULL;
            }
        }
    }
    return buf;
}

SOPC_ReturnStatus SOPC_Buffer_Init(SOPC_Buffer* buffer, uint32_t size)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (buffer != NULL && size > 0)
    {
        status = SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        buffer->position = 0;
        buffer->length = 0;
        buffer->max_size = size;
        buffer->data = (uint8_t*) calloc((size_t) size, sizeof(uint8_t));
        if (buffer->data == NULL)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    return status;
}

SOPC_Buffer* SOPC_Buffer_Attach(uint8_t* data, uint32_t size)
{
    SOPC_Buffer* b = calloc(1, sizeof(SOPC_Buffer));

    if (b == NULL)
    {
        return NULL;
    }

    b->data = data;
    b->length = size;
    b->max_size = size;

    return b;
}

void SOPC_Buffer_Clear(SOPC_Buffer* buffer)
{
    if (buffer != NULL)
    {
        if (buffer->data != NULL)
        {
            free(buffer->data);
            buffer->data = NULL;
        }
    }
}

void SOPC_Buffer_Delete(SOPC_Buffer* buffer)
{
    if (buffer != NULL)
    {
        SOPC_Buffer_Clear(buffer);
        free(buffer);
    }
}

void SOPC_Buffer_Reset(SOPC_Buffer* buffer)
{
    if (buffer != NULL && buffer->data != NULL)
    {
        buffer->position = 0;
        buffer->length = 0;
        memset(buffer->data, 0, buffer->max_size);
    }
}

SOPC_ReturnStatus SOPC_Buffer_ResetAfterPosition(SOPC_Buffer* buffer, uint32_t position)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (buffer != NULL && buffer->data != NULL && position <= buffer->length)
    {
        status = SOPC_STATUS_OK;
        buffer->position = position;
        buffer->length = position;
        memset(&(buffer->data[buffer->position]), 0, buffer->max_size - buffer->position);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_SetPosition(SOPC_Buffer* buffer, uint32_t position)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (buffer != NULL && buffer->data != NULL && buffer->length >= position)
    {
        status = SOPC_STATUS_OK;
        buffer->position = position;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_SetDataLength(SOPC_Buffer* buffer, uint32_t length)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    uint8_t* data = NULL;
    if (buffer != NULL && buffer->data != NULL && buffer->max_size >= length && buffer->position <= length)
    {
        status = SOPC_STATUS_OK;
        if (buffer->length > length)
        {
            data = &(buffer->data[length]);
            // Reset unused bytes to 0
            memset(data, 0, buffer->length - length);
        }
        buffer->length = length;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_Write(SOPC_Buffer* buffer, const uint8_t* data_src, uint32_t count)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL == data_src || NULL == buffer || NULL == buffer->data)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        if (buffer->position + count > buffer->max_size)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            if (memcpy(&(buffer->data[buffer->position]), data_src, count) == &(buffer->data[buffer->position]))
            {
                buffer->position = buffer->position + count;
                // In case we write in existing buffer position: does not change length
                if (buffer->position > buffer->length)
                {
                    buffer->length = buffer->position;
                }
                status = SOPC_STATUS_OK;
            }
            else
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_Read(uint8_t* data_dest, SOPC_Buffer* buffer, uint32_t count)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (buffer != NULL && buffer->data != NULL && buffer->position + count <= buffer->length)
    {
        if (memcpy(data_dest, &(buffer->data[buffer->position]), count) == data_dest)
        {
            buffer->position = buffer->position + count;
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_CopyWithLength(SOPC_Buffer* dest, SOPC_Buffer* src, uint32_t limitedLength)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && src != NULL && dest->data != NULL && src->data != NULL && src->length >= limitedLength &&
        src->position <= limitedLength && limitedLength <= dest->max_size)
    {
        assert(src->position <= src->length);

        memcpy(dest->data, src->data, limitedLength);
        status = SOPC_Buffer_SetPosition(dest, 0);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_SetDataLength(dest, limitedLength);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_SetPosition(dest, src->position);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_Copy(SOPC_Buffer* dest, SOPC_Buffer* src)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (src != NULL)
    {
        status = SOPC_Buffer_CopyWithLength(dest, src, src->length);
    }

    return status;
}

uint32_t SOPC_Buffer_Remaining(SOPC_Buffer* buffer)
{
    assert(buffer != NULL);
    return buffer->length - buffer->position;
}

int64_t SOPC_Buffer_ReadFrom(SOPC_Buffer* buffer, SOPC_Buffer* src, uint32_t n)
{
    if ((buffer->max_size - buffer->length) < n)
    {
        return -1;
    }

    uint32_t available = src->length - src->position;

    if (available < n)
    {
        n = available;
    }

    memcpy(buffer->data + buffer->position, src->data + src->position, n * sizeof(uint8_t));
    buffer->length += n;
    src->position += n;
    return (int64_t) n;
}

static long get_file_size(FILE* fd)
{
    if (fseek(fd, 0, SEEK_END) == -1)
    {
        return -1;
    }

    long sz = ftell(fd);

    if (sz == -1)
    {
        return -1;
    }

    if (fseek(fd, 0, SEEK_SET) == -1)
    {
        return -1;
    }

    return sz;
}

static bool read_file(FILE* fd, char* data, size_t len)
{
    size_t read = 0;

    while (true)
    {
        size_t res = fread(data + read, sizeof(char), len - read, fd);

        if (res == 0)
        {
            break;
        }

        read += res;
    }

    return ferror(fd) == 0;
}

SOPC_ReturnStatus SOPC_Buffer_ReadFile(const char* path, SOPC_Buffer** buf)
{
    FILE* fd = fopen(path, "rb");

    if (fd == NULL)
    {
        return SOPC_STATUS_NOK;
    }

    long size = get_file_size(fd);

    if (size == -1 || ((unsigned long) size) > UINT32_MAX || ((unsigned long) size) > SIZE_MAX)
    {
        fclose(fd);
        return SOPC_STATUS_NOK;
    }

    SOPC_Buffer* buffer = SOPC_Buffer_Create((uint32_t) size);

    if (buffer == NULL)
    {
        fclose(fd);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    bool ok = read_file(fd, (char*) buffer->data, (size_t) size);
    fclose(fd);

    if (!ok)
    {
        return SOPC_STATUS_NOK;
    }

    buffer->length = buffer->max_size;
    *buf = buffer;
    return SOPC_STATUS_OK;
}
