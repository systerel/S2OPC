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
 * Copy of CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256 to isaolate a pb with mbedtls.
 */

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline bool NewMsgDigestBuffer(const uint8_t* pInput,
                                      uint32_t lenInput,
                                      const mbedtls_md_info_t* pmd_info_hash,
                                      uint8_t** ppHash);

static int no_random_function(void* p_rng, unsigned char* output, size_t len)
{
    (void) p_rng;
    memset(output, 1, len);
    return 0;
}

int main(int argc, char* argv[])
{
    uint8_t* hash = NULL;
    mbedtls_rsa_context* prsa = NULL;
    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256); // Hash the message with SHA-256
    const uint8_t* pInput = "foobar\n";
    uint32_t lenInput = strlen(pInput) + 1;
    mbedtls_entropy_context ctxEnt;
    mbedtls_ctr_drbg_context ctxDrbg;
    uint8_t pSignature[256];
    mbedtls_pk_context pk;

    /* Prepare entropy contexts */
    mbedtls_entropy_init(&ctxEnt);
    mbedtls_ctr_drbg_init(&ctxDrbg);
    if (mbedtls_ctr_drbg_seed(&ctxDrbg, &mbedtls_entropy_func, &ctxEnt, NULL, 0) != 0)
        return 1;

    /* Hashes the message once */
    /*if (!NewMsgDigestBuffer(pInput, lenInput, pmd_info, &hash))
        return 2;
    */
    /* Arbitrary hash, hash3 is validated */
    uint8_t hash2[32] = {0x49, 0x7a, 0xe2, 0xc5, 0xc2, 0xe9, 0x76, 0xb1, 0x57, 0x03, 0x5f,
                         0xba, 0xec, 0x51, 0xc1, 0x0d, 0x70, 0x7a, 0x6b, 0x92, 0x00, 0x24,
                         0x2f, 0x14, 0xdb, 0xe9, 0x05, 0x97, 0x17, 0x05, 0xa6, 0x37};
    uint8_t hash3[32] = {0x6f, 0x13, 0xeb, 0x68, 0xdd, 0xbe, 0x59, 0x16, 0xcb, 0xe3, 0x48,
                         0x69, 0x6b, 0x31, 0x78, 0xf0, 0x56, 0x13, 0xb0, 0xd8, 0x8a, 0x72,
                         0x46, 0x1c, 0x26, 0x5f, 0xf6, 0xe1, 0xbd, 0x23, 0x4d, 0x02};
    hash = hash2;

    /* Loads the key */
    mbedtls_pk_init(&pk);
    if (mbedtls_pk_parse_keyfile(&pk, "./server_2k.key", NULL) != 0)
        return 3;

    // Sets the appropriate padding mode (no hash-id for PKCS_V15)
    prsa = mbedtls_pk_rsa(pk);
    mbedtls_rsa_set_padding(prsa, MBEDTLS_RSA_PKCS_V15, 0);

    /* Loop sign */
    int i = 0, res = 0;
    for (; res == 0; ++i)
    {
        // res = mbedtls_rsa_rsassa_pkcs1_v15_sign(prsa, mbedtls_ctr_drbg_random, &ctxDrbg,
        res = mbedtls_rsa_rsassa_pkcs1_v15_sign(prsa, no_random_function, &ctxDrbg, MBEDTLS_RSA_PRIVATE,
                                                MBEDTLS_MD_SHA256,
                                                32,                // hashlen is optional, as md_alg is not MD_NONE
                                                hash, pSignature); // signature is as long as the key
        if ((i + 1) % 1000 == 0)
        {
            printf("+");
            fflush(stdout);
        }
    }

    printf("Failed in %i tests\n", i);

    if (NULL != hash)
        free(hash);

    /* Correctly failed... */
    return 0;
}

static inline bool NewMsgDigestBuffer(const uint8_t* pInput,
                                      uint32_t lenInput,
                                      const mbedtls_md_info_t* pmd_info_hash,
                                      uint8_t** ppHash)
{
    uint8_t* hash = NULL;
    uint32_t lenHash = 0;

    if (NULL == ppHash)
        return false;
    *ppHash = NULL;

    if (NULL == pmd_info_hash)
        return false;

    lenHash = mbedtls_md_get_size(pmd_info_hash);
    hash = malloc(lenHash);
    if (NULL == hash)
        return false;
    *ppHash = hash;

    if (mbedtls_md(pmd_info_hash, pInput, lenInput, hash) != 0)
    {
        free(hash);
        return false;
    }

    return true;
}
