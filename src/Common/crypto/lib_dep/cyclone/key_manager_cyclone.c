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
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_helper_encode.h"
#include "sopc_helper_string.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_random.h"

#include "key_manager_cyclone.h"

#include "core/crypto.h"
#include "pkc/rsa.h"
#include "pkix/pem_decrypt.h"
#include "pkix/pem_export.h"
#include "pkix/pem_import.h"
#include "pkix/pem_key_export.h"
#include "pkix/pem_key_import.h"
#include "pkix/pkcs8_key_parse.h"
#include "pkix/x509_cert_parse.h"
#include "pkix/x509_cert_validate.h"
#include "pkix/x509_crl_parse.h"
#include "pkix/x509_csr_create.h"
#include "pkix/x509_key_format.h"
#include "pkix/x509_key_parse.h"
#include "pkix/x509_sign_verify.h"
#include "rng/yarrow.h"

#ifndef X509_MAX_SUBJECT_ALT_NAMES
#define X509_MAX_SUBJECT_ALT_NAMES 4u
#endif

#define SOPC_RSA_EXPONENT 65537u
#define SOPC_KEY_MANAGER_SHA1_SIZE 20
#define SOPC_CSR_DER_MAX_SIZE 8192u

YarrowContext yarrowContext;

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
    if (NULL == buffer || 0 == lenBuf || NULL == ppKey)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_AsymmetricKey* key = NULL;
    key = SOPC_Malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == key)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    key->isBorrowedFromCert = false;
    rsaInitPublicKey(&key->pubKey);
    rsaInitPrivateKey(&key->privKey);

    error_t errLib = 1;

    if (is_public)
    {
        errLib = pemImportRsaPublicKey(&key->pubKey, (const char_t*) buffer, (size_t) lenBuf);

        if (0 != errLib) // The buffer is probably in DER format...
        {
            // We will then directly parse it.
            // Corresponds to the case "PUBLIC KEY" when calling the function pemImportRsaPublicKey.
            size_t lenParsed = 0;
            X509SubjectPublicKeyInfo publicKeyInfo = {0};
            errLib = x509ParseSubjectPublicKeyInfo(buffer, lenBuf, &lenParsed, &publicKeyInfo);

            // If no error, continue...
            if (0 == errLib)
            {
                errLib = x509ImportRsaPublicKey(&key->pubKey, &publicKeyInfo);
            }
        }
    }
    else // Key is private
    {
        errLib = pemImportRsaPrivateKey(&key->privKey, (const char_t*) buffer, (size_t) lenBuf, NULL);

        if (0 != errLib) // The buffer is probably in DER format...
        {
            // We will then directly parse it.
            // Corresponds to the case "RSA PRIVATE KEY" when calling the function pemImportRsaPrivateKey.
            Pkcs8PrivateKeyInfo privateKeyInfo = {0};
            errLib = pkcs8ParseRsaPrivateKey(buffer, lenBuf, &privateKeyInfo.rsaPrivateKey);

            // If no error, continue...
            if (0 == errLib)
            {
                privateKeyInfo.oid.value = RSA_ENCRYPTION_OID;
                privateKeyInfo.oid.length = sizeof(RSA_ENCRYPTION_OID);
                errLib = pkcs8ImportRsaPrivateKey(&key->privKey, &privateKeyInfo);
            }

            // Clear private key info
            memset(&privateKeyInfo, 0, sizeof(Pkcs8PrivateKeyInfo));
        }

        /* If no error, fill the public part */
        if (0 == errLib)
        {
            errLib = rsaGeneratePublicKey(&key->privKey, &key->pubKey);
        }
    }

    if (0 != errLib)
    {
        SOPC_KeyManager_AsymmetricKey_Free(key);
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
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check password
    if (NULL == password && 0 != lenPassword)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL != password && (0 == lenPassword || '\0' != password[lenPassword]))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SecretBuffer* pSecretBuffer = SOPC_SecretBuffer_NewFromFile(szPath);
    const SOPC_ExposedBuffer* pBuffer = SOPC_SecretBuffer_Expose(pSecretBuffer);
    uint32_t length = SOPC_SecretBuffer_GetLength(pSecretBuffer);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL ==
        password) // not-encrypted private key, we can directly call the function AsymmetriKey_CreateFromBuffer()
    {
        status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(pBuffer, length, false, ppKey);
    }
    else
    {
        char_t* buffer_decrypted =
            SOPC_Malloc((2 * length) * sizeof(char_t)); // Length of the decrypted buffer, 2*size should be enough.
        size_t lenBuffer_decrypted;
        error_t errLib =
            pemDecryptPrivateKey((const char_t*) pBuffer, length, password, buffer_decrypted, &lenBuffer_decrypted);

        if (0 == errLib)
        {
            status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer((uint8_t*) buffer_decrypted,
                                                                    (uint32_t) lenBuffer_decrypted, false, ppKey);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }

        SOPC_Free(buffer_decrypted);
    }

    SOPC_SecretBuffer_Unexpose(pBuffer, pSecretBuffer);
    SOPC_SecretBuffer_DeleteClear(pSecretBuffer);
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_GenRSA(uint32_t RSAKeySize, SOPC_AsymmetricKey** ppKey)
{
    if (NULL == ppKey || 0 == RSAKeySize)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppKey = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* seed = NULL;

    SOPC_AsymmetricKey* pKey = SOPC_Calloc(1, sizeof(SOPC_AsymmetricKey));
    if (NULL == pKey)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    pKey->isBorrowedFromCert = false;
    rsaInitPrivateKey(&pKey->privKey);
    rsaInitPublicKey(&pKey->pubKey);

    error_t err = yarrowInit(&yarrowContext);
    if (NO_ERROR == err)
    {
        seed = SOPC_Buffer_Create(32);
        status = SOPC_GetRandom(seed, 32);
    }
    if (SOPC_STATUS_OK == status)
    {
        // SOPC_GetRandom returned good status : seed != NULL
        SOPC_ASSERT(NULL != seed);
        err = yarrowSeed(&yarrowContext, seed->data, seed->length);
    }
    if (NO_ERROR == err)
    {
        err = rsaGenerateKeyPair(YARROW_PRNG_ALGO, &yarrowContext, (size_t) RSAKeySize, SOPC_RSA_EXPONENT,
                                 &pKey->privKey, &pKey->pubKey);
    }

    if (NO_ERROR != err || SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_AsymmetricKey_Free(pKey);
        status = SOPC_STATUS_NOK;
    }
    yarrowDeinit(&yarrowContext);
    memset(seed->data, 0, seed->length);
    SOPC_Buffer_Delete(seed);

    *ppKey = pKey;
    return (status == SOPC_STATUS_OK) ? status : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(const SOPC_CertificateList* pCert,
                                                                      SOPC_AsymmetricKey** pKey)
{
    if (NULL == pCert || NULL == pKey)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pKey = SOPC_Malloc(sizeof(SOPC_AsymmetricKey));
    if (NULL == *pKey)
    {
        return SOPC_STATUS_NOK;
    }

    (*pKey)->isBorrowedFromCert = true;

    rsaInitPublicKey(&(*pKey)->pubKey);
    rsaInitPrivateKey(&(*pKey)->privKey);

    return SOPC_KeyManagerInternal_Certificate_GetPublicKey(pCert, *pKey);
}

/**
 * Frees an asymmetric key created with KeyManager_AsymmetricKey_Create*().
 *
 * \warning     Only keys created with KeyManager_AsymmetricKey_Create*() should be freed that way.
 */
void SOPC_KeyManager_AsymmetricKey_Free(SOPC_AsymmetricKey* pKey)
{
    if (NULL == pKey)
    {
        return;
    }

    if (false == pKey->isBorrowedFromCert)
    {
        rsaFreePublicKey(&pKey->pubKey);
        rsaFreePrivateKey(&pKey->privKey);
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

    if (NULL == pKey || NULL == pDest || 0 == lenDest || NULL == pLenWritten)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Declare and alloc the buffer which will receive the der data
    uint8_t* buffer = NULL;
    size_t lengthWritten = 0;
    buffer = SOPC_Malloc(lenDest);
    if (NULL == buffer)
    {
        return SOPC_STATUS_NOK;
    }

    error_t errLib = 1;

    if (is_public)
    {
        // The function x509ExportRsaPublicKey does not give the full DER,
        // so we first export the key to PEM, then we do PEM to DER.
        char_t* bufferPEM = SOPC_Malloc((2 * lenDest) * sizeof(char_t));
        size_t writtenPEM;
        errLib = pemExportRsaPublicKey(&pKey->pubKey, bufferPEM, &writtenPEM, PEM_PUBLIC_KEY_FORMAT_DEFAULT);
        if (0 == errLib)
        {
            errLib = pemDecodeFile(bufferPEM, writtenPEM, "PUBLIC KEY", buffer, &lengthWritten, NULL, NULL);
        }

        SOPC_Free(bufferPEM);
    }
    else // Key is private
    {
        errLib = x509ExportRsaPrivateKey(&pKey->privKey, buffer, &lengthWritten);
    }

    if (0 == errLib)
    {
        if (lengthWritten > 0 && (uint32_t) lengthWritten <= lenDest)
        {
            *pLenWritten = (uint32_t) lengthWritten;
            memcpy(pDest, buffer, lengthWritten);
            status = SOPC_STATUS_OK;
        }
    }

    // Clear and free the temporary buffer
    memset(buffer, 0, lengthWritten);
    SOPC_Free(buffer);

    return status;
}

#if SOPC_HAS_FILESYSTEM
static SOPC_ReturnStatus sopc_write_pem_file(const char* filePath, const char* pem, size_t pemLen)
{
    if (NULL == filePath || NULL == pem || 0 == pemLen)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    FILE* fp = fopen(filePath, "wb");
    if (NULL == fp)
    {
        return SOPC_STATUS_NOK;
    }

    size_t nbWritten = fwrite(pem, 1, pemLen, fp);
    fclose(fp);

    if (nbWritten != pemLen)
    {
        int err = remove(filePath);
        if (0 != err)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "KeyManager: removing partially written PEM file '%s' failed.", filePath);
        }
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
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

    /* The Cyclone library does not support encryption for PEM private keys. */
    if (NULL != pwd)
    {
        return SOPC_STATUS_NOK;
    }

    error_t errLib = NO_ERROR;
    size_t pemLen = 0;
    char_t* pem = NULL;

    if (bIsPublic)
    {
        errLib = pemExportRsaPublicKey(&pKey->pubKey, NULL, &pemLen, PEM_PUBLIC_KEY_FORMAT_RFC7468);
    }
    else
    {
        errLib = pemExportRsaPrivateKey(&pKey->privKey, NULL, &pemLen, PEM_PRIVATE_KEY_FORMAT_PKCS1);
    }

    if (NO_ERROR != errLib || 0 == pemLen)
    {
        return SOPC_STATUS_NOK;
    }

    pem = SOPC_Calloc(pemLen, sizeof(char_t));
    if (NULL == pem)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (bIsPublic)
    {
        errLib = pemExportRsaPublicKey(&pKey->pubKey, pem, &pemLen, PEM_PUBLIC_KEY_FORMAT_RFC7468);
    }
    else
    {
        errLib = pemExportRsaPrivateKey(&pKey->privKey, pem, &pemLen, PEM_PRIVATE_KEY_FORMAT_PKCS1);
    }

    if (NO_ERROR != errLib)
    {
        memset(pem, 0, pemLen);
        SOPC_Free(pem);
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status = sopc_write_pem_file(filePath, pem, pemLen);

    memset(pem, 0, pemLen);
    SOPC_Free(pem);

    return (status == SOPC_STATUS_OK) ? status : SOPC_STATUS_NOK;
}
#else
SOPC_ReturnStatus SOPC_KeyManager_AsymmetricKey_ToPEMFile(SOPC_AsymmetricKey* pKey,
                                                          const bool bIsPublic,
                                                          const char* filePath,
                                                          const char* pwd,
                                                          const uint32_t pwdLen)
{
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(bIsPublic);
    SOPC_UNUSED_ARG(filePath);
    SOPC_UNUSED_ARG(pwd);
    SOPC_UNUSED_ARG(pwdLen);
    return SOPC_STATUS_NOT_SUPPORTED;
}
#endif /* SOPC_HAS_FILESYSTEM */

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromKey(const SOPC_AsymmetricKey* pKey,
                                                                        bool is_public,
                                                                        SOPC_SerializedAsymmetricKey** out)
{
    if (NULL == pKey || NULL == out)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint_t lenBits = mpiGetBitLength(&pKey->pubKey.n);

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

    // Clear and free the temporary buffer
    memset(buffer, 0, pLenWritten);
    SOPC_Free(buffer);

    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                                 uint32_t lenDER,
                                                                 SOPC_CertificateList** ppCert)
{
    if (NULL == bufferDER || 0 == lenDER || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    error_t errLib = 1;

    /* Create the new cert and initialize it */
    SOPC_CertificateList* pCertNew = SOPC_Calloc(1, sizeof(SOPC_CertificateList));
    if (NULL == pCertNew)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pCertNew->next = NULL;
    pCertNew->raw = SOPC_Buffer_Create(lenDER);
    if (NULL == pCertNew->raw)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(pCertNew->raw, bufferDER, lenDER);
    }

    /* Fill the crt part of the cert */
    if (SOPC_STATUS_OK == status)
    {
        // Create the certificate from its attached buffer because Cyclone cert will point on it
        errLib = x509ParseCertificate(pCertNew->raw->data, (size_t) lenDER, &pCertNew->crt);
        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "> KeyManager: certificate buffer parse failed with error code: %d\n", errLib);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Allocate and fill the public key of the cert */
        rsaInitPublicKey(&pCertNew->pubKey);
        errLib = x509ImportRsaPublicKey(&pCertNew->pubKey, &pCertNew->crt.tbsCert.subjectPublicKeyInfo);
        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* In case of error, free the new certificate and the whole list */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCertNew);
        SOPC_KeyManager_Certificate_Free(*ppCert);
        *ppCert = NULL;
    }
    else /* Finally add the cert to the list in ppCert */
    {
        /* Special case when the input list is not created */
        if (NULL == *ppCert)
        {
            *ppCert = pCertNew;
        }
        else
        {
            /* Otherwise, put the new certificate at the end of ppCert */
            SOPC_CertificateList* pCertLast = *ppCert;
            while (NULL != pCertLast->next)
            {
                pCertLast = pCertLast->next;
            }
            pCertLast->next = pCertNew;
        }
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 * \note    The file can be DER, or PEM with only 1 certificate.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromFile(const char* szPath, SOPC_CertificateList** ppCert)
{
    if (NULL == szPath || NULL == ppCert)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "KeyManager: certificate file \"%s\" parse failed: misses the trailing '\n'", szPath);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    error_t errLib = 1;

    SOPC_Buffer* pBuffer = NULL;
    SOPC_ReturnStatus status = SOPC_Buffer_ReadFile(szPath, &pBuffer);

    if (SOPC_STATUS_OK == status)
    {
        // File is PEM or DER ? X.509 certificates are encoded using the "CERTIFICATE" label. This functions returns -1
        // if no label has been found.
        int_t i = pemFindTag((char_t*) pBuffer->data, pBuffer->length, "-----BEGIN ", "CERTIFICATE", "-----");

        /* If the file is in DER format, directly call the certificate creation function */
        if (0 > i)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBuffer->data, pBuffer->length, ppCert);
        }
        else // The file is PEM, decode it first.
        {
            uint8_t* bufferDER = SOPC_Malloc((2 * pBuffer->length) * sizeof(uint8_t));
            size_t len_bufferDER;

            // Convert the PEM to DER
            size_t length_read = 0;
            errLib =
                pemImportCertificate((char_t*) pBuffer->data, pBuffer->length, bufferDER, &len_bufferDER, &length_read);
            if (0 == errLib)
            {
                if (length_read + 1 < pBuffer->length) // + 1 if the data is a null-terminated string
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                           "KeyManager: PEM certificate file \"%s\" probably contains more than 1 "
                                           "certificate. Please use PEM that contains one unique certificate.'\n'",
                                           szPath);
                    status = SOPC_STATUS_NOK;
                }
                else
                {
                    status =
                        SOPC_KeyManager_Certificate_CreateOrAddFromDER(bufferDER, (uint32_t) len_bufferDER, ppCert);
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }

            SOPC_Free(bufferDER);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> KeyManager: certificate file \"%s\" parse failed.\n", szPath);
    }

    SOPC_Buffer_Delete(pBuffer);

    return status;
}

void SOPC_KeyManager_Certificate_Free(SOPC_CertificateList* pCert)
{
    if (NULL == pCert)
    {
        return;
    }

    /* Free all the data in the cert, then free the Certificate. */
    SOPC_Buffer_Delete(pCert->raw);
    rsaFreePublicKey(&pCert->pubKey);
    SOPC_KeyManager_Certificate_Free(pCert->next);
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

    if (NULL == pCert->raw || NULL == ppDest || NULL == pLenAllocated)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = certificate_check_single(pCert);

    /* Allocation */
    if (SOPC_STATUS_OK == status)
    {
        lenToAllocate = pCert->raw->length;
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
        memcpy((void*) (*ppDest), (void*) (pCert->raw->data), lenToAllocate);
        *pLenAllocated = lenToAllocate;
    }

    return status;
}

/**
 * Hashes the DER-encoded certificate with SHA1.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetThumbprint(const SOPC_CryptoProvider* pProvider,
                                                            const SOPC_CertificateList* pCert,
                                                            uint8_t** ppDest,
                                                            uint32_t* pLenDest)
{
    if (NULL == pProvider || NULL == pCert || NULL == ppDest || NULL != *ppDest || NULL == pLenDest)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint8_t* pDest = NULL;
    uint32_t lenAllocated = 0;

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = certificate_check_single(pCert);
    error_t errLib = 1;

    /* Assert allocation length */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_CertificateGetLength_Thumbprint(pProvider, &lenAllocated);
    }
    if (SOPC_STATUS_OK == status)
    {
        pDest = SOPC_Malloc(sizeof(uint8_t) * lenAllocated);
        if (NULL == pDest)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // Get DER
    uint8_t* pDER = NULL;
    uint32_t lenDER = 0;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_ToDER(pCert, &pDER, &lenDER);
    }

    const HashAlgo* hash = NULL;
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
            hash = &sha1HashAlgo;
            break;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        errLib = hash->compute(pDER, (size_t) lenDER, pDest);
        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pDest);
        *ppDest = NULL;
    }
    else
    {
        *ppDest = pDest;
        *pLenDest = lenAllocated;
    }

    SOPC_Free(pDER);

    return status;
}

/**
 * \brief       Fills \p pKey with RSA public key information retrieved from \p pCert.
 * \warning     \p pKey points on the fields of the cert. Free the cert will make the key unusable.
 */
SOPC_ReturnStatus SOPC_KeyManagerInternal_Certificate_GetPublicKey(const SOPC_CertificateList* pCert,
                                                                   SOPC_AsymmetricKey* pKey)
{
    if (NULL == pCert || NULL == pKey)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    memcpy(&pKey->pubKey, &pCert->pubKey, sizeof(RsaPublicKey));

    return SOPC_STATUS_OK;
}

bool SOPC_KeyManager_Certificate_CheckApplicationUri(const SOPC_CertificateList* pCert, const char* application_uri)
{
    SOPC_ASSERT(NULL != pCert);
    SOPC_ASSERT(NULL != application_uri);

    size_t str_len = 0;
    size_t application_uri_len = strlen(application_uri);
    bool bUriFound = false;
    int comparison = -1;

    for (int i = 0; i < X509_MAX_SUBJECT_ALT_NAMES && (false == bUriFound); i++)
    {
        if (X509_GENERAL_NAME_TYPE_URI == pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].type)
        {
            str_len = pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].length;
            if (application_uri_len == str_len)
            {
                comparison = strncmp(application_uri,
                                     pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].value, str_len);
                if (0 == comparison)
                {
                    bUriFound = true;
                }
            }
        }
    }

    return bUriFound;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetMaybeApplicationUri(const SOPC_CertificateList* pCert,
                                                                     char** ppApplicationUri,
                                                                     size_t* pStringLength)
{
    SOPC_ASSERT(NULL != pCert);
    SOPC_ASSERT(NULL != ppApplicationUri);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    size_t str_len = 0;
    char_t* data_copy = NULL;

    for (int i = 0; i < X509_MAX_SUBJECT_ALT_NAMES && (SOPC_STATUS_OK != status); i++)
    {
        if (X509_GENERAL_NAME_TYPE_URI == pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].type)
        {
            str_len = pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].length;
            if (0 != str_len)
            {
                data_copy = SOPC_Calloc(str_len + 1U, sizeof(char_t)); // Must be freed out of the function.
                if (NULL != data_copy)
                {
                    memcpy(data_copy, pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].value, str_len);
                    status = SOPC_STATUS_OK; // If we arrive here, we found an ApplicationURI.
                }
                else
                {
                    str_len = 0;
                }
            }
        }
    }

    *ppApplicationUri = data_copy;
    if (NULL != pStringLength)
    {
        *pStringLength = str_len;
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetListLength(const SOPC_CertificateList* pCert, size_t* pLength)
{
    if (NULL == pCert || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CertificateList* pCertCpy = pCert;
    size_t i = 0;
    for (; NULL != pCertCpy; ++i)
    {
        pCertCpy = pCertCpy->next;
    }

    *pLength = i;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus SOPC_AppendSubjectField(char* buffer,
                                                 uint32_t maxLen,
                                                 uint32_t* pPos,
                                                 const char* prefix,
                                                 const X509String* field)
{
    SOPC_ASSERT(NULL != buffer);
    SOPC_ASSERT(NULL != pPos);
    SOPC_ASSERT(NULL != prefix);
    SOPC_ASSERT(NULL != field);

    if (field->length == 0 || NULL == field->value)
    {
        return SOPC_STATUS_OK;
    }

    int written =
        snprintf(buffer + *pPos, maxLen - *pPos, "%s=%.*s", prefix, (int) field->length, (const char*) field->value);

    if (written < 0 || (uint32_t) written >= (maxLen - *pPos))
    {
        return SOPC_STATUS_NOK;
    }

    *pPos += (uint32_t) written;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetSubjectName(const SOPC_CertificateList* pCert,
                                                             char** ppSubjectName,
                                                             uint32_t* pSubjectNameLen)
{
    SOPC_ASSERT(NULL != pCert);
    SOPC_ASSERT(NULL != ppSubjectName);
    SOPC_ASSERT(NULL != pSubjectNameLen);

    *ppSubjectName = NULL;
    *pSubjectNameLen = 0;

    const X509Name* subject = &pCert->crt.tbsCert.subject;

    const uint32_t maxLen = 1024;
    char* subjectName = SOPC_Calloc(maxLen, sizeof(char));
    if (NULL == subjectName)
    {
        return SOPC_STATUS_NOK;
    }

    uint32_t pos = 0;

    SOPC_ReturnStatus status = SOPC_AppendSubjectField(subjectName, maxLen, &pos, "C", &subject->countryName);
    if (SOPC_STATUS_OK == status)
        status = SOPC_AppendSubjectField(subjectName, maxLen, &pos, ", ST", &subject->stateOrProvinceName);
    if (SOPC_STATUS_OK == status)
        status = SOPC_AppendSubjectField(subjectName, maxLen, &pos, ", L", &subject->localityName);
    if (SOPC_STATUS_OK == status)
        status = SOPC_AppendSubjectField(subjectName, maxLen, &pos, ", O", &subject->organizationName);
    if (SOPC_STATUS_OK == status)
        status = SOPC_AppendSubjectField(subjectName, maxLen, &pos, ", OU", &subject->organizationalUnitName);
    if (SOPC_STATUS_OK == status)
        status = SOPC_AppendSubjectField(subjectName, maxLen, &pos, ", CN", &subject->commonName);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(subjectName);
        return SOPC_STATUS_NOK;
    }

    *ppSubjectName = subjectName;
    *pSubjectNameLen = pos;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_GetSanDnsNames(const SOPC_CertificateList* pCert,
                                                             char*** ppDnsNameArray,
                                                             uint32_t* pArrayLength)
{
    SOPC_ASSERT(NULL != pCert);
    SOPC_ASSERT(NULL != ppDnsNameArray);
    SOPC_ASSERT(NULL != pArrayLength);

    *ppDnsNameArray = NULL;
    *pArrayLength = 0;

    uint32_t arrayLength = 0;
    char** dnsNameArray = NULL;

    // Count the number of alternate DNS entries
    for (int i = 0; i < X509_MAX_SUBJECT_ALT_NAMES; i++)
    {
        if (X509_GENERAL_NAME_TYPE_DNS == pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].type)
        {
            arrayLength++;
        }
    }

    // Check for alternate DNS entries
    if (arrayLength > 0)
    {
        // Create an array to store them
        dnsNameArray = SOPC_Calloc(arrayLength, sizeof(char*));
        if (NULL == dnsNameArray)
        {
            return SOPC_STATUS_OUT_OF_MEMORY;
        }

        // Loop through and fill the array
        arrayLength = 0;
        for (int i = 0; i < X509_MAX_SUBJECT_ALT_NAMES; i++)
        {
            if (X509_GENERAL_NAME_TYPE_DNS == pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].type)
            {
                uint32_t len = (uint32_t) pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].length;
                dnsNameArray[arrayLength] = SOPC_Malloc((size_t) len + 1);
                strncpy(dnsNameArray[arrayLength], pCert->crt.tbsCert.extensions.subjectAltName.generalNames[i].value,
                        len);
                dnsNameArray[arrayLength][len] = '\0';
                arrayLength++;
            }
        }
    }

    *ppDnsNameArray = dnsNameArray;
    *pArrayLength = arrayLength;

    return SOPC_STATUS_OK;
}

/* Creates a new string. Later have to free the result */
static char* get_raw_sha1(SOPC_Buffer* raw)
{
    SOPC_ASSERT(NULL != raw);

    uint8_t pDest[20];

    error_t errLib = sha1Compute(raw->data, raw->length, pDest);

    if (0 != errLib)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Cannot compute thumbprint of certificate, err -0x%X", errLib);
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

/* Get the SHA1 of a CertificateList */
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

    SOPC_Buffer* raw = pCert->raw;
    sha_1_cert = get_raw_sha1(raw);
    return sha_1_cert;
}

static void sopc_key_manager_remove_cert_from_list(SOPC_CertificateList** ppCur,
                                                   SOPC_CertificateList** ppPrev,
                                                   SOPC_CertificateList** ppHeadCertList)
{
    SOPC_ASSERT(NULL != ppCur);
    SOPC_ASSERT(NULL != *ppCur); /* Current cert shall not be NULL */
    SOPC_ASSERT(NULL != ppPrev);
    SOPC_ASSERT(NULL != ppHeadCertList);
    SOPC_ASSERT(NULL != *ppHeadCertList); /* Head shall not be NULL */

    SOPC_CertificateList* pHeadCertList = *ppHeadCertList;
    SOPC_CertificateList* pCur = *ppCur;      /* Current cert */
    SOPC_CertificateList* pPrev = *ppPrev;    /* Parent of current cert */
    SOPC_CertificateList* pNext = pCur->next; /* Next cert */
    pCur->next = NULL;
    SOPC_Buffer_Delete(pCur->raw);
    rsaFreePublicKey(&pCur->pubKey);
    if (NULL == pPrev)
    {
        if (NULL == pNext)
        {
            /* The list is empty, Free it and stop the iteration  */
            SOPC_Free(pHeadCertList);
            pHeadCertList = NULL;
            pCur = NULL;
        }
        else
        {
            /* Head of the list is a special case */
            *pHeadCertList = *pNext; /* Use an assignment operator to do the copy */
            /* We have to free the new next certificate */
            SOPC_Free(pNext);

            /* Do not iterate: current certificate has changed with the new head (pCur = &pHeadCertList->crt) */
        }
    }
    else
    {
        /* We have to free the certificate if it is not the first in the list */
        SOPC_Free(pCur);
        pPrev->next = pNext;
        /* Iterate */
        pCur = pNext;
    }
    *ppCur = pCur;
    *ppPrev = pPrev;
    *ppHeadCertList = pHeadCertList;
}

/**
 * \brief Checks if the issuer subject name of \p pCa is the same as the CRL \p pCrl .
 *        Checks if the CRL \p pCrl is correctly signed by the CA \p pCa .
 *
 * \param pCrl The CRL.
 * \param pCa The CA.
 * \param[out] pbMatch Defines whether the CRL \p pCrl matches with the CA \p pCa .
 *
 * \return SOPC_STATUS_OK if successful.
 *
 */
static SOPC_ReturnStatus sopc_key_manager_check_crl_ca_match(const SOPC_CRLList* pCrl,
                                                             SOPC_CertificateList* pCa,
                                                             bool* pbMatch)
{
    SOPC_ASSERT(NULL != pCrl);
    SOPC_ASSERT(NULL != pCa);
    SOPC_ASSERT(pCa->crt.tbsCert.extensions.basicConstraints.cA);
    SOPC_ASSERT(NULL != pbMatch);
    bool_t bMatch = false;
    *pbMatch = false;

    /* Compare the subject name if it exists */
    if (pCrl->crl.tbsCertList.issuer.raw.length == pCa->crt.tbsCert.subject.raw.length)
    {
        bMatch = x509CompareName(pCrl->crl.tbsCertList.issuer.raw.value, pCrl->crl.tbsCertList.issuer.raw.length,
                                 pCa->crt.tbsCert.subject.raw.value, pCa->crt.tbsCert.subject.raw.length);
    }

    /* Check if the CRL is correctly signed by the CA */
    if (bMatch)
    {
        error_t errLib = x509VerifySignature(&pCrl->crl.tbsCertList.raw, &pCrl->crl.signatureAlgo,
                                             &pCa->crt.tbsCert.subjectPublicKeyInfo, &pCrl->crl.signatureValue);

        if (0 == errLib)
        {
            *pbMatch = true;
        }
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_CheckCRL(SOPC_CertificateList* pCert,
                                                           const SOPC_CRLList* pCRL,
                                                           bool* bMatch)
{
    if (NULL == pCRL || NULL == pCert || NULL == bMatch)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* For each CA, find its CRL. If not found, log and match = false */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int crl_count = 0;                          /* Number of CRL for the current CA */
    bool crl_match = false;                     /* Defines whether the current CRL matches the current CA. */
    bool list_match = true;                     /* Defines if all CA have exactly one CRL */
    SOPC_CertificateList* current_cert = pCert; /* Current cert */
    while (NULL != current_cert && SOPC_STATUS_OK == status)
    {
        /* Skip certificates that are not authorities */
        if (current_cert->crt.tbsCert.extensions.basicConstraints.cA)
        {
            crl_count = 0;
            crl_match = false;
            const SOPC_CRLList* crl = pCRL;
            while (NULL != crl && SOPC_STATUS_OK == status)
            {
                status = sopc_key_manager_check_crl_ca_match(crl, current_cert, &crl_match);
                if (SOPC_STATUS_OK == status)
                {
                    if (crl_match)
                    {
                        crl_count = crl_count + 1;
                    }
                    /* Iterate */
                    crl = crl->next;
                    crl_match = false;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                if (0 == crl_count)
                {
                    list_match = false;
                    char* fpr = get_raw_sha1(current_cert->raw);
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                             "MatchCRLList: CA Certificate with SHA-1 fingerprint %s has no "
                                             "CRL and will not be considered as valid issuer.",
                                             fpr);
                    SOPC_Free(fpr);
                }
                /* Iterate */
                current_cert = current_cert->next;
            }
        }
        else
        {
            /* Iterate */
            current_cert = current_cert->next;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *bMatch = list_match;
    }
    return status;
}

#if SOPC_HAS_FILESYSTEM
static SOPC_ReturnStatus raw_buf_to_der_file(SOPC_Buffer* buf, const char* directoryPath)
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
        size_t nb_written = fwrite(buf->data, 1, buf->length, fp);
        if (buf->length != nb_written)
        {
            int err = remove(filePath);
            if (0 != err)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                       "KeyManager: removing partially written DER file '%s' failed.", filePath);
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
    SOPC_CertificateList* crt = pCerts;
    while (crt != NULL && SOPC_STATUS_OK == status)
    {
        status = raw_buf_to_der_file(crt->raw, directoryPath);
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

    int comparison = -1;
    SOPC_Buffer* raw = pCert->raw;
    for (; (!*pbMatch) && NULL != pList; pList = pList->next)
    {
        if (pList->raw->length == raw->length)
        {
            comparison = memcmp(pList->raw->data, raw->data, raw->length);
            if (0 == comparison)
            {
                *pbMatch = true;
            }
        }
    }

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus sopc_key_manager_crl_list_remove_crl_from_ca(SOPC_CRLList** ppCRLList,
                                                                      SOPC_CertificateList* ca)
{
    SOPC_ASSERT(NULL != ppCRLList);
    SOPC_ASSERT(NULL != ca);

    SOPC_CRLList* pHeadCRLList = *ppCRLList;
    if (NULL == pHeadCRLList)
    {
        /* the CRL list is empty, do nothing */
        return SOPC_STATUS_OK;
    }
    SOPC_CRLList* cur = pHeadCRLList; /* Current crl */
    SOPC_CRLList* prev = NULL;        /* Parent of current crl */
    SOPC_CRLList* next = NULL;        /* Next crl */
    bool bFound = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Search the crl */
    while (NULL != cur && SOPC_STATUS_OK == status)
    {
        bFound = false;
        status = sopc_key_manager_check_crl_ca_match(cur, ca, &bFound);
        /* Remove the current crl if found */
        if (SOPC_STATUS_OK == status && bFound)
        {
            next = cur->next;
            cur->next = NULL;
            SOPC_Buffer_Delete(cur->raw);
            if (NULL == prev)
            {
                if (NULL == next)
                {
                    /* The list is empty, Free it and stop the iteration  */
                    SOPC_Free(pHeadCRLList);
                    pHeadCRLList = NULL;
                    cur = NULL;
                }
                else
                {
                    /* Head of the list is a special case */
                    *pHeadCRLList = *next; /* Use an assignment operator to do the copy */
                    /* We have to free the new next crl */
                    SOPC_Free(next);
                    /* Do not iterate: current crl has changed with the new head (cur = &pHeadCRLList->crl) */
                }
            }
            else
            {
                /* We have to free the crl if it is not the first in the list */
                SOPC_Free(cur);
                prev->next = next;
                /* Iterate */
                cur = next;
            }
        }
        else
        {
            /* iterate */
            prev = cur;
            cur = cur->next;
        }
    }
    *ppCRLList = pHeadCRLList;
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(SOPC_CertificateList** ppCertList,
                                                                     SOPC_CRLList** ppCRLList,
                                                                     const char* pThumbprint,
                                                                     bool* pbMatch,
                                                                     bool* pbIsIssuer)
{
    if (NULL == ppCertList || NULL == ppCRLList || NULL == pThumbprint || NULL == pbMatch || NULL == pbIsIssuer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Initialize the return values */
    *pbMatch = false;
    *pbIsIssuer = false;

    size_t lenThumbprint = strlen(pThumbprint);
    if (40 != lenThumbprint)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pHeadCertList = *ppCertList; /* Head of list */
    if (NULL == pHeadCertList)
    {
        /* the certificate list is empty, do nothing*/
        return SOPC_STATUS_OK;
    }
    SOPC_CertificateList* cur = pHeadCertList; /* Current cert */
    SOPC_CertificateList* prev = NULL;         /* Parent of current cert */
    error_t errLib = 1;
    int err = -1;
    bool bFound = false;
    bool bIsIssuer = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* pHash = SOPC_Calloc(SOPC_KEY_MANAGER_SHA1_SIZE, sizeof(uint8_t));
    if (NULL == pHash)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    uint8_t* pThumb = SOPC_Calloc(SOPC_KEY_MANAGER_SHA1_SIZE, sizeof(uint8_t));
    if (NULL == pThumb)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        status = SOPC_HelperDecode_Hex(pThumbprint, pThumb, SOPC_KEY_MANAGER_SHA1_SIZE);
    }

    /* Search the certificate */
    while (NULL != cur && !bFound && SOPC_STATUS_OK == status)
    {
        /* Get the current hash */
        errLib = sha1Compute(cur->raw->data, cur->raw->length, pHash);
        status = 0 == errLib ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        /* Compare with the hash ref */
        if (SOPC_STATUS_OK == status)
        {
            err = memcmp(pThumb, pHash, SOPC_KEY_MANAGER_SHA1_SIZE);
            /* Check match*/
            if (0 == err)
            {
                bFound = true;
                bIsIssuer = cur->crt.tbsCert.extensions.basicConstraints.cA;
            }
        }
        /* If the current cert is a CA then remove all the CRLs for that CA */
        if (SOPC_STATUS_OK == status && bFound && bIsIssuer)
        {
            status = sopc_key_manager_crl_list_remove_crl_from_ca(ppCRLList, cur);
        }
        /* Remove the certificate if found */
        if (SOPC_STATUS_OK == status && bFound)
        {
            sopc_key_manager_remove_cert_from_list(&cur, &prev, &pHeadCertList);
        }
        else
        {
            /* iterate */
            prev = cur;
            cur = cur->next;
        }
    }
    /* Clear */
    SOPC_Free(pHash);
    SOPC_Free(pThumb);
    *ppCertList = pHeadCertList;
    *pbMatch = bFound;
    *pbIsIssuer = bIsIssuer;
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_IsSelfSigned(const SOPC_CertificateList* pCert, bool* pbIsSelfSigned)
{
    if (NULL == pCert)
    {
        return SOPC_STATUS_NOK;
    }

    *pbIsSelfSigned = false;
    const X509CertificateInfo crt = pCert->crt;
    /* Verify that the CA is self sign */
    bool_t match = x509CompareName(crt.tbsCert.issuer.raw.value, crt.tbsCert.issuer.raw.length,
                                   crt.tbsCert.subject.raw.value, crt.tbsCert.subject.raw.length);
    if (match)
    {
        error_t errLib = x509VerifySignature(&crt.tbsCert.raw, &crt.signatureAlgo, &crt.tbsCert.subjectPublicKeyInfo,
                                             &crt.signatureValue);
        if (0 == errLib)
        {
            /* Finally the certificate is self signed */
            *pbIsSelfSigned = true;
        }
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_Certificate_Copy(const SOPC_CertificateList* pCert, SOPC_CertificateList** ppCertCopy)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == pCert && NULL == ppCertCopy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const SOPC_CertificateList* pCertCopy = pCert;
    while (NULL != pCertCopy && SOPC_STATUS_OK == status)
    {
        status =
            SOPC_KeyManager_Certificate_CreateOrAddFromDER(pCertCopy->raw->data, pCertCopy->raw->length, ppCertCopy);
        pCertCopy = pCertCopy->next;
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(*ppCertCopy);
        *ppCertCopy = NULL;
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_CertificateList_AttachToSerializedArray(const SOPC_CertificateList* pCerts,
                                                                          SOPC_SerializedCertificate** pSerializedArray,
                                                                          uint32_t* pLenArray)
{
    if (NULL == pCerts || NULL == pSerializedArray || NULL == pLenArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pSerializedArray = NULL;
    *pLenArray = 0;
    SOPC_SerializedCertificate* pArray = NULL;
    size_t listLen = 0;
    uint32_t nbCert = 0;
    SOPC_Buffer* pBuffer = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCerts, &listLen);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check before cast */
    if (UINT32_MAX < listLen)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    nbCert = (uint32_t) listLen;
    pArray = SOPC_Calloc(nbCert, sizeof(SOPC_SerializedCertificate));
    if (NULL == pArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    const SOPC_CertificateList* crt = pCerts;
    uint32_t idx = 0;

    for (idx = 0; idx < nbCert && SOPC_STATUS_OK == status && NULL != crt; idx++)
    {
        pBuffer = (SOPC_Buffer*) &pArray[idx];

        if (NULL == crt->raw || NULL == crt->raw->data)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            pBuffer->position = 0;
            pBuffer->length = crt->raw->length;
            pBuffer->initial_size = crt->raw->length;
            pBuffer->current_size = crt->raw->length;
            pBuffer->maximum_size = crt->raw->length;
            pBuffer->data = crt->raw->data; // Attach data
        }
        crt = crt->next;
    }
    /* Check the length */
    if (SOPC_STATUS_OK == status && nbCert != idx)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    /* Clear in case of error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pArray);
        pArray = NULL;
        nbCert = 0;
    }
    *pSerializedArray = pArray;
    *pLenArray = nbCert;
    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Certificate Revocation List API
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromDER(const uint8_t* bufferDER,
                                                         uint32_t lenDER,
                                                         SOPC_CRLList** ppCRL)
{
    if (NULL == bufferDER || 0 == lenDER || NULL == ppCRL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    error_t errLib = 1;

    /* Create the new CRL and initialize it */
    SOPC_CRLList* pCRLNew = SOPC_Calloc(1, sizeof(SOPC_CRLList));
    if (NULL == pCRLNew)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    pCRLNew->next = NULL;
    pCRLNew->raw = SOPC_Buffer_Create(lenDER);
    if (NULL == pCRLNew->raw)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(pCRLNew->raw, bufferDER, lenDER);
    }

    /* Fill the CRL */
    if (SOPC_STATUS_OK == status)
    {
        // Create the crl from the attached buffer because Cyclone crl will point on it
        errLib = x509ParseCrl(pCRLNew->raw->data, (size_t) lenDER, &pCRLNew->crl);
        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_COMMON,
                "KeyManager: crl buffer parse failed with error code: %d. Maybe the CRL is empty ?\n", errLib);
        }
    }

    /* In case of error, free the new CRL and the whole list */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pCRLNew);
        SOPC_KeyManager_CRL_Free(*ppCRL);
        *ppCRL = NULL;
    }
    else /* Finally add the CRL to the list in ppCRL */
    {
        /* Special case when the input list is not created */
        if (NULL == *ppCRL)
        {
            *ppCRL = pCRLNew;
        }
        else
        {
            /* Otherwise, put the new CRL at the end of ppCRL */
            SOPC_CRLList* pCRLLast = *ppCRL;
            while (NULL != pCRLLast->next)
            {
                pCRLLast = pCRLLast->next;
            }
            pCRLLast->next = pCRLNew;
        }
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 * \note    The file can be DER, or PEM with only 1 crl.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromFile(const char* szPath, SOPC_CRLList** ppCRL)
{
    if (NULL == szPath || NULL == ppCRL)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "KeyManager: crl file \"%s\" parse failed: misses the trailing '\n'", szPath);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    error_t errLib = 1;

    SOPC_Buffer* pBuffer = NULL;
    SOPC_ReturnStatus status = SOPC_Buffer_ReadFile(szPath, &pBuffer);

    if (SOPC_STATUS_OK == status)
    {
        // File is PEM or DER ? CRLs are PEM-encoded using the "X509 CRL" label. This functions returns -1 if no label
        // has been found.
        int_t i = pemFindTag((char_t*) pBuffer->data, pBuffer->length, "-----BEGIN ", "X509 CRL", "-----");

        /* If the file is in DER format, directly call the crl creation function */
        if (0 > i)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(pBuffer->data, pBuffer->length, ppCRL);
        }
        else // The file is PEM, decode it first.
        {
            uint8_t* bufferDER = SOPC_Malloc((2 * pBuffer->length) * sizeof(uint8_t));
            size_t len_bufferDER;

            // Convert the PEM to DER
            size_t length_read = 0;
            errLib = pemImportCrl((char_t*) pBuffer->data, pBuffer->length, bufferDER, &len_bufferDER, &length_read);
            if (0 == errLib)
            {
                if (length_read + 1 < pBuffer->length) // + 1 for the case: the data is a null-terminated string
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                           "KeyManager: PEM crl file \"%s\" probably contains more than 1 crl. Please "
                                           "use PEM that contains one unique crl.'\n'",
                                           szPath);
                    status = SOPC_STATUS_NOK;
                }
                else
                {
                    status = SOPC_KeyManager_CRL_CreateOrAddFromDER(bufferDER, (uint32_t) len_bufferDER, ppCRL);
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }

            SOPC_Free(bufferDER);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> KeyManager: crl file \"%s\" parse failed.\n", szPath);
    }

    SOPC_Buffer_Delete(pBuffer);

    return status;
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
    SOPC_CRLList* crl = pCrls;
    while (crl != NULL && SOPC_STATUS_OK == status)
    {
        status = raw_buf_to_der_file(crl->raw, directoryPath);
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
    const SOPC_CRLList* pCrlCopy = pCrl;
    while (NULL != pCrlCopy && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(pCrlCopy->raw->data, pCrlCopy->raw->length, ppCrlCopy);
        pCrlCopy = pCrlCopy->next;
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(*ppCrlCopy);
        *ppCrlCopy = NULL;
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_CRL_GetListLength(const SOPC_CRLList* pCrl, size_t* pLength)
{
    if (NULL == pCrl || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CRLList* pCrlCpy = pCrl;
    size_t i = 0;
    for (; NULL != pCrlCpy; ++i)
    {
        pCrlCpy = pCrlCpy->next;
    }

    *pLength = i;

    return SOPC_STATUS_OK;
}

void SOPC_KeyManager_CRL_Free(SOPC_CRLList* pCRL)
{
    if (NULL == pCRL)
    {
        return;
    }

    /* Free all the data in the cert, then free the Certificate. */
    SOPC_Buffer_Delete(pCRL->raw);
    SOPC_KeyManager_CRL_Free(pCRL->next);
    SOPC_Free(pCRL);
}

SOPC_ReturnStatus SOPC_KeyManager_CRLList_AttachToSerializedArray(const SOPC_CRLList* pCRLs,
                                                                  SOPC_SerializedCRL** pSerializedArray,
                                                                  uint32_t* pLenArray)
{
    if (NULL == pCRLs || NULL == pSerializedArray || NULL == pLenArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pSerializedArray = NULL;
    *pLenArray = 0;
    size_t listLen = 0;
    uint32_t nbCrl = 0;
    SOPC_Buffer* pBuffer = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_CRL_GetListLength(pCRLs, &listLen);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Check before cast */
    if (UINT32_MAX < listLen)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    nbCrl = (uint32_t) listLen;
    SOPC_SerializedCRL* pArray = SOPC_Calloc(nbCrl, sizeof(SOPC_SerializedCRL));
    if (NULL == pArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    const SOPC_CRLList* crl = pCRLs;
    uint32_t idx = 0;

    for (idx = 0; idx < nbCrl && SOPC_STATUS_OK == status && NULL != crl; idx++)
    {
        pBuffer = (SOPC_Buffer*) &pArray[idx];
        if (NULL == crl->raw || NULL == crl->raw->data)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            pBuffer->position = 0;
            pBuffer->length = crl->raw->length;
            pBuffer->initial_size = crl->raw->length;
            pBuffer->current_size = crl->raw->length;
            pBuffer->maximum_size = crl->raw->length;
            pBuffer->data = crl->raw->data; // Attach data
        }
        crl = crl->next;
    }

    /* Check the length */
    if (SOPC_STATUS_OK == status && nbCrl != idx)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    /* Clear in case of error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pArray);
        pArray = NULL;
        nbCrl = 0;
    }
    *pSerializedArray = pArray;
    *pLenArray = nbCrl;
    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Certificate Signing request API
 * ------------------------------------------------------------------------------------------------
 */

static void sopc_set_x509_string(X509String* dest, const char* value, size_t len)
{
    if (NULL == dest)
    {
        return;
    }

    dest->value = value;
    dest->length = len;
}

static SOPC_ReturnStatus sopc_parse_subject_name(const char* subjectName, X509Name* name)
{
    if (NULL == subjectName || NULL == name)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    memset(name, 0, sizeof(*name));

    const char* p = subjectName;

    while (*p != '\0')
    {
        while (*p == ',' || *p == '/' || isspace((unsigned char) *p))
        {
            p++;
        }

        if (*p == '\0')
        {
            break;
        }

        const char* tokenStart = p;
        while (*p != '\0' && *p != ',' && *p != '/')
        {
            p++;
        }

        const char* tokenEnd = p;
        while (tokenEnd > tokenStart && isspace((unsigned char) tokenEnd[-1]))
        {
            tokenEnd--;
        }

        const char* eq = tokenStart;
        while (eq < tokenEnd && *eq != '=')
        {
            eq++;
        }

        if (eq == tokenEnd)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }

        const char* key = tokenStart;
        size_t keyLen = (size_t)(eq - tokenStart);
        SOPC_strtrim(&key, &keyLen);

        const char* val = eq + 1;
        size_t valLen = (size_t)(tokenEnd - val);
        SOPC_strtrim(&val, &valLen);

        if (0 == valLen || 0 == keyLen)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }

        if (keyLen == 2 && 0 == SOPC_strncmp_ignore_case(key, "CN", 2))
        {
            sopc_set_x509_string(&name->commonName, val, valLen);
        }
        else if (keyLen == 1 && 0 == SOPC_strncmp_ignore_case(key, "O", 1))
        {
            sopc_set_x509_string(&name->organizationName, val, valLen);
        }
        else if (keyLen == 2 && 0 == SOPC_strncmp_ignore_case(key, "OU", 2))
        {
            sopc_set_x509_string(&name->organizationalUnitName, val, valLen);
        }
        else if (keyLen == 1 && 0 == SOPC_strncmp_ignore_case(key, "C", 1))
        {
            sopc_set_x509_string(&name->countryName, val, valLen);
        }
        else if (keyLen == 1 && 0 == SOPC_strncmp_ignore_case(key, "L", 1))
        {
            sopc_set_x509_string(&name->localityName, val, valLen);
        }
        else if (keyLen == 2 && 0 == SOPC_strncmp_ignore_case(key, "ST", 2))
        {
            sopc_set_x509_string(&name->stateOrProvinceName, val, valLen);
        }
    }

    if (NULL == name->commonName.value || 0 == name->commonName.length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_STATUS_OK;
}

static void sopc_set_rsa_sha256_signature_oid(X509SignAlgoId* algo)
{
    static const uint8_t OID_SHA256_WITH_RSA[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B};

    SOPC_ASSERT(NULL != algo);

    memset(algo, 0, sizeof(*algo));
    algo->oid.value = OID_SHA256_WITH_RSA;
    algo->oid.length = sizeof(OID_SHA256_WITH_RSA);
}

SOPC_ReturnStatus SOPC_KeyManager_CSR_Create(const char* subjectName,
                                             const bool bIsServer,
                                             const char* mdType,
                                             const char* uri,
                                             char** pDnsArray,
                                             uint32_t arrayLength,
                                             SOPC_CSR** ppCSR)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_CSR* pCSR = NULL;

    if (NULL == subjectName || NULL == mdType || NULL == ppCSR)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (arrayLength > 0 && NULL == pDnsArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (((NULL != uri) ? 1u : 0u) + arrayLength > X509_MAX_SUBJECT_ALT_NAMES)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppCSR = NULL;
    pCSR = SOPC_Calloc(1, sizeof(SOPC_CSR));
    if (NULL == pCSR)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    memset(&pCSR->csr, 0, sizeof(pCSR->csr));

    /* CSR v1 */
    pCSR->csr.certReqInfo.version = X509_VERSION_1;

    /* Subject */
    status = sopc_parse_subject_name(subjectName, &pCSR->csr.certReqInfo.subject);
    if (SOPC_STATUS_OK == status)
    {
        /* Signature algorithm */
        sopc_set_rsa_sha256_signature_oid(&pCSR->csr.signatureAlgo);

        /* Extensions */
        X509Extensions* ext = &pCSR->csr.certReqInfo.attributes.extensionReq;
        memset(ext, 0, sizeof(*ext));

        /* Basic constraints : a client/server CSR can't be CA */
        ext->basicConstraints.critical = true;
        ext->basicConstraints.cA = false;
        ext->basicConstraints.pathLenConstraint = 0;

        /* Key usage */
        ext->keyUsage.critical = true;
        ext->keyUsage.bitmap = X509_KEY_USAGE_DIGITAL_SIGNATURE | X509_KEY_USAGE_NON_REPUDIATION |
                               X509_KEY_USAGE_KEY_ENCIPHERMENT | X509_KEY_USAGE_DATA_ENCIPHERMENT;

        /* Extended key usage */
        ext->extKeyUsage.critical = false;
        ext->extKeyUsage.bitmap = bIsServer ? X509_EXT_KEY_USAGE_SERVER_AUTH : X509_EXT_KEY_USAGE_CLIENT_AUTH;

        /* SubjectAltName */
        if (NULL != uri || arrayLength > 0)
        {
            uint32_t sanIdx = 0;
            ext->subjectAltName.critical = false;

            if (NULL != uri)
            {
                ext->subjectAltName.generalNames[sanIdx].type = X509_GENERAL_NAME_TYPE_URI;
                ext->subjectAltName.generalNames[sanIdx].value = uri;
                ext->subjectAltName.generalNames[sanIdx].length = strlen(uri);
                sanIdx++;
            }

            for (uint32_t i = 0; i < arrayLength && SOPC_STATUS_OK == status; ++i)
            {
                if (NULL == pDnsArray[i])
                {
                    status = SOPC_STATUS_INVALID_PARAMETERS;
                }
                else
                {
                    ext->subjectAltName.generalNames[sanIdx].type = X509_GENERAL_NAME_TYPE_DNS;
                    ext->subjectAltName.generalNames[sanIdx].value = pDnsArray[i];
                    ext->subjectAltName.generalNames[sanIdx].length = strlen(pDnsArray[i]);
                    sanIdx++;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                ext->subjectAltName.numGeneralNames = sanIdx;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            /* subjectPublicKeyInfo will be filled in ToDER function */
            memset(&pCSR->csr.certReqInfo.subjectPublicKeyInfo, 0, sizeof(pCSR->csr.certReqInfo.subjectPublicKeyInfo));
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *ppCSR = pCSR;
    }
    else
    {
        SOPC_Free(pCSR);
    }

    return status;
}

// Convert RsaPublicKey into X509SubjectPublicKeyInfo
static SOPC_ReturnStatus sopc_fill_subject_public_key_info_rsa(X509SubjectPublicKeyInfo* pSpki,
                                                               const RsaPublicKey* pPub,
                                                               uint8_t** ppModulus,
                                                               uint8_t** ppExponent)
{
    static const uint8_t OID_RSA_ENCRYPTION[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01};

    if (NULL == pSpki || NULL == pPub || NULL == ppModulus || NULL == ppExponent)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    size_t modulusLen = 0;
    size_t exponentLen = 0;

    *ppModulus = NULL;
    *ppExponent = NULL;
    memset(pSpki, 0, sizeof(*pSpki));

    modulusLen = mpiGetByteLength(&pPub->n);
    exponentLen = mpiGetByteLength(&pPub->e);

    if (0 == modulusLen || 0 == exponentLen)
    {
        return SOPC_STATUS_NOK;
    }

    *ppModulus = SOPC_Malloc(modulusLen);
    *ppExponent = SOPC_Malloc(exponentLen);

    if (NULL == *ppModulus || NULL == *ppExponent)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Export MPI integers to big-endian format */
        if (NO_ERROR != mpiExport(&pPub->n, *ppModulus, modulusLen, MPI_FORMAT_BIG_ENDIAN) ||
            NO_ERROR != mpiExport(&pPub->e, *ppExponent, exponentLen, MPI_FORMAT_BIG_ENDIAN))
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Fill SPKI */
        pSpki->oid.value = OID_RSA_ENCRYPTION;
        pSpki->oid.length = sizeof(OID_RSA_ENCRYPTION);
        pSpki->rsaPublicKey.n.value = *ppModulus;
        pSpki->rsaPublicKey.n.length = modulusLen;
        pSpki->rsaPublicKey.e.value = *ppExponent;
        pSpki->rsaPublicKey.e.length = exponentLen;
    }
    else
    {
        SOPC_Free(*ppModulus);
        SOPC_Free(*ppExponent);
        *ppModulus = NULL;
        *ppExponent = NULL;
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_CSR_ToDER(SOPC_CSR* pCSR,
                                            SOPC_AsymmetricKey* pKey,
                                            uint8_t** ppDest,
                                            uint32_t* pLenAllocated)
{
    size_t written = 0;
    uint8_t* pOut = NULL;
    uint8_t* modulus = NULL;
    uint8_t* exponent = NULL;
    RsaPrivateKey* pRsaPriv = NULL;
    RsaPublicKey* pRsaPub = NULL;
    const PrngAlgo* prngAlgo = NULL;
    void* prngContext = NULL;

    if (NULL == pCSR || NULL == pKey || NULL == ppDest || NULL == pLenAllocated)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppDest = NULL;
    *pLenAllocated = 0;

    pRsaPriv = &pKey->privKey;
    pRsaPub = &pKey->pubKey;

    if (0 == mpiGetBitLength(&pRsaPub->n) || 0 == mpiGetBitLength(&pRsaPub->e))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = sopc_fill_subject_public_key_info_rsa(&pCSR->csr.certReqInfo.subjectPublicKeyInfo,
                                                                     pRsaPub, &modulus, &exponent);

    if (SOPC_STATUS_OK == status)
    {
        pOut = SOPC_Malloc(SOPC_CSR_DER_MAX_SIZE);
        if (NULL == pOut)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        error_t err = x509CreateCsr(prngAlgo, prngContext, &pCSR->csr.certReqInfo, pRsaPub, &pCSR->csr.signatureAlgo,
                                    pRsaPriv, pOut, &written);
        if (NO_ERROR != err || written > UINT32_MAX)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *ppDest = pOut;
        *pLenAllocated = (uint32_t) written;
    }
    else
    {
        SOPC_Free(pOut);
    }

    SOPC_Free(modulus);
    SOPC_Free(exponent);

    return status;
}

void SOPC_KeyManager_CSR_Free(SOPC_CSR* pCSR)
{
    if (NULL == pCSR)
    {
        return;
    }
    memset(&pCSR->csr, 0, sizeof(pCSR->csr));
    SOPC_Free(pCSR);
}
