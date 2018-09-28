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
#include <stdlib.h>

#include "check_crypto_certificates.h"
#include "hexlify.h"
#include "sopc_key_manager.h"

static SOPC_Certificate* unpack_certificate(const char* hex_data)
{
    size_t der_len = strlen(hex_data) / 2;
    uint8_t* der_data = calloc(der_len, sizeof(uint8_t));
    ck_assert_ptr_nonnull(der_data);

    ck_assert(unhexlify(hex_data, der_data, der_len) == (int) der_len);
    SOPC_Certificate* crt = NULL;
    ck_assert(der_len <= SIZE_MAX);
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_KeyManager_Certificate_CreateFromDER(der_data, (uint32_t) der_len, &crt));
    free(der_data);

    return crt;
}

START_TEST(test_crypto_check_app_uri)
{
    SOPC_Certificate* crt_uri = unpack_certificate(SRV_CRT);
    ck_assert(SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S2OPC:localhost"));
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_uri, "urn:S1OPC:localhost"));
    SOPC_KeyManager_Certificate_Free(crt_uri);

    SOPC_Certificate* crt_no_uri = unpack_certificate(CA_CRT);
    ck_assert(!SOPC_KeyManager_Certificate_CheckApplicationUri(crt_no_uri, "urn:S2OPC:localhost"));
    SOPC_KeyManager_Certificate_Free(crt_no_uri);
}
END_TEST

Suite* tests_make_suite_crypto_tools(void)
{
    Suite* s = suite_create("Crypto tools test");
    TCase* tc_check_app_uri = tcase_create("Check application URI");
    tcase_add_test(tc_check_app_uri, test_crypto_check_app_uri);
    suite_add_tcase(s, tc_check_app_uri);

    return s;
}
