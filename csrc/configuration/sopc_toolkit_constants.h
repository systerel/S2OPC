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

#ifndef SOPC_TOOLKIT_CONSTANTS_H_
#define SOPC_TOOLKIT_CONSTANTS_H_

#include <stdbool.h>
#include <stdint.h>

/* Check IEEE-754 compliance */
#include "sopc_ieee_check.h"

/* COMMON CONFIGURATION */

/** @brief Version of the toolkit */
#define SOPC_TOOLKIT_VERSION_MAJOR 0
#define SOPC_TOOLKIT_VERSION_MEDIUM 2
#define SOPC_TOOLKIT_VERSION_MINOR 1

/** @brief Maximum Message Length used (must be > SOPC_TCP_UA_MIN_BUFFER_SIZE) */
#ifndef SOPC_MAX_MESSAGE_LENGTH
# define SOPC_MAX_MESSAGE_LENGTH  UINT16_MAX
#endif /* SOPC_MAX_MESSAGE_LENGTH */

/* TCP SOCKETS CONFIGURATION */

/** @brief Maximum number of TCP sockets (listeners and connections) */
#ifndef SOPC_MAX_SOCKETS
# define SOPC_MAX_SOCKETS  150
#endif /* SOPC_MAX_SOCKETS */

/** @brief Maximum number of TCP sockets connections on a socket listener */
#ifndef SOPC_MAX_SOCKETS_CONNECTIONS
# define SOPC_MAX_SOCKETS_CONNECTIONS  50
#endif /* SOPC_MAX_SOCKETS_CONNECTIONS */

/* SECURE CHANNEL CONFIGURATION */

/** @brief Maximum number of endpoint description configured (same as number of connection listeners) */
#ifndef SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS
# define SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS  10
#endif /* SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS */

/* Maximum value accepted in B model */
#if SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS > INT32_MAX
#error "Max number of endpoint descriptions cannot be more than INT32_MAX"
#endif

#ifndef SOPC_LISTENER_LISTEN_ALL_INTERFACES
# define SOPC_LISTENER_LISTEN_ALL_INTERFACES  true
#endif /* SOPC_LISTENER_LISTEN_ALL_INTERFACES */


/** @brief Maximum number of secure channel connections (and configurations) established */
#ifndef SOPC_MAX_SECURE_CONNECTIONS
# define SOPC_MAX_SECURE_CONNECTIONS  20
#endif /* SOPC_MAX_SECURE_CONNECTIONS */

#if SOPC_MAX_SECURE_CONNECTIONS > SOPC_MAX_SOCKETS
#error "Max number of secure connections cannot be greater than max number of sockets"
#endif

/* Maximum value accepted in B model */
#if SOPC_MAX_SECURE_CONNECTIONS > INT32_MAX
#error "Max number of secure connections cannot be more than INT32_MAX"
#endif

/** @brief Maximum number of sessions established */
#ifndef SOPC_MAX_SESSIONS
# define SOPC_MAX_SESSIONS  20
#endif /* SOPC_MAX_SESSIONS */

/* Maximum value accepted in B model */
#if SOPC_MAX_SESSIONS > INT32_MAX
#error "Max number of sessions cannot be more than INT32_MAX"
#endif

/** @brief Maximum number of requests sent by client pending */
#ifndef SOPC_MAX_PENDING_REQUESTS
# define SOPC_MAX_PENDING_REQUESTS  UINT16_MAX
#endif /* SOPC_MAX_PENDING_REQUESTS */

/* OPC UA SPECIFICATION CONFIGURATION */

/** @brief Version of the used protocol */
# define SOPC_PROTOCOL_VERSION  0

#define SOPC_TCP_UA_MIN_BUFFER_SIZE 8192 // now defined only for OPC UA Secure Conversation (minimum chunk size): see mantis #3447

#define SOPC_TCP_UA_MAX_URL_LENGTH 4096 // see Part 6 Table 35

/* Length of a TCP UA message Header */
#define SOPC_TCP_UA_HEADER_LENGTH 8
/* Length of a TCP UA ACK message */
#define SOPC_TCP_UA_ACK_MSG_LENGTH 28
/* Minimum length of a TCP UA HELLO message (without including URL string content but only its size)*/
#define SOPC_TCP_UA_HEL_MIN_MSG_LENGTH 32
/* Minimum length of a TCP UA ERROR message */
#define SOPC_TCP_UA_ERR_MIN_MSG_LENGTH 16

/* Position of MessageSize header field in a UA message chunk*/
#define SOPC_UA_HEADER_LENGTH_POSITION 4
/* Position of IsFinal header field in a UA message chunk*/
#define SOPC_UA_HEADER_ISFINAL_POSITION 3

/* Length of an UA secure message chunk header */
#define SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH 12
/* Length of an UA symmetric security header chunk header */
#define SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH 4
/* Length of an UA secure message chunk sequence header */
#define SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH 8

/* Maximum number of milliseconds that a session shall remain open without activity */
#define SOPC_SESSION_TIMEOUT 10000

/* Minimum value for OPN requestedLifetime parameter */
#define SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME 10000

/* DEBUG CONFIGURATION */
#define SOPC_DEBUG_PRINTING false

/* OPC UA statuses */
// TODO: remove status code from non OPC UA services and define type in builtintypes module
typedef uint32_t SOPC_StatusCode;
#define STATUS_OK 0x0 // TODO: change values
#define STATUS_OK_INCOMPLETE 0x00000001
#define STATUS_NOK 0x80000000//0x10000000
#define STATUS_INVALID_PARAMETERS 0x80760001//0x20000000
#define STATUS_INVALID_STATE 0x80760002//0x30000000
#define STATUS_INVALID_RCV_PARAMETER 0x80000003//0x40000000

#define SOPC_USE_GCC true

#ifdef __GNUC__
    #ifndef __clang__
        #define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST _Pragma("GCC diagnostic ignored \"-Wcast-qual\"");
        #define SOPC_GCC_DIAGNOSTIC_RESTORE _Pragma("GCC diagnostic pop")
    #else
        #define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        #define SOPC_GCC_DIAGNOSTIC_RESTORE
    #endif
#else
    #define SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    #define SOPC_GCC_DIAGNOSTIC_RESTORE
#endif

#endif /* SOPC_TOOLKIT_CONSTANTS_H_ */
