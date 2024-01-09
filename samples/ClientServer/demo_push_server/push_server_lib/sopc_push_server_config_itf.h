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
 * \brief API to manage methods, properties and variables of the ServerConfigurationType according the Push model.
 *        Initialize the whole API (CertificateGroup and TrustList).
 */

#ifndef SOPC_PUSH_SERVER_CONFIG_ITF_
#define SOPC_PUSH_SERVER_CONFIG_ITF_

#include "sopc_certificate_group_itf.h"
#include "sopc_key_manager.h"

/**
 * \brief Structure to gather the ServerConfiguration object configuration data
 */
typedef struct SOPC_PushServerConfig_Config SOPC_PushServerConfig_Config;

/**
 * \brief Initialise the whole API (CertificateGroup and TrustList)
 *
 * \warning The function shall be called before the server is started.
 *
 * \return SOPC_STATUS_OK if successful. If the CertificateGroup or the TrustList API are already initialized
 *         then the function returns SOPC_STATUS_INVALID_STATE.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_Initialize(void);

/**
 * \brief Get the default configuration for the ServerConfigurationType object.
 *
 * \param pPKIApp             A valid pointer to the PKI of the application TrustList.
 * \param appCertType         The application certificate type.
 * \param pServerKeyCertPair  A valid pointer to the server private key and certificate.
 * \param pServerKeyPath      Path to the server private key (NULL if the platform has no file system).
 * \param pServerCertPath     Path to the server certificate (NULL if the platform has no file system).
 * \param pPKIUsr             A valid pointer to the PKI of the users TrustList (NULL if not used).
 * \param maxTrustListSize    Defined the maximum size in byte of the TrustList.
 * \param[out] ppConfig       A newly created configuration. You should delete it with
 *                            ::SOPC_PushServerConfig_DeleteConfiguration .
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_GetDefaultConfiguration(SOPC_PKIProvider* pPKIApp,
                                                                const SOPC_Certificate_Type appCertType,
                                                                SOPC_KeyCertPair* pServerKeyCertPair,
                                                                const char* pServerKeyPath,
                                                                const char* pServerCertPath,
                                                                SOPC_PKIProvider* pPKIUsr,
                                                                const uint32_t maxTrustListSize,
                                                                SOPC_PushServerConfig_Config** ppConfig);
/**
 * \brief Get the default configuration for the ServerConfigurationType object in TOFU state.
 *
 * \param pPKIApp             A valid pointer to the PKI of the application TrustList.
 * \param appCertType         The application certificate type.
 * \param maxTrustListSize    Defined the maximum size in byte of the TrustList.
 * \param pFnUpdateCompleted  The callback when a new valid update of the TrustList has occurred.
 * \param[out] ppConfig       A newly created configuration. You should delete it with
 *                            ::SOPC_PushServerConfig_DeleteConfiguration .
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_GetTOFUConfiguration(SOPC_PKIProvider* pPKIApp,
                                                             const SOPC_Certificate_Type appCertType,
                                                             const uint32_t maxTrustListSize,
                                                             SOPC_TrustList_UpdateCompleted_Fct* pFnUpdateCompleted,
                                                             SOPC_PushServerConfig_Config** ppConfig);

/**
 * \brief Delete configuration.
 *
 * \param ppConfig The configuration.
 */
void SOPC_PushServerConfig_DeleteConfiguration(SOPC_PushServerConfig_Config** ppConfig);

/**
 * \brief Adding a ServerConfiguration object to the API.
 *
 * \note This function shall be call after ::SOPC_PushServerConfig_Initialize.
 *       This function shall be call before the server is started.
 *
 * \param pCfg        Pointer to the structure which gather the configuration data of the ServerConfiguration object.
 * \param pMcm        A valid pointer to a ::SOPC_MethodCallManager.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_Configure(SOPC_PushServerConfig_Config* pCfg, SOPC_MethodCallManager* pMcm);

/**
 * \brief Uninitialized the whole API (CertificateGroup and TrustList)
 */
void SOPC_PushServerConfig_Clear(void);

#endif /* SOPC_PUSH_SERVER_CONFIG_ITF_ */
