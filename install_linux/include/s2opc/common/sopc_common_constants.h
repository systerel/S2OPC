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
#include <stdint.h>

#include "sopc_protocol_constants.h"

/** @brief Configuration structure of message and types encoding limits **/
typedef struct SOPC_Common_EncodingConstants
{
    /* TCP UA configuration */
    uint32_t buffer_size;
    uint32_t receive_max_msg_size;
    uint32_t receive_max_nb_chunks;
    uint32_t send_max_msg_size;
    uint32_t send_max_nb_chunks;

    /* OPC UA types configuration */
    int32_t max_string_length; // reception only
    int32_t max_array_length;  // reception only
    uint32_t max_nested_diag_info;
    uint32_t max_nested_struct;
} SOPC_Common_EncodingConstants;

/**
 * \brief Get the default encoding constants (contains values below by default)
 *
 * \return The default value of the encoding constants
 */
SOPC_Common_EncodingConstants SOPC_Common_GetDefaultEncodingConstants(void);

/**
 * \brief Set the encodings constants with current structure value. It shall be done before initialization and cannot be
 * called twice.
 *
 * \param config  The new encoding constants structure to be configured. It should be a modified version of
 * SOPC_Common_GetDefaultEncodingConstants().
 *
 * \return True if it succeeds, false otherwise if it is not the first call or if it is not compliant with constraints
 * defined on default values below
 */
bool SOPC_Common_SetEncodingConstants(SOPC_Common_EncodingConstants config);

/** @brief Maximum chunk buffer size used (must be >= SOPC_TCP_UA_MIN_BUFFER_SIZE) */
#ifndef SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE
#define SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE UINT16_MAX
#endif /* SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE */

/** @brief Maximum number of chunks received accepted for 1 message, 0 means no limit.
 *  Note: if 0 is chosen SOPC_RECEIVE_MAX_MESSAGE_LENGTH definition shall be changed not to use it and shall not be 0.
 */
#ifndef SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS
#define SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS 5
#endif /* SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS */

/** @brief Maximum message length accepted in reception (must be >= SOPC_TCP_UA_MAX_BUFFER_SIZE), 0 means no limit.
 *  Note: if 0 is chosen SOPC_RECEIVE_MAX_NB_CHUNKS shall not be 0.
 * */
#ifndef SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH
#define SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE* SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS
#endif /* SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH */

/** @brief Maximum number of chunks sent for 1 message, 0 means no limit.
 *  Note: if 0 is chosen SOPC_SEND_MAX_MESSAGE_LENGTH definition shall be changed not to use it and shall not be 0.
 */
#ifndef SOPC_DEFAULT_SEND_MAX_NB_CHUNKS
#define SOPC_DEFAULT_SEND_MAX_NB_CHUNKS 12
#endif /* SOPC_DEFAULT_SEND_MAX_NB_CHUNKS */

/** @brief Maximum message length sent (must be >= SOPC_TCP_UA_MAX_BUFFER_SIZE), 0 means no limit.
 *  Note: if 0 is chosen SOPC_SEND_MAX_NB_CHUNKS shall not be 0.
 * */
#ifndef SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH
#define SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE* SOPC_DEFAULT_SEND_MAX_NB_CHUNKS
#endif /* SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH */

/** @brief Maximum ByteString/String/XmlElement length in bytes used */
#ifndef SOPC_DEFAULT_MAX_STRING_LENGTH
#define SOPC_DEFAULT_MAX_STRING_LENGTH UINT16_MAX
#endif /* SOPC_DEFAULT_MAX_STRING_LENGTH */

/** @brief Maximum array length that could be stored in a variant */
#ifndef SOPC_DEFAULT_MAX_ARRAY_LENGTH
#define SOPC_DEFAULT_MAX_ARRAY_LENGTH 1000000
#endif /* SOPC_DEFAULT_MAX_ARRAY_LENGTH */

/** @brief Maximum levels of nested diagnostic information structure
 *  Note: OPC UA specification v1.03 part 6 §5.2.2.12 indicates
 *  "Decoders shall support at least 100 nesting levels ..."*/
#ifndef SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL
#define SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL 100
#endif /* SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL */

/** @brief Maximum levels of nested structs, excluding Diagnostic Information which is
 * handled by SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL */
#ifndef SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL
#define SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL 50
#endif /* SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL */

/* @brief Maximum number of elements in Async Queue */
#ifndef SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE
#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE 5000
#endif /* SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE */

#ifndef SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY
#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY true
#endif /* SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY */

/** \brief Maximum length of a User-defined log line. */
#ifndef SOPC_LOG_MAX_USER_LINE_LENGTH
#define SOPC_LOG_MAX_USER_LINE_LENGTH 512
#endif /* SOPC_LOG_MAX_USER_LINE_LENGTH */

/** @brief Indicates whether the host has a file system */
#ifndef SOPC_HAS_FILESYSTEM
#define SOPC_HAS_FILESYSTEM true
#endif /* SOPC_HAS_FILESYSTEM */

/** @brief define host-specific console print function
 * If no console is provided or log wants to be omitted, the following can be used:
 * \code{.c}
 * #define SOPC_CONSOLE_PRINTF(...) do{} while (0)
 * \endcode
 **/
#ifndef SOPC_CONSOLE_PRINTF
#define SOPC_CONSOLE_PRINTF printf
#endif /* SOPC_CONSOLE_PRINTF */

/* Check use of uintptr_t is not an issue on the current platform */
#if UINTPTR_MAX < UINT32_MAX
#error "UINTPTR_MAX < UINT32_MAX whereas uintptr_t are used to store uint32_t values"
#endif

/* Check uintptr_t has a maximum value compatible with pointer size. Since it is an insufficient check
 * to ensure same size, check at runtime on actual size is also done by SOPC_Internal_Common_Constants_RuntimeCheck */
#if SOPC_POINTER_SIZE == 4
#if UINTPTR_MAX != UINT32_MAX
#error "UINTPTR_MAX != UINT32_MAX whereas it is expected to have pointer size for other language binding"
#endif
#elif SOPC_POINTER_SIZE == 8
#if UINTPTR_MAX != UINT64_MAX
#error "UINTPTR_MAX != UINT64_MAX whereas it is expected to have pointer size for other language binding"
#else
#error "Unsupported SOPC_POINTER_SIZE size"
#endif
#endif

/* Check casts from uint32_t / int32_t to size_t are valid without verification */
#if SIZE_MAX < UINT32_MAX
#error "SIZE_MAX < UINT32_MAX whereas uint32_t are casted to size_t values"
#endif

/* Check casts from long to size_t are valid without verification */
#if SIZE_MAX < LONG_MAX
#error "SIZE_MAX < LONG_MAX whereas long are casted to size_t values"
#endif

/* Check that the message buffer is large enough to hold the minimal TCP UA chunk */
#if SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE < SOPC_TCP_UA_MIN_BUFFER_SIZE
#error "SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE is not large enough, must be >= SOPC_TCP_UA_MIN_BUFFER_SIZE"
#endif

/* Check that the message buffer is large enough to hold the minimal TCP UA chunk */
#if 0 != SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH && \
    (SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH < SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE)
#error "SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH is not large enough, must be >= SOPC_TCP_UA_MAX_BUFFER_SIZE"
#endif

/* Check that both number of chunks and message length are not defined to 0
 * Note: it is required to have a maximum size of message body buffer defined
 */
#if 0 == SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH && 0 == SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS
#error \
    "It is forbidden to define both SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH and SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS to value 0 (no limit)"
#endif

/* Check that the message buffer is large enough to hold the minimal TCP UA chunk */
#if 0 != SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH && \
    (SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH < SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE)
#error "SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH is not large enough, must be >= SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE"
#endif

/* Check that both number of chunks and message length are not defined to 0
 * Note: it is required to have a maximum size of message body buffer defined
 */
#if 0 == SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH && 0 == SOPC_DEFAULT_SEND_MAX_NB_CHUNKS
#error \
    "It is forbidden to define both SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH and SOPC_DEFAULT_SEND_MAX_NB_CHUNKS to value 0 (no limit)"
#endif

/* \brief Internal use only */
const SOPC_Common_EncodingConstants* SOPC_Internal_Common_GetEncodingConstants(void);

/* \brief Internal use to check properties at runtime */
bool SOPC_Internal_Common_Constants_RuntimeCheck(void);

#endif /* SOPC_COMMON_CONSTANTS_H_ */
