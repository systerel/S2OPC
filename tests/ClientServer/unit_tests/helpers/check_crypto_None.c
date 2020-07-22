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
 * \brief Cryptographic test suite. This suite tests "http://opcfoundation.org/UA/SecurityPolicy#None".
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
static SOPC_CryptoProvider* cryptops = NULL; /* None is supported in both client-server and PubSub cases */

static inline void setup_crypto(void)
{
    crypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
    ck_assert(NULL != crypto);
    cryptops = SOPC_CryptoProvider_CreatePubSub(SOPC_SecurityPolicy_None_URI);
    ck_assert(NULL != cryptops);
}

static inline void teardown_crypto(void)
{
    SOPC_CryptoProvider_Free(crypto);
    crypto = NULL;
    SOPC_CryptoProvider_Free(cryptops);
    cryptops = NULL;
}

START_TEST(test_crypto_load_None)
{
    ck_assert_ptr_null(SOPC_CryptoProvider_GetProfilePubSub(crypto));
    const SOPC_CryptoProfile* profile = SOPC_CryptoProvider_GetProfileServices(crypto);
    ck_assert_ptr_nonnull(profile);

    ck_assert(SOPC_SecurityPolicy_None_ID == profile->SecurityPolicyID);
    ck_assert(NULL == profile->pFnSymmEncrypt);
    ck_assert(NULL == profile->pFnSymmDecrypt);
    ck_assert(NULL == profile->pFnSymmSign);
    ck_assert(NULL == profile->pFnSymmVerif);
    ck_assert(NULL != profile->pFnGenRnd);
    ck_assert(NULL == profile->pFnDeriveData);
    ck_assert(NULL == profile->pFnAsymEncrypt);
    ck_assert(NULL == profile->pFnAsymDecrypt);
    ck_assert(NULL == profile->pFnAsymSign);
    ck_assert(NULL == profile->pFnAsymVerify);
    ck_assert(NULL == profile->pFnCertVerify);

    ck_assert_ptr_null(SOPC_CryptoProvider_GetProfileServices(cryptops));
    const SOPC_CryptoProfile_PubSub* profileps = SOPC_CryptoProvider_GetProfilePubSub(cryptops);
    ck_assert_ptr_nonnull(profileps);

    ck_assert(SOPC_SecurityPolicy_None_ID == profileps->SecurityPolicyID);
    ck_assert(NULL == profileps->pFnCrypt);
    ck_assert(NULL == profileps->pFnSymmSign);
    ck_assert(NULL == profileps->pFnSymmVerif);
    ck_assert(NULL != profileps->pFnGenRnd);
}
END_TEST

START_TEST(test_crypto_symm_lengths_None)
{
    uint32_t len = 0, lenCiph = 0, lenDeci = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SignKey(crypto, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Signature(crypto, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Encryption(crypto, 15, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Decryption(crypto, 15, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(crypto, &len) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, &lenDeci) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, &lenDeci) ==
              SOPC_STATUS_INVALID_PARAMETERS);

    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(cryptops, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SignKey(cryptops, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Signature(cryptops, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Encryption(cryptops, 15, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Decryption(cryptops, 15, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_PubSubGetLength_KeyNonce(cryptops, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_PubSubGetLength_MessageRandom(cryptops, &len) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_symm_crypt_None)
{
    uint32_t local = 0;
    unsigned char* input = (unsigned char*) &local;
    unsigned char* output = (unsigned char*) &local;
    unsigned char* random = (unsigned char*) &local;
    SOPC_SecretBuffer *pSecKey = NULL, *pSecIV = NULL, *pSecNonce = NULL;
    uint32_t uSeqNum = 42;

    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) &input, sizeof(local));
    ck_assert(NULL != pSecKey);
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) &input, sizeof(local));
    ck_assert(NULL != pSecIV);
    pSecNonce = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) &input, sizeof(local));
    ck_assert(NULL != pSecNonce);

    // Encrypt
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 100, pSecKey, pSecNonce, random, 4, uSeqNum, output,
                                              100) == SOPC_STATUS_INVALID_PARAMETERS);

    // Decrypt
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) ==
              SOPC_STATUS_INVALID_PARAMETERS);

    // Assert failure on wrong parameters (TODO: assert attended error code instead of != OK)
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
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(NULL, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, NULL, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 19, pSecKey, pSecNonce, random, 4, uSeqNum, output,
                                              20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 20, NULL, pSecNonce, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 20, pSecKey, NULL, random, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 20, pSecKey, pSecNonce, NULL, 4, uSeqNum, output, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 20, pSecKey, pSecNonce, random, 3, uSeqNum, output,
                                              20) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, NULL, 20) !=
              SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_PubSubCrypt(cryptops, input, 20, pSecKey, pSecNonce, random, 4, uSeqNum, output,
                                              19) != SOPC_STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);
    SOPC_SecretBuffer_DeleteClear(pSecNonce);
}
END_TEST

START_TEST(test_crypto_symm_sign_None)
{
    uint32_t local = 0;
    unsigned char* input = (unsigned char*) &local;
    unsigned char* output = (unsigned char*) &local;
    SOPC_SecretBuffer* pSecKey = NULL;

    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) &input, sizeof(local));
    ck_assert(NULL != pSecKey);

    // Signature
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 20) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(cryptops, input, 64, pSecKey, output, 32) ==
              SOPC_STATUS_INVALID_PARAMETERS);

    // Check verify
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 20) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) ==
              SOPC_STATUS_INVALID_PARAMETERS);

    // Check invalid parameters (TODO: assert attended error code instead of != OK)
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
    ck_assert(SOPC_CryptoProvider_SymmetricSign(cryptops, NULL, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(cryptops, input, 64, NULL, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(cryptops, input, 64, pSecKey, NULL, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(cryptops, input, 64, pSecKey, output, 0) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(cryptops, input, 64, pSecKey, output, 31) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(cryptops, NULL, 64, pSecKey, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(cryptops, input, 64, NULL, output, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(cryptops, input, 64, pSecKey, NULL, 32) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(cryptops, input, 64, pSecKey, output, 0) != SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(cryptops, input, 64, pSecKey, output, 31) != SOPC_STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
}
END_TEST

/* This test is the same for security policy that are not None,
 * as its length is not specified by the policy.
 * It should not fail in None, but this is not required, as it is not used.
 */
START_TEST(test_crypto_generate_nbytes_None)
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
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(cryptops, 64, &pExpBuffer0) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(cryptops, 64, &pExpBuffer1) == SOPC_STATUS_OK);
    ck_assert_msg(memcmp(pExpBuffer0, pExpBuffer1, 64) != 0,
                  "Randomly generated two times the same 64 bytes, which should happen once in pow(2, 512) tries.");
    SOPC_Free(pExpBuffer0);
    SOPC_Free(pExpBuffer1);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(NULL, 64, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 0, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(cryptops, 0, &pExpBuffer0) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(cryptops, 64, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_generate_nonce_None)
{
    SOPC_SecretBuffer* pSecNonce;

    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, &pSecNonce) == SOPC_STATUS_INVALID_PARAMETERS);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(NULL, &pSecNonce) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_generate_uint32_None)
{
    // Yes, you should still be able to generate simple uint32_t in None
    uint32_t i = 0, j = 0;

    // It is random, so you should not have two times the same number (unless you are unlucky (1/2**32)).
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, &i) == SOPC_STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, &j) == SOPC_STATUS_OK);
    ck_assert(i != j);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(NULL, &i) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, NULL) == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_derive_lengths_None)
{
    uint32_t lenCryptoKey = 0, lenSignKey = 0, lenIV = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenCryptoKey, &lenSignKey, &lenIV) ==
              SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_derive_data_None)
{
    SOPC_ExposedBuffer secret[32], seed[32], output[1024];
    uint32_t lenKey, lenKeyBis, lenIV, lenSecr;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenSecr) ==
              SOPC_STATUS_INVALID_PARAMETERS); // TODO: use future GetLength_Nonce

    memset(secret, 0, 32);
    memset(seed, 0, 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 16, output, 100) ==
              SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_derive_keysets_None)
{
    // Keeping arbitrary 32-long buffers
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32], zeros[32];
    uint32_t lenCryptoKey, lenSignKey, lenIV, lenCliNonce, lenSerNonce;
    SOPC_SC_SecurityKeySet cliKS, serKS;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenCryptoKey, &lenSignKey, &lenIV) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenCliNonce) ==
              SOPC_STATUS_INVALID_PARAMETERS); // TODO: use future GetLength_Nonce

    // Prepares security key sets
    memset(zeros, 0, 32);
    lenSignKey = 32;
    lenCryptoKey = 32;
    lenIV = 32;
    lenCliNonce = 32;
    lenSerNonce = 32;
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
                                                &serKS) == SOPC_STATUS_NOK);

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
    uint8_t der_cert[1215];

    setup_crypto();

    // Loads a certificate. This is the server/server.der.
    ck_assert(unhexlify("308204bb308202a3a003020102020106300d06092a864886f70d01010b0500308188310b3009060355040613024652"
                        "310c300a06035504080c03494446310e30"
                        "0c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f50"
                        "43533113301106035504030c0a494e474f"
                        "5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d31"
                        "36313030333038313333385a170d313731"
                        "3030333038313333385a3057310b3009060355040613024652310c300a06035504080c03494446310e300c06035504"
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
                        "03551d23041830168014db180c557814e7"
                        "cffd868827b7b00d28f572abb2300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d0609"
                        "2a864886f70d01010b0500038202010039"
                        "ce25d423f265c38a6df573c1027c6997cc4e5d44db3135ac782180253c6bbdc5017464630d8b17853b214a7866f092"
                        "a25316f296d342df15ccb443392fa914d5"
                        "513a91ddc6112cdb70806e9f89898e911c1928ff5ce9139649a8ae11cef04ec645f2f4aef6187c1f044de6ae884537"
                        "3f9eea33d9148125815ac472f4ab1fe601"
                        "b99ca01cb683005728ef2f588339f33d433db7afbf1e0695ca5fa5ee5fcd5324a41eadf1ef717c90f2920be8361517"
                        "6df11d347a1e291602a66b248578c2648b"
                        "f77009f28c3e0bfdceb7acf2f248939bcb260357db378de10eabcf30432952fb9c5a717fcf75884c697253ff6dca23"
                        "65fcda670921180939e011b195f1190565"
                        "efa25daefe393d8a67261abe881e98264258fef473423d15c3fc5fa87bce0b8c22dff409017842e0c60dfeb5c88ccc"
                        "8005080c803c4935a82d762877b9513584"
                        "6dfd407d49fc3faa523169bfdbbeb5fc5880fed2fa518ee017e42edfa872e781052a47e294c8d82c9858877496dfb7"
                        "6f6bd1c4ab1f0eaa71f48296d88a9950ce"
                        "cc2937b32eaf54eb14fabf84d4519c3e9d5f3434570a24a16f19efa5a7df4a6fc76f317021188b2e39421bb36289f2"
                        "6f71264fd7962eb513030d14b5262b220b"
                        "fa067ba9c1255458d6d570a15f715bc00c2d405809652ac372e2cbc2fdfd7b20681310829ca88ef844ccd8c89a8c5b"
                        "e2bf893c1299380675e82455cbef6ccc",
                        der_cert, 1215) == 1215);
    ck_assert(SOPC_KeyManager_Certificate_CreateOrAddFromDER(der_cert, 1215, &crt_pub) == SOPC_STATUS_OK);
}

static inline void teardown_certificate(void)
{
    SOPC_KeyManager_Certificate_Free(crt_pub);
    crt_pub = NULL;

    teardown_crypto();
}

START_TEST(test_cert_load_None)
{
    ;
}
END_TEST

START_TEST(test_cert_lengths_None)
{
    uint32_t len = 0;

    ck_assert(SOPC_CryptoProvider_CertificateGetLength_Thumbprint(crypto, &len) == SOPC_STATUS_NOK);
}
END_TEST

START_TEST(test_cert_thumbprint_None)
{
    uint8_t thumb[20];

    // ck_assert(KeyManager_Certificate_CreateFromFile(keyman, (int8_t *)"./server_public/server.der", &crt_pub) ==
    // SOPC_STATUS_OK);

    // Compute thumbprint
    ck_assert(SOPC_KeyManager_Certificate_GetThumbprint(crypto, crt_pub, thumb, 20) == SOPC_STATUS_NOK);
}
END_TEST

START_TEST(test_cert_loadkey_None)
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
// in the tests. This is the key used for Basic256. KeyLengths and some lengths should still work with such key.
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

START_TEST(test_crypto_asym_load_None)
{
    ;
}
END_TEST

START_TEST(test_crypto_asym_lengths_None)
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
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(crypto, key_pub, &lenPlain) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(crypto, key_pub, &lenCiph) ==
              SOPC_STATUS_OK); // TODO: this may be weird...
    ck_assert(128 == lenCiph);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_pub, &lenCiph, &lenPlain) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_priv, &lenCiph, &lenPlain) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 32, &len) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 128, &len) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 688, &len) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 1024, &len) == SOPC_STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(crypto, &len) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_pub, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len); // One block
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_priv, &len) == SOPC_STATUS_OK);
    ck_assert(128 == len); // One block
}
END_TEST

START_TEST(test_crypto_asym_crypt_None)
{
    uint8_t input[688], output[1024], input_bis[688];
    uint32_t len = 0;
    const char* errorReason = "";

    // Encryption/Decryption
    memset(input, 0, 688);
    memset(output, 0, 1024);
    strncpy((char*) input, "Test S2OPC Test", 32);
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 32, key_pub, output, 128, &errorReason) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 128, key_priv, input_bis, 86, &len, &errorReason) ==
              SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_asym_sign_verify_None)
{
    uint8_t input[688], sig[128];
    const char* errorReason = "";

    // Signature
    memset(input, 0, 688);
    memset(sig, 0, 128);
    strncpy((char*) input, "Test S2OPC Test", 32);
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 32, key_priv, sig, 128, &errorReason) ==
              SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 32, key_pub, sig, 128, &errorReason) ==
              SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_crypto_asym_copykey_None)
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

START_TEST(test_crypto_asym_uri_None)
{
    ck_assert(SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(crypto) == NULL);
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

START_TEST(test_pki_load_None)
{
    ck_assert(NULL != pki->pFnValidateCertificate);
}
END_TEST

START_TEST(test_pki_cert_validation_None)
{
    uint32_t errorStatus;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_Certificate_Validate(crypto, pki, crt_pub, &errorStatus);
    // Checks that the PKI validates our server.pub with our cacert.der
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
}
END_TEST

START_TEST(test_cert_copyder_None)
{
    uint8_t *buffer0 = NULL, *buffer1 = NULL;
    uint8_t der_cert[1215];
    uint32_t lenAlloc0 = 0, lenAlloc1 = 0;

    // Reference certificate, this is server.der
    ck_assert(unhexlify("308204bb308202a3a003020102020106300d06092a864886f70d01010b0500308188310b3009060355040613024652"
                        "310c300a06035504080c03494446310e30"
                        "0c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f50"
                        "43533113301106035504030c0a494e474f"
                        "5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d31"
                        "36313030333038313333385a170d313731"
                        "3030333038313333385a3057310b3009060355040613024652310c300a06035504080c03494446310e300c06035504"
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
                        "03551d23041830168014db180c557814e7"
                        "cffd868827b7b00d28f572abb2300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d0609"
                        "2a864886f70d01010b0500038202010039"
                        "ce25d423f265c38a6df573c1027c6997cc4e5d44db3135ac782180253c6bbdc5017464630d8b17853b214a7866f092"
                        "a25316f296d342df15ccb443392fa914d5"
                        "513a91ddc6112cdb70806e9f89898e911c1928ff5ce9139649a8ae11cef04ec645f2f4aef6187c1f044de6ae884537"
                        "3f9eea33d9148125815ac472f4ab1fe601"
                        "b99ca01cb683005728ef2f588339f33d433db7afbf1e0695ca5fa5ee5fcd5324a41eadf1ef717c90f2920be8361517"
                        "6df11d347a1e291602a66b248578c2648b"
                        "f77009f28c3e0bfdceb7acf2f248939bcb260357db378de10eabcf30432952fb9c5a717fcf75884c697253ff6dca23"
                        "65fcda670921180939e011b195f1190565"
                        "efa25daefe393d8a67261abe881e98264258fef473423d15c3fc5fa87bce0b8c22dff409017842e0c60dfeb5c88ccc"
                        "8005080c803c4935a82d762877b9513584"
                        "6dfd407d49fc3faa523169bfdbbeb5fc5880fed2fa518ee017e42edfa872e781052a47e294c8d82c9858877496dfb7"
                        "6f6bd1c4ab1f0eaa71f48296d88a9950ce"
                        "cc2937b32eaf54eb14fabf84d4519c3e9d5f3434570a24a16f19efa5a7df4a6fc76f317021188b2e39421bb36289f2"
                        "6f71264fd7962eb513030d14b5262b220b"
                        "fa067ba9c1255458d6d570a15f715bc00c2d405809652ac372e2cbc2fdfd7b20681310829ca88ef844ccd8c89a8c5b"
                        "e2bf893c1299380675e82455cbef6ccc",
                        der_cert, 1215) == 1215);

    // Extract 2 copies from loaded certificate
    ck_assert(SOPC_KeyManager_Certificate_ToDER(crt_pub, &buffer0, &lenAlloc0) == SOPC_STATUS_OK);
    ck_assert(SOPC_KeyManager_Certificate_ToDER(crt_pub, &buffer1, &lenAlloc1) == SOPC_STATUS_OK);

    // Both should be identical, and identical to der_cert
    ck_assert(lenAlloc0 == lenAlloc1);
    ck_assert(memcmp(buffer0, buffer1, lenAlloc0) == 0);
    ck_assert(1215 == lenAlloc0);
    ck_assert(memcmp(buffer0, der_cert, lenAlloc0) == 0);

    // Modifying 0 should not modify 1
    ck_assert(buffer0 != buffer1);

    // Clear
    SOPC_Free(buffer0);
    SOPC_Free(buffer1);
}
END_TEST

Suite* tests_make_suite_crypto_None(void)
{
    Suite* s = NULL;
    TCase *tc_crypto_symm = NULL, *tc_providers = NULL, *tc_rands = NULL, *tc_derives = NULL, *tc_km = NULL,
          *tc_crypto_asym = NULL, *tc_pki_stack = NULL;

    s = suite_create("Crypto tests None (incl. PubSub)");
    tc_crypto_symm = tcase_create("Symmetric Crypto");
    tc_providers = tcase_create("Crypto Provider");
    tc_rands = tcase_create("Random Generation");
    tc_derives = tcase_create("Crypto Data Derivation");
    tc_km = tcase_create("Key Management");
    tc_crypto_asym = tcase_create("Asymmetric Crypto");
    tc_pki_stack = tcase_create("PKI stack");

    suite_add_tcase(s, tc_crypto_symm);
    tcase_add_checked_fixture(tc_crypto_symm, setup_crypto, teardown_crypto);
    tcase_add_test(tc_crypto_symm, test_crypto_load_None);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_lengths_None);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_crypt_None);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_sign_None);

    suite_add_tcase(s, tc_rands);
    tcase_add_checked_fixture(tc_rands, setup_crypto, teardown_crypto);
    tcase_add_test(tc_rands, test_crypto_generate_nbytes_None);
    tcase_add_test(tc_rands, test_crypto_generate_nonce_None);
    tcase_add_test(tc_rands, test_crypto_generate_uint32_None);

    suite_add_tcase(s, tc_providers);
    tcase_add_checked_fixture(tc_providers, setup_crypto, teardown_crypto);

    suite_add_tcase(s, tc_derives);
    tcase_add_checked_fixture(tc_derives, setup_crypto, teardown_crypto);
    tcase_add_test(tc_derives, test_crypto_derive_lengths_None);
    tcase_add_test(tc_derives, test_crypto_derive_data_None);
    tcase_add_test(tc_derives, test_crypto_derive_keysets_None);
    // TODO: derive_keysets_client
    // TODO: derive_keysets_server

    suite_add_tcase(s, tc_km);
    tcase_add_checked_fixture(tc_km, setup_certificate, teardown_certificate);
    tcase_add_test(tc_km, test_cert_load_None);
    tcase_add_test(tc_km, test_cert_lengths_None);
    tcase_add_test(tc_km, test_cert_thumbprint_None);
    tcase_add_test(tc_km, test_cert_loadkey_None);
    tcase_add_test(tc_km, test_cert_copyder_None);

    suite_add_tcase(s, tc_crypto_asym);
    tcase_add_checked_fixture(tc_crypto_asym, setup_asym_keys, teardown_asym_keys);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_load_None);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_lengths_None);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_crypt_None);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_sign_verify_None);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_copykey_None);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_uri_None);

    suite_add_tcase(s, tc_pki_stack);
    tcase_add_checked_fixture(tc_pki_stack, setup_pki_stack, teardown_pki_stack);
    tcase_add_test(tc_pki_stack, test_pki_load_None);
    tcase_add_test(tc_pki_stack, test_pki_cert_validation_None);

    return s;
}
