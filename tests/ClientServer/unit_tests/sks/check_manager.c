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

#include "check_sks.h"

#include <check.h>
#include <inttypes.h>

#include "sopc_crypto_profiles.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_manager.h"

START_TEST(test_default_manager_create)
{
    SOPC_SKManager* skm = SOPC_SKManager_Create();

    ck_assert_uint_eq(0, SOPC_SKManager_Size(skm));

    uint32_t expectedKeyLifetime = 5000;
    SOPC_ReturnStatus res = SOPC_SKManager_SetKeyLifetime(skm, expectedKeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);

    // add policy
    SOPC_String expectedPolicy;
    SOPC_String_Initialize(&expectedPolicy);
    SOPC_String_CopyFromCString(&expectedPolicy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    res = SOPC_SKManager_SetSecurityPolicyUri(skm, &expectedPolicy);
    ck_assert(SOPC_STATUS_OK == res);

    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;
    res = SOPC_SKManager_GetKeys(skm, 0, /* current token id */
                                 &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);

    /* No managed Keys => return empty  */
    ck_assert_ptr_null(SecurityPolicyUri);
    ck_assert_uint_eq(0, FirstTokenId);
    ck_assert_ptr_null(Keys);
    ck_assert_uint_eq(0, NbToken);
    ck_assert_uint_eq(0, TimeToNextKey);
    ck_assert_uint_eq(0, KeyLifetime);

    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);
    SOPC_String_Clear(&expectedPolicy);
    SOPC_SKManager_Clear(skm);
    SOPC_Free(skm);
}
END_TEST

START_TEST(test_default_manager_add)
{
    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;

    SOPC_SKManager* skm = SOPC_SKManager_Create();

    ck_assert_uint_eq(0, SOPC_SKManager_Size(skm));

    uint32_t expectedKeyLifetime = 5000;
    SOPC_ReturnStatus res = SOPC_SKManager_SetKeyLifetime(skm, expectedKeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);

    // add policy
    SOPC_String expectedPolicy;
    SOPC_String_Initialize(&expectedPolicy);
    SOPC_String_CopyFromCString(&expectedPolicy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    res = SOPC_SKManager_SetSecurityPolicyUri(skm, &expectedPolicy);
    ck_assert(SOPC_STATUS_OK == res);

    SOPC_ByteString* expectedKeys = SOPC_Calloc(6, sizeof(SOPC_ByteString));
    SOPC_ByteString_Initialize(&expectedKeys[0]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[0], (const SOPC_Byte*) "Bytes 1", 7));
    SOPC_ByteString_Initialize(&expectedKeys[1]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[1], (const SOPC_Byte*) "Bytes 2", 7));
    SOPC_ByteString_Initialize(&expectedKeys[2]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[2], (const SOPC_Byte*) "Bytes 3", 7));
    SOPC_ByteString_Initialize(&expectedKeys[3]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[3], (const SOPC_Byte*) "Bytes 4", 7));
    SOPC_ByteString_Initialize(&expectedKeys[4]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[4], (const SOPC_Byte*) "Bytes 5", 7));
    SOPC_ByteString_Initialize(&expectedKeys[5]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[5], (const SOPC_Byte*) "Bytes 6", 7));

    uint32_t nbAddedKeys = SOPC_SKManager_AddKeys(skm, expectedKeys, 3);
    ck_assert_uint_eq(3, nbAddedKeys);

    res = SOPC_SKManager_GetKeys(skm, 0, /* current token id */
                                 &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);

    ck_assert_str_eq(SOPC_SecurityPolicy_PubSub_Aes256_URI, SOPC_String_GetRawCString(SecurityPolicyUri));
    ck_assert_uint_eq(1, FirstTokenId); /* First Token Id */
    ck_assert_ptr_nonnull(Keys);
    ck_assert_uint_eq(3, NbToken);
    ck_assert(0 < TimeToNextKey);
    ck_assert(TimeToNextKey <= expectedKeyLifetime);
    ck_assert_uint_eq(expectedKeyLifetime, KeyLifetime);
    for (int i = 0; i < 3; i++)
    {
        ck_assert(SOPC_ByteString_Equal(&expectedKeys[i], &Keys[i]));
    }

    // Clear returned data
    for (int i = 0; i < 3; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);

    // Add  3 new Token
    nbAddedKeys = SOPC_SKManager_AddKeys(skm, expectedKeys + 3, 3);
    ck_assert_uint_eq(3, nbAddedKeys);

    /* Get Keys. Should return 6 Token */
    res = SOPC_SKManager_GetKeys(skm, 0, /* current token id */
                                 &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);
    ck_assert_str_eq(SOPC_SecurityPolicy_PubSub_Aes256_URI, SOPC_String_GetRawCString(SecurityPolicyUri));
    ck_assert_uint_eq(1, FirstTokenId); /* First Token Id */
    ck_assert_ptr_nonnull(Keys);
    ck_assert_uint_eq(6, NbToken);
    ck_assert(0 < TimeToNextKey);
    ck_assert(TimeToNextKey <= expectedKeyLifetime);
    ck_assert_uint_eq(expectedKeyLifetime, KeyLifetime);
    for (int i = 0; i < 6; i++)
    {
        ck_assert(SOPC_ByteString_Equal(&expectedKeys[i], &Keys[i]));
    }

    /* Clear data */
    for (int i = 0; i < 6; i++)
    {
        SOPC_ByteString_Clear(&expectedKeys[i]);
    }

    SOPC_Free(expectedKeys);
    for (int i = 0; i < 6; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);

    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);
    SOPC_String_Clear(&expectedPolicy);
    SOPC_SKManager_Clear(skm);
    SOPC_Free(skm);
}
END_TEST

START_TEST(test_default_manager_setkeys)
{
    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;

    SOPC_SKManager* skm = SOPC_SKManager_Create();

    ck_assert_uint_eq(0, SOPC_SKManager_Size(skm));

    uint32_t expectedKeyLifetime = 5000;
    SOPC_ReturnStatus res = SOPC_SKManager_SetKeyLifetime(skm, expectedKeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);

    // add policy
    SOPC_String expectedPolicy;
    SOPC_String_Initialize(&expectedPolicy);
    SOPC_String_CopyFromCString(&expectedPolicy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    res = SOPC_SKManager_SetSecurityPolicyUri(skm, &expectedPolicy);
    ck_assert(SOPC_STATUS_OK == res);

    SOPC_ByteString* expectedKeys = SOPC_Calloc(6, sizeof(SOPC_ByteString));
    SOPC_ByteString_Initialize(&expectedKeys[0]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[0], (const SOPC_Byte*) "Bytes 1", 7));
    SOPC_ByteString_Initialize(&expectedKeys[1]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[1], (const SOPC_Byte*) "Bytes 2", 7));
    SOPC_ByteString_Initialize(&expectedKeys[2]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[2], (const SOPC_Byte*) "Bytes 3", 7));
    SOPC_ByteString_Initialize(&expectedKeys[3]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[3], (const SOPC_Byte*) "Bytes 4", 7));
    SOPC_ByteString_Initialize(&expectedKeys[4]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[4], (const SOPC_Byte*) "Bytes 5", 7));
    SOPC_ByteString_Initialize(&expectedKeys[5]);
    ck_assert(SOPC_STATUS_OK == SOPC_ByteString_CopyFromBytes(&expectedKeys[5], (const SOPC_Byte*) "Bytes 6", 7));

    /* Set 2 Keys */
    res = SOPC_SKManager_SetKeys(skm, &expectedPolicy, 15, expectedKeys, 2, 6000, 20000);
    ck_assert(SOPC_STATUS_OK == res);

    res = SOPC_SKManager_GetKeys(skm, 0, /* current token id */
                                 &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);

    ck_assert_str_eq(SOPC_SecurityPolicy_PubSub_Aes256_URI, SOPC_String_GetRawCString(SecurityPolicyUri));
    ck_assert_uint_eq(15, FirstTokenId); /* First Token Id */
    ck_assert_ptr_nonnull(Keys);
    ck_assert_uint_eq(2, NbToken);
    ck_assert(0 < TimeToNextKey);
    ck_assert(TimeToNextKey <= 6000);
    ck_assert_uint_eq(20000, KeyLifetime);
    for (int i = 0; i < 2; i++)
    {
        ck_assert(SOPC_ByteString_Equal(&expectedKeys[i], &Keys[i]));
    }

    // Clear returned data
    for (int i = 0; i < 2; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);
    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);

    /* Set 4 Keys */
    res = SOPC_SKManager_SetKeys(skm, &expectedPolicy, 50, expectedKeys + 2, 4, 10000, 50000);
    ck_assert(SOPC_STATUS_OK == res);

    res = SOPC_SKManager_GetKeys(skm, 0, /* current token id */
                                 &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
    ck_assert(SOPC_STATUS_OK == res);
    ck_assert_str_eq(SOPC_SecurityPolicy_PubSub_Aes256_URI, SOPC_String_GetRawCString(SecurityPolicyUri));
    ck_assert_uint_eq(50, FirstTokenId); /* First Token Id */
    ck_assert_ptr_nonnull(Keys);
    ck_assert_uint_eq(4, NbToken);
    ck_assert(0 < TimeToNextKey);
    ck_assert(TimeToNextKey <= 10000);
    ck_assert_uint_eq(KeyLifetime, 50000);
    for (int i = 0; i < 4; i++)
    {
        ck_assert(SOPC_ByteString_Equal(&expectedKeys[i + 2], &Keys[i]));
    }

    /* Clear data */
    for (int i = 0; i < 6; i++)
    {
        SOPC_ByteString_Clear(&expectedKeys[i]);
    }
    SOPC_Free(expectedKeys);

    for (int i = 0; i < 4; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);

    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);
    SOPC_String_Clear(&expectedPolicy);
    SOPC_SKManager_Clear(skm);
    SOPC_Free(skm);
}
END_TEST

Suite* tests_make_suite_manager(void)
{
    Suite* s;
    TCase* tc_manager;

    s = suite_create("SKManager tests");
    tc_manager = tcase_create("SKManager");

    tcase_add_test(tc_manager, test_default_manager_create);
    tcase_add_test(tc_manager, test_default_manager_add);
    tcase_add_test(tc_manager, test_default_manager_setkeys);

    suite_add_tcase(s, tc_manager);

    return s;
}
