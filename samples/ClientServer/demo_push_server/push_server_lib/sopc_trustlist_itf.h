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
 * \brief API to manage methods, properties and variables of the TrustListType according the Push model.
 *        S2OPC shall be build with S2OPC_DYNAMIC_TYPE_RESOLUTION option.
 */

#ifndef SOPC_TRUSTLIST_ITF_
#define SOPC_TRUSTLIST_ITF_

#include "sopc_builtintypes.h"
#include "sopc_call_method_manager.h"
#include "sopc_pki_stack.h"

/**
 * \brief Defined the default maximum size in byte of the trustList certificate
 */
#define SOPC_TRUSTLIST_DEFAULT_MAX_SIZE 65535u

/**
 * \brief The TrustList type
 */
typedef enum
{
    SOPC_TRUSTLIST_GROUP_APP,
    SOPC_TRUSTLIST_GROUP_USR,
} SOPC_TrustList_Type;

/**
 * \brief Structure to gather TrustList configuration data
 */
typedef struct SOPC_TrustList_Config SOPC_TrustList_Config;

/**
 * \brief Initialise the API.
 *
 * \warning The function shall be called after ::SOPC_HelperConfigServer_Initialize and before the server startup.
 *
 * \return SOPC_STATUS_OK if successful. If the TrustList API is already initialized
 *         then the function returns SOPC_STATUS_INVALID_STATE.
 */
SOPC_ReturnStatus SOPC_TrustList_Initialize(void);

/**
 * \brief Get the TrustList configuration with the default values.
 *
 * \param groupType        Defined the certificate group type of the TrustList.
 * \param pPKI             A valid pointer to the PKI of the TrustList.
 * \param maxTrustListSize Defined the maximum size in byte of the TrustList.
 * \param[out] ppConfig    A newly created configuration. You should delete it with
 *                         ::SOPC_TrustList_DeleteConfiguration .
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_TrustList_GetDefaultConfiguration(const SOPC_TrustList_Type groupType,
                                                         SOPC_PKIProvider* pPKI,
                                                         const uint32_t maxTrustListSize,
                                                         SOPC_TrustList_Config** ppConfig);

/**
 * \brief Delete TrustList configuration.
 *
 * \param ppConfig The configuration.
 */
void SOPC_TrustList_DeleteConfiguration(SOPC_TrustList_Config** ppConfig);

/**
 * \brief Adding a Trustlist object to the API from the address space information.
 *
 * \note This function shall be call after ::SOPC_TrustList_Initialize and before the server is started.
 *
 * \param pCfg Pointer to the structure which gather Trustlist configuration data.
 * \param pMcm A valid pointer to a ::SOPC_MethodCallManager.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_TrustList_Configure(SOPC_TrustList_Config* pCfg, SOPC_MethodCallManager* pMcm);

/**
 * \brief Uninitialized the TrustList API
 */
void SOPC_TrustList_Clear(void);

#endif /* SOPC_TRUSTLIST_ITF_ */
