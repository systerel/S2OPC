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

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* S2OPC includes */
#include "sopc_macros.h"

#include "threading_alt.h"

static void mutex_init(mbedtls_threading_mutex_t* pMutex)
{
    SOPC_UNUSED_RESULT(SOPC_Mutex_Initialization(pMutex));
}

static void mutex_free(mbedtls_threading_mutex_t* pMutex)
{
    SOPC_UNUSED_RESULT(SOPC_Mutex_Clear(pMutex));
}

static int mutex_lock(mbedtls_threading_mutex_t* pMutex)
{
    return (int) SOPC_Mutex_Lock(pMutex);
}

static int mutex_unlock(mbedtls_threading_mutex_t* pMutex)
{
    return (int) SOPC_Mutex_Unlock(pMutex);
}

/*---------Mutex-------- --------------------------------------------------*/

void mbedtls_threading_set_alt(void (*mutex_init)(mbedtls_threading_mutex_t*),
                               void (*mutex_free)(mbedtls_threading_mutex_t*),
                               int (*mutex_lock)(mbedtls_threading_mutex_t*),
                               int (*mutex_unlock)(mbedtls_threading_mutex_t*));

void mbedtls_threading_initialize(void)
{
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
}
