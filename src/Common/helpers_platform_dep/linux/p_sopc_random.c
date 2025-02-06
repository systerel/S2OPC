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

#include "sopc_mem_alloc.h"
#include "sopc_random.h"

#include <stdio.h>
#include <stdlib.h>

SOPC_ReturnStatus SOPC_GetRandom(SOPC_Buffer* buffer, uint32_t length)
{
    // Check parameters
    if (NULL == buffer || 0 == length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint8_t* random_data = SOPC_Malloc(length * sizeof(uint8_t));
    if (NULL == random_data)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    FILE* file = fopen("/dev/urandom", "rb");
    if (NULL == file)
    {
        return SOPC_STATUS_NOK;
    }

    size_t read_len = fread(random_data, 1, length, file);
    fclose(file);
    if (read_len != length)
    {
        SOPC_Free(random_data);
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status = SOPC_Buffer_Write(buffer, random_data, length);
    SOPC_Free(random_data);
    return status;
}
