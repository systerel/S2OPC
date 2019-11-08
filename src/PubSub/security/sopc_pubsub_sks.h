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

#ifndef SOPC_PUBSUB_SKS_H_
#define SOPC_PUBSUB_SKS_H_

#include "sopc_secret_buffer.h"
#include "sopc_sk_manager.h"

#define SOPC_PUBSUB_SKS_DEFAULT_GROUPID 1
#define SOPC_PUBSUB_SKS_DEFAULT_TOKENID 1
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
 * \brief Initialise the PubSubSKS
 */
void SOPC_PubSubSKS_Init(void);

/**
 * \brief Set the Security Keys Manager
 * Only one SK Manager is used. Set to NULL to stop the service.
 *
 * \param skm a Security Keys Manager to use to get keys.
 *
 */
void SOPC_PubSubSKS_SetSkManager(SOPC_SKManager* skm);

/**
 * \brief Return security key from a security group id.
 *
 * Only SOPC_PUBSUB_SKS_DEFAULT_GROUPID is accepted in this version
 *
 * \param groupid a Security Group Id
 * \param tokenId token id of the requested keys. Current token is requested with SOPC_PUBSUB_SKS_CURRENT_TOKENID
 * \return tokenid and group keys
 *
 */
SOPC_PubSubSKS_Keys* SOPC_PubSubSKS_GetSecurityKeys(uint32_t groupid, uint32_t tokenId);

/**
 * \brief Clear a SOPC_PubSubSKS_Keys
 * the given parameter can be freed after the function returns.
 *
 * \param keys object to clear
 *
 */
void SOPC_PubSubSKS_Keys_Delete(SOPC_PubSubSKS_Keys* keys);

#endif /* SOPC_PUBSUB_SKS_H_ */
