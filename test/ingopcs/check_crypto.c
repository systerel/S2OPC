/*
 * Crypto test suite. See check_stack.c for more details.
 *
 *  Created on: Sep 27, 2016
 *      Author: PAB
 */


#include <check.h>
#include "check_stack.h"
#include "mbedtls/aes.h"
#include <string.h>
#include <stdio.h>


// You should allocate strlen(src)*2 in dst. n is strlen(src)
// Returns n the number of translated chars (< 0 for errors)
int hexlify(const unsigned char *src, size_t n, char *dst)
{
    size_t i;

    if(! src || ! dst)
        return -1;

    for(i=0; i<n; ++i)
        sprintf(&dst[2*i], "%02hhx", src[i]);

    return n;
}

// You should allocate strlen(src)/2 in dst. n is strlen(dst)
// Returns n the number of translated couples (< 0 for errors)
int unhexlify(const char *src, size_t n, unsigned char *dst)
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
        ck_assert(hexlify(&c, 1, (char *)buf) == 1);
        ck_assert(unhexlify((char *)buf, 1, &d) == 1);
        ck_assert(c == d);
    }

    // Test vector
    ck_assert(hexlify((unsigned char *)"\x00 Test \xFF", 8, (char *)buf) == 8);
    ck_assert(strncmp((char *)buf, "00205465737420ff", 16) == 0);
    ck_assert(unhexlify((char *)buf, 8, buf+16) == 8);
    ck_assert(strncmp((char *)(buf+16), "\x00 Test \xFF", 8) == 0);
}
END_TEST


// TODO: wrap these tests inside a cryptoprovider !!
START_TEST(test_aes)
{
    // Tests based on the test vectors provided by the NIST
    //  (http://csrc.nist.gov/groups/STM/cavp/block-ciphers.html#aes)
    mbedtls_aes_context aes;
    unsigned char key[32];
    unsigned char iv[16];
    unsigned char input[128];
    unsigned char output[128];
    char hexoutput[256];
    int i;

    // Encrypt
    // This single test is not taken from the NIST test vectors...
    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    mbedtls_aes_setkey_enc(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);
    ck_assert(hexlify(output, 16, hexoutput) == 16);
    ck_assert(memcmp(hexoutput, "dc95c078a2408989ad48a21492842087", 32) == 0);

    ck_assert(unhexlify("c47b0294dbbbee0fec4757f22ffeee3587ca4730c3d33b691df38bab076bc558", 32, key) == 32);
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    mbedtls_aes_setkey_enc(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);
    ck_assert(hexlify(output, 16, hexoutput) == 16);
    ck_assert(memcmp(hexoutput, "46f2fb342d6f0ab477476fc501242c5f", 32) == 0);

    ck_assert(unhexlify("ccd1bc3c659cd3c59bc437484e3c5c724441da8d6e90ce556cd57d0752663bbc", 32, key) == 32);
    memset(iv, 0, sizeof(iv));
    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    mbedtls_aes_setkey_enc(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);
    ck_assert(hexlify(output, 16, hexoutput) == 16);
    ck_assert(memcmp(hexoutput, "304f81ab61a80c2e743b94d5002a126b", 32) == 0);

    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("0b24af36193ce4665f2825d7b4749c98", 16, input) == 16);
    memset(output, 0, sizeof(output));
    mbedtls_aes_setkey_enc(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);
    ck_assert(hexlify(output, 16, hexoutput) == 16);
    ck_assert(memcmp(hexoutput, "a9ff75bd7cf6613d3731c77c3b6d0c04", 32) == 0);

    // Decrypt
    ck_assert(unhexlify("28d46cffa158533194214a91e712fc2b45b518076675affd910edeca5f41ac64", 32, key) == 32);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("4bf3b0a69aeb6657794f2901b1440ad4", 16, input) == 16);
    memset(output, 0, sizeof(output));
    mbedtls_aes_setkey_dec(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, iv, input, output);
    ck_assert(hexlify(output, 16, hexoutput) == 16);
    for(i=0; i<16; ++i)
        ck_assert(output[i] == 0);

    ck_assert(unhexlify("07eb03a08d291d1b07408bf3512ab40c91097ac77461aad4bb859647f74f00ee", 32, key) == 32);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("47cb030da2ab051dfc6c4bf6910d12bb", 16, input) == 16);
    memset(output, 0, sizeof(output));
    mbedtls_aes_setkey_dec(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, iv, input, output);
    for(i=0; i<16; ++i)
        ck_assert(output[i] == 0);

    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("623a52fcea5d443e48d9181ab32c7421", 16, input) == 16);
    memset(output, 0, sizeof(output));
    mbedtls_aes_setkey_dec(&aes, key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, iv, input, output);
    ck_assert(hexlify(output, 16, hexoutput) == 16);
    ck_assert(memcmp(hexoutput, "761c1fe41a18acf20d241650611d90f1", 32) == 0);
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

    suite_add_tcase(s, tc_ciphers);
    tcase_add_test(tc_ciphers, test_aes);

    suite_add_tcase(s, tc_providers);

    suite_add_tcase(s, tc_misc);
    tcase_add_test(tc_misc, test_hexlify);

    return s;
}

