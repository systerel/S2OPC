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

/** \file sopc_key_manager.c
 *
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 * Most of the functions are lib-dependent. This file defines the others.
 */

#include <string.h>

#include "key_manager_lib.h"
#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_key_manager.h"
#include "sopc_mem_alloc.h"

/* ------------------------------------------------------------------------------------------------
 * AsymetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 * Serialization/Deserialization API
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(const uint8_t* data,
                                                                         uint32_t len,
                                                                         SOPC_SerializedAsymmetricKey** key)
{
    SOPC_SecretBuffer* sec = SOPC_SecretBuffer_New(len);

    if (sec == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ExposedBuffer* buf = SOPC_SecretBuffer_ExposeModify(sec);
    memcpy(buf, data, (size_t) len);
    SOPC_SecretBuffer_UnexposeModify(buf, sec);
    *key = sec;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(const char* path,
                                                                         SOPC_SerializedAsymmetricKey** key)
{
    SOPC_SecretBuffer* sec = SOPC_SecretBuffer_NewFromFile(path);

    if (sec == NULL)
    {
        return SOPC_STATUS_NOK;
    }

    *key = sec;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(const SOPC_SerializedAsymmetricKey* cert,
                                                                      bool is_public,
                                                                      SOPC_AsymmetricKey** res)
{
    uint32_t len = SOPC_SecretBuffer_GetLength(cert);
    const SOPC_ExposedBuffer* buf = SOPC_SecretBuffer_Expose(cert);
    SOPC_ReturnStatus status = SOPC_KeyManager_AsymmetricKey_CreateFromBuffer(buf, len, is_public, res);
    SOPC_SecretBuffer_Unexpose(buf, cert);
    return status;
}

void SOPC_KeyManager_SerializedAsymmetricKey_Delete(SOPC_SerializedAsymmetricKey* key)
{
    SOPC_SecretBuffer_DeleteClear(key);
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromDER(const uint8_t* der,
                                                                      uint32_t len,
                                                                      SOPC_SerializedCertificate** cert)
{
    SOPC_Buffer* buf = SOPC_Buffer_Create(len);

    if (buf == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_Buffer_Write(buf, der, len);
    *cert = buf;
    return status;
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_CreateFromFile(const char* path,
                                                                       SOPC_SerializedCertificate** cert)
{
    return SOPC_Buffer_ReadFile(path, cert);
}

SOPC_ReturnStatus SOPC_KeyManager_SerializedCertificate_Deserialize(const SOPC_SerializedCertificate* cert,
                                                                    SOPC_CertificateList** res)
{
    if (NULL == res || NULL != *res)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_KeyManager_Certificate_CreateOrAddFromDER(cert->data, cert->length, res);
}

const SOPC_Buffer* SOPC_KeyManager_SerializedCertificate_Data(const SOPC_SerializedCertificate* cert)
{
    return cert;
}

void SOPC_KeyManager_SerializedCertificate_Delete(SOPC_SerializedCertificate* cert)
{
    SOPC_Buffer_Delete(cert);
}
