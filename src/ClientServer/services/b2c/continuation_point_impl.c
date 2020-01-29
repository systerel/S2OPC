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

#include "continuation_point_impl.h"

#include <assert.h>

#include <sopc_buffer.h>
#include <sopc_encoder.h>

SOPC_ReturnStatus SOPC_ContinuationPointId_Encode(uint64_t continuationPointId, SOPC_ByteString* bs)
{
    SOPC_Buffer tmpBuf;
    assert(bs != NULL);
    SOPC_ReturnStatus status = SOPC_ByteString_InitializeFixedSize(bs, sizeof(continuationPointId));
    if (SOPC_STATUS_OK == status)
    {
        // Mapping the bytestring onto the buffer
        tmpBuf.data = bs->Data;
        tmpBuf.maximum_size = sizeof(continuationPointId);
        tmpBuf.initial_size = tmpBuf.maximum_size;
        tmpBuf.current_size = tmpBuf.maximum_size;
        tmpBuf.length = 0;
        tmpBuf.position = 0;

        status = SOPC_UInt64_Write(&continuationPointId, &tmpBuf, 0);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_ByteString_Clear(bs);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ContinuationPointId_Decode(const SOPC_ByteString* bs, uint64_t* continuationPointId)
{
    SOPC_Buffer tmpBuf;
    assert(bs != NULL);
    if (bs->Length != sizeof(*continuationPointId))
    {
        return SOPC_STATUS_NOK;
    }

    // Mapping the bytestring onto the buffer
    tmpBuf.data = bs->Data;
    tmpBuf.maximum_size = sizeof(*continuationPointId);
    tmpBuf.initial_size = tmpBuf.maximum_size;
    tmpBuf.current_size = tmpBuf.maximum_size;
    tmpBuf.length = sizeof(*continuationPointId);
    tmpBuf.position = 0;

    return SOPC_UInt64_Read(continuationPointId, &tmpBuf, 0);
}
