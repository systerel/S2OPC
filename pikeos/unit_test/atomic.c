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

#include <vm.h>

#include "unit_test_include.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_enums.h"

void suite_test_atomic(int* index)
{
    vm_cprintf("\nTEST %d: sopc_atomic.h \n", *index);
    int32_t atomicCounter = 5;
    int32_t counter = 0;
    counter = SOPC_Atomic_Int_Get(&atomicCounter);
    SOPC_ASSERT(counter == atomicCounter);
    vm_cprintf("Test1 : ok\n");

    counter = 2;
    SOPC_Atomic_Int_Set(&atomicCounter, counter);
    SOPC_ASSERT(atomicCounter == counter);
    vm_cprintf("Test2 : ok\n");

    SOPC_Atomic_Int_Add(&atomicCounter, counter);
    counter += counter;
    SOPC_ASSERT(atomicCounter == counter);
    vm_cprintf("Test3 : ok\n");

    int32_t* pAtomicCounter = &counter;
    int32_t* pCounter = NULL;
    pCounter = (int32_t*) SOPC_Atomic_Ptr_Get((void**) &pAtomicCounter);
    SOPC_ASSERT(pCounter == pAtomicCounter);
    vm_cprintf("Test4 : ok\n");

    SOPC_Atomic_Ptr_Set((void**) &pAtomicCounter, &atomicCounter);
    SOPC_ASSERT(pAtomicCounter == &atomicCounter);
    vm_cprintf("Test5 : ok\n");
    *index += 1;
}
