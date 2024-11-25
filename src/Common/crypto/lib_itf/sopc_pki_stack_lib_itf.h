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
 * \brief Defines the cryptographic abstraction interface for
 *        the minimal PKI implementation provided by the stack.
 *        A cryptographic implementation must define all the function declared in this file.
 *        The stack will not to provide a full-blown configurable PKI.
 *        The stack provides only a minimal, always safe validating PKI.
 */

#ifndef SOPC_PKI_STACK_LIB_ITF_H_
#define SOPC_PKI_STACK_LIB_ITF_H_

#include "sopc_pki_decl.h"

/*
TODO :
    - Handle that the security level of the update is not higher than the security level of the endpoint
      (The following issue has been SUBMITTED : https://mantis.opcfoundation.org/view.php?id=8976)
*/

/** \brief Verify every certificate of the PKI
 *
 *   Each certificate of the chain is checked for signature, validity and profile.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param pProfile A valid pointer to the PKI chain profile.
 * \param[out] pErrors Array to store the OpcUa error code when a certificate is invalid.
 * \param[out] ppThumbprints Array to store the certificate thumbprint when a certificate is invalid.
 * \param[out] pLength The length of \p pErrors and \p ppThumbprints.
 *
 * \note \p pErrors and \p ppThumbprints are only created and set if the returned status is SOPC_STATUS_NOK.
 *       In case of invalid certificate (SOPC_STATUS_NOK) the thumbprint is associated to the error
 *       at the same index.
 *
 * \return SOPC_STATUS_OK when every certificate is successfully validated, and
 *         SOPC_STATUS_INVALID_PARAMETERS, SOPC_STATUS_INVALID_STATE, SOPC_STATUS_OUT_OF_MEMORY or SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_PKIProvider_VerifyEveryCertificate(SOPC_PKIProvider* pPKI,
                                                          const SOPC_PKI_ChainProfile* pProfile,
                                                          uint32_t** pErrors,
                                                          char*** ppThumbprints,
                                                          uint32_t* pLength);

/** \brief Add a certificate to the PKI rejected list.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param pCert A valid pointer to the certificate to be added.
 *
 * \note The function removes the oldest certificate if the list size reaches \c SOPC_PKI_MAX_NB_CERT_REJECTED.
 *
 * \warning \p pCert shall contains a single certificate.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_AddCertToRejectedList(SOPC_PKIProvider* pPKI, const SOPC_CertificateList* pCert);

/**
 * \brief Checks if the Common Name attribute of a certificate thumbprint is specified.
 *
 * \param pToValidate A valid pointer to the Certificate to validate.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckCommonName(const SOPC_CertificateList* pToValidate);

/**
 * \brief Checks if the PKI public keys are valid.
 *
 * \param pToValidate A valid pointer to the Certificate to validate.
 *
 * \param pConfig A valid pointer to a profile configuration.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckSecurityPolicy(const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_LeafProfile* pConfig);

/**
 * \brief Checks if the host name in the URL used
 * to connect to the server is the same as the host name in the certificate URL.
 *
 * \param pToValidate A valid pointer to the Certificate to validate.
 *
 * \param url A valid URL extracted from a certificate.
 * \param certUrl An optional pointer. If not NULL, it will be allocated and filled with a copy of the hostname
 *        found in \p pToValidate
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckHostName(const SOPC_CertificateList* pToValidate,
                                                 const char* url,
                                                 char** certUrl);

/**
 * \brief Checks if the certificate uses are matching the required uses.
 *
 * \param pToValidate A valid pointer to the certificate to validate.
 *
 * \param pProfile A valid pointer to the expected profile of a certificate.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckCertificateUsage(const SOPC_CertificateList* pToValidate,
                                                         const SOPC_PKI_LeafProfile* pProfile);

#endif /* SOPC_PKI_STACK_LIB_ITF_H_ */
