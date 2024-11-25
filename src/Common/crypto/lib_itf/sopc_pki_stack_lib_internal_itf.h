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

/** \file sopc_pki_stack_lib_internal_itf.h
 *
 * \brief Defines the PKI interface functions used internally.
 *
 */

#ifndef SOPC_PKI_STACK_LIB_INT_ITF_H
#define SOPC_PKI_STACK_LIB_INT_ITF_H

#include "sopc_pki_decl.h"

/** \brief Validation function for a certificate with the PKI chain
 *
 *  It implements the validation with the certificate chain of the PKI.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param pToValidate A valid pointer to the Certificate to validate.
 * \param pProfile A valid pointer to the PKI profile.
 * \param[out] error Pointer to store the OpcUa error code when certificate validation failed.
 * \param[out] context Pointer to store more details when certificate validation failed. Can be NULL. Only
 *  significant when return value is not \p SOPC_STATUS_OK
 *
 * \note Default validation function used by PKIProvider when not created by ::SOPC_PKIPermissive_Create (without
 * security)
 *
 * \warning In case of user PKI, the leaf profile part of \p pProfile is not applied to the certificate.
 *          The user leaf properties should be checked separately with ::SOPC_PKIProvider_CheckLeafCertificate .
 *
 * \return SOPC_STATUS_OK when the certificate is successfully validated, and
 *         SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK.
 *
 */
SOPC_ReturnStatus SOPC_PKIProviderInternal_ValidateProfileAndCertificate(SOPC_PKIProvider* pPKI,
                                                                         const SOPC_CertificateList* pToValidate,
                                                                         const SOPC_PKI_Profile* pProfile,
                                                                         uint32_t* error,
                                                                         SOPC_PKI_Cert_Failure_Context* context);

/**
 * \brief Delete the roots CAs of the list \p ppCerts. Create a new list \p ppRootCa with all roots CA from \p ppCerts .
 *        If there is no root CA, the content of \p ppRootCa is set to NULL.
 *        If \p ppCerts becomes empty, its content is set to NULL.
 *
 * \param ppCerts A valid pointer to the certificate list to delete the roots CA of.
 *
 * \param ppRootCa A valid pointer to the new certificate list with the roots CA from \p ppCerts .
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProviderInternal_SplitRootFromCertList(SOPC_CertificateList** ppCerts,
                                                                 SOPC_CertificateList** ppRootCa);

/**
 * \brief Get some statistics about the \p pCert .
 *
 * \param pCert A valid pointer to the certificate list.
 *
 * \param[out] caCount A valid pointer to store the number of certificate authorities.
 *
 * \param[out] listLength A valid pointer to store the length of the certificate list.
 *
 * \param[out] rootCount A valid pointer to store the number of root CA (self-signed certificate authority).
 *
 */
void SOPC_PKIProviderInternal_GetListStats(SOPC_CertificateList* pCert,
                                           uint32_t* caCount,
                                           uint32_t* listLength,
                                           uint32_t* rootCount);

#endif /* SOPC_PKI_STACK_LIB_INT_ITF_H */
