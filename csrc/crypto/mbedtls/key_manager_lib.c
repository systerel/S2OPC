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

#include <assert.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "key_manager_lib.h"

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
                        : mbedtls_pk_parse_key(&key->pk, null_terminated_buffer, 1 + lenBuf, NULL, 0);
        SOPC_Free(null_terminated_buffer);
    }

    if (res != 0)
    {
        res = is_public ? mbedtls_pk_parse_public_key(&key->pk, buffer, lenBuf)
                        : mbedtls_pk_parse_key(&key->pk, buffer, lenBuf, NULL, 0);
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
    SOPC_AsymmetricKey* key = NULL;

    if (NULL == szPath || NULL == ppKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check password
    if (NULL == password && 0 != lenPassword)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != password && (0 == lenPassword || '\0' != password[lenPassword]))
        return SOPC_STATUS_INVALID_PARAMETERS;

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
 *          This function does not allocate the buffer containing the DER. The operation may fail if the allocated
 * buffer is not large enough. The required length cannot be precisely calculated, but a value of 8 times the key length
 * in bytes is recommended.
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

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateFromDER(const uint8_t* bufferDER,
                                                            uint32_t lenDER,
                                                            SOPC_CertificateList** ppCert)
{
    mbedtls_x509_crt* crt = NULL;
    SOPC_CertificateList* certif = NULL;

    if (NULL == bufferDER || 0 == lenDER || NULL == ppCert)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Mem alloc
    certif = SOPC_Malloc(sizeof(SOPC_CertificateList));
    if (NULL == certif)
        return SOPC_STATUS_NOK;

    // Init
    crt = &certif->crt;
    mbedtls_x509_crt_init(crt);

    // Parsing
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int status_code = mbedtls_x509_crt_parse(crt, bufferDER, lenDER);
    if (status_code == 0 && crt->raw.len <= UINT32_MAX)
    {
        *ppCert = certif;
    }
    else
    {
        status = SOPC_STATUS_NOK;
        SOPC_Logger_TraceError("Crypto: certificate buffer parse failed with error code: %i", status_code);
        SOPC_KeyManager_Certificate_Free(certif);
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 * \note    Same as CreateFromDER, except for a single call, can we refactor?
 *
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateFromFile(const char* szPath, SOPC_CertificateList** ppCert)
{
    mbedtls_x509_crt* crt = NULL;
    SOPC_CertificateList* certif = NULL;

    if (NULL == szPath || NULL == ppCert)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Mem alloc
    certif = SOPC_Malloc(sizeof(SOPC_CertificateList));
    if (NULL == certif)
        return SOPC_STATUS_NOK;

    // Init
    crt = &certif->crt;
    mbedtls_x509_crt_init(crt);

    // Parsing
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int status_code = mbedtls_x509_crt_parse_file(crt, szPath);
    if (status_code == 0 && crt->raw.len <= UINT32_MAX)
    {
        *ppCert = certif;
    }
    else
    {
        status = SOPC_STATUS_NOK;
        SOPC_Logger_TraceError("Crypto: certificate file parse failed with error code: %i", status_code);
        SOPC_KeyManager_Certificate_Free(certif);
    }

    return status;
}

void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert)
{
    if (NULL == pCert)
        return;

    mbedtls_x509_crt_free(&pCert->crt);
    SOPC_Free(pCert);
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER(const SOPC_CertificateList* pCert,
                                                    uint8_t** ppDest,
                                                    uint32_t* pLenAllocated)
{
    uint32_t lenToAllocate = 0;

    if (NULL == pCert || NULL == ppDest || 0 == pLenAllocated)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Assert single certificate */
    size_t nCert = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &nCert);
    if (SOPC_STATUS_OK == status && 1 != nCert)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

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

    /* Assert single certificate */
    size_t nCert = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &nCert);
    if (SOPC_STATUS_OK == status && 1 != nCert)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

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
    assert(p >= start);
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
    assert(offset < mem_len);

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
    assert(pCert != NULL);
    assert(application_uri != NULL);
    size_t nCert = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &nCert);
    assert(SOPC_STATUS_OK == status && 1 == nCert);

    uint8_t str_len = 0;
    const void* str_data = get_application_uri_ptr_from_crt_data(pCert, &str_len);

    if (NULL == str_data)
    {
        return false;
    }

    if (strlen(application_uri) != str_len)
    {
        return false;
    }

    return strncmp(application_uri, str_data, str_len) == 0;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_CertificateList* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength)
{
    assert(NULL != pCert);
    assert(NULL != ppApplicationUri);
    size_t nCert = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &nCert);
    assert(SOPC_STATUS_OK == status && 1 == nCert);

    uint8_t str_len = 0;
    const void* str_data = get_application_uri_ptr_from_crt_data(pCert, &str_len);

    if (NULL == str_data)
    {
        return SOPC_STATUS_NOK;
    }

    char* data_copy = SOPC_Calloc(str_len + 1U, sizeof(char));

    if (NULL == data_copy)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    memcpy(data_copy, str_data, str_len);
    *ppApplicationUri = data_copy;
    if (NULL != pStringLength)
    {
        *pStringLength = str_len;
    }

    return SOPC_STATUS_OK;
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
