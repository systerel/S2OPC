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

#include "sopc_doublebuffer.h"

struct SOPC_DoubleBuffer
{
    uint32_t eltSizeInBytes; // Elt size in bytes. Header of 4 bytes (size of record), atomic elt size. Always aligned
                             // on 4 bytes.
    uint32_t bufferSizeInBytes; // Equal to eltsize * number of concurrent readers
    uint32_t nbBuffers;         // Number of readers
    uint32_t lastRecord;        // Last row writed, between 0 and nbBuffers - 1. pBufferStatus indicate if row 0 or
                                // row 1 has been writed.
    uint32_t* pReadersCounter;  // Indicate for each row how readers are reading it
    uint32_t* pBufferStatus;    // Indicate for each row if row 0 or row 1 is the most up to date
    uint8_t* doubleBuffer;      // Double buffer
    /* Memory organization
    DBO             [NB_READERS] [NB_STATUS] (2 status)
    READERS_COUNTER [NB_READERS]
    BUFFER_STATUS   [NB_READERS]
    */
};

// Creation of double buffer
// Returns: DBO object
SOPC_DoubleBuffer* SOPC_DoubleBuffer_Create(uint32_t nbReaders,       // Nb readers
                                            uint32_t atomic_elt_size) // Size of atomic element
{
    int result = 0;
    SOPC_DoubleBuffer* pBuffer = (SOPC_DoubleBuffer*) SOPC_Malloc(sizeof(SOPC_DoubleBuffer));
    memset((void*) pBuffer, 0, sizeof(SOPC_DoubleBuffer));
    if (NULL == pBuffer)
    {
        return NULL;
    }

    uint64_t nbBuffers = nbReaders + 1;

    // Calculates real elt size (aligned on 4 bytes)
    uint64_t eltSizeInBytes = (                                   //
        (atomic_elt_size + sizeof(uint32_t)) +                    // Elt size + field size
        sizeof(uint32_t) -                                        // Increment of nb bytes to align on size of uint32_t
        (atomic_elt_size + sizeof(uint32_t)) % (sizeof(uint32_t)) //
    );                                                            //

    // Calculates for one status buffer size
    uint64_t bufferSizeInBytes = nbBuffers * eltSizeInBytes;

    // For 2 status, double the size of the buffer and add CURRENT STATUS and NB READERS READING for each row
    uint64_t totalSize = (bufferSizeInBytes * 2 + 2 * nbBuffers * sizeof(uint32_t));

    // Check for overflow
    if (totalSize > (uint64_t) UINT32_MAX)
    {
        result = -1;
    }

    if (0 == result)
    {
        // Allocation of 1 contiguous memory zone and initialize pointers pBufferStatus and pReadersCounter
        pBuffer->doubleBuffer = (uint8_t*) SOPC_Malloc(totalSize);
    }

    if (NULL == pBuffer->doubleBuffer)
    {
        result = -1;
    }
    if (0 == result)
    {
        memset((void*) pBuffer->doubleBuffer, 0, totalSize);

        pBuffer->pReadersCounter = (uint32_t*) (pBuffer->doubleBuffer + bufferSizeInBytes * 2);
        pBuffer->pBufferStatus = pBuffer->pReadersCounter + nbBuffers;
        pBuffer->eltSizeInBytes = (uint32_t) eltSizeInBytes;
        pBuffer->bufferSizeInBytes = (uint32_t) bufferSizeInBytes;
        pBuffer->nbBuffers = (uint32_t) nbBuffers;
    }
    else
    {
        SOPC_Free(pBuffer);
        pBuffer = NULL;
    }

    return pBuffer;
}

// Destroy double buffer
void SOPC_DoubleBuffer_Destroy(SOPC_DoubleBuffer** p)
{
    if ((NULL != p) && (NULL != (*p)))
    {
        if ((*p)->doubleBuffer != NULL)
        {
            (*p)->pBufferStatus = NULL;
            (*p)->pReadersCounter = NULL;
            SOPC_Free((void*) ((*p)->doubleBuffer));
            (*p)->doubleBuffer = NULL;
        }
        SOPC_Free(*p);
        *p = NULL;
    }
}

// Reserve a buffer for write operation
// Returns: UINT32_MAX in case of failure, else idBuffer to pass to other functions
SOPC_ReturnStatus SOPC_DoubleBuffer_GetWriteBuffer(SOPC_DoubleBuffer* p, // DBO object
                                                   uint32_t* pIdBuffer,
                                                   uint32_t* pMaxSize) // Address to write idBuffer
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == p || pIdBuffer == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t idBufferToWrite = UINT32_MAX;
    uint32_t readCounter = 0;
    for (uint32_t i = 0; i < p->nbBuffers && UINT32_MAX == idBufferToWrite; i++)
    {
        __atomic_load(&p->pReadersCounter[(i + p->lastRecord) % p->nbBuffers], &readCounter, __ATOMIC_SEQ_CST);
        if (0 == readCounter)
        {
            idBufferToWrite = (i + p->lastRecord) % p->nbBuffers;
        }
    }

    if (idBufferToWrite == UINT32_MAX)
    {
        result = SOPC_STATUS_NOK;
    }

    if (pMaxSize != NULL)
    {
        *pMaxSize = (uint32_t)(p->eltSizeInBytes - sizeof(uint32_t));
    }
    *pIdBuffer = idBufferToWrite;
    return result;
}

// Release a reserved buffer after a write operation. Modification are taken into account.
// If you want no push modif, don't call this function.
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseWriteBuffer(SOPC_DoubleBuffer* p, // DBO buffer object reference
                                                       uint32_t* pIdBuffer)  // Id of buffer return by GetWriteBuffer
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == p || NULL == pIdBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (*pIdBuffer >= p->nbBuffers)
    {
        result = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == result)
    {
        uint32_t idBuffer = *pIdBuffer;
        // Update status W1 to W2 or W2 to W1
        uint32_t newValue = (p->pBufferStatus[idBuffer] + 1) % 2;
        __atomic_store(&p->pBufferStatus[idBuffer], &newValue, __ATOMIC_SEQ_CST);
        // Id of last record updated
        __atomic_store(&p->lastRecord, &idBuffer, __ATOMIC_SEQ_CST);
    }
    *pIdBuffer = UINT32_MAX;
    return result;
}

// Set size to 0 of a buffer
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferErase(SOPC_DoubleBuffer* p, // DBO object
                                                     uint32_t idBuffer)    // Id of buffer returned by GetWriteBuffer
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == p || idBuffer >= p->nbBuffers)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint8_t* pStartElt = p->doubleBuffer +                                               //
                         ((p->pBufferStatus[idBuffer] + 1) % 2) * p->bufferSizeInBytes + //
                         idBuffer * p->eltSizeInBytes;                                   //

    memset(pStartElt, 0, p->eltSizeInBytes);
    return result;
}

// Expose directly double buffer data field and size field.
// No check of size and data written.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferGetPtr(
    SOPC_DoubleBuffer* p,   // DBO object
    uint32_t idBuffer,      // Buffer identifier
    uint8_t** ppDataField,  // Pointer on data field
    uint32_t** ppSizeField, // Pointer on size field
    bool ignorePrevious)    // Copy last updated value of DBO on output ptr
{
    if (NULL == p || idBuffer >= p->nbBuffers || NULL == ppDataField || NULL == ppSizeField)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppDataField = NULL;
    *ppSizeField = NULL;

    uint8_t* pData = NULL;
    uint32_t* pSizeField = NULL;
    uint8_t* pPreviousData = NULL;
    uint32_t previousDataSize = 0;

    pData =                                                             // Data pointer to update externally
        p->doubleBuffer +                                               // Double buffer data zone
        ((p->pBufferStatus[idBuffer] + 1) % 2) * p->bufferSizeInBytes + // Status
        idBuffer * p->eltSizeInBytes +                                  // Buffer (size + data)
        sizeof(uint32_t);                                               // Offset added to point on data

    // Get pointer on data size
    pSizeField = (uint32_t*) (pData - sizeof(uint32_t));

    // Duplicate previous data or not
    if (!ignorePrevious)
    {
        pPreviousData = p->doubleBuffer +                                        //
                        p->pBufferStatus[p->lastRecord] * p->bufferSizeInBytes + //
                        p->lastRecord * p->eltSizeInBytes +                      //
                        sizeof(uint32_t);                                        //

        previousDataSize = *((uint32_t*) ((pPreviousData - sizeof(uint32_t))));

        for (uint32_t i = 0; i < previousDataSize && i < p->eltSizeInBytes; i++)
        {
            pData[i] = pPreviousData[i];
        }
    }

    // Update field size with previous if necessary, else 0
    *pSizeField = previousDataSize;

    // Out pData and pSize fields pointers
    *ppDataField = pData;
    *ppSizeField = pSizeField;

    return SOPC_STATUS_OK;
}

// Write data to a buffer.
// Return the current total size of significant bytes from 0 in the buffer.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBuffer(
    SOPC_DoubleBuffer* p,   // DBO object
    uint32_t idBuffer,      // Id of buffer
    uint32_t offset,        // Offset of writing start. Total size = current size + offset + size of data to write
    uint8_t* data,          // Data to write
    uint32_t size,          // Size of data to write
    uint32_t* pWrittenSize, // Size written
    bool ignorePrevious,    // Ignore previous data : don't copy previous record information before offset
    bool ignoreAfter)       // Ignore previous data after offset + size
{
    if (NULL == p || NULL == pWrittenSize || idBuffer >= p->nbBuffers || ((NULL == data) && (size > 0)))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint32_t newSize = size + offset;
    if (newSize > p->eltSizeInBytes - sizeof(uint32_t))
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    uint8_t* pPreviousData = NULL;
    uint8_t* pStartPreviousElt = NULL;
    uint32_t previousSize = 0;

    uint32_t bufferStatus = (p->pBufferStatus[idBuffer] + 1) % 2;
    uint8_t* pStartElt = p->doubleBuffer + bufferStatus * p->bufferSizeInBytes + idBuffer * p->eltSizeInBytes;
    uint8_t* pData = pStartElt + sizeof(uint32_t);

    // If previous record not ignored, data of previous record between 0 and offset are copied into new record
    if (!ignorePrevious)
    {
        pStartPreviousElt = p->doubleBuffer + p->pBufferStatus[p->lastRecord] * p->bufferSizeInBytes +
                            p->lastRecord * p->eltSizeInBytes;
        previousSize = *((uint32_t*) pStartPreviousElt);
        pPreviousData = pStartPreviousElt + sizeof(uint32_t);
    }
    else
    {
        pStartPreviousElt = pStartElt;
        pPreviousData = pData;
        previousSize = *((uint32_t*) pStartElt);
    }

    // From 0 to offset, copy old to new
    for (uint32_t i = 0; i < offset; i++)
    {
        *pData = *pPreviousData;
        pData++;
        pPreviousData++;
    }

    // From offset to offset + size, copy new data
    for (uint32_t i = offset; i < newSize; i++)
    {
        *pData = *data;
        pData++;
        pPreviousData++;
        data++;
    }

    // If ignore after is not set, data after offset + size are copied from previous record if exist.
    if (ignoreAfter == false)
    {
        // From newSize to previousSize
        for (uint32_t i = newSize; i < previousSize; i++)
        {
            *pData = *pPreviousData;
            pData++;
            pPreviousData++;
        }

        // If not enough data, new size is used.
        *((uint32_t*) pStartElt) = previousSize > newSize ? previousSize : newSize;
    }
    else
    {
        *((uint32_t*) pStartElt) = newSize;
    }

    // Return total size of record

    *pWrittenSize = *((uint32_t*) pStartElt);

    return SOPC_STATUS_OK;
}

// Get most update buffer to read. This buffer is marked as reading status.
// Returns buffer to read.
SOPC_ReturnStatus SOPC_DoubleBuffer_GetReadBuffer(SOPC_DoubleBuffer* p, // DBO Object
                                                  uint32_t* pIdBuffer)  // Address point to id buffer
{
    if (NULL == p || NULL == pIdBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Get last record
    uint32_t idBuffer = 0;
    bool result = false;
    uint32_t currentValue = 0;
    uint32_t newValue = 0;

    __atomic_load(&p->lastRecord, &idBuffer, __ATOMIC_SEQ_CST);

    do
    {
        // Load current counter and atomic increment it if possible
        __atomic_load(&p->pReadersCounter[idBuffer], &currentValue, __ATOMIC_SEQ_CST);
        newValue = currentValue;
        if (currentValue < p->nbBuffers)
        {
            newValue = currentValue + 1;
        }
        else
        {
            newValue = currentValue;
        }

        result = __atomic_compare_exchange(&p->pReadersCounter[idBuffer], //
                                           &currentValue,                 //
                                           &newValue,                     //
                                           false, __ATOMIC_SEQ_CST,
                                           __ATOMIC_SEQ_CST); //
    } while (!result);

    // Return buffer
    *pIdBuffer = idBuffer;
    return SOPC_STATUS_OK;
}

// Get data pointer
// Returns: size of data
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBufferPtr(SOPC_DoubleBuffer* p,  // DBO object
                                                  uint32_t idBuffer,     // Buffer identifier
                                                  uint8_t** pData,       // Pointer on data buffer
                                                  uint32_t* pDataToRead) // Returns data size can be read from offset 0
{
    if (NULL == p || NULL == pDataToRead || idBuffer >= p->nbBuffers || NULL == pData)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Get buffer status R1 or R2
    uint32_t bufferStatus = 0;

    __atomic_load(&p->pBufferStatus[idBuffer], //
                  &bufferStatus,               //
                  __ATOMIC_SEQ_CST);           //

    bufferStatus = bufferStatus % 2;

    uint8_t* pStartElt = p->doubleBuffer +                                //
                         bufferStatus * p->bufferSizeInBytes +            //
                         idBuffer * p->eltSizeInBytes + sizeof(uint32_t); //

    uint32_t sizeOfRecord = *((uint32_t*) (pStartElt - sizeof(uint32_t)));

    *pData = pStartElt;
    *pDataToRead = sizeOfRecord;

    return SOPC_STATUS_OK;
}

// Read buffer from offset on sizeRequest
// Returns: size of data read
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBuffer(SOPC_DoubleBuffer* p, // DBO object
                                               uint32_t idBuffer,    // Buffer identifier
                                               uint32_t offset,      // Offset from start read
                                               uint8_t* data,        // Data output
                                               uint32_t sizeRequest, // Request of read
                                               uint32_t* pReadData)  // Returns size of data read
{
    if (NULL == p || NULL == pReadData || idBuffer >= p->nbBuffers || NULL == data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint32_t bufferStatus = 0;

    __atomic_load(&p->pBufferStatus[idBuffer], &bufferStatus, __ATOMIC_SEQ_CST);

    bufferStatus = bufferStatus % 2;

    uint8_t* pStartElt = p->doubleBuffer + bufferStatus * p->bufferSizeInBytes + idBuffer * p->eltSizeInBytes;
    uint32_t sizeOfRecord = *((uint32_t*) (pStartElt));

    if (offset >= sizeOfRecord)
    {
        *pReadData = 0;
        return SOPC_STATUS_OK;
    }

    // Size to return
    uint32_t sizeToReturn = sizeOfRecord - offset;
    sizeToReturn = sizeToReturn < sizeRequest ? sizeToReturn : sizeRequest;

    // Data to read
    uint8_t* pData = pStartElt + sizeof(uint32_t) + offset;

    for (uint32_t i = 0; i < sizeToReturn && i < sizeRequest; i++)
    {
        *data = *pData;
        pData++;
        data++;
    }

    // Return nb bytes read
    *pReadData = sizeToReturn;

    return SOPC_STATUS_OK;
}

// Release read buffer to allow writer to write.
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseReadBuffer(SOPC_DoubleBuffer* p, // DBO object
                                                      uint32_t* pIdBuffer)  // Buffer identifier
{
    if (NULL == p || NULL == pIdBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint32_t idBuffer = *pIdBuffer;
    if (idBuffer >= p->nbBuffers)
    {
        return SOPC_STATUS_NOK;
    }

    // Atomically decrement reader counter for current row
    bool result = false;
    uint32_t currentValue = 0;
    uint32_t newValue = 0;
    do
    {
        __atomic_load(&p->pReadersCounter[idBuffer], //
                      &currentValue,                 //
                      __ATOMIC_SEQ_CST);             //

        newValue = currentValue;
        if (currentValue > 0)
        {
            newValue = currentValue - 1;
        }
        else
        {
            newValue = currentValue;
        }
        result = __atomic_compare_exchange(&p->pReadersCounter[idBuffer], //
                                           &currentValue,                 //
                                           &newValue,                     //
                                           false,                         //
                                           __ATOMIC_SEQ_CST,              //
                                           __ATOMIC_SEQ_CST);             //

    } while (!result);

    *pIdBuffer = UINT32_MAX;
    return SOPC_STATUS_OK;
}
