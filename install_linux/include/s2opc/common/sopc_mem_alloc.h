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

void* SOPC_Malloc(size_t size);
void SOPC_Free(void* ptr);
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
