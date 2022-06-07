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

/**
 *  \file sopc_buffer.h
 *
 *  \brief A buffer of bytes with a maximum size, length and position.
 */

#ifndef SOPC_BUFFER_H_
#define SOPC_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_enums.h"

/**
 *  \brief Bytes buffer structure
 */
typedef struct
{
    uint32_t initial_size; /**< initial size (also used as size increment step) */
    uint32_t current_size; /**< current size */
    uint32_t maximum_size; /**< maximum size */
    uint32_t position;     /**< read/write position */
    uint32_t length;       /**< data length */
    uint8_t* data;         /**< data bytes */
} SOPC_Buffer;

/**
 *  \brief          Allocate a buffer and its data bytes of the given definitive size and returns it
 *
 *  \param size     The size of the data bytes allocated (static size).
 *  \return         Pointer on the allocated buffer or NULL if allocation failed
 */
SOPC_Buffer* SOPC_Buffer_Create(uint32_t size);

/**
 *  \brief              Allocate a resizable buffer and its initial and maximum data bytes
 *
 *  \param initial_size The size of the data bytes allocated initially (also used as increment step)
 *  \param maximum_size The size of the maximum data bytes finally allocated
 *  \return             Pointer on the allocated buffer or NULL if allocation failed
 */
SOPC_Buffer* SOPC_Buffer_CreateResizable(uint32_t initial_size, uint32_t maximum_size);

/**
 * \brief Wraps a raw memory area into an SOPC_Buffer.
 *
 * \param data  A pointer the memory zone.
 * \param size  The size of the memory zone.
 *
 * \return The created SOPC_Buffer, or NULL on memory allocation failure.
 *
 * The ownership of the memory area is tranfered to the SOPC_Buffer. In other
 * words, the memory will be freed when \ref SOPC_Buffer_Clear or
 * \ref SOPC_Buffer_Delete is called.
 */
SOPC_Buffer* SOPC_Buffer_Attach(uint8_t* data, uint32_t size);

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
 *  \return         SOPC_STATUS_OK if succeeded, SOPC_STATUS_INVALID_PARAMETERS otherwise (NULL pointer, non allocated
 * buffer content, invalid position)
 */
SOPC_ReturnStatus SOPC_Buffer_ResetAfterPosition(SOPC_Buffer* buffer, uint32_t position);

/**
 *  \brief             Get buffer current position
 *
 *  \param buffer      Pointer to the buffer from which to get current position
 *  \param position    Non-NULL pointer to store the current position of the buffer
 *
 *  \return            SOPC_STATUS_OK if succeeded, SOPC_STATUS_INVALID_PARAMETERS otherwise (NULL pointer, non
 * allocated buffer content)
 */
SOPC_ReturnStatus SOPC_Buffer_GetPosition(SOPC_Buffer* buffer, uint32_t* position);

/**
 *  \brief             Set buffer to the given position
 *
 *  \param buffer      Pointer to the buffer to set to the given position
 *  \param position    New position of the buffer (<= buffer->length)
 *
 *  \return            SOPC_STATUS_OK if succeeded, SOPC_STATUS_INVALID_PARAMETERS otherwise (NULL pointer, non
 * allocated buffer content, invalid position)
 */
SOPC_ReturnStatus SOPC_Buffer_SetPosition(SOPC_Buffer* buffer, uint32_t position);

/**
 *  \brief           Set buffer to the given length
 *
 *  \param buffer    Pointer to the buffer to set to the given length
 *  \param length    New length of the buffer (<= buffer->maxsize && >= buffer->position)
 *
 *  \return          SOPC_STATUS_OK if succeeded, SOPC_STATUS_INVALID_PARAMETERS otherwise (NULL pointer, non allocated
 * buffer content, invalid length)
 */
SOPC_ReturnStatus SOPC_Buffer_SetDataLength(SOPC_Buffer* buffer, uint32_t length);

/**
 *  \brief             Write the given bytes into the buffer data bytes from the buffer position (adapting buffer
 * position and length if necessary)
 *
 *  \param buffer      Pointer to the buffer to write into
 *  \param data_src    Pointer to the bytes to be written in the buffer (HYP: nb bytes >= count bytes)
 *  \param count       Number of bytes to write in the buffer (count + buffer->position <= buffer->maxsize)
 *
 *  \return            SOPC_STATUS_OK if succeeded, an error code otherwise (NULL pointer, non allocated buffer
 * content, full buffer avoiding operation)
 */
SOPC_ReturnStatus SOPC_Buffer_Write(SOPC_Buffer* buffer, const uint8_t* data_src, uint32_t count);

/**
 *  \brief              Read the given bytes of the buffer data bytes from the buffer position (adapting buffer position
 * to next position to read)
 *
 *  \param data_dest    Pointer to the bytes to set to read bytes value of the buffer (HYP: nb bytes >= count bytes)
 *                      If NULL ,then data are simply skipped
 *  \param buffer       Pointer to the buffer to read from
 *  \param count        Number of bytes to read from the buffer (count + buffer->position <= buffer->length)
 *
 *  \return             SOPC_STATUS_OK if succeeded, an error code otherwise (NULL pointer, non allocated buffer
 * content, empty buffer avoiding operation)
 */
SOPC_ReturnStatus SOPC_Buffer_Read(uint8_t* data_dest, SOPC_Buffer* buffer, uint32_t count);

/**
 *  \brief         Copy the data bytes and properties from the source buffer to the destination buffer
 *
 *  \param dest    Pointer to the destination buffer of the copy operation (dest->maxsize >= src->length)
 *  \param src     Pointer to the source buffer of the copy operation
 *
 *  \return        SOPC_STATUS_OK if succeeded, an error code otherwise (NULL pointer, non allocated buffer content,
 * incompatible size)
 */
SOPC_ReturnStatus SOPC_Buffer_Copy(SOPC_Buffer* dest, SOPC_Buffer* src);

/**
 *  \brief                  Copy the data bytes and properties for the given length from the source buffer to the
 *                          destination buffer.
 *
 *
 *  \param dest             Pointer to the destination buffer of the copy operation (dest->maxsize >= limitedLength)
 *  \param src              Pointer to the source buffer of the copy operation (src->length >= limitedLength,
 * src->position <= limitedLength) \param limitedLength    The length to use for the copy, number of bytes copied and
 * length set in destination buffer
 *
 *  \return                 SOPC_STATUS_OK if succeeded, an error code otherwise (NULL pointer, non allocated buffer
 * content, incompatible size)
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

/**
 * \brief Reads the contents of a file into a SOPC_Buffer.
 *
 * \param path  The path to the file.
 * \param buf   Out parameter, points to a new buffer holding the contents of
 *              the file. This buffer should be freed by the caller.
 *
 * \return \c SOPC_STATUS_OK on success, an error code in case of failure.
 */
SOPC_ReturnStatus SOPC_Buffer_ReadFile(const char* path, SOPC_Buffer** buf);

#endif /* SOPC_BUFFER_H_ */
