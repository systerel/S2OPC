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
 * \brief Internal API to manage the CertificateGroupType according the Push model.
 */

#ifndef SOPC_CERTIFICATE_GROUP_
#define SOPC_CERTIFICATE_GROUP_

#include "sopc_builtintypes.h"
#include "sopc_certificate_group_itf.h"
#include "sopc_key_cert_pair.h"
#include "sopc_key_manager.h"
#include "sopc_toolkit_config.h"
#include "sopc_trustlist.h"

#define SOPC_CERT_GRP_MIN_KEY_SIZE 2048
#define SOPC_CERT_GRP_MIN_MD_ALG "SHA256"
#define SOPC_CERT_GRP_MAX_KEY_SIZE 4096
#define SOPC_CERT_GRP_MAX_MD_ALG "SHA256"

/**
 * \brief Internal context for the CertificateGroup
 */
typedef struct SOPC_CertGroupContext
{
    const SOPC_NodeId* pObjectId;          /*!< The nodeId of the the CertificateGroup */
    char* cStrId;                          /*!< The C string nodeId of the CertificateGroup (it is used for logs) */
    SOPC_KeyCertPair* pKeyCertPair;        /*!< A valid pointer to the private key and certificate that belongs to the
                                                CertificateGroup object. */
    SOPC_AsymmetricKey* pNewKeyPair;       /*!< Pointer to the new generated key */
    char* pKeyPath;                        /*!< Path to the private key that belongs to the CertificateGroup object */
    char* pCertPath;                       /*!< Path to the certificate that belongs to the CertificateGroup object */
    const SOPC_NodeId* pCertificateTypeId; /*!< The nodeId of the certificateType variable that belongs to the
                                               CertificateGroup object. */
    const SOPC_NodeId* pCertificateTypeValueId; /*!< The value of the certificateType variable. */
    const SOPC_NodeId* pTrustListId;            /*!< The TrustList nodeId that belongs to the CertificateGroup object */
} SOPC_CertGroupContext;

/**
 * \brief Internal structure to gather nodeIds.
 */
typedef struct CertificateGroup_NodeIds
{
    const SOPC_NodeId* pCertificateGroupId; /*!< The NodeId of the Certificate Group Object. */
    const SOPC_NodeId* pCertificateTypesId; /*!< The nodeId of the CertificateTypes variable. */
    const SOPC_NodeId* pTrustListId;        /*!< The nodeId of the TrustList that belongs to the group */
} CertificateGroup_NodeIds;

/**
 * \brief Structure to gather CertificateGroup configuration data.
 */
struct SOPC_CertificateGroup_Config
{
    const CertificateGroup_NodeIds* pIds; /*!< Define all the nodeId of the CertificateGroup. */
    SOPC_TrustList_Config* pTrustListCfg; /*!< the TrustList configuration that belongs to the CertificateGroup. */
    SOPC_Certificate_Type certType;       /*!< The CertificateType. */
    const SOPC_PKIProvider* pPKI;         /*!< The PKI that belongs to the group. */
    SOPC_TrustList_Type groupType;        /*!< Define the group type (user or app) */
    SOPC_KeyCertPair* pKeyCertPair;       /*!< The private key and certificate that belongs to the group.*/
    char* pKeyPath;                       /*!< Path to the private key that belongs to the group */
    char* pCertPath;                      /*!< Path to the certificate that belongs to the group */
    bool bIsTOFUSate;                     /*!< When flag is set, the TOFU state is enabled. */
};

/**
 * \brief Get the CertificateGroupe context from a nodeId.
 *
 * \param pObjectId  The objectId of the CertificateGroup.
 * \param[out] bFound Define whatever the CertificateGroup is found that belongs to \p objectId .
 *
 * \return Return a valid ::SOPC_CertGroupContext or NULL if error.
 */
SOPC_CertGroupContext* CertificateGroup_GetFromNodeId(const SOPC_NodeId* pObjectId, bool* bFound);

/**
 * \brief Get the C string of the CertificateGroup nodeId
 *
 * \param pGroupCtx The CertificateGroup context.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 *\return The CertificateGroup nodeId (C string)
 */
const char* CertificateGroup_GetStrNodeId(const SOPC_CertGroupContext* pGroupCtx);

/**
 * \brief Check if the given certificate group nodeId is valid (!= DefaultUserTokenGroup)
 *
 * \param pGroupId  The nodeId of the group to check.
 *
 * \return True if valid.
 */
bool CertificateGroup_CheckIsApplicationGroup(const SOPC_NodeId* pGroupId);

/**
 * \brief Check the certificate type that belongs to the group.
 *
 * \param pGroupCtx The CertificateGroup context.
 * \param expected  The expected CertificateType.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return True if match.
 */
bool CertificateGroup_CheckType(const SOPC_CertGroupContext* pGroupCtx, const SOPC_NodeId* pExpectedCertTypeId);

/**
 * \brief Check the format of the given subjectName \p pSubjectName .
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param[out] pSubjectName The subject name to used.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return Return true if the subjectName \p pSubjectName is valid otherwise false.
 */
bool CertificateGroup_CheckSubjectName(SOPC_CertGroupContext* pGroupCtx, const SOPC_String* pSubjectName);

/**
 * \brief Get the list of certificates that have been rejected by the server,
 *        through the PKI attached to \p pGroupCtx .
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param[out] ppBsCertArray A valid pointer to the newly created ByteString array.
 * \param[out] pLength The length of \p ppBsCertArray
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return Return SOPC_GoodGenericStatus if successful.
 */
SOPC_StatusCode CertificateGroup_GetRejectedList(const SOPC_CertGroupContext* pGroupCtx,
                                                 SOPC_ByteString** ppBsCertArray,
                                                 uint32_t* pLength);

/**
 * \brief Create a PKCS #10 DER encoded Certificate Request that is signed with the Server’s private key
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param pSubjectName The subjectName of the CSR, if NULL then the subjectName of the current certificate is used.
 * \param bRegeneratePrivateKey Defines whether the private key of the server shall be regenerated.
 * \param[out] pCertificateRequest A valid byte string to store the CSR.
 * \param endpointConfigIdx The index associated to the configuration endpoint.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return Return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus CertificateGroup_CreateSigningRequest(SOPC_CertGroupContext* pGroupCtx,
                                                        const SOPC_String* pSubjectName,
                                                        const bool bRegeneratePrivateKey,
                                                        SOPC_ByteString* pCertificateRequest,
                                                        SOPC_EndpointConfigIdx endpointConfigIdx);

/**
 * \brief Update the new key-cert pair (TrustList is used to validate the new certificate instead
 *        of using the given issuers, see mantis #0009247 - https://mantis.opcfoundation.org/view.php?id=9247)
 *
 * \note If the key is new, it has been generated using the CreateSigningRequest method and stored in context
 *       \p pGroupCtx .
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param pCertificate A valid pointer to the new certificate.
 * \param endpointConfigIdx The index associated to the configuration endpoint.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL).
 *
 * \return Return SOPC_GoodGenericStatus if successful or an OpcUa error code.
 */
SOPC_StatusCode CertificateGroup_UpdateCertificate(SOPC_CertGroupContext* pGroupCtx,
                                                   const SOPC_ByteString* pCertificate,
                                                   SOPC_EndpointConfigIdx endpointConfigIdx);

/**
 * \brief Export the new key-cert pair to the file system.
 *
 * \note If the key is new, it has been generated using the CreateSigningRequest method and stored in context \p
 * pGroupCtx .
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus CertificateGroup_Export(const SOPC_CertGroupContext* pGroupCtx);

/**
 * \brief  Discards previously generated new key pair.
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return Return SOPC_STATUS_OK if successful.
 */
void CertificateGroup_DiscardNewKey(SOPC_CertGroupContext* pGroupCtx);

/**
 * \brief Export the list of certificates that have been rejected by the server to the file system
 *        through the PKI attached to \p pGroupCtx
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_StatusCode CertificateGroup_ExportRejectedList(const SOPC_CertGroupContext* pGroupCtx);

#endif /* SOPC_CERTIFICATE_GROUP_ */
