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

#include "threading_alt.h"

// Must be included first
#include "config_custom_mbedtls.h"

#include <mbedtls/memory_buffer_alloc.h>

#include <mbedtls/threading.h>
#include <zephyr/linker/section_tags.h>

// Place MBEDTLS HEAP into a different RAM section
#ifdef CONFIG_SOPC_ALLOC_SECTION
__attribute__((section(CONFIG_SOPC_ALLOC_SECTION)))
#endif
static unsigned char _mbedtls_heap[MBEDTLS_HEAP_SIZE];

static void mutex_init(mbedtls_threading_mutex_t* pMutex)
{
    int res_lock = 0;
    res_lock = Mutex_Initialization(pMutex);
    // printk("\r\nres init = %d - %d\r\n", res_lock, (uint32_t)*pMutex);
}

static void mutex_free(mbedtls_threading_mutex_t* pMutex)
{
    int res_lock = 0;
    res_lock = Mutex_Clear(pMutex);
    // printk("\r\nres clear = %d\r\n", res_lock);
}

static int mutex_lock(mbedtls_threading_mutex_t* pMutex)
{
    int res_lock = 0;
    res_lock = Mutex_Lock(pMutex);
    // printk("\r\nres lock = %d - %d\r\n", res_lock, (uint32_t)*pMutex);
    return (int) res_lock;
}

static int mutex_unlock(mbedtls_threading_mutex_t* pMutex)
{
    int res_lock = 0;
    res_lock = Mutex_Unlock(pMutex);
    // printk("\r\nres unlock = %d - %d\r\n", res_lock, (uint32_t)*pMutex);
    return (int) res_lock;
}

void tls_threading_initialize(void)
{
    printk("\r\n Thread safe mbedtls init\r\n");
    mbedtls_memory_buffer_alloc_init(_mbedtls_heap, sizeof(_mbedtls_heap));
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
}
