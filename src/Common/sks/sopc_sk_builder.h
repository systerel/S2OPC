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
 *  \file sopc_sk_builder.h
 *
 *  \brief Object to build/fill a Security Keys Manager.
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
};

/**
 * \brief  Create an instance of a default SOPC_SKBuilder which append data to the Security Keys Manager.
 *
 * Request for Current Token
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Append_Create(void);

/**
 * \brief  Create an instance of a default SOPC_SKBuilder which delete old keys
 *
 * \param sizeMax The max use to trigger this builder
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Truncate_Create(SOPC_SKBuilder* skb, uint32_t sizeMax);

/**
 * \brief  Create an instance of a default SOPC_SKBuilder which call 2 builders
 *
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Sequence_Create(SOPC_SKBuilder* skb1, SOPC_SKBuilder* skb2);

/**
 * \brief  Create an instance of a default SOPC_SKBuilder which replace all the keys.
 *
 * \return a SOPC_SKBuilder object or NULL if not enough memory
 */
SOPC_SKBuilder* SOPC_SKBuilder_Setter_Create(void);

/**
 *  \brief          Get Keys from a Security Keys Provider and fill Security Keys Manager
 *
 *  \param skb      Pointer to Security Keys Builder. Should not be NULL
 *  \param skb      Pointer to Security Keys Provider. Should not be NULL
 *  \param skb      Pointer to Security Keys Manager. Should not be NULL
 *  \return         SOPC_STATUS_OK if keys are set otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKBuilder_Update(SOPC_SKBuilder* skb, SOPC_SKProvider* skp, SOPC_SKManager* skm);

/**
 *  \brief          Deallocate Security Keys Builder data bytes content
 *
 *  \param skb      Pointer to Security Keys Builder. Should not be NULL
 */
void SOPC_SKBuilder_Clear(SOPC_SKBuilder* skb);

#endif /* SOPC_SK_BUILDER_H_ */
