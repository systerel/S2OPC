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

#if defined(__has_include)
#if __has_include(<mbedtls/build_info.h>)
#include <mbedtls/build_info.h>
#else
#include <mbedtls/config.h>
#endif
#else
#include <mbedtls/config.h>
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include <mbedtls/memory_buffer_alloc.h>
#endif

#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
#include <mbedtls/threading.h>
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C) && !defined(MBEDTLS_HEAP_SIZE)
#define MBEDTLS_HEAP_SIZE (64U * 1024U)
#endif

/*
 * Place the private Mbed TLS heap in the requested memory section.
 * MBEDTLS_HEAP_SIZE / MBEDTLS_HEAP_SECTION are provided by the active
 * config-mbedtls.h + config_custom_mbedtls.h combination. Keep the legacy
 * CONFIG_SOPC_ALLOC_SECTION fallback for existing applications.
 */
#if defined(CONFIG_SOPC_ALLOC_SECTION)
#undef MBEDTLS_HEAP_SECTION
#define MBEDTLS_HEAP_SECTION __attribute__((section(CONFIG_SOPC_ALLOC_SECTION)))
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
MBEDTLS_HEAP_SECTION static unsigned char _mbedtls_heap[MBEDTLS_HEAP_SIZE];
#endif

#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
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
#endif

void tls_threading_initialize(void)
{
#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
#endif
#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    mbedtls_memory_buffer_alloc_init(_mbedtls_heap, sizeof(_mbedtls_heap));
#endif
}
