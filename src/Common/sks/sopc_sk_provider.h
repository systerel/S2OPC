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
 *  \file
 *  \brief Security Keys Provider: source providing the keys for SKS
 *
 *  \note Keys might be generated locally in case of SKS server side
 *        or might be retrieved from an external source for a SKS "client".
 *        Only the local keys generation function is provided.
 */

#ifndef SOPC_SK_PROVIDER_H_
#define SOPC_SK_PROVIDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"

typedef struct SOPC_SKProvider SOPC_SKProvider;

typedef SOPC_ReturnStatus (*SOPC_SKProvider_GetKeys_Func)(SOPC_SKProvider* skp,
                                                          const char* securityGroupId,
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
 *  \brief Security Keys Provider structure
 *
 */
struct SOPC_SKProvider
{
    SOPC_SKProvider_GetKeys_Func ptrGetKeys; /** securityGroup, token */
    SOPC_SKProvider_Clear_Func ptrClear;     /** securityGroup, startingToken, nb of key  */
    uintptr_t data;                          /**< data bytes */
};

/**
 * \brief  Creates an instance of SOPC_SKProvider which call sequentially all provider of a list until one returns valid
 * Keys
 *
 * \param providers        A valid pointer of SOPC_SKProvider array. Should not be NULL
 * \param nbProviders      The number of element of the the given array. Should not be 0
 *
 * \return a SOPC_SKProvider object or NULL if not enough memory or invalid parameters
 */
SOPC_SKProvider* SOPC_SKProvider_TryList_Create(SOPC_SKProvider** providers, uint32_t nbProviders);

/**
 * \brief  Creates an instance of SOPC_SKProvider which return random Keys for PubSub Policy.
 *
 * \param   maxKeys  Maximum number of Keys returned by SOPC_SKProvider_GetKeys()
 * \return a SOPC_SKProvider object or NULL if not enough memory
 */
SOPC_SKProvider* SOPC_SKProvider_RandomPubSub_Create(uint32_t maxKeys);

/**
 *  \brief          Gets Keys of a Security Keys Provider for a given security group.
 *                  All returned data are copied by this function. The caller is responsible for deleting these data.
 *                  Output parameters may be NULL except Keys and NbKeys
 *
 *  \param skp                Pointer to Security Keys Provider. Input parameter. Should not be NULL
 *  \param securityGroupId    The Security Group Id for which the keys are requested
 *  \param StartingTokenId    The current token is requested by passing 0. It can be a SecurityTokenId from the past to
 * get a key valid for previously sent messages
 *  \param NbRequestedToken        The number of requested keys tokens which should be returned in the response
 *  \param[out] SecurityPolicyUri  The URI for the set of algorithms and key lengths used to secure the messages
 *  \param[out] FirstTokenId       The SecurityTokenId of the first key in the array of returned keys.
 *  \param[out] Keys               An ordered list of keys that are used when the KeyLifetime elapses
 *  \param[out] NbKeys             The number of keys tokens in \p Keys array
 *  \param[out] TimeToNextKey      The time, in milliseconds, before the CurrentKey is expected to expire
 *  \param[out] KeyLifetime        The lifetime of a key in milliseconds
 *  \return                        SOPC_STATUS_OK if keys are set
 */
SOPC_ReturnStatus SOPC_SKProvider_GetKeys(SOPC_SKProvider* skp,
                                          const char* securityGroupId,
                                          uint32_t StartingTokenId,
                                          uint32_t NbRequestedToken,
                                          SOPC_String** SecurityPolicyUri,
                                          uint32_t* FirstTokenId,
                                          SOPC_ByteString** Keys,
                                          uint32_t* NbKeys,
                                          uint32_t* TimeToNextKey,
                                          uint32_t* KeyLifetime);

/**
 *  \brief          Deallocates Security Keys Provider data bytes content
 *
 *  \param skp      Pointer to Security Keys Provider. Should not be NULL
 */
void SOPC_SKProvider_Clear(SOPC_SKProvider* skp);

#endif /* SOPC_SK_PROVIDER_H_ */
