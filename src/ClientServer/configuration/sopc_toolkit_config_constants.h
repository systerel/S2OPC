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

#include "sopc_common_constants.h"

/* See sopc_common_constants.h for common part constants */

/** @brief Maximum number of operations in a request accepted by server (Read, Write, etc.) */
#ifndef SOPC_MAX_OPERATIONS_PER_MSG
#define SOPC_MAX_OPERATIONS_PER_MSG 5000
#endif /* SOPC_MAX_OPERATIONS_PER_MSG */

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

/** @brief SOPC_MAX_SECURE_CONNECTIONS defines the simultaneous stable SC,
 *         More slots are used (+0.25%) to check if old connection can be closed when maximum is reached.
 *         If an old connection is closed for each of the buffered new connection attempt,
 *         those are kept otherwise buffered new connection are closed
 */
#define SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED (5 * SOPC_MAX_SECURE_CONNECTIONS / 4)

/** @brief Minimum value for OPN requestedLifetime parameter */
#ifndef SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME
#define SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME 1000
#endif

/** @brief Maximum number of requests sent by client pending */
#ifndef SOPC_MAX_PENDING_REQUESTS
#define SOPC_MAX_PENDING_REQUESTS 128
#endif

/** @brief Maximum time before a response shall be received after sending a request (0 means no limit).
 *  Note: the request timeoutHint parameter is set to SOPC_REQUEST_TIMEOUT_MS/2 to let a margin regarding timeoutHint.
 */
#ifndef SOPC_REQUEST_TIMEOUT_MS
#define SOPC_REQUEST_TIMEOUT_MS 10000
#endif

/** @brief Elapsed time after which SC connection establishment is considered timeout
 * (elapsed time between socket connection and OPN response message)
 * */
#ifndef SOPC_SC_CONNECTION_TIMEOUT_MS
#define SOPC_SC_CONNECTION_TIMEOUT_MS 10000
#endif

/** @brief Maximum number of configured reverse connection from a server endpoint to clients */
#ifndef SOPC_MAX_REVERSE_CLIENT_CONNECTIONS
#define SOPC_MAX_REVERSE_CLIENT_CONNECTIONS 5
#endif

/** @brief Delay before retrying to establish a reverse connection when socket connection failed */
#ifndef SOPC_REVERSE_CONNECTION_RETRY_DELAY_MS
#define SOPC_REVERSE_CONNECTION_RETRY_DELAY_MS 1000
#endif

/* SESSION CONFIGURATION */

/** @brief Maximum number of sessions (and subscriptions: 1 per session) established */
#ifndef SOPC_MAX_SESSIONS
#define SOPC_MAX_SESSIONS 20
#endif /* SOPC_MAX_SESSIONS */

/** @brief Client requested timeout for which a session shall remain open without activity */
#ifndef SOPC_REQUESTED_SESSION_TIMEOUT
#define SOPC_REQUESTED_SESSION_TIMEOUT 60000 // 60 seconds
#endif

/** @brief Minimum session timeout accepted by server */
#ifndef SOPC_MIN_SESSION_TIMEOUT
#define SOPC_MIN_SESSION_TIMEOUT 10000 // 10 seconds
#endif

/** @brief Maximum session timeout accepted by server */
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

/** @brief By default resolution of HasSubtype references in address space used by services is static and
 *         based on base namespace 0 (OPC UA) content. The namespace 0 content extraction is provided by
 *         sopc_embedded_nodeset2.h header. It is particularly suitable for embedded devices since it avoids
 *         a recursive resolution in address space.
 *         If variable is set to true the references are also searched into instantiated address space
 *         when not available in static extraction content, the search is recursive but
 *         it still limited by SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL levels of recursion.*/
#ifndef SOPC_HAS_SUBTYPE_HYBRID_RESOLUTION
#define SOPC_HAS_SUBTYPE_HYBRID_RESOLUTION false
#endif

#include "sopc_config_constants_check.h"

#endif /* SOPC_TOOLKIT_CONFIG_CONSTANTS_H_ */
