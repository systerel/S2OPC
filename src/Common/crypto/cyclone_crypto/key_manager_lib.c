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
#include <stdio.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "key_manager_lib.h"

// TODO: the right cyclone_crypto includes here

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
    SOPC_UNUSED_ARG(buffer);
    SOPC_UNUSED_ARG(lenBuf);
    SOPC_UNUSED_ARG(is_public);
    SOPC_UNUSED_ARG(ppKey);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

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
    SOPC_UNUSED_ARG(szPath);
    SOPC_UNUSED_ARG(ppKey);
    SOPC_UNUSED_ARG(password);
    SOPC_UNUSED_ARG(lenPassword);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_CertificateList* pCert,
                                                                      SOPC_AsymmetricKey** pKey)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pKey);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/**
 * Frees an asymmetric key created with KeyManager_AsymmetricKey_Create*().
 *
 * \warning     Only keys created with KeyManager_AsymmetricKey_Create*() should be freed that way.
 */
void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey)
{
    SOPC_UNUSED_ARG(pKey);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");
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
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(is_public);
    SOPC_UNUSED_ARG(pDest);
    SOPC_UNUSED_ARG(lenDest);
    SOPC_UNUSED_ARG(pLenWritten);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(const SOPC_AsymmetricKey* pKey,
                                                                        bool is_public,
                                                                        SOPC_SerializedAsymmetricKey** out)
{
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(is_public);
    SOPC_UNUSED_ARG(out);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

/* Create a certificate if \p ppCert points to NULL, do nothing otherwise */
/* static SOPC_ReturnStatus certificate_maybe_create(SOPC_CertificateList** ppCert)
{
    SOPC_UNUSED_ARGppCert;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
} */
/* Defined but not used */

/* Check lengths of loaded certificates */
/* static SOPC_ReturnStatus certificate_check_length(SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARGpCert;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
} */
/* Defined but not used */

SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                                 uint32_t lenDER,
                                                                 SOPC_CertificateList** ppCert)
{
    SOPC_UNUSED_ARG(bufferDER);
    SOPC_UNUSED_ARG(lenDER);
    SOPC_UNUSED_ARG(ppCert);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/**
 * \note    Tested but not part of the test suites.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromFile(const char* szPath, SOPC_CertificateList** ppCert)
{
    SOPC_UNUSED_ARG(szPath);
    SOPC_UNUSED_ARG(ppCert);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pCert);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");
}

/* static SOPC_ReturnStatus certificate_check_single(const SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARGpCert;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
} */
/* Defined but not used */

SOPC_ReturnStatus SOPC_KeyManager_Certificate_ToDER(const SOPC_CertificateList* pCert,
                                                    uint8_t** ppDest,
                                                    uint32_t* pLenAllocated)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppDest);
    SOPC_UNUSED_ARG(pLenAllocated);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_CertificateList* pCert,
                                                            uint8_t* pDest,
                                                            uint32_t lenDest)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pDest);
    SOPC_UNUSED_ARG(lenDest);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/**
 * \brief       Fills \p pKey with public key information retrieved from \p pCert.
 * \warning     \p pKey is not valid anymore when \p pCert is freed.
 */
SOPC_ReturnStatus KeyManager_Certificate_GetPublicKey(const SOPC_CertificateList* pCert, SOPC_AsymmetricKey* pKey)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pKey);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/* static size_t ptr_offset(const void* p, const void* start)
{
    SOPC_UNUSED_ARGp;
    SOPC_UNUSED_ARGstart;
    size_t ret = 0;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return ret;
} */
/* Defined but not used */

/* static void* mem_search(const void* mem, size_t mem_len, const void* needle, size_t needle_len)
{
    SOPC_UNUSED_ARGmem;
    SOPC_UNUSED_ARGmem_len;
    SOPC_UNUSED_ARGneedle;
    SOPC_UNUSED_ARGneedle_len;
    void* ret = NULL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return ret;
} */
/* Defined but not used */

/* Does not copy the string. Returns NULL if nothing found. In this case str_len might have been modified. */
/* static const uint8_t* get_application_uri_ptr_from_crt_data(const SOPC_CertificateList* crt, uint8_t* str_len)
{
    SOPC_UNUSED_ARGcrt;
    SOPC_UNUSED_ARGstr_len;
    uint8_t* ret = NULL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return ret;
} */
/* Defined but not used */

bool SOPC_KeyManager_Certificate_CheckApplicationUri(const SOPC_CertificateList* pCert, const char* application_uri)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(application_uri);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return true;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_CertificateList* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(ppApplicationUri);
    SOPC_UNUSED_ARG(pStringLength);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetListLength(const SOPC_CertificateList* pCert, size_t* pLength)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pLength);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/* Creates a new string: free the result */
/* static char* get_raw_sha1(const int raw)
{
    char* ret = NULL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return ret;
} */
/* Defined but not used */

/* Creates a new string: free the result */
/* static char* get_crt_sha1(const int crt)
{
    char* ret = NULL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return ret;
} */
/* Defined but not used */

char* SOPC_KeyManager_Certificate_GetCstring_SHA1(SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pCert);
    char* p = NULL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return p;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(SOPC_CertificateList* pCert,
                                                                     const SOPC_CRLList* pCRL,
                                                                     bool* pbMatch)
{
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pCRL);
    SOPC_UNUSED_ARG(pbMatch);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_FindCertInList(const SOPC_CertificateList* pList,
                                                                 const SOPC_CertificateList* pCert,
                                                                 bool* pbMatch)
{
    SOPC_UNUSED_ARG(pList);
    SOPC_UNUSED_ARG(pCert);
    SOPC_UNUSED_ARG(pbMatch);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/* ------------------------------------------------------------------------------------------------
 * Certificate Revocation List API
 * ------------------------------------------------------------------------------------------------
 */
/* static SOPC_ReturnStatus crl_maybe_create(SOPC_CRLList** ppCRL)
{
    SOPC_UNUSED_ARGppCRL;

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
} */
/* Defined but not used */

SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                         uint32_t lenDER,
                                                         SOPC_CRLList** ppCRL)
{
    SOPC_UNUSED_ARG(bufferDER);
    SOPC_UNUSED_ARG(lenDER);
    SOPC_UNUSED_ARG(ppCRL);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

/**
 * \note    Tested but not part of the test suites.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromFile(const char* szPath, SOPC_CRLList** ppCRL)
{
    SOPC_UNUSED_ARG(szPath);
    SOPC_UNUSED_ARG(ppCRL);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

void SOPC_KeyManager_CRL_Free(SOPC_CRLList* pCRL)
{
    SOPC_UNUSED_ARG(pCRL);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");
}
