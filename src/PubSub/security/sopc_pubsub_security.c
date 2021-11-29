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

#include "sopc_pubsub_security.h"
#include <assert.h>
#include "sopc_buffer.h"
#include "sopc_crypto_provider.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_constants.h"
#include "sopc_secret_buffer.h"

/* Spec OPCUA Part 14 define size of Nonce ( group, nonce, sequence number */
#define SOPC_PUBSUB_SECURITY_RANDOM_LENGTH 4

SOPC_Buffer* SOPC_PubSub_Security_Encrypt(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* payload)
{
    if (NULL == security || NULL == security->provider || NULL == security->groupKeys || NULL == payload)
    {
        return NULL;
    }
    SOPC_ReturnStatus status;
    uint32_t payload_size = payload->length;
    uint8_t* encrypted = SOPC_Malloc(payload->length * sizeof(uint8_t));
    if (NULL == encrypted)
    {
        return NULL;
    }

    uint32_t encrypted_size;
    status = SOPC_CryptoProvider_SymmetricGetLength_Encryption(security->provider, payload_size, &encrypted_size);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(encrypted);
        return NULL;
    }

    uint32_t lengthMessageRandom;
    status = SOPC_CryptoProvider_PubSubGetLength_MessageRandom(security->provider, &lengthMessageRandom);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(encrypted);
        return NULL;
    }
    // Check with length in UADP  (OPC UA Spec Part 14)
    assert(SOPC_PUBSUB_SECURITY_RANDOM_LENGTH == lengthMessageRandom);
    status = SOPC_CryptoProvider_PubSubCrypt(security->provider, payload->data, payload->length,
                                             security->groupKeys->encryptKey, security->groupKeys->keyNonce,
                                             security->msgNonceRandom, lengthMessageRandom, security->sequenceNumber,
                                             encrypted, encrypted_size);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(encrypted);
        return NULL;
    }

    SOPC_Buffer* result = SOPC_Buffer_Attach(encrypted, encrypted_size);
    if (NULL == result)
    {
        SOPC_Free(encrypted);
        return NULL;
    }

    return result;
}

SOPC_Buffer* SOPC_PubSub_Security_Decrypt(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* src, uint32_t len)
{
    if (NULL == security || NULL == src || len == 0 || NULL == src->data || src->position + len > src->length)
    {
        return NULL;
    }

    // Create a buffer containing encrypted data
    SOPC_Buffer* buffer_encrypted = SOPC_Buffer_Create(len);
    if (NULL == buffer_encrypted)
    {
        return NULL;
    }
    SOPC_ReturnStatus result = SOPC_Buffer_Write(buffer_encrypted, &src->data[src->position], len);
    if (SOPC_STATUS_OK != result)
    {
        SOPC_Buffer_Delete(buffer_encrypted);
        return NULL;
    }

    // Decrypt is encrypt
    SOPC_Buffer* buffer_clear = SOPC_PubSub_Security_Encrypt(security, buffer_encrypted);

    SOPC_Buffer_Delete(buffer_encrypted);
    return buffer_clear;
}

// return size of signature. Size given in Octet
SOPC_ReturnStatus SOPC_PubSub_Security_GetSignSize(const SOPC_PubSub_SecurityType* security,
                                                   bool enabled,
                                                   uint32_t* length)
{
    if (NULL == security || NULL == length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (!enabled)
    {
        *length = 0;
    }
    else
    {
        status = SOPC_CryptoProvider_SymmetricGetLength_Signature(security->provider, length);
    }
    return status;
}
/* Sign from 0 to current position and add signature after current position */
SOPC_ReturnStatus SOPC_PubSub_Security_Sign(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* src)
{
    if (NULL == security || NULL == security->groupKeys || NULL == src)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    uint32_t length;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_SymmetricGetLength_Signature(security->provider, &length);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    uint8_t* signature = SOPC_Calloc(length, sizeof(uint8_t));
    if (NULL == signature)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_CryptoProvider_SymmetricSign(security->provider, src->data, src->position,
                                               security->groupKeys->signingKey, signature, length);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(src, signature, length);
    }
    SOPC_Free(signature);
    return status;
}

bool SOPC_PubSub_Security_Verify(const SOPC_PubSub_SecurityType* security, SOPC_Buffer* src, uint32_t payloadPosition)
{
    if (NULL == security || NULL == security->groupKeys || NULL == src || NULL == src->data)
    {
        return false;
    }

    uint32_t sizeSignature;
    SOPC_ReturnStatus status = SOPC_PubSub_Security_GetSignSize(security, true, &sizeSignature);
    if (SOPC_STATUS_OK != status || sizeSignature >= src->length)
    {
        return false;
    }
    // Sign from begining of buffer to begining of signature
    uint32_t lenPayload = (src->length - payloadPosition) - sizeSignature;
    status = SOPC_CryptoProvider_SymmetricVerify(security->provider, src->data + payloadPosition, lenPayload,
                                                 security->groupKeys->signingKey,
                                                 &src->data[payloadPosition + lenPayload], sizeSignature);
    return (SOPC_STATUS_OK == status);
}

void SOPC_PubSub_Security_Clear(SOPC_PubSub_SecurityType* security)
{
    if (NULL != security)
    {
        SOPC_LocalSKS_Keys_Delete(security->groupKeys);
        SOPC_Free(security->groupKeys);
        security->groupKeys = NULL;
        SOPC_Free(security->msgNonceRandom);
        security->msgNonceRandom = NULL;
        SOPC_CryptoProvider_Free(security->provider);
    }
}

SOPC_ExposedBuffer* SOPC_PubSub_Security_Random(const SOPC_CryptoProvider* provider)
{
    uint32_t length;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_PubSubGetLength_MessageRandom(provider, &length);
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }

    SOPC_ExposedBuffer* ppBuffer;
    status = SOPC_CryptoProvider_GenerateRandomBytes(provider, length, &ppBuffer);
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    return ppBuffer;
}
