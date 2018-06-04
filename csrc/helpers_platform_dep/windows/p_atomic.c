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

#include <windows.h>

int32_t SOPC_Atomic_Int_Get(int32_t* atomic)
{
    MemoryBarrier();
    return *atomic;
}

void SOPC_Atomic_Int_Set(int32_t* atomic, int32_t val)
{
    *atomic = val;
    MemoryBarrier();
}

int32_t SOPC_Atomic_Int_Add(int32_t* atomic, int32_t val)
{
    return (int32_t) InterlockedExchangeAdd((LONG*) atomic, (LONG) val);
}

void* SOPC_Atomic_Ptr_Get(void** atomic)
{
    MemoryBarrier();
    return *atomic;
}

void SOPC_Atomic_Ptr_Set(void** atomic, void* val)
{
    *atomic = val;
    MemoryBarrier();
}
