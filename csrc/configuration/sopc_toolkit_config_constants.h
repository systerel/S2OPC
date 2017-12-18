/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

/** @brief Maximum Message Length used (must be > SOPC_TCP_UA_MIN_BUFFER_SIZE) */
#ifndef SOPC_MAX_MESSAGE_LENGTH
#define SOPC_MAX_MESSAGE_LENGTH UINT16_MAX
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/** @brief Maximum ByteString/String/XmlElement length in bytes used */
#ifndef SOPC_MAX_STRING_LENGTH
#define SOPC_MAX_STRING_LENGTH 16777216
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/** @brief Maximum array length that could be stored in a variant */
#ifndef SOPC_MAX_ARRAY_LENGTH
#define SOPC_MAX_ARRAY_LENGTH 1000000
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/** @brief Maximum levels of nested diagnostic information structure
 *  Note: OPC UA specification v1.03 part 6 §5.2.2.12 indicates
 *  "Decoders shall support at least 100 nesting levels ..."*/
#ifndef SOPC_MAX_DIAG_INFO_NESTED_LEVEL
#define SOPC_MAX_DIAG_INFO_NESTED_LEVEL 100
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/** @brief Maximum levels of nested variant
 * (e.g.: variant containing array of variants / data value) */
#ifndef SOPC_MAX_VARIANT_NESTED_LEVEL
#define SOPC_MAX_VARIANT_NESTED_LEVEL 10
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/* TCP SOCKETS CONFIGURATION */

/** @brief Maximum number of TCP sockets (listeners and connections) */
#ifndef SOPC_MAX_SOCKETS
#define SOPC_MAX_SOCKETS 150
#endif /* SOPC_MAX_SOCKETS */

/** @brief Maximum number of TCP sockets connections on a socket listener */
#ifndef SOPC_MAX_SOCKETS_CONNECTIONS
#define SOPC_MAX_SOCKETS_CONNECTIONS 50
#endif /* SOPC_MAX_SOCKETS_CONNECTIONS */

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
#define SOPC_MAX_PENDING_REQUESTS UINT16_MAX
#endif /* SOPC_MAX_PENDING_REQUESTS */

/* SESSION CONFIGURATION */

/** @brief Maximum number of sessions established */
#ifndef SOPC_MAX_SESSIONS
#define SOPC_MAX_SESSIONS 20
#endif /* SOPC_MAX_SESSIONS */

/* Maximum number of milliseconds that a session shall remain open without activity */
#ifndef SOPC_SESSION_TIMEOUT
#define SOPC_SESSION_TIMEOUT 10000
#endif

/* DEBUG CONFIGURATION */

#ifndef SOPC_DEBUG_PRINTING
#define SOPC_DEBUG_PRINTING false
#endif

#endif /* SOPC_TOOLKIT_CONFIG_CONSTANTS_H_ */
