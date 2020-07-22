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
 * \brief Cryptographic test suite. This suite tests "http://opcfoundation.org/UA/SecurityPolicy#Basic256".
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
    crypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_Basic256_URI);
    ck_assert(NULL != crypto);
}

static inline void teardown_crypto(void)
{
    SOPC_CryptoProvider_Free(crypto);
    crypto = NULL;
}

START_TEST(test_crypto_load_B256)
{
    ck_assert_ptr_null(SOPC_CryptoProvider_GetProfilePubSub(crypto));
    const SOPC_CryptoProfile* profile = SOPC_CryptoProvider_GetProfileServices(crypto);
    ck_assert_ptr_nonnull(profile);

    ck_assert(SOPC_SecurityPolicy_Basic256_ID == profile->SecurityPolicyID);
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

START_TEST(test_crypto_symm_lengths_B256)
{
    uint32_t len = 0, lenCiph = 0, lenDeci = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(32 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SignKey(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(24 == len);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Signature(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(20 == len);
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

START_TEST(test_crypto_symm_crypt_B256)
{
    // Same symm encryption as B256S256.
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

START_TEST(test_crypto_symm_sign_B256)
{
    unsigned char key[256];
    unsigned char input[256];
    unsigned char output[32];
    char hexoutput[1024];
    SOPC_SecretBuffer* pSecKey = NULL;

    // Test cases of https://tools.ietf.org/html/rfc4231 cannot be used for Basic256
    // Test cases of http://csrc.nist.gov/groups/STM/cavp/message-authentication.html#hmac cannot be used either, as
    // there is no corresponding key_length=24 and sig_length=20 So this is a test case from an informal source (Python3
    // Crypto module and online) The text is obtained by concatenating sha256 hashes of the strings "InGoPcS" and
    // "iNgOpCs",
    //  and the key is obtained by taking the first 24 bytes of the sha256 hash of "INGOPCS".
    // Python code: HMAC.new(SHA256.new(b"INGOPCS").digest()[:24],
    // SHA256.new(b"InGoPcS").digest()+SHA256.new(b"iNgOpCs").digest(), SHA).hexdigest()
    memset(input, 0, sizeof(input));
    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("ec7b07fb4f3a6b87ca8cff06ba9e0ec619a34a2d9618dc2a02bde67709ded8b4e7069d582665f23a361324d1f84807"
                        "e30d2227b266c287cc342980d62cb53017",
                        input, 64) == 64);
    ck_assert(unhexlify("7203d5e504eafe00e5dd77519eb640de3bbac660ec781166", key, 24) == 24);
    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer(key, 24);
    ck_assert(NULL != pSecKey);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 20) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 20) == 20);
    ck_assert(memcmp(hexoutput, "2b0aee11aa84cb5ddae7b5ef9c46c4a249e5b981", 40) == 0);

    // Check verify
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 20) == SOPC_STATUS_OK);
    output[1] ^= 0x20; // Change 1 bit
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 20) == SOPC_STATUS_NOK);
    output[1] ^= 0x20; // Revert changed bit
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 20) == SOPC_STATUS_OK);
    output[19] = 0x80; // Change 1 bit in last byte
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 20) == SOPC_STATUS_NOK);

    // Check invalid parameters
    ck_assert(SOPC_CryptoProvider_SymmetricSign(NULL, input, 64, pSecKey, output, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, NULL, 64, pSecKey, output, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, NULL, output, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, NULL, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 0) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(NULL, input, 64, pSecKey, output, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, NULL, 64, pSecKey, output, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, NULL, output, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, NULL, 20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 0) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) != SOPC_STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
}
END_TEST

/* This test is the same for security policies that are not None,
 * as its length is not specified by the policy.
 * It should not fail in None, but this is not required, as it is not used.
 */
START_TEST(test_crypto_generate_nbytes_B256)
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

START_TEST(test_crypto_generate_nonce_B256)
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

START_TEST(test_crypto_generate_uint32_B256)
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

START_TEST(test_crypto_derive_lengths_B256)
{
    uint32_t lenCryptoKey = 0, lenSignKey = 0, lenIV = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenCryptoKey, &lenSignKey, &lenIV) == SOPC_STATUS_OK);
    ck_assert(32 == lenCryptoKey);
    ck_assert(24 == lenSignKey);
    ck_assert(16 == lenIV);
}
END_TEST

START_TEST(test_crypto_derive_data_B256)
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

    // This test vectors is unofficial, taken from https://www.ietf.org/mail-archive/web/tls/current/msg03416.html.
    // However, it only covers SHA-2 family... So I used a Python implementation to generate the test vectors...
    ck_assert(unhexlify("9bbe436ba940f017b17652849a71db35", secret, 16) == 16);
    memcpy(seed, "test label",
           10); // We don't use labels in DerivePseudoRandomData, but RFC 5246 specifies that label is prepend to seed
    ck_assert(unhexlify("a0ba9f936cda311827a6f796ffd5198c", seed + 10, 16) == 16);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 26, output, 100) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(
        memcmp(hexoutput,
               "811429c07ba1f6aee5059e6071ff3e69de62e7cd767fd55700042ec2fcd7db6ca3143cf3c78bb929c1ae51f51cdd3804a3bd"
               "642e63c09267c3c97e0916509e0060553688f6ced4f09ce66ad0ead90e81b85e1e9dd32cd68363a073346eb075c1843537fc",
               200) == 0);
    // A second call to the same function should reset the contexts and provide the same result
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 26, output, 100) == SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(
        memcmp(hexoutput,
               "811429c07ba1f6aee5059e6071ff3e69de62e7cd767fd55700042ec2fcd7db6ca3143cf3c78bb929c1ae51f51cdd3804a3bd"
               "642e63c09267c3c97e0916509e0060553688f6ced4f09ce66ad0ead90e81b85e1e9dd32cd68363a073346eb075c1843537fc",
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
                     "bd379d47069bafec0980db941a84e241b0a0a30c7d3048ebcadd4a8bd0f1674c57f97b6b1b3637bba3ca1d9484302189c"
                     "407b9a894ff4e621c6bf74154cde24c",
                     128) == 0);

    ck_assert(unhexlify("6bc8af2863fcc9e7e1d4441d8d87ae0dc42d9f62155bca420703537b05c53756", secret, lenSecr) ==
              (int32_t) lenSecr);
    ck_assert(unhexlify("c33f3f15ae9537c4d1e618dff2260ad0f6757c0201073fc265281e60b939a322", seed, lenSeed) ==
              (int32_t) lenSeed);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, lenOutp) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, lenOutp) == (int32_t) lenOutp);
    ck_assert(memcmp(hexoutput,
                     "bb8ac92ed4c8f7a369cd0de953a94fd742addb1fbd32751754d94e0b6ae5003f72f49b7bec45ced4202bc6bf9db2312c6"
                     "bb510b0287b7be756da6743c6234945c61f773eaf0bbfc9bf3046ef31871122",
                     2 * lenOutp) == 0);

    ck_assert(unhexlify("d53d3776ecf8540fe1f579f6278f90cec832a19de09c915cd7ccb7bd942377a5", secret, lenSecr) ==
              (int32_t) lenSecr);
    ck_assert(unhexlify("87f48b64bffff0a20efeb62347fa995e574aad63c7371a5dac4b3fe2ae689b65", seed, lenSeed) ==
              (int32_t) lenSeed);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, 1024) ==
              SOPC_STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 1024) == 1024);
    ck_assert(memcmp(hexoutput,
                     "8b5a820cc5d986920df2fb505852f039a80fa66bf9043722cafa5357b37755d63360e69caab7925ed80f7b62a74b860b8"
                     "9e9a0a30ed8a4ad557c0d9194a34b1f"
                     "1d91e6b943b8133cdae6900529979f2ca8d38635ee19048922b259da20a446687c01c2574e10c9138d43aaf8014bc8f0b"
                     "7943f86339f725e5b0629bf946cd1b8"
                     "c2be4cd52c4f70af32b014addc6bea03f89c62645c49ee16b5a96ace2072491b0e14e421245afa84d0d27faca248c0842"
                     "905976bbb47745a140b8deffcc9b406"
                     "081926010b4308a3c62943bfef3a7bd251527f4179d7b598462e3d62273c4e9742df88a1c7dea2dc8c4cfc8270d30a227"
                     "c8bf9e44f2308c5e8c5e9ecb691f48e"
                     "a54b26d22f0b6ff9da21b4473e4ec4940403b0d5bd8eb2ea72d61ac3669964e3db5d007ef7991735a175b6a37e734f7c9"
                     "f24fe65d53c4c38af913c2041c3bda3"
                     "0ace967d81092b7af9695dd4c2b8ede9e4b40d299bf02215509691e809cf717efbfa01e0a0dfb5c3394e1dc469fe604ba"
                     "36ba73b87f013df7f49feb960fbf84b"
                     "a24ee4e2281f96de710d08c5c279b838a90cb8af2c8f96ef6c72a80c295b1c9e70706f8503113d1b2459907199dbefd9f"
                     "fdaaf4ac64bad1ae8885ee1d16277e9"
                     "8d55680d261e1ee3ace663d2516fc9cff7fc67dae70c7ee09e9140968cc2155ca4208b0e1e02f41b1d30208103d79192b"
                     "ff23966ef2156241d4b647a2c375ac0"
                     "10479f19f13209ad91f3668b2f5befbf1aabfd1ba0f90be678fb0c5465bdc34a25638e7da4286ef695e29ff7159f748f4"
                     "5eef5e69678f30cfc6d4d8295b5b3e3"
                     "619aa0d10859a4d8726071060f02080e33a07d683b240e59d1017d1cfff2766c2b2cc798f1ad5df885d21192c3a0187bf"
                     "d815d9bdbb8d59fd2a36a6d21d8e592"
                     "5956dc3b7adddf1f73392461aa76b3b6cdbaff92d66740d64eaf1cb7b95422ab21706508178044d12e975738b24fdb224"
                     "8daa1e3e5f2ce47904e66b70b1c5ae6"
                     "729625df34ab071915645b925d1d21547a24b2a95b15d7ace8e3901b9c5bbfa9e4e76b20c193c09634684663363573e60"
                     "fd01f85fb1bbb3efa5e4fd7c6b2d1d0"
                     "e444b3837e7fed92c7624c806c927f4486436ddba0a67564c1878d6d23ec24788cf9da2a731b703858688a050357786d9"
                     "30387908d6baf40c434fe0c4e8d73f4"
                     "fb1d26825e9c09b8d049bd6e21afb6927a6a1aa4c27dd5407731e2eafcbb1c9bdc30770dd3dc879d7445df60803ceab66"
                     "c3226dffda42cdd67c33ce8b66ef0b6"
                     "a8794178ec1b407bc67c1e9acda1a31f983a611262360f5f40216bb3e25c23e9ac6c8df47533029ad7ad46c735ac0992f"
                     "83e139aa97ef9362bb09d78b26cd0d4"
                     "a76299e5b958f4a2e1d182330d4f912e9c3836bcb496a3402d84a304378173e757be0eb88cabcf18a7c9b2c646963d224"
                     "7317e857867cb0f802520c45f01eba6",
                     2048) == 0);
}
END_TEST

START_TEST(test_crypto_derive_keysets_B256)
{
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32], zeros[32];
    const SOPC_ExposedBuffer* pout;
    char hexoutput[160];
    uint32_t lenCryptoKey, lenSignKey, lenIV, lenCliNonce, lenSerNonce, lenOutp;
    SOPC_SC_SecurityKeySet cliKS, serKS;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenCryptoKey, &lenSignKey, &lenIV) == SOPC_STATUS_OK);
    lenOutp = lenCryptoKey + lenSignKey + lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenCliNonce) ==
              SOPC_STATUS_OK); // TODO: use future GetLength_Nonce
    lenSerNonce = lenCliNonce;

    // Prepares security key sets
    memset(zeros, 0, 32);
    cliKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenSignKey);
    ck_assert_ptr_nonnull(cliKS.signKey);
    cliKS.encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenCryptoKey);
    ck_assert_ptr_nonnull(cliKS.encryptKey);
    cliKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV);
    ck_assert_ptr_nonnull(cliKS.initVector);
    serKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenSignKey);
    ck_assert_ptr_nonnull(serKS.signKey);
    serKS.encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenCryptoKey);
    ck_assert_ptr_nonnull(serKS.encryptKey);
    serKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV);
    ck_assert_ptr_nonnull(serKS.initVector);

    // These come from a stub_client working with OPC foundation code (e.g. commit "Bugfix: used CryptoKey instead of
    // SignKey")
    ck_assert(unhexlify("26353d1e608669d81dcc1ca7ca1f7e2b0aac53166d512a6f09527fbe54b114b5", clientNonce, lenCliNonce) ==
              (int32_t) lenCliNonce);
    ck_assert(unhexlify("0928c7fe64e3bfcfb99ffd396f1fb6d6048778a9ec70114c400753ee9af66ec6", serverNonce, lenSerNonce) ==
              (int32_t) lenSerNonce);
    ck_assert(SOPC_CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS,
                                                &serKS) == SOPC_STATUS_OK);
    // 4 lines for each assert
    pout = SOPC_SecretBuffer_Expose(cliKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "3a5dcd4af4db9bee2d4c8dcbaeb5471b56d03fc25d08d1c2", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.signKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "90c4fc7d1e9e321fae485f70b9fbb9745c821cca74f0aa7f36f58dcb7d3b85ea", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "647cbf8f5e0b3374434f49d9082fe045", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.initVector);
    pout = SOPC_SecretBuffer_Expose(serKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "46ec958d79b5690eb8d14f9ba2e3a5bb3335da1e235a77ff", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.signKey);
    pout = SOPC_SecretBuffer_Expose(serKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "367b5f02c15b5fbc44a1c332c7b36bfb4b728ec6f6742161911ee17c77d0555c", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(serKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "662cbc7f4ad064515e6c7824b22efdf5", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.initVector);

    // Another run, just to be sure...
    ck_assert(unhexlify("66407d42aa46d2e38e79e225467915b64cca887039c81c1c23584274a79dc1bc", clientNonce, lenCliNonce) ==
              (int32_t) lenCliNonce);
    ck_assert(unhexlify("6874dd63e91e57987e661622d2179a833c8e16a47fb97ceabc45ebe37112471d", serverNonce, lenSerNonce) ==
              (int32_t) lenSerNonce);
    ck_assert(SOPC_CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS,
                                                &serKS) == SOPC_STATUS_OK);
    pout = SOPC_SecretBuffer_Expose(cliKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "4ea2e84c14d4a1de0c84980d355c51cdef83281f770e5cf7", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.signKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "524dfcc42085c6df27bc03669bcba4981940cadc1c204dae64ef035a9f43c4e3", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(cliKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "34225334b9efebb9b9477ea1c9a1521e", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.initVector);
    pout = SOPC_SecretBuffer_Expose(serKS.signKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "644176e265fc190fa8013ce06f76e4fee3fb8754151fa364", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.signKey);
    pout = SOPC_SecretBuffer_Expose(serKS.encryptKey);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "90d8c836ed240f73b8e2ac7ceb6bd9fa15588b2cc94aa0aef0ea828f6e0539b3", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.encryptKey);
    pout = SOPC_SecretBuffer_Expose(serKS.initVector);
    ck_assert_ptr_nonnull(pout);
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "dcd99a892fe5a467416cbe73039572e8", 2 * lenIV) == 0);
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

START_TEST(test_cert_load_B256)
{
    ;
}
END_TEST

START_TEST(test_cert_lengths_B256)
{
    uint32_t len = 0;

    ck_assert(SOPC_CryptoProvider_CertificateGetLength_Thumbprint(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(20 == len); // SHA-1
}
END_TEST

START_TEST(test_cert_thumbprint_B256)
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

START_TEST(test_cert_loadkey_B256)
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
// in the tests. This key is 1024 bits long, which is the smallest possible. Note: The signature of the key expires on
// 05/03/2017, which is not a problem, as the key usage is to test asymmetric cryptographic primitives
#define DER_ASYM_PUB_HEXA                                                                                              \
    "30820286308201efa003020102020900c4d03aaaf2bbde98300d06092a864886f70d01010b0500305c310b3009060355040613024652310f" \
    "300d06035504080c064672616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504" \
    "030c12494e474f5043532054657374207375697465301e170d3136313132353137353033385a170d3137303330353137353033385a305c31" \
    "0b3009060355040613024652310f300d06035504080c064672616e6365310c300a06035504070c034169783111300f060355040a0c085379" \
    "73746572656c311b301906035504030c12494e474f504353205465737420737569746530819f300d06092a864886f70d010101050003818d" \
    "0030818902818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b0db5d668a5f10ec8d17828919b436f64cc2540ea246d9bca" \
    "e0d891a4983d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01eefca8e3ae6ec2878a77bd960edc5acf1ea7f8af18a" \
    "6de2f0a6b4f8c52da4a70816718c20da7dfa629fc9e613510203010001a350304e301d0603551d0e041604142417c23c516564e10e04671a" \
    "234ca8939b492a5d301f0603551d230418301680142417c23c516564e10e04671a234ca8939b492a5d300c0603551d13040530030101ff30" \
    "0d06092a864886f70d01010b050003818100630e6c7d008cc1b681608231dbb28d274683498c654a8244c2fc1411c4d239a0512803bf4dfb" \
    "d2f1ddd96a83f7de68c4fd6b8df80782043dcb8d80a27021dd03b2f403c06ebd45b9e75a5a06715283c6796988bb23fce0749c8aecee4cc2" \
    "05cb423ebfb84f57759e1b02cc48e89cd3cb90b27e2785d7a22ebee8bf0988edec28"
#define DER_ASYM_PUB_LENG 650
#define DER_ASYM_PRIV_HEXA                                                                                             \
    "3082025e02010002818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b0db5d668a5f10ec8d17828919b436f64cc2540ea24" \
    "6d9bcae0d891a4983d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01eefca8e3ae6ec2878a77bd960edc5acf1ea7f" \
    "8af18a6de2f0a6b4f8c52da4a70816718c20da7dfa629fc9e613510203010001028181008e8f9d7564c60c7961351e62465766140ef07643" \
    "e07c99b9a9834b56c2ffa9d325c43b73d719cd4e167341bbf74cc4f290bb0edd1f958e29e86fc83c267d9b21c45a7618c4c5ca124e2dd8bb" \
    "b2828669f57a9dc5395f4ce49f7afb251ddb4ebe97cadf648f26fc850e2587d73bd86bbda4769615de4fcbc4de6b1d9cf15b8d81024100f6" \
    "401a1f0eae030171ae99edf82e708fb46912889189315ad27c759a75207cc0f11d129aa995393174a045fb29a6476487e6cd92242978729e" \
    "2136d5ce953f65024100f5b909a77e0bd1de7d9979429508e17f4339fbe05cffd0c9d4c3ee886f39183bb5ac452d3c3668a2af1a01fe435b" \
    "c4ad14be9b1dbd359eca5a89aa923001337d024012d2296cf0414a8784b9d498049d000b6bbd902612018b5d26b34e85c4a7fc00ff2cbaac" \
    "4983d740396aba8e8ccb61af845796a4b1d0dd9cdd0b2ad6c29853a5024100f34a19fcf417cfdb72901a378a4818bc605b70bf5c550cec48" \
    "f5159f903fff765f120a0c17a9e73fec0edc1a5ba6e8bc55e5c2bf572f57e112736ba70250ae21024100ca81e8f4360563e0721f3a2227be" \
    "2f52141806f398801e3f7cba4c1e960bf7b53ff4babc92075300f69c8d1f2e56738165896558707f1831bd84f929cf12fb51"
#define DER_ASYM_PRIV_LENG 610
#define DER_ASYM_PUB_KEYONLY_HEXA                                                                                      \
    "30819f300d06092a864886f70d010101050003818d0030818902818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b0db5d6" \
    "68a5f10ec8d17828919b436f64cc2540ea246d9bcae0d891a4983d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01e" \
    "efca8e3ae6ec2878a77bd960edc5acf1ea7f8af18a6de2f0a6b4f8c52da4a70816718c20da7dfa629fc9e613510203010001"
#define DER_ASYM_PUB_KEYONLY_LENG 162

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

START_TEST(test_crypto_asym_load_B256)
{
    ;
}
END_TEST

START_TEST(test_crypto_asym_lengths_B256)
{
    uint32_t lenPlain = 0, lenCiph = 0, len = 0;

    // Check lengths
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(crypto, key_pub, &lenPlain) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(crypto, key_pub, &lenCiph) == SOPC_STATUS_OK);
    ck_assert(128 == lenCiph);
    ck_assert(86 == lenPlain); // 128 - 2*20 - 2
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_pub, &lenCiph, &lenPlain) == SOPC_STATUS_OK);
    ck_assert(128 == lenCiph);
    ck_assert(86 == lenPlain); // 128 - 2*20 - 2
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_priv, &lenCiph, &lenPlain) == SOPC_STATUS_OK);
    ck_assert(128 == lenCiph);
    ck_assert(86 == lenPlain); // 128 - 2*20 - 2
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 32, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 128, &len) == SOPC_STATUS_OK);
    ck_assert(86 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 688, &len) == SOPC_STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 1024, &len) == SOPC_STATUS_OK);
    ck_assert(688 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(crypto, &len) == SOPC_STATUS_OK);
    ck_assert(20 == len); // SHA-1
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len); // One block
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len); // One block
}
END_TEST

START_TEST(test_crypto_asym_crypt_B256)
{
    uint8_t input[688], output[1024], input_bis[688];
    uint32_t len = 0;
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32];
    const char* errorReason = "";

    // Encryption/Decryption
    // a) Single message (< 214)
    memset(input, 0, 688);
    memset(output, 0, 1024);
    strncpy((char*) input, "Test S2OPC Test", 32); // And test padding btw...
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 32, key_pub, output, 128, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 128, key_priv, input_bis, 86, &len, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(len == 32);
    ck_assert(memcmp(input, input_bis, 32) == 0);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("c3cc8578608ae88e9690b921254d028e1b9cdc75fbf5070c4e39e5712b4a8bdf", clientNonce, 32) == 32);
    ck_assert(unhexlify("9b8a2d541f4b3ed8ae69111cc85c4ea875fb7e2a541aa87d703fe1a5d037dcfc", serverNonce, 32) == 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input + 32,
                                                         688 - 32) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 688, key_pub, output, 1024, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 1024, key_priv, input_bis, 688, &len,
                                                    &errorReason) == SOPC_STATUS_OK);
    ck_assert(len == 688);
    ck_assert(memcmp(input, input_bis, 688) == 0);
}
END_TEST

START_TEST(test_crypto_asym_sign_verify_B256)
{
    uint8_t input[688], sig[128];
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32];
    const char* errorReason = "";

    // Signature
    // a) Single message (< 214)
    memset(input, 0, 688);
    memset(sig, 0, 128);
    strncpy((char*) input, "Test S2OPC Test", 32); // And test padding btw...
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 32, key_priv, sig, 128, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 32, key_pub, sig, 128, &errorReason) ==
              SOPC_STATUS_OK);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("c3cc8578608ae88e9690b921254d028e1b9cdc75fbf5070c4e39e5712b4a8bdf", clientNonce, 32) == 32);
    ck_assert(unhexlify("9b8a2d541f4b3ed8ae69111cc85c4ea875fb7e2a541aa87d703fe1a5d037dcfc", serverNonce, 32) == 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input + 32,
                                                         688 - 32) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 688, key_priv, sig, 128, &errorReason) ==
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 688, key_pub, sig, 128, &errorReason) ==
              SOPC_STATUS_OK);
}
END_TEST

START_TEST(test_crypto_asym_copykey_B256)
{
    uint8_t buffer[2048], der_priv[DER_ASYM_PRIV_LENG], der_pub_key[DER_ASYM_PUB_LENG];
    uint32_t lenDER = 0;
    SOPC_AsymmetricKey* pKey = NULL;

    // Private Key
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

START_TEST(test_crypto_asym_uri_B256)
{
    int res = strncmp(SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(crypto),
                      SOPC_SecurityPolicy_Basic256_URI_SignAlgo, strlen(SOPC_SecurityPolicy_Basic256_URI_SignAlgo) + 1);
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

START_TEST(test_pki_load_B256)
{
    ck_assert(NULL != pki->pFnValidateCertificate);
}
END_TEST

START_TEST(test_pki_cert_validation_B256)
{
    uint32_t errorStatus;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_Certificate_Validate(crypto, pki, crt_pub, &errorStatus);
    // Checks that the PKI validates our server.pub with our cacert.der
    ck_assert_msg(status == SOPC_STATUS_OK, "Validation failed, is this a \"date\" problem?");
}
END_TEST

START_TEST(test_cert_copyder_B256)
{
    uint8_t *buffer0 = NULL, *buffer1 = NULL;
    size_t der_len = strlen(SRV_CRT) / 2;
    uint8_t* der_cert = SOPC_Calloc(der_len, sizeof(uint8_t));
    ck_assert_ptr_nonnull(der_cert);
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

Suite* tests_make_suite_crypto_B256(void)
{
    Suite* s = NULL;
    TCase *tc_crypto_symm = NULL, *tc_providers = NULL, *tc_rands = NULL, *tc_derives = NULL, *tc_km = NULL,
          *tc_crypto_asym = NULL, *tc_pki_stack = NULL;

    s = suite_create("Crypto tests Basic256");
    tc_crypto_symm = tcase_create("Symmetric Crypto");
    tc_providers = tcase_create("Crypto Provider");
    tc_rands = tcase_create("Random Generation");
    tc_derives = tcase_create("Crypto Data Derivation");
    tc_km = tcase_create("Key Management");
    tc_crypto_asym = tcase_create("Asymmetric Crypto");
    tc_pki_stack = tcase_create("PKI stack");

    suite_add_tcase(s, tc_crypto_symm);
    tcase_add_checked_fixture(tc_crypto_symm, setup_crypto, teardown_crypto);
    tcase_add_test(tc_crypto_symm, test_crypto_load_B256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_lengths_B256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_crypt_B256);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_sign_B256);

    suite_add_tcase(s, tc_rands);
    tcase_add_checked_fixture(tc_rands, setup_crypto, teardown_crypto);
    tcase_add_test(tc_rands, test_crypto_generate_nbytes_B256);
    tcase_add_test(tc_rands, test_crypto_generate_nonce_B256);
    tcase_add_test(tc_rands, test_crypto_generate_uint32_B256);

    suite_add_tcase(s, tc_providers);
    tcase_add_checked_fixture(tc_providers, setup_crypto, teardown_crypto);

    suite_add_tcase(s, tc_derives);
    tcase_add_checked_fixture(tc_derives, setup_crypto, teardown_crypto);
    tcase_add_test(tc_derives, test_crypto_derive_lengths_B256);
    tcase_add_test(tc_derives, test_crypto_derive_data_B256);
    tcase_add_test(tc_derives, test_crypto_derive_keysets_B256);
    // TODO: derive_keysets_client
    // TODO: derive_keysets_server

    suite_add_tcase(s, tc_km);
    tcase_add_checked_fixture(tc_km, setup_certificate, teardown_certificate);
    tcase_add_test(tc_km, test_cert_load_B256);
    tcase_add_test(tc_km, test_cert_lengths_B256);
    tcase_add_test(tc_km, test_cert_thumbprint_B256);
    tcase_add_test(tc_km, test_cert_loadkey_B256);
    tcase_add_test(tc_km, test_cert_copyder_B256);

    suite_add_tcase(s, tc_crypto_asym);
    tcase_add_checked_fixture(tc_crypto_asym, setup_asym_keys, teardown_asym_keys);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_load_B256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_lengths_B256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_crypt_B256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_sign_verify_B256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_copykey_B256);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_uri_B256);

    suite_add_tcase(s, tc_pki_stack);
    tcase_add_checked_fixture(tc_pki_stack, setup_pki_stack, teardown_pki_stack);
    tcase_add_test(tc_pki_stack, test_pki_load_B256);
    tcase_add_test(tc_pki_stack, test_pki_cert_validation_B256);

    return s;
}
