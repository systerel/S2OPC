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
 * \brief API to manage methods, properties and variables of the CertificateGroupType according the Push model.
 */

#ifndef SOPC_CERTIFICATE_GROUP_ITF_
#define SOPC_CERTIFICATE_GROUP_ITF_

#include "sopc_trustlist_itf.h"

/**
 * \brief The Certificate type
 */
typedef enum
{
    SOPC_CERT_TYPE_UNKNOW = 0x00,
    SOPC_CERT_TYPE_RSA_MIN_APPLICATION = 0x01, /*!< Describes the RsaMinApplicationCertificateType (RSA key size of 1024
                                                  or 2048 bits for profile Basic256) */
    SOPC_CERT_TYPE_RSA_SHA256_APPLICATION = 0x02, /*!< Describes the RsaSha256ApplicationCertificateType (2048, 3072 or
                                                     4096 bits for profile Basic256Sha256) */
} SOPC_Certificate_Type;

/**
 * \brief Structure to gather the CertificateGroup object configuration data
 */
typedef struct SOPC_CertificateGroup_Config
{
    const char* certificateGroupNodeId;    /*!< The NodeId of the Certificate Group Object.*/
    const char* varCertificateTypesNodeId; /*!< The nodeId of the CertificateTypes variable.*/
    const SOPC_TrustList_Config*
        pTrustListCfg;              /*!< the TrustList configuration that belongs to the CertificateGroup object */
    SOPC_Certificate_Type certType; /*!< The CertificateType */
} SOPC_CertificateGroup_Config;

/**
 * \brief Initialise the API.
 *
 * \warning The function shall be called before the server is started.
 *
 * \return SOPC_STATUS_OK if successful. If the CertificateGroup API is already initialized
 *         then the function returns SOPC_STATUS_INVALID_STATE.
 */
SOPC_ReturnStatus SOPC_CertificateGroup_Initialize(void);

/**
 * \brief Get the CertificateGroup object configuration with the default address space.
 *
 * \param groupType  Defined the certificate group type of the TrustList (application or user).
 * \param certType   Defined the certificate type (certificate properties).
 * \param pPKI       A valid pointer to the TrustList PKI that belongs to the CertificateGroup object.
 *
 * \note The function return NULL in case of error.
 *
 * \return Return the structure ::SOPC_CertificateGroup_Config filed with default values.
 */
const SOPC_CertificateGroup_Config* SOPC_CertificateGroup_GetDefaultConfiguration(const SOPC_TrustList_Type groupType,
                                                                                  const SOPC_Certificate_Type certType,
                                                                                  SOPC_PKIProvider* pPKI);

/**
 * \brief Adding a CertificateGroup object to the API from the address space information.
 *
 * \note This function shall be call after ::SOPC_CertificateGroup_Initialize and ::SOPC_TrustList_Initialize .
 *       This function shall be call before the server is started.
 *
 * \param pCfg  Pointer to the structure which gather the configuration data of the CertificateGroup object
 * \param pKey  A valid pointer to the private key that belongs to the CertificateGroup object (NULL if not use)
 * \param pCert A valid pointer to the certificate that belongs to the CertificateGroup object (NULL if not use)
 * \param pMcm  A valid pointer to a ::SOPC_MethodCallManager.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus SOPC_CertificateGroup_Configure(const SOPC_CertificateGroup_Config* pCfg,
                                                  SOPC_SerializedAsymmetricKey* pKey,
                                                  SOPC_SerializedCertificate* pCert,
                                                  SOPC_MethodCallManager* pMcm);

/**
 * \brief Uninitialized the API
 */
void SOPC_CertificateGroup_Clear(void);

#endif /* SOPC_CERTIFICATE_GROUP_ITF_ */
