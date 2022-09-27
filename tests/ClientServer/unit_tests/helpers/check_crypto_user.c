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

/** \file check_crypto_user.c
 *
 * \brief Cryptographic test suite of the SOPC_CryptoUser API.
 *
 * See check_stack.c for more details.
 */

#include <check.h>
#include <stdio.h>

#include "check_helpers.h"
#include "hexlify.h"
#include "sopc_crypto_user.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"

#define MAX_BUFFER_SIZE 200

START_TEST(test_pbkdf2_hmac_sha256)
{
    SOPC_ExposedBuffer password[MAX_BUFFER_SIZE], salt[MAX_BUFFER_SIZE];
    SOPC_ExposedBuffer hashHex[MAX_BUFFER_SIZE];
    SOPC_ExposedBuffer* hash = NULL;
    SOPC_CryptoUser_Ctx* ctx = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

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
    uint32_t lenPassword = 6;
    uint32_t lenSalt = 4;
    uint32_t lenHash = 64;
    uint32_t iteration_count = 1;

    // Context init
    status = SOPC_CryptoUser_Ctx_Create(&ctx, PBKDF2_HMAC_SHA256);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(ctx);
    // Set the salt and the counter
    ck_assert(unhexlify("73616c74", salt, lenSalt) == (int32_t) lenSalt);
    status = SOPC_CryptoUser_Config_PBKDF2(ctx, salt, lenSalt, iteration_count, lenHash);
    ck_assert(SOPC_STATUS_OK == status);
    // Hash the password
    ck_assert(unhexlify("706173737764", password, lenPassword) == (int32_t) lenPassword);
    status = SOPC_CryptoUser_Hash(ctx, password, lenPassword, &hash);
    // Check result
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(hash);
    ck_assert(hexlify(hash, (char*) hashHex, lenHash) == (int32_t) lenHash);
    ck_assert(memcmp(hashHex,
                     "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc49ca9cccf179b645991664b39d77ef317"
                     "c71b845b1e30bd509112041d3a19783",
                     2 * lenHash) == 0);

    // Prepare next TC
    SOPC_Free(hash);
    hash = NULL;

    /*
        test [RFC7914] PBKDF2-HMAC-SHA-256 (P="Password", S="NaCl", c=80000, dkLen=64)
    */

    lenPassword = 8;
    lenSalt = 4;
    lenHash = 64;
    iteration_count = 80000;

    // Set the salt and the counter
    ck_assert(unhexlify("4e61436c", salt, lenSalt) == (int32_t) lenSalt);
    status = SOPC_CryptoUser_Config_PBKDF2(ctx, salt, lenSalt, iteration_count, lenHash);
    ck_assert(SOPC_STATUS_OK == status);
    // Hash the password
    ck_assert(unhexlify("50617373776f7264", password, lenPassword) == (int32_t) lenPassword);
    status = SOPC_CryptoUser_Hash(ctx, password, lenPassword, &hash);
    // Check result
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(hash);
    ck_assert(hexlify(hash, (char*) hashHex, lenHash) == (int32_t) lenHash);
    ck_assert(memcmp(hashHex,
                     "4ddcd8f60b98be21830cee5ef22701f9641a4418d04c0414aeff08876b34ab56a1d425a1225833549adb841b51c9b3176"
                     "a272bdebba1d078478f62b397f33c8d",
                     2 * lenHash) == 0);

    // Prepare next TC
    SOPC_Free(hash);
    hash = NULL;

    /*
        To be sure, another test from PyCryptodome, a standalone Python package of low-level cryptographic primitives.
        PBKDF2-HMAC-SHA-256 (P="2022-my_LonG_pas2word!\@", S= 128 bit random value, c=10000000, dkLen=32)
    */

    lenPassword = 24;
    lenSalt = 16;
    lenHash = 32;
    iteration_count = 10000000;

    // Set the salt and the counter
    ck_assert(unhexlify("f595e6284725a66b07c3575d9dfa95b9", salt, lenSalt) == (int32_t) lenSalt);
    status = SOPC_CryptoUser_Config_PBKDF2(ctx, salt, lenSalt, iteration_count, lenHash);
    ck_assert(SOPC_STATUS_OK == status);
    // Hash the password
    ck_assert(unhexlify("323032322d6d795f4c6f6e475f70617332776f7264215c40", password, lenPassword) ==
              (int32_t) lenPassword);
    status = SOPC_CryptoUser_Hash(ctx, password, lenPassword, &hash);
    // Check result
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert_ptr_nonnull(hash);
    ck_assert(hexlify(hash, (char*) hashHex, lenHash) == (int32_t) lenHash);
    ck_assert(memcmp(hashHex, "05ed6d82d4ef3ccec2a3eda5fb850806fd2734a80152885c34cdb0eaca302688", 2 * lenHash) == 0);

    SOPC_Free(hash);
    SOPC_CryptoUser_Ctx_Free(ctx);
}
END_TEST

Suite* tests_make_suite_crypto_user(void)
{
    Suite* s;
    TCase* tc_crypto_user;

    s = suite_create("Crypto user tests");
    tc_crypto_user = tcase_create("test of PBKDF2_HMAC_SHA256");
    tcase_set_timeout(tc_crypto_user, 20); // Set 20s timeout
    tcase_add_test(tc_crypto_user, test_pbkdf2_hmac_sha256);

    suite_add_tcase(s, tc_crypto_user);

    return s;
}
