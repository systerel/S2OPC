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

#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_helper_string.h"
#include "sopc_key_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "key_manager_lib.h"

// Note : this file MUST be included before other mbedtls headers
#include "mbedtls_common.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/md5.h"
#include "mbedtls/pem.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509.h"

/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */

#define SOPC_CBC_BLOCK_SIZE_BYTES 16
#define SOPC_AES_256_KEY_SIZE_BITS 256
#define SOPC_AES_256_KEY_SIZE_BYTES 32
#define SOPC_RSA_EXPONENT 65537
/* Recommended size by mbedtls sample programs */
#define SOPC_RSA_PRV_PEM_MAX_BYTES 16000
#define SOPC_RSA_PEM_HEADER_ENCRYPTED "-----BEGIN RSA PRIVATE KEY-----\nProc-Type: 4,ENCRYPTED\nDEK-Info: AES-256-CBC,"
#define SOPC_RSA_PEM_FOOTER "-----END RSA PRIVATE KEY-----\n"

/**
 * Creates an asymmetric key from a \p buffer, in DER or PEM format.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(const uint8_t* buffer,
                                                                 uint32_t lenBuf,
                                                                 bool is_public,
                                                                 SOPC_AsymmetricKey** ppKey)
{
    SOPC_AsymmetricKey* key = NULL;

    if (NULL == buffer || 0 == lenBuf || NULL == ppKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    key = SOPC_Malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == key)
        return SOPC_STATUS_NOK;
    key->isBorrowedFromCert = false;
    mbedtls_pk_init(&key->pk);

    int res = -1;

    // MbedTLS fix: mbedtls_pk_parse_key needs a NULL terminated buffer to parse
    // PEM keys.
    if (buffer[lenBuf - 1] != '\0')
    {
        uint8_t* null_terminated_buffer = SOPC_Calloc(1 + lenBuf, sizeof(uint8_t));

        if (null_terminated_buffer == NULL)
        {
            SOPC_Free(key);
            return SOPC_STATUS_OUT_OF_MEMORY;
        }

        memcpy(null_terminated_buffer, buffer, lenBuf);
        res = is_public ? mbedtls_pk_parse_public_key(&key->pk, null_terminated_buffer, 1 + lenBuf)
                        : MBEDTLS_PK_PARSE_KEY(&key->pk, null_terminated_buffer, 1 + lenBuf, NULL, 0);
        SOPC_Free(null_terminated_buffer);
    }

    if (res != 0)
    {
        res = is_public ? mbedtls_pk_parse_public_key(&key->pk, buffer, lenBuf)
                        : MBEDTLS_PK_PARSE_KEY(&key->pk, buffer, lenBuf, NULL, 0);
    }

    if (res != 0)
    {
        SOPC_Free(key);
        return SOPC_STATUS_NOK;
    }

    *ppKey = key;

    return SOPC_STATUS_OK;
}

/**
 * Helper function: calls KeyManager_AsymmetricKey_CreateFromBuffer() on the content of the file \p szPath.
 *
 * \note    Not in unit tests.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromFile(const char* szPath,
                                                               SOPC_AsymmetricKey** ppKey,
                                                               char* password,
                                                               uint32_t lenPassword)
{
    if (NULL == szPath || NULL == ppKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check password
    if (NULL == password && 0 != lenPassword)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != password && (0 == lenPassword || '\0' != password[lenPassword]))
        return SOPC_STATUS_INVALID_PARAMETERS;
#if defined(MBEDTLS_FS_IO)
    SOPC_AsymmetricKey* key = NULL;

    key = SOPC_Malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == key)
        return SOPC_STATUS_NOK;
    key->isBorrowedFromCert = false;
    mbedtls_pk_init(&key->pk);

    if (mbedtls_pk_parse_keyfile(&key->pk, szPath, password) != 0)
    {
        SOPC_Free(key);
        return SOPC_STATUS_NOK;
    }

    *ppKey = key;

    return SOPC_STATUS_OK;
#else
    return SOPC_STATUS_NOK;
#endif
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_GenRSA(uint32_t RSAKeySize, SOPC_AsymmetricKey** ppKey)
{
    if (NULL == ppKey || 0 == RSAKeySize)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    mbedtls_entropy_context ctxEnt = {0};
    mbedtls_ctr_drbg_context ctxDrbg = {0};
    SOPC_AsymmetricKey* pKey = SOPC_Malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == pKey)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pKey->isBorrowedFromCert = false;
    /* Let mbedtls looks in the host system to get the entropy sources
      (standards like the /dev/urandom or Windows CryptoAPI.) */
    mbedtls_entropy_init(&ctxEnt);
    mbedtls_ctr_drbg_init(&ctxDrbg);
    mbedtls_pk_init(&pKey->pk);
    int errLib = mbedtls_ctr_drbg_seed(&ctxDrbg, &mbedtls_entropy_func, &ctxEnt, NULL, 0);
    if (!errLib)
    {
        /* Generate the key */
        const mbedtls_pk_info_t* pPkInfo = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);
        errLib = mbedtls_pk_setup(&pKey->pk, pPkInfo);
    }
    if (!errLib)
    {
        mbedtls_rsa_context* pCtxRSA = mbedtls_pk_rsa(pKey->pk);
        errLib = mbedtls_rsa_gen_key(pCtxRSA, mbedtls_ctr_drbg_random, &ctxDrbg, RSAKeySize, SOPC_RSA_EXPONENT);
    }
    if (errLib)
    {
        SOPC_KeyManager_AsymmetricKey_Free(pKey);
        pKey = NULL;
    }

    *ppKey = pKey;
    mbedtls_entropy_free(&ctxEnt);
    mbedtls_ctr_drbg_free(&ctxDrbg);
    return errLib ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_CertificateList* pCert,
                                                                      SOPC_AsymmetricKey** pKey)
{
    if (NULL == pCert || NULL == pKey)
        return SOPC_STATUS_INVALID_PARAMETERS;
    *pKey = SOPC_Malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == *pKey)
        return SOPC_STATUS_NOK;
    (*pKey)->isBorrowedFromCert = true;
    mbedtls_pk_init(&(*pKey)->pk);

    return KeyManager_Certificate_GetPublicKey(pCert, *pKey);
}

/**
 * Frees an asymmetric key created with KeyManager_AsymmetricKey_Create*().
 *
 * \warning     Only keys created with KeyManager_AsymmetricKey_Create*() should be freed that way.
 */
void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey)
{
    if (NULL == pKey)
        return;
    if (false == pKey->isBorrowedFromCert)
    {
        mbedtls_pk_free(&pKey->pk);
    }
    SOPC_Free(pKey);
}

/**
 * \brief   Creates a DER from the AsymmetricKey \p pKey and copies it to \p pDest.
 *
 *   This function does not allocate the buffer containing the DER.
 *   The operation may fail if the allocated buffer is not large enough.
 *   The required length cannot be precisely calculated, but a value of 8 times the key length in bytes is recommended.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToDER(const SOPC_AsymmetricKey* pKey,
                                                      bool is_public,
                                                      uint8_t* pDest,
                                                      uint32_t lenDest,
                                                      uint32_t* pLenWritten)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint8_t* buffer = NULL;
    int lengthWritten = 0;

    if (NULL == pKey || NULL == pDest || 0 == lenDest || NULL == pLenWritten)
        return SOPC_STATUS_INVALID_PARAMETERS;

    buffer = SOPC_Malloc(lenDest);
    if (NULL == buffer)
        return SOPC_STATUS_NOK;
    // Asymmetric key should be const in mbedtls
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    lengthWritten = is_public ? mbedtls_pk_write_pubkey_der(&((SOPC_AsymmetricKey*) pKey)->pk, buffer, lenDest)
                              : mbedtls_pk_write_key_der(&((SOPC_AsymmetricKey*) pKey)->pk, buffer, lenDest);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    if (lengthWritten > 0 && (uint32_t) lengthWritten <= lenDest)
    {
        *pLenWritten = (uint32_t) lengthWritten;
        memcpy(pDest, buffer + lenDest - *pLenWritten, *pLenWritten);
        status = SOPC_STATUS_OK;
    }

    SOPC_Free(buffer);

    return status;
}

static int sopc_write_key_pem_file(unsigned char* PEMToWrite, const char* filePath)
{
    SOPC_ASSERT(NULL != PEMToWrite);
    SOPC_ASSERT(NULL != filePath);

    size_t lenToWrite = 0;
    FILE* fp = NULL;
    lenToWrite = strlen((char*) PEMToWrite);
    fp = fopen(filePath, "wb");
    if (NULL != fp)
    {
        size_t nb_written = fwrite(PEMToWrite, 1, lenToWrite, fp);
        fclose(fp);
        if (lenToWrite != nb_written)
        {
            remove(filePath);
            return -2;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

static int sopc_export_private_key(SOPC_AsymmetricKey* pKey, const bool bIsPublic, const char* filePath)
{
    SOPC_ASSERT(NULL != pKey);
    SOPC_ASSERT(NULL != filePath);

    unsigned char bufPEM[SOPC_RSA_PRV_PEM_MAX_BYTES];
    int errLib = -1;
    if (bIsPublic)
    {
        errLib = mbedtls_pk_write_pubkey_pem(&pKey->pk, bufPEM, sizeof(bufPEM));
    }
    else
    {
        errLib = mbedtls_pk_write_key_pem(&pKey->pk, bufPEM, sizeof(bufPEM));
    }
    if (!errLib)
    {
        errLib = sopc_write_key_pem_file(bufPEM, filePath);
    }
    /* Clear */
    mbedtls_platform_zeroize(bufPEM, sizeof(bufPEM));
    return errLib;
}

static int sopc_set_pem_header(unsigned char* pIv, char** ppPEMHeader)
{
    SOPC_ASSERT(NULL != pIv);
    SOPC_ASSERT(NULL != ppPEMHeader);
    *ppPEMHeader = NULL;
    /* Convert the IV to an hex string */
    char pIvHex[SOPC_CBC_BLOCK_SIZE_BYTES * 2 + 1]; // +1 for the \0 (necessary for snprintf)
    int res = 0;
    for (size_t i = 0; i < SOPC_CBC_BLOCK_SIZE_BYTES && 0 <= res; ++i)
    {
        res = snprintf(pIvHex + 2 * i, 3, "%02X", pIv[i]);
    }
    if (0 > res)
    {
        return -1;
    }
    pIvHex[SOPC_CBC_BLOCK_SIZE_BYTES * 2] = '\0';
    size_t PEMHeaderLen =
        strlen(SOPC_RSA_PEM_HEADER_ENCRYPTED) + SOPC_CBC_BLOCK_SIZE_BYTES * 2 + 3; // +3 for \n + \n + \0
    char* pPEMHeader = SOPC_Malloc(PEMHeaderLen * sizeof(unsigned char));
    if (NULL == pPEMHeader)
    {
        return -2;
    }
    res = snprintf(pPEMHeader, PEMHeaderLen, "%s%s\n\n", SOPC_RSA_PEM_HEADER_ENCRYPTED, pIvHex);
    if (0 > res)
    {
        SOPC_Free(pPEMHeader);
        return -1;
    }
    *ppPEMHeader = pPEMHeader;
    return 0;
}

static int sopc_md5_update_pwd_iv(mbedtls_md5_context* ctx,
                                  unsigned char* pIv,
                                  const unsigned char* pwd,
                                  size_t pwdLen,
                                  unsigned char* pSum)
{
    SOPC_ASSERT(NULL != ctx);
    SOPC_ASSERT(NULL != pIv);
    SOPC_ASSERT(NULL != pwd);
    SOPC_ASSERT(0 != pwdLen);
    SOPC_ASSERT('\0' == pwd[pwdLen]);
    SOPC_ASSERT(NULL != pSum);

    /*  S = pIv[0..7]
        pSum[0..15]  = MD5(pwd || S)
    */
    unsigned char sum[16];
    int errLib = mbedtls_md5_update_ret(ctx, pwd, pwdLen);
    if (!errLib)
    {
        errLib = mbedtls_md5_update_ret(ctx, pIv, 8);
    }
    if (!errLib)
    {
        errLib = mbedtls_md5_finish_ret(ctx, sum);
    }
    if (!errLib)
    {
        memcpy(pSum, sum, SOPC_CBC_BLOCK_SIZE_BYTES);
    }
    /* clear */
    mbedtls_platform_zeroize(sum, SOPC_CBC_BLOCK_SIZE_BYTES);
    return errLib;
}

/**
 * \brief PBKDF1-MD5
 *
 * 			S = pIv[0..7]
 * 			pKey[0..15]  = MD5(pwd || S)
 * 			pKey[16..31] = MD5(pKey[0..15] || pwd || S)
 *
 */
static int sopc_create_aes256_key_with_pbkdf1_md5(unsigned char* pKey,
                                                  unsigned char* pIv,
                                                  const unsigned char* pwd,
                                                  size_t pwdLen)
{
    SOPC_ASSERT(NULL != pKey);
    SOPC_ASSERT(NULL != pIv);
    SOPC_ASSERT(NULL != pwd);
    SOPC_ASSERT(0 != pwdLen);
    SOPC_ASSERT('\0' == pwd[pwdLen]);

    mbedtls_md5_context ctx = {0};
    unsigned char sum[16];
    int errLib = mbedtls_md5_starts_ret(&ctx);
    if (!errLib)
    {
        errLib = sopc_md5_update_pwd_iv(&ctx, pIv, pwd, pwdLen, sum);
    }
    if (!errLib)
    {
        memcpy(pKey, sum, SOPC_CBC_BLOCK_SIZE_BYTES);
        errLib = mbedtls_md5_starts_ret(&ctx);
    }
    if (!errLib)
    {
        errLib = mbedtls_md5_update_ret(&ctx, sum, SOPC_CBC_BLOCK_SIZE_BYTES);
    }
    if (!errLib)
    {
        errLib = sopc_md5_update_pwd_iv(&ctx, pIv, pwd, pwdLen, sum);
    }
    if (!errLib)
    {
        memcpy(pKey + SOPC_CBC_BLOCK_SIZE_BYTES, sum, SOPC_CBC_BLOCK_SIZE_BYTES);
    }
    /* Clear */
    mbedtls_md5_free(&ctx);
    mbedtls_platform_zeroize(sum, SOPC_CBC_BLOCK_SIZE_BYTES);
    return errLib;
}

static int sopc_rsa_pem_aes256_cbc_encrypt(unsigned char* pIv,
                                           unsigned char* pRsaKeyDER,
                                           size_t rsaKeyDERLen,
                                           const char* pwd,
                                           size_t pwdLen)
{
    SOPC_ASSERT(NULL != pIv);
    SOPC_ASSERT(NULL != pRsaKeyDER);
    SOPC_ASSERT(0 != rsaKeyDERLen);
    SOPC_ASSERT(NULL != pwd);
    SOPC_ASSERT(0 != pwdLen);
    SOPC_ASSERT('\0' == pwd[pwdLen]);

    unsigned char pIvCopy[SOPC_CBC_BLOCK_SIZE_BYTES];
    memcpy(pIvCopy, pIv, SOPC_CBC_BLOCK_SIZE_BYTES);
    mbedtls_aes_context ctx = {0};
    unsigned char aesKey[SOPC_AES_256_KEY_SIZE_BYTES];
    mbedtls_aes_init(&ctx);
    int errLib = sopc_create_aes256_key_with_pbkdf1_md5(aesKey, pIv, (const unsigned char*) pwd, pwdLen);
    if (!errLib)
    {
        errLib = mbedtls_aes_setkey_enc(&ctx, aesKey, SOPC_AES_256_KEY_SIZE_BITS);
    }
    if (!errLib)
    {
        errLib = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, rsaKeyDERLen, pIvCopy, pRsaKeyDER, pRsaKeyDER);
    }

    /* Clear */
    mbedtls_aes_free(&ctx);
    mbedtls_platform_zeroize(aesKey, SOPC_AES_256_KEY_SIZE_BYTES);
    mbedtls_platform_zeroize(pIvCopy, SOPC_CBC_BLOCK_SIZE_BYTES);

    return errLib;
}

static int sopc_gen_aes_iv(unsigned char pIv[SOPC_CBC_BLOCK_SIZE_BYTES])
{
    SOPC_ASSERT(NULL != pIv);
    mbedtls_entropy_context ctxEnt = {0};
    mbedtls_ctr_drbg_context ctxDrbg = {0};
    /* Let mbedtls looks in the host system to get the entropy sources
      (standards like the /dev/urandom or Windows CryptoAPI.) */
    mbedtls_entropy_init(&ctxEnt);
    mbedtls_ctr_drbg_init(&ctxDrbg);
    int errLib = mbedtls_ctr_drbg_seed(&ctxDrbg, &mbedtls_entropy_func, &ctxEnt, NULL, 0);
    if (!errLib)
    {
        errLib = mbedtls_ctr_drbg_random(&ctxDrbg, pIv, SOPC_CBC_BLOCK_SIZE_BYTES);
    }
    /* Clear */
    mbedtls_entropy_free(&ctxEnt);
    mbedtls_ctr_drbg_free(&ctxDrbg);
    return errLib;
}

static size_t sopc_set_pkcs5_padding(unsigned char* bufDERtoEnc, size_t bufDERtoEncLen, size_t bufDERLen)
{
    SOPC_ASSERT(NULL != bufDERtoEnc);
    size_t padLen = SOPC_CBC_BLOCK_SIZE_BYTES - (bufDERLen % SOPC_CBC_BLOCK_SIZE_BYTES);
    SOPC_ASSERT(bufDERLen + padLen <= bufDERtoEncLen && "Buffer is too small for padding");
    if (padLen)
    {
        for (size_t i = 0; i < padLen; i++)
        {
            bufDERtoEnc[bufDERLen + i] = (unsigned char) padLen;
        }
    }
    return padLen;
}

static int sopc_encrypt_and_export_private_key(SOPC_AsymmetricKey* pKey,
                                               const char* filePath,
                                               const char* pwd,
                                               size_t pwdLen)
{
    SOPC_ASSERT(NULL != pKey);
    SOPC_ASSERT(NULL != filePath);
    SOPC_ASSERT(NULL != pwd);
    SOPC_ASSERT(0 != pwdLen);
    SOPC_ASSERT('\0' == pwd[pwdLen]);

    int errLib = -1;
    /* PEM is larger than DER */
    unsigned char bufDER[SOPC_RSA_PRV_PEM_MAX_BYTES];
    unsigned char bufDERtoEnc[SOPC_RSA_PRV_PEM_MAX_BYTES];
    unsigned char bufPEMToWrite[SOPC_RSA_PRV_PEM_MAX_BYTES];
    unsigned char pIv[SOPC_CBC_BLOCK_SIZE_BYTES];
    char* pPEMHeader = NULL;
    size_t padLen = 0;
    size_t bufDERLen = 0;
    /* Extract the DER part */
    errLib = mbedtls_pk_write_key_der(&pKey->pk, bufDER, sizeof(bufDER));
    if (0 > errLib)
    {
        return errLib;
    }
    bufDERLen = (size_t) errLib;
    /* mbedtls writes the data at the end of the buffer */
    memcpy(bufDERtoEnc, bufDER + sizeof(bufDER) - bufDERLen, bufDERLen);
    /* PKCS5 Padding */
    padLen = sopc_set_pkcs5_padding(bufDERtoEnc, sizeof(bufDERtoEnc), bufDERLen);
    /* Generate the IV */
    errLib = sopc_gen_aes_iv(pIv);
    /* Encrypt the key */
    if (!errLib)
    {
        errLib = sopc_rsa_pem_aes256_cbc_encrypt(pIv, bufDERtoEnc, bufDERLen + padLen, pwd, pwdLen);
    }
    /* Set the IV in the header */
    if (!errLib)
    {
        errLib = sopc_set_pem_header(pIv, &pPEMHeader);
    }
    /* Write the key in a PEM buffer */
    if (!errLib)
    {
        size_t olen = 0;
        errLib = mbedtls_pem_write_buffer(pPEMHeader, SOPC_RSA_PEM_FOOTER, bufDERtoEnc, bufDERLen + padLen,
                                          bufPEMToWrite, SOPC_RSA_PRV_PEM_MAX_BYTES, &olen);
    }
    /* Export the key in a PEM file */
    if (!errLib)
    {
        errLib = sopc_write_key_pem_file(bufPEMToWrite, filePath);
    }
    /* Clear */
    mbedtls_platform_zeroize(bufDER, SOPC_RSA_PRV_PEM_MAX_BYTES);
    mbedtls_platform_zeroize(bufDERtoEnc, SOPC_RSA_PRV_PEM_MAX_BYTES);
    mbedtls_platform_zeroize(bufPEMToWrite, SOPC_RSA_PRV_PEM_MAX_BYTES);
    SOPC_Free(pPEMHeader);
    return errLib;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToPEMFile(SOPC_AsymmetricKey* pKey,
                                                          const bool bIsPublic,
                                                          const char* filePath,
                                                          const char* pwd,
                                                          const uint32_t pwdLen)
{
    if (NULL == pKey || NULL == filePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pwd && 0 != pwdLen)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pwd && (0 == pwdLen || '\0' != pwd[pwdLen] || bIsPublic))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    int errLib = -1;
    if (NULL == pwd)
    {
        errLib = sopc_export_private_key(pKey, bIsPublic, filePath);
    }
    else
    {
        errLib = sopc_encrypt_and_export_private_key(pKey, filePath, pwd, pwdLen);
    }
    return errLib ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(const SOPC_AsymmetricKey* pKey,
                                                                        bool is_public,
                                                                        SOPC_SerializedAsymmetricKey** out)
{
    if (NULL == pKey || NULL == out)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t lenBits = mbedtls_pk_get_bitlen(&pKey->pk);
    if (lenBits > UINT32_MAX)
    {
        return SOPC_STATUS_NOK;
    }

    uint32_t lenBytes = (uint32_t)(lenBits / 8);
    uint8_t* buffer =
        SOPC_Malloc(sizeof(uint8_t) * lenBytes * 8); // a value of 8 times the key length in bytes is recommended
    if (NULL == buffer)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    uint32_t pLenWritten = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_ToDER(pKey, is_public, buffer, lenBytes * 8, &pLenWritten);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(buffer, pLenWritten, out);
    }

    SOPC_Free(buffer);

    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

/* Create a certificate if \p ppCert points to NULL, do nothing otherwise */
static SOPC_ReturnStatus certificate_maybe_create(SOPC_CertificateList** ppCert)
{
    if (NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* certif = *ppCert;
    if (NULL == certif)
    {
        certif = SOPC_Calloc(1, sizeof(SOPC_CertificateList)); /* Also init certificate */
    }
    if (NULL == certif)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    *ppCert = certif;

    return SOPC_STATUS_OK;
}

/* Check lengths of loaded certificates */
static SOPC_ReturnStatus certificate_check_length(SOPC_CertificateList* pCert)
{
    /* Check that all loaded certificates fit in uint32-addressable buffers */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    mbedtls_x509_crt* crt = &pCert->crt;
    for (; NULL != crt && SOPC_STATUS_OK == status; crt = crt->next)
    {
        if (crt->raw.len > UINT32_MAX)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                                 uint32_t lenDER,
                                                                 SOPC_CertificateList** ppCert)
{
    if (NULL == bufferDER || 0 == lenDER || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = certificate_maybe_create(ppCert);
    SOPC_CertificateList* pCert = *ppCert;

    if (SOPC_STATUS_OK == status)
    {
        int err = mbedtls_x509_crt_parse(&pCert->crt, bufferDER, lenDER);
        if (0 != err)
        {
            status = SOPC_STATUS_NOK;
            fprintf(stderr, "> KeyManager: certificate buffer parse failed with error code: -0x%X\n",
                    (unsigned int) -err);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = certificate_check_length(pCert);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCert);
        *ppCert = NULL;
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromFile(const char* szPath, SOPC_CertificateList** ppCert)
{
    if (NULL == szPath || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

#if defined(MBEDTLS_FS_IO)
    SOPC_ReturnStatus status = certificate_maybe_create(ppCert);
    SOPC_CertificateList* pCert = NULL;

    if (SOPC_STATUS_OK == status)
    {
        pCert = *ppCert;
    }

    if (SOPC_STATUS_OK == status)
    {
        int err = mbedtls_x509_crt_parse_file(&pCert->crt, szPath);
        if (0 != err)
        {
            status = SOPC_STATUS_NOK;
            fprintf(stderr, "> KeyManager: certificate file \"%s\" parse failed with error code: -0x%X\n", szPath,
                    (unsigned int) -err);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = certificate_check_length(pCert);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCert);
        *ppCert = NULL;
    }

    return status;
#else
    return SOPC_STATUS_NOK;
#endif
}

void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert)
{
    if (NULL == pCert)
        return;

    /* Frees all the certificates in the chain */
    mbedtls_x509_crt_free(&pCert->crt);
    SOPC_Free(pCert);
}

/** Assert that a certificate is passed, returns SOPC_STATUS_OK if the certificate chain is of length 1 */
static SOPC_ReturnStatus certificate_check_single(const SOPC_CertificateList* pCert)
{
    size_t nCert = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &nCert);
    if (SOPC_STATUS_OK == status && 1 != nCert)
    {
        status = SOPC_STATUS_NOK;
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER(const SOPC_CertificateList* pCert,
                                                    uint8_t** ppDest,
                                                    uint32_t* pLenAllocated)
{
    uint32_t lenToAllocate = 0;

    if (NULL == pCert || NULL == ppDest || NULL == pLenAllocated)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = certificate_check_single(pCert);

    /* Allocation */
    if (SOPC_STATUS_OK == status)
    {
        lenToAllocate = (uint32_t) pCert->crt.raw.len;
        if (lenToAllocate == 0)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        (*ppDest) = SOPC_Malloc(lenToAllocate);
        if (NULL == *ppDest)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Copy */
    if (SOPC_STATUS_OK == status)
    {
        memcpy((void*) (*ppDest), (void*) (pCert->crt.raw.p), lenToAllocate);
        *pLenAllocated = lenToAllocate;
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_CertificateList* pCert,
                                                            uint8_t* pDest,
                                                            uint32_t lenDest)
{
    if (NULL == pProvider || NULL == pCert || NULL == pDest || 0 == lenDest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = certificate_check_single(pCert);

    /* Assert allocation length */
    uint32_t lenSupposed = 0;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_CertificateGetLength_Thumbprint(pProvider, &lenSupposed);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (lenDest != lenSupposed)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    // Get DER
    uint8_t* pDER = NULL;
    uint32_t lenDER = 0;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_ToDER(pCert, &pDER, &lenDER);
    }

    mbedtls_md_type_t type = MBEDTLS_MD_NONE;
    if (SOPC_STATUS_OK == status)
    {
        // Hash DER with SHA-1
        switch (pProfile->SecurityPolicyID)
        {
        case SOPC_SecurityPolicy_Invalid_ID:
        default:
            status = SOPC_STATUS_NOK;
            break;
        case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        case SOPC_SecurityPolicy_Basic256Sha256_ID:
        case SOPC_SecurityPolicy_Basic256_ID:
            type = MBEDTLS_MD_SHA1;
            break;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        const mbedtls_md_info_t* pmd = mbedtls_md_info_from_type(type);
        if (mbedtls_md(pmd, pDER, lenDER, pDest) != 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    SOPC_Free(pDER);

    return status;
}

/**
 * \brief       Fills \p pKey with public key information retrieved from \p pCert.
 * \warning     \p pKey is not valid anymore when \p pCert is freed.
 */
SOPC_ReturnStatus KeyManager_Certificate_GetPublicKey(const SOPC_CertificateList* pCert, SOPC_AsymmetricKey* pKey)
{
    if (NULL == pCert || NULL == pKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    memcpy(&pKey->pk, &pCert->crt.pk, sizeof(mbedtls_pk_context));

    return SOPC_STATUS_OK;
}

static size_t ptr_offset(const void* p, const void* start)
{
    SOPC_ASSERT(p >= start);
    return (size_t)(((const uint8_t*) p) - ((const uint8_t*) start));
}

static void* mem_search(const void* mem, size_t mem_len, const void* needle, size_t needle_len)
{
    if (mem_len == 0 || needle_len == 0)
    {
        return NULL;
    }

    void* start = memchr(mem, *((const uint8_t*) needle), mem_len);

    if (start == NULL)
    {
        return NULL;
    }

    size_t offset = ptr_offset(start, mem);
    SOPC_ASSERT(offset < mem_len);

    if (mem_len - offset < needle_len)
    {
        return NULL;
    }

    if (memcmp(start, needle, needle_len) == 0)
    {
        return start;
    }
    else
    {
        // This is tail recursive, so we could turn this into a loop... I
        // find the recursive version more readable (at the cost of a potential
        // stack overflow).
        return mem_search(((const uint8_t*) mem) + offset + 1, mem_len - offset - 1, needle, needle_len);
    }
}

/* Does not copy the string. Returns NULL if nothing found. In this case str_len might have been modified. */
static const uint8_t* get_application_uri_ptr_from_crt_data(const SOPC_CertificateList* crt, uint8_t* str_len)
{
    // Number belows are taken from ASN1 and RFC5280 section 4.2.1.6
    static const uint8_t ASN_SEQUENCE_TAG = 0x30;
    // id-ce-subjectAltName
    static const uint8_t ALT_NAMES_START[] = {0x03, 0x55, 0x1D, 0x11};

    const void* alt_names_start =
        mem_search(crt->crt.v3_ext.p, crt->crt.v3_ext.len, ALT_NAMES_START, sizeof(ALT_NAMES_START));

    if (alt_names_start == NULL)
    {
        return NULL;
    }

    size_t remaining_len = crt->crt.v3_ext.len - ptr_offset(alt_names_start, crt->crt.v3_ext.p);

    // We should have:
    // - id-ce-subjectAltName: 4 bytes
    // - "critical" flag: 1 byte
    // - length of object: 1 byte
    // - sequence tag for GeneralNames: 1 byte
    // - length of GeneralNames sequence: 1 byte
    // - sequence data...

    if (remaining_len < 8)
    {
        // Probably not the start of subjectAltNames, or invalid encoding
        return NULL;
    }

    uint8_t object_len = *((const uint8_t*) alt_names_start + 5);

    if (object_len < 2 || object_len > (remaining_len - 6))
    {
        // Invalid object length
        return NULL;
    }

    uint8_t sequence_tag = *((const uint8_t*) alt_names_start + 6);

    if (sequence_tag != ASN_SEQUENCE_TAG)
    {
        return NULL;
    }

    uint8_t sequence_len = *((const uint8_t*) alt_names_start + 7);

    if (sequence_len > (object_len - 2))
    {
        // Invalid sequence length
        return NULL;
    }

    const void* sequence_data_start = ((const uint8_t*) alt_names_start) + 8;
    const size_t sequence_data_len = remaining_len - 8;

    // Each GeneralName (sequence item) has a tag which is (0x80 | index) where
    // index is the choice index (0x80 is the "context specific tag 0").
    // We're interested in the uniformResourceIdentifier choice, which has index
    // 6.
    //
    // The tag should be followed by a IA5String, which is one byte for the
    // string length followed by the string data.

    const void* uri_start = memchr(sequence_data_start, 0x86, sequence_data_len);

    if (uri_start == NULL)
    {
        return NULL;
    }

    remaining_len = sequence_data_len - ptr_offset(uri_start, sequence_data_start);

    // tag + string length
    if (remaining_len < 2)
    {
        return NULL;
    }

    *str_len = *(((const uint8_t*) uri_start) + 1);

    // An URI is a scheme + some data, so at least 3 characters.
    if ((*str_len < 3) || (*str_len > (remaining_len - 2)))
    {
        return NULL;
    }

    return (((const uint8_t*) uri_start) + 2);
}

bool SOPC_KeyManager_Certificate_CheckApplicationUri(const SOPC_CertificateList* pCert, const char* application_uri)
{
    SOPC_ASSERT(pCert != NULL);
    SOPC_ASSERT(application_uri != NULL);
    SOPC_ReturnStatus status = certificate_check_single(pCert);

    uint8_t str_len = 0;
    const void* str_data = NULL;

    if (SOPC_STATUS_OK == status)
    {
        str_data = get_application_uri_ptr_from_crt_data(pCert, &str_len);
        if (NULL == str_data)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        if (strlen(application_uri) != str_len)
        {
            return false;
        }
    }

    return SOPC_STATUS_OK == status && strncmp(application_uri, str_data, str_len) == 0;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_CertificateList* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength)
{
    SOPC_ASSERT(NULL != pCert);
    SOPC_ASSERT(NULL != ppApplicationUri);
    SOPC_ReturnStatus status = certificate_check_single(pCert);

    uint8_t str_len = 0;
    const void* str_data = NULL;

    if (SOPC_STATUS_OK == status)
    {
        str_data = get_application_uri_ptr_from_crt_data(pCert, &str_len);
        if (NULL == str_data)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    char* data_copy = NULL;

    if (SOPC_STATUS_OK == status)
    {
        data_copy = SOPC_Calloc(str_len + 1U, sizeof(char));
        if (NULL == data_copy)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        memcpy(data_copy, str_data, str_len);
        *ppApplicationUri = data_copy;
        if (NULL != pStringLength)
        {
            *pStringLength = str_len;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetListLength(const SOPC_CertificateList* pCert, size_t* pLength)
{
    if (NULL == pCert || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const mbedtls_x509_crt* cert = &pCert->crt;
    size_t i = 0;
    for (; NULL != cert; ++i)
    {
        cert = cert->next;
    }

    *pLength = i;

    return SOPC_STATUS_OK;
}

/* Creates a new string: free the result */
static char* get_raw_sha1(const mbedtls_x509_buf* raw)
{
    SOPC_ASSERT(NULL != raw);

    /* Make SHA-1 thumbprint */
    const mbedtls_md_info_t* pmd = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    uint8_t pDest[20];

    int err = mbedtls_md(pmd, raw->p, raw->len, pDest);
    if (0 != err)
    {
        fprintf(stderr, "Cannot compute thumbprint of certificate, err -0x%X\n", (unsigned int) -err);
        return NULL;
    }

    /* Poor-man's SHA-1 format */
    char* ret = SOPC_Calloc(41, sizeof(char));
    if (NULL == ret)
    {
        return NULL;
    }
    for (size_t i = 0; i < 20; ++i)
    {
        snprintf(ret + 2 * i, 3, "%02X", pDest[i]);
    }
    ret[40] = '\0';

    return ret;
}

/* Creates a new string: free the result */
static char* get_crt_sha1(const mbedtls_x509_crt* crt)
{
    return get_raw_sha1(&crt->raw);
}

char* SOPC_KeyManager_Certificate_GetCstring_SHA1(const SOPC_CertificateList* pCert)
{
    char* sha_1_cert = NULL;

    if (NULL == pCert)
    {
        return sha_1_cert;
    }

    SOPC_ReturnStatus status = certificate_check_single(pCert);
    if (SOPC_STATUS_OK != status)
    {
        return sha_1_cert;
    }

    const mbedtls_x509_crt* crt = &pCert->crt;
    sha_1_cert = get_crt_sha1(crt);
    return sha_1_cert;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(SOPC_CertificateList* pCert,
                                                                     const SOPC_CRLList* pCRL,
                                                                     bool* pbMatch)
{
    if (NULL == pCRL || NULL == pCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* For each CA, find its CRL. If not found, log and match = false */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool list_match = true;
    mbedtls_x509_crt* crt = &pCert->crt;
    mbedtls_x509_crt* prev = NULL; /* Parent of current cert */
    while (NULL != crt)
    {
        /* Skip certificates that are not authorities */
        if (!crt->ca_istrue)
        {
            /* Iterate */
            prev = crt;
            crt = crt->next;
            continue;
        }

        int crl_found = 0;
        const mbedtls_x509_crl* crl = &pCRL->crl;
        for (; NULL != crl; crl = crl->next)
        {
            /* This is the test done by mbedtls internally in x509_crt_verifycrl.
             * It verifies the subject only, but further verifications are done by mbedtls.
             * With this test, the CA list is restricted to have only one CA with a given subject.
             * Without this restriction, we could have a newer and an older version of the same CA,
             * which in any case would be confusing for end users. */
            bool match = crl->issuer_raw.len == crt->subject_raw.len &&
                         memcmp(crl->issuer_raw.p, crt->subject_raw.p, crl->issuer_raw.len) == 0;
            if (crl_found > 0 && match)
            {
                if (1 == crl_found)
                {
                    char* fpr = get_crt_sha1(crt);
                    fprintf(
                        stderr,
                        "> MatchCRLList warning: Certificate with SHA-1 fingerprint %s has more than one associated "
                        "CRL.\n",
                        fpr);
                    SOPC_Free(fpr);
                }
                if (crl_found < INT_MAX)
                {
                    ++crl_found;
                }
            }
            else if (match)
            {
                ++crl_found;
            }
        }

        if (1 != crl_found)
        {
            list_match = false;
            char* fpr = get_crt_sha1(crt);
            fprintf(stderr,
                    "> MatchCRLList warning: Certificate with SHA-1 fingerprint %s has no CRL or multiple CRLs, and is "
                    "removed from the CAs list.\n",
                    fpr);
            SOPC_Free(fpr);

            /* Remove the certificate from the chain and safely delete it */
            mbedtls_x509_crt* next = crt->next;
            crt->next = NULL;
            mbedtls_x509_crt_free(crt);

            /* Set new next certificate (if possible) */
            if (NULL == prev)
            {
                if (NULL == next)

                {
                    /*
                     * The list would be empty, but we cannot do it here.
                     * We have no choice but failing with current design.
                     */
                    crt = NULL; // make iteration stop
                    status = SOPC_STATUS_NOK;
                }
                else
                {
                    /* Head of the chain is a special case */
                    pCert->crt = *next;
                    /* We have to free the new next certificate (copied as first one) */
                    SOPC_Free(next);

                    /* Do not iterate: current certificate has changed */
                }
            }
            else
            {
                /* We have to free the certificate if it is not the first in the list */
                SOPC_Free(crt);
                prev->next = next;

                /* Iterate */
                crt = next;
            }
        }
        else
        {
            /* Iterate */
            prev = crt;
            crt = crt->next;
        }
    }

    /* There may be unused CRLs */
    if (NULL != pbMatch)
    {
        *pbMatch = list_match;
    }

    return status;
}

static SOPC_ReturnStatus raw_buf_to_der_file(mbedtls_x509_buf* buf, const char* directoryPath)
{
    SOPC_ASSERT(NULL != buf && NULL != directoryPath);

    char* basePath = NULL;
    char* filePath = NULL;
    char* fileName = NULL;
    FILE* fp = NULL;
    /* Compute the file name (SHA1) */
    char* thumbprint = get_raw_sha1(buf);
    SOPC_ReturnStatus status = NULL == thumbprint ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StrConcat(thumbprint, ".der", &fileName);
    }
    /* Compute the file path */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StrConcat(directoryPath, "/", &basePath);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StrConcat(basePath, fileName, &filePath);
    }
    /* MODE = write in binary format and erase if existing */
    if (SOPC_STATUS_OK == status)
    {
        fp = fopen(filePath, "wb+");
        if (NULL == fp)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        size_t nb_written = fwrite(buf->p, 1, buf->len, fp);
        if (buf->len != nb_written)
        {
            int err = remove(filePath);
            if (0 != err)
            {
                fprintf(stderr, "> KeyManager: removing partially written DER file '%s' failed.\n", filePath);
            }
            status = SOPC_STATUS_NOK;
        }
    }
    /* Close and clear */
    if (NULL != fp)
    {
        fclose(fp);
    }
    SOPC_Free(basePath);
    SOPC_Free(filePath);
    SOPC_Free(fileName);
    SOPC_Free(thumbprint);

    return status;
}

#if SOPC_HAS_FILESYSTEM
SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER_Files(SOPC_CertificateList* pCerts, const char* directoryPath)
{
    if (NULL == pCerts)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == directoryPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    mbedtls_x509_crt* crt = &pCerts->crt;
    while (crt != NULL && SOPC_STATUS_OK == status)
    {
        status = raw_buf_to_der_file(&crt->raw, directoryPath);
        crt = crt->next;
    }
    return status;
}
#else
SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER_Files(SOPC_CertificateList* pCerts, const char* directoryPath)
{
    SOPC_UNUSED_ARG(pCerts);
    SOPC_UNUSED_ARG(directoryPath);
    return SOPC_STATUS_NOT_SUPPORTED;
}
#endif /* SOPC_HAS_FILESYSTEM */

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_FindCertInList(const SOPC_CertificateList* pList,
                                                                 const SOPC_CertificateList* pCert,
                                                                 bool* pbMatch)
{
    if (NULL == pbMatch)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pbMatch = false;
    if (NULL == pList || NULL == pCert)
    {
        return SOPC_STATUS_OK;
    }

    size_t nCert = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &nCert);
    if (SOPC_STATUS_OK != status && nCert > 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const mbedtls_x509_crt* haystack = &pList->crt;
    const mbedtls_x509_crt* needle = &pCert->crt;
    for (; (!*pbMatch) && NULL != haystack; haystack = haystack->next)
    {
        if (haystack->raw.len == needle->raw.len && 0 == memcmp(haystack->raw.p, needle->raw.p, needle->raw.len))
        {
            *pbMatch = true;
        }
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_IsSelfSigned(const SOPC_CertificateList* pCert, bool* pbIsSelfSigned)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == pCert)
    {
        return SOPC_STATUS_NOK;
    }

    *pbIsSelfSigned = false;
    const mbedtls_x509_crt* crt = &pCert->crt;
    /* Verify that the CA is self sign */
    int res = memcmp(crt->issuer_raw.p, crt->subject_raw.p, crt->issuer_raw.len);
    if (crt->issuer_raw.len == crt->subject_raw.len && 0 == res)
    {
        /* Is it correctly signed? Inspired by x509_crt_check_signature */
        const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(crt->sig_md);
        unsigned char hash[MBEDTLS_MD_MAX_SIZE];

        /* First hash the certificate, then verify it is signed */
        res = mbedtls_md(md_info, crt->tbs.p, crt->tbs.len, hash);
        if (0 == res)
        {
            mbedtls_pk_context crt_pk = crt->pk;
            res = mbedtls_pk_verify_ext(crt->sig_pk, crt->sig_opts, &crt_pk, crt->sig_md, hash,
                                        mbedtls_md_get_size(md_info), crt->sig.p, crt->sig.len);
            if (0 == res)
            {
                /* Finally the certificate is self signed */
                *pbIsSelfSigned = true;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_Copy(const SOPC_CertificateList* pCert, SOPC_CertificateList** ppCertCopy)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == pCert && NULL == ppCertCopy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const mbedtls_x509_crt* crt = &pCert->crt;
    while (NULL != crt && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(crt->raw.p, (uint32_t) crt->raw.len, ppCertCopy);
        crt = crt->next;
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(*ppCertCopy);
        *ppCertCopy = NULL;
    }
    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Certificate Revocation List API
 * ------------------------------------------------------------------------------------------------
 */

/* Create a certificate if \p ppCert points to NULL, do nothing otherwise */
static SOPC_ReturnStatus crl_maybe_create(SOPC_CRLList** ppCRL)
{
    if (NULL == ppCRL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CRLList* crl = *ppCRL;
    if (NULL == crl)
    {
        crl = SOPC_Calloc(1, sizeof(SOPC_CRLList)); /* Also init certificate */
    }
    if (NULL == crl)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    *ppCRL = crl;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                         uint32_t lenDER,
                                                         SOPC_CRLList** ppCRL)
{
    if (NULL == bufferDER || 0 == lenDER || NULL == ppCRL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = crl_maybe_create(ppCRL);
    SOPC_CRLList* pCRL = *ppCRL;

    if (SOPC_STATUS_OK == status)
    {
        int err = mbedtls_x509_crl_parse(&pCRL->crl, bufferDER, lenDER);
        if (0 != err)
        {
            status = SOPC_STATUS_NOK;
            fprintf(stderr, "> KeyManager: crl buffer parse failed with error code: -0x%X\n", (unsigned int) -err);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pCRL);
        *ppCRL = NULL;
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromFile(const char* szPath, SOPC_CRLList** ppCRL)
{
    if (NULL == szPath || NULL == ppCRL)
    {
        fprintf(stderr, "> KeyManager: crl file \"%s\" parse failed: misses the trailing '\n'", szPath);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

#if defined(MBEDTLS_FS_IO)
    SOPC_ReturnStatus status = crl_maybe_create(ppCRL);
    SOPC_CRLList* pCRL = *ppCRL;

    if (SOPC_STATUS_OK == status)
    {
        int err = mbedtls_x509_crl_parse_file(&pCRL->crl, szPath);
        if (0 != err)
        {
            status = SOPC_STATUS_NOK;
            fprintf(stderr, "> KeyManager: crl file \"%s\" parse failed with error code: -0x%X", szPath,
                    (unsigned int) -err);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pCRL);
        *ppCRL = NULL;
    }

    return status;
#else
    return SOPC_STATUS_NOK;
#endif
}

#if SOPC_HAS_FILESYSTEM
SOPC_ReturnStatus SOPC_KeyManager_CRL_ToDER_Files(SOPC_CRLList* pCrls, const char* directoryPath)
{
    if (NULL == pCrls)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == directoryPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    mbedtls_x509_crl* crl = &pCrls->crl;
    while (crl != NULL && SOPC_STATUS_OK == status)
    {
        status = raw_buf_to_der_file(&crl->raw, directoryPath);
        crl = crl->next;
    }
    return status;
}
#else
SOPC_ReturnStatus SOPC_KeyManager_CRL_ToDER_Files(SOPC_CRLList* pCrls, const char* directoryPath)
{
    SOPC_UNUSED_ARG(pCrls);
    SOPC_UNUSED_ARG(directoryPath);
    return SOPC_STATUS_NOT_SUPPORTED;
}
#endif /* SOPC_HAS_FILESYSTEM */

SOPC_ReturnStatus SOPC_KeyManager_CRL_Copy(const SOPC_CRLList* pCrl, SOPC_CRLList** ppCrlCopy)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == pCrl && NULL == ppCrlCopy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const mbedtls_x509_crl* crl = &pCrl->crl;
    while (NULL != crl && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(crl->raw.p, (uint32_t) crl->raw.len, ppCrlCopy);
        crl = crl->next;
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(*ppCrlCopy);
        *ppCrlCopy = NULL;
    }
    return status;
}

void SOPC_KeyManager_CRL_Free(SOPC_CRLList* pCRL)
{
    if (NULL == pCRL)
        return;

    /* Frees all the crls in the chain */
    mbedtls_x509_crl_free(&pCRL->crl);
    SOPC_Free(pCRL);
}
