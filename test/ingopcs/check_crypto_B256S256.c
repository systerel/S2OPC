/** \file check_crypto_B256S256
 *
 * Cryptographic test suite. This suite tests Basic256Sha256.
 * See check_stack.c for more details.
 *
 *  Created on: Sep 27, 2016
 *      Author: PAB
 */


#include <stdio.h>
#include <string.h>
#include <check.h>
#include <stddef.h> // NULL
#include <stdlib.h> // malloc, free

#include "check_stack.h"
#include "crypto_provider.h"
#include "ua_base_types.h"
#include "crypto_profiles.h"
#include "crypto_types.h"
#include "secret_buffer.h"
#include "crypto_provider_lib.h"
#include "key_manager.h"
#include "pki_stack.h"


// Helper
// You should allocate strlen(src)*2 in dst. n is strlen(src)
// Returns n the number of translated chars (< 0 for errors)
int hexlify(const unsigned char *src, char *dst, size_t n)
{
    size_t i;
    char buffer[3];

    if(! src || ! dst)
        return -1;

    for(i=0; i<n; ++i) {
        sprintf(buffer, "%02hhx", src[i]); // sprintf copies the last \0 too
        memcpy(dst+2*i, buffer, 2);
    }

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
    unsigned char buf[33], c, d = 0;
    int i;

    // Init
    memset(buf, 0, 33);

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

    // Test overflow
    buf[32] = 0xDD;
    ck_assert(hexlify((unsigned char *)"\x00 Test \xFF\x00 Test \xFF", (char *)buf, 16) == 16);
    ck_assert(buf[32] == 0xDD);
    ck_assert(unhexlify("00205465737420ff00205465737420ff", buf, 16) == 16);
    ck_assert(buf[32] == 0xDD);
}
END_TEST


// Using fixtures
CryptoProvider *crypto = NULL;

void setup_crypto(void)
{
    crypto = CryptoProvider_Create(SecurityPolicy_Basic256Sha256_URI);
    ck_assert(NULL != crypto);
}

void teardown_crypto(void)
{
    CryptoProvider_Free(crypto);
    crypto = NULL;
}


START_TEST(test_crypto_load)
{
    ck_assert(NULL != crypto->pProfile);
    ck_assert(SecurityPolicy_Basic256Sha256_ID == crypto->pProfile->SecurityPolicyID);
    ck_assert(NULL != crypto->pProfile->pFnSymmEncrypt);
    ck_assert(NULL != crypto->pProfile->pFnSymmDecrypt);
    ck_assert(NULL != crypto->pProfile->pFnSymmSign);
    ck_assert(NULL != crypto->pProfile->pFnSymmVerif);
    ck_assert(NULL != crypto->pProfile->pFnSymmGenKey);
    ck_assert(NULL != crypto->pProfile->pFnDeriveData);
    ck_assert(NULL != crypto->pProfile->pFnAsymEncrypt);
    ck_assert(NULL != crypto->pProfile->pFnAsymDecrypt);
    ck_assert(NULL != crypto->pProfile->pFnAsymSign);
    ck_assert(NULL != crypto->pProfile->pFnAsymVerify);
    ck_assert(NULL != crypto->pProfile->pFnCertVerify);
}
END_TEST


START_TEST(test_crypto_symm_lengths)
{
    uint32_t len = 0, lenCiph = 0, lenDeci = 0;

    // Check sizes
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &len) == STATUS_OK);
    ck_assert(32 == len);
    ck_assert(CryptoProvider_SymmetricGetLength_Signature(crypto, &len) == STATUS_OK);
    ck_assert(32 == len);
    ck_assert(CryptoProvider_SymmetricGetLength_Encryption(crypto, 15, &len) == STATUS_OK);
    ck_assert(15 == len);
    ck_assert(CryptoProvider_SymmetricGetLength_Decryption(crypto, 15, &len) == STATUS_OK);
    ck_assert(15 == len);
    ck_assert(CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, NULL) == STATUS_OK);
    ck_assert(CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, NULL) == STATUS_OK);
    ck_assert(16 == lenCiph);
    ck_assert(CryptoProvider_SymmetricGetLength_Blocks(crypto, NULL, &lenDeci) == STATUS_OK);
    ck_assert(16 == lenDeci);
    lenCiph = 0;
    lenDeci = 0;
    ck_assert(CryptoProvider_SymmetricGetLength_Blocks(crypto, &lenCiph, &lenDeci) == STATUS_OK);
    ck_assert(16 == lenCiph);
    ck_assert(16 == lenDeci);
}
END_TEST


START_TEST(test_crypto_symm_crypt)
{
    // Tests based on the test vectors provided by the NIST
    //  (http://csrc.nist.gov/groups/STM/cavp/block-ciphers.html#aes)
    unsigned char key[32];
    unsigned char iv[16];
    unsigned char input[128];
    unsigned char output[128];
    char hexoutput[256];
    int i;
    SecretBuffer *pSecKey = NULL, *pSecIV = NULL;

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

    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("256fd73ce35ae3ea9c25dd2a9454493e", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "90b7b9630a2378f53f501ab7beff0391", 32) == 0);
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

    memset(key, 0, sizeof(key));
    ck_assert(unhexlify("458b67bf212d20f3a57fce392065582dcefbf381aa22949f8338ab9052260e1d", key, 32) == 32);
    pSecKey = SecretBuffer_NewFromExposedBuffer(key, sizeof(key));
    ck_assert(NULL != pSecKey);
    memset(iv, 0, sizeof(iv));
    ck_assert(unhexlify("4c12effc5963d40459602675153e9649", iv, 16) == 16);
    pSecIV = SecretBuffer_NewFromExposedBuffer(iv, sizeof(iv));
    ck_assert(NULL != pSecIV);
    ck_assert(unhexlify("90b7b9630a2378f53f501ab7beff0391", input, 16) == 16);
    memset(output, 0, sizeof(output));
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, input, 16, pSecKey, pSecIV, output, 16) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 16) == 16);
    ck_assert(memcmp(hexoutput, "256fd73ce35ae3ea9c25dd2a9454493e", 32) == 0);
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
    memset(input, 0, sizeof(input));
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
}
END_TEST


START_TEST(test_crypto_symm_sign)
{
    unsigned char key[256];
    unsigned char input[256];
    unsigned char output[32];
    char hexoutput[1024];
    SecretBuffer *pSecKey = NULL;

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
}
END_TEST


START_TEST(test_crypto_symm_generate_key)
{
    SecretBuffer *pSecKey0, *pSecKey1;
    ExposedBuffer *pExpKey0, *pExpKey1;
    //char hexoutput[64];

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
}
END_TEST


START_TEST(test_crypto_derive_lengths)
{
    uint32_t lenKey = 0, lenKeyBis = 0, lenIV = 0;

    // Check sizes
    ck_assert(CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == STATUS_OK);
    ck_assert(32 == lenKey);
    ck_assert(32 == lenKeyBis);
    ck_assert(16 == lenIV);
}
END_TEST


START_TEST(test_crypto_derive_data)
{
    ExposedBuffer secret[32], seed[32], output[1024];
    char hexoutput[2048];
    uint32_t lenKey, lenKeyBis, lenIV, lenSecr, lenSeed, lenOutp;

    // Context init
    ck_assert(CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == STATUS_OK);
    lenOutp = lenKey+lenKeyBis+lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &lenSecr) == STATUS_OK);
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
}
END_TEST


START_TEST(test_crypto_derive_keysets)
{
    ExposedBuffer clientNonce[32], serverNonce[32], *pout, zeros[32];
    char hexoutput[160];
    uint32_t lenKey, lenKeyBis, lenIV, lenCliNonce, lenSerNonce, lenOutp;
    SC_SecurityKeySet cliKS, serKS;

    // Context init
    ck_assert(CryptoProvider_DeriveGetLengths(crypto, &lenKey, &lenKeyBis, &lenIV) == STATUS_OK);
    lenOutp = lenKey+lenKeyBis+lenIV;
    ck_assert(lenOutp < 1024);
    ck_assert(CryptoProvider_SymmetricGetLength_Key(crypto, &lenCliNonce) == STATUS_OK);
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
}
END_TEST


// Fixture for certificate load
Certificate *crt_pub = NULL;

void setup_certificate(void)
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
    ck_assert(KeyManager_Certificate_CreateFromDER(der_cert, 1215, &crt_pub) == STATUS_OK);
}

void teardown_certificate(void)
{
    KeyManager_Certificate_Free(crt_pub);
    crt_pub = NULL;

    teardown_crypto();
}


START_TEST(test_cert_load)
{
    ;
}
END_TEST


START_TEST(test_cert_lengths)
{
    uint32_t len = 0;

    CryptoProvider_CertificateGetLength_Thumbprint(crypto, &len);
    ck_assert(20 == len); // SHA-1
}
END_TEST


START_TEST(test_cert_thumbprint)
{
    uint8_t thumb[20];
    char hexoutput[40];

    //ck_assert(KeyManager_Certificate_CreateFromFile(keyman, (int8_t *)"./server_public/server.der", &crt_pub) == STATUS_OK);

    // Compute thumbprint
    ck_assert(KeyManager_Certificate_GetThumbprint(crypto, crt_pub, thumb, 20) == STATUS_OK);
    ck_assert(hexlify(thumb, hexoutput, 20) == 20);
    // The expected thumbprint for this certificate was calculated with openssl tool, and mbedtls API.
    ck_assert(memcmp(hexoutput, "af17d03e1605277489815ab88bc4760655b3e2cd", 40) == 0);

}
END_TEST


START_TEST(test_cert_loadkey)
{
    AsymmetricKey key_pub;

    // Loads the public key from cert
    ck_assert(KeyManager_Certificate_GetPublicKey(crt_pub, &key_pub) == STATUS_OK);

    // Note: as the AsymmetricKey is not malloc-ed, it is not free-d, so it is not cleared neither
}
END_TEST


// Fixtures for Asymetric crypto
AsymmetricKey *key_pub = NULL, *key_priv = NULL;

// Certificates: these are not the same cert as in setup_certificate. This one was created to also embed the private key in the tests.
#define DER_ASYM_PUB_HEXA "3082038b30820273a003020102020900cf163f0b5124ff4c300d06092a864886f70d01010b0500305c310b3009060355040613024652310f300d06035504080c"\
                          "064672616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504030c12494e474f504353205465737420"\
                          "7375697465301e170d3136313032363136333035345a170d3137303230333136333035345a305c310b3009060355040613024652310f300d06035504080c0646"\
                          "72616e6365310c300a06035504070c034169783111300f060355040a0c08537973746572656c311b301906035504030c12494e474f5043532054657374207375"\
                          "69746530820122300d06092a864886f70d01010105000382010f003082010a0282010100cbe0cd29bbcdd824999fc5571122e7540405ac94d0a9b3ab3630ce2c"\
                          "f361d50d9e737ce3f7746959003cbe90fc1019dce4797f4a87a05cd83521531e1391cf11f2e49ce6b0f68db31fb91675be4bbd4380920fccf46518ac2bff4208"\
                          "5ebc6ca107ecef53964e14617aecd75e1f27035c326f1757273047ca4d623bc5b08d278e3a320b964b11116df912bf91e99d3fdb78989e3daa144570647efc4c"\
                          "983c4159aecbf99aeb8bdfbf242ac5c43f0092a28aecddb8bdabf4aad7af68ae6bfe6d5cf6cb6e3a6a0c2d33ad3d592514703578d1cead67aa2c497600e0b983"\
                          "0ee8671f59f25262d596e4dbfe60ec6f5acb0c4f1cedf6b138fa12fd661b65e537c3539b0203010001a350304e301d0603551d0e041604147d33f73c22b315aa"\
                          "bd26ecb5a6db8a0bb511fbd0301f0603551d230418301680147d33f73c22b315aabd26ecb5a6db8a0bb511fbd0300c0603551d13040530030101ff300d06092a"\
                          "864886f70d01010b05000382010100550cb4f83c4b3567dc7aed66698056a034f38685e8227c0c0f00de0d7bd267f728d3b05c6f0fc089163e5654a833fd84cb"\
                          "6e5cc71853483cf09c4804ff862a01e920234578f2d6c8cd89008d017dce5d15be8a52396a101d32434d34aef346387216f550b1f932c94072168cdc68ad460d"\
                          "100bce4792c57b87f1a431d2b456698dd3c248fc6e2644d446f952255f98e3dcb7e5cd200b46f769d581833c21b08d07c4343973e93bed9a2d66ece5b6083e6e"\
                          "42b3378987339ab01aab362890dbf57dc22e9d86c0cd4edfa43f489d250bc4244542368c8682125645bd610fbf1c60ec5f94bc697284bde3915e9e051bb255ae"\
                          "e1685265a487bd1d72c5f49ef621e0"
#define DER_ASYM_PUB_LENG 911
#define DER_ASYM_PRIV_HEXA "308204a40201000282010100cbe0cd29bbcdd824999fc5571122e7540405ac94d0a9b3ab3630ce2cf361d50d9e737ce3f7746959003cbe90fc1019dce4797f4a"\
                           "87a05cd83521531e1391cf11f2e49ce6b0f68db31fb91675be4bbd4380920fccf46518ac2bff42085ebc6ca107ecef53964e14617aecd75e1f27035c326f1757"\
                           "273047ca4d623bc5b08d278e3a320b964b11116df912bf91e99d3fdb78989e3daa144570647efc4c983c4159aecbf99aeb8bdfbf242ac5c43f0092a28aecddb8"\
                           "bdabf4aad7af68ae6bfe6d5cf6cb6e3a6a0c2d33ad3d592514703578d1cead67aa2c497600e0b9830ee8671f59f25262d596e4dbfe60ec6f5acb0c4f1cedf6b1"\
                           "38fa12fd661b65e537c3539b020301000102820101009c87cb5d2868e1733053bfc29a508f052d5561ec9bcc3f3acb8f6b2c8dec66145fbc517e01866a3fbff3"\
                           "e368136f153c485a940597dde28ac937fdc5d0c6991231c79e436c48d0005ff1ce31b65a1644d658ce32d0cd31c536be736753bd1d36018cc32f0cee83ad5820"\
                           "b135fd7b099466d06e3e26c365cb07e0ccfd7a10d5f57879f21648083e9997cb1f78a3bd934dd472bafd852458e4fc843e14959d46cc2252e7bb12c0cfaee462"\
                           "3196595ce587921c600908e10c2e7257ea99a83c6df5b392220b88a11e3dcaf88c55a1a3ce8222037e19585cf644ccca65c188e7d109c447773c9e06cf15e2e2"\
                           "b745b0195d042cb264184d3b711d3e9e7e89858aa96102818100f2c690168005c536c5958a45ada4c1cad84203f961c560d996158d2b184d93f48934a0d46ec0"\
                           "512ee0670c2e49fda8b5de29fad03c3e5da406885a6d9775af2dfd5e61357997f2dbcaa087f79e076e95606904cfeab68185bbb4d2854d8f835e1eb38da5614b"\
                           "944970e8b5e4130262219f69394ede5c16e78112cfb3512b10b102818100d6fbd2ed02d9529b4e3a04a27da4659b2968d082cf660c0c4520cb1909084ff77ec3"\
                           "8dccc74f924a0db25869855ea95e6c61990837c9a46658ce233104b1bd2b9d1c16221561f41116926bd963406f789cea1b730c326bd0e4cf01ebc6e2d047f2bb"\
                           "c591a5bfff19512186fbfcbfe1fa32776163a11bef64a8cd1316ba0a5c0b0281803b53787c771671e5fb8c9a7882816375fd38cc9dd15d9958328bdbae6f46ed"\
                           "e3f0ef7269d7129a04198434fecec7f4c5549fef919957282ce007cc0941dcd94d24c03e8301ceb6e32cf5e3a407f30afbe7ce6205a8f6a65a16cf8e2e5310c1"\
                           "ea6b183781f56fb1b1ecac815e55a2dc7618ed6ebaae2dd4cf07c4a00ad2c7f25102818100c22e052f75024c9de0c380ca30081c8a5095ceb8489298d1406345"\
                           "6f207c74964cd65f2f16dba57be3f131f065b9c1eb7aa390f11e4ab0868d31ec116b770b31e89fa4d236541a7a90d3c23c416cc302c360a5587e2cd0bb86dfff"\
                           "91323c4dfa9ea1c1eb33363f3963d18fb5ed6e77b3607ff9e45e71f8020881eafafd213c4f02818004fbb2f7ca0e8e7f644f40939f9743f8996439a926244282"\
                           "1981268f36fba3e4656fc6e9c69bab8b5f56c7b033bed95eeca96952b3d62edd935b80d5187649683196702a0b304e802de7841d6bab06e6877b74bdf2b5e7f2"\
                           "673ac6939c1427fb899a4cb26f656b5621914592f61b10d4ff50a4bb360d134d224a780db10f0f97"
#define DER_ASYM_PRIV_LENG 1192

void setup_asym_keys(void)
{
    uint8_t der_cert[DER_ASYM_PUB_LENG], der_priv[DER_ASYM_PRIV_LENG];

    setup_crypto();

    // Loads certificate from DER
    ck_assert(unhexlify(DER_ASYM_PUB_HEXA, der_cert, DER_ASYM_PUB_LENG) == DER_ASYM_PUB_LENG);
    ck_assert(KeyManager_Certificate_CreateFromDER(der_cert, DER_ASYM_PUB_LENG, &crt_pub) == STATUS_OK);//*/

    // Loads the public key from cert
    key_pub = malloc(sizeof(AsymmetricKey));
    ck_assert(NULL != key_pub);
    ck_assert(KeyManager_Certificate_GetPublicKey(crt_pub, key_pub) == STATUS_OK);

    // Loads the corresponding private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);
    ck_assert(KeyManager_AsymmetricKey_CreateFromBuffer(der_priv, DER_ASYM_PRIV_LENG, &key_priv) == STATUS_OK);
}

void teardown_asym_keys(void)
{
    free(key_pub);
    key_pub = NULL;
    KeyManager_Certificate_Free(crt_pub);
    crt_pub = NULL;
    KeyManager_AsymmetricKey_Free(key_priv);
    key_priv = NULL;

    teardown_crypto();
}


START_TEST(test_crypto_asym_load)
{
    ;
}
END_TEST


START_TEST(test_crypto_asym_lengths)
{
    uint32_t lenPlain = 0, lenCiph = 0, len = 0;

    // Check lengths
    ck_assert(CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_pub, &len) == STATUS_OK);
    ck_assert(2048 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_KeyBits(crypto, key_priv, &len) == STATUS_OK);
    ck_assert(2048 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_pub, &len) == STATUS_OK);
    ck_assert(256 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_KeyBytes(crypto, key_priv, &len) == STATUS_OK);
    ck_assert(256 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_MsgPlainText(crypto, key_pub, &lenPlain) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricGetLength_MsgCipherText(crypto, key_pub, &lenCiph) == STATUS_OK);
    ck_assert(256 == lenCiph);
    ck_assert(214 == lenPlain); // 256 - 2*20 - 2
    ck_assert(CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_pub, &lenCiph, &lenPlain) == STATUS_OK);
    ck_assert(256 == lenCiph);
    ck_assert(214 == lenPlain); // 256 - 2*20 - 2
    ck_assert(CryptoProvider_AsymmetricGetLength_Msgs(crypto, key_priv, &lenCiph, &lenPlain) == STATUS_OK);
    ck_assert(256 == lenCiph);
    ck_assert(214 == lenPlain); // 256 - 2*20 - 2
    ck_assert(CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 32, &len) == STATUS_OK);
    ck_assert(256 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 256, &len) == STATUS_OK);
    ck_assert(214 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_Encryption(crypto, key_pub, 856, &len) == STATUS_OK);
    ck_assert(1024 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_Decryption(crypto, key_priv, 1024, &len) == STATUS_OK);
    ck_assert(856 == len);
    ck_assert(CryptoProvider_AsymmetricGetLength_OAEPHashLength(crypto, &len) == STATUS_OK);
    ck_assert(20 == len); // SHA-1
    ck_assert(CryptoProvider_AsymmetricGetLength_PSSHashLength(crypto, &len) == STATUS_OK);
    ck_assert(32 == len); // SHA-256
    ck_assert(CryptoProvider_AsymmetricGetLength_Signature(crypto, key_pub, &len) == STATUS_OK);
    ck_assert(256 == len); // One block
    ck_assert(CryptoProvider_AsymmetricGetLength_Signature(crypto, key_priv, &len) == STATUS_OK);
    ck_assert(256 == len); // One block
}
END_TEST


START_TEST(test_crypto_asym_crypt)
{
    uint8_t input[856], output[1024], input_bis[856];
    uint32_t len = 0;
    ExposedBuffer clientNonce[32], serverNonce[32];

    // Encryption/Decryption
    // a) Single message (< 214)
    memset(input, 0, 856);
    memset(output, 0, 1024);
    strncpy((char *)input, "Test INGOPCS Test", 32); // And test padding btw...
    ck_assert(CryptoProvider_AsymmetricEncrypt(crypto, input, 32, key_pub, output, 256) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricDecrypt(crypto, output, 256, key_priv, input_bis, 214, &len) == STATUS_OK);
    ck_assert(len == 32);
    ck_assert(memcmp(input, input_bis, 32) == 0);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("3d3b4768f275d5023c2145cbe3a4a592fb843643d791f7bd7fce75ff25128b68", clientNonce, 32) == 32);
    ck_assert(unhexlify("ccee418cbc77c2ebb38d5ffac9d2a9d0a6821fa211798e71b2d65b3abb6aec8f", serverNonce, 32) == 32);
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input+32, 856-32) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricEncrypt(crypto, input, 856, key_pub, output, 1024) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricDecrypt(crypto, output, 1024, key_priv, input_bis, 856, &len) == STATUS_OK);
    ck_assert(len == 856);
    ck_assert(memcmp(input, input_bis, 856) == 0);
}
END_TEST


START_TEST(test_crypto_asym_sign_verify)
{
    uint8_t input[856], sig[256];
    ExposedBuffer clientNonce[32], serverNonce[32];

    // Signature
    // a) Single message (< 214)
    memset(input, 0, 856);
    memset(sig, 0, 256);
    strncpy((char *)input, "Test INGOPCS Test", 32); // And test padding btw...
    ck_assert(CryptoProvider_AsymmetricSign(crypto, input, 32, key_priv, sig, 256) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricVerify(crypto, input, 32, key_pub, sig, 256) == STATUS_OK);
    // b) Multiple messages (> 214, and as output is 1024, < 856)
    //  Using previously generated nonce, to fill input[32:856]
    ck_assert(unhexlify("3d3b4768f275d5023c2145cbe3a4a592fb843643d791f7bd7fce75ff25128b68", clientNonce, 32) == 32);
    ck_assert(unhexlify("ccee418cbc77c2ebb38d5ffac9d2a9d0a6821fa211798e71b2d65b3abb6aec8f", serverNonce, 32) == 32);
    ck_assert(CryptoProvider_DerivePseudoRandomData(crypto, clientNonce, 32, serverNonce, 32, input+32, 856-32) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricSign(crypto, input, 856, key_priv, sig, 256) == STATUS_OK);
    ck_assert(CryptoProvider_AsymmetricVerify(crypto, input, 856, key_pub, sig, 256) == STATUS_OK);
}
END_TEST


START_TEST(test_crypto_asym_copykey)
{
    uint8_t buffer[2048], der_priv[DER_ASYM_PRIV_LENG];
    uint32_t lenDER = 0;

    // Copy to DER
    ck_assert(KeyManager_AsymmetricKey_ToDER(key_priv, buffer, 2048, &lenDER) == STATUS_OK);

    // Loads DER of private key
    ck_assert(unhexlify(DER_ASYM_PRIV_HEXA, der_priv, DER_ASYM_PRIV_LENG) == DER_ASYM_PRIV_LENG);

    // Verifies
    ck_assert(lenDER == DER_ASYM_PRIV_LENG);
    ck_assert(memcmp(buffer, der_priv, DER_ASYM_PRIV_LENG) == 0);
}
END_TEST


// Fixtures for PKI: server.der certificate and CA
Certificate *crt_ca = NULL;
PKIProvider *pki = NULL;

void setup_pki_stack()
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
    ck_assert(KeyManager_Certificate_CreateFromDER(der_ca, 1529, &crt_ca) == STATUS_OK);

    // Creates PKI with ca
    ck_assert(PKIProviderStack_Create(crt_ca, NULL, &pki) == STATUS_OK);
}

void teardown_pki_stack()
{
    PKIProviderStack_Free(pki);
    KeyManager_Certificate_Free(crt_ca);

    teardown_certificate();
}


START_TEST(test_pki_load)
{
    ck_assert(NULL != pki->pFnValidateCertificate);
}
END_TEST


START_TEST(test_pki_cert_validation)
{
    // Checks that the PKI validates our server.pub with our cacert.der
    ck_assert(CryptoProvider_Certificate_Validate(crypto, pki, crt_pub) == STATUS_OK);
}
END_TEST


START_TEST(test_cert_copyder)
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
    ck_assert(KeyManager_Certificate_CopyDER(crt_pub, &buffer0, &lenAlloc0) == STATUS_OK);
    ck_assert(KeyManager_Certificate_CopyDER(crt_pub, &buffer1, &lenAlloc1) == STATUS_OK);

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


Suite *tests_make_suite_crypto_B256S256()
{
    Suite *s = NULL;
    TCase *tc_crypto_symm = NULL, *tc_providers = NULL, *tc_derives = NULL, *tc_misc = NULL, *tc_km = NULL, *tc_crypto_asym = NULL, *tc_pki_stack = NULL;

    s = suite_create("Crypto tests Basic256Sha256");
    tc_crypto_symm = tcase_create("Symmetric Crypto");
    tc_providers = tcase_create("Crypto Provider");
    tc_derives = tcase_create("Crypto Data Derivation");
    tc_misc = tcase_create("Crypto Misc");
    tc_km = tcase_create("Key Management");
    tc_crypto_asym = tcase_create("Asymmetric Crypto");
    tc_pki_stack = tcase_create("PKI stack");

    suite_add_tcase(s, tc_misc);
    tcase_add_test(tc_misc, test_hexlify);

    suite_add_tcase(s, tc_crypto_symm);
    tcase_add_checked_fixture(tc_crypto_symm, setup_crypto, teardown_crypto);
    tcase_add_test(tc_crypto_symm, test_crypto_load);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_lengths);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_crypt);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_sign);
    tcase_add_test(tc_crypto_symm, test_crypto_symm_generate_key);

    suite_add_tcase(s, tc_providers);
    tcase_add_checked_fixture(tc_providers, setup_crypto, teardown_crypto);

    suite_add_tcase(s, tc_derives);
    tcase_add_checked_fixture(tc_derives, setup_crypto, teardown_crypto);
    tcase_add_test(tc_derives, test_crypto_derive_lengths);
    tcase_add_test(tc_derives, test_crypto_derive_data);
    tcase_add_test(tc_derives, test_crypto_derive_keysets);
    // TODO: derive_keysets_client
    // TODO: derive_keysets_server

    suite_add_tcase(s, tc_km);
    tcase_add_checked_fixture(tc_km, setup_certificate, teardown_certificate);
    tcase_add_test(tc_km, test_cert_load);
    tcase_add_test(tc_km, test_cert_lengths);
    tcase_add_test(tc_km, test_cert_thumbprint);
    tcase_add_test(tc_km, test_cert_loadkey);
    tcase_add_test(tc_km, test_cert_copyder);

    suite_add_tcase(s, tc_crypto_asym);
    tcase_add_checked_fixture(tc_crypto_asym, setup_asym_keys, teardown_asym_keys);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_load);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_lengths);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_crypt);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_sign_verify);
    tcase_add_test(tc_crypto_asym, test_crypto_asym_copykey);

    suite_add_tcase(s, tc_pki_stack);
    tcase_add_checked_fixture(tc_pki_stack, setup_pki_stack, teardown_pki_stack);
    tcase_add_test(tc_pki_stack, test_pki_load);
    tcase_add_test(tc_pki_stack, test_pki_cert_validation);

    return s;
}

