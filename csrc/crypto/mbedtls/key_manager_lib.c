/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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

#include <stdlib.h>
#include <string.h>

#include "key_manager_lib.h"

#include "../sopc_crypto_profiles.h"
#include "../sopc_crypto_provider.h"
#include "../sopc_key_manager.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509.h"

/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * Creates an asymmetric key from a \p buffer, in DER or PEM format.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(const uint8_t* buffer,
                                                                 uint32_t lenBuf,
                                                                 SOPC_AsymmetricKey** ppKey)
{
    SOPC_AsymmetricKey* key = NULL;

    if (NULL == buffer || 0 == lenBuf || NULL == ppKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    key = malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == key)
        return SOPC_STATUS_NOK;
    key->isBorrowedFromCert = false;
    mbedtls_pk_init(&key->pk);

    if (mbedtls_pk_parse_key(&key->pk, buffer, lenBuf, NULL, 0) != 0) // This should also parse PEM keys.
    {
        free(key);
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
    SOPC_AsymmetricKey* key = NULL;

    if (NULL == szPath || NULL == ppKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check password
    if (NULL == password && 0 != lenPassword)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != password && (0 == lenPassword || '\0' != password[lenPassword]))
        return SOPC_STATUS_INVALID_PARAMETERS;

    key = malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == key)
        return SOPC_STATUS_NOK;
    key->isBorrowedFromCert = false;
    mbedtls_pk_init(&key->pk);

    if (mbedtls_pk_parse_keyfile(&key->pk, szPath, password) != 0)
    {
        free(key);
        return SOPC_STATUS_NOK;
    }

    *ppKey = key;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_Certificate* pCert,
                                                                      SOPC_AsymmetricKey** pKey)
{
    if (NULL == pCert || NULL == pKey)
        return SOPC_STATUS_INVALID_PARAMETERS;
    *pKey = malloc(sizeof(SOPC_AsymmetricKey));
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
    free(pKey);
}

/**
 * \brief   Creates a DER from the AsymmetricKey \p pKey and copies it to \p pDest.
 *
 *          This function does not allocate the buffer containing the DER. The operation may fail if the allocated
 * buffer is not large enough. The required length cannot be precisely calculated, but a value of 8 times the key length
 * in bytes is recommended.
 */
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToDER(const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pDest,
                                                      uint32_t lenDest,
                                                      uint32_t* pLenWritten)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint8_t* buffer = NULL;
    int lengthWritten = 0;

    if (NULL == pKey || NULL == pDest || 0 == lenDest || NULL == pLenWritten)
        return SOPC_STATUS_INVALID_PARAMETERS;

    buffer = malloc(lenDest);
    if (NULL == buffer)
        return SOPC_STATUS_NOK;
    // Asymmetric key should be const in mbedtls
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    lengthWritten = mbedtls_pk_write_key_der(&((SOPC_AsymmetricKey*) pKey)->pk, buffer, lenDest);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    if (lengthWritten > 0 && (uint32_t) lengthWritten <= lenDest)
    {
        *pLenWritten = (uint32_t) lengthWritten;
        memcpy(pDest, buffer + lenDest - *pLenWritten, *pLenWritten);
        status = SOPC_STATUS_OK;
    }

    free(buffer);

    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateFromDER(const uint8_t* bufferDER,
                                                            uint32_t lenDER,
                                                            SOPC_Certificate** ppCert)
{
    mbedtls_x509_crt* crt = NULL;
    SOPC_Certificate* certif = NULL;

    if (NULL == bufferDER || 0 == lenDER || NULL == ppCert)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Mem alloc
    certif = malloc(sizeof(SOPC_Certificate));
    if (NULL == certif)
        return SOPC_STATUS_NOK;

    // Init
    crt = &certif->crt;
    mbedtls_x509_crt_init(crt);
    certif->crt_der = NULL;
    certif->len_der = 0;

    // Parsing
    if (mbedtls_x509_crt_parse(crt, bufferDER, lenDER) == 0)
    {
        certif->crt_der = certif->crt.raw.p;
        if (certif->crt.raw.len > UINT32_MAX)
            return SOPC_STATUS_NOK;
        certif->len_der = (uint32_t) certif->crt.raw.len;
        *ppCert = certif;
        return SOPC_STATUS_OK;
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(certif);
        return SOPC_STATUS_NOK;
    }
}

/**
 * \note    Tested but not part of the test suites.
 * \note    Same as CreateFromDER, except for a single call, can we refactor?
 *
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateFromFile(const char* szPath, SOPC_Certificate** ppCert)
{
    mbedtls_x509_crt* crt = NULL;
    SOPC_Certificate* certif = NULL;

    if (NULL == szPath || NULL == ppCert)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Mem alloc
    certif = malloc(sizeof(SOPC_Certificate));
    if (NULL == certif)
        return SOPC_STATUS_NOK;

    // Init
    crt = &certif->crt;
    mbedtls_x509_crt_init(crt);
    certif->crt_der = NULL;
    certif->len_der = 0;

    // Parsing
    if (mbedtls_x509_crt_parse_file(crt, szPath) == 0)
    {
        certif->crt_der = certif->crt.raw.p;
        if (certif->crt.raw.len > UINT32_MAX)
            return SOPC_STATUS_NOK;
        certif->len_der = (uint32_t) certif->crt.raw.len;
        *ppCert = certif;
        return SOPC_STATUS_OK;
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(certif);
        return SOPC_STATUS_NOK;
    }
}

void SOPC_KeyManager_Certificate_Free(SOPC_Certificate* pCert)
{
    if (NULL == pCert)
        return;

    mbedtls_x509_crt_free(&pCert->crt);
    pCert->crt_der = NULL;
    pCert->len_der = 0;
    free(pCert);
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_Certificate* pCert,
                                                            uint8_t* pDest,
                                                            uint32_t lenDest)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t lenSupposed = 0;
    uint8_t* pDER = NULL;
    uint32_t lenDER = 0;
    mbedtls_md_type_t type = MBEDTLS_MD_NONE;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pCert || NULL == pDest || 0 == lenDest)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_CertificateGetLength_Thumbprint(pProvider, &lenSupposed) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    if (lenDest != lenSupposed)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Get DER
    if (SOPC_KeyManager_Certificate_CopyDER(pCert, &pDER, &lenDER) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    // Hash DER with SHA-1
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    default:
        status = SOPC_STATUS_NOK;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
    case SOPC_SecurityPolicy_Basic256_ID:
        type = MBEDTLS_MD_SHA1;
        break;
    }

    if (SOPC_STATUS_OK == status)
    {
        const mbedtls_md_info_t* pmd = mbedtls_md_info_from_type(type);
        if (mbedtls_md(pmd, pDER, lenDER, pDest) != 0)
            status = SOPC_STATUS_NOK;
    }

    free(pDER);

    return status;
}

/**
 * \brief       Fills \p pKey with public key information retrieved from \p pCert.
 * \warning     \p pKey is not valid anymore when \p pCert is freed.
 */
SOPC_ReturnStatus KeyManager_Certificate_GetPublicKey(const SOPC_Certificate* pCert, SOPC_AsymmetricKey* pKey)
{
    if (NULL == pCert || NULL == pKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    memcpy(&pKey->pk, &pCert->crt.pk, sizeof(mbedtls_pk_context));

    return SOPC_STATUS_OK;
}
