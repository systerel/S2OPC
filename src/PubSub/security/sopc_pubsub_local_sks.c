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

#include "sopc_pubsub_local_sks.h"
#include "sopc_mem_alloc.h"

static struct
{
    const char* signing_key;
    const char* encrypt_key;
    const char* key_nonce;
} PUBSUB_LOCAL_SKS_KEYS_FILES = {.signing_key = NULL, .encrypt_key = NULL, .key_nonce = NULL};

void SOPC_LocalSKS_init(const char* pathSigningKey, const char* pathEncryptKey, const char* pathKeyNonce)
{
    PUBSUB_LOCAL_SKS_KEYS_FILES.signing_key = pathSigningKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.encrypt_key = pathEncryptKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.key_nonce = pathKeyNonce;
}

SOPC_LocalSKS_Keys* SOPC_LocalSKS_GetSecurityKeys(uint32_t groupid, uint32_t tokenId)
{
    if (SOPC_PUBSUB_SKS_DEFAULT_GROUPID != groupid)
    {
        return NULL;
    }
    if (SOPC_PUBSUB_SKS_DEFAULT_TOKENID != tokenId && SOPC_PUBSUB_SKS_CURRENT_TOKENID != tokenId)
    { // Token Id is not managed
        return NULL;
    }
    SOPC_LocalSKS_Keys* keys = SOPC_Calloc(1, sizeof(SOPC_LocalSKS_Keys));
    if (NULL == keys)
    {
        return NULL;
    }
    keys->tokenId = SOPC_PUBSUB_SKS_DEFAULT_TOKENID;
    keys->signingKey = SOPC_SecretBuffer_NewFromFile(PUBSUB_LOCAL_SKS_KEYS_FILES.signing_key);
    keys->encryptKey = SOPC_SecretBuffer_NewFromFile(PUBSUB_LOCAL_SKS_KEYS_FILES.encrypt_key);
    keys->keyNonce = SOPC_SecretBuffer_NewFromFile(PUBSUB_LOCAL_SKS_KEYS_FILES.key_nonce);
    if (NULL == keys->signingKey || NULL == keys->encryptKey || NULL == keys->keyNonce)
    {
        SOPC_LocalSKS_Keys_Delete(keys);
        SOPC_Free(keys);
        keys = NULL;
    }
    return keys;
}

void SOPC_LocalSKS_Keys_Delete(SOPC_LocalSKS_Keys* keys)
{
    if (NULL != keys)
    {
        SOPC_SecretBuffer_DeleteClear(keys->signingKey);
        keys->signingKey = NULL;
        SOPC_SecretBuffer_DeleteClear(keys->encryptKey);
        keys->encryptKey = NULL;
        SOPC_SecretBuffer_DeleteClear(keys->keyNonce);
        keys->keyNonce = NULL;
    }
}
