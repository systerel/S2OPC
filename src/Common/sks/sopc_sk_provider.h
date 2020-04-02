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

/**
 *  \file sopc_sk_provider.h
 *
 *  \brief A buffer of bytes with a maximum size, length and position.
 */

#ifndef SOPC_SK_PROVIDER_H_
#define SOPC_SK_PROVIDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"

typedef struct SOPC_SKProvider SOPC_SKProvider;

typedef SOPC_ReturnStatus (*SOPC_SKProvider_GetKeys_Func)(SOPC_SKProvider* skp,
                                                          uint32_t StartingTokenId,
                                                          uint32_t NbRequestedToken,
                                                          SOPC_String** SecurityPolicyUri,
                                                          uint32_t* FirstTokenId,
                                                          SOPC_ByteString** Keys,
                                                          uint32_t* NbKeys,
                                                          uint32_t* TimeToNextKey,
                                                          uint32_t* KeyLifetime);
typedef void (*SOPC_SKProvider_Clear_Func)(void* data);

/**
 *  \brief Bytes Security Keys Provider structure
 *
 */
struct SOPC_SKProvider
{
    SOPC_SKProvider_GetKeys_Func
        ptrGetKeys; /** securityGroup, token ( ne renvoit q'un jeu de clé. Faire une version avec plusieurs clé ? )*/
    SOPC_SKProvider_Clear_Func ptrClear; /** securityGroup, startingToken, nb of key  */
    void* data;                          /**< data bytes */
};

/**
 * \brief  Create an instance of SOPC_SKProvider which call sequentially all provider of a list until one returns valid
 * Keys
 *
 * \param providers       A valid pointer of SOPC_SKProvider array. Should not be NULL
 * \param nbProviders     The number of element of the the given array. Should not be 0
 * \return a SOPC_SKProvider object or NULL if not enough memory
 */
SOPC_SKProvider* SOPC_SKProvider_TryList_Create(SOPC_SKProvider** providers, uint32_t nbProviders);

/**
 * \brief  Create an instance of SOPC_SKProvider which return random Keys for PubSub Policy.
 *
 * \param   maxKeys  Maximum number of Keys returned by SOPC_SKProvider_GetKeys()
 * \return a SOPC_SKProvider object or NULL if not enough memory
 */
SOPC_SKProvider* SOPC_SKProvider_RandomPubSub_Create(uint32_t maxKeys);

/**
 *  \brief          Get Keys of a Security Keys Provider for a given security group.
 *                  All returned data are copied by this function. The caller is reponsible for deleting these data.
 *                  Output parameters may be NULL exept Keys and NbKeys
 *
 *  \param skp      Pointer to Security Keys Provider. Input parameter. Should not be NULL
 *  \return         SOPC_STATUS_OK if keys are set otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKProvider_GetKeys(SOPC_SKProvider* skp,
                                          uint32_t StartingTokenId,
                                          uint32_t NbRequestedToken,
                                          SOPC_String** SecurityPolicyUri,
                                          uint32_t* FirstTokenId,
                                          SOPC_ByteString** Keys,
                                          uint32_t* NbKeys,
                                          uint32_t* TimeToNextKey,
                                          uint32_t* KeyLifetime);

/**
 *  \brief          Deallocate Security Keys Provider data bytes content
 *
 *  \param skp      Pointer to Security Keys Provider. Should not be NULL
 */
void SOPC_SKProvider_Clear(SOPC_SKProvider* skp);

#endif /* SOPC_SK_PROVIDER_H_ */
