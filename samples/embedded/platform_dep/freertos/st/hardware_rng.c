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

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT

#include <string.h>

#include "entropy_poll.h"
#include "main.h"
#include "mbedtls/entropy.h"
#include "stm32h7xx_hal.h"

extern RNG_HandleTypeDef hrng;

int mbedtls_hardware_poll(void* data, unsigned char* output, size_t len, size_t* olen)
{
    if (NULL == olen || NULL == output)
    {
        return -1;
    }
    uint32_t index;
    uint32_t randomValue;
    *olen = 0;
    int ret = 0;
    for (index = 0; index < len && 0 == ret; index++)
    {
        if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
        {
            *olen += 1;
            memset(&(output[index]), (int) randomValue, 1);
        }
        else
        {
            ret = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }
    }

    return ret;
}

#endif /*MBEDTLS_ENTROPY_HARDWARE_ALT*/
