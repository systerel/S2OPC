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
 * \brief Contains the configuration constants used by the Tookit. Those constants could be modified for specific use.
 *
 */

#ifndef SOPC_TOOLKIT_CONFIG_CONSTANTS_H_
#define SOPC_TOOLKIT_CONFIG_CONSTANTS_H_

#include <stdbool.h>

/* COMMON CONFIGURATION */

/** @brief Maximum chunk buffer size used (must be >= SOPC_TCP_UA_MIN_BUFFER_SIZE) */
#ifndef SOPC_TCP_UA_MAX_BUFFER_SIZE
#define SOPC_TCP_UA_MAX_BUFFER_SIZE 8192
#endif /* SOPC_TCP_UA_MAX_BUFFER_SIZE */

/** @brief Maximum number of chunks accepted for 1 message, 0 means no limit.
 *  Note: if 0 is chosen SOPC_MAX_MESSAGE_LENGTH definition shall be changed not to use it and shall not be 0.
 */
#ifndef SOPC_MAX_NB_CHUNKS
#define SOPC_MAX_NB_CHUNKS 1
#endif /* SOPC_MAX_NB_CHUNKS */

/** @brief Maximum message length used (must be >= SOPC_TCP_UA_MAX_BUFFER_SIZE), 0 means no limit.
 *  Note: if 0 is chosen SOPC_MAX_NB_CHUNK shall not be 0.
 * */
#ifndef SOPC_MAX_MESSAGE_LENGTH
#define SOPC_MAX_MESSAGE_LENGTH SOPC_TCP_UA_MAX_BUFFER_SIZE* SOPC_MAX_NB_CHUNKS
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/** @brief Maximum ByteString/String/XmlElement length in bytes used */
#ifndef SOPC_MAX_STRING_LENGTH
#define SOPC_MAX_STRING_LENGTH 8192
#endif /* SOPC_MAX_MESSAGE_LENGTH */

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

/* @brief Maximum number of operations in a request accepted by server (Read, Write, etc.) */
#ifndef SOPC_MAX_OPERATIONS_PER_MSG
#define SOPC_MAX_OPERATIONS_PER_MSG 5000
#endif /* SOPC_MAX_OPERATIONS_PER_MSG */

/* @brief Maximum number of elements in Async Queue */
#ifndef SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE
#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE 5000
#endif /* SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE */

#ifndef SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY
#define SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY true
#endif /* SOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE_WARNING_ONLY */

/* TCP SOCKETS CONFIGURATION */

/** @brief Maximum number of TCP sockets (listeners and connections) */
#ifndef SOPC_MAX_SOCKETS
#define SOPC_MAX_SOCKETS 150
#endif /* SOPC_MAX_SOCKETS */

/** @brief Maximum number of TCP sockets connections on a socket listener */
#ifndef SOPC_MAX_SOCKETS_CONNECTIONS
#define SOPC_MAX_SOCKETS_CONNECTIONS 50
#endif /* SOPC_MAX_SOCKETS_CONNECTIONS */

/** @brief Minimum byte buffer size allocated to read data from socket and send it to chunk manager */
#ifndef SOPC_MIN_BYTE_BUFFER_SIZE_READ_SOCKET
#define SOPC_MIN_BYTE_BUFFER_SIZE_READ_SOCKET 1024
#endif /* SOPC_MIN_BYTE_BUFFER_SIZE_READ_SOCKET */

/* SECURE CHANNEL CONFIGURATION */

/** @brief Maximum number of endpoint description configured (same as number of connection listeners) */
#ifndef SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS
#define SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS 10
#endif /* SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS */

#ifndef SOPC_LISTENER_LISTEN_ALL_INTERFACES
#define SOPC_LISTENER_LISTEN_ALL_INTERFACES true
#endif /* SOPC_LISTENER_LISTEN_ALL_INTERFACES */

/** @brief Maximum number of secure channel connections (and configurations) established */
#ifndef SOPC_MAX_SECURE_CONNECTIONS
#define SOPC_MAX_SECURE_CONNECTIONS 20
#endif /* SOPC_MAX_SECURE_CONNECTIONS */

/* Minimum value for OPN requestedLifetime parameter */
#ifndef SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME
#define SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME 1000
#endif

/** @brief Maximum number of requests sent by client pending */
#ifndef SOPC_MAX_PENDING_REQUESTS
#define SOPC_MAX_PENDING_REQUESTS 128
#endif

/** @brief Maximum time before a response shall be received after sending a request */
#ifndef SOPC_REQUEST_TIMEOUT_MS
#define SOPC_REQUEST_TIMEOUT_MS 5000
#endif

/** @brief Elapsed time after which SC connection establishment is considered timeout
 * (elapsed time between socket connection and OPN response message)
 * */
#ifndef SOPC_SC_CONNECTION_TIMEOUT_MS
#define SOPC_SC_CONNECTION_TIMEOUT_MS 60000
#endif

/* SESSION CONFIGURATION */

/** @brief Maximum number of sessions (and subscriptions: 1 per session) established */
#ifndef SOPC_MAX_SESSIONS
#define SOPC_MAX_SESSIONS 20
#endif /* SOPC_MAX_SESSIONS */

/* @brief Client requested timeout for which a session shall remain open without activity */
#ifndef SOPC_REQUESTED_SESSION_TIMEOUT
#define SOPC_REQUESTED_SESSION_TIMEOUT 60000 // 60 seconds
#endif

/* @brief Minimum session timeout accepted by server */
#ifndef SOPC_MIN_SESSION_TIMEOUT
#define SOPC_MIN_SESSION_TIMEOUT 60000 // 10 seconds
#endif

/* @brief Maximum session timeout accepted by server */
#ifndef SOPC_MAX_SESSION_TIMEOUT
#define SOPC_MAX_SESSION_TIMEOUT 600000 // 10 minutes
#endif

/* SUBSCRIPTION CONFIGURATION */

/** @brief Maximum publish requests stored by server for a subscription */
#ifndef SOPC_MAX_SUBSCRIPTION_PUBLISH_REQUESTS
#define SOPC_MAX_SUBSCRIPTION_PUBLISH_REQUESTS 10
#endif

/** @brief Minimum publish interval of a subscription in milliseconds */
#ifndef SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION
#define SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION 100 // 100 ms
#endif

/** @brief Maximum publish interval of a subscription in milliseconds */
#ifndef SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION
#define SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION 3600000 // 1 hour
#endif

/** @brief Minimum number of publish intervals before a keep alive is sent (server to client) */
#ifndef SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS
#define SOPC_MIN_KEEPALIVE_PUBLISH_INTERVALS 1
#endif

/** @brief Maximum number of publish intervals before a keep alive is sent (server to client) */
#ifndef SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS
#define SOPC_MAX_KEEPALIVE_PUBLISH_INTERVALS 100
#endif

/** @brief Minimum number of publish intervals before lifetime expired (client to server communication) */
#ifndef SOPC_MIN_LIFETIME_PUBLISH_INTERVALS
#define SOPC_MIN_LIFETIME_PUBLISH_INTERVALS 3 // >= 3 KeepAlive
#endif

/** @brief Maximum number of publish intervals before lifetime expired (client to server communication) */
#ifndef SOPC_MAX_LIFETIME_PUBLISH_INTERVALS
#define SOPC_MAX_LIFETIME_PUBLISH_INTERVALS 300 // >= 3 KeepAlive
#endif

/* TRANSLATE BROWSE PATH MANAGEMENT */

/** @brief Maximum number of matches to return for a given relative path
 * Note: 3 arrays of this size are allocated in B model.
 */
#ifndef SOPC_MAX_TRANSLATE_BROWSE_PATH_MATCHES
#define SOPC_MAX_TRANSLATE_BROWSE_PATH_MATCHES 10
#endif

/* ADDRESS SPACE MANAGEMENT */

/** @brief By default resolution of HasSubtype references used in services uses only the static and limited extraction
 * provided by sopc_embedded_nodeset2.h header. If variable is set to true the references are also searched into
 * instantiated address space when not available in static extraction.*/
#ifndef SOPC_HAS_SUBTYPE_HYBRID_RESOLUTION
#define SOPC_HAS_SUBTYPE_HYBRID_RESOLUTION false
#endif

#include "sopc_config_constants_check.h"

#endif /* SOPC_TOOLKIT_CONFIG_CONSTANTS_H_ */
