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

#include "p_mem.h"

// Block info structure is used by heap_4.c. If another memory model is used, this code is not compatible
// This bloc is defined as private structure by heap_4.c
struct T_BLOCK_INFO
{
    struct T_BLOCK_INFO* pNextFreeBlock;
    size_t blockSize;
};

// This constant is the size of a block info with alignement constraints
static size_t heap4StackInfo =
    (sizeof(struct T_BLOCK_INFO) + ((size_t)(portBYTE_ALIGNMENT - 1))) & ~((size_t) portBYTE_ALIGNMENT_MASK);
// This constant define the bit of allocated status used by FreeRTOS in the blockSize field of block info
static size_t xBlockAllocatedBit = ((size_t) 1) << ((sizeof(size_t) * 8) - 1);

void* SOPC_Malloc(size_t size)
{
    return pvPortMalloc(size);
}

void* SOPC_Realloc(void* aptr, size_t nbytes)
{
    configASSERT(configFRTOS_MEMORY_SCHEME == 4);
    struct T_BLOCK_INFO* pBlockInfo = NULL;
    uint8_t* ptr = (uint8_t*) aptr;
    uint8_t* n_ptr = NULL;

    if (aptr == NULL)
    {
        return pvPortMalloc(nbytes);
    }

    n_ptr = pvPortMalloc(nbytes);
    if (n_ptr == NULL)
    {
        return NULL;
    }

    // Retrieve current heap 4 bloc information

    taskENTER_CRITICAL();

    ptr -= heap4StackInfo;
    pBlockInfo = ((struct T_BLOCK_INFO*) ptr);
    configASSERT((pBlockInfo->blockSize & xBlockAllocatedBit) != 0);
    configASSERT(pBlockInfo->pNextFreeBlock == NULL);

    // Copy source to destination
    if ((pBlockInfo->blockSize & ~xBlockAllocatedBit) > nbytes)
    {
        memcpy(n_ptr, aptr, nbytes);
    }
    else
    {
        memcpy(n_ptr, aptr, (pBlockInfo->blockSize & ~xBlockAllocatedBit));
    }

    taskEXIT_CRITICAL();
    // Free source
    vPortFree(aptr);
    return n_ptr;
}

void* SOPC_Calloc(size_t n, size_t s)
{
    uint8_t* p = NULL;
    size_t size = n * s;
    p = pvPortMalloc(size);
    if (p != NULL)
    {
        memset(p, 0, size);
    }
    return p;
}

void SOPC_Free(void* aptr)
{
    if (aptr != NULL)
    {
        vPortFree(aptr);
    }
}

#ifdef P_MEM_LIB_C_WRAPPER
void* __wrap_malloc(size_t size)
{
    void* ptr = NULL;
    vTaskSuspendAll();
    ptr = __real_malloc(size);
    xTaskResumeAll();
    return ptr;
}

void __wrap_free(void* aptr)
{
    vTaskSuspendAll();
    __real_free(aptr);
    xTaskResumeAll();
}

void* __wrap_calloc(size_t n, size_t s)
{
    void* ptr = NULL;
    vTaskSuspendAll();
    ptr = __real_calloc(n, s);
    xTaskResumeAll();
    return ptr;
}

void* __wrap_realloc(void* aptr, size_t nbytes)
{
    void* ptr = NULL;
    vTaskSuspendAll();
    ptr = __real_realloc(aptr, nbytes);
    xTaskResumeAll();
    return ptr;
}
#else
void* __wrap_malloc(size_t size)
{
    void* ptr = NULL;
    ptr = SOPC_Malloc(size);
    return ptr;
}

void __wrap_free(void* aptr)
{
    SOPC_Free(aptr);
}

void* __wrap_calloc(size_t n, size_t s)
{
    void* ptr = NULL;
    ptr = SOPC_Calloc(n, s);
    return ptr;
}

void* __wrap_realloc(void* aptr, size_t nbytes)
{
    void* ptr = NULL;
    ptr = SOPC_Realloc(aptr, nbytes);
    return ptr;
}
#endif
