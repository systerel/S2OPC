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

/// @file sopc_doublebuffer.h

#ifndef SOPC_DBO_H_
#define SOPC_DBO_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

/// @brief Double buffer object
typedef struct SOPC_DoubleBuffer SOPC_DoubleBuffer;

/// @brief    Creation of double buffer
/// @warning  Not thread safe!
/// @param[in]    nbReaders           Nb of readers will access to DBO object via SOPC_DBO_Read API
/// @param[in]    atomic_elt_size     Size of an atomic element
/// @return   DBO object. Shall be destroyed via SOPC_DoubleBuffer_Destroy.
SOPC_DoubleBuffer* SOPC_DoubleBuffer_Create(uint32_t nbReaders,        // Nb readers
                                            uint32_t atomic_elt_size); // Size of atomic element

/// @brief   Destroy a DBO object
/// @warning Not thread safe!
/// @param [inout]  p  DBO object to destroy. Pointer is setted to NULL.
void SOPC_DoubleBuffer_Destroy(SOPC_DoubleBuffer** p); // DBO object to destroy

// Following functions (SOPC_DoubleBuffer_XxWriteBufferXx) shall be used from one same thread execution context.
// because not thread safe. SOPC_DoubleBuffer is designed for 1 writer / Multiple readers

/// @brief Reserve a buffer for write operation.
/// @warning Not thread safe for concurrent WRITERS.
/// @param [in] p DBO object
/// @param [out] pIdBuffer Buffer identifier which will be used with write buffer functions.
/// @param [out] pMaxSize Max allowed size
/// @return SOPC_STATUS_NOK in case of failure (returned idBuffer set to UINT32_MAX), else SOPC_STATUS_OK
SOPC_ReturnStatus SOPC_DoubleBuffer_GetWriteBuffer(SOPC_DoubleBuffer* p, // DBO object
                                                   uint32_t* pIdBuffer,  // Address to write idBuffer
                                                   uint32_t* pMaxSize);  // Max allowed size

/// @brief Write data to a buffer.
/// @warning Not thread safe for concurrent WRITERS!
/// @warning Shall be called between GetWriteBuffer and ReleaseWriteBuffer
/// @param [in] p DBO object
/// @param [in] idBuffer Buffer identifier returned by SOPC_DoubleBuffer_GetWriteBuffer
/// @param [in] offset Offset of writing start. Total size = current size + offset + size of data to write
/// @param [in] data Data to write
/// @param [in] size Size of data to write
/// @param [out] pWrittenSize Address where is returned the current total size of significant bytes from 0 in the
/// buffer.
/// @param [in] ignorePrevious Ignore previous data: don't copy previous last record information before offset
/// @param [in] ignoreAfter Ignore previous data after offset + size
/// @return Returns SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBuffer(
    SOPC_DoubleBuffer* p,   // DBO object
    uint32_t idBuffer,      // Id of buffer
    uint32_t offset,        // Offset of writing start. Total size = current size + offset + size of data to write
    uint8_t* data,          // Data to write
    uint32_t size,          // Size of data to write
    uint32_t* pWrittenSize, // Size written
    bool ignorePrevious,    // Ignore previous data : don't copy previous record information before offset
    bool ignoreAfter);      // Ignore previous data after offset + size

/// @brief Expose directly double buffer data field and size field.
/// @warning Not thread safe for concurrent WRITERS!
/// @warning Use it carefully, no check of size and data written !
/// @param [in] p DBO object
/// @param [in] idBuffer Buffer identifier returned by SOPC_DoubleBuffer_GetWriteBuffer
/// @param [out] ppDataField Address where is returned pointer on buffer data field
/// @param [out] ppSizeField Address where is returned pointer on buffer size field
/// @param [in] ignorePrevious If true, last updated value of DBO are NOT copied to reserved buffer.
/// @return SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferGetPtr(
    SOPC_DoubleBuffer* p,   // DBO object
    uint32_t idBuffer,      // Buffer identifier
    uint8_t** ppDataField,  // Pointer on data field
    uint32_t** ppSizeField, // Pointer on size field
    bool ignorePrevious);   // Copy last updated value of DBO on output ptr

/// @brief Set nb of significant bytes to 0 of a double buffer
/// @warning Not thread safe for concurrent WRITERS!
/// @warning Shall be called between GetWriteBuffer and ReleaseWriteBuffer
/// @param [in] p DBO object
/// @param [in] idBuffer Identifier of buffer returned by SOPC_DoubleBuffer_GetWriteBuffer
/// @return SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_DoubleBuffer_WriteBufferErase(SOPC_DoubleBuffer* p, // DBO object
                                                     uint32_t idBuffer);   // Id of buffer returned by GetWriteBuffer

/// @brief Release a reserved buffer after a write operation. Modification by SOPC_DoubleBuffer_WriteXXX are taken into
/// account.
/// @warning If you want cancel write operation, don't call this function!
/// @param [in] p DBO buffer object reference
/// @param [inout] pIdBuffer Id of buffer returned by SOPC_DoubleBuffer_GetWriteBuffer. Set to UINT32_MAX.
/// @return SOPC_STATUS_OK in case of success, else SOPC_STATUS_NOK.
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseWriteBuffer(SOPC_DoubleBuffer* p, // DBO buffer object reference
                                                       uint32_t* pIdBuffer); // Id of buffer return by GetWriteBuffer

/// @brief Get most up to date buffer to read. This buffer is marked as reading status.
/// @warning Shall always be called before ReadBuffer/ReadBufferPtr and ReleaseReadBuffer
/// @param [in] p DBO Object
/// @param [inout] pIdBuffer Address where buffer identifier is returned
/// @return Returns SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_DoubleBuffer_GetReadBuffer(SOPC_DoubleBuffer* p, // DBO Object
                                                  uint32_t* pIdBuffer); // Address point to id buffer

/// @brief Expose directly pointer on data buffer and return size of significant bytes
/// @warning Shall be called between GetReadBuffer and ReleaseBuffer
/// @param [in] p DBO Object
/// @param [in] idBuffer Buffer identifier returned by SOPC_DoubleBuffer_GetReadBuffer
/// @param [out] pData Address where is returned pointer on data buffer
/// @param [out] pDataToRead Address where is returned data size can be read from offset 0
/// @return Returns SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBufferPtr(SOPC_DoubleBuffer* p,   // DBO object
                                                  uint32_t idBuffer,      // Buffer identifier
                                                  uint8_t** pData,        // Pointer on data buffer
                                                  uint32_t* pDataToRead); // Returns data size can be read from offset 0

/// @brief Read buffer from offset, on sizeRequest
/// @param [in] p DBO object
/// @param [in] idBuffer Buffer identifier returned by SOPC_DoubleBuffer_GetReadBuffer
/// @param [in] offset Absolute offset in the internal buffer where start reading
/// @param [out] data Output buffer where data are returned
/// @param [in] sizeRequest Number of bytes to copy from internal buffer from its offset to output buffer
/// @param [out] pReadData Address where size of data read is returned
/// @return Returns SOPC_STATUS_OK in case of success
SOPC_ReturnStatus SOPC_DoubleBuffer_ReadBuffer(SOPC_DoubleBuffer* p, // DBO object
                                               uint32_t idBuffer,    // Buffer identifier
                                               uint32_t offset,      // Offset from start read
                                               uint8_t* data,        // Data output
                                               uint32_t sizeRequest, // Request of read
                                               uint32_t* pReadData); // Returns size of data read

/// @brief Release read buffer to allow writer to write.
/// @param [in] p DBO Object
/// @param [inout] pIdBuffer Address where can be found buffer identifier to release. Set to UINT32_MAX.
/// @return Returns SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_DoubleBuffer_ReleaseReadBuffer(SOPC_DoubleBuffer* p, // DBO object
                                                      uint32_t* pIdBuffer); // Buffer identifier

#endif /* SOPC_DBO_H_ */
