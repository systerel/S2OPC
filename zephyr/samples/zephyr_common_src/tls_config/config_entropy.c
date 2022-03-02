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

#include <random/rand32.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

/* Zephyr includes */
#include <device.h>
#include <drivers/entropy.h>

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

#if defined(CONFIG_MBEDTLS)
#if !defined(CONFIG_MBEDTLS_CFG_FILE)
#include "mbedtls/config.h"
#else
#include CONFIG_MBEDTLS_CFG_FILE
#endif /* CONFIG_MBEDTLS_CFG_FILE */
#endif

#define P_ENTROPY_DEBUG (0)

int32_t mbedtls_hardware_poll(void* data, uint8_t* output, int32_t len, int32_t* olen)
{
    (void) data;
#if P_ENTROPY_DEBUG == 1
    printk("\r\n mbedtls_hardware_poll - %d - \r\n", len);
#endif

    int err = (-1);

    if ((NULL == output) || (NULL == olen) || (0 == len))
    {
        return -1;
    }
#ifdef DT_CHOSEN_ZEPHYR_ENTROPY_LABEL
    /* static to obtain it once in a first call */
    static const struct device* dev = NULL;
    if (NULL == dev)
    {
#if P_ENTROPY_DEBUG == 1
        printk("\r\nFirst call to obtain entropy device %s\r\n", DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);
#endif
        dev = device_get_binding(DT_CHOSEN_ZEPHYR_ENTROPY_LABEL);
        if (NULL == dev)
        {
#if P_ENTROPY_DEBUG == 1
            printk("\r\nFailed to obtain entropy device\r\n");
#endif
        }
    }
    err = entropy_get_entropy(dev, output, len);
#endif
    if (err != 0)
    {
#if P_ENTROPY_DEBUG == 1
        printk("\r\nFailed to obtain entropy, err %d, software mode engaged !!!\r\n", err);
#endif
        uint32_t lenIter = 0;
        uint32_t blocksize = 4;
        uint32_t random_num = 0;
        while (lenIter < len)
        {
            random_num = k_cycle_get_32();
            if ((len - lenIter) < sizeof(random_num))
            {
                blocksize = lenIter;
                (void) memcpy(&(output[lenIter]), &random_num, len - lenIter);
            }
            else
            {
                *((uint32_t*) &output[lenIter]) = random_num;
            }
            lenIter += blocksize;
        }
    }
#if P_ENTROPY_DEBUG == 1
    printk("\r\n Entropy = ");
    for (int32_t i = 0; i < len; i++)
    {
        printk("%02X ", output[i]);
    }
    printk("\r\n");
#endif
    *olen = len;
    return 0;
}
