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
 * \brief PubSub Security Keys Service configuration: define the ::SOPC_SKManager(s) to use to retrieve the keys.
 *
 * To define a security keys manager(s) ::SOPC_PubSubSKS_Init and ::SOPC_PubSubSKS_AddSkManager shall be called.
 * Each security group in configuration shall have its own ::SOPC_SKManager.
 *
 * The Publisher and Subscriber schedulers will then automatically call ::SOPC_PubSubSKS_GetSecurityKeys.
 */

#ifndef SOPC_PUBSUB_SKS_H_
#define SOPC_PUBSUB_SKS_H_

#include "sopc_pubsub_conf.h"
#include "sopc_secret_buffer.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_scheduler.h"
#include "sopc_sk_secu_group_managers.h"

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
 * \brief Initialise the PubSubSKS module
 */
void SOPC_PubSubSKS_Init(void);

/**
 * \brief Clear the PubSubSKS and SKManager(s).
 *        It shall be called after clearing the ::SOPC_SKScheduler managing the managers if used.
 *
 * \warning When a ::SOPC_SKScheduler is used, the scheduler shall be cleared using ::SOPC_SKScheduler_StopAndClear
 * before calling this function.
 */
void SOPC_PubSubSKS_Clear(void);

/**
 * \brief Creates and adds the SK Managers for each security group from the PubSub configuration (readers and writers).
 *
 * \param pubSubConfig the PubSub configuration to find the security group ids from.
 *
 * \return true if the SK Managers are created successfully, false otherwise.
 */
bool SOPC_PubSubSKS_CreateManagersFromConfig(const SOPC_PubSubConfiguration* pubSubConfig);

/**
 * \brief Add the Security Keys Manager to use to retrieve the keys for UADP secure exchanges
 *        with the associated security group id defined in the manager.
 *
 *        This function shall be called once for each security group id that is used in the PubSub configuration.
 *        If the security group id is already used, the previous SK Manager is discarded and cleared. The security group
 *        id is defined in the manager and shall not be NULL.
 *
 * \note When using ::SOPC_PubSubSKS_CreateManagersFromConfig, this function should not be needed.
 *
 * \param skm the Security Keys Manager to use to get keys.
 *
 * \return true if the SK Manager is added successfully, false otherwise.
 */
bool SOPC_PubSubSKS_AddSkManager(SOPC_SKManager* skm);

/**
 * \brief Return security key from a security group id.
 *        This function is automatically called by Publisher and Subscriber schedulers.
 *
 * \param securityGroupId a Group Id with security
 * \param tokenId token id of the requested keys. Current token is requested with ::SOPC_PUBSUB_SKS_CURRENT_TOKENID
 * \return tokenid and group keys
 *
 */
SOPC_PubSubSKS_Keys* SOPC_PubSubSKS_GetSecurityKeys(const char* securityGroupId, uint32_t tokenId);

/**
 * \brief Clear a SOPC_PubSubSKS_Keys
 * the given parameter can be freed after the function returns.
 *
 * \param keys object to clear
 *
 */
void SOPC_PubSubSKS_Keys_Delete(SOPC_PubSubSKS_Keys* keys);

/**
 * \brief Add a task to the SK scheduler for each security group SK manager add with ::SOPC_PubSubSKS_AddSkManager.
 *        For each task, the scheduler will trigger the builder
 *        to update the keys from provider to manager.
 *        An initial period is provided for the first update, and subsequent updates
 *        are done at half the keys lifetime.
 *
 * \warning This function shall only be called after ::SOPC_PubSubSKS_AddSkManager has been called for all security
 *          groups.
 *
 * \param scheduler     The scheduler to add tasks to
 * \param builder       The builder to use for updating the keys from provider to managers
 * \param provider      The provider to get keys from
 * \param msFirstUpdate The period used for first update in milliseconds, next updates are done at half keys lifetime.
 *
 * \return true if all tasks are added successfully, false otherwise
 *         (invalid parameters or ::SOPC_SKscheduler_AddTask failure)
 */
bool SOPC_PubSubSKS_AddTasks(SOPC_SKscheduler* scheduler,
                             SOPC_SKBuilder* builder,
                             SOPC_SKProvider* provider,
                             uint32_t msFirstUpdate);

/**
 * \brief Get the Security Keys Manager to use to retrieve the keys for UADP secure exchanges
 *        with the given security group id.
 *
 * \note This function should only be needed to retrieve user context in the SK manager.
 *
 * \param securityGroupId the security group id to associate the manager with
 *
 * \return the SK Manager if it exists for the given security group id, NULL otherwise.
 *        The returned SK Manager is not owned by the caller, it should not be cleared and no reference shall be kept.
 */
SOPC_SKManager* SOPC_PubSubSKS_GetSkManager(const char* securityGroupId);

#endif /* SOPC_PUBSUB_SKS_H_ */
