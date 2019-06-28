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

#include <stdlib.h>

#include "sopc_macros.h"

#include "../sopc_crypto_provider.h"
#include "../sopc_key_manager.h"
#include "../sopc_pki.h"
#include "../sopc_pki_stack.h"
#include "key_manager_lib.h"
#include "mbedtls/x509.h"

/**
 * The minimal profile supported by the PKIProviderStack. It requires cacert signed with
 *  at least SHA-256, with an RSA key of at least 2048 bits.
 */
static const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_minimal = {
    /* Hashes from SHA-256 and above */
    .allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
                   MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512),
    .allowed_pks = 0xFFFFFFF,     /* Any PK alg */
    .allowed_curves = 0xFFFFFFFF, /* Any curve  */
    .rsa_min_bitlen = 2048,
};

static SOPC_ReturnStatus PKIProviderStack_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                              const SOPC_Certificate* pToValidate)
{
    (void) (pPKI);
    SOPC_Certificate* cert_ca = NULL;
    CertificateRevList* cert_rev_list = NULL;
    mbedtls_x509_crl* rev_list = NULL;
    uint32_t failure_reasons = 0;

    if (NULL == pPKI || NULL == pToValidate)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pPKI->pFnValidateCertificate || NULL == pPKI->pUserCertAuthList) // TODO: useful pFn verification?
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Gathers certificates from pki structure
    if (NULL != pPKI->pUserCertRevocList)
    {
        cert_rev_list = (CertificateRevList*) (pPKI->pUserCertRevocList);
        rev_list = (mbedtls_x509_crl*) (&cert_rev_list->crl);
    }
    cert_ca = (SOPC_Certificate*) (pPKI->pUserCertAuthList);

    // Now, verifies the certificate
    // crt are not const in crt_verify, but this function does not look like to modify them

    // Assumption that certificate is not modified by mbedtls
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    if (mbedtls_x509_crt_verify_with_profile(
            (mbedtls_x509_crt*) (&pToValidate->crt), (mbedtls_x509_crt*) (&cert_ca->crt), rev_list,
            &mbedtls_x509_crt_profile_minimal, NULL, /* You can specify an expected Common Name here */
            &failure_reasons, NULL, NULL) != 0)
    {
        // TODO: you could further analyze here...
        return SOPC_STATUS_NOK;
    }
    SOPC_GCC_DIAGNOSTIC_RESTORE

    return SOPC_STATUS_OK;
}

static void PKIProviderStack_Free(SOPC_PKIProvider* pPKI)
{
    if (pPKI == NULL)
    {
        return;
    }

    SOPC_KeyManager_Certificate_Free(pPKI->pUserCertAuthList);
    free(pPKI);
}

SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               struct SOPC_CertificateRevList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI)
{
    SOPC_PKIProvider* pki = NULL;
    SOPC_Certificate* caCert = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == pCertAuth || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pki = SOPC_Malloc(sizeof(SOPC_PKIProvider));
    status = SOPC_KeyManager_SerializedCertificate_Deserialize(pCertAuth, &caCert);

    if (NULL == pki || status != SOPC_STATUS_OK)
    {
        SOPC_KeyManager_Certificate_Free(caCert);
        free(pki);
        return SOPC_STATUS_NOK;
    }

    // The pki function pointer shall be const after this init
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *(SOPC_PKIProvider_Free_Func*) (&pki->pFnFree) = &PKIProviderStack_Free;
    *(SOPC_FnValidateCertificate*) (&pki->pFnValidateCertificate) = &PKIProviderStack_ValidateCertificate;
    SOPC_GCC_DIAGNOSTIC_RESTORE

    pki->pUserCertAuthList = caCert;
    pki->pUserCertRevocList = pRevocationList; // Can be NULL
    pki->pUserData = NULL;
    *ppPKI = pki;

    return SOPC_STATUS_OK;
}
