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
 */

#ifndef SOPC_PUSH_SERVER_CONFIG_ITF_
#define SOPC_PUSH_SERVER_CONFIG_ITF_

#include "sopc_certificate_group_itf.h"
#include "sopc_key_manager.h"

/**
 * \brief Structure to gather the ServerConfiguration object configuration data
 */
typedef struct SOPC_PushServerConfig_Config
{
    const char* serverConfigurationNodeId;                /*!< The nodeId of the FileType object. */
    const char* metUpdateCertificateNodeId;               /*!< The nodeId of the UpdateCertificate method. */
    const char* metApplyChangesNodeId;                    /*!< The nodeId of the ApplyChanges method.*/
    const char* metCreateSigningRequestNodeId;            /*!< The nodeId of the CreateSigningRequest method.*/
    const char* metGetRejectedListNodeId;                 /*!< The nodeId of the CreateSigningRequest method.*/
    const char* varServerCapabilitiesNodeId;              /*!< The nodeId of the ServerCapabilities variable.*/
    const char* varSupportedPrivateKeyFormatsNodeId;      /*!< The nodeId of the SupportedPrivateKeyFormat variable.*/
    const char* varMaxTrustListSizeNodeId;                /*!< The nodeId of the MaxTrustListSize variable.*/
    const char* varMulticastDnsEnabledNodeId;             /*!< The nodeId of the MulticastDnsEnabled variable.*/
    const SOPC_CertificateGroup_Config* pAppCertGroupCfg; /*!< Application certificate group configuration
                                                               belongs the CertificateGroupeFolderType */
    const SOPC_CertificateGroup_Config* pUsrCertGroupCfg; /*!< Users certificate group configuration belongs the
                                                               CertificateGroupeFolderType (NULL if not used) */
    SOPC_Certificate_Type appCertType;                    /*!< The application certificate type */
    SOPC_Certificate_Type usrCertType;                    /*!< The users certificate type */
} SOPC_PushServerConfig_Config;

/**
 * \brief Initialise the whole API (CertificateGroup and TrustList)
 *
 * \warning The function shall be called after ::SOPC_HelperConfigServer_Initialize and before the server startup.
 *
 * \return SOPC_STATUS_OK if successful. If the CertificateGroup or the TrustList API are already initialized
 *         then the function returns SOPC_STATUS_INVALID_STATE.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_Initialize(void);

/**
 * \brief Get the configuration with the default address space for the ServerConfigurationType object.
 *
 * \param pPKIApp     A valid pointer to the PKI of the application TrustList.
 * \param appCertType The application certificate type.
 * \param pPKIUser    A valid pointer to the PKI of the users TrustList (NULL if not used).
 * \param usrCertType The users certificate type (ignored if \p pPKIUser is NULL).
 * \param[out] pCfg   A valid pointer to set the configuration structure.
 *
 * \return SOPC_STATUS_OK if successful otherwise SOPC_STATUS_INVALID_PARAMETERS.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_GetDefaultConfiguration(SOPC_PKIProvider* pPKIApp,
                                                                const SOPC_Certificate_Type appCertType,
                                                                SOPC_PKIProvider* pPKIUsers,
                                                                const SOPC_Certificate_Type usrCertType,
                                                                SOPC_PushServerConfig_Config* pCfg);

/**
 * \brief Adding a ServerConfiguration object to the API from the address space information.
 *
 * \note This function shall be call after ::SOPC_PushServerConfig_Initialize , ::SOPC_CertificateGroup_Initialize and
 *       ::SOPC_TrustList_Initialize . This function shall be call before the server is started.
 *
 * \param pCfg        Pointer to the structure which gather the configuration data of the ServerConfiguration object.
 * \param pServerKey  A valid pointer to the server private key.
 * \param pServerCert A valid pointer to the server certificate.
 * \param pMcm        A valid pointer to a ::SOPC_MethodCallManager.
 *
 * \warning In case of error, the API is uninitialized (except for SOPC_STATUS_INVALID_PARAMETERS and
 *          SOPC_STATUS_INVALID_STATE errors). Only one ServerConfigurationType could be added to the server.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_PushServerConfig_Configure(const SOPC_PushServerConfig_Config* pCfg,
                                                  SOPC_SerializedAsymmetricKey* pServerKey,
                                                  SOPC_SerializedCertificate* pServerCert,
                                                  SOPC_MethodCallManager* pMcm);

/**
 * \brief Uninitialized the API
 */
void SOPC_PushServerConfig_Clear(void);

#endif /* SOPC_PUSH_SERVER_CONFIG_ITF_ */
