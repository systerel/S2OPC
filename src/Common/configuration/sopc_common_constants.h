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

/** \file
 *
 * \brief Contains the configuration constants used by S2OPC common library. Those constants could be modified for
 * specific use.
 *
 */

#ifndef SOPC_COMMON_CONSTANTS_H_
#define SOPC_COMMON_CONSTANTS_H_

#include <stdbool.h>

// TODO: move the next 3 constants to ClientServer constants but dependency shall be break in builtintypes

/** @brief Maximum chunk buffer size used (must be >= SOPC_TCP_UA_MIN_BUFFER_SIZE) */
#ifndef SOPC_TCP_UA_MAX_BUFFER_SIZE
#define SOPC_TCP_UA_MAX_BUFFER_SIZE UINT16_MAX
#endif /* SOPC_TCP_UA_MAX_BUFFER_SIZE */

/** @brief Maximum number of chunks accepted for 1 message, 0 means no limit.
 *  Note: if 0 is chosen SOPC_MAX_MESSAGE_LENGTH definition shall be changed not to use it and shall not be 0.
 */
#ifndef SOPC_MAX_NB_CHUNKS
#define SOPC_MAX_NB_CHUNKS 5
#endif /* SOPC_MAX_NB_CHUNKS */

/** @brief Maximum message length used (must be >= SOPC_TCP_UA_MAX_BUFFER_SIZE), 0 means no limit.
 *  Note: if 0 is chosen SOPC_MAX_NB_CHUNK shall not be 0.
 * */
#ifndef SOPC_MAX_MESSAGE_LENGTH
#define SOPC_MAX_MESSAGE_LENGTH SOPC_TCP_UA_MAX_BUFFER_SIZE* SOPC_MAX_NB_CHUNKS
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/** @brief Maximum ByteString/String/XmlElement length in bytes used */
#ifndef SOPC_MAX_STRING_LENGTH
#define SOPC_MAX_STRING_LENGTH UINT16_MAX
#endif /* SOPC_MAX_STRING_LENGTH */

/** @brief Maximum array length that could be stored in a variant */
#ifndef SOPC_MAX_ARRAY_LENGTH
#define SOPC_MAX_ARRAY_LENGTH 1000000
#endif /* SOPC_MAX_ARRAY_LENGTH */

/** @brief Maximum levels of nested diagnostic information structure
 *  Note: OPC UA specification v1.03 part 6 §5.2.2.12 indicates
 *  "Decoders shall support at least 100 nesting levels ..."*/
#ifndef SOPC_MAX_DIAG_INFO_NESTED_LEVEL
#define SOPC_MAX_DIAG_INFO_NESTED_LEVEL 100
#endif /* SOPC_MAX_DIAG_INFO_NESTED_LEVEL */

/** @brief Maximum levels of nested variant
 * (e.g.: variant containing array of variants / data value) */
#ifndef SOPC_MAX_VARIANT_NESTED_LEVEL
#define SOPC_MAX_VARIANT_NESTED_LEVEL 10
#endif /* SOPC_MAX_VARIANT_NESTED_LEVEL */

/* @brief Maximum number of elements in Async Queue */
#ifndef SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE
#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE 5000
#endif /* SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE */

#ifndef SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY
#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY true
#endif /* SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY */

/* Check use of uintptr_t is not an issue on the current platform */
#if UINTPTR_MAX < UINT32_MAX
#error "UINTPTR_MAX < UINT32_MAX whereas uintptr_t are used to store uint32_t values"
#endif

/* Check casts from uint32_t / int32_t to size_t are valid without verification */
#if SIZE_MAX < UINT32_MAX
#error "SIZE_MAX < UINT32_MAX whereas uint32_t are casted to size_t values"
#endif

/* Check casts from long to size_t are valid without verification */
#if SIZE_MAX < LONG_MAX
#error "SIZE_MAX < LONG_MAX whereas long are casted to size_t values"
#endif

#endif /* SOPC_COMMON_CONSTANTS_H_ */
