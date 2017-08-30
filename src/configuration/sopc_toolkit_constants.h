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

/* COMMON CONFIGURATION */

/** @brief Version of the toolkit */
#define SOPC_TOOLKIT_VERSION_MAJOR 0
#define SOPC_TOOLKIT_VERSION_MEDIUM 2
#define SOPC_TOOLKIT_VERSION_MINOR 0

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

/** @brief Maximum number of endpoint description configured */
#ifndef SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS
# define SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS  10
#endif /* SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS */

#ifndef SOPC_LISTENER_LISTEN_ALL_INTERFACES
# define SOPC_LISTENER_LISTEN_ALL_INTERFACES  true
#endif /* SOPC_LISTENER_LISTEN_ALL_INTERFACES */


/** @brief Maximum number of secure channel connections established (should be < SOPC_MAX_SOCKETS) */
#ifndef SOPC_MAX_SECURE_CONNECTIONS
# define SOPC_MAX_SECURE_CONNECTIONS  20
#endif /* SOPC_MAX_SECURE_CONNECTIONS */

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


#endif /* SOPC_TOOLKIT_CONSTANTS_H_ */
