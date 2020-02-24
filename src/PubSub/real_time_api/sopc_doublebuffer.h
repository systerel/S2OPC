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

#ifndef SOPC_DBO_H_
#define SOPC_DBO_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

// Double buffer object
typedef struct SOPC_DoubleBuffer SOPC_DoubleBuffer;

// Creation of double buffer
// Returns: DBO object
SOPC_DoubleBuffer* SOPC_DoubleBuffer_Create(uint32_t nbReaders,        // Nb readers
                                            uint32_t atomic_elt_size); // Size of atomic element

// Destroy double buffer
void SOPC_DoubleBuffer_Destroy(SOPC_DoubleBuffer** pp);

// Following functions (SOPC_DoubleBuffer_XxWriteBufferXx) shall be used from one same thread execution context.
// because not thread safe. SOPC_DoubleBuffer is designed for 1 writer / Multiple readers

// Reserve a buffer for write operation. Not thread safe.
// Returns: UINT32_MAX in case of failure, else idBuffer to pass to other functions
SOPC_ReturnStatus SOPC_DoubleBuffer_GetWriteBuffer(SOPC_DoubleBuffer* p, // DBO object
                                                   uint32_t* pIdBuffer,  // Address to write idBuffer
                                                   uint32_t* pMaxSize);  // Max allowed size

// Write data to a buffer. Not thread safe.
// Return the current total size of significant bytes from 0 in the buffer.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBuffer(
    SOPC_DoubleBuffer* p,   // DBO object
    uint32_t idBuffer,      // Id of buffer
    uint32_t offset,        // Offset of writing start. Total size = current size + offset + size of data to write
    uint8_t* data,          // Data to write
    uint32_t size,          // Size of data to write
    uint32_t* pWrittenSize, // Size written
    bool ignorePrevious,    // Ignore previous data : don't copy previous record information before offset
    bool ignoreAfter);      // Ignore previous data after offset + size

// Expose directly double buffer data field and size field. Not thread safe.
// !!!!! WARNING !!! Use it carefully, no check of size and data written.
// Expose directly double buffer data field and size field.
// No check of size and data written.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferGetPtr(
    SOPC_DoubleBuffer* p,   // DBO object
    uint32_t idBuffer,      // Buffer identifier
    uint8_t** ppDataField,  // Pointer on data field
    uint32_t** ppSizeField, // Pointer on size field
    bool ignorePrevious);   // Copy last updated value of DBO on output ptr

// Set size to 0 of a buffer. Not thread safe.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferErase(SOPC_DoubleBuffer* p, // DBO object
                                                     uint32_t idBuffer);   // Id of buffer returned by GetWriteBuffer

// Release a reserved buffer after a write operation. Modification are taken into account. Not thread safe.
// !!!!! WARNING If you don't want push modifications, don't call this function. !!!
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseWriteBuffer(SOPC_DoubleBuffer* p, // DBO buffer object reference
                                                       uint32_t* pIdBuffer); // Id of buffer return by GetWriteBuffer

// Get most update buffer to read. This buffer is marked as reading status.
// Returns buffer to read.
// !!!!! WARNING !!! SHALL ALWAYS BE CALLED BEFORE READ BUFFER
SOPC_ReturnStatus SOPC_DoubleBuffer_GetReadBuffer(SOPC_DoubleBuffer* p, // DBO Object
                                                  uint32_t* pIdBuffer); // Address point to id buffer

// Get data pointer
// Returns: size of data
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBufferPtr(SOPC_DoubleBuffer* p,   // DBO object
                                                  uint32_t idBuffer,      // Buffer identifier
                                                  uint8_t** pData,        // Pointer on data buffer
                                                  uint32_t* pDataToRead); // Returns data size can be read from offset 0

// Read buffer from offset on sizeRequest
// Returns: size of data read
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBuffer(SOPC_DoubleBuffer* p, // DBO object
                                               uint32_t idBuffer,    // Buffer identifier
                                               uint32_t offset,      // Offset from start read
                                               uint8_t* data,        // Data output
                                               uint32_t sizeRequest, // Request of read
                                               uint32_t* pReadData); // Returns size of data read

// Release read buffer to allow writer to write.
// !!!!! WARNING !!! SHALL ALWAYS BE CALLED AFTER READ BUFFER
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseReadBuffer(SOPC_DoubleBuffer* p, // DBO object
                                                      uint32_t* pIdBuffer); // Buffer identifier

#endif /* SOPC_DBO_H_ */
