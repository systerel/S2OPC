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

/*
 * An important part of the cases of PKI certificate validation are tested
 * in UACTT, but some cases are not tested. This file contains these last
 * cases.
 */

#include <check.h>

#include "check_helpers.h"
#include "sopc_crypto_profiles.h"
#include "sopc_key_manager.h"
#include "sopc_pki_stack.h"

START_TEST(certificate_validation_one_ca_trusted_only_in_chain)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_CertificateList* cacert = NULL;
    SOPC_CertificateList* int_cli_cacert = NULL;
    SOPC_CRLList* int_cli_cacrl = NULL;
    SOPC_CRLList* cacrl = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t validation_error = 0;

    // Create a profile for the PKI
    status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_SERVER_APP);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Create the certificates and the CRLs of the PKI
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/certs/int_cli_cacert.pem",
                                                             &int_cli_cacert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/crl/int_cli_cacrl.pem", &int_cli_cacrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/certs/cacert.der", &cacert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/crl/cacrl.der", &cacrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Create the PKI
    status = SOPC_PKIProvider_CreateFromList(cacert, cacrl, int_cli_cacert, int_cli_cacrl, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Create the certificate to validate
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/int_client_cert.pem", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // 1st validation: Root CA trusted only
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, pCertToValidate, pProfile, &validation_error);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // 2nd validation: Intermediate CA trusted only
    // Update the PKI
    status = SOPC_PKIProvider_UpdateFromList(pPKI, SOPC_SecurityPolicy_Basic256Sha256_URI, int_cli_cacert,
                                             int_cli_cacrl, cacert, cacrl, 0);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, pCertToValidate, pProfile, &validation_error);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Free
    SOPC_KeyManager_Certificate_Free(cacert);
    SOPC_KeyManager_Certificate_Free(int_cli_cacert);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
    SOPC_KeyManager_CRL_Free(cacrl);
    SOPC_KeyManager_CRL_Free(int_cli_cacrl);
    SOPC_PKIProvider_DeleteProfile(&pProfile);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(certificate_validation_self_signed_ca_without_crl)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_CertificateList* cacert = NULL;
    SOPC_CertificateList* self_signed_ca_pathLen0 = NULL;
    SOPC_CertificateList* self_signed_ca_pathLen1 = NULL;
    SOPC_CertificateList* self_signed_ca_missingPathLen = NULL;
    SOPC_CRLList* cacrl = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t validation_error = 0;

    // Create a profile for the PKI
    status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_SERVER_APP);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Create the certificates and CRLs
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/certs/cacert.der", &cacert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/crl/cacrl.der", &cacrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(
        "./S2OPC_Demo_PKI/trusted/certs/ca_selfsigned_pathLen0.der", &self_signed_ca_pathLen0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(
        "./check_pki_cert_validate_test_data/ca_selfsigned_pathLen1.der", &self_signed_ca_pathLen1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(
        "./check_pki_cert_validate_test_data/ca_selfsigned_missingPathLen.der", &self_signed_ca_missingPathLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // 1st validation: self_signed_ca_pathLen0 is trusted and we want to validate it
    // Create the PKI and validate
    status = SOPC_PKIProvider_CreateFromList(self_signed_ca_pathLen0, NULL, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, self_signed_ca_pathLen0, pProfile, &validation_error);
    // Validation result: must be OK.
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // 2nd validation: self_signed_ca_pathLen0 is issuer and we want to validate it
    // Update the PKI and validate
    // There must be at least one trusted certificate in the PKI (see the PKI function check_lists())
    status = SOPC_PKIProvider_UpdateFromList(pPKI, SOPC_SecurityPolicy_Basic256Sha256_URI, cacert, cacrl,
                                             self_signed_ca_pathLen0, NULL, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, self_signed_ca_pathLen0, pProfile, &validation_error);
    // Validation result: must be NOK cert untrusted
    ck_assert_int_eq(SOPC_STATUS_NOK, status);

    // 3rd validation: self_signed_ca_pathLen1 is trusted and we want to validate it
    // Update the PKI and validate
    status = SOPC_PKIProvider_UpdateFromList(pPKI, SOPC_SecurityPolicy_Basic256Sha256_URI, self_signed_ca_pathLen1,
                                             NULL, NULL, NULL, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, self_signed_ca_pathLen1, pProfile, &validation_error);
    // Validation result: must be NOK cert untrusted
    ck_assert_int_eq(SOPC_STATUS_NOK, status);

    // 4th validation: self_signed_ca_missingPathLen is trusted and we want to validate it
    // Update the PKI and validate
    status = SOPC_PKIProvider_UpdateFromList(pPKI, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                             self_signed_ca_missingPathLen, NULL, NULL, NULL, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, self_signed_ca_missingPathLen, pProfile, &validation_error);
    // Validation result: must be NOK cert untrusted
    ck_assert_int_eq(SOPC_STATUS_NOK, status);

    // Free
    SOPC_KeyManager_Certificate_Free(cacert);
    SOPC_KeyManager_Certificate_Free(self_signed_ca_pathLen0);
    SOPC_KeyManager_Certificate_Free(self_signed_ca_pathLen1);
    SOPC_KeyManager_Certificate_Free(self_signed_ca_missingPathLen);
    SOPC_KeyManager_CRL_Free(cacrl);
    SOPC_PKIProvider_DeleteProfile(&pProfile);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

START_TEST(certificate_validation_crl_not_renewed)
{
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_CertificateList* pCertToValidate = NULL;
    SOPC_CertificateList* cacert = NULL;
    SOPC_CRLList* cacrl_not_renewed = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t validation_error = 0;

    // Create a profile for the PKI
    status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_SERVER_APP);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Create the certificates and the CRLs of the PKI
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/certs/cacert.der", &cacert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./check_pki_cert_validate_test_data/cacrl_not_renewed.der",
                                                     &cacrl_not_renewed);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Create the PKI
    status = SOPC_PKIProvider_CreateFromList(cacert, cacrl_not_renewed, NULL, NULL, &pPKI);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Create the certificate to validate
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Validate. It must fail there since the CRL has not been renewed.
    status = SOPC_PKIProvider_ValidateCertificate(pPKI, pCertToValidate, pProfile, &validation_error);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);

    // Free
    SOPC_KeyManager_Certificate_Free(cacert);
    SOPC_KeyManager_Certificate_Free(pCertToValidate);
    SOPC_KeyManager_CRL_Free(cacrl_not_renewed);
    SOPC_PKIProvider_DeleteProfile(&pProfile);
    SOPC_PKIProvider_Free(&pPKI);
}
END_TEST

Suite* tests_make_suite_pki_cert_validate(void)
{
    Suite* s = suite_create("PKI certificate validation tests");
    TCase* certificate_validation = tcase_create("certificate validation");
    tcase_add_test(certificate_validation, certificate_validation_one_ca_trusted_only_in_chain);
    tcase_add_test(certificate_validation, certificate_validation_self_signed_ca_without_crl);
    tcase_add_test(certificate_validation, certificate_validation_crl_not_renewed);
    suite_add_tcase(s, certificate_validation);

    return s;
}
