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
    crypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
}

static inline void teardown_crypto(void)
{
    SOPC_CryptoProvider_Free(crypto);
    crypto = NULL;
}

START_TEST(test_crypto_load_B256S256)
{
    ck_assert_ptr_null(SOPC_CryptoProvider_GetProfilePubSub(crypto));
    const SOPC_CryptoProfile* profile = SOPC_CryptoProvider_GetProfileServices(crypto);
    ck_assert_ptr_nonnull(profile);

    ck_assert(SOPC_SecurityPolicy_Basic256Sha256_ID == profile->SecurityPolicyID);
    ck_assert(NULL != profile->pFnSymmEncrypt);
    ck_assert(NULL != profile->pFnSymmDecrypt);
    ck_assert(NULL != profile->pFnSymmSign);
    ck_assert(NULL != profile->pFnSymmVerif);
    ck_assert(NULL != profile->pFnGenRnd);
    ck_assert(NULL != profile->pFnDeriveData);
    ck_assert(NULL != profile->pFnAsymEncrypt);
    ck_assert(NULL != profile->pFnAsymDecrypt);
    ck_assert(NULL != profile->pFnAsymSign);
    ck_assert(NULL != profile->pFnAsymVerify);
    ck_assert(NULL != profile->pFnCertVerify);
}
END_TEST

START_TEST(test_crypto_symm_lengths_B256S256)
{
    uint32_t len = 0, lenCiph = 0, lenDeci = 0;

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
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(32 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, NULL) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, NULL) == SOPC_STATUS_OK);
    ck_assert(16 == lenCiph);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, &lenDeci) == SOPC_STATUS_OK);
    ck_assert(16 == lenDeci);
    lenCiph = 0;
    lenDeci = 0;
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, &lenDeci) == SOPC_STATUS_OK);
    ck_assert(16 == lenCiph);
    ck_assert(16 == lenDeci);
}
END_TEST

START_TEST(test_crypto_symm_crypt_B256S256)
{
    // Tests based on the test vectors provided by the NIST
    //  (http://csrc.nist.gov/groups/STM/cavp/block-ciphers.html#aes)
    unsigned char key[32];
    unsigned char iv[16];
    unsigned char input[128];
    unsigned char output[128];
    char hexoutput[256];
    int i;
    SOPC_SecretBuffer *pSecKey = NULL, *pSecIV = NULL;

    // Encrypt
    // This single test is not taken from the NIST test vectors...
    memset(key, 0, sizeof(key));
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "dc95c078a2408989ad48a21492842087", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    ck_assert(unhexlify("c47b0294dbbbee0fec4757f22ffeee3587ca4730c3d33b691df38bab076bc558", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "46f2fb342d6f0ab477476fc501242c5f", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    ck_assert(unhexlify("ccd1bc3c659cd3c59bc437484e3c5c724441da8d6e90ce556cd57d0752663bbc", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "304f81ab61a80c2e743b94d5002a126b", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    memset(key, 0, sizeof(key));
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("0b24af36193ce4665f2825d7b4749c98", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "a9ff75bd7cf6613d3731c77c3b6d0c04", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("256fd73ce35ae3ea9c25dd2a9454493e", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "90b7b9630a2378f53f501ab7beff0391", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    // Decrypt
    ck_assert(unhexlify("28d46cffa158533194214a91e712fc2b45b518076675affd910edeca5f41ac64", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("4bf3b0a69aeb6657794f2901b1440ad4", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    for (i = 0; i < 16; ++i)
        ck_assert(output[i] == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("47cb030da2ab051dfc6c4bf6910d12bb", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    for (i = 0; i < 16; ++i)
        ck_assert(output[i] == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    memset(key, 0, sizeof(key));
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("623a52fcea5d443e48d9181ab32c7421", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "761c1fe41a18acf20d241650611d90f1", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("90b7b9630a2378f53f501ab7beff0391", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "256fd73ce35ae3ea9c25dd2a9454493e", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    // Encrypt + Decrypt
    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "47cb030da2ab051dfc6c4bf6910d12bb", 32) == 0);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, output, 16, pSecKey, pSecIV, input, 16) == SOPC_STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "00000000000000000000000000000000", 32) == 0);
    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);

    // Multi-block messages
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f1"
                        "3bbc11b93bf6f39b3469be14858b5b720d"
                        "4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e"
                        "614197e86a30ecc28df64ccb3ccf5437aa",
                        input, 128) == 128);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 128, pSecKey, pSecIV, output, 128) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput,
                     "90b7b9630a2378f53f501ab7beff039155008071bc8438e789932cfd3eb1299195465e6633849463fdb44375278e2fdb1"
                     "310821e6492cf80ff15cb772509fb42"
                     "6f3aeee27bd4938882fd2ae6b5bd9d91fa4a43b17bb439ebbe59c042310163a82a5fe5388796eee35a181a1271f00be29"
                     "b852d8fa759bad01ff4678f010594cd",
                     256) == 0);
    memset(input, 0, sizeof(input));
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, output, 128, pSecKey, pSecIV, input, 128) == SOPC_STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput,
                     "256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bb"
                     "c11b93bf6f39b3469be14858b5b720d"
                     "4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614"
                     "197e86a30ecc28df64ccb3ccf5437aa",
                     256) == 0);
    // Here we keep the SecretBuffers of key and iv for the following tests

    // Assert failure on wrong parameters
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(NULL, input, 16, pSecKey, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, NULL, 16, pSecKey, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 15, pSecKey, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, NULL, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, NULL, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, NULL, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 15) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(NULL, input, 16, pSecKey, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, NULL, 16, pSecKey, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 15, pSecKey, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, NULL, pSecIV, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, NULL, output, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, NULL, 16) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 15) != SOPC_STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);
}
END_TEST

START_TEST(test_crypto_symm_sign_B256S256)
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

/* This test is the same for security policies that are not None,
 * as its length is not specified by the policy.
 * It should not fail in None, but this is not required, as it is not used.
 */
START_TEST(test_crypto_generate_nbytes_B256S256)
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

START_TEST(test_crypto_generate_nonce_B256S256)
{
    SOPC_SecretBuffer *pSecNonce0, *pSecNonce1;
    const SOPC_ExposedBuffer *pExpKey0, *pExpKey1;

    // It is random, so...
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, &pSecNonce0) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, &pSecNonce1) == SOPC_STATUS_OK);
    pExpKey0 = SOPC_SecretBuffer_Expose(pSecNonce0);
    ck_assert_ptr_nonnull(pExpKey0);
    pExpKey1 = SOPC_SecretBuffer_Expose(pSecNonce1);
    ck_assert_ptr_nonnull(pExpKey1);
    // You have a slight chance to fail here (1/(2**256))
    ck_assert_msg(
        memcmp(pExpKey0, pExpKey1, 32) != 0,
        "Randomly generated two times the same 32 bytes long nonce, which should happen once in pow(2, 256) tries.");
    SOPC_SecretBuffer_Unexpose(pExpKey0, pSecNonce0);
    SOPC_SecretBuffer_Unexpose(pExpKey1, pSecNonce1);
    SOPC_SecretBuffer_DeleteClear(pSecNonce0);
    SOPC_SecretBuffer_DeleteClear(pSecNonce1);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(NULL, &pSecNonce0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_generate_uint32_B256S256)
{
    uint32_t i = 0, j = 0;

    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, &i) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, &j) == SOPC_STATUS_OK);
    // It is random, so you should not have two times the same number (unless you are unlucky (1/2**32)).
    ck_assert_msg(
        i != j,
        "Randomly generated two times the same 4 bytes random ID, which should happen once in pow(2, 32) tries");

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(NULL, &i) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_derive_lengths_B256S256)
{
    uint32_t lenKey = 0, lenKeyBis = 0, lenIV = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == SOPC_STATUS_OK);
    ck_assert(32 == lenKey);
    ck_assert(32 == lenKeyBis);
    ck_assert(16 == lenIV);
}
END_TEST

START_TEST(test_crypto_derive_data_B256S256)
{
    SOPC_ExposedBuffer secret[32], seed[32], output[1024];
    char hexoutput[2048];
    uint32_t lenKey, lenKeyBis, lenIV, lenSecr, lenSeed, lenOutp;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == SOPC_STATUS_OK);
    lenOutp = lenKey + lenKeyBis + lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenSecr) ==
              SOPC_STATUS_OK); // TODO: use future GetLength_Nonce
    lenSeed = lenSecr;

    // This test vectors is unofficial, taken from https://www.ietf.org/mail-archive/web/tls/current/msg03416.html
    ck_assert(unhexlify("9bbe436ba940f017b17652849a71db35", secret, 16) == 16);
    memcpy(seed, "test label",
           10); // We don't use labels in DerivePseudoRandomData, but RFC 5246 specifies that label is prepend to seed
    ck_assert(unhexlify("a0ba9f936cda311827a6f796ffd5198c", seed + 10, 16) == 16);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 26, output, 100) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(
        memcmp(hexoutput,
               "e3f229ba727be17b8d122620557cd453c2aab21d07c3d495329b52d4e61edb5a6b301791e90d35c9c9a46b4e14baf9af0fa0"
               "22f7077def17abfd3797c0564bab4fbc91666e9def9b97fce34f796789baa48082d122ee42c5a72e5a5110fff70187347b66",
               200) == 0);
    // A second call to the same function should reset the contexts and provide the same result
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 26, output, 100) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(
        memcmp(hexoutput,
               "e3f229ba727be17b8d122620557cd453c2aab21d07c3d495329b52d4e61edb5a6b301791e90d35c9c9a46b4e14baf9af0fa0"
               "22f7077def17abfd3797c0564bab4fbc91666e9def9b97fce34f796789baa48082d122ee42c5a72e5a5110fff70187347b66",
               200) == 0);

    // More appropriate examples (generated by the test-writer with a Python implementation that conforms to the
    // previous test vector)
    ck_assert(unhexlify("8bcc1010ba96bc055c1168cf84167410893d6cc4cff090f6ded0eb476b118e17", secret, lenSecr) ==
              (int32_t) lenSecr);
    ck_assert(unhexlify("8c4584155b3df8aba84ede20a3a3778e087f0cf40d850f395b356345b0426614", seed, lenSeed) ==
              (int32_t) lenSeed);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, 64) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 64) == 64);
    ck_assert(memcmp(hexoutput,
                     "5a6cee5d3f4881816d4d5fa890ea9333a0ccb47998efa8c3c1f7e04ffd778b0ab71c5bc89bb418031ae54e34c6ab78a8e"
                     "7a39113d72d7446ff5e54738d9d1d7e",
                     128) == 0);

    ck_assert(unhexlify("6bc8af2863fcc9e7e1d4441d8d87ae0dc42d9f62155bca420703537b05c53756", secret, lenSecr) ==
              (int32_t) lenSecr);
    ck_assert(unhexlify("c33f3f15ae9537c4d1e618dff2260ad0f6757c0201073fc265281e60b939a322", seed, lenSeed) ==
              (int32_t) lenSeed);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, lenOutp) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, lenOutp) == (int32_t) lenOutp);
    ck_assert(memcmp(hexoutput,
                     "ba523f60d02e153670604816cbb25301ce8cc27a04f2be01163f3dd517c2b7f636a08d3ca6ed5811d65a9605efcaf5fd1"
                     "37984ac4a7efc141a181f5dacaac1bd249a8e6424ad5133efd751b2c418160f",
                     2 * lenOutp) == 0);

    ck_assert(unhexlify("d53d3776ecf8540fe1f579f6278f90cec832a19de09c915cd7ccb7bd942377a5", secret, lenSecr) ==
              (int32_t) lenSecr);
    ck_assert(unhexlify("87f48b64bffff0a20efeb62347fa995e574aad63c7371a5dac4b3fe2ae689b65", seed, lenSeed) ==
              (int32_t) lenSeed);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, 1024) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 1024) == 1024);
    ck_assert(memcmp(hexoutput,
                     "addbefdaf4e8c0b14fa7ac19e302bf45e908910ee833975c3328bfa6b7a464c8ced5976887a9fd98b824da473eb88cf1c"
                     "a24c17268da6e6176cc8023c120e1d1"
                     "60f3755cb017b1bb67127dc9ceef24647a313ac230e68cd91b57115c60b55af8bde0c37667291678f2cd874ce2b5a8722"
                     "3b048e43f32319dea5c9d421c29ecb7"
                     "dd15630f5b11feaffd7d4facd49995a6cb1c6d95002045870474868815d13fb9159697b09859e8ca720c50754655bfb1b"
                     "07aea7522af545393cd178e1be80959"
                     "84b353958d6ce3a9da7ef03bd839f19dd400e513ff14ea3f0b6e39d9682d8f3d5f11ff202d3577ac2f5f5cce02b225a83"
                     "9a70a2b6d060527a900377f151f286f"
                     "33a8adfe8b3cc7874e7188ea0a949d9b67de6adecdce269b0e473f14600c800d53bed6e0ee879495668ee10a036fafdf0"
                     "15ea655f870e7d8630e055ac6f6fce2"
                     "e9dfb467439007be895461ef6910db1ccdec32b1f0640a856e961a743358a459b89f60f3089b41607bc0d111512c5a7b1"
                     "bd3ce955254b6393110ccf0f646b26d"
                     "399407fdc9983656b6c9e3cb81c2aae5b73d43d456145e5db3c1d84dddef0da42c1f9f6994b53bd7bb01392fae0882fe2"
                     "05be759d5bbea87f4f2a3d3e0b17585"
                     "7114419a37f4f83bb35ec87a008dddf04300f670818c7ff70d4d5b2de4bcee563b676c05edc4de5f7077d2d92f825e1a2"
                     "03cb9e37b8732e338b4aa188bb00dfb"
                     "fa35223936cee77bfab4e0fee46b23f0397700a81ae257cb144594ee241c5eb5ffde74132b505e7f8f85b6b140b743403"
                     "714f28eb4a1082f2b8536bf068ba0ad"
                     "0b49b873323881a0bba915d678f2548fb7e4424d42ced72d9f7b818b11da8ef1fcd698da53521cfcd02e559f0ae9ee85c"
                     "e046f47ae2215baaf9b08b0ef8733cb"
                     "78a00aae90b8dda614db5e647f690c4d310cd71dbb95c092ab2a2d5d036c1bf4f160c59ce099f185a9c638f09fa1d3332"
                     "8d5827bda760c258f3957d954147324"
                     "e678202b2fe791926f914d099c715f9eec751f526134ec84c4ba26c4fcbafc29b0d5cef8b35d0422c3c8835fe0f11dbbb"
                     "a31970c0c7f6b94767ba9b4b24fa53f"
                     "eda6e2453122a4a7fc28ddc77424740287d34e85971c87ead468496774ad6d6883768f4de4ce30c604395822262d7fabe"
                     "2cf3147f1f95dc603038f7d088140df"
                     "190e143720b19cf65b8a67d725dc1d043f9b2a168bfcef2c4f086f3966aca72b56a308e3a3c2eb7fcf8e6f6804065041a"
                     "e8fd65072a3f7913acde01ff387cde9"
                     "336574782a7dd28baf983e359a1afd6d0513809d1a83ec9f0a2a2a8a8fb1686635e068c4bce4bee77bb817662335a9131"
                     "2920063dbca5a23adb064bc6e3dad65"
                     "c53fe61599241d26c9615562b9456ede80587da078e639a66a160066241d9dabc8bde9c8c16d46ebb3ff5bc2698dd56a9"
                     "a8b924ef20eb0b67fa679f6fd41bdc6",
                     2048) == 0);
}
END_TEST

START_TEST(test_crypto_derive_keysets_B256S256)
{
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32], zeros[32];
    const SOPC_ExposedBuffer* pout;
    char hexoutput[160];
    uint32_t lenKey, lenKeyBis, lenIV, lenCliNonce, lenSerNonce, lenOutp;
    SOPC_SC_SecurityKeySet cliKS, serKS;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == SOPC_STATUS_OK);
    lenOutp = lenKey + lenKeyBis + lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenCliNonce) ==
              SOPC_STATUS_OK); // TODO: use future GetLength_Nonce
    lenSerNonce = lenCliNonce;

    // Prepares security key sets
    memset(zeros, 0, 32);
    cliKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenKey);
    ck_assert_ptr_nonnull(cliKS.signKey);
    cliKS.encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenKey);
    ck_assert_ptr_nonnull(cliKS.encryptKey);
    cliKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV);
    ck_assert_ptr_nonnull(cliKS.initVector);
    serKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenKey);
    ck_assert_ptr_nonnull(serKS.signKey);
    serKS.encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenKey);
    ck_assert_ptr_nonnull(serKS.encryptKey);
    serKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV);
    ck_assert_ptr_nonnull(serKS.initVector);

    // These come from a stub_client working with OPC foundation code (e.g. commit
    // 0fbccc98472c781a7f44ac09c1d36d2b4a0c3fb0)
    ck_assert(unhexlify("3d3b4768f275d5023c2145cbe3a4a592fb843643d791f7bd7fce75ff25128b68", clientNonce, lenCliNonce) ==
              (int32_t) lenCliNonce);
    ck_assert(unhexlify("ccee418cbc77c2ebb38d5ffac9d2a9d0a6821fa211798e71b2d65b3abb6aec8f", serverNonce, lenSerNonce) ==
              (int32_t) lenSerNonce);
    ck_assert(SOPC_CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS,
                                                &serKS) == SOPC_STATUS_OK);
    // 4 lines for each assert
    pout = SOPC_SecretBuffer_Expose(cliKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "86842427475799fa782efa5c63f5eb6f0b6dbf8a549dd5452247feaa5021714b", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.signKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "d8de10ac4fb579f2718ddcb50ea68d1851c76644b26454e3f9339958d23429d5", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "4167de62880e0bdc023aa133965c34ff", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.initVector);
    pout = SOPC_SecretBuffer_Expose(serKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "f6db2ad48ad3776f83086b47e9f905ee00193f87e85ccde0c3bf7eb8650e236e", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.signKey);
    pout = SOPC_SecretBuffer_Expose(serKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "2c86aecfd5629ee05c49345bce3b2a7ca959a0bf4c9c281b8516a369650dbc4e", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(serKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "39a4f596bcbb99e0b48114f60fc6af21", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.initVector);

    // Another run, just to be sure...
    ck_assert(unhexlify("d821ea93a6a48a4ef49b36c5e7d1bae6c49ccb2b2ddb07c99dcf046e2225617f", clientNonce, lenCliNonce) ==
              (int32_t) lenCliNonce);
    ck_assert(unhexlify("00a8cb99446410a70bf221d5c498d0d0b3e968a306f1a4dc5d1acbe7a37644da", serverNonce, lenSerNonce) ==
              (int32_t) lenSerNonce);
    ck_assert(SOPC_CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS,
                                                &serKS) == SOPC_STATUS_OK);
    // 4 lines for each assert
    pout = SOPC_SecretBuffer_Expose(cliKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "185e860da28d3a224729926ba5b5b800214b2f74257ed39e694596520e67e574", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.signKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "7a6c2cdc20a842a0e2039075935b14a07f578c157091328adc9d52bbb8ef727d", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "dcf97c356f5ef87b7049900f74355c13", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.initVector);
    pout = SOPC_SecretBuffer_Expose(serKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "105b1805ecc3a25de8e2eaa5c9e94504b355990243c6163c2c8b95c1f5681694", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.signKey);
    pout = SOPC_SecretBuffer_Expose(serKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t) lenKey);
    ck_assert(memcmp(hexoutput, "2439bdd8fc365b0fe7b7e2cfcefee67ea7bdea6c157d0b23092f0abc015792d5", 2 * lenKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(serKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "005a70781b43979940c77368677718cd", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.initVector);

    // Clears KS
    SOPC_SecretBuffer_DeleteClear(cliKS.signKey);
    SOPC_SecretBuffer_DeleteClear(cliKS.encryptKey);
    SOPC_SecretBuffer_DeleteClear(cliKS.initVector);
    SOPC_SecretBuffer_DeleteClear(serKS.signKey);
    SOPC_SecretBuffer_DeleteClear(serKS.encryptKey);
    SOPC_SecretBuffer_DeleteClear(serKS.initVector);
}
END_TEST

// Fixture for certificate load
static SOPC_CertificateList* crt_pub = NULL;

static inline void setup_certificate(void)
{
    setup_crypto();
    crt_pub = SOPC_UnhexlifyCertificate(SRV_CRT);
}

static inline void teardown_certificate(void)
{
    SOPC_KeyManager_Certificate_Free(crt_pub);
    crt_pub = NULL;

    teardown_crypto();
}

START_TEST(test_cert_load_B256S256)
{
    ;
}
END_TEST

START_TEST(test_cert_lengths_B256S256)
{
    uint32_t len = 0;

    ck_assert(SOPC_CryptoProvider_CertificateGetLength_Thumbprint(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(20 == len); // SHA-1
}
END_TEST

START_TEST(test_cert_thumbprint_B256S256)
{
    uint8_t thumb[20];
    char hexoutput[40];

    // Compute thumbprint
    ck_assert(SOPC_KeyManager_Certificate_GetThumbprint(crypto, crt_pub, thumb, 20) == SOPC_STATUS_OK);
    ck_assert(hexlify(thumb, hexoutput, 20) == 20);
    // The expected thumbprint for this certificate was calculated with openssl tool, and mbedtls API.
    ck_assert(memcmp(hexoutput, SRV_CRT_THUMB, strlen(SRV_CRT_THUMB) / 2) == 0);
}
END_TEST

START_TEST(test_cert_loadkey_B256S256)
{
    SOPC_AsymmetricKey* key_pub = NULL;

    // Loads the public key from cert
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(crt_pub, &key_pub) == SOPC_STATUS_OK);

    SOPC_KeyManager_AsymmetricKey_Free(key_pub);
}
END_TEST

// Fixtures for Asymetric crypto
static SOPC_AsymmetricKey *key_pub = NULL, *key_priv = NULL;

// Certificates: these are not the same cert as in setup_certificate. This one was created to also embed the private key
// in the tests. This key is 2048 bits long, which is the smallest possible. Note: The signature of the key expires on
// 03/02/2017, which is not a problem, as the key usage is to test asymmetric cryptographic primitives
#define DER_ASYM_PUB_HEXA                                                                                              \
    "3082038b30820273a003020102020900cf163f0b5124ff4c300d06092a864886f70d01010b0500305c310b3009060355040613024652310f" \
    "300d06035504080c064672616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504" \
    "030c12494e474f5043532054657374207375697465301e170d3136313032363136333035345a170d3137303230333136333035345a305c31" \
    "0b3009060355040613024652310f300d06035504080c064672616e6365310c300a06035504070c034169783111300f060355040a0c085379" \
    "73746572656c311b301906035504030c12494e474f504353205465737420737569746530820122300d06092a864886f70d01010105000382" \
    "010f003082010a0282010100cbe0cd29bbcdd824999fc5571122e7540405ac94d0a9b3ab3630ce2cf361d50d9e737ce3f7746959003cbe90" \
    "fc1019dce4797f4a87a05cd83521531e1391cf11f2e49ce6b0f68db31fb91675be4bbd4380920fccf46518ac2bff42085ebc6ca107ecef53" \
    "964e14617aecd75e1f27035c326f1757273047ca4d623bc5b08d278e3a320b964b11116df912bf91e99d3fdb78989e3daa144570647efc4c" \
    "983c4159aecbf99aeb8bdfbf242ac5c43f0092a28aecddb8bdabf4aad7af68ae6bfe6d5cf6cb6e3a6a0c2d33ad3d592514703578d1cead67" \
    "aa2c497600e0b9830ee8671f59f25262d596e4dbfe60ec6f5acb0c4f1cedf6b138fa12fd661b65e537c3539b0203010001a350304e301d06" \
    "03551d0e041604147d33f73c22b315aabd26ecb5a6db8a0bb511fbd0301f0603551d230418301680147d33f73c22b315aabd26ecb5a6db8a" \
    "0bb511fbd0300c0603551d13040530030101ff300d06092a864886f70d01010b05000382010100550cb4f83c4b3567dc7aed66698056a034" \
    "f38685e8227c0c0f00de0d7bd267f728d3b05c6f0fc089163e5654a833fd84cb6e5cc71853483cf09c4804ff862a01e920234578f2d6c8cd" \
    "89008d017dce5d15be8a52396a101d32434d34aef346387216f550b1f932c94072168cdc68ad460d100bce4792c57b87f1a431d2b456698d" \
    "d3c248fc6e2644d446f952255f98e3dcb7e5cd200b46f769d581833c21b08d07c4343973e93bed9a2d66ece5b6083e6e42b3378987339ab0" \
    "1aab362890dbf57dc22e9d86c0cd4edfa43f489d250bc4244542368c8682125645bd610fbf1c60ec5f94bc697284bde3915e9e051bb255ae" \
    "e1685265a487bd1d72c5f49ef621e0"
#define DER_ASYM_PUB_LENG 911
#define DER_ASYM_PRIV_HEXA                                                                                             \
    "308204a40201000282010100cbe0cd29bbcdd824999fc5571122e7540405ac94d0a9b3ab3630ce2cf361d50d9e737ce3f7746959003cbe90" \
    "fc1019dce4797f4a87a05cd83521531e1391cf11f2e49ce6b0f68db31fb91675be4bbd4380920fccf46518ac2bff42085ebc6ca107ecef53" \
    "964e14617aecd75e1f27035c326f1757273047ca4d623bc5b08d278e3a320b964b11116df912bf91e99d3fdb78989e3daa144570647efc4c" \
    "983c4159aecbf99aeb8bdfbf242ac5c43f0092a28aecddb8bdabf4aad7af68ae6bfe6d5cf6cb6e3a6a0c2d33ad3d592514703578d1cead67" \
    "aa2c497600e0b9830ee8671f59f25262d596e4dbfe60ec6f5acb0c4f1cedf6b138fa12fd661b65e537c3539b020301000102820101009c87" \
    "cb5d2868e1733053bfc29a508f052d5561ec9bcc3f3acb8f6b2c8dec66145fbc517e01866a3fbff3e368136f153c485a940597dde28ac937" \
    "fdc5d0c6991231c79e436c48d0005ff1ce31b65a1644d658ce32d0cd31c536be736753bd1d36018cc32f0cee83ad5820b135fd7b099466d0" \
    "6e3e26c365cb07e0ccfd7a10d5f57879f21648083e9997cb1f78a3bd934dd472bafd852458e4fc843e14959d46cc2252e7bb12c0cfaee462" \
    "3196595ce587921c600908e10c2e7257ea99a83c6df5b392220b88a11e3dcaf88c55a1a3ce8222037e19585cf644ccca65c188e7d109c447" \
    "773c9e06cf15e2e2b745b0195d042cb264184d3b711d3e9e7e89858aa96102818100f2c690168005c536c5958a45ada4c1cad84203f961c5" \
    "60d996158d2b184d93f48934a0d46ec0512ee0670c2e49fda8b5de29fad03c3e5da406885a6d9775af2dfd5e61357997f2dbcaa087f79e07" \
    "6e95606904cfeab68185bbb4d2854d8f835e1eb38da5614b944970e8b5e4130262219f69394ede5c16e78112cfb3512b10b102818100d6fb" \
    "d2ed02d9529b4e3a04a27da4659b2968d082cf660c0c4520cb1909084ff77ec38dccc74f924a0db25869855ea95e6c61990837c9a46658ce" \
    "233104b1bd2b9d1c16221561f41116926bd963406f789cea1b730c326bd0e4cf01ebc6e2d047f2bbc591a5bfff19512186fbfcbfe1fa3277" \
    "6163a11bef64a8cd1316ba0a5c0b0281803b53787c771671e5fb8c9a7882816375fd38cc9dd15d9958328bdbae6f46ede3f0ef7269d7129a" \
    "04198434fecec7f4c5549fef919957282ce007cc0941dcd94d24c03e8301ceb6e32cf5e3a407f30afbe7ce6205a8f6a65a16cf8e2e5310c1" \
    "ea6b183781f56fb1b1ecac815e55a2dc7618ed6ebaae2dd4cf07c4a00ad2c7f25102818100c22e052f75024c9de0c380ca30081c8a5095ce" \
    "b8489298d14063456f207c74964cd65f2f16dba57be3f131f065b9c1eb7aa390f11e4ab0868d31ec116b770b31e89fa4d236541a7a90d3c2" \
    "3c416cc302c360a5587e2cd0bb86dfff91323c4dfa9ea1c1eb33363f3963d18fb5ed6e77b3607ff9e45e71f8020881eafafd213c4f028180" \
    "04fbb2f7ca0e8e7f644f40939f9743f8996439a9262442821981268f36fba3e4656fc6e9c69bab8b5f56c7b033bed95eeca96952b3d62edd" \
    "935b80d5187649683196702a0b304e802de7841d6bab06e6877b74bdf2b5e7f2673ac6939c1427fb899a4cb26f656b5621914592f61b10d4" \
    "ff50a4bb360d134d224a780db10f0f97"
#define DER_ASYM_PRIV_LENG 1192
#define DER_ASYM_PUB_KEYONLY_HEXA                                                                                      \
    "30820122300d06092a864886f70d01010105000382010f003082010a0282010100cbe0cd29bbcdd824999fc5571122e7540405ac94d0a9b3" \
    "ab3630ce2cf361d50d9e737ce3f7746959003cbe90fc1019dce4797f4a87a05cd83521531e1391cf11f2e49ce6b0f68db31fb91675be4bbd" \
    "4380920fccf46518ac2bff42085ebc6ca107ecef53964e14617aecd75e1f27035c326f1757273047ca4d623bc5b08d278e3a320b964b1111" \
    "6df912bf91e99d3fdb78989e3daa144570647efc4c983c4159aecbf99aeb8bdfbf242ac5c43f0092a28aecddb8bdabf4aad7af68ae6bfe6d" \
    "5cf6cb6e3a6a0c2d33ad3d592514703578d1cead67aa2c497600e0b9830ee8671f59f25262d596e4dbfe60ec6f5acb0c4f1cedf6b138fa12" \
    "fd661b65e537c3539b0203010001"
#define DER_ASYM_PUB_KEYONLY_LENG 294

static inline void setup_asym_keys(void)
{
    uint8_t der_cert[DER_ASYM_PUB_LENG], der_priv[DER_ASYM_PRIV_LENG];

    setup_crypto();

    // Loads certificate from DER
    ck_assert(unhexlify(DER_ASYM_PUB_HEXA, der_cert, DER_ASYM_PUB_LENG) == DER_ASYM_PUB_LENG);
    ck_assert(SOPC_KeyManager_Certificate_CreateOrAddFromDER(der_cert, DER_ASYM_PUB_LENG, &crt_pub) ==
              SOPC_STATUS_OK); //*/

    // Loads the public key from cert
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(crt_pub, &key_pub) == SOPC_STATUS_OK);

    // Loads the corresponding private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(der_priv, DER_ASYM_PRIV_LENG, false, &key_priv) ==
              SOPC_STATUS_OK);
}

static inline void teardown_asym_keys(void)
{
    SOPC_Free(key_pub);
    key_pub = NULL;
    SOPC_KeyManager_Certificate_Free(crt_pub);
    crt_pub = NULL;
    SOPC_KeyManager_AsymmetricKey_Free(key_priv);
    key_priv = NULL;

    teardown_crypto();
}

START_TEST(test_crypto_asym_load_B256S256)
{
    ;
}
END_TEST

START_TEST(test_crypto_asym_lengths_B256S256)
{
    uint32_t lenPlain = 0, lenCiph = 0, len = 0;

    // Check lengths
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(2048 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(2048 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(256 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(256 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(crypto, key_pub, &lenPlain) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(crypto, key_pub, &lenCiph) == SOPC_STATUS_OK);
    ck_assert(256 == lenCiph);
    ck_assert(214 == lenPlain); // 256 - 2*20 - 2
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_pub, &lenCiph, &lenPlain) == SOPC_STATUS_OK);
    ck_assert(256 == lenCiph);
    ck_assert(214 == lenPlain); // 256 - 2*20 - 2
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_priv, &lenCiph, &lenPlain) == SOPC_STATUS_OK);
    ck_assert(256 == lenCiph);
    ck_assert(214 == lenPlain); // 256 - 2*20 - 2
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 32, &len) == SOPC_STATUS_OK);
    ck_assert(256 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 256, &len) == SOPC_STATUS_OK);
    ck_assert(214 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 856, &len) == SOPC_STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 1024, &len) == SOPC_STATUS_OK);
    ck_assert(856 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(20 == len); // SHA-1
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(256 == len); // One block
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(256 == len); // One block
}
END_TEST

START_TEST(test_crypto_asym_crypt_B256S256)
{
    uint8_t input[856], output[1024], input_bis[856];
    uint32_t len = 0;
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32];
    const char* errorReason = "";

    // Encryption/Decryption
    // a) Single message (< 214)
    memset(input, 0, 856);
    memset(output, 0, 1024);
    strncpy((char*) input, "Test S2OPC Test", 32); // And test padding btw...
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 32, key_pub, output, 256, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 256, key_priv, input_bis, 214, &len,
                                                    &errorReason) == SOPC_STATUS_OK);
    ck_assert(len == 32);
    ck_assert(memcmp(input, input_bis, 32) == 0);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("3d3b4768f275d5023c2145cbe3a4a592fb843643d791f7bd7fce75ff25128b68", clientNonce, 32) == 32);
    ck_assert(unhexlify("ccee418cbc77c2ebb38d5ffac9d2a9d0a6821fa211798e71b2d65b3abb6aec8f", serverNonce, 32) == 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input + 32,
                                                         856 - 32) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 856, key_pub, output, 1024, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 1024, key_priv, input_bis, 856, &len,
                                                    &errorReason) == SOPC_STATUS_OK);
    ck_assert(len == 856);
    ck_assert(memcmp(input, input_bis, 856) == 0);
}
END_TEST

START_TEST(test_crypto_asym_sign_verify_B256S256)
{
    uint8_t input[856], sig[256];
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32];
    const char* errorReason = "";

    // Signature
    // a) Single message (< 214)
    memset(input, 0, 856);
    memset(sig, 0, 256);
    strncpy((char*) input, "Test S2OPC Test", 32); // And test padding btw...
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 32, key_priv, sig, 256, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 32, key_pub, sig, 256, &errorReason) ==
              SOPC_STATUS_OK);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("3d3b4768f275d5023c2145cbe3a4a592fb843643d791f7bd7fce75ff25128b68", clientNonce, 32) == 32);
    ck_assert(unhexlify("ccee418cbc77c2ebb38d5ffac9d2a9d0a6821fa211798e71b2d65b3abb6aec8f", serverNonce, 32) == 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input + 32,
                                                         856 - 32) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 856, key_priv, sig, 256, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 856, key_pub, sig, 256, &errorReason) ==
              SOPC_STATUS_OK);
}
END_TEST

START_TEST(test_crypto_asym_copykey_B256S256)
{
    uint8_t buffer[2048], der_priv[DER_ASYM_PRIV_LENG], der_pub_key[DER_ASYM_PUB_LENG];
    uint32_t lenDER = 0;
    SOPC_AsymmetricKey* pKey = NULL;

    // Copy to DER
    ck_assert(SOPC_KeyManager_AsymmetricKey_ToDER(key_priv, false, buffer, 2048, &lenDER) == SOPC_STATUS_OK);

    // Loads DER of key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);

    // Verifies
    ck_assert(lenDER == DER_ASYM_PRIV_LENG);
    ck_assert(memcmp(buffer, der_priv, DER_ASYM_PRIV_LENG) == 0);

    // Public Key
    // Copy to DER
    ck_assert(SOPC_KeyManager_AsymmetricKey_ToDER(key_pub, true, buffer, 2048, &lenDER) == SOPC_STATUS_OK);

    // The produced DER is the key only, not the whole cert
    ck_assert(unhexlify(DER_ASYM_PUB_KEYONLY_HEXA, der_pub_key, DER_ASYM_PUB_KEYONLY_LENG) ==
              DER_ASYM_PUB_KEYONLY_LENG);

    // Verifies
    ck_assert(lenDER == DER_ASYM_PUB_KEYONLY_LENG);
    ck_assert(memcmp(buffer, der_pub_key, DER_ASYM_PUB_KEYONLY_LENG) == 0);

    // Also verifies that the DER is useable in CreateFromBuffer, even if the result cannot be memcmp-ed.
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(der_pub_key, DER_ASYM_PUB_KEYONLY_LENG, true, &pKey) ==
              SOPC_STATUS_OK);
    SOPC_KeyManager_AsymmetricKey_Free(pKey);
}
END_TEST

START_TEST(test_crypto_asym_uri_B256S256)
{
    int res = strncmp(SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(crypto),
                      SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo,
                      strlen(SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo) + 1);
    ck_assert_int_eq(res, 0);
}
END_TEST

// Fixtures for PKI: server.der certificate and CA
static SOPC_SerializedCertificate* crt_ca = NULL;
static SOPC_CRLList* crl = NULL;
static SOPC_PKIProvider* pki = NULL;

static inline void setup_pki_stack(void)
{
    uint8_t* der_ca = SOPC_Malloc(CA_CRT_LEN);
    ck_assert_ptr_nonnull(der_ca);

    setup_certificate();

    // Loads CA cert which signed server.der. This is trusted/cacert.der.
    ck_assert(unhexlify(CA_CRT, der_ca, CA_CRT_LEN) == (int) (CA_CRT_LEN));
    ck_assert(SOPC_KeyManager_SerializedCertificate_CreateFromDER(der_ca, (uint32_t)(CA_CRT_LEN), &crt_ca) ==
              SOPC_STATUS_OK);

    crl = SOPC_UnhexlifyCRL(CA_CRL);

    // Creates PKI with ca
    ck_assert(SOPC_PKIProviderStack_Create(crt_ca, crl, &pki) == SOPC_STATUS_OK);
    SOPC_Free(der_ca);
}

static inline void teardown_pki_stack(void)
{
    SOPC_PKIProvider_Free(&pki);
    SOPC_KeyManager_SerializedCertificate_Delete(crt_ca);

    teardown_certificate();
}

START_TEST(test_pki_load_B256S256)
{
    ck_assert(NULL != pki->pFnValidateCertificate);
}
END_TEST

START_TEST(test_pki_cert_validation_B256S256)
{
    uint32_t errorStatus;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_Certificate_Validate(crypto, pki, crt_pub, &errorStatus);
    // Checks that the PKI validates our server.pub with our cacert.der
    ck_assert_msg(status == SOPC_STATUS_OK, "Validation failed, is this a \"date\" problem?");
}
END_TEST

START_TEST(test_cert_copyder_B256S256)
{
    uint8_t *buffer0 = NULL, *buffer1 = NULL;
    size_t der_len = strlen(SRV_CRT) / 2;
    uint8_t* der_cert = SOPC_Calloc(der_len, sizeof(uint8_t));
    uint32_t lenAlloc0 = 0, lenAlloc1 = 0;

    // Reference certificate. This is server_public/server_2k.der.
    ck_assert(unhexlify(SRV_CRT, der_cert, der_len) == (int) der_len);

    // Extract 2 copies from loaded certificate
    ck_assert(SOPC_KeyManager_Certificate_ToDER(crt_pub, &buffer0, &lenAlloc0) == SOPC_STATUS_OK);
    ck_assert(SOPC_KeyManager_Certificate_ToDER(crt_pub, &buffer1, &lenAlloc1) == SOPC_STATUS_OK);

    // Both should be identical, and identical to der_cert
    ck_assert(lenAlloc0 == lenAlloc1);
    ck_assert(memcmp(buffer0, buffer1, lenAlloc0) == 0);
    ck_assert(der_len == lenAlloc0);
    ck_assert(memcmp(buffer0, der_cert, lenAlloc0) == 0);

    // Modifying 0 should not modify 1
    ck_assert(buffer0 != buffer1);

    // Clear
    SOPC_Free(buffer0);
    SOPC_Free(buffer1);
    SOPC_Free(der_cert);
}
END_TEST

Suite* tests_make_suite_crypto_B256S256(void)
{
    Suite* s = NULL;
    TCase *tc_crypto_symm = NULL, *tc_providers = NULL, *tc_rands = NULL, *tc_derives = NULL, *tc_km = NULL,
          *tc_crypto_asym = NULL, *tc_pki_stack = NULL;

    s = suite_create("Crypto tests Basic256Sha256");
    tc_crypto_symm = tcase_create("Symmetric Crypto");
    tc_providers = tcase_create("Crypto Provider");
    tc_rands = tcase_create("Random Generation");
    tc_derives = tcase_create("Crypto Data Derivation");
    tc_km = tcase_create("Key Management");
    tc_crypto_asym = tcase_create("Asymmetric Crypto");
    tc_pki_stack = tcase_create("PKI stack");

    suite_add_tcase(s, tc_crypto_symm);
    tcase_add_checked_fixture(tc_crypto_symm, setup_crypto, teardown_crypto);
    tcase_add_test(tc_crypto_symm, test_crypto_load_B256S256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_lengths_B256S256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_crypt_B256S256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_sign_B256S256);

    suite_add_tcase(s, tc_rands);
    tcase_add_checked_fixture(tc_rands, setup_crypto, teardown_crypto);
    tcase_add_test(tc_rands, test_crypto_generate_nbytes_B256S256);
    tcase_add_test(tc_rands, test_crypto_generate_nonce_B256S256);
    tcase_add_test(tc_rands, test_crypto_generate_uint32_B256S256);

    suite_add_tcase(s, tc_providers);
    tcase_add_checked_fixture(tc_providers, setup_crypto, teardown_crypto);

    suite_add_tcase(s, tc_derives);
    tcase_add_checked_fixture(tc_derives, setup_crypto, teardown_crypto);
    tcase_add_test(tc_derives, test_crypto_derive_lengths_B256S256);
    tcase_add_test(tc_derives, test_crypto_derive_data_B256S256);
    tcase_add_test(tc_derives, test_crypto_derive_keysets_B256S256);
    // TODO: derive_keysets_client
    // TODO: derive_keysets_server

    suite_add_tcase(s, tc_km);
    tcase_add_checked_fixture(tc_km, setup_certificate, teardown_certificate);
    tcase_add_test(tc_km, test_cert_load_B256S256);
    tcase_add_test(tc_km, test_cert_lengths_B256S256);
    tcase_add_test(tc_km, test_cert_thumbprint_B256S256);
    tcase_add_test(tc_km, test_cert_loadkey_B256S256);
    tcase_add_test(tc_km, test_cert_copyder_B256S256);

    suite_add_tcase(s, tc_crypto_asym);
    tcase_add_checked_fixture(tc_crypto_asym, setup_asym_keys, teardown_asym_keys);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_load_B256S256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_lengths_B256S256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_crypt_B256S256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_sign_verify_B256S256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_copykey_B256S256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_uri_B256S256);

    suite_add_tcase(s, tc_pki_stack);
    tcase_add_checked_fixture(tc_pki_stack, setup_pki_stack, teardown_pki_stack);
    tcase_add_test(tc_pki_stack, test_pki_load_B256S256);
    tcase_add_test(tc_pki_stack, test_pki_cert_validation_B256S256);

    return s;
}
