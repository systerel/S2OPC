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
 * \file
 * \brief Security Keys Builder: provides update function to retrieve keys from the Security Keys Provider
 *                               and fills the Security Keys Manager.
 *
 * \note Keys can be set or appended (with or without a maximum limit of elements)
 */

#ifndef SOPC_SK_BUILDER_H_
#define SOPC_SK_BUILDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"

#define SOPC_SK_BUILDER_NB_GENERATED_KEYS 5

typedef struct SOPC_SKBuilder SOPC_SKBuilder;

typedef SOPC_ReturnStatus (*SOPC_SKBuilder_Update_Func)(SOPC_SKBuilder* skb, SOPC_SKProvider* skp, SOPC_SKManager* skm);
typedef void (*SOPC_SKBuilder_Clear_Func)(void* data);

/**
 *  \brief Bytes Security Keys Builder structure
 *
 */
struct SOPC_SKBuilder
{
    SOPC_SKBuilder_Update_Func ptrUpdate; /* Function to update keys of a SK Manager  */
    SOPC_SKBuilder_Clear_Func ptrClear;   /* Function to clear data  */
    void* data;                           /* data bytes */
    uintptr_t referencesCounter;          /* Counter of references on this instance in SK scheduler(s) */
};

/**
 * \brief  Creates an instance of a default SOPC_SKBuilder which append data to the Security Keys Manager.
 *
 * \warning This builder only appends keys and never limits the number of keys stored.
 *          Thus it shall be used as inner builder of a builder that manages the maximum number of keys as
 *          ::SOPC_SKBuilder_Truncate_Create.
 *
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Append_Create(void);

/**
 * \brief  Creates an instance of a default SOPC_SKBuilder which deletes old keys
 *
 * \param skb     The builder used to retrieve the keys that will be truncated
 * \param sizeMax The maximum number of keys to keep in the security keys manager
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Truncate_Create(SOPC_SKBuilder* skb, uint32_t sizeMax);

/**
 * \brief  Creates an instance of a default SOPC_SKBuilder which replaces all the keys.
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Setter_Create(void);

/** \brief The type for the callback that shall be provided to create a builder ::SOPC_SKBuilder_Callback_Create */
typedef void (*SOPC_SKBuilder_Callback_Func)(SOPC_SKBuilder* skb, SOPC_SKManager* skm, uintptr_t userParam);

/**
 * \brief  Creates an instance of a default SOPC_SKBuilder which trigger a callback after an update.
 *
 * \param skb       The builder used to update the keys
 * \param callback  The function to notify an update of the keys in manager
 * \param userParam A user parameter that will be passed to the callback function
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Callback_Create(SOPC_SKBuilder* skb,
                                               SOPC_SKBuilder_Callback_Func callback,
                                               uintptr_t userParam);

/**
 *  \brief          Gets Keys from a Security Keys Provider and fill Security Keys Manager
 *
 *  \param skb      Pointer to Security Keys Builder. Should not be NULL
 *  \param skp      Pointer to Security Keys Provider. Should not be NULL
 *  \param skm      Pointer to Security Keys Manager. Should not be NULL
 *  \return         SOPC_STATUS_OK if keys are set
 */
SOPC_ReturnStatus SOPC_SKBuilder_Update(SOPC_SKBuilder* skb, SOPC_SKProvider* skp, SOPC_SKManager* skm);

/**
 *  \brief          Deallocates Security Keys Builder data bytes content
 *
 *  \param skb      Pointer to Security Keys Builder. Should not be NULL
 */
void SOPC_SKBuilder_Clear(SOPC_SKBuilder* skb);

/**
 *  \brief          Deletes the Security Keys Builder and its data bytes content
 *                  if no more references to this instance exist in schedulers.
 *
 *  \param skb      Pointer to Security Keys Builder pointer. Should not be NULL
 */
void SOPC_SKBuilder_MayDelete(SOPC_SKBuilder** skb);

#endif /* SOPC_SK_BUILDER_H_ */
