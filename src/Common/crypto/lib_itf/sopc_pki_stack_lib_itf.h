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

/**
 * \brief Create the PKIProvider from list representation.
 *
 * Notions :
 * - CA is a root CA if it is self-signed.
 * - \p pTrustedCerts = trusted root CA + trusted link CA + trusted cert.
 * - \p pTrustedCrl = CRLs of the trusted root CA + trusted link CA.
 * - \p pIssuerCerts = untrusted root CA + untrusted link CA.
 * - \p pIssuerCrl = CRLs of the untrusted root CA + untrusted link CA.
 * - CAs from trusted/certs and issuers/certs allow to verify the signing chain of a cert which is included into
 *   trusted/certs.
 * - CAs from trusted/certs allow to verify the signing chain of a cert which is not included into trusted/certs.
 *
 * This function checks that :
 * - the number of certificates plus CRLs does not exceed \c SOPC_PKI_MAX_NB_CERT_AND_CRL .
 * - at least one cert from \p pTrustedCerts is provided.
 * - each certificate from \p pIssuerCerts is CA.
 * - each CA has exactly one Certificate Revocation List (CRL).
 *
 * \param pTrustedCerts A valid pointer to the trusted certificate list.
 * \param pTrustedCrl A valid pointer to the trusted CRL list.
 * \param pIssuerCerts A valid pointer to the issuer certificate list. NULL if not used.
 * \param pIssuerCrl A valid pointer to the issuer CRL list. NULL if not used.
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProvider_Free().
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProvider_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  SOPC_PKIProvider** ppPKI);

/** \brief Verify every certificate of the PKI
 *
 *   Each certificate of the chain is checked for signature, validity and profile.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param pProfile A valid pointer to the PKI chain profile.
 * \param[out] pErrors Array to store the OpcUa error code when a certificate is invalid.
 * \param[out] ppThumbprints Array to store the certificate thumbprint when a certificate is invalid.
 * \param[out] pLength The length of \p pErrors and \p ppThumbprints .
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
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckHostName(const SOPC_CertificateList* pToValidate, const char* url);

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

/**
 * \brief Get some statistics about the certificate chain.
 *
 * \param pCert A valid pointer to the certificate chain.
 *
 * \param caCount A valid pointer to store the number of certificate authorities.
 *
 * \param listLength A valid pointer to store the length of the certificate chain.
 *
 * \param rootCount A valid pointer to store the number of self-signed certificates.
 *
 */
void SOPC_PKIProvider_GetListStats(SOPC_CertificateList* pCert,
                                   uint32_t* caCount,
                                   uint32_t* listLength,
                                   uint32_t* rootCount);

/**
 * \brief Free a PKI provider.
 *
 * \param ppPKI The PKI.
 */
void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI);

// TODO: complete doc
/**
 * \brief Delete the roots of the list ppCerts. Create a new list ppRootCa with all roots from ppCerts.
 *        If there is no root, the content of ppRootCa is set to NULL.
 *        If ppCerts becomes empty, its content is set to NULL.
 *
 * \param ppCerts
 *
 * \param ppRootCa
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_SplitRootFromCertList(SOPC_CertificateList** ppCerts,
                                                         SOPC_CertificateList** ppRootCa);

// TODO: complete doc
/**
 * \brief
 *
 * \param pPKI
 *
 * \param pToValidate
 *
 * \param pProfile
 *
 * \param error
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_ValidateProfileAndCertificate(SOPC_PKIProvider* pPKI,
                                                                 const SOPC_CertificateList* pToValidate,
                                                                 const SOPC_PKI_Profile* pProfile,
                                                                 uint32_t* error);

#endif /* SOPC_PKI_STACK_LIB_ITF_H_ */
