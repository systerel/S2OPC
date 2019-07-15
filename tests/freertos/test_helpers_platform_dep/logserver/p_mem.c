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
//
// --wrap option shall be used to wrap default newlib implementation of malloc functions :
// Add to your linker those option :
// --wrap=_malloc_r --wrap=_calloc_r --wrap=_free_r --wrap=_realloc_r
// --wrap=malloc --wrap=calloc --wrap=realloc --wrap=free

struct T_BLOCK_INFO
{
    struct T_BLOCK_INFO* pNextFreeBlock;
    size_t blockSize;
};

// This constant is the size of a block info with alignement constraints
static const size_t heap4StackInfo =
    (sizeof(struct T_BLOCK_INFO) + ((size_t)(portBYTE_ALIGNMENT - 1))) & ~((size_t) portBYTE_ALIGNMENT_MASK);
// This constant define the bit of allocated status used by FreeRTOS in the blockSize field of block info
static const size_t xBlockAllocatedBit = ((size_t) 1) << ((sizeof(size_t) * 8) - 1);

// This counters are used to trace malloc and free
static uint32_t gFreeRTOSTotalMalloc = 0;
static uint32_t gFreeRTOSTotalFree = 0;
static uint32_t bOverflowDetected = 0;
static uint32_t bMallocFailed = 0;

/*---------HEAP_4 calloc and realloc--------------------------------------------------*/

void* pvPortCalloc(size_t num, size_t size)
{
    if (size > (SIZE_MAX / num))
    {
        return NULL;
    }
    size_t totalSize = num * size;
    unsigned char* p = (unsigned char*) pvPortMalloc(totalSize);
    memset(p, 0, totalSize);
    return p;
}

void* pvPortRealloc(void* aptr, size_t nbytes)
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
        memset(n_ptr + (pBlockInfo->blockSize & ~xBlockAllocatedBit), 0,
               nbytes - (pBlockInfo->blockSize & ~xBlockAllocatedBit));
    }

    taskEXIT_CRITICAL();
    // Free source
    vPortFree(aptr);
    return n_ptr;
}

// Malloc wrapping function
void* __attribute__((weak)) _malloc_r(void* reent, size_t size)
{
    void* ptr = NULL;
    ptr = pvPortMalloc(size);
    return ptr;
}

void* __attribute__((weak)) _realloc_r(void* reent, void* arg, size_t size)
{
    void* ptr = NULL;
    ptr = pvPortRealloc(arg, size);
    return ptr;
}

void* __attribute__((weak)) _calloc_r(void* reeant, size_t n, size_t s)
{
    void* ptr = NULL;
    ptr = pvPortCalloc(n, s);
    return ptr;
}

void __attribute__((weak)) _free_r(void* reent, void* ptr)
{
    vPortFree(ptr);
}

void* __attribute__((weak)) malloc(size_t size)
{
    void* ptr = NULL;
    ptr = pvPortMalloc(size);
    return ptr;
}

void* __attribute__((weak)) realloc(void* arg, size_t size)
{
    void* ptr = NULL;
    ptr = pvPortRealloc(arg, size);
    return ptr;
}

void* __attribute__((weak)) calloc(size_t n, size_t s)
{
    void* ptr = NULL;
    ptr = pvPortCalloc(n, s);
    return ptr;
}

void __attribute__((weak)) free(void* ptr)
{
    vPortFree(ptr);
}

// New lib Cleanup stdout reentrant structure. Called by freeRTOS.
// This structure is redefine, because NewLib consider that reent structure is allocated
// from the stack, and not from the heap.
void __attribute__((weak)) _reclaim_reent(struct _reent* reent)
{
    if (reent != NULL)
    {
        if (reent->__sf[1]._bf._base != NULL)
        {
            vPortFree(reent->__sf[1]._bf._base);
        }
        if (reent->__sf[1]._ub._base != NULL)
        {
            vPortFree(reent->__sf[1]._ub._base);
        }
        if (reent->__sf[1]._lb._base != NULL)
        {
            vPortFree(reent->__sf[1]._lb._base);
        }
    }
}

// New lib lock functions redefinition for thread safe buffer
void __attribute__((weak)) __malloc_lock()
{
    vTaskSuspendAll();
};

void __attribute__((weak)) __malloc_unlock()
{
    xTaskResumeAll();
};

void __attribute__((weak)) __env_lock()
{
    vTaskSuspendAll();
};

void __attribute__((weak)) __env_unlock()
{
    xTaskResumeAll();
};

// FreeRTOS trace function, defines macro traceMALLOC and traceFREE defined by FreeRTOS_Config.h
void freeRTOS_TRACE_MALLOC(void* pvAddress, uint32_t uiSize)
{
    if ((gFreeRTOSTotalMalloc + 1) < UINT32_MAX)
    {
        gFreeRTOSTotalMalloc++;
    }
}

void freeRTOS_TRACE_FREE(void* pvAddress, uint32_t uiSize)
{
    if ((gFreeRTOSTotalFree + 1) < UINT32_MAX)
    {
        gFreeRTOSTotalFree++;
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    bOverflowDetected = 0xAAAAAAAA;
}

void vApplicationMallocFailedHook(void)
{
    bMallocFailed++;
}
