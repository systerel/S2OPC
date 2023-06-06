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
#include <string.h>

#include "check_crypto_certificates.h"
#include "hexlify.h"
#include "sopc_key_manager.h"
#include "sopc_mem_alloc.h"

#define PASSWORD "password"

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

Suite* tests_make_suite_crypto_tools(void)
{
    Suite* s = suite_create("Crypto tools test");
    TCase *tc_check_app_uri = NULL, *tc_gen_rsa = NULL;

    tc_check_app_uri = tcase_create("Check application URI");
    tc_gen_rsa = tcase_create("Generate RSA keys");

    suite_add_tcase(s, tc_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_get_app_uri);
    suite_add_tcase(s, tc_gen_rsa);
    tcase_add_test(tc_gen_rsa, test_crypto_gen_rsa_export_import);

    return s;
}
