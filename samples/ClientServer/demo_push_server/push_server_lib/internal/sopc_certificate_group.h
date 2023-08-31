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
 * \brief Get the rejected list
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param[out] ppBsCertArray A valid pointer to the newly created ByteString array.
 * \param[out] pLength The length of \p ppBsCertArray
 *
 * \return Return SOPC_GoodGenericStatus if successful.
 */
SOPC_StatusCode CertificateGroup_GetRejectedList(const SOPC_CertGroupContext* pGroupCtx,
                                                 SOPC_ByteString** ppBsCertArray,
                                                 uint32_t* pLength);

/**
 * \brief Export the rejected list.
 *
 * \param pGroupCtx A valid pointer to the CertificateGroup context.
 * \param bEraseExiting Define if the existing certificate shall be deleted or include.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_StatusCode CertificateGroup_ExportRejectedList(const SOPC_CertGroupContext* pGroupCtx, const bool bEraseExisting);

#endif /* SOPC_CERTIFICATE_GROUP_ */
