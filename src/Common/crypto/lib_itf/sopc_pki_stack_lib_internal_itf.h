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

#ifndef SOPC_PKI_STACK_LIB_INT_ITF_H
#define SOPC_PKI_STACK_LIB_INT_ITF_H

#include "sopc_pki_decl.h"

/**
 * \brief Verifies if a certificate is self-signed or a CA (Certificate Authority) certificate.
 *        If yes to one or both of these conditions, an error is raised.
 *        If no, verifies if the certificate properties are good. If not, an error is raised.
 *        Sets the profile for configuration, and if correct, validates the certificate.
 *        If the certificate is not valid, raises an error.
 *        If there is an error with the certificate, the certificate is added to a certificate rejection list.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 *
 * \param pToValidate A valid pointer to the Certificate to validate.
 *
 * \param pProfile A valid pointer to the PKI chain profile.
 *
 * \param error Ouput error code set when returned status is not SOPC_STATUS_OK.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_ValidateProfileAndCertificate(SOPC_PKIProvider* pPKI,
                                                                 const SOPC_CertificateList* pToValidate,
                                                                 const SOPC_PKI_Profile* pProfile,
                                                                 uint32_t* error);

/**
 * \brief Delete the roots of the list ppCerts. Create a new list ppRootCa with all roots from ppCerts.
 *        If there is no root, the content of ppRootCa is set to NULL.
 *        If ppCerts becomes empty, its content is set to NULL.
 *
 * \param ppCerts A valid pointer to the certificate list to delete the roots of.
 *
 * \param ppRootCa A valid pointer to the new certificate list with the roots from ppCerts.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_SplitRootFromCertList(SOPC_CertificateList** ppCerts,
                                                         SOPC_CertificateList** ppRootCa);

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

#endif /* SOPC_PKI_STACK_LIB_INT_ITF_H */
