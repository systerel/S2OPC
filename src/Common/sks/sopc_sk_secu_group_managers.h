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
 * \brief SK security group managers stores the ::SOPC_SKManager to use to retrieve the keys for different
 * security groups
 *
 * The initialization ::SOPC_SK_SecurityGroup_Managers_Init shall be called prior to any other function of this module
 * and the clear ::SOPC_SK_SecurityGroup_Managers_Clear when this module need to be reset or is not used anymore.
 *
 * To define a security group manager, ::SOPC_SK_SecurityGroup_Managers_Init and ::SOPC_SK_SecurityGroup_SetSkManager
 * shall be called.
 *
 * The Publisher and Subscriber schedulers will then automatically call ::SOPC_PubSubSKS_GetSecurityKeys.
 */

#ifndef SOPC_SK_SECU_GROUP_MANAGERS_H_
#define SOPC_SK_SECU_GROUP_MANAGERS_H_

#include "sopc_sk_manager.h"

/**
 * \brief Initialises SKManagers dictionary for security groups
 */
void SOPC_SK_SecurityGroup_Managers_Init(void);

/**
 * \brief Clear the SKManagers dictionary for security groups and delete all managers
 */
void SOPC_SK_SecurityGroup_Managers_Clear(void);

/**
 * \brief Set the Security Keys Manager for the given security group identifier
 *
 * \param securityGroupId the identifier of the security group to associate the manager with
 *
 * \param skm the manager to use to get keys of the given security group
 *
 * \warning if this function is called several times with the same security group identifier,
 *          the previous manager is discarded.
 */
void SOPC_SK_SecurityGroup_SetSkManager(const char* securityGroupid, SOPC_SKManager* skm);

/**
 * \brief Get the Security Keys Manager associated with the given security group identifier
 *
 * \param securityGroupid   The identifier of the security group to get the manager for
 *
 * \return The SKManager associated with the security group if it exists,
 *         NULL otherwise
 */
SOPC_SKManager* SOPC_SK_SecurityGroup_GetSkManager(const char* securityGroupid);

#endif /* SOPC_SK_SECU_GROUP_MANAGERS_H_ */
