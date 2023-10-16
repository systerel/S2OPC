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
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_mem_alloc.h"

#include "key_manager_lib.h"

#include "pkix/pem_decrypt.h"
#include "pkix/pem_export.h"
#include "pkix/pem_import.h"
#include "pkix/pkcs8_key_parse.h"
#include "pkix/x509_cert_parse.h"
#include "pkix/x509_cert_validate.h"
#include "pkix/x509_crl_parse.h"
#include "pkix/x509_key_format.h"
#include "pkix/x509_key_parse.h"

/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * Creates an asymmetric key from a \p buffer.
 * If the \p buffer contains a private key, the key must NOT be encrypted.
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
        return SOPC_STATUS_NOK;
    }

    key->isBorrowedFromCert = false;
    rsaInitPublicKey(&key->pubKey);
    rsaInitPrivateKey(&key->privKey);

    error_t errLib = 1;

    if (is_public)
    {
        errLib = pemImportRsaPublicKey((const char_t*) buffer, (size_t) lenBuf, &key->pubKey);

        if (errLib) // The buffer is probably in DER format...
        {
            // We will then directly parse it.
            // Corresponds to the case "PUBLIC KEY" when calling the function pemImportRsaPublicKey.
            size_t lenParsed = 0;
            X509SubjectPublicKeyInfo publicKeyInfo = {0};
            errLib = x509ParseSubjectPublicKeyInfo(buffer, lenBuf, &lenParsed, &publicKeyInfo);

            // If no error, continue...
            if (!errLib)
            {
                errLib = x509ImportRsaPublicKey(&publicKeyInfo, &key->pubKey);
            }
        }
    }
    else // Key is private
    {
        errLib = pemImportRsaPrivateKey((const char_t*) buffer, (size_t) lenBuf, NULL, &key->privKey);

        if (errLib) // The buffer is probably in DER format...
        {
            // We will then directly parse it.
            // Corresponds to the case "RSA PRIVATE KEY" when calling the function pemImportRsaPrivateKey.
            Pkcs8PrivateKeyInfo privateKeyInfo = {0};
            errLib = pkcs8ParseRsaPrivateKey(buffer, lenBuf, &privateKeyInfo.rsaPrivateKey);

            // If no error, continue...
            if (!errLib)
            {
                privateKeyInfo.oid = RSA_ENCRYPTION_OID;
                privateKeyInfo.oidLen = sizeof(RSA_ENCRYPTION_OID);
                errLib = pkcs8ImportRsaPrivateKey(&privateKeyInfo, &key->privKey);
            }

            // Clear private key info
            memset(&privateKeyInfo, 0, sizeof(Pkcs8PrivateKeyInfo));
        }

        /* If no error, fill the public part */
        if (!errLib)
        {
            errLib = rsaGeneratePublicKey(&key->privKey, &key->pubKey);
        }
    }

    if (errLib)
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

        if (!errLib)
        {
            status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer((uint8_t*) buffer_decrypted,
                                                                    (uint32_t) lenBuffer_decrypted, false, ppKey);
        }

        SOPC_Free(buffer_decrypted);
    }

    SOPC_SecretBuffer_Unexpose(pBuffer, pSecretBuffer);
    SOPC_SecretBuffer_DeleteClear(pSecretBuffer);
    return status;
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
 * \brief Creates a DER from the AsymmetricKey \p pKey and copies it to \p pDest.
 * This function does not allocate the buffer containing the DER.
 * The operation may fail if the allocated buffer is not large enough.
 * The required length cannot be precisely calculated, but a value of 8 times the key length in bytes is recommended.
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
        errLib = pemExportRsaPublicKey(&pKey->pubKey, bufferPEM, &writtenPEM);
        if (!errLib)
        {
            errLib = pemDecodeFile(bufferPEM, writtenPEM, "PUBLIC KEY", buffer, &lengthWritten, NULL, NULL);
        }

        SOPC_Free(bufferPEM);
    }
    else // Key is private
    {
        errLib = x509ExportRsaPrivateKey(&pKey->privKey, buffer, &lengthWritten);
    }

    if (!errLib)
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
        if (errLib)
        {
            status = SOPC_STATUS_NOK;
            fprintf(stderr, "> KeyManager: certificate buffer parse failed with error code: %d\n", errLib);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Allocate and fill the public key of the cert */
        rsaInitPublicKey(&pCertNew->pubKey);
        errLib = x509ImportRsaPublicKey(&pCertNew->crt.tbsCert.subjectPublicKeyInfo, &pCertNew->pubKey);
        if (errLib)
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
        return status;
    }

    /* Finally add the cert to the list in ppCert */
    if (SOPC_STATUS_OK == status)
    {
        /* Special case when the input list is not created */
        if (NULL == *ppCert)
        {
            *ppCert = pCertNew;
            return SOPC_STATUS_OK;
        }
    }
    SOPC_CertificateList* pCertLast = *ppCert;
    if (SOPC_STATUS_OK == status)
    {
        while (NULL != pCertLast->next)
        {
            pCertLast = pCertLast->next;
        }
        pCertLast->next = pCertNew;
        return SOPC_STATUS_OK;
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 * The file can be DER, or PEM with only 1 certificate.
 */
SOPC_ReturnStatus SOPC_KeyManager_Certificate_CreateOrAddFromFile(const char* szPath, SOPC_CertificateList** ppCert)
{
    if (NULL == szPath || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    error_t errLib = 1;

    SOPC_Buffer* pBuffer = NULL;
    SOPC_ReturnStatus status = SOPC_Buffer_ReadFile(szPath, &pBuffer);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBuffer->data, pBuffer->length, ppCert);

        /* It failed. Maybe the file is PEM ? */
        if (SOPC_STATUS_OK != status)
        {
            uint8_t* bufferDER = SOPC_Malloc((2 * pBuffer->length) * sizeof(uint8_t));
            size_t len_bufferDER;

            // Convert the PEM to DER
            errLib = pemImportCertificate((char_t*) pBuffer->data, pBuffer->length, bufferDER, &len_bufferDER, NULL);

            if (!errLib)
            {
                status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(bufferDER, (uint32_t) len_bufferDER, ppCert);
            }

            SOPC_Free(bufferDER);
        }
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

    // SOPC_ReturnStatus status = certificate_check_single(pCert);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    error_t errLib = 1;

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
        if (errLib)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    SOPC_Free(pDER);

    return status;
}

/**
 * \brief   Fills \p pKey with RSA public key information retrieved from \p pCert.
 * Warning : The \p pKey points on the fields of the cert. Free the cert will make the key unusable.
 */

SOPC_ReturnStatus KeyManager_Certificate_GetPublicKey(const SOPC_CertificateList* pCert, SOPC_AsymmetricKey* pKey)
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

/* Creates a new string. Later have to free the result */
static char* get_raw_sha1(SOPC_Buffer* raw)
{
    SOPC_ASSERT(NULL != raw);

    uint8_t pDest[20];

    error_t errLib = sha1Compute(raw->data, raw->length, pDest);

    if (errLib)
    {
        fprintf(stderr, "Cannot compute thumbprint of certificate, err -0x%X\n", errLib);
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
char* SOPC_KeyManager_Certificate_GetCstring_SHA1(SOPC_CertificateList* pCert)
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
    SOPC_CertificateList* current_cert = pCert;
    SOPC_CertificateList* prev_cert = NULL; // The parent of the current cert
    while (NULL != current_cert)
    {
        /* Skip certificates that are not authorities */
        if (!current_cert->crt.tbsCert.extensions.basicConstraints.cA)
        {
            prev_cert = current_cert;
            current_cert = current_cert->next;
            continue;
        }

        int crl_found = 0;
        const SOPC_CRLList* current_crl = pCRL;
        for (; NULL != current_crl; current_crl = current_crl->next)
        {
            bool match = x509CompareName(
                current_crl->crl.tbsCertList.issuer.rawData, current_crl->crl.tbsCertList.issuer.rawDataLen,
                current_cert->crt.tbsCert.subject.rawData, current_cert->crt.tbsCert.subject.rawDataLen);

            if (crl_found > 0 && match)
            {
                if (crl_found)
                {
                    char* fpr = get_raw_sha1(current_cert->raw);
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
            char* fpr = get_raw_sha1(current_cert->raw);
            fprintf(stderr,
                    "> MatchCRLList warning: Certificate with SHA-1 fingerprint %s has no CRL or multiple CRLs, and is "
                    "removed from the CAs list.\n",
                    fpr);
            SOPC_Free(fpr);

            /* Remove the certificate from the chain and safely delete it */
            SOPC_CertificateList* next = current_cert->next;
            current_cert->next = NULL;
            // We can't call SOPC_KeyManager_Certificate_Free() because it will delete the CertificateList.
            SOPC_Buffer_Delete(current_cert->raw);
            rsaFreePublicKey(&current_cert->pubKey);

            /* Set new next certificate (if possible) */
            if (NULL == prev_cert)
            {
                if (NULL == next)
                {
                    /*
                     * The list would be empty, but we cannot do it here.
                     * We have no choice but failing with current design.
                     */
                    current_cert = NULL; // make iteration stop
                    status = SOPC_STATUS_NOK;
                }
                else
                {
                    /* Head of the chain is a special case */
                    *current_cert = *next;
                    /* We have to free the new next certificate (copied as first one) */
                    SOPC_Free(next);

                    /* Do not iterate: current certificate has changed */
                }
            }
            else
            {
                /* We have to free the certificate if it is not the first in the list */
                prev_cert->next = next;
                SOPC_Free(current_cert);
                /* Iterate */
                current_cert = next;
            }
        }
        else
        {
            /* Iterate */
            prev_cert = current_cert;
            current_cert = current_cert->next;
        }
    }

    /* There may be unused CRLs */
    if (NULL != pbMatch)
    {
        *pbMatch = list_match;
    }

    return status;
}

/* Warning : this function only checks the first cert of the list pCert. */
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
        // Create the certificate from the attached buffer because Cyclone cert will point on it
        errLib = x509ParseCrl(pCRLNew->raw->data, (size_t) lenDER, &pCRLNew->crl);
        if (errLib)
        {
            status = SOPC_STATUS_NOK;
            fprintf(stderr, "> KeyManager: crl buffer parse failed with error code: %d. Maybe the CRL is empty ?\n",
                    errLib);
        }
    }

    /* In case of error, free the new certificate and the whole list */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pCRLNew);
        SOPC_KeyManager_CRL_Free(*ppCRL);
        *ppCRL = NULL;
        return status;
    }

    /* Finally add the cert to the list in ppCert */
    if (SOPC_STATUS_OK == status)
    {
        /* Special case when the input list is not created */
        if (NULL == *ppCRL)
        {
            *ppCRL = pCRLNew;
            return SOPC_STATUS_OK;
        }
    }
    SOPC_CRLList* pCRLLast = *ppCRL;
    if (SOPC_STATUS_OK == status)
    {
        while (NULL != pCRLLast->next)
        {
            pCRLLast = pCRLLast->next;
        }
        pCRLLast->next = pCRLNew;
        return SOPC_STATUS_OK;
    }

    return status;
}

/**
 * \note    Tested but not part of the test suites.
 * The file can be DER, or PEM with only 1 crl.
 */
SOPC_ReturnStatus SOPC_KeyManager_CRL_CreateOrAddFromFile(const char* szPath, SOPC_CRLList** ppCRL)
{
    if (NULL == szPath || NULL == ppCRL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Buffer* pBuffer = NULL;
    SOPC_ReturnStatus status = SOPC_Buffer_ReadFile(szPath, &pBuffer);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(pBuffer->data, pBuffer->length, ppCRL);

        /* It failed. Maybe the file is PEM ? */
        if (SOPC_STATUS_OK != status)
        {
            uint8_t* bufferDER = SOPC_Malloc((2 * pBuffer->length) * sizeof(uint8_t));
            size_t len_bufferDER;

            // Convert the PEM to DER
            error_t errLib = pemImportCrl((char_t*) pBuffer->data, pBuffer->length, bufferDER, &len_bufferDER, NULL);
            if (!errLib)
            {
                status = SOPC_KeyManager_CRL_CreateOrAddFromDER(bufferDER, (uint32_t) len_bufferDER, ppCRL);
            }

            SOPC_Free(bufferDER);
        }
    }

    SOPC_Buffer_Delete(pBuffer);

    return status;
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
