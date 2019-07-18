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

/**
 * \file sopc_mem_alloc.h
 * \brief S2OPC memory allocation wrappers.
 */

#ifndef SOPC_MEM_ALLOC_H_
#define SOPC_MEM_ALLOC_H_

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG_MEMORY_LEAKS
#ifndef DEBUG_MEMORY_LEAKS

void* SOPC_Malloc(size_t size);
void SOPC_Free(void* ptr);
void* SOPC_Calloc(size_t nmemb, size_t size);
void* SOPC_Realloc(void* ptr, size_t old_size, size_t new_size);

#else

#include "FreeRTOS.h"
#include "Task.h"
#include "p_logsrv.h"

void* SOPC_Malloc_(size_t size);
void SOPC_Free_(void* ptr);
void* SOPC_Calloc_(size_t nmemb, size_t size);
void* SOPC_Realloc_(void* ptr, size_t old_size, size_t new_size);

#undef SOPC_FreeD
#define SOPC_FreeD(ptr)                                                                                      \
    ({                                                                                                       \
        char ptr2[256] = {0};                                                                                \
                                                                                                             \
        snprintf(ptr2, 255, "FREE\t;\t;%08X\t;%s\t;%u\t;%s\r\n", (unsigned int) xTaskGetCurrentTaskHandle(), \
                 __FILENAME__, __LINE__, __FUNCTION__);                                                      \
                                                                                                             \
        SOPC_LogSrv_Print((uint8_t*) ptr2, strlen(ptr2));                                                    \
                                                                                                             \
        SOPC_Free_(ptr);                                                                                     \
    })

#undef SOPC_ReallocD
#define SOPC_ReallocD(ptr, old_size, new_size)                                                      \
    ({                                                                                              \
        void* _ptr;                                                                                 \
        char ptr2[256] = {0};                                                                       \
                                                                                                    \
        snprintf(ptr2, 255, "REALLOC\t;%u\t;%08X\t;%s\t;%u\t;%s\r\n", (unsigned int) (new_size),    \
                 (unsigned int) xTaskGetCurrentTaskHandle(), __FILENAME__, __LINE__, __FUNCTION__); \
                                                                                                    \
        SOPC_LogSrv_Print((uint8_t*) ptr2, strlen(ptr2));                                           \
        _ptr = SOPC_Realloc_(ptr, old_size, new_size);                                              \
        _ptr;                                                                                       \
    })

#undef SOPC_CallocD
#define SOPC_CallocD(nmemb, size)                                                                               \
    ({                                                                                                          \
        void* _ptr;                                                                                             \
        /*fprintf(stdout, "CALLOC of %u bytes from thread=%08X:file=%s:line=%u:function=%s\r\n",  */            \
        /* (unsigned int) (nmemb * size), (unsigned int) xTaskGetCurrentTaskHandle(), __FILENAME__, __LINE__,*/ \
        /* __FUNCTION__);     */                                                                                \
                                                                                                                \
        char ptr2[256] = {0};                                                                                   \
                                                                                                                \
        snprintf(ptr2, 255, "CALLOC\t;%u\t;%08X\t;%s\t;%u\t;%s\r\n", (unsigned int) (nmemb * size),             \
                 (unsigned int) xTaskGetCurrentTaskHandle(), __FILENAME__, __LINE__, __FUNCTION__);             \
                                                                                                                \
        SOPC_LogSrv_Print((uint8_t*) ptr2, strlen(ptr2));                                                       \
                                                                                                                \
        _ptr = SOPC_Calloc_(nmemb, size);                                                                       \
        _ptr;                                                                                                   \
    })

#undef SOPC_MallocD
#define SOPC_MallocD(size)                                                                                        \
    ({                                                                                                            \
        void* _ptr;                                                                                               \
        /*fprintf(stdout, "MALLOC of %u bytes from thread=%08X:file=%s:line=%u:function=%s\r\n", (unsigned int)*/ \
        /* (size),*/                                                                                              \
        /*     (unsigned int) xTaskGetCurrentTaskHandle(), __FILENAME__, __LINE__, __FUNCTION__);          */     \
                                                                                                                  \
        char ptr2[256] = {0};                                                                                     \
                                                                                                                  \
        snprintf(ptr2, 255, "MALLOC\t;%u\t;%08X\t;%s\t;%u\t;%s\r\n", (unsigned int) (size),                       \
                 (unsigned int) xTaskGetCurrentTaskHandle(), __FILENAME__, __LINE__, __FUNCTION__);               \
                                                                                                                  \
        SOPC_LogSrv_Print((uint8_t*) ptr2, strlen(ptr2));                                                         \
                                                                                                                  \
        _ptr = SOPC_Malloc_(size);                                                                                \
        _ptr;                                                                                                     \
    })

#define SOPC_Malloc(size) SOPC_MallocD(size)
#define SOPC_Realloc(ptr, old_size, size) SOPC_ReallocD(ptr, old_size, size)
#define SOPC_Calloc(nmemb, size) SOPC_CallocD(nmemb, size)
#define SOPC_Free(ptr) SOPC_FreeD(ptr)

#endif

#endif /* SOPC_MEM_ALLOC_H_ */
