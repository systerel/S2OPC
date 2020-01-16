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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "sopc_encodeabletype.h"
#include "sopc_types.h"
#include "sopc_buffer.h"
#include "sopc_mem_alloc.h"
#include "sopc_helper_endianness_cfg.h"

static int32_t counter = 0;

int LLVMFuzzerTestOneInput(const uint8_t* buf, size_t len)
{
    if (len <= 1)
    {
        return 0;
    }
    counter++;
    SOPC_Helper_EndiannessCfg_Initialize();
    //printf("Iteration #%d\n", counter);
    //printf("sizeofbuffer: %lu\n", sizeof(SOPC_Buffer));

    //printf("input (size: %lu):\n", len);
    for (size_t i = 0; i < len; i++)
    {
        //printf("%u ", buf[i]);
    }
    //printf("\n");

    const size_t type_index = buf[0] % SOPC_TypeInternalIndex_SIZE;

    /* get an encodeable type by looking in encodeableType array */
    SOPC_EncodeableType* type = SOPC_KnownEncodeableTypes[type_index];

    void* pValue = SOPC_Calloc(1, type->AllocationSize);
    if (NULL == pValue)
    {
        return 0;
    }

    SOPC_EncodeableObject_Initialize(type, pValue);

    /* create a buffer using remaining data */
    SOPC_Buffer* buffer = SOPC_Buffer_Attach((uint8_t*) &buf[1], (uint32_t) len - 1);

    SOPC_ReturnStatus status = SOPC_EncodeableObject_Decode(type,
                                                            pValue,
                                                            buffer);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_EncodeableObject_Clear(type, pValue);
    }
    //printf("- AfterClear, status: %d\n", status);

    /* clear */
    //SOPC_Buffer_Delete(buffer); // double free if uncommented
    SOPC_Free(buffer); // delete tries to free data which is also freed by libfuzzer
    SOPC_Free(pValue);

    //printf("- AfterFree\n");

    return 0;
}
