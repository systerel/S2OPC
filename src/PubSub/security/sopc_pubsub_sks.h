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

/**
 * \file
 * \brief PubSub Security Keys Service configuration: define the ::SOPC_SKManager to use to retrieve the keys.
 *
 * To define a security keys service, ::SOPC_PubSubSKS_Init and ::SOPC_PubSubSKS_SetSkManager shall be called.
 *
 * The Publisher and Subscriber schedulers will then automatically call ::SOPC_PubSubSKS_GetSecurityKeys.
 */

#ifndef SOPC_PUBSUB_SKS_H_
#define SOPC_PUBSUB_SKS_H_

#include "sopc_secret_buffer.h"
#include "sopc_sk_secu_group_managers.h"

#ifndef SOPC_PUBSUB_SKS_DEFAULT_TOKENID
#define SOPC_PUBSUB_SKS_DEFAULT_TOKENID 1
#endif

// To requested current token in getSecurityKey
#define SOPC_PUBSUB_SKS_CURRENT_TOKENID SOPC_SK_MANAGER_CURRENT_TOKEN_ID

typedef struct SOPC_PubSubSKS_Keys
{
    // The ID of the security token that identifies the security key in a SecurityGroup.
    // not managed. Shall be one
    uint32_t tokenId;

    SOPC_SecretBuffer* signingKey;
    SOPC_SecretBuffer* encryptKey;
    SOPC_SecretBuffer* keyNonce;
} SOPC_PubSubSKS_Keys;

/**
 * \brief Return security key from a security group id.
 *        This function is automatically called by Publisher and Subscriber schedulers.
 *
 * \param securityGroupId a Group Id with security
 * \param tokenId token id of the requested keys. Current token is requested with ::SOPC_PUBSUB_SKS_CURRENT_TOKENID
 * \return tokenid and group keys
 *
 */
SOPC_PubSubSKS_Keys* SOPC_PubSubSKS_GetSecurityKeys(const char* securityGroupid, uint32_t tokenId);

/**
 * \brief Clear a SOPC_PubSubSKS_Keys
 * the given parameter can be freed after the function returns.
 *
 * \param keys object to clear
 *
 */
void SOPC_PubSubSKS_Keys_Delete(SOPC_PubSubSKS_Keys* keys);

#endif /* SOPC_PUBSUB_SKS_H_ */
