/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 * \brief Cryptographic test suite. This suite tests "http://opcfoundation.org/UA/SecurityPolicy#Basic256".
 *
 * See check_stack.c for more details.
 */

#include <check.h>
#include <stdio.h>
#include <stdlib.h> // malloc, free

#include "check_helpers.h"
#include "crypto_provider_lib.h"
#include "hexlify.h"
#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
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
    ck_assert(NULL != crypto->pProfile);
    ck_assert(SOPC_SecurityPolicy_Basic256_ID == crypto->pProfile->SecurityPolicyID);
    ck_assert(NULL != crypto->pProfile->pFnSymmEncrypt);
    ck_assert(NULL != crypto->pProfile->pFnSymmDecrypt);
    ck_assert(NULL != crypto->pProfile->pFnSymmSign);
    ck_assert(NULL != crypto->pProfile->pFnSymmVerif);
    ck_assert(NULL != crypto->pProfile->pFnGenRnd);
    ck_assert(NULL != crypto->pProfile->pFnDeriveData);
    ck_assert(NULL != crypto->pProfile->pFnAsymEncrypt);
    ck_assert(NULL != crypto->pProfile->pFnAsymDecrypt);
    ck_assert(NULL != crypto->pProfile->pFnAsymSign);
    ck_assert(NULL != crypto->pProfile->pFnAsymVerify);
    ck_assert(NULL != crypto->pProfile->pFnCertVerify);
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

/* This test is the same for security policy that are not None,
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
    free(pExpBuffer0);
    free(pExpBuffer1);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(NULL, 64, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 0, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_generate_nonce_B256)
{
    SOPC_SecretBuffer *pSecNonce0, *pSecNonce1;
    SOPC_ExposedBuffer *pExpKey0, *pExpKey1;

    // It is random, so...
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, &pSecNonce0) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, &pSecNonce1) == SOPC_STATUS_OK);
    ck_assert(NULL != (pExpKey0 = SOPC_SecretBuffer_Expose(pSecNonce0)));
    ck_assert(NULL != (pExpKey1 = SOPC_SecretBuffer_Expose(pSecNonce1)));
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
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32], *pout, zeros[32];
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
    ck_assert(NULL != (cliKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenSignKey)));
    ck_assert(NULL != (cliKS.encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenCryptoKey)));
    ck_assert(NULL != (cliKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV)));
    ck_assert(NULL != (serKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenSignKey)));
    ck_assert(NULL != (serKS.encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenCryptoKey)));
    ck_assert(NULL != (serKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV)));

    // These come from a stub_client working with OPC foundation code (e.g. commit "Bugfix: used CryptoKey instead of
    // SignKey")
    ck_assert(unhexlify("26353d1e608669d81dcc1ca7ca1f7e2b0aac53166d512a6f09527fbe54b114b5", clientNonce, lenCliNonce) ==
              (int32_t) lenCliNonce);
    ck_assert(unhexlify("0928c7fe64e3bfcfb99ffd396f1fb6d6048778a9ec70114c400753ee9af66ec6", serverNonce, lenSerNonce) ==
              (int32_t) lenSerNonce);
    ck_assert(SOPC_CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS,
                                                &serKS) == SOPC_STATUS_OK);
    // 4 lines for each assert
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(cliKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "3a5dcd4af4db9bee2d4c8dcbaeb5471b56d03fc25d08d1c2", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.signKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(cliKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "90c4fc7d1e9e321fae485f70b9fbb9745c821cca74f0aa7f36f58dcb7d3b85ea", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.encryptKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(cliKS.initVector)));
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "647cbf8f5e0b3374434f49d9082fe045", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.initVector);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(serKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "46ec958d79b5690eb8d14f9ba2e3a5bb3335da1e235a77ff", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.signKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(serKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "367b5f02c15b5fbc44a1c332c7b36bfb4b728ec6f6742161911ee17c77d0555c", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.encryptKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(serKS.initVector)));
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
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(cliKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "4ea2e84c14d4a1de0c84980d355c51cdef83281f770e5cf7", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.signKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(cliKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "524dfcc42085c6df27bc03669bcba4981940cadc1c204dae64ef035a9f43c4e3", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.encryptKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(cliKS.initVector)));
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t) lenIV);
    ck_assert(memcmp(hexoutput, "34225334b9efebb9b9477ea1c9a1521e", 2 * lenIV) == 0);
    SOPC_SecretBuffer_Unexpose(pout, cliKS.initVector);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(serKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenSignKey) == (int32_t) lenSignKey);
    ck_assert(memcmp(hexoutput, "644176e265fc190fa8013ce06f76e4fee3fb8754151fa364", 2 * lenSignKey) == 0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.signKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(serKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenCryptoKey) == (int32_t) lenCryptoKey);
    ck_assert(memcmp(hexoutput, "90d8c836ed240f73b8e2ac7ceb6bd9fa15588b2cc94aa0aef0ea828f6e0539b3", 2 * lenCryptoKey) ==
              0);
    SOPC_SecretBuffer_Unexpose(pout, serKS.encryptKey);
    ck_assert(NULL != (pout = SOPC_SecretBuffer_Expose(serKS.initVector)));
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
static SOPC_Certificate* crt_pub = NULL;

static inline void setup_certificate(void)
{
#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    uint8_t der_cert[1069];
#else
    uint8_t der_cert[1215];
#endif

    setup_crypto();

#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    // Loads a certificate. This is server_public/server_2k.der.
    ck_assert(unhexlify("3082042930820311a003020102020106300d06092a864886f70d01010b0500308188310b3009060355040613024652"
                        "310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a0c07494e474f5043"
                        "533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a"
                        "864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d3137313130323136353835345a17"
                        "0d3138313130323136353835345a3068310b3009060355040613024652310c300a06035504080c03494446310e300c"
                        "06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f5043"
                        "533117301506035504030c0e494e474f5043532053455256455230820122300d06092a864886f70d01010105000382"
                        "010f003082010a0282010100b245842f18a25870b4bdcf162ca5b2d48cb531f0c5ba5d509e5051b892fe535d6dbfba"
                        "fc6f3e430af250468659476cfd8381c57a9dc4063d14ef48b6ede87b1a15e3560a8a759a4cda397a71034c812142c7"
                        "43d16685b6fe2f2bb3a56570a88f87452121ecb6bce7865e14ef83a5bd436661995a8a0635d3ba9b22324bd2d06f3c"
                        "f117418d6e9e2fa3e1d317c77ba00972e622d602d8f1c07a720ba7d064150ee0533f256ccc0bb6fa0447dcacc97462"
                        "7c54bb544c60cbef5aeb70ebfe8c07acaf64f315088e8461a4af2f9d8846065c416d0c1cc4075e4baa2d7f44c1be2b"
                        "d1e2d8dec1f9796b8727cac05f5e931069192fae496298a549bae6df699e2638270203010001a381bc3081b9301d06"
                        "03551d0e0416041449da0583068c3c894ac32777d92ba84e8763f722301f0603551d2304183016801431ae8e653364"
                        "06e4379b518fc3379dce459bb61f300f0603551d130101ff040530030101ff300b0603551d0f040403020106302b06"
                        "03551d1104243022861575726e3a494e474f5043533a6c6f63616c686f737482096c6f63616c686f7374302c060960"
                        "86480186f842010d041f161d4f70656e53534c2047656e657261746564204365727469666963617465300d06092a86"
                        "4886f70d01010b0500038201010053e982480661e4bffb4af3f1b37d663e4191a3b0cfb590e9f50008634c63d3b442"
                        "97b7544dee44f103f589c15f59b080e037603309e0afe6b9d294f61ad85f5de27275983d6d81353e97dfc30bbbcc3b"
                        "359a8720206bc298bc9f0da97a4dbd24c0c07a9607071c3d729ad2849ee0c3d2b410ee9e066aef3a9bf3e755f19270"
                        "808c5efbaf46a8c8ca7d290162a818c1e6abba08c973e4d6857487a62e0a34a61930150e2e2639fc5ec8234f5a86a3"
                        "8bd9e7454f8157732f6028adbcb1525770671c0443724270d4bf8fa530193e033712a2e2f2b8581d3dcaa974cb3504"
                        "c475582322b8f0ffb212a714fea2b6016eb928ef523ec65f0b160063ba207c45caa2d7",
                        der_cert, 1069) == 1069);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_cert, 1069, &crt_pub) == SOPC_STATUS_OK);
#else
    // Loads a certificate. This is the server/server.der.
    ck_assert(unhexlify("308204bb308202a3a003020102020102300d06092a864886f70d01010b0500308188310b3009060355040613024652"
                        "310c300a06035504080c03494446310e30"
                        "0c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f50"
                        "43533113301106035504030c0a494e474f"
                        "5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d31"
                        "37303532323132343431335a170d313830"
                        "3532323132343431335a3057310b3009060355040613024652310c300a06035504080c03494446310e300c06035504"
                        "070c0550415249533111300f060355040a"
                        "0c08535953544552454c3117301506035504030c0e494e474f5043535f53455256455230820122300d06092a864886"
                        "f70d01010105000382010f003082010a02"
                        "82010100ad9921f924639e125c0cde520755f44028d65eaecaf16867823be446b977e0631d64509953b7fe467d1afc"
                        "449bca6edfe11e1e6d71207c33e2250f3c"
                        "66875d369a1cda02efc661e73bdf01c517470f2a09ea500b56842fcb125779917b8deb58dc6f2f9511e66c29ba57a6"
                        "9435bc3aab1a23982f531ec763f494ef8b"
                        "6c6360ea194d7ca2efd777b9a32c295809cf39d2c2ed0dbfc4bfd6fbd24bf782f8d83795cb51964e1dd0a8cdd8f2a0"
                        "ef2fd0d2b126eb8fc00f00411f362cd4e3"
                        "0a0a20cde108efa69faede8d9f756838306569c6ea27f1ba5aefac790ff18bcbcc81d7acaa1fac2acede3acd2a61d7"
                        "b62f202c7bab7df08ee2241a0f08dffdb6"
                        "2914cf210203010001a360305e301d0603551d0e04160414a3f8e031d1f6f412bace4ddf0eeb62da209d3c79301f06"
                        "03551d2304183016801478a2ae09c22875"
                        "23d9b20f7fb31293ecf5ce2d14300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d0609"
                        "2a864886f70d01010b0500038202010033"
                        "75d0fef58f5d7fcea8ac5a7aa9f94aacab123925f0298ee5b8f81df188e149df1e7771539437bef947dcd90dc12bd4"
                        "2ae185e715d7633ba4386d99e39e11c012"
                        "b998a2f127a6e515f7fa657346518332f00ae4320d03d461f6bff99240e54093c6bf98f24747f9a2080ea6391d6fd3"
                        "4f85a7daae1c1c10c21129950542f89715"
                        "ef2fc11d2c73982bdbbeac58627b13c8702ee9dacccd4b8b903bc9834beb4c898d8a70b323dadd7db8235146c0ed0a"
                        "a26eda611a0fb1c5ea7cd97928317d6735"
                        "945dbe16fb72c5477b9403a7e34e7528090e37543e988fdf3fc72669991c8161b6e8cd9231c987a8bd35541d646a6f"
                        "f3146b229f53b8c650df69d1aa7b292076"
                        "25c443a8f4fc87cb848abc5937c1b230d9b940386f11c35318659ef6712424f7d21943fe7ad3b6acbe15cd903a33b3"
                        "96c755661967a31026576220a547b43e3f"
                        "8f2e4df13c14f8250c02a56b4add9fc1f503bc7d8a8573909d8e9d3c5a008be8170b528711cc0084072ad6839b5288"
                        "1e613c9c8121415a4c8bcdd6e4d448616e"
                        "003addd166727c198735601abd2526b2fe9cd3158df9b258ef3101f8ddeaf2b36e8d7f6348dea70e3f5d69f8e1b4eb"
                        "e8156405f55692e9342630264f1dc948a4"
                        "095da188cd50e7f8cd30fce5232cb564dc0bef2d761e12f5dde9ac93a81f67fae30d251a59f578dd6aaa7d3edbc813"
                        "87dae9d67ee148d1662ca139446a8de3",
                        der_cert, 1215) == 1215);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_cert, 1215, &crt_pub) == SOPC_STATUS_OK);
#endif
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

    // ck_assert(KeyManager_Certificate_CreateFromFile(keyman, (int8_t *)"./server_public/server.der", &crt_pub) ==
    // SOPC_STATUS_OK);

    // Compute thumbprint
    ck_assert(SOPC_KeyManager_Certificate_GetThumbprint(crypto, crt_pub, thumb, 20) == SOPC_STATUS_OK);
    ck_assert(hexlify(thumb, hexoutput, 20) == 20);
    // The expected thumbprint for this certificate was calculated with openssl tool, and mbedtls API.
#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    ck_assert(memcmp(hexoutput, "8bbe16605756eebcb6e10756bc17c57df3f1ef01", 40) == 0);
#else
    ck_assert(memcmp(hexoutput, "80968e5e796b36c6c5cc8546092c36f72137d8b0", 40) == 0);
#endif
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
    "300d06035504080c"                                                                                                 \
    "064672616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504030c12494e474f50" \
    "4353205465737420"                                                                                                 \
    "7375697465301e170d3136313132353137353033385a170d3137303330353137353033385a305c310b3009060355040613024652310f300d" \
    "06035504080c0646"                                                                                                 \
    "72616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504030c12494e474f504353" \
    "2054657374207375"                                                                                                 \
    "69746530819f300d06092a864886f70d010101050003818d0030818902818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b" \
    "0db5d668a5f10ec8"                                                                                                 \
    "d17828919b436f64cc2540ea246d9bcae0d891a4983d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01eefca8e3ae6" \
    "ec2878a77bd960ed"                                                                                                 \
    "c5acf1ea7f8af18a6de2f0a6b4f8c52da4a70816718c20da7dfa629fc9e613510203010001a350304e301d0603551d0e041604142417c23c" \
    "516564e10e04671a"                                                                                                 \
    "234ca8939b492a5d301f0603551d230418301680142417c23c516564e10e04671a234ca8939b492a5d300c0603551d13040530030101ff30" \
    "0d06092a864886f7"                                                                                                 \
    "0d01010b050003818100630e6c7d008cc1b681608231dbb28d274683498c654a8244c2fc1411c4d239a0512803bf4dfbd2f1ddd96a83f7de" \
    "68c4fd6b8df80782"                                                                                                 \
    "043dcb8d80a27021dd03b2f403c06ebd45b9e75a5a06715283c6796988bb23fce0749c8aecee4cc205cb423ebfb84f57759e1b02cc48e89c" \
    "d3cb90b27e2785d7"                                                                                                 \
    "a22ebee8bf0988edec28"
#define DER_ASYM_PUB_LENG 650
#define DER_ASYM_PRIV_HEXA                                                                                             \
    "3082025e02010002818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b0db5d668a5f10ec8d17828919b436f64cc2540ea24" \
    "6d9bcae0d891a498"                                                                                                 \
    "3d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01eefca8e3ae6ec2878a77bd960edc5acf1ea7f8af18a6de2f0a6b4" \
    "f8c52da4a7081671"                                                                                                 \
    "8c20da7dfa629fc9e613510203010001028181008e8f9d7564c60c7961351e62465766140ef07643e07c99b9a9834b56c2ffa9d325c43b73" \
    "d719cd4e167341bb"                                                                                                 \
    "f74cc4f290bb0edd1f958e29e86fc83c267d9b21c45a7618c4c5ca124e2dd8bbb2828669f57a9dc5395f4ce49f7afb251ddb4ebe97cadf64" \
    "8f26fc850e2587d7"                                                                                                 \
    "3bd86bbda4769615de4fcbc4de6b1d9cf15b8d81024100f6401a1f0eae030171ae99edf82e708fb46912889189315ad27c759a75207cc0f1" \
    "1d129aa995393174"                                                                                                 \
    "a045fb29a6476487e6cd92242978729e2136d5ce953f65024100f5b909a77e0bd1de7d9979429508e17f4339fbe05cffd0c9d4c3ee886f39" \
    "183bb5ac452d3c36"                                                                                                 \
    "68a2af1a01fe435bc4ad14be9b1dbd359eca5a89aa923001337d024012d2296cf0414a8784b9d498049d000b6bbd902612018b5d26b34e85" \
    "c4a7fc00ff2cbaac"                                                                                                 \
    "4983d740396aba8e8ccb61af845796a4b1d0dd9cdd0b2ad6c29853a5024100f34a19fcf417cfdb72901a378a4818bc605b70bf5c550cec48" \
    "f5159f903fff765f"                                                                                                 \
    "120a0c17a9e73fec0edc1a5ba6e8bc55e5c2bf572f57e112736ba70250ae21024100ca81e8f4360563e0721f3a2227be2f52141806f39880" \
    "1e3f7cba4c1e960b"                                                                                                 \
    "f7b53ff4babc92075300f69c8d1f2e56738165896558707f1831bd84f929cf12fb51"
#define DER_ASYM_PRIV_LENG 610

static inline void setup_asym_keys(void)
{
    uint8_t der_cert[DER_ASYM_PUB_LENG], der_priv[DER_ASYM_PRIV_LENG];

    setup_crypto();

    // Loads certificate from DER
    ck_assert(unhexlify(DER_ASYM_PUB_HEXA, der_cert, DER_ASYM_PUB_LENG) == DER_ASYM_PUB_LENG);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_cert, DER_ASYM_PUB_LENG, &crt_pub) == SOPC_STATUS_OK); //*/

    // Loads the public key from cert
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(crt_pub, &key_pub) == SOPC_STATUS_OK);

    // Loads the corresponding private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(der_priv, DER_ASYM_PRIV_LENG, &key_priv) ==
              SOPC_STATUS_OK);
}

static inline void teardown_asym_keys(void)
{
    free(key_pub);
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

    // Encryption/Decryption
    // a) Single message (< 214)
    memset(input, 0, 688);
    memset(output, 0, 1024);
    strncpy((char*) input, "Test INGOPCS Test", 32); // And test padding btw...
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 32, key_pub, output, 128) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 128, key_priv, input_bis, 86, &len) ==
              SOPC_STATUS_OK);
    ck_assert(len == 32);
    ck_assert(memcmp(input, input_bis, 32) == 0);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("c3cc8578608ae88e9690b921254d028e1b9cdc75fbf5070c4e39e5712b4a8bdf", clientNonce, 32) == 32);
    ck_assert(unhexlify("9b8a2d541f4b3ed8ae69111cc85c4ea875fb7e2a541aa87d703fe1a5d037dcfc", serverNonce, 32) == 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input + 32,
                                                         688 - 32) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 688, key_pub, output, 1024) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 1024, key_priv, input_bis, 688, &len) ==
              SOPC_STATUS_OK);
    ck_assert(len == 688);
    ck_assert(memcmp(input, input_bis, 688) == 0);
}
END_TEST

START_TEST(test_crypto_asym_sign_verify_B256)
{
    uint8_t input[688], sig[128];
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32];

    // Signature
    // a) Single message (< 214)
    memset(input, 0, 688);
    memset(sig, 0, 128);
    strncpy((char*) input, "Test INGOPCS Test", 32); // And test padding btw...
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 32, key_priv, sig, 128) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 32, key_pub, sig, 128) == SOPC_STATUS_OK);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("c3cc8578608ae88e9690b921254d028e1b9cdc75fbf5070c4e39e5712b4a8bdf", clientNonce, 32) == 32);
    ck_assert(unhexlify("9b8a2d541f4b3ed8ae69111cc85c4ea875fb7e2a541aa87d703fe1a5d037dcfc", serverNonce, 32) == 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input + 32,
                                                         688 - 32) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 688, key_priv, sig, 128) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 688, key_pub, sig, 128) == SOPC_STATUS_OK);
}
END_TEST

START_TEST(test_crypto_asym_copykey_B256)
{
    uint8_t buffer[2048], der_priv[DER_ASYM_PRIV_LENG];
    uint32_t lenDER = 0;

    // Copy to DER
    ck_assert(SOPC_KeyManager_AsymmetricKey_ToDER(key_priv, buffer, 2048, &lenDER) == SOPC_STATUS_OK);

    // Loads DER of private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);

    // Verifies
    ck_assert(lenDER == DER_ASYM_PRIV_LENG);
    ck_assert(memcmp(buffer, der_priv, DER_ASYM_PRIV_LENG) == 0);
}
END_TEST

START_TEST(test_crypto_asym_uri_B256)
{
    ck_assert(strncmp(SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(crypto),
                      SOPC_SecurityPolicy_Basic256_URI_SignAlgo,
                      strlen(SOPC_SecurityPolicy_Basic256_URI_SignAlgo) + 1) == 0);
}
END_TEST

// Fixtures for PKI: server.der certificate and CA
static SOPC_Certificate* crt_ca = NULL;
static SOPC_PKIProvider* pki = NULL;

static inline void setup_pki_stack(void)
{
#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    uint8_t der_ca[1017];
#else
    uint8_t der_ca[1529];
#endif

    setup_certificate();

    // Loads CA cert which signed server.der. This is trusted/cacert.der.
#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    ck_assert(unhexlify("308203f5308202dda003020102020900ccd2d9ade3419143300d06092a864886f70d01010b0500308188310b300906"
                        "0355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a"
                        "0c07494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043"
                        "413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d313730393237"
                        "3135353032355a170d3138303932373135353032355a308188310b3009060355040613024652310c300a0603550408"
                        "0c03494446310e300c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e06035504"
                        "0b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a864886f70d01090116"
                        "13696e676f70637340737973746572656c2e667230820122300d06092a864886f70d01010105000382010f00308201"
                        "0a02820101009b3ba50f9a27b27370513de979667a3a4b24b3cc0fbd90fb03c6b19395a7a11eadc6aefeab54cb9c86"
                        "3c9b52eb8a48cd187c02b598f2e8d611ef56c18009701e480f4bcdfba3a339d79453fb0897b89e20f5ae5381f2a0bd"
                        "a97599c5e4167d243b03caa5b4bc7a05a424e2176ea04b2ef71ac273b20bd7c727e1a716f07e4ec1251debc6c98409"
                        "c8eafa16c05254495a7833bee269aa0fdb4c98e30fef2345afa65d1630e657e6a4c10ab51b9c34a4ddbd0e70909848"
                        "59bb2565b3fd10497ede6274ee7968a3bfc503a6f380e292a4c46f2e37e01a89dbd5af6e9af7f9fb34bbda53e62a87"
                        "a0427823c4866e5877d5c2660d6cd2649ccbd57265e6b56c1ad2570203010001a360305e301d0603551d0e04160414"
                        "31ae8e65336406e4379b518fc3379dce459bb61f301f0603551d2304183016801431ae8e65336406e4379b518fc337"
                        "9dce459bb61f300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d06092a864886f70d01"
                        "010b050003820101004edb4347e6ae979f1df03965bf831d3e70de123b8dbd93c0a262347948b25ba93ab893f26f22"
                        "9c41485198f2f6e4bcb5236516906318e3e00555ba11f27343e689078d87fda403b94f54cbe12fa50eecb98af00305"
                        "19260bd5cba4fef5254087ad8cd63306d481ca3bb883587e1bb2f23bd2b8e9e0c434a140bdba510099e6d64adbb586"
                        "28bbca257785f72db559dbf291fd6c43a5701f63d7805d705b3387cb032724cb8e0e949b9dc376e730a2d977c3838b"
                        "cea74ee9ed2b6997ae0f2dd268ced21cd1d698481f56b3206721a5937cceb5a79241335a259d49d38b550e39901707"
                        "81d5b6d6817476c9389f75293254515b64a9419d6e1cda971d79578906ce",
                        der_ca, 1017) == 1017);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_ca, 1017, &crt_ca) == SOPC_STATUS_OK);
#else
    ck_assert(unhexlify("308205f5308203dda003020102020900a5d3e9dfcf8e3f74300d06092a864886f70d01010b0500308188310b300906"
                        "0355040613024652310c300a0603550408"
                        "0c03494446310e300c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e06035504"
                        "0b0c07494e474f50435331133011060355"
                        "04030c0a494e474f5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c"
                        "2e6672301e170d31373035323231323331"
                        "34365a170d3230303231363132333134365a308188310b3009060355040613024652310c300a06035504080c034944"
                        "46310e300c06035504070c055041524953"
                        "3110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a"
                        "494e474f5043532043413122302006092a"
                        "864886f70d0109011613696e676f70637340737973746572656c2e667230820222300d06092a864886f70d01010105"
                        "000382020f003082020a028202010095de"
                        "e52dfa083d805290adbbaa426d2d06cf2b2ac3b80e3405c9671057a5ceff4c0ba7c5def83fbdeb9a068cb0bf8b9432"
                        "288690085d9c6b0fce95a8b427d5a4e5ec"
                        "d9bb20b1d07683abac38df75d31a54eddd2e01eb496409fa65f37b11fea245f2d155d253b665443a5077e33cc73f8e"
                        "895cbd42f9914f9542ee4bf0f84da2b26b"
                        "1feab00a937e45f44431ccfe24a431a35e9177f643f7a6517fa19ce31ec707ed970a480bb2532a9f7a209d5057c709"
                        "c5ef7f5a62c796bff9d8c5a0d38581e1ed"
                        "2497e05cff1da8c3ba36c867e47d825daee0e0a57ece5c04c208de69ae4ecf831b2ba628de4ddd61202c77ab1edb93"
                        "6b84ac3f2edf7a46f0d110901636fab5cd"
                        "612acc98e181a42a20ad8b080be80196f72b8550ccd5e4c5843e555b25eea8b2135f0410b7f070071b61027b5b14f8"
                        "36737404cc4a23c96535ca99e3c987d59d"
                        "612be96b8c2f3d142ba2f4b64af96bd74f6473fac57d3e14e40762721e030bb8020f0194ac226a17e86beaf4d84cfe"
                        "79d394fe2c8be7753356aa116234b5e71b"
                        "8b44964f1bf957184973f8427dcca50d4563f1e823b05a37cd4ab40fd210bc51b1ad14c3962d8defb5e6342abf6c43"
                        "88f9fb299588d639b97a336f9ae465da34"
                        "79460dcf99944175dc148949e178a466b9e1bdf56d249ff8afe1bdceed9f714d94d3fd27a78933b511f55cf549cfad"
                        "8190e8dbd54ae292d64fc5580a34730203"
                        "010001a360305e301d0603551d0e0416041478a2ae09c2287523d9b20f7fb31293ecf5ce2d14301f0603551d230418"
                        "3016801478a2ae09c2287523d9b20f7fb3"
                        "1293ecf5ce2d14300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d06092a864886f70d"
                        "01010b050003820201007307149108131c"
                        "f30e577bcc5326791d8cea6354516b8c59e4a4cdd88f9f0f14cb9794b3537e5445e1234d6d133f96fff97562fd4815"
                        "540dba252d4174d08499809ba4b018f258"
                        "4ecb0ee4657ded621030213425a6441986cf69070bb78bd29da1e463f3bccd3abdef9b0f95ef61293ff15a07338a6b"
                        "50f9d85b4f1f23ba2568c4a8ca7bd455e5"
                        "25d34047ae918f91b9692e09869d1767a5b7d1c35ed0f3469b08bd3aa07f09f8f596ce6a5b1ab117625bea466cb21a"
                        "f93c97005e5bb89c83039e1af4c34fc8ed"
                        "6001a86990c38b47d19a32039851134d8d4cf50680445f5a86dab3396ad03c18c3c2ab7676a04be94347a6f3c1a5c2"
                        "fd709287a64635e374b844185276978a23"
                        "a564931e8b0c295b6e24004f5a883e56216f75281fa76506a2e2cf9ab00faf629e2fac43f6a3af49bd9326092c8741"
                        "1a3900d962626051d650e6daf00e3d307f"
                        "43487813718d302fdb1dc1db0924920641bd0e57148c1e7fb0162649d7e631b7035961b5c64b845a42fbb16476ccea"
                        "083fe6a496d5cf501683e39488a31fa5f2"
                        "f82ae85ba3194486def2b90341a72beb797c082a950116a51bce080faced2b22ab78b46aa45c7db37aab808b4482cc"
                        "af55352bc6f6d82c26787b477b8245d17a"
                        "ee497253724b25ca941782133382164b33fa2140c978a2cd72c59a567f60b76f4208333c5fe2224ca37e702f42a317"
                        "148895383afb8f581493",
                        der_ca, 1529) == 1529);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_ca, 1529, &crt_ca) == SOPC_STATUS_OK);
#endif

    // Creates PKI with ca
    ck_assert(SOPC_PKIProviderStack_Create(crt_ca, NULL, &pki) == SOPC_STATUS_OK);
}

static inline void teardown_pki_stack(void)
{
    SOPC_PKIProviderStack_Free(pki);
    SOPC_KeyManager_Certificate_Free(crt_ca);

    teardown_certificate();
}

START_TEST(test_pki_load_B256)
{
    ck_assert(NULL != pki->pFnValidateCertificate);
}
END_TEST

START_TEST(test_pki_cert_validation_B256)
{
    // Checks that the PKI validates our server.pub with our cacert.der
    ck_assert_msg(SOPC_CryptoProvider_Certificate_Validate(crypto, pki, crt_pub) == SOPC_STATUS_OK,
                  "Validation failed, is this a \"date\" problem?");
}
END_TEST

START_TEST(test_cert_copyder_B256)
{
    uint8_t *buffer0 = NULL, *buffer1 = NULL;
#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    uint8_t der_cert[1069];
#else
    uint8_t der_cert[1215];
#endif
    uint32_t lenAlloc0 = 0, lenAlloc1 = 0;

#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    // Reference certificate. This is server_public/server_2k.der.
    ck_assert(unhexlify("3082042930820311a003020102020106300d06092a864886f70d01010b0500308188310b3009060355040613024652"
                        "310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a0c07494e474f5043"
                        "533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a"
                        "864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d3137313130323136353835345a17"
                        "0d3138313130323136353835345a3068310b3009060355040613024652310c300a06035504080c03494446310e300c"
                        "06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f5043"
                        "533117301506035504030c0e494e474f5043532053455256455230820122300d06092a864886f70d01010105000382"
                        "010f003082010a0282010100b245842f18a25870b4bdcf162ca5b2d48cb531f0c5ba5d509e5051b892fe535d6dbfba"
                        "fc6f3e430af250468659476cfd8381c57a9dc4063d14ef48b6ede87b1a15e3560a8a759a4cda397a71034c812142c7"
                        "43d16685b6fe2f2bb3a56570a88f87452121ecb6bce7865e14ef83a5bd436661995a8a0635d3ba9b22324bd2d06f3c"
                        "f117418d6e9e2fa3e1d317c77ba00972e622d602d8f1c07a720ba7d064150ee0533f256ccc0bb6fa0447dcacc97462"
                        "7c54bb544c60cbef5aeb70ebfe8c07acaf64f315088e8461a4af2f9d8846065c416d0c1cc4075e4baa2d7f44c1be2b"
                        "d1e2d8dec1f9796b8727cac05f5e931069192fae496298a549bae6df699e2638270203010001a381bc3081b9301d06"
                        "03551d0e0416041449da0583068c3c894ac32777d92ba84e8763f722301f0603551d2304183016801431ae8e653364"
                        "06e4379b518fc3379dce459bb61f300f0603551d130101ff040530030101ff300b0603551d0f040403020106302b06"
                        "03551d1104243022861575726e3a494e474f5043533a6c6f63616c686f737482096c6f63616c686f7374302c060960"
                        "86480186f842010d041f161d4f70656e53534c2047656e657261746564204365727469666963617465300d06092a86"
                        "4886f70d01010b0500038201010053e982480661e4bffb4af3f1b37d663e4191a3b0cfb590e9f50008634c63d3b442"
                        "97b7544dee44f103f589c15f59b080e037603309e0afe6b9d294f61ad85f5de27275983d6d81353e97dfc30bbbcc3b"
                        "359a8720206bc298bc9f0da97a4dbd24c0c07a9607071c3d729ad2849ee0c3d2b410ee9e066aef3a9bf3e755f19270"
                        "808c5efbaf46a8c8ca7d290162a818c1e6abba08c973e4d6857487a62e0a34a61930150e2e2639fc5ec8234f5a86a3"
                        "8bd9e7454f8157732f6028adbcb1525770671c0443724270d4bf8fa530193e033712a2e2f2b8581d3dcaa974cb3504"
                        "c475582322b8f0ffb212a714fea2b6016eb928ef523ec65f0b160063ba207c45caa2d7",
                        der_cert, 1069) == 1069);
#else
    // Reference certificate, this is server.der
    ck_assert(unhexlify("308204bb308202a3a003020102020102300d06092a864886f70d01010b0500308188310b3009060355040613024652"
                        "310c300a06035504080c03494446310e30"
                        "0c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f50"
                        "43533113301106035504030c0a494e474f"
                        "5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d31"
                        "37303532323132343431335a170d313830"
                        "3532323132343431335a3057310b3009060355040613024652310c300a06035504080c03494446310e300c06035504"
                        "070c0550415249533111300f060355040a"
                        "0c08535953544552454c3117301506035504030c0e494e474f5043535f53455256455230820122300d06092a864886"
                        "f70d01010105000382010f003082010a02"
                        "82010100ad9921f924639e125c0cde520755f44028d65eaecaf16867823be446b977e0631d64509953b7fe467d1afc"
                        "449bca6edfe11e1e6d71207c33e2250f3c"
                        "66875d369a1cda02efc661e73bdf01c517470f2a09ea500b56842fcb125779917b8deb58dc6f2f9511e66c29ba57a6"
                        "9435bc3aab1a23982f531ec763f494ef8b"
                        "6c6360ea194d7ca2efd777b9a32c295809cf39d2c2ed0dbfc4bfd6fbd24bf782f8d83795cb51964e1dd0a8cdd8f2a0"
                        "ef2fd0d2b126eb8fc00f00411f362cd4e3"
                        "0a0a20cde108efa69faede8d9f756838306569c6ea27f1ba5aefac790ff18bcbcc81d7acaa1fac2acede3acd2a61d7"
                        "b62f202c7bab7df08ee2241a0f08dffdb6"
                        "2914cf210203010001a360305e301d0603551d0e04160414a3f8e031d1f6f412bace4ddf0eeb62da209d3c79301f06"
                        "03551d2304183016801478a2ae09c22875"
                        "23d9b20f7fb31293ecf5ce2d14300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d0609"
                        "2a864886f70d01010b0500038202010033"
                        "75d0fef58f5d7fcea8ac5a7aa9f94aacab123925f0298ee5b8f81df188e149df1e7771539437bef947dcd90dc12bd4"
                        "2ae185e715d7633ba4386d99e39e11c012"
                        "b998a2f127a6e515f7fa657346518332f00ae4320d03d461f6bff99240e54093c6bf98f24747f9a2080ea6391d6fd3"
                        "4f85a7daae1c1c10c21129950542f89715"
                        "ef2fc11d2c73982bdbbeac58627b13c8702ee9dacccd4b8b903bc9834beb4c898d8a70b323dadd7db8235146c0ed0a"
                        "a26eda611a0fb1c5ea7cd97928317d6735"
                        "945dbe16fb72c5477b9403a7e34e7528090e37543e988fdf3fc72669991c8161b6e8cd9231c987a8bd35541d646a6f"
                        "f3146b229f53b8c650df69d1aa7b292076"
                        "25c443a8f4fc87cb848abc5937c1b230d9b940386f11c35318659ef6712424f7d21943fe7ad3b6acbe15cd903a33b3"
                        "96c755661967a31026576220a547b43e3f"
                        "8f2e4df13c14f8250c02a56b4add9fc1f503bc7d8a8573909d8e9d3c5a008be8170b528711cc0084072ad6839b5288"
                        "1e613c9c8121415a4c8bcdd6e4d448616e"
                        "003addd166727c198735601abd2526b2fe9cd3158df9b258ef3101f8ddeaf2b36e8d7f6348dea70e3f5d69f8e1b4eb"
                        "e8156405f55692e9342630264f1dc948a4"
                        "095da188cd50e7f8cd30fce5232cb564dc0bef2d761e12f5dde9ac93a81f67fae30d251a59f578dd6aaa7d3edbc813"
                        "87dae9d67ee148d1662ca139446a8de3",
                        der_cert, 1215) == 1215);
#endif

    // Extract 2 copies from loaded certificate
    ck_assert(SOPC_KeyManager_Certificate_CopyDER(crt_pub, &buffer0, &lenAlloc0) == SOPC_STATUS_OK);
    ck_assert(SOPC_KeyManager_Certificate_CopyDER(crt_pub, &buffer1, &lenAlloc1) == SOPC_STATUS_OK);

    // Both should be identical, and identical to der_cert
    ck_assert(lenAlloc0 == lenAlloc1);
    ck_assert(memcmp(buffer0, buffer1, lenAlloc0) == 0);
#ifdef __TRUSTINSOFT_PATCH__
  // cherry-pick 028f9048 from gitlab master (new certiticate)
    ck_assert(1069 == lenAlloc0);
#else
    ck_assert(1215 == lenAlloc0);
#endif
    ck_assert(memcmp(buffer0, der_cert, lenAlloc0) == 0);

    // Modifying 0 should not modify 1
    ck_assert(buffer0 != buffer1);

    // Clear
    free(buffer0);
    free(buffer1);
}
END_TEST

Suite* tests_make_suite_crypto_B256()
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
#ifdef __TRUSTINSOFT_SKIP_CRYPTO_TESTS__
    // skip 'test_cert_thumbprint_B256' because
    // it needs to specify valid allocation in 'mbedtls_x509_crt_parse'.
#else
    tcase_add_test(tc_km, test_cert_thumbprint_B256);
#endif
    tcase_add_test(tc_km, test_cert_loadkey_B256);
#ifdef __TRUSTINSOFT_SKIP_CRYPTO_TESTS__
    // skip 'test_cert_copyder_B256' because
    // it needs to specify valid allocation in 'mbedtls_x509_crt_parse'.
#else
    tcase_add_test(tc_km, test_cert_copyder_B256);
#endif

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
