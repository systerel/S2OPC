/*
 * Crypto test suite. See check_stack.c for more details.
 *
 *  Created on: Sep 27, 2016
 *      Author: PAB
 */


#include <stdio.h>
#include <string.h>
#include <check.h>
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "check_stack.h"
#include "crypto_provider.h"
#include "ua_builtintypes.h"


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
    CryptoProvider *crypto = UA_NULL;

    // Context init
    crypto = CryptoProvider_Create_Low(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(UA_NULL != crypto);
    ck_assert(CryptoProvider_Symmetric_GetKeyLength_Low(crypto, &len) == STATUS_OK);
    ck_assert(len == 32);

    // Encrypt
    // This single test is not taken from the NIST test vectors...
    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "dc95c078a2408989ad48a21492842087", 32) == 0);

    ck_assert(unhexlify("c47b0294dbbbee0fec4757f22ffeee3587ca4730c3d33b691df38bab076bc558", key, 32) == 32);
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "46f2fb342d6f0ab477476fc501242c5f", 32) == 0);

    ck_assert(unhexlify("ccd1bc3c659cd3c59bc437484e3c5c724441da8d6e90ce556cd57d0752663bbc", key, 32) == 32);
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "304f81ab61a80c2e743b94d5002a126b", 32) == 0);

    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("0b24af36193ce4665f2825d7b4749c98", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "a9ff75bd7cf6613d3731c77c3b6d0c04", 32) == 0);

    // Decrypt
    ck_assert(unhexlify("28d46cffa158533194214a91e712fc2b45b518076675affd910edeca5f41ac64", key, 32) == 32);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("4bf3b0a69aeb6657794f2901b1440ad4", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    for(i=0; i<16; ++i)
        ck_assert(output[i] == 0);

    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", key, 32) == 32);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("47cb030da2ab051dfc6c4bf6910d12bb", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    for(i=0; i<16; ++i)
        ck_assert(output[i] == 0);

    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("623a52fcea5d443e48d9181ab32c7421", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "761c1fe41a18acf20d241650611d90f1", 32) == 0);

    // Encrypt + Decrypt
    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", key, 32) == 32);
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "47cb030da2ab051dfc6c4bf6910d12bb", 32) == 0);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, output, 16, key, iv, input, 16) == STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "00000000000000000000000000000000", 32) == 0);

    // Multi-block messages
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    ck_assert(unhexlify("256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa", input, 128) == 128);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 128, key, iv, output, 128) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput, "90b7b9630a2378f53f501ab7beff039155008071bc8438e789932cfd3eb1299195465e6633849463fdb44375278e2fdb1310821e6492cf80ff15cb772509fb426f3aeee27bd4938882fd2ae6b5bd9d91fa4a43b17bb439ebbe59c042310163a82a5fe5388796eee35a181a1271f00be29b852d8fa759bad01ff4678f010594cd", 256) == 0);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, output, 128, key, iv, input, 128) == STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput, "256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa", 256) == 0);


    // Assert failure on wrong parameteres
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(UA_NULL, input, 16, key, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, UA_NULL, 16, key, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 15, key, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, UA_NULL, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, UA_NULL, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, UA_NULL, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricEncrypt_Low(crypto, input, 16, key, iv, output, 15) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(UA_NULL, input, 16, key, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, UA_NULL, 16, key, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 15, key, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, UA_NULL, iv, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, key, UA_NULL, output, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, key, iv, UA_NULL, 16) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricDecrypt_Low(crypto, input, 16, key, iv, output, 15) != STATUS_OK);

    CryptoProvider_Delete_Low(crypto);
    crypto = UA_NULL;
}
END_TEST


START_TEST(test_crypto_symm_sign)
{
    // TODO: these tests test only Basic256Sha256
    unsigned char key[256];
    unsigned char input[256];
    unsigned char output[32];
    char hexoutput[1024];
    CryptoProvider *crypto = UA_NULL;
    uint32_t len;

    // Context init
    crypto = CryptoProvider_Create_Low(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(UA_NULL != crypto);
    ck_assert(CryptoProvider_Symmetric_GetKeyLength_Low(crypto, &len) == STATUS_OK);
    ck_assert(len == 32);
    ck_assert(CryptoProvider_SymmetricSignature_GetLength_Low(crypto, &len) == STATUS_OK);
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
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    len = 0;
    ck_assert(CryptoProvider_SymmetricSign_Low(crypto, input, 64, key, output, &len) == STATUS_OK);
    ck_assert(len == 32);
    ck_assert(hexlify(output, hexoutput, 32) == 32);
    ck_assert(memcmp(hexoutput, "e4185b6d49f06e8b94a552ad950983852ef20b58ee75f2c448fea587728d94db", 64) == 0);

    // Check verify
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, input, 64, key, output) == STATUS_OK);
    output[1] ^= 0x20; // Change 1 bit
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, input, 64, key, output) == STATUS_NOK);
    output[1] ^= 0x20; // Revert changed bit
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, input, 64, key, output) == STATUS_OK);
    output[31] = 0x04; // Change 1 bit in last byte
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, input, 64, key, output) == STATUS_NOK);

    // Check optional feature
    ck_assert(CryptoProvider_SymmetricSign_Low(crypto, input, 64, key, output, UA_NULL) == STATUS_OK);

    // Check invalid parameters
    ck_assert(CryptoProvider_SymmetricSign_Low(UA_NULL, input, 64, key, output, UA_NULL) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign_Low(crypto, UA_NULL, 64, key, output, UA_NULL) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign_Low(crypto, input, 64, UA_NULL, output, UA_NULL) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricSign_Low(crypto, input, 64, key, UA_NULL, UA_NULL) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify_Low(UA_NULL, input, 64, key, output) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, UA_NULL, 64, key, output) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, input, 64, UA_NULL, output) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricVerify_Low(crypto, input, 64, key, UA_NULL) != STATUS_OK);

    CryptoProvider_Delete_Low(crypto);
    crypto = UA_NULL;
}
END_TEST


START_TEST(test_crypto_symm_gen)
{
    // TODO: these tests test only Basic256Sha256
    unsigned char keys[64];
    //char hexoutput[64];

    CryptoProvider *crypto = UA_NULL;
    uint32_t i;

    // Context init
    crypto = CryptoProvider_Create_Low(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(UA_NULL != crypto);
    ck_assert(CryptoProvider_Symmetric_GetKeyLength_Low(crypto, &i) == STATUS_OK);
    ck_assert(i == 32);

    // It is random, so...
    ck_assert(CryptoProvider_SymmetricGenerateKey_Low(crypto, keys) == STATUS_OK);
    ck_assert(CryptoProvider_SymmetricGenerateKey_Low(crypto, &keys[32]) == STATUS_OK);
    //ck_assert(hexlify(key, hexoutput, 32) == 32);
    //printf("Random key: %64s\n", hexoutput);
    ck_assert(memcmp(keys, &keys[32], 32) != 0);

    // Test invalid inputs
    ck_assert(CryptoProvider_SymmetricGenerateKey_Low(UA_NULL, keys) != STATUS_OK);
    ck_assert(CryptoProvider_SymmetricGenerateKey_Low(crypto, UA_NULL) != STATUS_OK);
}
END_TEST


Suite *tests_make_suite_crypto()
{
    Suite *s;
    TCase *tc_ciphers, *tc_providers, *tc_misc;

    s = suite_create("Crypto lib");
    tc_ciphers = tcase_create("Ciphers");
    tc_providers = tcase_create("Crypto Provider");
    tc_misc = tcase_create("Misc");

    suite_add_tcase(s, tc_misc);
    tcase_add_test(tc_misc, test_hexlify);

    suite_add_tcase(s, tc_ciphers);
    tcase_add_test(tc_ciphers, test_crypto_symm_crypt);
    tcase_add_test(tc_ciphers, test_crypto_symm_sign);
    tcase_add_test(tc_ciphers, test_crypto_symm_gen);

    suite_add_tcase(s, tc_providers);

    return s;
}

