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

#ifndef SOPC_PUBSUB_CONSTANTS_H_
#define SOPC_PUBSUB_CONSTANTS_H_

#include "sopc_crypto_profiles.h"

// Size of buffer (max size of message)
#ifndef SOPC_PUBSUB_BUFFER_SIZE
#define SOPC_PUBSUB_BUFFER_SIZE 4096
#endif

// Size of array. Use for subscriber context
#ifndef SOPC_PUBSUB_MAX_PUBLISHER_PER_SCHEDULER
#define SOPC_PUBSUB_MAX_PUBLISHER_PER_SCHEDULER 10
#endif
// Size of array. Use for subscriber context
#ifndef SOPC_PUBSUB_MAX_MESSAGE_PER_PUBLISHER
#define SOPC_PUBSUB_MAX_MESSAGE_PER_PUBLISHER 10
#endif

#define SOPC_MAX_LENGTH_UINT64_TO_STRING                                                                              \
    21 /* 2^64 = 1.8447*10^19 maximum number you could represent that use maximum chars would be 1.8447*10^19 plus \0 \
          at the end */
#define SOPC_MAX_LENGTH_UINT32_TO_STRING                                                                              \
    11 /* 2^32 = 4294967296 maximum number you could represent plus \0 at the end */
#define SOPC_MAX_LENGTH_UINT16_TO_STRING \
    6 /* 2^16 = 65536 maximum number you could represent using maximum chars would be 65536 plus \0 at the end */
#define SOPC_MAX_LENGTH_UINT8_TO_STRING                                                                              \
    4 /* 2^8 = 256 maximum number you could represent plus \0 at the end */

#define SOPC_MAX_LENGTH_INT64_TO_STRING                                                                              \
    21 /* 2^63 = 9.22*10^18 maximum number you could represent that use maximum chars would be 1.8447*10^19 plus \0 \
          at the end plus '-'*/
#define SOPC_MAX_LENGTH_INT32_TO_STRING                                                                              \
    12 /* 2^31 = 2147483648 maximum number you could represent plus \0 at the end plus '-'*/
#define SOPC_MAX_LENGTH_INT16_TO_STRING \
    7 /* 2^15 = 32768 maximum number you could represent using maximum chars would be 65536 plus \0 at the end plus '-' */
#define SOPC_MAX_LENGTH_INT8_TO_STRING                                                                              \
    5 /* 2^7 = 128 maximum number you could represent plus \0 at the end plus '-'*/

#define SOPC_PRECISION_PRINTING_FLOAT_NUMBERS 10


// Pub Sub Security Policy
#define SOPC_PUBSUB_SECURITY_POLICY SOPC_SecurityPolicy_PubSub_Aes256_URI

#endif /* SOPC_PUBSUB_CONSTANTS_H_ */
