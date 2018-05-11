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

/**
 *  \file sopc_buffer.h
 *
 *  \brief A buffer of bytes with a maximum size, length and position.
 */

#ifndef SOPC_BUFFER_H_
#define SOPC_BUFFER_H_

#include <stdint.h>

#include "sopc_toolkit_constants.h"

/**
 *  \brief Bytes buffer structure
 */
typedef struct
{
    uint32_t max_size; /**< maximum size (allocated bytes) */
    uint32_t position; /**< read/write position */
    uint32_t length;   /**< data length */
    uint8_t* data;     /**< data bytes */
} SOPC_Buffer;

/**
 *  \brief          Allocate a buffer and its data bytes of the given size and returns it
 *
 *  \param size     The size of the data bytes allocated
 *  \return         Pointer on the allocated buffer or NULL if allocation failed
 */
SOPC_Buffer* SOPC_Buffer_Create(uint32_t size);

/**
 *  \brief          Initialize a buffer by allocating data bytes
 *
 *  \param buffer   Pointer to a non allocated buffer (not NULL)
 *  \param size     The size of the buffer
 *  \return         0 if succeeded, non zero otherwise
 */
SOPC_ReturnStatus SOPC_Buffer_Init(SOPC_Buffer* buffer, uint32_t size);

/**
 *  \brief          Deallocate buffer data bytes content
 *
 *  \param buffer   Pointer to the buffer in which data bytes content must be deallocated.
 */
void SOPC_Buffer_Clear(SOPC_Buffer* buffer);

/**
 *  \brief          Deallocate buffer and its data bytes content (Clear + deallocate pointer)
 *
 *  \param buffer   Pointer to the buffer to deallocate (pointer must not be used anymore after operation)
 */
void SOPC_Buffer_Delete(SOPC_Buffer* buffer);

/**
 *  \brief          Reset length, position and data bytes to zero value of an allocated buffer
 *
 *  \param buffer   Pointer to the buffer to reset
 *
 */
void SOPC_Buffer_Reset(SOPC_Buffer* buffer);

/**
 *  \brief          reset data bytes after position (>=) to zero and set buffer position and length to given position
 *
 *  \param buffer   Pointer to the buffer to reset to the given position
 *  \param position New position of the reset buffer (position <= buffer->length)
 *
 *  \return         0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, invalid
 * position)
 */
SOPC_ReturnStatus SOPC_Buffer_ResetAfterPosition(SOPC_Buffer* buffer, uint32_t position);

/**
 *  \brief             Set buffer to the given position
 *
 *  \param buffer      Pointer to the buffer to set to the given position
 *  \param position    New position of the buffer (<= buffer->length)
 *
 *  \return            0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, invalid
 * position)
 */
SOPC_ReturnStatus SOPC_Buffer_SetPosition(SOPC_Buffer* buffer, uint32_t position);

/**
 *  \brief           Set buffer to the given length
 *
 *  \param buffer    Pointer to the buffer to set to the given length
 *  \param length    New length of the buffer (<= buffer->maxsize && >= buffer->position)
 *
 *  \return          0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, invalid
 * length)
 */
SOPC_ReturnStatus SOPC_Buffer_SetDataLength(SOPC_Buffer* buffer, uint32_t length);

/**
 *  \brief             Write the given bytes into the buffer data bytes from the buffer position (adapting buffer
 * position and length if necessary)
 *
 *  \param buffer      Pointer to the buffer to write into
 *  \param data_src    Pointer to the bytes to be write in the buffer (HYP: nb bytes >= count bytes)
 *  \param count       Number of bytes to write in the buffer (count + buffer->position <= buffer->maxsize)
 *
 *  \return            0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, full buffer
 * avoiding operation)
 */
SOPC_ReturnStatus SOPC_Buffer_Write(SOPC_Buffer* buffer, const uint8_t* data_src, uint32_t count);

/**
 *  \brief              Read the given bytes of the buffer data bytes from the buffer position (adapting buffer position
 * to next position to read)
 *
 *  \param data_dest    Pointer to the bytes to set to read bytes value of the buffer (HYP: nb bytes >= count bytes)
 *  \param buffer       Pointer to the buffer to read from
 *  \param count        Number of bytes to read from the buffer (count + buffer->position <= buffer->length)
 *
 *  \return             0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, empty
 * buffer avoiding operation)
 */
SOPC_ReturnStatus SOPC_Buffer_Read(uint8_t* data_dest, SOPC_Buffer* buffer, uint32_t count);

/**
 *  \brief         Copy the data bytes and properties from the source buffer to the destination buffer
 *
 *  \param dest    Pointer to the destination buffer of the copy operation (dest->maxsize >= src->length)
 *  \param src     Pointer to the source buffer of the copy operation
 *
 *  \return        0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, incompatible
 * size)
 */
SOPC_ReturnStatus SOPC_Buffer_Copy(SOPC_Buffer* dest, SOPC_Buffer* src);

/**
 *  \brief                  Copy the data bytes and properties for the given length from the source buffer to the
 * destination buffer
 *
 *  \param dest             Pointer to the destination buffer of the copy operation (dest->maxsize >= limitedLength)
 *  \param src              Pointer to the source buffer of the copy operation (src->length >= limitedLength,
 * src->position <= limitedLength) \param limitedLength    The length to use for the copy, number of bytes copied and
 * length set in destination buffer
 *
 *  \return                 0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content,
 * incompatible size)
 */
SOPC_ReturnStatus SOPC_Buffer_CopyWithLength(SOPC_Buffer* dest, SOPC_Buffer* src, uint32_t limitedLength);

/**
 * \brief Returns the remaining number of unread bytes in the buffer.
 *
 * \param buffer  The buffer.
 *
 * \return The number of unread bytes in the buffer.
 */
uint32_t SOPC_Buffer_Remaining(SOPC_Buffer* buffer);

/**
 * \brief Reads \c n bytes from \c src into \c buffer.
 *
 * \param buffer  The buffer to read the bytes into.
 * \param src     The source buffer.
 * \param n       The maximum number of bytes to read.
 *
 * \return The number of bytes read in case of success, -1 in case of failure.
 */
int64_t SOPC_Buffer_ReadFrom(SOPC_Buffer* buffer, SOPC_Buffer* src, uint32_t n);

#endif /* SOPC_BUFFER_H_ */
