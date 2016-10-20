/*
 * Crypto test suite. See check_stack.c for more details.
 *
 *  Created on: Sep 27, 2016
 *      Author: PAB
 */


#include <stdio.h>
#include <string.h>
#include <check.h>
#include <stddef.h> // NULL

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "check_stack.h"
#include "crypto_provider.h"
#include "ua_base_types.h"
#include "crypto_profiles.h"
#include "crypto_types.h"
#include "secret_buffer.h"
#include "crypto_provider_lib.h"
#include "key_manager.h"


// Helper
// You should allocate strlen(src)*2 in dst. n is strlen(src)
// Returns n the number of translated chars (< 0 for errors)
int hexlify(const unsigned char *src, char *dst, size_t n)
{
    size_t i;

    if(! src || ! dst)
        return -1;

    for(i=0; i<n; ++i)
        sprintf(&dst[2*i], "%02hhx", src[i]);

    return n;
}

// Helper
// You should allocate strlen(src)/2 in dst. n is strlen(dst)
// Returns n the number of translated couples (< 0 for errors)
int unhexlify(const char *src, unsigned char *dst, size_t n)
{
    size_t i;

    if(! src || ! dst)
        return -1;

    for(i=0; i<n; ++i)
    {
        if(sscanf(&src[2*i], "%02hhx", &dst[i]) < 1)
            return i;
    }

    return n;
}


START_TEST(test_hexlify)
{
    unsigned char buf[32], c, d;
    int i;

    // Test single chars
    for(i=0; i<256; ++i)
    {
        c = (unsigned char)i;
        ck_assert(hexlify(&c, (char *)buf, 1) == 1);
        ck_assert(unhexlify((char *)buf, &d, 1) == 1);
        ck_assert(c == d);
    }

    // Test vector
    ck_assert(hexlify((unsigned char *)"\x00 Test \xFF", (char *)buf, 8) == 8);
    ck_assert(strncmp((char *)buf, "00205465737420ff", 16) == 0);
    ck_assert(unhexlify((char *)buf, buf+16, 8) == 8);
    ck_assert(strncmp((char *)(buf+16), "\x00 Test \xFF", 8) == 0);
}
END_TEST


START_TEST(test_crypto_symm_crypt)
{
    // TODO: these tests test only Basic256Sha256

    // Tests based on the test vectors provided by the NIST
    //  (http://csrc.nist.gov/groups/STM/cavp/block-ciphers.html#aes)
    unsigned char key[32];
    unsigned char iv[16];
    unsigned char input[128];
    unsigned char output[128];
    char hexoutput[256];
    int i;
    uint32_t len;
    CryptoProvider *crypto = NULL;
    SecretBuffer *pSecKey = NULL, *pSecIV = NULL;

    // Context init
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &len) == STATUS_OK);
    ck_assert(len == 32);

    // Encrypt
    // This single test is not taken from the NIST test vectors...
    memset(key, 0, sizeof(key));
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "dc95c078a2408989ad48a21492842087", 32) == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    ck_assert(unhexlify("c47b0294dbbbee0fec4757f22ffeee3587ca4730c3d33b691df38bab076bc558", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "46f2fb342d6f0ab477476fc501242c5f", 32) == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    ck_assert(unhexlify("ccd1bc3c659cd3c59bc437484e3c5c724441da8d6e90ce556cd57d0752663bbc", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "304f81ab61a80c2e743b94d5002a126b", 32) == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    memset(key, 0, sizeof(key));
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("0b24af36193ce4665f2825d7b4749c98", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "a9ff75bd7cf6613d3731c77c3b6d0c04", 32) == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    // Decrypt
    ck_assert(unhexlify("28d46cffa158533194214a91e712fc2b45b518076675affd910edeca5f41ac64", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("4bf3b0a69aeb6657794f2901b1440ad4", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    for(i=0; i<16; ++i)
        ck_assert(output[i] == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("47cb030da2ab051dfc6c4bf6910d12bb", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    for(i=0; i<16; ++i)
        ck_assert(output[i] == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    memset(key, 0, sizeof(key));
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("623a52fcea5d443e48d9181ab32c7421", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "761c1fe41a18acf20d241650611d90f1", 32) == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    // Encrypt + Decrypt
    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "47cb030da2ab051dfc6c4bf6910d12bb", 32) == 0);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, output, 16, pSecKey, pSecIV, input, 16) == STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "00000000000000000000000000000000", 32) == 0);
    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);

    // Multi-block messages
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d"
                        "4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa", input, 128) == 128);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 128, pSecKey, pSecIV, output, 128) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput, "90b7b9630a2378f53f501ab7beff039155008071bc8438e789932cfd3eb1299195465e6633849463fdb44375278e2fdb1310821e6492cf80ff15cb772509fb42"
                                "6f3aeee27bd4938882fd2ae6b5bd9d91fa4a43b17bb439ebbe59c042310163a82a5fe5388796eee35a181a1271f00be29b852d8fa759bad01ff4678f010594cd", 256) == 0);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, output, 128, pSecKey, pSecIV, input, 128) == STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput, "256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d"
                                "4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa", 256) == 0);
    // Here we keep the SecretBuffers of key and iv for the following tests


    // Assert failure on wrong parameters
    ck_assert(CryptoProvider_SymmetricEncrypt(NULL, input, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, NULL, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 15, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, NULL, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, NULL, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, NULL, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 15) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(NULL, input, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, NULL, 16, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 15, pSecKey, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, NULL, pSecIV, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, NULL, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, NULL, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 15) != STATUS_OK);

    SecretBuffer_DeleteClear(pSecKey);
    SecretBuffer_DeleteClear(pSecIV);
    CryptoProvider_Delete(crypto);
}
END_TEST


START_TEST(test_crypto_symm_sign)
{
    // TODO: these tests test only Basic256Sha256
    unsigned char key[256];
    unsigned char input[256];
    unsigned char output[32];
    char hexoutput[1024];
    CryptoProvider *crypto = NULL;
    uint32_t len;
    SecretBuffer *pSecKey = NULL;

    // Context init
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &len) == STATUS_OK);
    ck_assert(len == 32);
    ck_assert(CryptoProvider_SymmetricGetLength_Signature(crypto, &len) == STATUS_OK);
    ck_assert(len == 32);

    // Test cases of https://tools.ietf.org/html/rfc4231 cannot be used for Basic256Sha256
    // Test cases of http://csrc.nist.gov/groups/STM/cavp/message-authentication.html#hmac cannot be used either, as there is no corresponding key_length=32 and sig_length=32
    // So this is a test case from an informal source (Python3 Crypto module and online)
    // The text is obtained by concatenating sha256 hashes of the strings "InGoPcS" and "iNgOpCs",
    //  and the key is obtained by sha256 hashing "INGOPCS".
    // Python code: HMAC.new(SHA256.new(b"INGOPCS").digest(), SHA256.new(b"InGoPcS").digest()+SHA256.new(b"iNgOpCs").digest(), SHA256).hexdigest()
    memset(input, 0, sizeof(input));
    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("ec7b07fb4f3a6b87ca8cff06ba9e0ec619a34a2d9618dc2a02bde67709ded8b4e7069d582665f23a361324d1f84807e30d2227b266c287cc342980d62cb53017", input, 64) == 64);
    ck_assert(unhexlify("7203d5e504eafe00e5dd77519eb640de3bbac660ec781166c4d460362a94c372", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, 32);
    ck_assert(NULL != pSecKey);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 32) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 32) == 32);
    ck_assert(memcmp(hexoutput, "e4185b6d49f06e8b94a552ad950983852ef20b58ee75f2c448fea587728d94db", 64) == 0);

    // Check verify
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == STATUS_OK);
    output[1] ^= 0x20; // Change 1 bit
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == STATUS_NOK);
    output[1] ^= 0x20; // Revert changed bit
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == STATUS_OK);
    output[31] = 0x04; // Change 1 bit in last byte
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 32) == STATUS_NOK);

    // Check invalid parameters
    ck_assert(CryptoProvider_SymmetricSign(NULL, input, 64, pSecKey, output, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign(crypto, NULL, 64, pSecKey, output, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign(crypto, input, 64, NULL, output, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, NULL, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 0) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign(crypto, input, 64, pSecKey, output, 31) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify(NULL, input, 64, pSecKey, output, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify(crypto, NULL, 64, pSecKey, output, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, NULL, output, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, NULL, 32) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 0) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify(crypto, input, 64, pSecKey, output, 31) != STATUS_OK);

    SecretBuffer_DeleteClear(pSecKey);
    CryptoProvider_Delete(crypto);
}
END_TEST


START_TEST(test_crypto_symm_gen)
{
    // TODO: these tests test only Basic256Sha256
    SecretBuffer *pSecKey0, *pSecKey1;
    ExposedBuffer *pExpKey0, *pExpKey1;
    //char hexoutput[64];

    CryptoProvider *crypto = NULL;
    uint32_t i;

    // Context init
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &i) == STATUS_OK);
    ck_assert(i == 32);

    // It is random, so...
    ck_assert(CryptoProvider_SymmetricGenerateKey(crypto, &pSecKey0) == STATUS_OK);
    ck_assert(CryptoProvider_SymmetricGenerateKey(crypto, &pSecKey1) == STATUS_OK);
    ck_assert(NULL != (pExpKey0 = SecretBuffer_Expose(pSecKey0)));
    ck_assert(NULL != (pExpKey1 = SecretBuffer_Expose(pSecKey1)));
    ck_assert(memcmp(pExpKey0, pExpKey1, 32) != 0);
    SecretBuffer_Unexpose(pExpKey0);
    SecretBuffer_Unexpose(pExpKey1);
    SecretBuffer_DeleteClear(pSecKey0);
    SecretBuffer_DeleteClear(pSecKey1);

    // Test invalid inputs
    ck_assert(CryptoProvider_SymmetricGenerateKey(NULL, &pSecKey0) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricGenerateKey(crypto, NULL) != STATUS_OK);

    CryptoProvider_Delete(crypto);
}
END_TEST


START_TEST(test_crypto_derive_data)
{
    // TODO: these tests test only Basic256Sha256
    CryptoProvider *crypto = NULL;
    ExposedBuffer secret[32], seed[32], output[1024];
    char hexoutput[2048];
    uint32_t lenKey, lenKeyBis, lenIV, lenSecr, lenSeed, lenOutp;

    // Context init
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    ck_assert(CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == STATUS_OK);
    ck_assert(lenKey == 32);
    ck_assert(lenKeyBis == 32);
    ck_assert(lenIV == 16);
    lenOutp = lenKey+lenKeyBis+lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &lenSecr) == STATUS_OK);
    ck_assert(lenSecr == 32);
    lenSeed = lenSecr;

    // This test vectors is unofficial, taken from https://www.ietf.org/mail-archive/web/tls/current/msg03416.html
    ck_assert(unhexlify("9bbe436ba940f017b17652849a71db35", secret, 16) == 16);
    memcpy(seed, "test label", 10); // We don't use labels in DerivePseudoRandomData, but RFC 5246 specifies that label is prepend to seed
    ck_assert(unhexlify("a0ba9f936cda311827a6f796ffd5198c", seed+10, 16) == 16);
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 26, output, 100) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(memcmp(hexoutput, "e3f229ba727be17b8d122620557cd453c2aab21d07c3d495329b52d4e61edb5a6b301791e90d35c9c9a46b4e14baf9af0fa0"
                                "22f7077def17abfd3797c0564bab4fbc91666e9def9b97fce34f796789baa48082d122ee42c5a72e5a5110fff70187347b66",
                     200) == 0);
    // A second call to the same function should reset the contexts and provide the same result
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, secret, 16, seed, 26, output, 100) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 100) == 100);
    ck_assert(memcmp(hexoutput, "e3f229ba727be17b8d122620557cd453c2aab21d07c3d495329b52d4e61edb5a6b301791e90d35c9c9a46b4e14baf9af0fa0"
                                "22f7077def17abfd3797c0564bab4fbc91666e9def9b97fce34f796789baa48082d122ee42c5a72e5a5110fff70187347b66",
                     200) == 0);

    // More appropriate examples (generated by the test-writer with a Python implementation that conforms to the previous test vector)
    ck_assert(unhexlify("8bcc1010ba96bc055c1168cf84167410893d6cc4cff090f6ded0eb476b118e17", secret, lenSecr) == (int32_t)lenSecr);
    ck_assert(unhexlify("8c4584155b3df8aba84ede20a3a3778e087f0cf40d850f395b356345b0426614", seed, lenSeed) == (int32_t)lenSeed);
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, 64) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 64) == 64);
    ck_assert(memcmp(hexoutput, "5a6cee5d3f4881816d4d5fa890ea9333a0ccb47998efa8c3c1f7e04ffd778b0ab71c5bc89bb418031ae54e34c6ab78a8e7a39113d72d7446ff5e54738d9d1d7e", 128) == 0);

    ck_assert(unhexlify("6bc8af2863fcc9e7e1d4441d8d87ae0dc42d9f62155bca420703537b05c53756", secret, lenSecr) == (int32_t)lenSecr);
    ck_assert(unhexlify("c33f3f15ae9537c4d1e618dff2260ad0f6757c0201073fc265281e60b939a322", seed, lenSeed) == (int32_t)lenSeed);
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, lenOutp) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, lenOutp) == (int32_t)lenOutp);
    ck_assert(memcmp(hexoutput, "ba523f60d02e153670604816cbb25301ce8cc27a04f2be01163f3dd517c2b7f636a08d3ca6ed5811d65a9605efcaf5fd137984ac4a7efc141a181f5dacaac1bd249a8e6424ad5133efd751b2c418160f",
                     2*lenOutp) == 0);

    ck_assert(unhexlify("d53d3776ecf8540fe1f579f6278f90cec832a19de09c915cd7ccb7bd942377a5", secret, lenSecr) == (int32_t)lenSecr);
    ck_assert(unhexlify("87f48b64bffff0a20efeb62347fa995e574aad63c7371a5dac4b3fe2ae689b65", seed, lenSeed) == (int32_t)lenSeed);
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, secret, lenSecr, seed, lenSeed, output, 1024) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 1024) == 1024);
    ck_assert(memcmp(hexoutput, "addbefdaf4e8c0b14fa7ac19e302bf45e908910ee833975c3328bfa6b7a464c8ced5976887a9fd98b824da473eb88cf1ca24c17268da6e6176cc8023c120e1d1"
                                "60f3755cb017b1bb67127dc9ceef24647a313ac230e68cd91b57115c60b55af8bde0c37667291678f2cd874ce2b5a87223b048e43f32319dea5c9d421c29ecb7"
                                "dd15630f5b11feaffd7d4facd49995a6cb1c6d95002045870474868815d13fb9159697b09859e8ca720c50754655bfb1b07aea7522af545393cd178e1be80959"
                                "84b353958d6ce3a9da7ef03bd839f19dd400e513ff14ea3f0b6e39d9682d8f3d5f11ff202d3577ac2f5f5cce02b225a839a70a2b6d060527a900377f151f286f"
                                "33a8adfe8b3cc7874e7188ea0a949d9b67de6adecdce269b0e473f14600c800d53bed6e0ee879495668ee10a036fafdf015ea655f870e7d8630e055ac6f6fce2"
                                "e9dfb467439007be895461ef6910db1ccdec32b1f0640a856e961a743358a459b89f60f3089b41607bc0d111512c5a7b1bd3ce955254b6393110ccf0f646b26d"
                                "399407fdc9983656b6c9e3cb81c2aae5b73d43d456145e5db3c1d84dddef0da42c1f9f6994b53bd7bb01392fae0882fe205be759d5bbea87f4f2a3d3e0b17585"
                                "7114419a37f4f83bb35ec87a008dddf04300f670818c7ff70d4d5b2de4bcee563b676c05edc4de5f7077d2d92f825e1a203cb9e37b8732e338b4aa188bb00dfb"
                                "fa35223936cee77bfab4e0fee46b23f0397700a81ae257cb144594ee241c5eb5ffde74132b505e7f8f85b6b140b743403714f28eb4a1082f2b8536bf068ba0ad"
                                "0b49b873323881a0bba915d678f2548fb7e4424d42ced72d9f7b818b11da8ef1fcd698da53521cfcd02e559f0ae9ee85ce046f47ae2215baaf9b08b0ef8733cb"
                                "78a00aae90b8dda614db5e647f690c4d310cd71dbb95c092ab2a2d5d036c1bf4f160c59ce099f185a9c638f09fa1d33328d5827bda760c258f3957d954147324"
                                "e678202b2fe791926f914d099c715f9eec751f526134ec84c4ba26c4fcbafc29b0d5cef8b35d0422c3c8835fe0f11dbbba31970c0c7f6b94767ba9b4b24fa53f"
                                "eda6e2453122a4a7fc28ddc77424740287d34e85971c87ead468496774ad6d6883768f4de4ce30c604395822262d7fabe2cf3147f1f95dc603038f7d088140df"
                                "190e143720b19cf65b8a67d725dc1d043f9b2a168bfcef2c4f086f3966aca72b56a308e3a3c2eb7fcf8e6f6804065041ae8fd65072a3f7913acde01ff387cde9"
                                "336574782a7dd28baf983e359a1afd6d0513809d1a83ec9f0a2a2a8a8fb1686635e068c4bce4bee77bb817662335a91312920063dbca5a23adb064bc6e3dad65"
                                "c53fe61599241d26c9615562b9456ede80587da078e639a66a160066241d9dabc8bde9c8c16d46ebb3ff5bc2698dd56a9a8b924ef20eb0b67fa679f6fd41bdc6",
                     2048) == 0);

    CryptoProvider_Delete(crypto);
}
END_TEST


START_TEST(test_crypto_derive_keysets)
{
    // TODO: these tests test only Basic256Sha256
    CryptoProvider *crypto = NULL;
    ExposedBuffer clientNonce[32], serverNonce[32], *pout, zeros[32];
    char hexoutput[160];
    uint32_t lenKey, lenKeyBis, lenIV, lenCliNonce, lenSerNonce, lenOutp;
    SC_SecurityKeySet cliKS, serKS;

    // Context init
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    ck_assert(CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == STATUS_OK);
    ck_assert(lenKey == 32);
    ck_assert(lenKeyBis == 32);
    ck_assert(lenIV == 16);
    lenOutp = lenKey+lenKeyBis+lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &lenCliNonce) == STATUS_OK);
    ck_assert(lenCliNonce == 32);
    lenSerNonce = lenCliNonce;

    // Prepares security key sets
    memset(zeros, 0, 32);
    ck_assert(NULL != (cliKS.signKey = SecretBuffer_NewFromExposedBuffer(zeros, lenKey)));
    ck_assert(NULL != (cliKS.encryptKey= SecretBuffer_NewFromExposedBuffer(zeros, lenKey)));
    ck_assert(NULL != (cliKS.initVector = SecretBuffer_NewFromExposedBuffer(zeros, lenIV)));
    ck_assert(NULL != (serKS.signKey = SecretBuffer_NewFromExposedBuffer(zeros, lenKey)));
    ck_assert(NULL != (serKS.encryptKey= SecretBuffer_NewFromExposedBuffer(zeros, lenKey)));
    ck_assert(NULL != (serKS.initVector = SecretBuffer_NewFromExposedBuffer(zeros, lenIV)));

    // These come from a stub_client working with OPC foundation code (e.g. commit 0fbccc98472c781a7f44ac09c1d36d2b4a0c3fb0)
    ck_assert(unhexlify("3d3b4768f275d5023c2145cbe3a4a592fb843643d791f7bd7fce75ff25128b68", clientNonce, lenCliNonce) == (int32_t)lenCliNonce);
    ck_assert(unhexlify("ccee418cbc77c2ebb38d5ffac9d2a9d0a6821fa211798e71b2d65b3abb6aec8f", serverNonce, lenSerNonce) == (int32_t)lenSerNonce);
    ck_assert(CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS, &serKS) == STATUS_OK);
    // 4 lines for each assert
    ck_assert(NULL != (pout = SecretBuffer_Expose(cliKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "86842427475799fa782efa5c63f5eb6f0b6dbf8a549dd5452247feaa5021714b", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(cliKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "d8de10ac4fb579f2718ddcb50ea68d1851c76644b26454e3f9339958d23429d5", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(cliKS.initVector)));
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t)lenIV);
    ck_assert(memcmp(hexoutput, "4167de62880e0bdc023aa133965c34ff", 2*lenIV) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(serKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "f6db2ad48ad3776f83086b47e9f905ee00193f87e85ccde0c3bf7eb8650e236e", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(serKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "2c86aecfd5629ee05c49345bce3b2a7ca959a0bf4c9c281b8516a369650dbc4e", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(serKS.initVector)));
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t)lenIV);
    ck_assert(memcmp(hexoutput, "39a4f596bcbb99e0b48114f60fc6af21", 2*lenIV) == 0);
    SecretBuffer_Unexpose(pout);

    // Another run, just to be sure...
    ck_assert(unhexlify("d821ea93a6a48a4ef49b36c5e7d1bae6c49ccb2b2ddb07c99dcf046e2225617f", clientNonce, lenCliNonce) == (int32_t)lenCliNonce);
    ck_assert(unhexlify("00a8cb99446410a70bf221d5c498d0d0b3e968a306f1a4dc5d1acbe7a37644da", serverNonce, lenSerNonce) == (int32_t)lenSerNonce);
    ck_assert(CryptoProvider_DeriveKeySets(crypto, clientNonce, lenCliNonce, serverNonce, lenSerNonce, &cliKS, &serKS) == STATUS_OK);
    // 4 lines for each assert
    ck_assert(NULL != (pout = SecretBuffer_Expose(cliKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "185e860da28d3a224729926ba5b5b800214b2f74257ed39e694596520e67e574", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(cliKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "7a6c2cdc20a842a0e2039075935b14a07f578c157091328adc9d52bbb8ef727d", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(cliKS.initVector)));
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t)lenIV);
    ck_assert(memcmp(hexoutput, "dcf97c356f5ef87b7049900f74355c13", 2*lenIV) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(serKS.signKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "105b1805ecc3a25de8e2eaa5c9e94504b355990243c6163c2c8b95c1f5681694", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(serKS.encryptKey)));
    ck_assert(hexlify(pout, hexoutput, lenKey) == (int32_t)lenKey);
    ck_assert(memcmp(hexoutput, "2439bdd8fc365b0fe7b7e2cfcefee67ea7bdea6c157d0b23092f0abc015792d5", 2*lenKey) == 0);
    SecretBuffer_Unexpose(pout);
    ck_assert(NULL != (pout = SecretBuffer_Expose(serKS.initVector)));
    ck_assert(hexlify(pout, hexoutput, lenIV) == (int32_t)lenIV);
    ck_assert(memcmp(hexoutput, "005a70781b43979940c77368677718cd", 2*lenIV) == 0);
    SecretBuffer_Unexpose(pout);

    // Clears KS
    SecretBuffer_DeleteClear(cliKS.signKey);
    SecretBuffer_DeleteClear(cliKS.encryptKey);
    SecretBuffer_DeleteClear(cliKS.initVector);
    SecretBuffer_DeleteClear(serKS.signKey);
    SecretBuffer_DeleteClear(serKS.encryptKey);
    SecretBuffer_DeleteClear(serKS.initVector);

    CryptoProvider_Delete(crypto);
}
END_TEST


/*#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/rsa.h"
#include "mbedtls/md.h"

START_TEST(test_pk_x509)
{
    CryptoProvider *crypto = NULL;
    CryptolibContext *pCtx = NULL;
    uint32_t i, j;

    // Context init
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    pCtx = crypto->pCryptolibContext;
    ck_assert(NULL != pCtx);

    // This is the PKI:
    // Reads a public/private key pair
    mbedtls_x509_crt crt; // Client/Server certificate, which contains the pk struct
    mbedtls_pk_context pk_priv;
    mbedtls_x509_crt_init(&crt);
    mbedtls_pk_init(&pk_priv);
    ck_assert(mbedtls_x509_crt_parse_file(&crt, "client_public/client.der") == 0);
    ck_assert(mbedtls_pk_parse_keyfile(&pk_priv, "client_private/client.key", 0) == 0);
    // Get bits number
    i = mbedtls_pk_get_bitlen(&crt.pk);
    j = mbedtls_pk_get_bitlen(&pk_priv);
    // Assert len is enough but not too much
    printf("Bitlenghts: %u %u\n", i, j);
    // TODO: Assert can do
    // Ciphering
    mbedtls_rsa_context *prsa_pub, *prsa_priv;
    ck_assert(mbedtls_pk_get_type(&crt.pk) == MBEDTLS_PK_RSA);
    ck_assert(mbedtls_pk_get_type(&pk_priv) == MBEDTLS_PK_RSA);
    prsa_pub = mbedtls_pk_rsa(crt.pk);
    prsa_priv = mbedtls_pk_rsa(pk_priv);
    ck_assert(mbedtls_rsa_check_pub_priv(prsa_pub, prsa_priv) == 0);
    mbedtls_rsa_set_padding(prsa_pub, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);
    mbedtls_rsa_set_padding(prsa_priv, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);
    // Calculate max payload
    //printf("Output buffer lengths: %u %u\n", prsa_pub->N, prsa_priv->N); // Mauvaise interprétation de N, qui n'est pas un type de base...
    // Faut repasser par pk get_len, ou prsa->len
    printf("Hash IDs: %i %i\n", prsa_pub->hash_id, prsa_priv->hash_id);
    mbedtls_md_info_t *pmdinfo = mbedtls_md_info_from_type(prsa_pub->hash_id);
    printf("prsa_pub->len: %i %i\n", prsa_pub->len, prsa_priv->len);
    uint32_t len_hash = mbedtls_md_get_size(pmdinfo), len_max = prsa_pub->len - 2*len_hash - 2;
    printf("len_hash, len_max: %i %i\n", len_hash, len_max);
    // Alice is sending something to Bob
    // Encrypt with pk (Bob-pk)
    // Encrypt with priv: non sense
    unsigned char input[256], output[256], input_bis[256];
    char hexoutput[512];
    uint32_t len = 0;
    memset(input, 0, 256);
    memset(output, 0, 256);
    strncpy(input, "Test INGOPCS Test", 32); // 17 should be enough but we want to verify padding
    ck_assert(mbedtls_rsa_rsaes_oaep_encrypt(prsa_pub, mbedtls_ctr_drbg_random, &crypto->pCryptolibContext->ctxDrbg, MBEDTLS_RSA_PUBLIC, NULL, 0, 32, input, output) == 0);
    // Decrypt with priv (Bob-priv)
    // Decrypt with pk: why would you do that
    memset(input_bis, 0xFF, 256);
    ck_assert(mbedtls_rsa_rsaes_oaep_decrypt(prsa_priv, mbedtls_ctr_drbg_random, &crypto->pCryptolibContext->ctxDrbg, MBEDTLS_RSA_PRIVATE, NULL, 0, &len, output, input_bis, 256) == 0);
    ck_assert(len == 32);
    ck_assert(memcmp(input, input_bis, 32) == 0);
    // En/decrypt unpaddable message
    memset(input_bis, 0xFF, 256);
    ck_assert(mbedtls_rsa_rsaes_oaep_encrypt(prsa_pub, mbedtls_ctr_drbg_random, &crypto->pCryptolibContext->ctxDrbg, MBEDTLS_RSA_PUBLIC, NULL, 0, len_max, input, output) == 0);
    ck_assert(mbedtls_rsa_rsaes_oaep_decrypt(prsa_priv, mbedtls_ctr_drbg_random, &crypto->pCryptolibContext->ctxDrbg, MBEDTLS_RSA_PRIVATE, NULL, 0, &len, output, input_bis, 256) == 0);
    ck_assert(len == len_max);
    ck_assert(memcmp(input, input_bis, len_max) == 0);
    // Sign with priv (Alice-priv)
    pmdinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    // It should be specified that the content to sign is only hashed with a SHA-256, and then sent to pss_sign, which should be done with SHA-256 too.
    mbedtls_rsa_set_padding(prsa_priv, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256); // Don't forget that, bro!
    unsigned char hashed[32]; // Assert md_get_len(pmdinfo)
    mbedtls_md(pmdinfo, input, 32, hashed); // No prefixed-padding, no salt, but hashed
    ck_assert(mbedtls_rsa_rsassa_pss_sign(prsa_priv, mbedtls_ctr_drbg_random, &crypto->pCryptolibContext->ctxDrbg, MBEDTLS_RSA_PRIVATE,
                                          MBEDTLS_MD_SHA256, 32, // hashlen is optionnal, as md_alg is not MD_NONE
                                          hashed, output) == 0); // signature is as long as the key
    ck_assert(hexlify(hashed, hexoutput, 32) == 32);
    printf("Hash: %64s\n", hexoutput);
    ck_assert(hexlify(output, hexoutput, 256) == 256);
    hexoutput[128] = 0;
    printf("Sig: %128s...\n", hexoutput);
    // Verify with pk (Alice-pk)
    mbedtls_rsa_set_padding(prsa_pub, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    ck_assert(mbedtls_rsa_rsassa_pss_verify(prsa_pub, mbedtls_ctr_drbg_random, &crypto->pCryptolibContext->ctxDrbg, MBEDTLS_RSA_PUBLIC, // Random functions are optionnal for verification
                                            MBEDTLS_MD_SHA256, 32,
                                            hashed, output) == 0);

    // This is the X509 part of the PKI:
    // Reads a CA
    mbedtls_x509_crt ca;
    mbedtls_x509_crt_init(&ca);
    ck_assert(mbedtls_x509_crt_parse_file(&ca, "trusted/cacert.der") == 0);
    // Reads the public key (signed with the CA)
    // (reads a revocation list)
    // Verify the certificate signature
    uint32_t flags;
    ck_assert(mbedtls_x509_crt_verify(&crt, &ca, NULL, NULL, &flags, NULL, 0) == 0);

    // We should use the NIST Sign Verification test vectors, but they are not easy to use.

    // TODO: free the contexts & look at code to see if the aes contexts are correctly freed

    CryptoProvider_Delete(crypto);
}
END_TEST*/


START_TEST(test_cert_load)
{
    CryptoProvider *crypto = NULL;
    KeyManager *keyman = NULL;
    Certificate crt_pub;
    uint8_t thumb[20];
    char hexoutput[40];
    uint8_t der_cert[1215];

    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
    keyman = KeyManager_Create(crypto,
                               (int8_t *)"./trusted/", 10,
                               (int8_t *)"./revoked/", 10);
    ck_assert(NULL != keyman);

    //ck_assert(KeyManager_Certificate_LoadFromFile(keyman, (int8_t *)"./server_public/server.der", &crt_pub) == STATUS_OK);
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
    ck_assert(KeyManager_Certificate_Load(keyman, der_cert, 1215, &crt_pub) == STATUS_OK);
    ck_assert(KeyManager_Certificate_GetThumbprint(keyman, &crt_pub, thumb, 20) == STATUS_OK);
    ck_assert(hexlify(thumb, hexoutput, 20) == 20);
    // The expected thumbprint for this certificate was calculated with openssl tool, and mbedtls API.
    ck_assert(memcmp(hexoutput, "af17d03e1605277489815ab88bc4760655b3e2cd") == 0);

    KeyManager_Delete(keyman);
    CryptoProvider_Delete(crypto);
}
END_TEST


Suite *tests_make_suite_crypto()
{
    Suite *s;
    TCase *tc_ciphers, *tc_providers, *tc_derives, *tc_misc, *tc_km;

    s = suite_create("Crypto lib");
    tc_ciphers = tcase_create("Ciphers");
    tc_providers = tcase_create("Crypto Provider");
    tc_derives = tcase_create("Crypto Data Derivation");
    tc_misc = tcase_create("Crypto Misc");
    tc_km = tcase_create("Key Management");

    suite_add_tcase(s, tc_misc);
    tcase_add_test(tc_misc, test_hexlify);

    suite_add_tcase(s, tc_ciphers);
    tcase_add_test(tc_ciphers, test_crypto_symm_crypt);
    tcase_add_test(tc_ciphers, test_crypto_symm_sign);
    tcase_add_test(tc_ciphers, test_crypto_symm_gen);
    //tcase_add_test(tc_ciphers, test_pk_x509);

    suite_add_tcase(s, tc_providers);

    suite_add_tcase(s, tc_derives);
    tcase_add_test(tc_derives, test_crypto_derive_data);
    tcase_add_test(tc_derives, test_crypto_derive_keysets);

    suite_add_tcase(s, tc_km);
    tcase_add_test(tc_km, test_cert_load);

    return s;
}

