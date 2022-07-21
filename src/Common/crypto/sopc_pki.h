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

/** \file sopc_pki.h
 *
 * \brief Defines the common interface that a PKI should provide. This is a minimal interface, as the main
 *        API for certificate and key manipulation is provided by SOPC_KeyManager.
 *
 * The stack will not provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 *
 * The stack will not provide any advanced certificate storage.
 * You can use "user-specific" handles in the PKIProvider struct to implement more options.
 *
 * The pFnValidateCertificate function should not be called directly, but you should call
 * SOPC_CryptoProvider_Certificate_Validate() instead.
 */

#ifndef SOPC_PKI_H_
#define SOPC_PKI_H_

#include "sopc_crypto_decl.h"
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"

#define SOPC_CertificateValidationError_Invalid 0x80120000
#define SOPC_CertificateValidationError_TimeInvalid 0x80140000
#define SOPC_CertificateValidationError_IssuerTimeInvalid 0x80150000
#define SOPC_CertificateValidationError_HostNameInvalid 0x80160000
#define SOPC_CertificateValidationError_UriInvalid 0x80170000
#define SOPC_CertificateValidationError_UseNotAllowed 0x80180000
#define SOPC_CertificateValidationError_IssuerUseNotAllowed 0x80190000
#define SOPC_CertificateValidationError_Untrusted 0x801A0000
#define SOPC_CertificateValidationError_RevocationUnknown 0x801B0000
#define SOPC_CertificateValidationError_IssuerRevocationUnknown 0x801C0000
#define SOPC_CertificateValidationError_Revoked 0x801D0000
#define SOPC_CertificateValidationError_IssuerRevoked 0x801E0000
#define SOPC_CertificateValidationError_ChainIncomplete 0x810D0000
#define SOPC_CertificateValidationError_Unkown 0x80000000

typedef void SOPC_PKIProvider_Free_Func(SOPC_PKIProvider* pPKI);

/**
 * \brief \p error is only set if returned status is different from SOPC_STATUS_OK
 *
 */
typedef SOPC_ReturnStatus SOPC_FnValidateCertificate(const struct SOPC_PKIProvider* pPKI,
                                                     const SOPC_CertificateList* pToValidate,
                                                     uint32_t* error);

/**
 * \brief   The PKIProvider object defines the common interface for the Public Key Infrastructure.
 */
struct SOPC_PKIProvider
{
    /**
     * \brief   The free function, called upon generic SOPC_PKIProvider destruction.
     */
    SOPC_PKIProvider_Free_Func* const pFnFree;

    /** \brief The validation function, which is wrapped by SOPC_CryptoProvider_Certificate_Validate().
     *
     *   It implements the validation of the certificate. The SOPC_CryptoProvider_Certificate_Validate()
     *   assumes that a SOPC_STATUS_OK from this function means that the certificate can be trusted. Parameters are
     *   validated by SOPC_CryptoProvider_Certificate_Validate().
     *
     * \param pPKI     A valid pointer to the PKIProvider.
     * \param pToValidate  A valid pointer to the Certificate to validate.
     *
     * \return         SOPC_STATUS_OK when the certificate is successfully validated, and
     *                 SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK.
     */
    SOPC_FnValidateCertificate* const pFnValidateCertificate;

    /** \brief Placeholder for CAs of trusted issuer roots (only roots, not links). */
    void* pTrustedIssuerRootsList;
    /** \brief Placeholder for CAs of trusted issuers links (between a certificate to validate and a root). */
    void* pTrustedIssuerLinksList;
    /** \brief Placeholder for CAs of untrusted issuer roots (CAs used to validate issued certififcates, only roots). */
    void* pUntrustedIssuerRootsList;
    /** \brief Placeholder for CAs of untrusted issuers links (between a certificate to validate and a root). */
    void* pUntrustedIssuerLinksList;
    /** \brief Placeholder for issued certificates that are trusted on a one by one basis. */
    void* pIssuedCertsList;
    /** \brief PKI implementations can use this placeholder to store handles to certificate revocation list(s). */
    void* pCertRevocList;
    /** \brief PKI implementations can use this placeholder to store more specific data. */
    void* pUserData;
};

/**
 * \brief   Free a PKI provider.
 */
void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI);

#endif /* SOPC_PKI_H_ */
