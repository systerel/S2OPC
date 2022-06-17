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

/** \brief Allocates memory.
 *
 * This function acts as standard 'malloc' except that it ensures that a non-NULL pointer is returned even if the
 * requested size is zero
 *
 * \param size The amount of memory requested.
 * \return a pointer to the new allocated area, which has at least the requested size.
 *      The caller is responsible for calling SOPC_Free when the memory is no more required.
 *      NULL is returned if the memory cannot be allocated.
 *      The memory pointed to is initialized to '\0'
 */
void* SOPC_Malloc(size_t size);

/** \brief Frees memory.
 *
 * This function acts as standard 'free'.
 * \param ptr The previoulsy allocated pointer. Nothing is done if ptr is NULL.
 *      The caller shall ensure that the same pointer is not freed several times, and that
 *      only pointers previously allocated by SOPC_xalloc are passed as parameter.
 */
void SOPC_Free(void* ptr);

/** \brief Allocates memory.
 *
 * This function acts as standard 'calloc'. However, if size is passed with the value "0", a non-null
 *      pointer shall be returned.
 * \param size The size of each element allocated
 * \param nmemb The number of adjacent element requested.
 * \return a pointer to the new allocated area (total size equals size *nmemb), which has at least the requested size.
 *      The caller is responsible for calling SOPC_Free when the memory is not more required.
 *      The allocated memory never takes into account alignment considerations, whatever the value of size is.
 *      NULL is returned if the memory cannot be allocated.
 *      The memory pointed to is not initialized.
 */
void* SOPC_Calloc(size_t nmemb, size_t size);

/** \brief Re-allocates memory.
 *
 * \warning Deprecated. New implementations should not use this function.
 *
 * This function acts as realloc. However, realloc may not exist on the targetted architecture.
 * In these cases, SOPC_Realloc maps to malloc + memcpy.
 * For this operation, the current size of the buffer is required.
 */
void* SOPC_Realloc(void* ptr, size_t old_size, size_t new_size);

#endif /* SOPC_MEM_ALLOC_H_ */
