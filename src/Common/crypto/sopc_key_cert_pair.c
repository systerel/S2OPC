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

#include "sopc_key_cert_pair.h"

#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

#include <string.h>

struct SOPC_KeyCertPair
{
    SOPC_Mutex mutex;
    SOPC_SerializedCertificate* certificate;
    SOPC_SerializedAsymmetricKey* key;
    SOPC_KeyCertPairUpdateCb* callback;
    uintptr_t callbackParam;
};

static SOPC_ReturnStatus SOPC_Internal_KeyCertPair_Create(SOPC_SerializedCertificate* cert,
                                                          SOPC_SerializedAsymmetricKey* key,
                                                          SOPC_KeyCertPair** ppKeyCertPair)
{
    SOPC_ASSERT(NULL != cert);
    SOPC_ASSERT(NULL != ppKeyCertPair);
    SOPC_KeyCertPair* newPair = SOPC_Calloc(1, sizeof(*newPair));
    if (NULL == newPair)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(cert);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(key);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&newPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    newPair->certificate = cert;
    newPair->key = key;
    *ppKeyCertPair = newPair;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyCertPair_CreateFromPaths(const char* certPath,
                                                   const char* privateKeyPath,
                                                   char* keyPassword,
                                                   SOPC_KeyCertPair** ppKeyCertPair)
{
    SOPC_SerializedCertificate* cert = NULL;
    SOPC_SerializedAsymmetricKey* key = NULL;
    uint32_t keyPasswordLen = 0;
    if (NULL == certPath || NULL == privateKeyPath || NULL == ppKeyCertPair)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != keyPassword)
    {
        size_t lenPassword = strlen(keyPassword);
        if (UINT32_MAX < lenPassword)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        keyPasswordLen = (uint32_t) lenPassword;
    }

    SOPC_ReturnStatus status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certPath, &cert);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd(privateKeyPath, &key, keyPassword,
                                                                                keyPasswordLen);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_COMMON,
                "Failed to load key from path %s."
                " Please check the password if the key is encrypted and check the key format (PEM or DER)",
                privateKeyPath);
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "Failed to load certificate from path %s."
                               " Please check the certificate is X509 in DER format.",
                               certPath);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Internal_KeyCertPair_Create(cert, key, ppKeyCertPair);
    }
    else
    {
        SOPC_KeyManager_SerializedCertificate_Delete(cert);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(key);
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_CreateCertHolderFromPath(const char* certPath, SOPC_CertHolder** ppCertHolder)
{
    SOPC_SerializedCertificate* cert = NULL;
    if (NULL == certPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certPath, &cert);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "Failed to load certificate from path %s."
                               " Please check the certificate is X509 in DER format.",
                               certPath);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Internal_KeyCertPair_Create(cert, NULL, ppCertHolder);
    }
    else
    {
        SOPC_KeyManager_SerializedCertificate_Delete(cert);
    }
    return status;
}

static SOPC_ReturnStatus SOPC_Internal_CreateFromBytes(size_t certificateNbBytes,
                                                       const unsigned char* certificate,
                                                       bool noKeySet,
                                                       size_t keyNbBytes,
                                                       const unsigned char* privateKey,
                                                       SOPC_SerializedCertificate** pCert,
                                                       SOPC_SerializedAsymmetricKey** pKey)
{
    SOPC_ASSERT(NULL != certificate);
    SOPC_ASSERT(NULL != privateKey || noKeySet);
    SOPC_SerializedCertificate* cert = NULL;
    SOPC_SerializedAsymmetricKey* key = NULL;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_SerializedCertificate_CreateFromDER(certificate, (uint32_t) certificateNbBytes, &cert);
    if (SOPC_STATUS_OK == status)
    {
        if (!noKeySet)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(privateKey, (uint32_t) keyNbBytes, &key);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Failed to load key from bytes array\n");
            }
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Failed to load certificate from bytes array\n");
    }
    if (SOPC_STATUS_OK == status)
    {
        *pCert = cert;
        if (!noKeySet)
        {
            *pKey = key;
        }
    }
    else
    {
        SOPC_KeyManager_SerializedCertificate_Delete(cert);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(key);
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_CreateFromBytes(size_t certificateNbBytes,
                                                   const unsigned char* certificate,
                                                   size_t keyNbBytes,
                                                   const unsigned char* privateKey,
                                                   SOPC_KeyCertPair** ppKeyCertPair)
{
    if (0 == certificateNbBytes || NULL == certificate || 0 == keyNbBytes || NULL == privateKey)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_SerializedCertificate* cert = NULL;
    SOPC_SerializedAsymmetricKey* key = NULL;
    SOPC_ReturnStatus status =
        SOPC_Internal_CreateFromBytes(certificateNbBytes, certificate, false, keyNbBytes, privateKey, &cert, &key);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Internal_KeyCertPair_Create(cert, key, ppKeyCertPair);
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_CreateCertHolderFromBytes(size_t certificateNbBytes,
                                                             const unsigned char* certificate,
                                                             SOPC_CertHolder** ppCertHolder)
{
    if (0 == certificateNbBytes || NULL == certificate)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_SerializedCertificate* cert = NULL;
    SOPC_ReturnStatus status =
        SOPC_Internal_CreateFromBytes(certificateNbBytes, certificate, true, 0, NULL, &cert, NULL);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Internal_KeyCertPair_Create(cert, NULL, ppCertHolder);
    }

    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_SetUpdateCb(SOPC_KeyCertPair* keyCertPair,
                                               SOPC_KeyCertPairUpdateCb* updateCb,
                                               uintptr_t updateParam)
{
    if (NULL == keyCertPair || NULL == updateCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (NULL == keyCertPair->callback)
    {
        keyCertPair->callback = updateCb;
        keyCertPair->callbackParam = updateParam;
        status = SOPC_STATUS_OK;
    }
    mutStatus = SOPC_Mutex_Unlock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_UpdateFromBytes(SOPC_KeyCertPair* keyCertPair,
                                                   size_t certificateNbBytes,
                                                   const unsigned char* certificate,
                                                   size_t keyNbBytes,
                                                   const unsigned char* privateKey)
{
    if (NULL == keyCertPair || 0 == certificateNbBytes || NULL == certificate)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Key might be NULL, check coherent parameters are provided
    if ((0 == keyNbBytes && NULL != privateKey) || (0 != keyNbBytes && NULL == privateKey))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (NULL == keyCertPair->key && NULL != privateKey)
    {
        // SOPC_KeyCertPair is actually a SOPC_CertHolder and cannot be update with a key
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (NULL != keyCertPair->callback)
    {
        SOPC_SerializedCertificate* cert = NULL;
        SOPC_SerializedAsymmetricKey* key = NULL;
        status = SOPC_Internal_CreateFromBytes(certificateNbBytes, certificate, (NULL == privateKey), keyNbBytes,
                                               privateKey, &cert, &key);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_KeyManager_SerializedCertificate_Delete(keyCertPair->certificate);
            keyCertPair->certificate = cert;
            if (NULL != privateKey)
            {
                SOPC_KeyManager_SerializedAsymmetricKey_Delete(keyCertPair->key);
                keyCertPair->key = key;
            }
            keyCertPair->callback(keyCertPair->callbackParam);
        }
    }
    else
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "Failed to update a key /certificate pair since update is not authorized without associated callback set."
            "Use SOPC_KeyCertPair_SetUpdateCb to define a callback and implement necessary consequences of update.");
    }
    mutStatus = SOPC_Mutex_Unlock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_GetSerializedCertCopy(SOPC_KeyCertPair* keyCertPair,
                                                         SOPC_SerializedCertificate** ppCertCopy)
{
    if (NULL == keyCertPair || NULL == ppCertCopy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_SerializedCertificate* certCopy = SOPC_Buffer_Create(keyCertPair->certificate->length);
    if (NULL != certCopy)
    {
        status = SOPC_Buffer_Copy(certCopy, keyCertPair->certificate);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Reset position in case it is not at 0
        status = SOPC_Buffer_SetPosition(certCopy, 0);
    }
    mutStatus = SOPC_Mutex_Unlock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (SOPC_STATUS_OK == status)
    {
        *ppCertCopy = certCopy;
    }
    else
    {
        SOPC_Buffer_Delete(certCopy);
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_GetCertCopy(SOPC_KeyCertPair* keyCertPair, SOPC_CertificateList** ppCertCopy)
{
    if (NULL == keyCertPair || NULL == ppCertCopy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    SOPC_CertificateList* certCopy = NULL;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(keyCertPair->certificate->data,
                                                            keyCertPair->certificate->length, &certCopy);
    mutStatus = SOPC_Mutex_Unlock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (SOPC_STATUS_OK == status)
    {
        *ppCertCopy = certCopy;
    }
    return status;
}

SOPC_ReturnStatus SOPC_KeyCertPair_GetKeyCopy(SOPC_KeyCertPair* keyCertPair, SOPC_AsymmetricKey** ppKeyCopy)
{
    if (NULL == keyCertPair || NULL == ppKeyCopy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    SOPC_AsymmetricKey* keyCopy = NULL;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(keyCertPair->key, false, &keyCopy);
    mutStatus = SOPC_Mutex_Unlock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (SOPC_STATUS_OK == status)
    {
        *ppKeyCopy = keyCopy;
    }
    return status;
}

void SOPC_KeyCertPair_Delete(SOPC_KeyCertPair** ppKeyCertPair)
{
    if (NULL == ppKeyCertPair || NULL == *ppKeyCertPair)
    {
        return;
    }
    SOPC_KeyCertPair* keyCertPair = *ppKeyCertPair;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    keyCertPair->callback = NULL;
    SOPC_KeyManager_SerializedCertificate_Delete(keyCertPair->certificate);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(keyCertPair->key);
    keyCertPair->certificate = NULL;
    keyCertPair->key = NULL;
    mutStatus = SOPC_Mutex_Unlock(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Clear(&keyCertPair->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Free(keyCertPair);
    *ppKeyCertPair = NULL;
}
