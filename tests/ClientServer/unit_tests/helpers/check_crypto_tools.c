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

Suite* tests_make_suite_crypto_tools(void)
{
    Suite* s = suite_create("Crypto tools test");
    TCase* tc_check_app_uri = tcase_create("Check application URI");
    tcase_add_test(tc_check_app_uri, test_crypto_check_app_uri);
    tcase_add_test(tc_check_app_uri, test_crypto_get_app_uri);
    suite_add_tcase(s, tc_check_app_uri);

    return s;
}
