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
 * \brief Cryptographic test suite. This suite tests "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256".
 *
 * See check_stack.c for more details.
 */

#include <check.h>
#include <stdio.h>

#include "check_crypto_certificates.h"
#include "check_helpers.h"
#include "hexlify.h"
#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_secret_buffer.h"

// Using fixtures
static SOPC_CryptoProvider* crypto = NULL;

static inline void setup_crypto(void)
{
    crypto = SOPC_CryptoProvider_CreatePubSub(SOPC_SecurityPolicy_PubSub_Aes256_URI);
    ck_assert(NULL != crypto);
}

static inline void teardown_crypto(void)
{
    SOPC_CryptoProvider_Free(crypto);
    crypto = NULL;
}

START_TEST(test_crypto_load_PubSub256)
{
    ck_assert_ptr_null(SOPC_CryptoProvider_GetProfileServices(crypto));
    const SOPC_CryptoProfile_PubSub* profile = SOPC_CryptoProvider_GetProfilePubSub(crypto);
    ck_assert_ptr_nonnull(profile);

    ck_assert(SOPC_SecurityPolicy_PubSub_Aes256_ID == profile->SecurityPolicyID);
    ck_assert(NULL != profile->pFnCrypt);
    ck_assert(NULL != profile->pFnSymmSign);
    ck_assert(NULL != profile->pFnSymmVerif);
    ck_assert(NULL != profile->pFnGenRnd);
}
END_TEST

START_TEST(test_crypto_symm_lengths_PubSub256)
{
    uint32_t len = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(32 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SignKey(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(32 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Signature(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(32 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Encryption(crypto, 15, &len) == SOPC_STATUS_OK);
    ck_assert(15 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Decryption(crypto, 15, &len) == SOPC_STATUS_OK);
    ck_assert(15 == len);
    ck_assert(SOPC_CryptoProvider_PubSubGetLength_KeyNonce(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(4 == len);
    ck_assert(SOPC_CryptoProvider_PubSubGetLength_MessageRandom(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(4 == len);
}
END_TEST

START_TEST(test_crypto_symm_crypt_PubSub256)
{
    /* Testing the AES-256 in CTR is challenging through our interface,
     * as the block counter is always set to 0, according to the PubSub spec */
    unsigned char key[32];
    unsigned char nonce[4];
    unsigned char random[4];
    unsigned char input[128];
    unsigned char output[128];
    char hexoutput[256];
    SOPC_SecretBuffer *pSecKey = NULL, *pSecNonce = NULL;
    uint32_t uSeqNum;

    // Encrypt
    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("3f8679a793b2f2845bc7b796eb8ede23d663a77d145fb297f4859beef7b43025", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(nonce, 0, sizeof(nonce));
    ck_assert(unhexlify("8c764a8a", nonce, 4) == 4);
    pSecNonce = SOPC_SecretBuffer_NewFromExposedBuffer(nonce, sizeof(nonce));
    ck_assert(NULL != pSecNonce);
    memset(random, 0, sizeof(random));
    ck_assert(unhexlify("844af35b", random, 4) == 4);
    uSeqNum = 42;
    memset(input, 0, sizeof(input));
    ck_assert(
        unhexlify(
            "409fd257c571803a36ac2c36469e31601d171c585a3ade3ad2619fc5733027c6a0b91a1013301c57769c8f87cd1a75ac70c2995904"
            "4b86721335e6d9225807a32b4af3e7a89b5d3b04d119b11d1a664677ed0e24892e0f9e94267aaf9fe850bf6cc85d34",
            input, 100) == 100);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 100, pSecKey, pSecNonce, random, 4, uSeqNum, output,
                                              100) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(
        memcmp(hexoutput,
               "8bd92e19311e2188b52fd9177bb3b6cd52306fa9b4026bcd2e0a4bb1f50c008fa231e32661d4ddfcbb11d58a5acf474b4f4bd32"
               "fd58de7904cfe156502739b9eb1a93c311ff5de7335f5b6d6672d79ddd972a0f5a205302367978d64821fb3540c97e0b3",
               200) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecNonce);

    // Decrypt
    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("3f8679a793b2f2845bc7b796eb8ede23d663a77d145fb297f4859beef7b43025", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(nonce, 0, sizeof(nonce));
    ck_assert(unhexlify("8c764a8a", nonce, 4) == 4);
    pSecNonce = SOPC_SecretBuffer_NewFromExposedBuffer(nonce, sizeof(nonce));
    ck_assert(NULL != pSecNonce);
    memset(random, 0, sizeof(random));
    ck_assert(unhexlify("844af35b", random, 4) == 4);
    uSeqNum = 42;
    memset(input, 0, sizeof(input));
    ck_assert(
        unhexlify(
            "8bd92e19311e2188b52fd9177bb3b6cd52306fa9b4026bcd2e0a4bb1f50c008fa231e32661d4ddfcbb11d58a5acf474b4f4bd32fd5"
            "8de7904cfe156502739b9eb1a93c311ff5de7335f5b6d6672d79ddd972a0f5a205302367978d64821fb3540c97e0b3",
            input, 100) == 100);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 100, pSecKey, pSecNonce, random, 4, uSeqNum, output,
                                              100) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(
        memcmp(hexoutput,
               "409fd257c571803a36ac2c36469e31601d171c585a3ade3ad2619fc5733027c6a0b91a1013301c57769c8f87cd1a75ac70c2995"
               "9044b86721335e6d9225807a32b4af3e7a89b5d3b04d119b11d1a664677ed0e24892e0f9e94267aaf9fe850bf6cc85d34",
               200) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecNonce);

    // Encrypt + Decrypt
    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("3f8679a793b2f2845bc7b796eb8ede23d663a77d145fb297f4859beef7b43025", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(nonce, 0, sizeof(nonce));
    ck_assert(unhexlify("8c764a8a", nonce, 4) == 4);
    pSecNonce = SOPC_SecretBuffer_NewFromExposedBuffer(nonce, sizeof(nonce));
    ck_assert(NULL != pSecNonce);
    memset(random, 0, sizeof(random));
    ck_assert(unhexlify("844af35b", random, 4) == 4);
    uSeqNum = 42;
    memset(input, 0, sizeof(input));
    ck_assert(unhexlify("5cc73098fb26543f64fbc0b4d200bf739b1047f7", input, 20) == 20);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output, 20) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 20) == 20);
    ck_assert(memcmp(hexoutput, "9781ccd60f49f58de7783595ef2d38ded4373406", 40) == 0);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, output, 20, pSecKey, pSecNonce, random, 4, uSeqNum, input, 20) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 20) == 20);
    ck_assert(memcmp(hexoutput, "5cc73098fb26543f64fbc0b4d200bf739b1047f7", 40) == 0);
    // Here we keep the SecretBuffers of key and iv for the following tests

    // Assert failure on wrong parameters
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(NULL, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, NULL, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 19, pSecKey, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, NULL, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, pSecKey, NULL, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, pSecKey, pSecNonce, NULL, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, pSecKey, pSecNonce, random, 3, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, NULL, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(crypto, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output, 19) !=
              SOPC_STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecNonce);
}
END_TEST

START_TEST(test_crypto_symm_sign_PubSub256)
{
    unsigned char key[256];
    unsigned char input[256];
    unsigned char output[32];
    char hexoutput[1024];
    SOPC_SecretBuffer* pSecKey = NULL;

    // Test cases of https://tools.ietf.org/html/rfc4231 cannot be used for Basic256Sha256
    // Test cases of http://csrc.nist.gov/groups/STM/cavp/message-authentication.html#hmac cannot be used either, as
    // there is no corresponding key_length=32 and sig_length=32 So this is a test case from an informal source (Python3
    // Crypto module and online) The text is obtained by concatenating sha256 hashes of the strings "InGoPcS" and
    // "iNgOpCs",
    //  and the key is obtained by sha256 hashing "INGOPCS".
    // Python code: HMAC.new(SHA256.new(b"INGOPCS").digest(),
    // SHA256.new(b"InGoPcS").digest()+SHA256.new(b"iNgOpCs").digest(), SHA256).hexdigest()

    memset(input, 0, sizeof(input));
    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("ec7b07fb4f3a6b87ca8cff06ba9e0ec619a34a2d9618dc2a02bde67709ded8b4e7069d582665f23a361324d1f84807"
                        "e30d2227b266c287cc342980d62cb53017",
                        input, 64) == 64);
    ck_assert(unhexlify("7203d5e504eafe00e5dd77519eb640de3bbac660ec781166c4d460362a94c372", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, 32);
    ck_assert(NULL != pSecKey);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 32) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 32) == 32);
    ck_assert(memcmp(hexoutput, "e4185b6d49f06e8b94a552ad950983852ef20b58ee75f2c448fea587728d94db", 64) == 0);

    // Check verify
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == SOPC_STATUS_OK);
    output[1] ^= 0x20; // Change 1 bit
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == SOPC_STATUS_NOK);
    output[1] ^= 0x20; // Revert changed bit
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == SOPC_STATUS_OK);
    output[31] = 0x04; // Change 1 bit in last byte
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == SOPC_STATUS_NOK);

    // Check invalid parameters
    ck_assert(SOPC_CryptoProvider_SymmetricSign(NULL, input, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, NULL, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, NULL, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, NULL, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 0) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 31) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(NULL, input, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, NULL, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, NULL, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, NULL, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 0) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 31) != SOPC_STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
}
END_TEST

START_TEST(test_crypto_generate_nbytes_PubSub256)
{
    SOPC_ExposedBuffer *pExpBuffer0, *pExpBuffer1;

    // It is random, so...
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, &pExpBuffer0) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, &pExpBuffer1) == SOPC_STATUS_OK);
    // You have a slight chance to fail here (1/(2**512))
    ck_assert_msg(memcmp(pExpBuffer0, pExpBuffer1, 64) != 0,
                  "Randomly generated two times the same 64 bytes, which should happen once in pow(2, 512) tries.");
    SOPC_Free(pExpBuffer0);
    SOPC_Free(pExpBuffer1);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(NULL, 64, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 0, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

Suite* tests_make_suite_crypto_PubSub256(void)
{
    Suite* s = NULL;
    TCase *tc_crypto_symm = NULL, *tc_rands = NULL;

    s = suite_create("Crypto tests PubSub-Aes256-CTR");
    tc_crypto_symm = tcase_create("Symmetric Crypto");
    tc_rands = tcase_create("Random Generation");

    suite_add_tcase(s, tc_crypto_symm);
    tcase_add_checked_fixture(tc_crypto_symm, setup_crypto, teardown_crypto);
    tcase_add_test(tc_crypto_symm, test_crypto_load_PubSub256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_lengths_PubSub256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_crypt_PubSub256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_sign_PubSub256);

    suite_add_tcase(s, tc_rands);
    tcase_add_checked_fixture(tc_rands, setup_crypto, teardown_crypto);
    tcase_add_test(tc_rands, test_crypto_generate_nbytes_PubSub256);

    return s;
}
