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
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdio.h>

#include "check_helpers.h"
#include "sopc_crypto_profiles.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#define S2OPC_DEFAULT_ENDPOINT_URL "opc.tcp://LOCALhost:4841"
#define S2OPC_DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"

START_TEST(invalid_create)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CertificateList* pIssuersCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_CRLList* pIssuersCrl = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/ctt_ca1I_ca2T.der", &pTrustedCerts);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/ctt_ca1I_ca2T.crl", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted_usr/user_cacert.der", &pIssuersCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked_usr/user_cacrl.der", &pIssuersCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* No trusted certificate is provided */
    status = SOPC_PKIProvider_CreateFromList(NULL, pTrustedCrl, pIssuersCerts, pIssuersCrl, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* Only intermediate CA is provided for trustedCerts */
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* Trusted CA certificates is provided but no CRL */
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, NULL, pIssuersCerts, pIssuersCrl, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* Issuer CA certificate is provided but no CRL */
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* Not all issuer certificates are CAs */
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pIssuersCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* No certificates */
    status = SOPC_PKIProvider_CreateFromList(NULL, NULL, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* Invalid PKI */
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(pPKI);
    /* invalid store path */
    status = SOPC_PKIProvider_CreateFromStore("./path_does_not_exist", &pPKI);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pIssuersCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pIssuersCrl);
}
END_TEST

START_TEST(invalid_write)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pPKI);
    /* Directory store path is not defined */
    status = SOPC_PKIProvider_WriteToStore(pPKI, true);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    /* Invalid directory store path  */
    status = SOPC_PKIProvider_SetStorePath("invalid/not_exist", pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_PKIProvider_WriteToStore(pPKI, true);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(cert_invalid_application_uri)
{
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetURI(pLeafProfile, "invalid_uri");
    ck_assert_ptr_nonnull(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_UriInvalid, err);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_valid_application_uri)
{
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetURI(pLeafProfile, S2OPC_DEFAULT_APPLICATION_URI);
    ck_assert_ptr_nonnull(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_invalid_hostName)
{
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetURL(pLeafProfile, "invalid_hostName");
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_nonnull(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_HostNameInvalid, err);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_valid_hostName)
{
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetURL(pLeafProfile, S2OPC_DEFAULT_ENDPOINT_URL);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_nonnull(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_invalid_keyUsage)
{
    /*
       When the PKI type is defined as client or server then the KeyUsage is expected to be set with
       digitalSignature, nonRepudiation, keyEncipherment and dataEncipherment.
    */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./user_public/user_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetUsageFromType(pLeafProfile, SOPC_PKI_TYPE_CLIENT_APP);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert((SOPC_PKI_KU_KEY_ENCIPHERMENT | SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT | SOPC_PKI_KU_DIGITAL_SIGNATURE |
               SOPC_PKI_KU_NON_REPUDIATION) == pLeafProfile->keyUsage);
    pLeafProfile->extendedKeyUsage = SOPC_PKI_EKU_DISABLE_CHECK;
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_UseNotAllowed, err);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_valid_keyUsage)
{
    /*
       When the PKI type is defined as client or server then the KeyUsage is expected to be set with
       digitalSignature, nonRepudiation, keyEncipherment and dataEncipherment.
    */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetUsageFromType(pLeafProfile, SOPC_PKI_TYPE_CLIENT_APP);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq((SOPC_PKI_KU_KEY_ENCIPHERMENT | SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT | SOPC_PKI_KU_DIGITAL_SIGNATURE |
                      SOPC_PKI_KU_NON_REPUDIATION),
                     pLeafProfile->keyUsage);
    pLeafProfile->extendedKeyUsage = SOPC_PKI_EKU_DISABLE_CHECK;
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_invalid_extendedKeyUsage)
{
    /* When the PKI type is defined as server then the extendedKeyUsage is expected to be set with ClientAuth */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetUsageFromType(pLeafProfile, SOPC_PKI_TYPE_SERVER_APP);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_EKU_CLIENT_AUTH, pLeafProfile->extendedKeyUsage);
    pLeafProfile->keyUsage = SOPC_PKI_KU_DISABLE_CHECK;
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_UseNotAllowed, err);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_valid_extendedKeyUsage)
{
    /* When the PKI type is defined as client then the extendedKeyUsage is expected to be set with ServerAuth */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(NULL, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_LeafProfileSetUsageFromType(pLeafProfile, SOPC_PKI_TYPE_CLIENT_APP);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert(!pLeafProfile->bApplySecurityPolicy);
    ck_assert_int_eq(SOPC_PKI_EKU_SERVER_AUTH, pLeafProfile->extendedKeyUsage);
    pLeafProfile->keyUsage = SOPC_PKI_KU_DISABLE_CHECK;
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_invalid_security_policy_mdsig)
{
    /* Expected SH256 but profile is set with SHA1 */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile leafProfile = {.mdSign = SOPC_PKI_MD_SHA1,
                                        .pkAlgo = SOPC_PKI_PK_RSA,
                                        .RSAMinimumKeySize = 2048,
                                        .RSAMaximumKeySize = 4096,
                                        .bApplySecurityPolicy = true,
                                        .keyUsage = SOPC_PKI_KU_DISABLE_CHECK,
                                        .extendedKeyUsage = SOPC_PKI_EKU_DISABLE_CHECK,
                                        .sanApplicationUri = NULL,
                                        .sanURL = NULL};
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, &leafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_PolicyCheckFailed, err);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_invalid_security_policy_keySize)
{
    /* Invalid RSAMinimumKeySize (2048 is nominal for Basic256Sha256) */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile leafProfile = {.mdSign = SOPC_PKI_MD_SHA256,
                                        .pkAlgo = SOPC_PKI_PK_RSA,
                                        .RSAMinimumKeySize = 4000,
                                        .RSAMaximumKeySize = SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits,
                                        .bApplySecurityPolicy = true,
                                        .keyUsage = SOPC_PKI_KU_DISABLE_CHECK,
                                        .extendedKeyUsage = SOPC_PKI_EKU_DISABLE_CHECK,
                                        .sanApplicationUri = NULL,
                                        .sanURL = NULL};
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, &leafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_PolicyCheckFailed, err);
    /* Invalid RSAMaximumKeySize (4096 is nominal for Basic256Sha256) */
    leafProfile.RSAMinimumKeySize = SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits;
    leafProfile.RSAMaximumKeySize = 1000;
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, &leafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_PolicyCheckFailed, err);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(cert_valid_security_policy)
{
    /* Basic256Sha256 security policy */
    uint32_t err = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    status = SOPC_PKIProvider_CreateLeafProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pLeafProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(SOPC_PKI_MD_SHA256, pLeafProfile->mdSign);
    ck_assert_int_eq(SOPC_PKI_PK_RSA, pLeafProfile->pkAlgo);
    ck_assert_uint_le(SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits, pLeafProfile->RSAMaximumKeySize);
    ck_assert_uint_ge(SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits, pLeafProfile->RSAMaximumKeySize);
    ck_assert(pLeafProfile->bApplySecurityPolicy);
    ck_assert_ptr_null(pLeafProfile->sanApplicationUri);
    ck_assert_ptr_null(pLeafProfile->sanURL);
    ck_assert_int_eq(SOPC_PKI_KU_DISABLE_CHECK, pLeafProfile->keyUsage);
    ck_assert_int_eq(SOPC_PKI_EKU_DISABLE_CHECK, pLeafProfile->extendedKeyUsage);
    status = SOPC_PKIProvider_CheckLeafCertificate(pCertToValidate, pLeafProfile, &err);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
}
END_TEST

START_TEST(functional_test_from_list)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/ctt_ca1T.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/ctt_ca1T.crl", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* Validation will failed as expected (missing root cacert.der and its CRL cacrl.der) */
    uint32_t error = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_Profile* pProfile = NULL;
    status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_SERVER_APP);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, pCertToValidate, pProfile, &error);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_int_eq(SOPC_CertificateValidationError_Untrusted, error);
    /* Update the PKI with cacert.der and  cacrl.der */
    SOPC_CertificateList* pTrustedCertToUpdate = NULL;
    SOPC_CRLList* pTrustedCrlToUpdate = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCertToUpdate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrlToUpdate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_UpdateFromList(&pPKI, NULL, pTrustedCertToUpdate, pTrustedCrlToUpdate, NULL, NULL, true);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* Validation is OK */
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, pCertToValidate, pProfile, &error);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_SetStorePath("./unit_test_pki", pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* Write in the file system for the next functional test */
    status = SOPC_PKIProvider_WriteToStore(pPKI, true);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_PKIProvider_DeleteProfile(&pProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
    SOPC_KeyManager_Certificate_Free(pTrustedCertToUpdate);
    SOPC_KeyManager_CRL_Free(pTrustedCrlToUpdate);
    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(functional_test_from_store)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_ReturnStatus status = SOPC_PKIProvider_CreateFromStore("./unit_test_pki", &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    uint32_t error = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_PKI_Profile* pProfile = NULL;
    status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_SERVER_APP);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, pCertToValidate, pProfile, &error);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_PKIProvider_DeleteProfile(&pProfile);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(functional_test_write_to_list)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pPKI);
    /* Extracts the PKI certificates */
    SOPC_CertificateList* pWrittenTrustedCerts = NULL;
    SOPC_CRLList* pWrittenTrustedCrl = NULL;
    SOPC_CertificateList* pWrittenIssuersCerts = NULL;
    SOPC_CRLList* pWrittenIssuersCrl = NULL;
    status = SOPC_PKIProvider_WriteOrAppendToList(pPKI, &pWrittenTrustedCerts, &pWrittenTrustedCrl,
                                                  &pWrittenIssuersCerts, &pWrittenIssuersCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pWrittenTrustedCerts);
    ck_assert_ptr_nonnull(pWrittenTrustedCrl);
    ck_assert_ptr_null(pWrittenIssuersCerts);
    ck_assert_ptr_null(pWrittenIssuersCrl);
    /* Compares TrustedCerts list with the original */
    size_t nbCerts = 0;
    bool findCert = false;
    status = SOPC_KeyManager_Certificate_GetListLength(pWrittenTrustedCerts, &nbCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(1, nbCerts);
    status = SOPC_KeyManager_CertificateList_FindCertInList(pTrustedCerts, pWrittenTrustedCerts, &findCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(findCert);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_Certificate_Free(pWrittenTrustedCerts);
    SOPC_KeyManager_CRL_Free(pWrittenTrustedCrl);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(functional_test_append_to_list)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pPKI);

    SOPC_CertificateList* pOriginalCert = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pOriginalCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* Extracts the PKI certificates */
    SOPC_CertificateList* pAppendTrustedCerts = NULL;
    status = SOPC_KeyManager_Certificate_Copy(pOriginalCert, &pAppendTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_CRLList* pAppendTrustedCrl = NULL;
    SOPC_CertificateList* pAppendIssuersCerts = NULL;
    SOPC_CRLList* pAppendIssuersCrl = NULL;
    status = SOPC_PKIProvider_WriteOrAppendToList(pPKI, &pAppendTrustedCerts, &pAppendTrustedCrl, &pAppendIssuersCerts,
                                                  &pAppendIssuersCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pAppendTrustedCerts);
    ck_assert_ptr_nonnull(pAppendTrustedCrl);
    ck_assert_ptr_null(pAppendIssuersCerts);
    ck_assert_ptr_null(pAppendIssuersCrl);
    /* Compares */
    size_t nbCerts = 0;
    bool findCert = false;
    status = SOPC_KeyManager_Certificate_GetListLength(pAppendTrustedCerts, &nbCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(2, nbCerts);
    status = SOPC_KeyManager_CertificateList_FindCertInList(pAppendTrustedCerts, pTrustedCerts, &findCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(findCert);
    findCert = false;
    status = SOPC_KeyManager_CertificateList_FindCertInList(pAppendTrustedCerts, pOriginalCert, &findCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(findCert);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_Certificate_Free(pOriginalCert);
    SOPC_KeyManager_Certificate_Free(pAppendTrustedCerts);
    SOPC_KeyManager_CRL_Free(pAppendTrustedCrl);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(functional_test_pki_permissive)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_ReturnStatus status = SOPC_PKIPermissive_Create(&pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* Validate anything */
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, NULL, NULL, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* Disable SetStorePath */
    status = SOPC_PKIProvider_SetStorePath("./unit_test_pki", pPKI);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    /* Disable WriteToStore */
    status = SOPC_PKIProvider_WriteToStore(pPKI, true);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    /* Disable WriteOrAppendToList */
    SOPC_CertificateList* tCrt = NULL;
    SOPC_CertificateList* iCrt = NULL;
    SOPC_CRLList* tCrl = NULL;
    SOPC_CRLList* iCrl = NULL;
    status = SOPC_PKIProvider_WriteOrAppendToList(pPKI, &tCrt, &tCrl, &iCrt, &iCrl);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    /* Disable UpdateFromList */
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &tCrt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &tCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_UpdateFromList(&pPKI, NULL, tCrt, tCrl, NULL, NULL, true);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    SOPC_KeyManager_Certificate_Free(tCrt);
    SOPC_KeyManager_CRL_Free(tCrl);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(functional_test_verify_every_cert)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CertificateList* pTrustedCertToUpdate = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pTrustedCerts);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pPKI);
    uint32_t* pErrors = NULL;
    char** pThumbprints = NULL;
    uint32_t nbError = 0;
    const SOPC_PKI_ChainProfile profile = {.curves = SOPC_PKI_CURVES_ANY,
                                           .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                           .pkAlgo = SOPC_PKI_PK_ANY,
                                           .RSAMinimumKeySize = 2048};
    status = SOPC_PKIProvider_VerifyEveryCertificate(pPKI, &profile, &pErrors, &pThumbprints, &nbError);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(0, nbError);
    ck_assert_ptr_null(pErrors);
    ck_assert_ptr_null(pThumbprints);

    /* Updating without including the trusted root cacert.der */
    status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pTrustedCertToUpdate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_UpdateFromList(&pPKI, NULL, pTrustedCertToUpdate, pTrustedCrl, NULL, NULL, false);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    /* client_2k_cert is now invalid */
    status = SOPC_PKIProvider_VerifyEveryCertificate(pPKI, &profile, &pErrors, &pThumbprints, &nbError);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    ck_assert_uint_eq(1, nbError);
    ck_assert_ptr_nonnull(pErrors);
    ck_assert_ptr_nonnull(pThumbprints);

    char* crtThumb = pThumbprints[0];
    uint32_t crtErr = pErrors[0];
    ck_assert_int_eq(SOPC_CertificateValidationError_Untrusted, crtErr);
    int cmp = memcmp(crtThumb, "F4754CB40785156E074CF96F59D8378DF1FB7EF3", 40);
    ck_assert_int_eq(0, cmp);

    for (uint32_t i = 0; i < nbError; i++)
    {
        SOPC_Free(pThumbprints[i]);
    }
    SOPC_Free(pThumbprints);
    SOPC_Free(pErrors);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pTrustedCertToUpdate);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

Suite* tests_make_suite_pki(void)
{
    Suite* s;
    TCase *invalid, *certificate_properties, *functional;

    s = suite_create("PKI API test");
    invalid = tcase_create("invalid");
    tcase_add_test(invalid, invalid_create);
    tcase_add_test(invalid, invalid_write);
    suite_add_tcase(s, invalid);

    certificate_properties = tcase_create("certificate properties");
    tcase_add_test(certificate_properties, cert_invalid_application_uri);
    tcase_add_test(certificate_properties, cert_valid_application_uri);
    tcase_add_test(certificate_properties, cert_invalid_hostName);
    tcase_add_test(certificate_properties, cert_valid_hostName);
    tcase_add_test(certificate_properties, cert_invalid_keyUsage);
    tcase_add_test(certificate_properties, cert_valid_keyUsage);
    tcase_add_test(certificate_properties, cert_invalid_extendedKeyUsage);
    tcase_add_test(certificate_properties, cert_valid_extendedKeyUsage);
    tcase_add_test(certificate_properties, cert_invalid_security_policy_mdsig);
    tcase_add_test(certificate_properties, cert_invalid_security_policy_keySize);
    tcase_add_test(certificate_properties, cert_valid_security_policy);
    tcase_add_test(certificate_properties, cert_valid_extendedKeyUsage);
    suite_add_tcase(s, certificate_properties);

    functional = tcase_create("functional");
    tcase_add_test(functional, functional_test_from_list);
    tcase_add_test(functional, functional_test_from_store);
    tcase_add_test(functional, functional_test_write_to_list);
    tcase_add_test(functional, functional_test_append_to_list);
    tcase_add_test(functional, functional_test_pki_permissive);
    tcase_add_test(functional, functional_test_verify_every_cert);
    suite_add_tcase(s, functional);

    return s;
}
