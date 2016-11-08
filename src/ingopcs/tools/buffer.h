/*
 *  \file buffer.h
 *
 *  \brief A buffer of bytes with a maximum size, length and position.
 *
 *  Created on: Jul 22, 2016
 *      Author: VMO (Systerel)
 */

#ifndef INGOPCS_BUFFER_H_
#define INGOPCS_BUFFER_H_

#include "../core_types/sopc_base_types.h"

/**
 *  \brief Bytes buffer structure
 */
typedef struct {
    uint32_t max_size; /**< maximum size (allocated bytes) */
    uint32_t position; /**< read/write position */
    uint32_t length;   /**< data length */
    uint8_t* data;     /**< data bytes */
} Buffer;

/**
 *  \brief          Allocate a buffer and its data bytes of the given size and returns it
 *
 *  \param size     The size of the data bytes allocated
 *  \return         Pointer on the allocated buffer or NULL if allocation failed
 */
Buffer* Buffer_Create(uint32_t size);

/**
 *  \brief          Initialize a buffer by allocating data bytes
 *
 *  \param buffer   Pointer to a non allocated buffer (not NULL)
 *  \param size     The size of the buffer
 *  \return         0 if succeeded, non zero otherwise
 */
SOPC_StatusCode Buffer_Init(Buffer* buffer, uint32_t size);

/**
 *  \brief          Deallocate buffer data bytes content
 *
 *  \param buffer   Pointer to the buffer in which data bytes content must be deallocated.
 */
void Buffer_Clear(Buffer* buffer);

/**
 *  \brief          Deallocate buffer and its data bytes content (Clear + deallocate pointer)
 *
 *  \param buffer   Pointer to the buffer to deallocate (pointer must not be used anymore after operation)
 */
void Buffer_Delete(Buffer* buffer);

/**
 *  \brief          Reset length, position and data bytes to zero value of an allocated buffer
 *
 *  \param buffer   Pointer to the buffer to reset
 *
 */
void Buffer_Reset(Buffer* buffer);

/**
 *  \brief          reset data bytes after position (>=) to zero and set buffer position and length to given position
 *
 *  \param buffer   Pointer to the buffer to reset to the given position
 *  \param position New position of the reset buffer (position <= buffer->length)
 *
 *  \return         0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, invalid position)
 */
SOPC_StatusCode Buffer_ResetAfterPosition(Buffer*  buffer,
                                     uint32_t position);

/**
 *  \brief             Set buffer to the given position
 *
 *  \param buffer      Pointer to the buffer to set to the given position
 *  \param position    New position of the buffer (<= buffer->length)
 *
 *  \return            0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, invalid position)
 */
SOPC_StatusCode Buffer_SetPosition(Buffer* buffer, uint32_t position);

/**
 *  \brief           Set buffer to the given length
 *
 *  \param buffer    Pointer to the buffer to set to the given length
 *  \param length    New length of the buffer (<= buffer->maxsize && >= buffer->position)
 *
 *  \return          0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, invalid length)
 */
SOPC_StatusCode Buffer_SetDataLength(Buffer* buffer, uint32_t length);

/**
 *  \brief             Write the given bytes into the buffer data bytes from the buffer position (adapting buffer position and length if necessary)
 *
 *  \param buffer      Pointer to the buffer to write into
 *  \param data_src    Pointer to the bytes to be write in the buffer (HYP: nb bytes >= count bytes)
 *  \param count       Number of bytes to write in the buffer (count + buffer->position <= buffer->maxsize)
 *
 *  \return            0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, full buffer avoiding operation)
 */
SOPC_StatusCode Buffer_Write(Buffer* buffer, const uint8_t* data_src, uint32_t count);

/**
 *  \brief              Read the given bytes of the buffer data bytes from the buffer position (adapting buffer position to next position to read)
 *
 *  \param data_dest    Pointer to the bytes to set to read bytes value of the buffer (HYP: nb bytes >= count bytes)
 *  \param buffer       Pointer to the buffer to read from
 *  \param count        Number of bytes to read from the buffer (count + buffer->position <= buffer->length)
 *
 *  \return             0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, empty buffer avoiding operation)
 */
SOPC_StatusCode Buffer_Read(uint8_t* data_dest, Buffer* buffer, uint32_t count);

/**
 *  \brief         Copy the data bytes and properties from the source buffer to the destination buffer
 *
 *  \param dest    Pointer to the destination buffer of the copy operation (dest->maxsize >= src->length)
 *  \param src     Pointer to the source buffer of the copy operation
 *
 *  \return        0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, incompatible size)
 */
SOPC_StatusCode Buffer_Copy(Buffer* dest, Buffer* src);

/**
 *  \brief                  Copy the data bytes and properties for the given length from the source buffer to the destination buffer
 *
 *  \param dest             Pointer to the destination buffer of the copy operation (dest->maxsize >= limitedLength)
 *  \param src              Pointer to the source buffer of the copy operation (src->length >= limitedLength, src->position <= limitedLength)
 *  \param limitedLength    The length to use for the copy, number of bytes copied and length set in destination buffer
 *
 *  \return                 0 if succeeded, non zero value otherwise (NULL pointer, non allocated buffer content, incompatible size)
 */
SOPC_StatusCode Buffer_CopyWithLength(Buffer* dest, Buffer* src, uint32_t limitedLength);

#endif /* INGOPCS_BUFFER_H_ */
