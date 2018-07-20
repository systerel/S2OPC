/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "sopc_buffer.h"
#include <assert.h>
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
#ifdef __TRUSTINSOFT_HELPER__
      // use tis_memcpy_bounded instead of memcpy
      void *tis_memcpy_bounded(void *dest, const void *src, size_t n,
                               void * dest_bound, void * src_bound);
      //@ assert bw_a_count: val: 0 < count <= 65535;
      //@ assert bw_a_pos_uint: val: buffer->position < 65535;
      //@ assert bw_a_pos: wp: buffer->position < buffer->max_size;
      //@ assert bw_a_pos2: buffer->position <= buffer->max_size - count;
      //@ assert bw_a_count: wp: count <= buffer->max_size - buffer->position;
            if (tis_memcpy_bounded(&(buffer->data[buffer->position]), data_src, count, buffer->data + buffer->max_size, data_src+count) == &(buffer->data[buffer->position]))
#else
            if (memcpy(&(buffer->data[buffer->position]), data_src, count) == &(buffer->data[buffer->position]))
#endif
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

#ifdef __TRUSTINSOFT_HELPER__
// spec for SOPC_Buffer_Read (TODO: remove ?)
/*@
//   assigns data_dest[0..count-1] \from count, buffer->data[0..count-1];
//   assigns \result \from buffer->length, count, buffer->position,
//                   indirect:buffer, indirect:data_dest; // TODO: enhance ?
//   assigns buffer->position \from buffer->position, count;
  ensures br_e_status: \result > 0 || \result == 0;
  ensures br_e_init:  \result != 0
          || (\result == 0 && \initialized (data_dest + (0..count-1)));
  ensures br_e_position:
    (\result != 0 && buffer->position == \old (buffer->position))
    || (\result == 0 && buffer->position == \old (buffer->position) + count);
*/
#endif
SOPC_ReturnStatus SOPC_Buffer_Read(uint8_t* data_dest, SOPC_Buffer* buffer, uint32_t count)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (buffer != NULL && buffer->data != NULL && buffer->position + count <= buffer->length)
    {
#ifdef __TRUSTINSOFT_HELPER__
      // use tis_memcpy_bounded instead of memcpy
      void *tis_memcpy_bounded(void *dest, const void *src, size_t n,
                               void * dest_bound, void * src_bound);
      //@ assert br_a_pos: wp: buffer->position < buffer->length;
      //@ assert br_a_pos2: buffer->position <= buffer->length - count;
      //@ assert br_a_count: wp: count <= buffer->length - buffer->position;
        if(tis_memcpy_bounded(data_dest, &(buffer->data[buffer->position]), count, data_dest+count, buffer->data + buffer->max_size) == data_dest){
          //@ assert br_a_init: \initialized (data_dest + (0..count-1));
#else
        if (memcpy(data_dest, &(buffer->data[buffer->position]), count) == data_dest)
        {
#endif
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
