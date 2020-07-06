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

#include <stdbool.h>
#include <stdio.h>

#include "sopc_mem_alloc.h"
#include "sopc_pubsub_local_sks.h"

static struct
{
    const char* signing_key;
    const char* encrypt_key;
    const char* key_nonce;

    // Used if configuration static is used.
    // In that case, previous path will be not used
    // by SOPC_LocalSKS_GetSecurityKeys
    bool bUseStaticConfig;
    const uint8_t* sSigningKey;
    uint32_t uwSigningKeySize;
    const uint8_t* sEncryptKey;
    uint32_t uwEncryptKeySize;
    const uint8_t* sKeyNonce;
    uint32_t uwKeyNonceSize;
} PUBSUB_LOCAL_SKS_KEYS_FILES = //
    {
        //
        .signing_key = NULL,       //
        .encrypt_key = NULL,       //
        .key_nonce = NULL,         //
        .bUseStaticConfig = false, //
        .uwSigningKeySize = 0,     //
        .uwEncryptKeySize = 0,     //
        .uwKeyNonceSize = 0,       //
        .sSigningKey = NULL,       //
        .sEncryptKey = NULL,       //
        .sKeyNonce = NULL          //
};                                 //

void SOPC_LocalSKS_init(const char* pathSigningKey, const char* pathEncryptKey, const char* pathKeyNonce)
{
    // SOPC_LocalSKS_GetSecurityKeys will get key from files
    PUBSUB_LOCAL_SKS_KEYS_FILES.signing_key = pathSigningKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.encrypt_key = pathEncryptKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.key_nonce = pathKeyNonce;
    PUBSUB_LOCAL_SKS_KEYS_FILES.bUseStaticConfig = false;
    PUBSUB_LOCAL_SKS_KEYS_FILES.sSigningKey = NULL;
    PUBSUB_LOCAL_SKS_KEYS_FILES.sEncryptKey = NULL;
    PUBSUB_LOCAL_SKS_KEYS_FILES.sKeyNonce = NULL;
    PUBSUB_LOCAL_SKS_KEYS_FILES.uwSigningKeySize = 0;
    PUBSUB_LOCAL_SKS_KEYS_FILES.uwEncryptKeySize = 0;
    PUBSUB_LOCAL_SKS_KEYS_FILES.uwKeyNonceSize = 0;
}

void SOPC_LocalSKS_init_static(const uint8_t* signingKey, //
                               uint32_t lenSigningKey,    //
                               const uint8_t* encryptKey, //
                               uint32_t lenEncryptKey,    //
                               const uint8_t* keyNonce,   //
                               uint32_t lenKeyNonce)      //
{
    // SOPC_LocalSKS_GetSecurityKeys will get key from constant uint8_t buffer
    PUBSUB_LOCAL_SKS_KEYS_FILES.signing_key = NULL;
    PUBSUB_LOCAL_SKS_KEYS_FILES.encrypt_key = NULL;
    PUBSUB_LOCAL_SKS_KEYS_FILES.key_nonce = NULL;
    PUBSUB_LOCAL_SKS_KEYS_FILES.bUseStaticConfig = true;
    PUBSUB_LOCAL_SKS_KEYS_FILES.sSigningKey = signingKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.sEncryptKey = encryptKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.sKeyNonce = keyNonce;
    PUBSUB_LOCAL_SKS_KEYS_FILES.uwSigningKeySize = lenSigningKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.uwEncryptKeySize = lenEncryptKey;
    PUBSUB_LOCAL_SKS_KEYS_FILES.uwKeyNonceSize = lenKeyNonce;
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

    if (PUBSUB_LOCAL_SKS_KEYS_FILES.bUseStaticConfig)
    {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
        printf("# Load security signing key from static buffers... slen = %u\n", //
               PUBSUB_LOCAL_SKS_KEYS_FILES.uwSigningKeySize);                    //

        keys->signingKey =
            SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) PUBSUB_LOCAL_SKS_KEYS_FILES.sSigningKey, //
                                                   PUBSUB_LOCAL_SKS_KEYS_FILES.uwSigningKeySize);                 //

        printf("# Load security encrypt key from static buffers... slen = %u\n", //
               PUBSUB_LOCAL_SKS_KEYS_FILES.uwEncryptKeySize);                    //

        keys->encryptKey =
            SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) PUBSUB_LOCAL_SKS_KEYS_FILES.sEncryptKey, //
                                                   PUBSUB_LOCAL_SKS_KEYS_FILES.uwEncryptKeySize);                 //

        printf("# Load security nonce key from static buffers... slen = %u\n", //
               PUBSUB_LOCAL_SKS_KEYS_FILES.uwKeyNonceSize);                    //

        keys->keyNonce =
            SOPC_SecretBuffer_NewFromExposedBuffer((SOPC_ExposedBuffer*) PUBSUB_LOCAL_SKS_KEYS_FILES.sKeyNonce, //
                                                   PUBSUB_LOCAL_SKS_KEYS_FILES.uwKeyNonceSize);                 //
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    }
    else
    {
        keys->signingKey = SOPC_SecretBuffer_NewFromFile(PUBSUB_LOCAL_SKS_KEYS_FILES.signing_key);
        keys->encryptKey = SOPC_SecretBuffer_NewFromFile(PUBSUB_LOCAL_SKS_KEYS_FILES.encrypt_key);
        keys->keyNonce = SOPC_SecretBuffer_NewFromFile(PUBSUB_LOCAL_SKS_KEYS_FILES.key_nonce);
    }

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
