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

#include <stdlib.h>

#include "device.h"
#include "drivers/entropy.h"

#if defined(CONFIG_MBEDTLS)
#if !defined(CONFIG_MBEDTLS_CFG_FILE)
#include "mbedtls/config.h"
#else
#include CONFIG_MBEDTLS_CFG_FILE
#endif /* CONFIG_MBEDTLS_CFG_FILE */
#endif

#ifndef CONFIG_ENTROPY_NAME
#define CONFIG_ENTROPY_NAME ((const char*) ("TRNG"))
#endif

int32_t mbedtls_hardware_poll(void* data, uint8_t* output, int32_t len, int32_t* olen)
{
    (void) data;
    /* static to obtain it once in a first call */
    static struct device* dev = NULL;
    int err = (-1);

    if ((NULL == output) || (NULL == olen) || (0 == len))
    {
        return -1;
    }

    if (NULL == dev)
    {
        dev = device_get_binding(CONFIG_ENTROPY_NAME);
        if (NULL == dev)
        {
            return -1;
        }
    }

    err = entropy_get_entropy(dev, output, len);
    if (err != 0)
    {
        return -1;
    }

    *olen = len;
    return 0;
}
