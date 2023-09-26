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

#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h> // ensure __SIZEOF_POINTER__ is defined

#include "sopc_atomic.h"

/* Notes :
 * - In Zephyr, atomic_t/atomic_val_t are defined as a "long", which is large enough to hold an int32_t
 * - In Zephyr, atomic_ptr_t/atomic_ptr_val_t are defined as a "void*"
 */

int32_t SOPC_Atomic_Int_Get(int32_t* atomic)
{
    return (int32_t) atomic_get((atomic_t*) atomic);
}

void SOPC_Atomic_Int_Set(int32_t* atomic, int32_t val)
{
    atomic_set((atomic_t*) atomic, (atomic_val_t) val);
}

int32_t SOPC_Atomic_Int_Add(int32_t* atomic, int32_t val)
{
    return (int32_t) atomic_add((atomic_t*) atomic, (atomic_val_t) val);
}

void* SOPC_Atomic_Ptr_Get(void** atomic)
{
    return (void*) atomic_ptr_get((atomic_ptr_t*) atomic);
}

void SOPC_Atomic_Ptr_Set(void** atomic, void* val)
{
    atomic_ptr_set((atomic_ptr_t*) atomic, (atomic_ptr_val_t) val);
}
