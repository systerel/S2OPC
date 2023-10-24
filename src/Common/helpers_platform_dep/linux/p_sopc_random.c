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
#include <time.h>

SOPC_ReturnStatus SOPC_GetRandom(SOPC_Buffer* buffer, uint32_t length)
{
    // Initialize the seed once
    static bool initialized = false;
    if (!initialized)
    {
        // Make a seed base on current time
        struct timespec ts;
        int has_get_time = timespec_get(&ts, TIME_UTC);
        if (!has_get_time)
        {
            return SOPC_STATUS_NOK;
        }

        srandom((unsigned int) (ts.tv_nsec ^ ts.tv_sec));
        initialized = true;
    }

    // Get the number of random ints we will need
    const size_t intLen = (length + sizeof(int) - 1) / sizeof(int);

    int* random_data = SOPC_Malloc(intLen * sizeof(int));
    if (NULL == random_data)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Iterate in order to call random() multiple times
    for (unsigned int i = 0; i < intLen; i++)
    {
        random_data[i] = (int) random();
    }

    SOPC_ReturnStatus status = SOPC_Buffer_Write(buffer, (uint8_t*) random_data, length);
    SOPC_Free(random_data);
    return status;
}
