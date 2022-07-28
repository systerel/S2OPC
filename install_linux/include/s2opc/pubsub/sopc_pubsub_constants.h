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

// Pub Sub Security Policy
#define SOPC_PUBSUB_SECURITY_POLICY SOPC_SecurityPolicy_PubSub_Aes256_URI

#endif /* SOPC_PUBSUB_CONSTANTS_H_ */
