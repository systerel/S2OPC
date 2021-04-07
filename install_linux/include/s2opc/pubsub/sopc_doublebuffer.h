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

/** \file sopc_doublebuffer.h
 *
 * \brief Double buffer array: allow multiple read concurrently to one writing operation.
 */

#ifndef SOPC_DOUBLEBUFFER_H_
#define SOPC_DOUBLEBUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

/** \brief An array of double buffers of configurable size with a single writer but multiple readers
 *
 * Elements are each a double buffer with two banks of the given size.
 * At any given time, there should be only one writer to the structure.
 * The array works as follows.
 *
 * On the writer side, the SOPC_DoubleBuffer_GetWriteBuffer() is called to find in which cell it is possible to write.
 * With the returned index,
 * it is possible to write with SOPC_DoubleBuffer_WriteBuffer() or SOPC_DoubleBuffer_WriteBufferGetPtr().
 * The index must be released with SOPC_DoubleBuffer_ReleaseWriteBuffer() (which commits the write).
 * SOPC_DoubleBuffer_GetWriteBuffer() should not be called a second time before the first index has been released.
 *
 * On the reader side, any thread can call SOPC_DoubleBuffer_GetReadBuffer()
 * to obtain an index towards the most recently written element of the array.
 * SOPC_DoubleBuffer_ReadBuffer() and SOPC_DoubleBuffer_ReadBufferPtr() are used obtain data stored in the element.
 * The index must be released with SOPC_DoubleBuffer_ReleaseReadBuffer() as soon as possible,
 * as writes cannot be made in this element while it is read.
 * Hence, keeping the index may lead to a deprivation of writable elements.
 *
 * \note Upon Write, the functions are capable of retrieving the current "last value",
 *       meaning that it is possible to update the lastly written value
 *       (this is possible because the writers are not concurrent)
 *
 * \warning Writer functions are not thread safe and should not be called concurrently
 * \warning The structure should not be destroyed while in use
 */
typedef struct SOPC_DoubleBuffer SOPC_DoubleBuffer;

/** \brief  Creation of double buffer array
 *
 * \param   nbElements      Number of elements in the array.
 * \param   elementSize     Size of an atomic element
 *
 * \note    \p nbElements shall be chosen such that
 *          there are at most `nbElements-1` concurrent read access to the array
 *
 * \see     SOPC_DoubleBuffer
 * \warning Not thread safe!
 * \return  Allocated structure or NULL. Shall be destroyed via SOPC_DoubleBuffer_Destroy().
 */
SOPC_DoubleBuffer* SOPC_DoubleBuffer_Create(size_t nbElements, size_t elementSize);

/** \brief   Destroy a double buffer array
 * \param [inout]  pp  Object to destroy. Pointer is set to NULL.
 * \warning Not thread safe!
 */
void SOPC_DoubleBuffer_Destroy(SOPC_DoubleBuffer** pp);

// Following functions (SOPC_DoubleBuffer_XxWriteBufferXx) shall be used from one same thread execution context.
// because not thread safe. SOPC_DoubleBuffer is designed for 1 writer / Multiple readers

/** \brief Reserve a buffer for write operation.
 *
 * \param [in]  p           The double buffer array
 * \param [out] pIdBuffer   Pointer to the allocated buffer index for write operations
 * \param [out] pMaxSize    Pointer to write the element size
 *
 * \see SOPC_DoubleBuffer_ReleaseWriteBuffer()
 * \warning Not thread safe for concurrent writers.
 * \warning Returned buffer index must not be modified and used to access unreserved memory.
 * \return SOPC_STATUS_NOK in case of failure (unmodified outputs), else SOPC_STATUS_OK
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_GetWriteBuffer(SOPC_DoubleBuffer* p, size_t* pIdBuffer, size_t* pMaxSize);

/** \brief Write data to a buffer.
 *
 * This function is also able to copy the rest of the data (before \p offset and after \p offset + \p size)
 * from the last committed buffer in the array
 * (i.e. the buffer that was written by the previous call to SOPC_DoubleBuffer_ReleaseWriteBuffer())
 * with the parameters \p ignorePrevious and \p ignoreAfter.
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetWriteBuffer()
 * \param [in] offset   Offset of writing start. \p offset + \p size must not exceed element size
 * \param [in] data     Pointer to the data to write, of length at least \p size
 * \param [in] size     Number of bytes to write
 * \param [out] pWrittenSize    Pointer to the number of written bytes (including copies from previous and/or after)
 * \param [in] ignorePrevious   Ignore previous data: don't copy data from last committed buffer before offset
 * \param [in] ignoreAfter      Ignore previous data: don't copy data from last committed buffer after offset + size
 *
 * \warning Not thread safe for concurrent writers.
 * \warning Shall be called between SOPC_DoubleBuffer_GetWriteBuffer() and SOPC_DoubleBuffer_ReleaseWriteBuffer()
 * \return Returns SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS, or SOPC_STATUS_OUT_OF_MEMORY
 *         if given data does not fit in an element.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBuffer(SOPC_DoubleBuffer* p,
                                                size_t idBuffer,
                                                size_t offset,
                                                uint8_t* data,
                                                size_t size,
                                                size_t* pWrittenSize,
                                                bool ignorePrevious,
                                                bool ignoreAfter);

/** \brief Expose directly double buffer data field and size field.
 *
 * This function is also able to copy the data from the last committed buffer
 * (i.e. the buffer that was written by the previous call to SOPC_DoubleBuffer_ReleaseWriteBuffer())
 * in the returned buffer before returning with the parameter \p ignorePrevious.
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetWriteBuffer()
 * \param [out] ppDataField Returned pointer to the internal buffer to write
 * \param [in] ignorePrevious If false, the element that was committed in the previous write is copied to the buffer.
 *
 * \warning Not thread safe for concurrent writers.
 * \warning Shall be called between SOPC_DoubleBuffer_GetWriteBuffer() and SOPC_DoubleBuffer_ReleaseWriteBuffer()
 * \warning Never write before `*ppDataField` nor beyond `*ppDataField+p->elemSize`
 *          as it would write randomly to other buffers of the array.
 * \return SOPC_STATUS_OK in case of success, or SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferGetPtr(SOPC_DoubleBuffer* p,
                                                      size_t idBuffer,
                                                      uint8_t** ppDataField,
                                                      bool ignorePrevious);

/** \brief Reset the current element.
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetWriteBuffer()
 *
 * \warning Not thread safe for concurrent writers.
 * \warning Shall be called between SOPC_DoubleBuffer_GetWriteBuffer() and SOPC_DoubleBuffer_ReleaseWriteBuffer()
 * \return SOPC_STATUS_OK in case of success, or SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferErase(SOPC_DoubleBuffer* p, size_t idBuffer);

/** \brief Release a reserved buffer after a write operation and commit the result.
 *
 * Modifications to the buffer are not visible by the readers before calling this function.
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetWriteBuffer()
 *
 * \return SOPC_STATUS_OK in case of success, else SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseWriteBuffer(SOPC_DoubleBuffer* p, size_t idBuffer);

/** \brief Get most up to date buffer to read.
 *
 * This buffer is reserved for read operations
 * and cannot be written before it is released by calling SOPC_DoubleBuffer_ReleaseReadBuffer()
 *
 * \param [in] p            The double buffer array
 * \param [out] pIdBuffer   Pointer to the buffer reserved for read operations
 *
 * \warning Shall always be called before SOPC_DoubleBuffer_ReadBuffer() and SOPC_DoubleBuffer_ReadBufferPtr()
 * \warning Returned buffer index must not be modified and used to access unreserved memory.
 * \return Returns SOPC_STATUS_OK in case of success, or SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_GetReadBuffer(SOPC_DoubleBuffer* p, size_t* pIdBuffer);

/** \brief Expose directly pointer on data buffer and return size of significant bytes
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetReadBuffer()
 * \param [out] ppData  Pointer to the internal buffer storing the data reserved for read
 *
 * \warning Shall be called between SOPC_DoubleBuffer_GetReadBuffer() and SOPC_DoubleBuffer_ReleaseBuffer()
 * \warning Never read before `*ppData` or beyond `*ppData+p->elemSize` as it would read from unreserved buffers.
 * \return Returns SOPC_STATUS_OK in case of success, or SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBufferPtr(SOPC_DoubleBuffer* p, size_t idBuffer, uint8_t** ppData);

/** \brief Read buffer from offset, on sizeRequest
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetReadBuffer()
 * \param [in] offset   Offset in the internal buffer where read starts
 * \param [out] data    Output buffer of at least \p size bytes
 * \param [in] size     Number of bytes to copy from internal buffer from its offset to output buffer
 * \param [out] pReadSize Pointer to the number of bytes copied, set to `p->elemSize-offset`. May be NULL.
 *
 * \warning Shall be called between SOPC_DoubleBuffer_GetReadBuffer() and SOPC_DoubleBuffer_ReleaseBuffer()
 * \return  Returns SOPC_STATUS_OK in case of success, SOPC_INVALID_PARAMETERS,
 *          or SOPC_STATUS_NOK if cannot read \p size bytes.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBuffer(SOPC_DoubleBuffer* p,
                                               size_t idBuffer,
                                               size_t offset,
                                               uint8_t* data,
                                               size_t size,
                                               size_t* pReadSize);

/** \brief Release read buffer to allow writer to write.
 *
 * \param [in] p        The double buffer array
 * \param [in] idBuffer Buffer index returned by SOPC_DoubleBuffer_GetReadBuffer()
 *
 * \return Returns SOPC_STATUS_OK in case of success, or SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseReadBuffer(SOPC_DoubleBuffer* p, size_t idBuffer);

#endif /* SOPC_DOUBLEBUFFER_H_ */
