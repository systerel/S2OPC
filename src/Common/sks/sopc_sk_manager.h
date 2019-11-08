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
 *  \brief Security Keys Manager: manages local storage of the keys retrieved from the Security Keys Provider for the
 * SKS.
 *
 *  \note Keys can be set or appended to previous ones
 */

#ifndef SOPC_SK_MANAGER_H_
#define SOPC_SK_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"

#define SOPC_SK_MANAGER_CURRENT_TOKEN_ID 0
/* Default KeyLifetime 1 hour */
#define SOPC_SK_MANAGER_DEFAULT_KEYLIFETIME (60 * 60 * 1000)
#define SOPC_SK_MANAGER_DEFAULT_INITIAL_SIZE 10

typedef struct SOPC_SKManager SOPC_SKManager;

/**
 * \brief Type of functions to get size of the Security Key Manager.
 *
 */
typedef uint32_t (*SOPC_SKManager_GetSize_Func)(SOPC_SKManager* skm);

typedef SOPC_ReturnStatus (*SOPC_SKManager_SetKeyLifetime_Func)(SOPC_SKManager* skm, uint32_t KeyLifetime);

typedef uint32_t (*SOPC_SKManager_GetAllKeysLifeTime_Func)(SOPC_SKManager* skm);

typedef SOPC_ReturnStatus (*SOPC_SKManager_SetSecurityPolicyUri_Func)(SOPC_SKManager* skm,
                                                                      SOPC_String* SecurityPolicyUri);

typedef SOPC_ReturnStatus (*SOPC_SKManager_SetKeys_Func)(SOPC_SKManager* skm,
                                                         SOPC_String* SecurityPolicyUri,
                                                         uint32_t FirstTokenId,
                                                         SOPC_ByteString* Keys,
                                                         uint32_t NbKeys,
                                                         uint32_t TimeToNextKey,
                                                         uint32_t KeyLifetime);
typedef uint32_t (*SOPC_SKManager_AddKeys_Func)(SOPC_SKManager* skm, SOPC_ByteString* Keys, uint32_t NbToken);

typedef SOPC_ReturnStatus (*SOPC_SKManager_GetKeys_Func)(SOPC_SKManager* skm,
                                                         uint32_t StartingTokenId,
                                                         uint32_t NbRequestedToken,
                                                         SOPC_String** SecurityPolicyUri,
                                                         uint32_t* FirstTokenId,
                                                         SOPC_ByteString** Keys,
                                                         uint32_t* NbKeys,
                                                         uint32_t* TimeToNextKey,
                                                         uint32_t* KeyLifetime);
typedef void (*SOPC_SKManager_Clear_Func)(SOPC_SKManager* skm);

struct SOPC_SKManager
{
    SOPC_SKManager_GetSize_Func ptrSize;                   /* number of token */
    SOPC_SKManager_SetKeyLifetime_Func ptrSetKeyLifetime;  /** set  key lifetime  */
    SOPC_SKManager_SetSecurityPolicyUri_Func ptrSetPolicy; /** set SecurityPolicyUri  */
    SOPC_SKManager_SetKeys_Func ptrSetKeys;                /** set  keys */
    SOPC_SKManager_AddKeys_Func ptrAddKeys;                /** add a new keys */
    SOPC_SKManager_GetKeys_Func ptrGetKeys;                /** get  keys */
    SOPC_SKManager_GetAllKeysLifeTime_Func ptrGetAllKeysLifeTime;
    SOPC_SKManager_Clear_Func ptrClear; /** startingToken, nb of key  */
    void* data;                         /**< data bytes */
};

/**
 * \brief  Creates an instance of the default SOPC_SKManager
 *
 * \return a SOPC_SKManager object or NULL if not enough memory
 */
SOPC_SKManager* SOPC_SKManager_Create(void);

/**
 *  \brief          Gets number of managed Token for a given security group
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         The number of Token or 0 if bad parameters
 */
uint32_t SOPC_SKManager_Size(SOPC_SKManager* skm);

/**
 *  \brief              Sets the keys tokens lifetimes
 *
 *  \param skm          Pointer to Security Keys Manager. Should not be NULL
 *  \param KeyLifetime  The keys token lifetime before keys token becomes invalid in milliseconds
 *
 *  \return             SOPC_STATUS_OK if keys tokens lifetime set
 */
SOPC_ReturnStatus SOPC_SKManager_SetKeyLifetime(SOPC_SKManager* skm, uint32_t KeyLifetime);

/**
 *  \brief                    Sets the security policy URI for the security keys tokens
 *
 *  \param skm                Pointer to Security Keys Manager. Should not be NULL
 *  \param SecurityPolicyUri  The URI for the set of algorithms and key lengths used to secure the messages
 *  \return                   SOPC_STATUS_OK if security policy URI set
 */
SOPC_ReturnStatus SOPC_SKManager_SetSecurityPolicyUri(SOPC_SKManager* skm, SOPC_String* SecurityPolicyUri);

/**
 *  \brief          Sets Keys of a Security Keys Manager for a given security group.
 *                  After this function returns ( no matter the status ), old Keys are forgotten and cannot be accessed
 * anymore using this SKManager. All parameters are copied by this function.
 *
 *  \param skm                Pointer to Security Keys Manager. Should not be NULL
 *  \param SecurityPolicyUri  The URI for the set of algorithms and key lengths used to secure the messages
 *  \param FirstTokenId       The SecurityTokenId of the first key in the array of returned keys.
 *  \param Keys               An ordered list of keys that are used when the KeyLifetime elapses
 *  \param NbKeys             The number of keys tokens in \p Keys array
 *  \param TimeToNextKey      The time, in milliseconds, before the CurrentKey is expected to expire
 *  \param KeyLifetime        The lifetime of a key in milliseconds
 *  \return                   SOPC_STATUS_OK if keys are set
 */
SOPC_ReturnStatus SOPC_SKManager_SetKeys(SOPC_SKManager* skm,
                                         SOPC_String* SecurityPolicyUri,
                                         uint32_t FirstTokenId,
                                         SOPC_ByteString* Keys,
                                         uint32_t NbKeys,
                                         uint32_t TimeToNextKey,
                                         uint32_t KeyLifetime);

/**
 *  \brief          Adds Keys to a Security Keys Manager for a given security group.
 *                  New keys are appended to the end of the list of existing keys.
 *                  If no keys was managed before calling this function, the first token id is set to 1 and is
 * associated to the first new key. All parameters are copied by this function.
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \param Keys     The keys data
 *  \param NbToken  The number of keys token (set of keys) added
 *
 *  \return         number of added elements
 */
uint32_t SOPC_SKManager_AddKeys(SOPC_SKManager* skm, SOPC_ByteString* Keys, uint32_t NbToken);

/**
 *  \brief          Gets Keys of a Security Keys Manager for a given security group.
 *                  All returned data are copied by this function. The caller is responsible for deleting these data.
 *
 *  \param skm                Pointer to Security Keys Manager. Should not be NULL
 *  \param StartingTokenId    The current token is requested by passing 0. It can be a SecurityTokenId from the past to
 * get a key valid for previously sent messages
 *  \param NbRequestedToken   The number of requested keys tokens which should be returned in the response
 *  \param SecurityPolicyUri  The URI for the set of algorithms and key lengths used to secure the messages
 *  \param FirstTokenId       The SecurityTokenId of the first key in the array of returned keys.
 *  \param Keys               An ordered list of keys that are used when the KeyLifetime elapses
 *  \param NbKeys             The number of keys tokens in \p Keys array
 *  \param TimeToNextKey      The time, in milliseconds, before the CurrentKey is expected to expire
 *  \param KeyLifetime        The lifetime of a key in milliseconds
 *  \return                   SOPC_STATUS_OK if keys are get
 */
SOPC_ReturnStatus SOPC_SKManager_GetKeys(SOPC_SKManager* skm,
                                         uint32_t StartingTokenId,
                                         uint32_t NbRequestedToken,
                                         SOPC_String** SecurityPolicyUri,
                                         uint32_t* FirstTokenId,
                                         SOPC_ByteString** Keys,
                                         uint32_t* NbKeys,
                                         uint32_t* TimeToNextKey,
                                         uint32_t* KeyLifetime);

/**
 *  \brief          Returns the total remaining lifetime of available keys tokens.
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         Total remaining lifetime in milliseconds
 */
uint32_t SOPC_SKManager_GetAllKeysLifeTime(SOPC_SKManager* skm);

/**
 *  \brief          Deallocates Security Keys Manager data bytes content
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 */
void SOPC_SKManager_Clear(SOPC_SKManager* skm);

#endif /* SOPC_SK_MANAGER_H_ */
