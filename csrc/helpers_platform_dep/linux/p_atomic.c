/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_atomic.h"

int32_t SOPC_Atomic_Int_Get(int32_t* atomic)
{
#if !defined(__clang__) && (__GNUC__ > 4)
    // This version works with TSan, the other one creates false positives...
    return (int32_t) __atomic_load_4(atomic, __ATOMIC_SEQ_CST);
#else
    __sync_synchronize();
    return *atomic;
#endif
}

void SOPC_Atomic_Int_Set(int32_t* atomic, int32_t val)
{
#if !defined(__clang__) && (__GNUC__ > 4)
    __atomic_store_4(atomic, (unsigned int) val, __ATOMIC_SEQ_CST);
#else
    *atomic = val;
    __sync_synchronize();
#endif
}

int32_t SOPC_Atomic_Int_Add(int32_t* atomic, int32_t val)
{
    return __sync_fetch_and_add(atomic, val);
}

void* SOPC_Atomic_Ptr_Get(void** atomic)
{
#if !defined(__clang__) && (__GNUC__ > 4)

#if SOPC_PTR_SIZE == 4
    return (void*) __atomic_load_4(atomic, __ATOMIC_SEQ_CST);
#elif SOPC_PTR_SIZE == 8
    return (void*) __atomic_load_8(atomic, __ATOMIC_SEQ_CST);
#else
#error "Unsupported pointer size"
#endif // SOPC_PTR_SIZE

#else
    __sync_synchronize();
    return *atomic;
#endif // !defined(clang)
}

void SOPC_Atomic_Ptr_Set(void** atomic, void* val)
{
#if !defined(__clang__) && (__GNUC__ > 4)

#if SOPC_PTR_SIZE == 4
    __atomic_store_4(atomic, (uintptr_t) val, __ATOMIC_SEQ_CST);
#elif SOPC_PTR_SIZE == 8
    __atomic_store_8(atomic, (uintptr_t) val, __ATOMIC_SEQ_CST);
#else
#error "Unsupported pointer size"
#endif // SOPC_PTR_SIZE

#else
    *atomic = val;
    __sync_synchronize();
#endif // !defined(clang)
}
