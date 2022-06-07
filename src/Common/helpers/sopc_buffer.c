/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sopc_buffer.h"
#include "sopc_mem_alloc.h"

static SOPC_ReturnStatus SOPC_Buffer_Init(SOPC_Buffer* buffer, uint32_t initial_size, uint32_t maximum_size)
{
    if (buffer == NULL || initial_size <= 0 || initial_size > maximum_size)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    buffer->data = SOPC_Calloc((size_t) initial_size, sizeof(uint8_t));
    if (NULL == buffer->data)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    buffer->position = 0;
    buffer->length = 0;
    buffer->initial_size = initial_size;
    buffer->current_size = initial_size;
    buffer->maximum_size = maximum_size;

    return SOPC_STATUS_OK;
}

SOPC_Buffer* SOPC_Buffer_Create(uint32_t size)
{
    SOPC_Buffer* buf = NULL;
    if (size > 0)
    {
        buf = SOPC_Malloc(sizeof(SOPC_Buffer));
        if (buf != NULL)
        {
            SOPC_ReturnStatus status = SOPC_Buffer_Init(buf, size, size);
            if (status != SOPC_STATUS_OK)
            {
                SOPC_Buffer_Delete(buf);
                buf = NULL;
            }
        }
    }
    return buf;
}

SOPC_Buffer* SOPC_Buffer_CreateResizable(uint32_t initial_size, uint32_t maximum_size)
{
    SOPC_Buffer* buf = NULL;
    if (initial_size > 0)
    {
        buf = SOPC_Calloc(1, sizeof(SOPC_Buffer));
        if (buf != NULL)
        {
            SOPC_ReturnStatus status = SOPC_Buffer_Init(buf, initial_size, maximum_size);
            if (status != SOPC_STATUS_OK)
            {
                SOPC_Buffer_Delete(buf);
                buf = NULL;
            }
        }
    }
    return buf;
}

SOPC_Buffer* SOPC_Buffer_Attach(uint8_t* data, uint32_t size)
{
    SOPC_Buffer* b = SOPC_Calloc(1, sizeof(SOPC_Buffer));

    if (b == NULL)
    {
        return NULL;
    }

    b->data = data;
    b->length = size;
    b->initial_size = size;
    b->current_size = size;
    b->maximum_size = size;

    return b;
}

void SOPC_Buffer_Clear(SOPC_Buffer* buffer)
{
    if (buffer != NULL)
    {
        if (buffer->data != NULL)
        {
            SOPC_Free(buffer->data);
            buffer->data = NULL;
        }
    }
}

void SOPC_Buffer_Delete(SOPC_Buffer* buffer)
{
    if (buffer != NULL)
    {
        SOPC_Buffer_Clear(buffer);
        SOPC_Free(buffer);
    }
}

void SOPC_Buffer_Reset(SOPC_Buffer* buffer)
{
    if (buffer != NULL && buffer->data != NULL)
    {
        buffer->position = 0;
        buffer->length = 0;
        memset(buffer->data, 0, buffer->current_size);
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
        memset(&(buffer->data[buffer->position]), 0, buffer->current_size - buffer->position);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_GetPosition(SOPC_Buffer* buffer, uint32_t* position)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != buffer && NULL != position)
    {
        status = SOPC_STATUS_OK;
        *position = buffer->position;
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
    if (buffer != NULL && buffer->data != NULL && buffer->current_size >= length && buffer->position <= length)
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

/**
 * \brief Check the size of buffer can contains \p totalNbBytes.
 *        If it does not and the buffer is resizable:
 *        - (if \p exactResize == false) Compute new size to be the first multiple of initial_size
 *          that contains \p totalNbBytes or maximum_size if greater than maximum_size
 *        - Resize the buffer to the new size
 *
 *  \return true if buffer is large enough to contains \p totalNbBytes (with or without resize operation),
 *          false otherwise
 */
static bool SOPC_Buffer_CheckSizeAndResize(SOPC_Buffer* buffer, uint32_t totalNbBytes, bool exactResize)
{
    assert(buffer != NULL);
    if (totalNbBytes <= buffer->current_size)
    {
        // Enough bytes available in current buffer allocated bytes
        return true;
    }
    else if (totalNbBytes <= buffer->maximum_size)
    {
        // Enough bytes available if buffer is resized
        uint8_t* newData = NULL;
        uint32_t newSize = 0;
        if (exactResize)
        {
            // Use the exact number of bytes necessary
            newSize = totalNbBytes;
        }
        else
        {
            // Search the first multiple of initial_size which contains totalNbBytes
            uint32_t requiredSteps = totalNbBytes / buffer->initial_size;
            if (totalNbBytes % buffer->initial_size != 0)
            {
                requiredSteps++;
            }
            if (requiredSteps > buffer->maximum_size / buffer->initial_size)
            {
                // The first multiple found is greater than maximum_size => use maximum_size
                newSize = buffer->maximum_size;
            }
            else
            {
                // Resize to the multiple of initial_size which contains totalNbBytes
                newSize = requiredSteps * buffer->initial_size;
            }
        }
        // Resize buffer with computed size
        newData = SOPC_Realloc(buffer->data, (size_t) buffer->current_size, (size_t) newSize);
        if (NULL != newData)
        {
            buffer->data = newData;
            buffer->current_size = newSize;

            return true;
        }
    }
    return false;
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
        if (SOPC_Buffer_CheckSizeAndResize(buffer, buffer->position + count, false))
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
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_Read(uint8_t* data_dest, SOPC_Buffer* buffer, uint32_t count)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (buffer != NULL && buffer->data != NULL)
    {
        if (buffer->position + count <= buffer->length)
        {
            if (NULL == data_dest || memcpy(data_dest, &(buffer->data[buffer->position]), count) == data_dest)
            {
                buffer->position = buffer->position + count;
                status = SOPC_STATUS_OK;
            }
            else
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_Buffer_CopyWithLength(SOPC_Buffer* dest, SOPC_Buffer* src, uint32_t limitedLength)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (dest != NULL && src != NULL && dest->data != NULL && src->data != NULL && src->length >= limitedLength &&
        src->position <= limitedLength)
    {
        assert(src->position <= src->length);

        if (SOPC_Buffer_CheckSizeAndResize(dest, limitedLength, true))
        {
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
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
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
    if (NULL == buffer || NULL == src || (buffer->current_size - buffer->length) < n)
    {
        return -1;
    }

    uint32_t available = src->length - src->position;

    if (available < n)
    {
        n = available;
    }

    memcpy(buffer->data + buffer->length, src->data + src->position, n * sizeof(uint8_t));
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

    if ((-1 == size || 0 == size || ((unsigned long) size) > UINT32_MAX))
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
        SOPC_Buffer_Delete(buffer);
        return SOPC_STATUS_NOK;
    }

    buffer->length = buffer->current_size;
    *buf = buffer;
    return SOPC_STATUS_OK;
}
