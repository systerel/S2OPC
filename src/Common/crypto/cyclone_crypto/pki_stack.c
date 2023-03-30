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

/** \file pki_stack.c
 *
 * The minimal PKI implementation provided by the stack. It is lib-specific.
 *
 * This is not the role of the stack to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki.h"
#include "sopc_pki_stack.h"

#include "key_manager_lib.h"

// TODO: the right cyclone_crypto includes here

/**
 * The minimal profile supported by the PKIProviderStack. It requires cacert signed with
 *  at least SHA-256, with an RSA key of at least 2048 bits.
 */

// static const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_minimal = {
//     /* Hashes from SHA-256 and above */
//     .allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
//                    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512),
//     .allowed_pks = 0xFFFFFFF,     /* Any PK alg */
//     .allowed_curves = 0xFFFFFFFF, /* Any curve  */
//     .rsa_min_bitlen = 2048,
// };
/* Lib-specific definition */

/* static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    SOPC_UNUSED_ARGfailure_reasons;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_CertificateValidationError_Unkown;
} */
/* Defined but not used */

/* static int verify_cert(void* is_issued, int certificate_depth, uint32_t* flags)
{
    SOPC_UNUSED_ARGis_issued;
    SOPC_UNUSED_ARGcertificate_depth;
    SOPC_UNUSED_ARGflags;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return 0;
} */
/* Defined but not used */

/* Returns 0 if all key usages and extended key usages are ok */
/* static SOPC_ReturnStatus check_key_usages(bool isUserPki)
{
    SOPC_UNUSED_ARGisUserPki;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;

} */
/* Defined but not used */

/* static SOPC_ReturnStatus PKIProviderStack_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                              const SOPC_CertificateList* pToValidate,
                                                              uint32_t* error)
{
    SOPC_UNUSED_ARGpPKI;
    SOPC_UNUSED_ARGpToValidate;
    SOPC_UNUSED_ARGerror;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
} */
/* Defined but not used */

/* static void PKIProviderStack_Free(SOPC_PKIProvider* pPKI)
{
    SOPC_UNUSED_ARGpPKI;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");
} */
/* Defined but not used */

/* static SOPC_PKIProvider* create_pkistack(SOPC_CertificateList* lRootsTrusted,
                                         SOPC_CertificateList* lLinksTrusted,
                                         SOPC_CertificateList* lRootsUntrusted,
                                         SOPC_CertificateList* lLinksUntrusted,
                                         SOPC_CertificateList* lIssued,
                                         SOPC_CRLList* lCrl,
                                         bool isUserPki)
{
    SOPC_PKIProvider* pki;
    SOPC_UNUSED_ARGlRootsTrusted;
    SOPC_UNUSED_ARGlLinksTrusted;
    SOPC_UNUSED_ARGlRootsUntrusted;
    SOPC_UNUSED_ARGlLinksUntrusted;
    SOPC_UNUSED_ARGlIssued;
    SOPC_UNUSED_ARGlCrl;
    SOPC_UNUSED_ARGisUserPki;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return pki;
} */
/* Defined but not used */

SOPC_ReturnStatus SOPC_PKIProviderStack_SetUserCert(SOPC_PKIProvider* pPKI, bool bIsUserPki)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(bIsUserPki);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI)
{
    SOPC_UNUSED_ARG(pCertAuth);
    SOPC_UNUSED_ARG(pRevocationList);
    SOPC_UNUSED_ARG(ppPKI);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/* static SOPC_CertificateList* load_certificate_list(char** paths, SOPC_ReturnStatus* status)
{
    SOPC_UNUSED_ARGpaths;
    SOPC_UNUSED_ARGstatus;
    SOPC_CertificateList* pcert = NULL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return pcert;
} */
/* Defined but not used */

/** \brief Create the prev list if required */
/* static SOPC_ReturnStatus link_certificates(SOPC_CertificateList** ppPrev, SOPC_CertificateList** ppNext)
{
    SOPC_UNUSED_ARGppPrev;
    SOPC_UNUSED_ARGppNext;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
} */
/* Defined but not used */

SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathTrustedIssuerRoots,
                                                        char** lPathTrustedIssuerLinks,
                                                        char** lPathUntrustedIssuerRoots,
                                                        char** lPathUntrustedIssuerLinks,
                                                        char** lPathIssuedCerts,
                                                        char** lPathCRL,
                                                        SOPC_PKIProvider** ppPKI)
{
    SOPC_UNUSED_ARG(lPathTrustedIssuerRoots);
    SOPC_UNUSED_ARG(lPathTrustedIssuerLinks);
    SOPC_UNUSED_ARG(lPathUntrustedIssuerRoots);
    SOPC_UNUSED_ARG(lPathUntrustedIssuerLinks);
    SOPC_UNUSED_ARG(lPathIssuedCerts);
    SOPC_UNUSED_ARG(lPathCRL);
    SOPC_UNUSED_ARG(ppPKI);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}
