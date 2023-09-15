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

#include "check_helpers.h"

#include <check.h>
#include <stdio.h>
#include <string.h>

#include "check_crypto_certificates.h"
#include "hexlify.h"
#include "sopc_key_manager.h"
#include "sopc_mem_alloc.h"

#define PASSWORD "password"
#define CSR_KEY_PATH "./server_private/encrypted_server_2k_key.pem"
#define CSR_PATH "./crypto_tools_csr.der"
#define CSR_SAN_URI "URI:urn:S2OPC:localhost"
#define CSR_SAN_DNS "localhost"
#define CSR_MD "SHA256"

START_TEST(test_crypto_check_app_uri)
{
    SOPC_CertificateList* crt_uri = SOPC_UnhexlifyCertificate(SRV_CRT);
    ck_assert(SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S2OPC:localhost"));
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S1OPC:localhost"));
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S2OPC:localhost-postfix"));
    SOPC_KeyManager_Certificate_Free(crt_uri);

    SOPC_CertificateList* crt_no_uri = SOPC_UnhexlifyCertificate(CA_CRT);
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_no_uri, "urn:S2OPC:localhost"));
    SOPC_KeyManager_Certificate_Free(crt_no_uri);
}
END_TEST

START_TEST(test_crypto_get_app_uri)
{
    char* appUri = NULL;
    size_t len = 0;

    SOPC_CertificateList* crt_uri = SOPC_UnhexlifyCertificate(SRV_CRT);
    ck_assert(SOPC_STATUS_OK == SOPC_KeyManager_Certificate_GetMaybeApplicationUri(crt_uri, &appUri, &len));
    ck_assert(strcmp(appUri, "urn:S2OPC:localhost") == 0);
    SOPC_Free(appUri);
    SOPC_KeyManager_Certificate_Free(crt_uri);

    SOPC_CertificateList* crt_no_uri = SOPC_UnhexlifyCertificate(CA_CRT);
    ck_assert(SOPC_STATUS_OK != SOPC_KeyManager_Certificate_GetMaybeApplicationUri(crt_no_uri, &appUri, &len));
    SOPC_KeyManager_Certificate_Free(crt_no_uri);
}
END_TEST

START_TEST(test_crypto_gen_rsa_export_import)
{
    /* Generate a new RSA key*/
    SOPC_AsymmetricKey* pGenKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerGenKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_GenRSA(4096, &pGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pGenKey, false, &pSerGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Export the new key */
    status = SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGenKey, false, "./crypto_tools_gen_key.pem", NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    /* Import the new key */
    SOPC_SerializedAsymmetricKey* pSerImpKey = NULL;
    SOPC_AsymmetricKey* pImpKey = NULL;
    status = SOPC_KeyManager_AsymmetricKey_CreateFromFile("./crypto_tools_gen_key.pem", &pImpKey, NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pImpKey, false, &pSerImpKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Compare the generated key with the imported one */
    uint32_t genKeyLen = SOPC_SecretBuffer_GetLength(pSerGenKey);
    uint32_t impKeyLen = SOPC_SecretBuffer_GetLength(pSerImpKey);
    ck_assert(genKeyLen == impKeyLen);
    SOPC_ExposedBuffer* pRawGenKey = SOPC_SecretBuffer_ExposeModify(pSerGenKey);
    SOPC_ExposedBuffer* pRawImpKey = SOPC_SecretBuffer_ExposeModify(pSerImpKey);
    ck_assert_ptr_nonnull(pRawGenKey);
    ck_assert_ptr_nonnull(pRawImpKey);
    int match = memcmp(pRawGenKey, pRawImpKey, genKeyLen);
    ck_assert(0 == match);
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pGenKey);
    SOPC_KeyManager_AsymmetricKey_Free(pImpKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGenKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerImpKey);
}
END_TEST

START_TEST(test_crypto_gen_rsa_export_import_public)
{
    /* Generate a new RSA key */
    SOPC_AsymmetricKey* pGenKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerGenPubKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_GenRSA(4096, &pGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pGenKey, true, &pSerGenPubKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Export the new public key */
    status = SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGenKey, true, "./crypto_tools_gen_public_key.pem", NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    /* Import the new public key */
    SOPC_SerializedAsymmetricKey* pSerImpPubKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerImpPubKeyWithHeader = NULL;
    SOPC_ExposedBuffer* pRawImpPubKeyWithHeader = NULL;
    uint32_t impKeyPubWithHeaderLen = 0;
    SOPC_AsymmetricKey* pImpPubKey = NULL;
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd("./crypto_tools_gen_public_key.pem",
                                                                            &pSerImpPubKeyWithHeader, NULL, 0);
    ck_assert(SOPC_STATUS_OK == status);
    impKeyPubWithHeaderLen = SOPC_SecretBuffer_GetLength(pSerImpPubKeyWithHeader);
    pRawImpPubKeyWithHeader = SOPC_SecretBuffer_ExposeModify(pSerImpPubKeyWithHeader);
    ck_assert_ptr_nonnull(pRawImpPubKeyWithHeader);
    status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(pRawImpPubKeyWithHeader, impKeyPubWithHeaderLen, true,
                                                            &pImpPubKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pImpPubKey, true, &pSerImpPubKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Compare the generated public key with the imported one */
    uint32_t genKeyPubLen = SOPC_SecretBuffer_GetLength(pSerGenPubKey);
    uint32_t impKeyPubLen = SOPC_SecretBuffer_GetLength(pSerImpPubKey);
    ck_assert(genKeyPubLen == impKeyPubLen);
    SOPC_ExposedBuffer* pRawGenPubKey = SOPC_SecretBuffer_ExposeModify(pSerGenPubKey);
    SOPC_ExposedBuffer* pRawImpPubKey = SOPC_SecretBuffer_ExposeModify(pSerImpPubKey);
    ck_assert_ptr_nonnull(pRawGenPubKey);
    ck_assert_ptr_nonnull(pRawImpPubKey);
    int match = memcmp(pRawGenPubKey, pRawImpPubKey, genKeyPubLen);
    ck_assert(0 == match);
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pGenKey);
    SOPC_KeyManager_AsymmetricKey_Free(pImpPubKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGenPubKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerImpPubKeyWithHeader);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerImpPubKey);
}
END_TEST

START_TEST(test_crypto_gen_rsa_export_import_encrypted)
{
    /* Generate a new RSA key*/
    SOPC_AsymmetricKey* pGenKey = NULL;
    SOPC_SerializedAsymmetricKey* pSerGenKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_GenRSA(4096, &pGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(pGenKey, false, &pSerGenKey);
    ck_assert(SOPC_STATUS_OK == status);
    /* Export and encrypt the new key */
    size_t pwdLen = strlen(PASSWORD);
    ck_assert(pwdLen < UINT32_MAX);
    status = SOPC_KeyManager_AsymmetricKey_ToPEMFile(pGenKey, false, "./crypto_tools_encrypted_gen_key.pem", PASSWORD,
                                                     (uint32_t) pwdLen);
    ck_assert(SOPC_STATUS_OK == status);
    /* Import and decrypt the new key */
    SOPC_SerializedAsymmetricKey* pSerDecKey = NULL;
    status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd("./crypto_tools_encrypted_gen_key.pem",
                                                                            &pSerDecKey, PASSWORD, (uint32_t) pwdLen);
    ck_assert(SOPC_STATUS_OK == status);
    /* Compare the generated key with the imported one */
    uint32_t genKeyLen = SOPC_SecretBuffer_GetLength(pSerGenKey);
    uint32_t decKeyLen = SOPC_SecretBuffer_GetLength(pSerDecKey);
    ck_assert(genKeyLen == decKeyLen);
    SOPC_ExposedBuffer* pRawGenKey = SOPC_SecretBuffer_ExposeModify(pSerGenKey);
    SOPC_ExposedBuffer* pRawDecKey = SOPC_SecretBuffer_ExposeModify(pSerDecKey);
    ck_assert_ptr_nonnull(pRawGenKey);
    ck_assert_ptr_nonnull(pRawDecKey);
    int match = memcmp(pRawGenKey, pRawDecKey, genKeyLen);
    ck_assert(0 == match);
    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pGenKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerGenKey);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(pSerDecKey);
}
END_TEST

START_TEST(test_gen_csr)
{
    /* The purpose of this test is to generate a CSR file and verify manually
       its content though an external tool like openssl :
       > openssl req -inform der -in <csr_der> -out <csr_pem> -outform pem (convert DER to PEM)
       > openssl req -text -in <csr_pem> -noout -verify (to verify manually the content)
    */
    SOPC_AsymmetricKey* pKey = NULL;
    SOPC_CertificateList* pCert = NULL;
    char* subjectName = NULL;
    uint32_t subjectNameLen = 0;
    SOPC_CSR* pCSR = NULL;
    uint8_t* pDER = NULL;
    uint32_t pLen = 0;
    size_t pwdLen = strlen(PASSWORD);
    ck_assert_uint_gt(UINT32_MAX, pwdLen);
    SOPC_ReturnStatus status =
        SOPC_KeyManager_AsymmetricKey_CreateFromFile(CSR_KEY_PATH, &pKey, PASSWORD, (uint32_t) pwdLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./server_public/server_2k_cert.der", &pCert);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_Certificate_GetSubjectName(pCert, &subjectName, &subjectNameLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(subjectName);
    ck_assert_int_eq('\0', subjectName[subjectNameLen]);
    status = SOPC_KeyManager_CSR_Create(subjectName, true, CSR_MD, CSR_SAN_URI, CSR_SAN_DNS, &pCSR);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_KeyManager_CSR_ToDER(pCSR, pKey, &pDER, &pLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    FILE* fp = NULL;
    fp = fopen(CSR_PATH, "wb");
    ck_assert_ptr_nonnull(fp);
    size_t nb_written = fwrite(pDER, 1, pLen, fp);
    fclose(fp);
    ck_assert(pLen == nb_written);

    SOPC_KeyManager_AsymmetricKey_Free(pKey);
    SOPC_KeyManager_Certificate_Free(pCert);
    SOPC_KeyManager_CSR_Free(pCSR);
    SOPC_Free(subjectName);
    SOPC_Free(pDER);
}
END_TEST

Suite* tests_make_suite_crypto_tools(void)
{
    Suite* s = suite_create("Crypto tools test");
    TCase *tc_check_app_uri = NULL, *tc_gen_rsa = NULL, *tc_gen_csr = NULL;

    tc_check_app_uri = tcase_create("Check application URI");
    tc_gen_rsa = tcase_create("Generate RSA keys");
    tc_gen_csr = tcase_create("Generate CSR");

    suite_add_tcase(s, tc_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_get_app_uri);
    suite_add_tcase(s, tc_gen_rsa);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import_public);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import_encrypted);
    tcase_set_timeout(tc_gen_rsa, 10);
    suite_add_tcase(s, tc_gen_csr);
    tcase_add_test(tc_gen_csr, test_gen_csr);

    return s;
}
