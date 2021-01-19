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

/** \file sopc_doublebuffer.c */

#include <assert.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_doublebuffer.h"
#include "sopc_logger.h"

typedef struct SOPC_DbleBufElem
{
    /* TODO: these values are int32_t because of our current Atomic interface */
    /** Number of concurrent readers reading this element (more than 0 prevents writes) */
    int32_t readersCount;
    /** Whether the first memory bank (\verbatim bankB = false\endverbatim) currently stores data or the other */
    int32_t bankB;
    /* To reduce the number of calls to malloc, banks are stored in an array inside the double buffer array */
} SOPC_DbleBufElem;

/** \brief An array of double buffers of configurable size with a single writer but multiple readers
 *
 * Elements are each a double buffer with two banks of the given size.
 * The array works as follows.
 *
 * On the writer side, the `SOPC_DoubleBuffer_GetWriteBuffer` is called to find in which cell it is possible to write.
 * With the return index,
 * it is possible to write with `SOPC_DoubleBuffer_WriteBuffer` or `SOPC_DoubleBuffer_WriteBufferGetPtr`.
 * The index must be released with `SOPC_DoubleBuffer_ReleaseWriteBuffer` (which commits the write).
 * `SOPC_DoubleBuffer_GetWriteBuffer` should not be called a second time before the first index has been released.
 *
 * On the reader side, any thread can call `SOPC_DoubleBuffer_GetReadBuffer`
 * to obtain an index towards the most recently written element of the array.
 * `SOPC_DoubleBuffer_ReadBuffer` and `SOPC_DoubleBuffer_ReadBufferPtr` are used obtain data stored in the element.
 * The index must be released with `SOPC_DoubleBuffer_ReleaseReadBuffer` as soon as possible,
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
struct SOPC_DoubleBuffer
{
    size_t nbElems;
    /** Index of the element that was written lastly */
    size_t iLastWritten;
    /** Allocated memory in bytes for each element for each bank */
    size_t elemSize;
    /** Array of \p nbElems elements */
    SOPC_DbleBufElem* arrayElems;
    /** Array of all the buffers (two buffers per element) */
    uint8_t* arrayBuffers;
};

/** Buffer elements are arranged in two banks in the array buffer */
static inline uint8_t* get_buffer_ptr(SOPC_DoubleBuffer* p, size_t idBuffer, bool write);
static inline uint8_t* get_buffer_ptr(SOPC_DoubleBuffer* p, size_t idBuffer, bool write)
{
    size_t index = 2 * idBuffer;
    /* Write to the other bank (bankB indicates in which bank are currently stored data) */
    if (p->arrayElems[idBuffer].bankB ^ write)
    {
        index += 1;
    }
    return &p->arrayBuffers[index * p->elemSize];
}

SOPC_DoubleBuffer* SOPC_DoubleBuffer_Create(size_t nbElements, size_t elementSize)
{
    if (0 == nbElements || 0 == elementSize || nbElements > SIZE_MAX / 2 / elementSize)
    {
        return NULL;
    }

    /* Allocate all buffers, then allocate and fill the superstructure */
    SOPC_DbleBufElem* arrElems = (SOPC_DbleBufElem*) SOPC_Calloc(nbElements, sizeof(SOPC_DbleBufElem));
    uint8_t* arrBuffs = (uint8_t*) SOPC_Calloc(1, 2 * nbElements * elementSize);
    SOPC_DoubleBuffer* pBuffer = (SOPC_DoubleBuffer*) SOPC_Calloc(1, sizeof(SOPC_DoubleBuffer));

    if (NULL != pBuffer && NULL != arrElems && NULL != pBuffer)
    {
        pBuffer->nbElems = nbElements;
        pBuffer->elemSize = elementSize;
        pBuffer->arrayElems = arrElems;
        pBuffer->arrayBuffers = arrBuffs;
    }
    else
    {
        SOPC_Free(arrElems);
        arrElems = NULL;
        SOPC_Free(arrBuffs);
        arrBuffs = NULL;
        SOPC_Free(pBuffer);
        pBuffer = NULL;
    }

    return pBuffer;
}

void SOPC_DoubleBuffer_Destroy(SOPC_DoubleBuffer** pp)
{
    if (NULL == pp || NULL == *pp)
    {
        return;
    }
    SOPC_DoubleBuffer* p = *pp;
    SOPC_Free(p->arrayElems);
    p->arrayElems = NULL;
    SOPC_Free(p->arrayBuffers);
    p->arrayBuffers = NULL;
    SOPC_Free(p);
    p = NULL;
    *pp = NULL;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_GetWriteBuffer(SOPC_DoubleBuffer* p, size_t* pIdBuffer, size_t* pMaxSize)
{
    if (NULL == p || NULL == pIdBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    int32_t readCounter = 1;
    /* Find a buffer that is not being read.
     * Starts at the index after lastWritten to avoid returning lastWritten, which would be the next one to be read. */
    for (size_t i = 1; i < p->nbElems && readCounter > 0; ++i)
    {
        size_t index = (p->iLastWritten + i) % p->nbElems;
        readCounter = SOPC_Atomic_Int_Get(&p->arrayElems[index].readersCount);
        if (0 == readCounter)
        {
            *pIdBuffer = index;
        }
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (readCounter > 0)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                               "All elements of the double buffer array are being read, cannot write");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status && NULL != pMaxSize)
    {
        *pMaxSize = p->elemSize;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseWriteBuffer(SOPC_DoubleBuffer* p, size_t idBuffer)
{
    if (NULL == p)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    /* Commit the changes: switch the currently readable bank */
    /* TODO: Lock before all the following reads and updates */
    if (0 < p->arrayElems[idBuffer].readersCount)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                               "Double buffer element became read while being written, cannot commit");
        return SOPC_STATUS_INVALID_STATE;
    }

    p->arrayElems[idBuffer].bankB = !p->arrayElems[idBuffer].bankB;
    p->iLastWritten = idBuffer;

    /* TODO: count Get-Release */
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferErase(SOPC_DoubleBuffer* p, size_t idBuffer)
{
    if (NULL == p)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    memset(get_buffer_ptr(p, idBuffer, true), 0, p->elemSize);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferGetPtr(SOPC_DoubleBuffer* p,
                                                      size_t idBuffer,
                                                      uint8_t** ppDataField,
                                                      bool ignorePrevious)
{
    if (NULL == p || NULL == ppDataField)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    *ppDataField = get_buffer_ptr(p, idBuffer, true);

    /* Copy the last committed buffer, for update */
    if (!ignorePrevious)
    {
        memcpy(*ppDataField, get_buffer_ptr(p, p->iLastWritten, false), p->elemSize);
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBuffer(SOPC_DoubleBuffer* p,
                                                size_t idBuffer,
                                                size_t offset,
                                                uint8_t* data,
                                                size_t size,
                                                size_t* pWrittenSize,
                                                bool ignorePrevious,
                                                bool ignoreAfter)
{
    if (NULL == p || (NULL == data && size > 0))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    if (offset + size > p->elemSize)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    size_t writtenSize = size;

    /* Copy the start of the last committed buffer */
    if (!ignorePrevious && offset > 0)
    {
        memcpy(get_buffer_ptr(p, idBuffer, true), get_buffer_ptr(p, p->iLastWritten, false), offset);
        writtenSize += offset;
    }

    memcpy(get_buffer_ptr(p, idBuffer, true) + offset, data, size);

    /* Also copy the end of the last committed buffer */
    size_t remaining = p->elemSize - size - offset;
    if (!ignoreAfter && remaining > 0)
    {
        memcpy(get_buffer_ptr(p, idBuffer, true) + offset + size,
               get_buffer_ptr(p, p->iLastWritten, false) + offset + size, remaining);
        writtenSize += remaining;
    }

    /* Return total written size */
    if (NULL != pWrittenSize)
    {
        *pWrittenSize = writtenSize;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_GetReadBuffer(SOPC_DoubleBuffer* p, size_t* pIdBuffer)
{
    if (NULL == p || NULL == pIdBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* TODO: the following operations should be finished before the next GetWriter call,
     * as it will modify the readersCount of an element of the double buffer */
    size_t idBuffer = p->iLastWritten;

    int32_t prevCount = SOPC_Atomic_Int_Add(&p->arrayElems[idBuffer].readersCount, 1);
    /* TODO: The limit of p->nbElems seems rather unjustified, but this was the previous behavior */
    /* TODO: Have an atomic Get/Set/Add on size_t to avoid tedious casts */
    assert(prevCount + 1 >= 0 && (size_t)(prevCount + 1) <= p->nbElems - 1 &&
           "Double buffer array in inconsistent state because of too much readers, cannot proceed");

    // Return buffer
    *pIdBuffer = idBuffer;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBufferPtr(SOPC_DoubleBuffer* p, size_t idBuffer, uint8_t** ppData)
{
    if (NULL == p || NULL == ppData)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    /* TODO: Write should not be possible on this element while Ptr is potentially in use */
    *ppData = get_buffer_ptr(p, idBuffer, false);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBuffer(SOPC_DoubleBuffer* p,
                                               size_t idBuffer,
                                               size_t offset,
                                               uint8_t* data,
                                               size_t size,
                                               size_t* pReadSize)
{
    if (NULL == p || NULL == data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    if (offset + size > p->elemSize)
    {
        return SOPC_STATUS_NOK;
    }

    size_t remaining = p->elemSize - offset;
    memcpy(data, get_buffer_ptr(p, idBuffer, false) + offset, remaining);

    // Return nb bytes read
    if (NULL != pReadSize)
    {
        *pReadSize = remaining;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseReadBuffer(SOPC_DoubleBuffer* p, size_t idBuffer)
{
    if (NULL == p)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(idBuffer < p->nbElems);

    int32_t prevCount = SOPC_Atomic_Int_Add(&p->arrayElems[idBuffer].readersCount, -1);
    assert(prevCount - 1 >= 0 &&
           "Double buffer array in inconsistent state because more readers were released than got, cannot proceed");

    return SOPC_STATUS_OK;
}
