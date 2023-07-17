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

// Place MBEDTLS HEAP into a different RAM section
#ifdef CONFIG_SOPC_ALLOC_SECTION
__attribute__((section(CONFIG_SOPC_ALLOC_SECTION)))
#endif
static unsigned char _mbedtls_heap[MBEDTLS_HEAP_SIZE];

#ifdef CONFIG_MBEDTLS_USER_CONFIG_FILE
#include CONFIG_MBEDTLS_USER_CONFIG_FILE
#endif

#ifndef MBEDTLS_HEAP_SECTION
#define MBEDTLS_HEAP_SECTION
#endif

// Place MBEDTLS HEAP into a different RAM section
MBEDTLS_HEAP_SECTION static unsigned char _mbedtls_heap[MBEDTLS_HEAP_SIZE];

static void mutex_init(mbedtls_threading_mutex_t* pMutex)
{
    SOPC_Mutex_Initialization(pMutex);
}

static void mutex_free(mbedtls_threading_mutex_t* pMutex)
{
    SOPC_Mutex_Clear(pMutex);
}

static int mutex_lock(mbedtls_threading_mutex_t* pMutex)
{
    int res_lock = SOPC_Mutex_Lock(pMutex);
    return (int) res_lock;
}

static int mutex_unlock(mbedtls_threading_mutex_t* pMutex)
{
    int res_lock = SOPC_Mutex_Unlock(pMutex);
    return (int) res_lock;
}

void tls_threading_initialize(void)
{
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
    mbedtls_memory_buffer_alloc_init(_mbedtls_heap, sizeof(_mbedtls_heap));
}
