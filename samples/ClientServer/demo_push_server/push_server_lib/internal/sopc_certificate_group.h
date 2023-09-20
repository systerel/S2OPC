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
#include "sopc_key_manager.h"

/**
 * \brief Internal context for the CertificateGroup
 */
typedef struct SOPC_CertGroupContext
{
    SOPC_NodeId* pObjectId;               /*!< The nodeId of the the CertificateGroup */
    char* cStrId;                         /*!< The C string nodeId of the CertificateGroup (it is used for logs) */
    SOPC_SerializedAsymmetricKey* pKey;   /*!< A valid pointer to the private key that belongs to the
                                               CertificateGroup object. */
    SOPC_SerializedCertificate* pCert;    /*!< A valid pointer to the certificate that belongs to the
                                              CertificateGroup object. */
    SOPC_AsymmetricKey* pNewKey;          /*!< Pointer to the new generated key */
    char* pKeyPath;                       /*!< Path to the private key that belongs to the CertificateGroup object */
    char* pCertPath;                      /*!< Path to the certificate that belongs to the CertificateGroup object */
    SOPC_NodeId* pCertificateTypeId;      /*!< The nodeId of the certificateType variable that belongs to the
                                              CertificateGroup object. */
    SOPC_NodeId* pCertificateTypeValueId; /*!< The value of the certificateType variable. */
    SOPC_NodeId* pTrustListId;            /*!< The TrustList nodeId that belongs to the CertificateGroup object */
    bool bDoNotDelete;                    /*!< Defined whatever the CertificateGroup context shall be deleted */
} SOPC_CertGroupContext;

/**
 * \brief Insert a new objectId key and CertificateGroup context value.
 *
 * \param pObjectId The objectId of the CertificateGroup.
 * \param pContext  A valid pointer on the CertificateGroup context.
 *
 * \return \c TRUE in case of success.
 */
bool CertificateGroup_DictInsert(SOPC_NodeId* pObjectId, SOPC_CertGroupContext* pContext);

/**
 * \brief Get the CertificateGroupe context from the nodeId.
 *
 * \param pObjectId  The objectId of the CertificateGroup.
 * \param[out] bFound Defined whatever the CertificateGroup is found that belongs to \p objectId .
 *
 * \return Return a valid ::SOPC_CertGroupContext or NULL if error.
 */
SOPC_CertGroupContext* CertificateGroup_DictGet(const SOPC_NodeId* pObjectId, bool* bFound);

/**
 * \brief Removes a CertificateGroup context from the nodeId.
 *
 * \param pObjectId The objectId of the CertificateGroup.
 */
void CertificateGroup_DictRemove(const SOPC_NodeId* pObjectId);

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
 * \brief Check if the give certificate group nodeId is valid (!= DefaultUserTokenGroup)
 *
 * \param pGroupId  The nodeId of the group to check.
 *
 * \return True if valid.
 */
bool CertificateGroup_CheckGroup(const SOPC_NodeId* pGroupId);

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
 * \brief Get the rejected list
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
 * \brief Regenerates the server’s private key.
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return Return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus CertificateGroup_RegeneratePrivateKey(SOPC_CertGroupContext* pGroupCtx);

/**
 * \brief Create a PKCS #10 DER encoded Certificate Request that is signed with the Server’s private key
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param pSubjectName The subjectName of the CSR, if NULL then the subjectName of the current certificate is used.
 * \param bRegeneratePrivateKey Defines whether the private key of the server shall be regenerated.
 * \param[out] pCertificateRequest A valid byte string to store the CSR.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return Return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus CertificateGroup_CreateSigningRequest(SOPC_CertGroupContext* pGroupCtx,
                                                        const SOPC_String* pSubjectName,
                                                        const bool bRegeneratePrivateKey,
                                                        SOPC_ByteString* pCertificateRequest);

/**
 * \brief Update the new key-cert pair (Do all normal integrity checks on the certificate and all of the issuer
 * certificates)
 *
 * \note If the key is new, it has been generated using the CreateSigningRequest method and stored in context \p
 * pGroupCtx .
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param pCertificate A valid pointer to the new certificate.
 * \param pIssuerArray An array of issuer certificates needed to verify the signature on the new certificate.
 * \param arrayLength The length of the array \p pIssuerArray
 *
 * \warning \p pGroupCtx shall be valid (!= NULL).
 *
 * \return Return SOPC_GoodGenericStatus if successful or an OpcUa error code.
 */
SOPC_StatusCode CertificateGroup_UpdateCertificate(const SOPC_CertGroupContext* pGroupCtx,
                                                   const SOPC_ByteString* pCertificate,
                                                   const SOPC_ByteString* pIssuerArray,
                                                   const int32_t arrayLength);

/**
 * \brief Export the new key-cert pair.
 *
 * \note If the key is new, it has been generated using the CreateSigningRequest method and stored in context \p
 * pGroupCtx .
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param pCertificate A valid pointer to the new certificate.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus CertificateGroup_Export(const SOPC_CertGroupContext* pGroupCtx, const SOPC_ByteString* pCertificate);

/**
 * \brief Raise an event to re-evaluate the certificate for all SCs.
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL).
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_ReturnStatus CertificateGroup_RaiseEvent(const SOPC_CertGroupContext* pGroupCtx);

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
 * \brief Export the rejected list.
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param bEraseExiting Define if the existing certificate shall be deleted or include.
 *
 * \warning \p pGroupCtx shall be valid (!= NULL)
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_StatusCode CertificateGroup_ExportRejectedList(const SOPC_CertGroupContext* pGroupCtx, const bool bEraseExisting);

#endif /* SOPC_CERTIFICATE_GROUP_ */
