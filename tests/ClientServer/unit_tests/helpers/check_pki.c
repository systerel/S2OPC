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
#include "sopc_pki_stack.h"

START_TEST(invalid_create)
{
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(10);
    ck_assert(NULL == pConfig);
    pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    ck_assert(NULL != pConfig);
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CertificateList* pIssuersCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_CRLList* pIssuersCrl = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/ctt_ca1I_ca2T.der", &pTrustedCerts);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/ctt_ca1I_ca2T.crl", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted_usr/user_cacert.der", &pIssuersCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked_usr/user_cacrl.der", &pIssuersCrl);
    ck_assert(SOPC_STATUS_OK == status);

    /* No trusted certificate is provided */
    status = SOPC_PKIProviderNew_CreateFromList(NULL, pTrustedCrl, pIssuersCerts, pIssuersCrl, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* Only intermediate CA is provided for trustedCerts */
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* Trusted CA certificates is provided but no CRL */
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, NULL, pIssuersCerts, pIssuersCrl, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* Issuer CA certificate is provided but no CRL */
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* Not all issuer certificates are CAs */
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pIssuersCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* No certificates */
    status = SOPC_PKIProviderNew_CreateFromList(NULL, NULL, NULL, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* Invalid config */
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, NULL, &pPKI);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* Invalid PKI */
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuersCerts, pIssuersCrl, pConfig, NULL);
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == status);
    ck_assert(NULL == pPKI);
    /* invalid store path */
    status = SOPC_PKIProviderNew_CreateFromStore("./path_does_not_exist", NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_NOK == status);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pIssuersCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pIssuersCrl);
}
END_TEST

START_TEST(invalid_update)
{
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(NULL != pPKI);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_PKIProviderNew_Free(pPKI);
}
END_TEST

START_TEST(invalid_write)
{
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(NULL != pPKI);
    /* Directory store path is not defined */
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == SOPC_PKIProviderNew_WriteToStore(pPKI, true));
    /* Invalid directory store path  */
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == SOPC_PKIProviderNew_SetStorePath("invalid/not_exist", pPKI));
    ck_assert(SOPC_STATUS_INVALID_PARAMETERS == SOPC_PKIProviderNew_WriteToStore(pPKI, true));

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_PKIProviderNew_Free(pPKI);
}
END_TEST

START_TEST(functional_test_from_list)
{
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/ctt_ca1T.der", &pTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/ctt_ca1T.crl", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_OK == status);
    /* Validation will failed as expected (missing root cacert.der and its CRL cacrl.der) */
    uint32_t error = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Profile* pProfile = SOPC_PKIProviderNew_GetProfile(SOPC_SecurityPolicy_Basic256Sha256_URI);
    status = SOPC_PKIProviderNew_ValidateCertificate(pPKI, pCertToValidate, pProfile, &error);
    ck_assert(SOPC_STATUS_NOK == status);
    /* Update the PKI with cacert.der and  cacrl.der */
    SOPC_CertificateList* pTrustedCertToUpdate = NULL;
    SOPC_CRLList* pTrustedCrlToUpdate = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCertToUpdate);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrlToUpdate);
    ck_assert(SOPC_STATUS_OK == status);
    status =
        SOPC_PKIProviderNew_UpdateFromList(&pPKI, NULL, pTrustedCertToUpdate, pTrustedCrlToUpdate, NULL, NULL, true);
    ck_assert(SOPC_STATUS_OK == status);
    /* Validation is OK */
    status = SOPC_PKIProviderNew_ValidateCertificate(pPKI, pCertToValidate, pProfile, &error);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_PKIProviderNew_SetStorePath("./unit_test_pki", pPKI);
    ck_assert(SOPC_STATUS_OK == status);
    /* Write in the file system for the next functional test */
    status = SOPC_PKIProviderNew_WriteToStore(pPKI, true);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_KeyManager_Certificate_Free(pCertToValidate);
    SOPC_KeyManager_Certificate_Free(pTrustedCertToUpdate);
    SOPC_KeyManager_CRL_Free(pTrustedCrlToUpdate);
    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_PKIProviderNew_Free(pPKI);
}
END_TEST

START_TEST(functional_test_from_store)
{
    SOPC_PKIProviderNew* pPKI = NULL;
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    SOPC_ReturnStatus status = SOPC_PKIProviderNew_CreateFromStore("./unit_test_pki", NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_OK == status);

    uint32_t error = 0;
    SOPC_CertificateList* pCertToValidate = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pCertToValidate);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Profile* pProfile = SOPC_PKIProviderNew_GetProfile(SOPC_SecurityPolicy_Basic256Sha256_URI);
    status = SOPC_PKIProviderNew_ValidateCertificate(pPKI, pCertToValidate, pProfile, &error);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_KeyManager_Certificate_Free(pCertToValidate);
    SOPC_PKIProviderNew_Free(pPKI);
}
END_TEST

START_TEST(functional_test_write_to_list)
{
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(NULL != pPKI);
    /* Extracts the PKI certificates */
    SOPC_CertificateList* pWrittenTrustedCerts = NULL;
    SOPC_CRLList* pWrittenTrustedCrl = NULL;
    SOPC_CertificateList* pWrittenIssuersCerts = NULL;
    SOPC_CRLList* pWrittenIssuersCrl = NULL;
    status = SOPC_PKIProviderNew_WriteOrAppendToList(pPKI, &pWrittenTrustedCerts, &pWrittenTrustedCrl,
                                                     &pWrittenIssuersCerts, &pWrittenIssuersCrl);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(NULL != pWrittenTrustedCerts);
    ck_assert(NULL != pWrittenTrustedCrl);
    ck_assert(NULL == pWrittenIssuersCerts);
    ck_assert(NULL == pWrittenIssuersCrl);
    /* Compares TrustedCerts list with the original */
    size_t nbCerts = 0;
    bool findCert = false;
    status = SOPC_KeyManager_Certificate_GetListLength(pWrittenTrustedCerts, &nbCerts);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(1 == nbCerts);
    status = SOPC_KeyManager_CertificateList_FindCertInList(pTrustedCerts, pWrittenTrustedCerts, &findCert);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(true == findCert);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_Certificate_Free(pWrittenTrustedCerts);
    SOPC_KeyManager_CRL_Free(pWrittenTrustedCrl);
    SOPC_PKIProviderNew_Free(pPKI);
}
END_TEST

START_TEST(functional_test_append_to_list)
{
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedCerts = NULL;
    SOPC_CRLList* pTrustedCrl = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./trusted/cacert.der", &pTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./revoked/cacrl.der", &pTrustedCrl);
    ck_assert(SOPC_STATUS_OK == status);
    const SOPC_PKI_Config* pConfig = SOPC_PKIProviderNew_GetConfig(SOPC_PKI_TYPE_SERVER_APP);
    status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, NULL, NULL, pConfig, &pPKI);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(NULL != pPKI);

    SOPC_CertificateList* pOriginalCert = NULL;
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./client_public/client_2k_cert.der", &pOriginalCert);
    ck_assert(SOPC_STATUS_OK == status);
    /* Extracts the PKI certificates */
    SOPC_CertificateList* pAppendTrustedCerts = NULL;
    status = SOPC_KeyManager_Certificate_Copy(pOriginalCert, &pAppendTrustedCerts);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_CRLList* pAppendTrustedCrl = NULL;
    SOPC_CertificateList* pAppendIssuersCerts = NULL;
    SOPC_CRLList* pAppendIssuersCrl = NULL;
    status = SOPC_PKIProviderNew_WriteOrAppendToList(pPKI, &pAppendTrustedCerts, &pAppendTrustedCrl,
                                                     &pAppendIssuersCerts, &pAppendIssuersCrl);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(NULL != pAppendTrustedCerts);
    ck_assert(NULL != pAppendTrustedCrl);
    ck_assert(NULL == pAppendIssuersCerts);
    ck_assert(NULL == pAppendIssuersCrl);
    /* Compares */
    size_t nbCerts = 0;
    bool findCert = false;
    status = SOPC_KeyManager_Certificate_GetListLength(pAppendTrustedCerts, &nbCerts);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(2 == nbCerts);
    status = SOPC_KeyManager_CertificateList_FindCertInList(pAppendTrustedCerts, pTrustedCerts, &findCert);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(true == findCert);
    findCert = false;
    status = SOPC_KeyManager_CertificateList_FindCertInList(pAppendTrustedCerts, pOriginalCert, &findCert);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(true == findCert);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_Certificate_Free(pOriginalCert);
    SOPC_KeyManager_Certificate_Free(pAppendTrustedCerts);
    SOPC_KeyManager_CRL_Free(pAppendTrustedCrl);
    SOPC_PKIProviderNew_Free(pPKI);
}
END_TEST

Suite* tests_make_suite_pki(void)
{
    Suite* s;
    TCase *invalid, *functional;

    s = suite_create("PKI API test");
    invalid = tcase_create("invalid");
    tcase_add_test(invalid, invalid_create);
    tcase_add_test(invalid, invalid_update);
    tcase_add_test(invalid, invalid_write);
    suite_add_tcase(s, invalid);

    functional = tcase_create("functional");
    tcase_add_test(functional, functional_test_from_list);
    tcase_add_test(functional, functional_test_from_store);
    tcase_add_test(functional, functional_test_write_to_list);
    tcase_add_test(functional, functional_test_append_to_list);
    suite_add_tcase(s, functional);

    return s;
}
