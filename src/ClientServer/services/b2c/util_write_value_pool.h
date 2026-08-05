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
 * \file util_write_value_pool.h
 * \brief Shared static pool of ::OpcUa_WriteValue shells for ::SOPC_WriteDataChanged.
 *
 * Producers of data-changed events allocate old/new WriteValue shells here;
 * the Services layer frees them via ::SOPC_WriteValuePool_ClearAndFree after
 * processing ::SE_TO_SE_SERVER_DATA_CHANGED. Only the WriteValue struct is
 * pooled; Variant/NodeId payloads remain heap-owned and are released by Clear.
 *
 * Not thread-safe: intended for the single Services thread (same as other
 * static pools in this layer). Do not allocate shells that are freed on the
 * Application looper (address-space write notifications); those must stay on
 * the heap (::SOPC_Malloc).
 */

#ifndef UTIL_WRITE_VALUE_POOL_H_
#define UTIL_WRITE_VALUE_POOL_H_

#include <stddef.h>

#include "sopc_types.h"

/**
 * \brief Initializes the shared WriteValue pool (once at toolkit start).
 */
void SOPC_WriteValuePool_Initialize(void);

/**
 * \brief Allocates one ::OpcUa_WriteValue from the pool, initialized as by ::OpcUa_WriteValue_Initialize.
 * \return An initialized WriteValue, or NULL on fallback OOM.
 */
OpcUa_WriteValue* SOPC_WriteValuePool_Alloc(void);

/**
 * \brief Clears (only frees allocated memory in) the WriteValue then returns its shell to the pool (or heap).
 * \param wv  WriteValue from ::SOPC_WriteValuePool_Alloc; NULL is a no-op.
 */
void SOPC_WriteValuePool_ClearAndFree(OpcUa_WriteValue* wv);

/**
 * \brief Cumulative ::SOPC_Malloc fallback count since Initialize (never decremented).
 * \return Fallback allocation event count for sizing/observability.
 */
size_t SOPC_WriteValuePool_GetNbFallbackAllocsStat(void);

/**
 * \brief Fast-path equivalent of ::OpcUa_WriteValue_Initialize: calls the same
 *        underlying typed Initialize functions (NodeId, String, DataValue...)
 *        directly instead of through the generic reflective
 *        SOPC_EncodeableObject_InternalInitialize field-descriptor dispatch.
 *        Behaviorally identical to the generic path; only avoids the per-field
 *        descriptor walk and indirect function-pointer calls it performs.
 * \param wv  WriteValue to initialize; must not be NULL.
 */
void SOPC_WriteValue_FastInitialize(OpcUa_WriteValue* wv);

/**
 * \brief Internal use only or for content never accessed after call,
          ::SOPC_Free(wv) shall be done after call and pointer set to NULL.
          This function free the allocated content in the write value
          without clearing anything else.
          It is used internally by the pool to optimize ::SOPC_WriteValuePool_ClearAndFree treatment.
 * \param wv  WriteValue to clear; must not be NULL.
 */
void SOPC_WriteValue_FastFreeContent(OpcUa_WriteValue* wv);

#endif /* UTIL_WRITE_VALUE_POOL_H_ */
