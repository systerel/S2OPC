/** \file
 *
 * \brief Cryptographic test suite. This suite tests "http://opcfoundation.org/UA/SecurityPolicy#None".
 *
 * See check_stack.c for more details.
 */
/*
 *  Copyright (C) 2016 Systerel and others.
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


#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <check.h>

#include "../../src/helpers_crypto/sopc_crypto_decl.h"
#include "../../src/helpers_crypto/sopc_crypto_profiles.h"
#include "../../src/helpers_crypto/sopc_crypto_provider.h"
#include "../../src/helpers_crypto/sopc_key_manager.h"
#include "../../src/helpers_crypto/sopc_pki_stack.h"
#include "../../src/helpers_crypto/sopc_secret_buffer.h"
#include "check_helpers.h"
#include "hexlify.h"
#include "crypto_provider_lib.h"



// Using fixtures
static SOPC_CryptoProvider *crypto = NULL;

static inline void setup_crypto(void)
{
    crypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
    ck_assert(NULL != crypto);
}

static inline void teardown_crypto(void)
{
    SOPC_CryptoProvider_Free(crypto);
    crypto = NULL;
}


START_TEST(test_crypto_load_None)
{
    ck_assert(NULL != crypto->pProfile);
    ck_assert(SOPC_SecurityPolicy_None_ID == crypto->pProfile->SecurityPolicyID);
    ck_assert(NULL == crypto->pProfile->pFnSymmEncrypt);
    ck_assert(NULL == crypto->pProfile->pFnSymmDecrypt);
    ck_assert(NULL == crypto->pProfile->pFnSymmSign);
    ck_assert(NULL == crypto->pProfile->pFnSymmVerif);
    ck_assert(NULL != crypto->pProfile->pFnGenRnd);
    ck_assert(NULL == crypto->pProfile->pFnDeriveData);
    ck_assert(NULL == crypto->pProfile->pFnAsymEncrypt);
    ck_assert(NULL == crypto->pProfile->pFnAsymDecrypt);
    ck_assert(NULL == crypto->pProfile->pFnAsymSign);
    ck_assert(NULL == crypto->pProfile->pFnAsymVerify);
    ck_assert(NULL == crypto->pProfile->pFnCertVerify);
}
END_TEST


START_TEST(test_crypto_symm_lengths_None)
{
    uint32_t len = 0, lenCiph = 0, lenDeci = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SignKey(crypto, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Signature(crypto, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Encryption(crypto, 15, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Decryption(crypto, 15, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(crypto, &len) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, NULL) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, NULL) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, &lenDeci) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, &lenDeci) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_symm_crypt_None)
{
    uint32_t local = 0;
    unsigned char *input = (unsigned char *)&local;
    unsigned char *output = (unsigned char *)&local;
    SOPC_SecretBuffer *pSecKey = NULL, *pSecIV = NULL;

    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer *)&input, sizeof(local));
    ck_assert(NULL != pSecKey);
    pSecIV = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer *)&input, sizeof(local));
    ck_assert(NULL != pSecIV);

    // Encrypt
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_INVALID_PARAMETERS);

    // Decrypt
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_INVALID_PARAMETERS);

    // Assert failure on wrong parameters (TODO: assert attended error code instead of != OK)
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(NULL, input, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, NULL, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 15, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, NULL, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, NULL, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, NULL, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 15) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(NULL, input, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, NULL, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 15, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, NULL, pSecIV, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, NULL, output, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, NULL, 16) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 15) != STATUS_OK);

    SOPC_SecretBuffer_DeleteClear(pSecKey);
    SOPC_SecretBuffer_DeleteClear(pSecIV);
}
END_TEST


START_TEST(test_crypto_symm_sign_None)
{
    uint32_t local = 0;
    unsigned char *input = (unsigned char *)&local;
    unsigned char *output = (unsigned char *)&local;
    SOPC_SecretBuffer *pSecKey = NULL;

    pSecKey = SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer *)&input, sizeof(local));
    ck_assert(NULL != pSecKey);

    // Signature
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 20) == STATUS_INVALID_PARAMETERS);

    // Check verify
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 20) == STATUS_INVALID_PARAMETERS);

    // Check invalid parameters (TODO: assert attended error code instead of != OK)
    ck_assert(SOPC_CryptoProvider_SymmetricSign(NULL, input, 64, pSecKey, output, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, NULL, 64, pSecKey, output, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, NULL, output, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, NULL, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 0) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 32) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(NULL, input, 64, pSecKey, output, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, NULL, 64, pSecKey, output, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, NULL, output, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, NULL, 20) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 0) != STATUS_OK);
    ck_assert(SOPC_CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) != STATUS_OK);

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
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, &pExpBuffer0) == STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, &pExpBuffer1) == STATUS_OK);
    // You have a slight chance to fail here (1/(2**512))
    ck_assert_msg(memcmp(pExpBuffer0, pExpBuffer1, 64) != 0,
                  "Randomly generated two times the same 64 bytes, which should happen once in pow(2, 512) tries.");
    free(pExpBuffer0);
    free(pExpBuffer1);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(NULL, 64, &pExpBuffer0) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 0, &pExpBuffer0) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomBytes(crypto, 64, NULL) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_generate_nonce_None)
{
    SOPC_SecretBuffer *pSecNonce;

    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, &pSecNonce) == STATUS_NOK);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(NULL, &pSecNonce) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateSecureChannelNonce(crypto, NULL) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_generate_uint32_None)
{
    // Yes, you should still be able to generate simple uint32_t in None
    uint32_t i = 0, j = 0;

    // It is random, so you should not have two times the same number (unless you are unlucky (1/2**32)).
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, &i) == STATUS_OK);
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, &j) == STATUS_OK);
    ck_assert(i != j);

    // Test invalid inputs
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(NULL, &i) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_GenerateRandomID(crypto, NULL) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_derive_lengths_None)
{
    uint32_t lenCryptoKey = 0, lenSignKey = 0, lenIV = 0;

    // Check sizes
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenCryptoKey, &lenSignKey, &lenIV) == STATUS_NOK);
}
END_TEST


START_TEST(test_crypto_derive_data_None)
{
    SOPC_ExposedBuffer secret[32], seed[32], output[1024];
    uint32_t lenKey, lenKeyBis, lenIV, lenSecr;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenSecr) == STATUS_NOK); // TODO: use future GetLength_Nonce

    memset(secret, 0, 32);
    memset(seed, 0, 32);
    ck_assert(SOPC_CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 16, output, 100) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_derive_keysets_None)
{
    // Keeping arbitrary 32-long buffers
    SOPC_ExposedBuffer clientNonce[32], serverNonce[32], zeros[32];
    uint32_t lenCryptoKey, lenSignKey, lenIV, lenCliNonce, lenSerNonce;
    SOPC_SC_SecurityKeySet cliKS, serKS;

    // Context init
    ck_assert(SOPC_CryptoProvider_DeriveGetLengths(crypto, &lenCryptoKey, &lenSignKey, &lenIV) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(crypto, &lenCliNonce) == STATUS_NOK); // TODO: use future GetLength_Nonce

    // Prepares security key sets
    memset(zeros, 0, 32);
    lenSignKey = 32;
    lenCryptoKey = 32;
    lenIV = 32;
    lenCliNonce = 32;
    lenSerNonce = 32;
    ck_assert(NULL != (cliKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenSignKey)));
    ck_assert(NULL != (cliKS.encryptKey= SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenCryptoKey)));
    ck_assert(NULL != (cliKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV)));
    ck_assert(NULL != (serKS.signKey = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenSignKey)));
    ck_assert(NULL != (serKS.encryptKey= SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenCryptoKey)));
    ck_assert(NULL != (serKS.initVector = SOPC_SecretBuffer_NewFromExposedBuffer(zeros, lenIV)));

    // These come from a stub_client working with OPC foundation code (e.g. commit "Bugfix: used CryptoKey instead of SignKey")
    ck_assert(unhexlify("26353d1e608669d81dcc1ca7ca1f7e2b0aac53166d512a6f09527fbe54b114b5", clientNonce, lenCliNonce) == (int32_t)lenCliNonce);
    ck_assert(unhexlify("0928c7fe64e3bfcfb99ffd396f1fb6d6048778a9ec70114c400753ee9af66ec6", serverNonce, lenSerNonce) == (int32_t)lenSerNonce);
    ck_assert(SOPC_CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS, &serKS) == STATUS_NOK);

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
static SOPC_Certificate *crt_pub = NULL;

static inline void setup_certificate(void)
{
    uint8_t der_cert[1215];

    setup_crypto();

    // Loads a certificate. This is the server/server.der.
    ck_assert(unhexlify("308204bb308202a3a003020102020106300d06092a864886f70d01010b0500308188310b3009060355040613024652310c300a06035504080c03494446310e30"
                        "0c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f"
                        "5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d3136313030333038313333385a170d313731"
                        "3030333038313333385a3057310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533111300f060355040a"
                        "0c08535953544552454c3117301506035504030c0e494e474f5043535f53455256455230820122300d06092a864886f70d01010105000382010f003082010a02"
                        "82010100ad9921f924639e125c0cde520755f44028d65eaecaf16867823be446b977e0631d64509953b7fe467d1afc449bca6edfe11e1e6d71207c33e2250f3c"
                        "66875d369a1cda02efc661e73bdf01c517470f2a09ea500b56842fcb125779917b8deb58dc6f2f9511e66c29ba57a69435bc3aab1a23982f531ec763f494ef8b"
                        "6c6360ea194d7ca2efd777b9a32c295809cf39d2c2ed0dbfc4bfd6fbd24bf782f8d83795cb51964e1dd0a8cdd8f2a0ef2fd0d2b126eb8fc00f00411f362cd4e3"
                        "0a0a20cde108efa69faede8d9f756838306569c6ea27f1ba5aefac790ff18bcbcc81d7acaa1fac2acede3acd2a61d7b62f202c7bab7df08ee2241a0f08dffdb6"
                        "2914cf210203010001a360305e301d0603551d0e04160414a3f8e031d1f6f412bace4ddf0eeb62da209d3c79301f0603551d23041830168014db180c557814e7"
                        "cffd868827b7b00d28f572abb2300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d06092a864886f70d01010b0500038202010039"
                        "ce25d423f265c38a6df573c1027c6997cc4e5d44db3135ac782180253c6bbdc5017464630d8b17853b214a7866f092a25316f296d342df15ccb443392fa914d5"
                        "513a91ddc6112cdb70806e9f89898e911c1928ff5ce9139649a8ae11cef04ec645f2f4aef6187c1f044de6ae8845373f9eea33d9148125815ac472f4ab1fe601"
                        "b99ca01cb683005728ef2f588339f33d433db7afbf1e0695ca5fa5ee5fcd5324a41eadf1ef717c90f2920be83615176df11d347a1e291602a66b248578c2648b"
                        "f77009f28c3e0bfdceb7acf2f248939bcb260357db378de10eabcf30432952fb9c5a717fcf75884c697253ff6dca2365fcda670921180939e011b195f1190565"
                        "efa25daefe393d8a67261abe881e98264258fef473423d15c3fc5fa87bce0b8c22dff409017842e0c60dfeb5c88ccc8005080c803c4935a82d762877b9513584"
                        "6dfd407d49fc3faa523169bfdbbeb5fc5880fed2fa518ee017e42edfa872e781052a47e294c8d82c9858877496dfb76f6bd1c4ab1f0eaa71f48296d88a9950ce"
                        "cc2937b32eaf54eb14fabf84d4519c3e9d5f3434570a24a16f19efa5a7df4a6fc76f317021188b2e39421bb36289f26f71264fd7962eb513030d14b5262b220b"
                        "fa067ba9c1255458d6d570a15f715bc00c2d405809652ac372e2cbc2fdfd7b20681310829ca88ef844ccd8c89a8c5be2bf893c1299380675e82455cbef6ccc", der_cert, 1215) == 1215);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_cert, 1215, &crt_pub) == STATUS_OK);
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

    ck_assert(SOPC_CryptoProvider_CertificateGetLength_Thumbprint(crypto, &len) == STATUS_NOK);
}
END_TEST


START_TEST(test_cert_thumbprint_None)
{
    uint8_t thumb[20];

    //ck_assert(KeyManager_Certificate_CreateFromFile(keyman, (int8_t *)"./server_public/server.der", &crt_pub) == STATUS_OK);

    // Compute thumbprint
    ck_assert(SOPC_KeyManager_Certificate_GetThumbprint(crypto, crt_pub, thumb, 20) == STATUS_NOK);

}
END_TEST


START_TEST(test_cert_loadkey_None)
{
    SOPC_AsymmetricKey* key_pub = NULL;

    // Loads the public key from cert
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(crt_pub, &key_pub) == STATUS_OK);

    SOPC_KeyManager_AsymmetricKey_Free(key_pub);
}
END_TEST


// Fixtures for Asymetric crypto
static SOPC_AsymmetricKey *key_pub = NULL, *key_priv = NULL;

// Certificates: these are not the same cert as in setup_certificate. This one was created to also embed the private key in the tests.
// This is the key used for Basic256. KeyLengths and some lengths should still work with such key.
#define DER_ASYM_PUB_HEXA "30820286308201efa003020102020900c4d03aaaf2bbde98300d06092a864886f70d01010b0500305c310b3009060355040613024652310f300d06035504080c"\
                          "064672616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504030c12494e474f504353205465737420"\
                          "7375697465301e170d3136313132353137353033385a170d3137303330353137353033385a305c310b3009060355040613024652310f300d06035504080c0646"\
                          "72616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504030c12494e474f5043532054657374207375"\
                          "69746530819f300d06092a864886f70d010101050003818d0030818902818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b0db5d668a5f10ec8"\
                          "d17828919b436f64cc2540ea246d9bcae0d891a4983d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01eefca8e3ae6ec2878a77bd960ed"\
                          "c5acf1ea7f8af18a6de2f0a6b4f8c52da4a70816718c20da7dfa629fc9e613510203010001a350304e301d0603551d0e041604142417c23c516564e10e04671a"\
                          "234ca8939b492a5d301f0603551d230418301680142417c23c516564e10e04671a234ca8939b492a5d300c0603551d13040530030101ff300d06092a864886f7"\
                          "0d01010b050003818100630e6c7d008cc1b681608231dbb28d274683498c654a8244c2fc1411c4d239a0512803bf4dfbd2f1ddd96a83f7de68c4fd6b8df80782"\
                          "043dcb8d80a27021dd03b2f403c06ebd45b9e75a5a06715283c6796988bb23fce0749c8aecee4cc205cb423ebfb84f57759e1b02cc48e89cd3cb90b27e2785d7"\
                          "a22ebee8bf0988edec28"
#define DER_ASYM_PUB_LENG 650
#define DER_ASYM_PRIV_HEXA "3082025e02010002818100ec5d569bf77931e401c07a030921301a74cfe6a3f994175b0db5d668a5f10ec8d17828919b436f64cc2540ea246d9bcae0d891a498"\
                           "3d39f9342218a2c87836824e98fe63814429119f98bcfc4b81fc9946f01eefca8e3ae6ec2878a77bd960edc5acf1ea7f8af18a6de2f0a6b4f8c52da4a7081671"\
                           "8c20da7dfa629fc9e613510203010001028181008e8f9d7564c60c7961351e62465766140ef07643e07c99b9a9834b56c2ffa9d325c43b73d719cd4e167341bb"\
                           "f74cc4f290bb0edd1f958e29e86fc83c267d9b21c45a7618c4c5ca124e2dd8bbb2828669f57a9dc5395f4ce49f7afb251ddb4ebe97cadf648f26fc850e2587d7"\
                           "3bd86bbda4769615de4fcbc4de6b1d9cf15b8d81024100f6401a1f0eae030171ae99edf82e708fb46912889189315ad27c759a75207cc0f11d129aa995393174"\
                           "a045fb29a6476487e6cd92242978729e2136d5ce953f65024100f5b909a77e0bd1de7d9979429508e17f4339fbe05cffd0c9d4c3ee886f39183bb5ac452d3c36"\
                           "68a2af1a01fe435bc4ad14be9b1dbd359eca5a89aa923001337d024012d2296cf0414a8784b9d498049d000b6bbd902612018b5d26b34e85c4a7fc00ff2cbaac"\
                           "4983d740396aba8e8ccb61af845796a4b1d0dd9cdd0b2ad6c29853a5024100f34a19fcf417cfdb72901a378a4818bc605b70bf5c550cec48f5159f903fff765f"\
                           "120a0c17a9e73fec0edc1a5ba6e8bc55e5c2bf572f57e112736ba70250ae21024100ca81e8f4360563e0721f3a2227be2f52141806f398801e3f7cba4c1e960b"\
                           "f7b53ff4babc92075300f69c8d1f2e56738165896558707f1831bd84f929cf12fb51"
#define DER_ASYM_PRIV_LENG 610

static inline void setup_asym_keys(void)
{
    uint8_t der_cert[DER_ASYM_PUB_LENG], der_priv[DER_ASYM_PRIV_LENG];

    setup_crypto();

    // Loads certificate from DER
    ck_assert(unhexlify(DER_ASYM_PUB_HEXA, der_cert, DER_ASYM_PUB_LENG) == DER_ASYM_PUB_LENG);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_cert, DER_ASYM_PUB_LENG, &crt_pub) == STATUS_OK);//*/

    // Loads the public key from cert
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(crt_pub, &key_pub) == STATUS_OK);

    // Loads the corresponding private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);
    ck_assert(SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(der_priv, DER_ASYM_PRIV_LENG, &key_priv) == STATUS_OK);
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


START_TEST(test_crypto_asym_load_None)
{
    ;
}
END_TEST


START_TEST(test_crypto_asym_lengths_None)
{
    uint32_t lenPlain = 0, lenCiph = 0, len = 0;

    // Check lengths
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_pub, &len) == STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_priv, &len) == STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_pub, &len) == STATUS_OK);
    ck_assert(128 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_priv, &len) == STATUS_OK);
    ck_assert(128 == len);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(crypto, key_pub, &lenPlain) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(crypto, key_pub, &lenCiph) == STATUS_OK); // TODO: this may be weird...
    ck_assert(128 == lenCiph);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_pub, &lenCiph, &lenPlain) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_priv, &lenCiph, &lenPlain) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 32, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 128, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 688, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 1024, &len) == STATUS_NOK);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(crypto, &len) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_PSSHashLength(crypto, &len) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_pub, &len) == STATUS_OK);
    ck_assert(128 == len); // One block
    ck_assert(SOPC_CryptoProvider_AsymmetricGetLength_Signature(crypto, key_priv, &len) == STATUS_OK);
    ck_assert(128 == len); // One block
}
END_TEST


START_TEST(test_crypto_asym_crypt_None)
{
    uint8_t input[688], output[1024], input_bis[688];
    uint32_t len = 0;

    // Encryption/Decryption
    memset(input, 0, 688);
    memset(output, 0, 1024);
    strncpy((char *)input, "Test INGOPCS Test", 32);
    ck_assert(SOPC_CryptoProvider_AsymmetricEncrypt(crypto, input, 32, key_pub, output, 128) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricDecrypt(crypto, output, 128, key_priv, input_bis, 86, &len) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_asym_sign_verify_None)
{
    uint8_t input[688], sig[128];

    // Signature
    memset(input, 0, 688);
    memset(sig, 0, 128);
    strncpy((char *)input, "Test INGOPCS Test", 32);
    ck_assert(SOPC_CryptoProvider_AsymmetricSign(crypto, input, 32, key_priv, sig, 128) == STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_CryptoProvider_AsymmetricVerify(crypto, input, 32, key_pub, sig, 128) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_crypto_asym_copykey_None)
{
    uint8_t buffer[2048], der_priv[DER_ASYM_PRIV_LENG];
    uint32_t lenDER = 0;

    // Copy to DER
    ck_assert(SOPC_KeyManager_AsymmetricKey_ToDER(key_priv, buffer, 2048, &lenDER) == STATUS_OK);

    // Loads DER of private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);

    // Verifies
    ck_assert(lenDER == DER_ASYM_PRIV_LENG);
    ck_assert(memcmp(buffer, der_priv, DER_ASYM_PRIV_LENG) == 0);
}
END_TEST


START_TEST(test_crypto_asym_uri_None)
{
    ck_assert(SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(crypto) == NULL);
}
END_TEST


// Fixtures for PKI: server.der certificate and CA
static SOPC_Certificate *crt_ca = NULL;
static SOPC_PKIProvider *pki = NULL;

static inline void setup_pki_stack()
{
    uint8_t der_ca[1529];

    setup_certificate();

    // Loads CA cert which signed server.der. This is trusted/cacert.der.
    ck_assert(unhexlify("308205f5308203dda003020102020900e90749109a17369b300d06092a864886f70d01010b0500308188310b3009060355040613024652310c300a0603550408"
                        "0c03494446310e300c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f50435331133011060355"
                        "04030c0a494e474f5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d31363035313931343533"
                        "30345a170d3137303531393134353330345a308188310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c055041524953"
                        "3110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a"
                        "864886f70d0109011613696e676f70637340737973746572656c2e667230820222300d06092a864886f70d01010105000382020f003082020a0282020100abed"
                        "f46dfda704bf8335fb15dcc4e506bcd788085db31484cab130d470dcb2b7cf79a8a173312ba70ba3f4c8db69dd2f321f72784b23552b57bf7d4a40e1d92a73ed"
                        "ea41347bcdd48e9d14e83bfb6b462d272fc768063df0335e4db34b8bb98de42ce4f9be0927ddcd6e1906bc8ea1e66b5f115fe08576f4be4e0f09c43ae11ec291"
                        "d7573a260095754a3df53c6c8e96842f46adf87984e7529c535a818d05db40a683301f9a9cc69d511c6eaf409df6925ff1693d33dbb01622369d678b5731b774"
                        "b09796eb91f49064f7f93932599f66bdf55362bf39ddee38ec311e921b7f5f7840c67314664a9cedb2a922e9adce5ca9caeb734df90dbf1c5a472e1b4a57bb9a"
                        "bc77c84d9a02bfc6bb21e70d69836e93f6fd6dddb14f7bc13a20e279ebaeb22d8bc96984b6427c686b2b4fa44dd1fec1d534c19baf6f7c2794fb3019276d3929"
                        "e949670ef6da4667b8c54c5d2dedd430c6aca907a76ed8d8ec0809af203e64a0b5321af3fb636cf3aadbdbdcf6cb18dcb085bd9a38328b7f96f8e59498650fd1"
                        "67ea8277cf552eb8e33ac3e68ac8351b4c5f673732d7e4f972889e2ae38b4700e6d675a7a720ef9a264cdca78090ada9873656fa81463d59ec5b053ac73b066e"
                        "83e46d7248cfa47545bebed6885538d9ca87ed0761ff121f85544e6663f0ce4376fa03dc95edb16b0299eb0981ff9231080e881a6a16ecb424de0f4da7990203"
                        "010001a360305e301d0603551d0e04160414db180c557814e7cffd868827b7b00d28f572abb2301f0603551d23041830168014db180c557814e7cffd868827b7"
                        "b00d28f572abb2300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d06092a864886f70d01010b05000382020100a1c3776b6048f0"
                        "658860386ad7b45daba729bf9be81c5ca217a496dbcb0663ecf70e9d7f45c818f9574f8b0c53a378927ddec7ec81a2db09994f4cad32421cbdc23dd5cd1c52ae"
                        "98c8073da99a333c7ba91691339ae50457c3be352d34af45d93c25107065b3d7652e02ba1a80bea8501d8817186c6ecc7f28cfd903aa74926278668d2f6504ff"
                        "1491e024aab85e00d700d51d846655660e4ec59c225cec51b51150e91dba37ae953612758b5e79ca7c6ad56bd835bc4be28f95c5e2e34ab843fa569ff3f075ca"
                        "85d9d18715109a835478fde87368f0a8ab372a01a671ef307ec60564b031561806bb9a8c614aa480e1e1340c1eb67d5ace997996721c18016e3ac00f67e92499"
                        "b51ffef8d1f0f492b6209f41dff2c36507bdcb3b2ca36d24406444c48fbefa996801fd0611c6050745c15305547510814febcc39567b0fee022d380c6e8479bf"
                        "9018106a023e121848a1b6c30052e4f22d43dcc44896b6d2acfc63916b2e7eb0eb4c5061e9a09c50c8a81c293ef121a7b71d35bdca67b3d6c5bedc868c4511cb"
                        "06348fcc19015025e7dfd53d94fe46f7358e0c3dbb3929583001dc1a88d848e4ef229f3cf882f52a06641facd14529a39c4625ad43c7f7b9e1e9496f5ffcb219"
                        "b146d7ce56ad379adf4d2da72e7f1d7338e3b21df188c51d19b89a090ca514c7723213af58af2151e10890f23851030f801d0e241038462d3a", der_ca, 1529) == 1529);
    ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(der_ca, 1529, &crt_ca) == STATUS_OK);

    // Creates PKI with ca
    ck_assert(SOPC_PKIProviderStack_Create(crt_ca, NULL, &pki) == STATUS_OK);
}

static inline void teardown_pki_stack()
{
    SOPC_PKIProviderStack_Free(pki);
    SOPC_KeyManager_Certificate_Free(crt_ca);

    teardown_certificate();
}


START_TEST(test_pki_load_None)
{
    ck_assert(NULL != pki->pFnValidateCertificate);
}
END_TEST


START_TEST(test_pki_cert_validation_None)
{
    // Checks that the PKI validates our server.pub with our cacert.der
    ck_assert(SOPC_CryptoProvider_Certificate_Validate(crypto, pki, crt_pub) == STATUS_INVALID_PARAMETERS);
}
END_TEST


START_TEST(test_cert_copyder_None)
{
    uint8_t *buffer0 = NULL, *buffer1 = NULL;
    uint8_t der_cert[1215];
    uint32_t lenAlloc0 = 0, lenAlloc1 = 0;

    // Reference certificate, this is server.der
    ck_assert(unhexlify("308204bb308202a3a003020102020106300d06092a864886f70d01010b0500308188310b3009060355040613024652310c300a06035504080c03494446310e30"
                        "0c06035504070c0550415249533110300e060355040a0c07494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f"
                        "5043532043413122302006092a864886f70d0109011613696e676f70637340737973746572656c2e6672301e170d3136313030333038313333385a170d313731"
                        "3030333038313333385a3057310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533111300f060355040a"
                        "0c08535953544552454c3117301506035504030c0e494e474f5043535f53455256455230820122300d06092a864886f70d01010105000382010f003082010a02"
                        "82010100ad9921f924639e125c0cde520755f44028d65eaecaf16867823be446b977e0631d64509953b7fe467d1afc449bca6edfe11e1e6d71207c33e2250f3c"
                        "66875d369a1cda02efc661e73bdf01c517470f2a09ea500b56842fcb125779917b8deb58dc6f2f9511e66c29ba57a69435bc3aab1a23982f531ec763f494ef8b"
                        "6c6360ea194d7ca2efd777b9a32c295809cf39d2c2ed0dbfc4bfd6fbd24bf782f8d83795cb51964e1dd0a8cdd8f2a0ef2fd0d2b126eb8fc00f00411f362cd4e3"
                        "0a0a20cde108efa69faede8d9f756838306569c6ea27f1ba5aefac790ff18bcbcc81d7acaa1fac2acede3acd2a61d7b62f202c7bab7df08ee2241a0f08dffdb6"
                        "2914cf210203010001a360305e301d0603551d0e04160414a3f8e031d1f6f412bace4ddf0eeb62da209d3c79301f0603551d23041830168014db180c557814e7"
                        "cffd868827b7b00d28f572abb2300f0603551d130101ff040530030101ff300b0603551d0f040403020106300d06092a864886f70d01010b0500038202010039"
                        "ce25d423f265c38a6df573c1027c6997cc4e5d44db3135ac782180253c6bbdc5017464630d8b17853b214a7866f092a25316f296d342df15ccb443392fa914d5"
                        "513a91ddc6112cdb70806e9f89898e911c1928ff5ce9139649a8ae11cef04ec645f2f4aef6187c1f044de6ae8845373f9eea33d9148125815ac472f4ab1fe601"
                        "b99ca01cb683005728ef2f588339f33d433db7afbf1e0695ca5fa5ee5fcd5324a41eadf1ef717c90f2920be83615176df11d347a1e291602a66b248578c2648b"
                        "f77009f28c3e0bfdceb7acf2f248939bcb260357db378de10eabcf30432952fb9c5a717fcf75884c697253ff6dca2365fcda670921180939e011b195f1190565"
                        "efa25daefe393d8a67261abe881e98264258fef473423d15c3fc5fa87bce0b8c22dff409017842e0c60dfeb5c88ccc8005080c803c4935a82d762877b9513584"
                        "6dfd407d49fc3faa523169bfdbbeb5fc5880fed2fa518ee017e42edfa872e781052a47e294c8d82c9858877496dfb76f6bd1c4ab1f0eaa71f48296d88a9950ce"
                        "cc2937b32eaf54eb14fabf84d4519c3e9d5f3434570a24a16f19efa5a7df4a6fc76f317021188b2e39421bb36289f26f71264fd7962eb513030d14b5262b220b"
                        "fa067ba9c1255458d6d570a15f715bc00c2d405809652ac372e2cbc2fdfd7b20681310829ca88ef844ccd8c89a8c5be2bf893c1299380675e82455cbef6ccc", der_cert, 1215) == 1215);

    // Extract 2 copies from loaded certificate
    ck_assert(SOPC_KeyManager_Certificate_CopyDER(crt_pub, &buffer0, &lenAlloc0) == STATUS_OK);
    ck_assert(SOPC_KeyManager_Certificate_CopyDER(crt_pub, &buffer1, &lenAlloc1) == STATUS_OK);

    // Both should be identical, and identical to der_cert
    ck_assert(lenAlloc0 == lenAlloc1);
    ck_assert(memcmp(buffer0, buffer1, lenAlloc0) == 0);
    ck_assert(1215 == lenAlloc0);
    ck_assert(memcmp(buffer0, der_cert, lenAlloc0) == 0);

    // Modifying 0 should not modify 1
    ck_assert(buffer0 != buffer1);

    // Clear
    free(buffer0);
    free(buffer1);
}
END_TEST


Suite *tests_make_suite_crypto_None()
{
    Suite *s = NULL;
    TCase *tc_crypto_symm = NULL, *tc_providers = NULL, *tc_rands = NULL, *tc_derives = NULL, *tc_km = NULL, *tc_crypto_asym = NULL, *tc_pki_stack = NULL;

    s = suite_create("Crypto tests None");
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

