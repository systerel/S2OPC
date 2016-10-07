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
    ck_assert(unhexlify("256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa", input, 128) == 128);
    memset(output, 0, sizeof(output));
    memset(hexoutput, 0, sizeof(hexoutput));
    ck_assert(CryptoProvider_SymmetricEncrypt(crypto, input, 128, pSecKey, pSecIV, output, 128) == STATUS_OK);
    ck_assert(hexlify(output, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput, "90b7b9630a2378f53f501ab7beff039155008071bc8438e789932cfd3eb1299195465e6633849463fdb44375278e2fdb1310821e6492cf80ff15cb772509fb426f3aeee27bd4938882fd2ae6b5bd9d91fa4a43b17bb439ebbe59c042310163a82a5fe5388796eee35a181a1271f00be29b852d8fa759bad01ff4678f010594cd", 256) == 0);
    ck_assert(CryptoProvider_SymmetricDecrypt(crypto, output, 128, pSecKey, pSecIV, input, 128) == STATUS_OK);
    ck_assert(hexlify(input, hexoutput, 128) == 128);
    ck_assert(memcmp(hexoutput, "256fd73ce35ae3ea9c25dd2a9454493e96d8633fe633b56176dce8785ce5dbbb84dbf2c8a2eeb1e96b51899605e4f13bbc11b93bf6f39b3469be14858b5b720d4a522d36feed7a329c9b1e852c9280c47db8039c17c4921571a07d1864128330e09c308ddea1694e95c84500f1a61e614197e86a30ecc28df64ccb3ccf5437aa", 256) == 0);
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


#include "mbedtls/entropy.h"
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
    ck_assert(UA_NULL != crypto);
    pCtx = crypto->pCryptolibContext;
    ck_assert(UA_NULL != pCtx);

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
    tcase_add_test(tc_ciphers, test_pk_x509);

    suite_add_tcase(s, tc_providers);

    return s;
}

