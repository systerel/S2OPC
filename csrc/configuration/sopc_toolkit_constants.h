/*
 *  Copyright (C) 2018 Systerel and others.
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
 * \brief Contains the constants used by the Tookit. Those constants are not intended to be modified.
 *
 */

#ifndef SOPC_TOOLKIT_CONSTANTS_H_
#define SOPC_TOOLKIT_CONSTANTS_H_

#include "sopc_toolkit_config_constants.h"

/* Check configured constants values */
#include "sopc_config_constants_check.h"

/** @brief Version of the toolkit */
#define SOPC_TOOLKIT_VERSION_MAJOR 0
#define SOPC_TOOLKIT_VERSION_MEDIUM 3
#define SOPC_TOOLKIT_VERSION_MINOR 1

/* OPC UA SPECIFICATION CONFIGURATION */

/** @brief Version of the used protocol */
#define SOPC_PROTOCOL_VERSION 0

#define SOPC_TCP_UA_MIN_BUFFER_SIZE \
    8192 // now defined only for OPC UA Secure Conversation (minimum chunk size): see mantis #3447

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

/* SOPC return statuses */
typedef enum SOPC_ReturnStatus {
    SOPC_STATUS_OK = 0,
    SOPC_STATUS_NOK,
    SOPC_STATUS_INVALID_PARAMETERS,
    SOPC_STATUS_INVALID_STATE,
    SOPC_STATUS_ENCODING_ERROR,
    SOPC_STATUS_WOULD_BLOCK,
    SOPC_STATUS_TIMEOUT,
    SOPC_STATUS_OUT_OF_MEMORY,
    SOPC_STATUS_CLOSED,
    SOPC_STATUS_NOT_SUPPORTED
} SOPC_ReturnStatus;

/**
 * \brief the toolkit provide and use monotonic clock for time references (used for timers)
 * Note: it is possible to set the clock as non monotonic defining variable on configuration. Otherwise default value is
 * true.
 */
#ifndef SOPC_MONOTONIC_CLOCK
#define SOPC_MONOTONIC_CLOCK true
#endif

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
