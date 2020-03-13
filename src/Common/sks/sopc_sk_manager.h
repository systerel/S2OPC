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
 *  \file sopc_sk_manager.h
 *
 *  \brief A buffer of bytes with a maximum size, length and position.
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
                                                         SOPC_String** SecurityPolicyUri,
                                                         uint32_t* FirstTokenId,
                                                         SOPC_ByteString** Keys,
                                                         uint32_t* NbKeys,
                                                         uint32_t* TimeToNextKey,
                                                         uint32_t* KeyLifetime);
typedef void (*SOPC_SKManager_Clear_Func)(SOPC_SKManager* skm);

/**
 *  \brief Bytes buffer structure
 */
struct SOPC_SKManager
{
    SOPC_SKManager_GetSize_Func ptrSize;                   /* number of token */
    SOPC_SKManager_SetKeyLifetime_Func ptrSetKeyLifetime;  /** set  key lifetime  */
    SOPC_SKManager_SetSecurityPolicyUri_Func ptrSetPolicy; /** set SecurityPolicyUri  */
    SOPC_SKManager_SetKeys_Func ptrSetKeys;                /** add a new keys */
    SOPC_SKManager_AddKeys_Func ptrAddKeys;                /** get  keys */
    SOPC_SKManager_GetKeys_Func ptrGetKeys;                /** token */
    SOPC_SKManager_GetAllKeysLifeTime_Func ptrGetAllKeysLifeTime;
    SOPC_SKManager_Clear_Func ptrClear; /** startingToken, nb of key  */
    // uint32_t ptrCurrentToken; /* current token for a given group */
    void* data; /**< data bytes */
};

/**
 * \brief  Create an instance of the default SOPC_SKManager
 *
 * \return a SOPC_SKManager object or NULL if not enough memory
 */
SOPC_SKManager* SOPC_SKManager_Create(void);

/**
 *  \brief          Get number of managed Token for a given security group
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         The number of Token or 0 if bad parameters
 */
uint32_t SOPC_SKManager_Size(SOPC_SKManager* skm);

SOPC_ReturnStatus SOPC_SKManager_SetKeyLifetime(SOPC_SKManager* skm, uint32_t KeyLifetime);

SOPC_ReturnStatus SOPC_SKManager_SetSecurityPolicyUri(SOPC_SKManager* skm, SOPC_String* SecurityPolicyUri);

/**
 *  \brief          Set Keys of a Security Keys Manager for a given security group.
 *                  After this function returns ( no matter the status ), old Keys are forgotten and cannot be accessed
 * anymore using this SKManager. All parameters are copied by this function.
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         SOPC_STATUS_OK if keys are set otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKManager_SetKeys(SOPC_SKManager* skm,
                                         SOPC_String* SecurityPolicyUri,
                                         uint32_t FirstTokenId,
                                         SOPC_ByteString* Keys,
                                         uint32_t NbKeys,
                                         uint32_t TimeToNextKey,
                                         uint32_t KeyLifetime);

/**
 *  \brief          Add Keys to a Security Keys Manager for a given security group.
 *                  New keys are append to the end of the list of existing keys.
 *                  If no keys was managed before calling this function, the first token id is set to 1 and is
 * associated to the first new key. All parameters are copied by this function.
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         number of added elements
 */
uint32_t SOPC_SKManager_AddKeys(SOPC_SKManager* skm, SOPC_ByteString* Keys, uint32_t NbToken);

/** TODO add nb requested keys
 *  \brief          Get Keys of a Security Keys Manager for a given security group.
 *                  All returned data are copied by this function. The caller is reponsible for deleting these data.
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         SOPC_STATUS_OK if keys are set otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKManager_GetKeys(SOPC_SKManager* skm,
                                         uint32_t StartingTokenId,
                                         SOPC_String** SecurityPolicyUri,
                                         uint32_t* FirstTokenId,
                                         SOPC_ByteString** Keys,
                                         uint32_t* NbKeys,
                                         uint32_t* TimeToNextKey,
                                         uint32_t* KeyLifetime);

uint32_t SOPC_SKManager_GetAllKeysLifeTime(SOPC_SKManager* skm);

/**
 *  \brief          Deallocate Security Keys Manager data bytes content
 *
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 */
void SOPC_SKManager_Clear(SOPC_SKManager* skm);

#endif /* SOPC_SK_MANAGER_H_ */
