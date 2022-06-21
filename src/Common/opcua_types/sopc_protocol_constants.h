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
 * \brief OPC-UA specific constants.
 *
 */

#ifndef SOPC_PROTOCOL_CONSTANTS_H_
#define SOPC_PROTOCOL_CONSTANTS_H_

/* OPC UA SPECIFICATION CONFIGURATION */

/** @brief Version of the used protocol */
#define SOPC_PROTOCOL_VERSION 0

#define SOPC_TCP_UA_MIN_BUFFER_SIZE \
    8192 /* now defined only for OPC UA Secure Conversation (minimum chunk size): see mantis #3447 */

#define SOPC_TCP_UA_MAX_URL_AND_REASON_LENGTH 4096 /* see Part 6 1.03 Table 31, 35 and 37 */

/* Length of a TCP UA message Header */
#define SOPC_TCP_UA_HEADER_LENGTH 8
/* Length of a TCP UA ACK message */
#define SOPC_TCP_UA_ACK_MSG_LENGTH 28
/* Minimum length of a TCP UA HELLO message (without including URL string content but only its size)*/
#define SOPC_TCP_UA_HEL_MIN_MSG_LENGTH 32
/* Minimum length of a TCP UA ERROR message */
#define SOPC_TCP_UA_ERR_MIN_MSG_LENGTH 16
/* Minimum length of a TCP UA REVERSE HELLO message (without including URI/URL strings content but only its size)*/
#define SOPC_TCP_UA_RHE_MIN_MSG_LENGTH 16

/* Position of MessageSize header field in a UA message chunk*/
#define SOPC_UA_HEADER_MESSAGE_SIZE_POSITION 4
/* Position of IsFinal header field in a UA message chunk*/
#define SOPC_UA_HEADER_ISFINAL_POSITION 3

/* Length of an UA secure message chunk header */
#define SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH 12
/* Length of an UA symmetric security header chunk header */
#define SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH 4
/* Length of an UA secure message chunk sequence header */
#define SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH 8

/* Length of 3 previous header fields */
#define SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH                                \
    (SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH + SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH + \
     SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH)

/* Position of sequence header in a symmetric secure message */
#define SOPC_UA_SYMMETRIC_SEQUENCE_HEADER_POSITION \
    (SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH + SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH)

#endif // SOPC_PROTOCOL_CONSTANTS_H_
