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

/** \file check_hash_based_crypto.c
 *
 * \brief Cryptographic test suite of the SOPC_HashBasedCrypto API.
 *
 * See check_stack.c for more details.
 */

#include <check.h>
#include <stdio.h>

#include "check_helpers.h"
#include "sopc_hash_based_crypto.h"
#include "sopc_helper_encode.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"

#define MAX_BUFFER_SIZE 200

START_TEST(test_pbkdf2_hmac_sha256)
{
    SOPC_ByteString password;
    SOPC_ByteString salt;
    SOPC_ByteString hashHex;
    SOPC_ByteString* hash = NULL;
    SOPC_HashBasedCrypto_Config* config = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SOPC_ByteString_Initialize(&password);
    SOPC_ByteString_Initialize(&salt);
    SOPC_ByteString_Initialize(&hashHex);

    /* Create the configuration structure */
    status = SOPC_HashBasedCrypto_Config_Create(&config);
    ck_assert_ptr_nonnull(config);

    /*
    Part 11 of [RFC7914] https://www.rfc-editor.org/rfc/rfc7914.txt
    Test Vectors for PBKDF2 with HMAC-SHA-256

    Below is a sequence of octets that illustrate input and output values
    for PBKDF2-HMAC-SHA-256.  The octets are hex encoded and whitespace
    is inserted for readability.  The test vectors below can be used to
    verify the PBKDF2-HMAC-SHA-256 [RFC2898] function.  The password and
    salt strings are passed as sequences of ASCII [RFC20] octets.

    PBKDF2-HMAC-SHA-256 (P="passwd", S="salt",
                        c=1, dkLen=64) =
    55 ac 04 6e 56 e3 08 9f ec 16 91 c2 25 44 b6 05
    f9 41 85 21 6d de 04 65 e6 8b 9d 57 c2 0d ac bc
    49 ca 9c cc f1 79 b6 45 99 16 64 b3 9d 77 ef 31
    7c 71 b8 45 b1 e3 0b d5 09 11 20 41 d3 a1 97 83

    PBKDF2-HMAC-SHA-256 (P="Password", S="NaCl",
                            c=80000, dkLen=64) =
    4d dc d8 f6 0b 98 be 21 83 0c ee 5e f2 27 01 f9
    64 1a 44 18 d0 4c 04 14 ae ff 08 87 6b 34 ab 56
    a1 d4 25 a1 22 58 33 54 9a db 84 1b 51 c9 b3 17
    6a 27 2b de bb a1 d0 78 47 8f 62 b3 97 f3 3c 8d
    */

    /*
        test [RFC7914] PBKDF2-HMAC-SHA-256 (P="passwd", S="salt", c=1, dkLen=64)
    */

    password.Data = (SOPC_Byte*) "passwd";
    password.Length = 6;
    salt.Data = (SOPC_Byte*) "salt";
    salt.Length = 4;

    size_t lenHash = 64;
    size_t iteration_count = 1;

    // Set the salt and the counter
    status = SOPC_HashBasedCrypto_Config_PBKDF2(config, &salt, iteration_count, lenHash);
    ck_assert(SOPC_STATUS_OK == status);
    // Hash the password
    status = SOPC_HashBasedCrypto_Run(config, &password, &hash);
    // Check result
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(hash);
    hashHex.Data = SOPC_Malloc(sizeof(SOPC_Byte) * lenHash * 2 + 1);
    status = SOPC_HelperEncode_Hex(hash->Data, (char*) hashHex.Data, lenHash);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(memcmp(hashHex.Data,
                     "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc49ca9cccf179b645991664b39d77ef317"
                     "c71b845b1e30bd509112041d3a19783",
                     2 * lenHash) == 0);

    // Prepare next TC
    SOPC_ByteString_Clear(&hashHex);
    SOPC_ByteString_Delete(hash);

    /*
        test [RFC7914] PBKDF2-HMAC-SHA-256 (P="Password", S="NaCl", c=80000, dkLen=64)
    */

    password.Data = (SOPC_Byte*) "Password";
    password.Length = 8;
    salt.Data = (SOPC_Byte*) "NaCl";
    salt.Length = 4;

    iteration_count = 80000;

    status = SOPC_HashBasedCrypto_Config_PBKDF2(config, &salt, iteration_count, lenHash);
    ck_assert(SOPC_STATUS_OK == status);
    // Hash the password
    status = SOPC_HashBasedCrypto_Run(config, &password, &hash);
    // Check result
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(hash);
    hashHex.Data = SOPC_Malloc(sizeof(SOPC_Byte) * lenHash * 2 + 1); // need '\0' to hexlify
    status = SOPC_HelperEncode_Hex(hash->Data, (char*) hashHex.Data, lenHash);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(memcmp(hashHex.Data,
                     "4ddcd8f60b98be21830cee5ef22701f9641a4418d04c0414aeff08876b34ab56a1d425a1225833549adb841b51c9b3176"
                     "a272bdebba1d078478f62b397f33c8d",
                     2 * lenHash) == 0);

    // Prepare next TC
    SOPC_ByteString_Clear(&hashHex);
    SOPC_ByteString_Delete(hash);

    /*
        To be sure, another test from PyCryptodome, a standalone Python package of low-level cryptographic primitives.
        PBKDF2-HMAC-SHA-256 (P="this_is_a_test", S= 128 bit random value, c=10000, dkLen=32)
    */

    password.Data = (SOPC_Byte*) "this_is_a_test";
    password.Length = 14;
    salt.Length = 16;
    salt.Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) salt.Length);
    ck_assert(SOPC_HelperDecode_Hex("f595e6284725a66b07c3575d9dfa95b9", salt.Data, (size_t) salt.Length) ==
              SOPC_STATUS_OK);
    lenHash = 32;
    iteration_count = 10000;

    status = SOPC_HashBasedCrypto_Config_PBKDF2(config, &salt, iteration_count, lenHash);
    ck_assert(SOPC_STATUS_OK == status);
    // Hash the password
    status = SOPC_HashBasedCrypto_Run(config, &password, &hash);
    // Check result
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(hash);
    hashHex.Data = SOPC_Malloc(sizeof(SOPC_Byte) * lenHash * 2 + 1); // need '\0' to hexlify
    status = SOPC_HelperEncode_Hex(hash->Data, (char*) hashHex.Data, lenHash);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(memcmp(hashHex.Data, "797968c54e66bb8334571fb1b0f2edd014baf19dfb8a423f5352d6c13514f4d8", 2 * lenHash) ==
              0);

    SOPC_ByteString_Clear(&hashHex);
    SOPC_ByteString_Clear(&salt);
    SOPC_ByteString_Delete(hash);

    /* Free the configuration structure */
    SOPC_HashBasedCrypto_Config_Free(config);
}
END_TEST

Suite* tests_make_suite_hash_based_crypto(void)
{
    Suite* s;
    TCase* tc_hash_based_crypto;

    s = suite_create("Hash based crypto tests");
    tc_hash_based_crypto = tcase_create("test of PBKDF2_HMAC_SHA256");
    tcase_add_test(tc_hash_based_crypto, test_pbkdf2_hmac_sha256);

    suite_add_tcase(s, tc_hash_based_crypto);

    return s;
}
