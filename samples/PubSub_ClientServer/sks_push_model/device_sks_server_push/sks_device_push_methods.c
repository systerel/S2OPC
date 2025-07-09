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

#include "sks_device_push_methods.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_provider.h"
#include "sopc_sk_scheduler.h"

#include "sopc_pubsub_sks.h"

#include "device_config.h"

/*---------------------------------------------------------------------------
 *                 SKS Demo Methods for Call service definition
 *---------------------------------------------------------------------------*/

#define SETSECURITYKEYS_EXPECTED_NB_INPUT_ARGS 7

SOPC_StatusCode SOPC_Method_Func_PublishSubscribe_SetSecurityKeys(const SOPC_CallContext* callContextPtr,
                                                                  const SOPC_NodeId* objectId,
                                                                  uint32_t nbInputArgs,
                                                                  const SOPC_Variant* inputArgs,
                                                                  uint32_t* nbOutputArgs,
                                                                  SOPC_Variant** outputArgs,
                                                                  void* param)
{
    SOPC_UNUSED_ARG(objectId); /* Should be OpcUaId_PublishSubscribe */
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(param);

    /* Check Call Context */

    printf("Enter method SetSecurityKeys with param SecurityGroupId = %s\n",
           SOPC_String_GetRawCString(&inputArgs[0].Value.String));

    // SecureChannel shall use encryption to keep the provided keys secret, refuse possibly compromised keys update
    OpcUa_MessageSecurityMode msm = SOPC_CallContext_GetSecurityMode(callContextPtr);
    if (OpcUa_MessageSecurityMode_SignAndEncrypt != msm)
    {
        return OpcUa_BadSecurityModeInsufficient;
    }

    // User shall be authorized to call the SetSecurityKeys method
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    /* Check if the user is authorized to call the method for this Security Group
     *
     * IMPORTANT NOTE: this shall now be enforced using RolePermissions in the address space
     *                 by obtaining SecurityAdmin role on session activation
     *                 (see s2opc_base_sks_push_origin.xml)
     */
    if (SOPC_User_IsUsername(user))
    {
        const SOPC_String* username = SOPC_User_GetUsername(user);
        if (0 != strcmp("secuAdmin", SOPC_String_GetRawCString(username)))
        {
            // Only secuAdmin is allowed to call SetSecurityKeys()
            return OpcUa_BadUserAccessDenied;
        }
    }
    else
    {
        return OpcUa_BadUserAccessDenied;
    }

    /* Check Input Object */

    if (SETSECURITYKEYS_EXPECTED_NB_INPUT_ARGS != nbInputArgs || NULL == inputArgs)
    {
        /* Should not happen if method is well defined in address space */
        return OpcUa_BadInvalidArgument;
    }

    const SOPC_Variant* arguments = inputArgs;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Extract arguments for push operation
    const SOPC_Variant* securityGroupIdVariant = &arguments[0];
    const SOPC_Variant* securityPolicyUriVariant = &arguments[1];
    const SOPC_Variant* currentTokenIdVariant = &arguments[2];
    const SOPC_Variant* currentKeyVariant = &arguments[3];
    const SOPC_Variant* futureKeysVariant = &arguments[4];
    const SOPC_Variant* timeToNextKeyVariant = &arguments[5];
    const SOPC_Variant* keyLifetimeVariant = &arguments[6];

    /*
    // Security Group id
    printf("Security Group Id: ");
    SOPC_Variant_Print(securityGroupIdVariant);

    // Security Policy URI
    printf("Security Policy URI: ");
    SOPC_Variant_Print(securityPolicyUriVariant);

    // Current Token Id
    printf("Current Token Id: ");
    SOPC_Variant_Print(currentTokenIdVariant);

    // Current Key
    printf("Current Key: ");
    SOPC_Variant_Print(currentKeyVariant);

    // Future Keys
    printf("Future Keys: ");
    SOPC_Variant_Print(futureKeysVariant);

    // Time to Next Key
    printf("Time to Next Key: ");
    SOPC_Variant_Print(timeToNextKeyVariant);

    // Key Lifetime
    printf("Key Lifetime: ");
    SOPC_Variant_Print(keyLifetimeVariant);
    */

    // Security Group id
    if (securityGroupIdVariant->Value.String.Length <= 0)
    {
        return OpcUa_BadInvalidArgument;
    }
    const char* securityGroupId = SOPC_String_GetRawCString(&securityGroupIdVariant->Value.String);

    SOPC_SKManager* skManager = SOPC_PubSubSKS_GetSkManager(securityGroupId);
    if (NULL == skManager)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Prepare local variables for push operation
    const SOPC_String* securityPolicyUri = &securityPolicyUriVariant->Value.String;
    uint32_t startingTokenId = currentTokenIdVariant->Value.Uint32;
    uint32_t timeToNextKey = (uint32_t) timeToNextKeyVariant->Value.Doublev;
    uint32_t keyLifetime = (uint32_t) keyLifetimeVariant->Value.Doublev;

    // Calculate total number of keys (CurrentKey + FutureKeys)
    uint32_t nbFutureKeys =
        (uint32_t)(futureKeysVariant->Value.Array.Length >= 0 ? futureKeysVariant->Value.Array.Length : 0);
    uint32_t totalNbKeys = 1 + nbFutureKeys; // CurrentKey + FutureKeys

    // Allocate array for all keys (CurrentKey + FutureKeys)
    SOPC_ByteString* allKeys = SOPC_Calloc(totalNbKeys, sizeof(SOPC_ByteString));
    if (NULL == allKeys)
    {
        return OpcUa_BadOutOfMemory;
    }

    // Initialize all keys
    for (uint32_t i = 0; i < totalNbKeys; i++)
    {
        SOPC_ByteString_Initialize(&allKeys[i]);
    }

    // Copy CurrentKey as first key
    status = SOPC_ByteString_Copy(&allKeys[0], &currentKeyVariant->Value.Bstring);

    // Copy FutureKeys
    if (SOPC_STATUS_OK == status && nbFutureKeys > 0)
    {
        for (uint32_t i = 0; i < nbFutureKeys && SOPC_STATUS_OK == status; i++)
        {
            status = SOPC_ByteString_Copy(&allKeys[i + 1], &futureKeysVariant->Value.Array.Content.BstringArr[i]);
        }
    }

    // Push keys to the SK provider
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKManager_SetKeys(skManager, securityPolicyUri, startingTokenId, allKeys, totalNbKeys,
                                        timeToNextKey, keyLifetime);
        printf("SetSecurityKeys: SetKeys in manager with CurrentTokenId=%" PRIu32 " CurrentKeyByte=%" PRIu8 " %s\n",
               startingTokenId, allKeys[0].Data[0], (SOPC_STATUS_OK == status ? "SUCCEEDED" : "FAILED"));
    }

    // Clean up allocated keys
    for (uint32_t i = 0; i < totalNbKeys; i++)
    {
        SOPC_ByteString_Clear(&allKeys[i]);
    }
    SOPC_Free(allKeys);

    if (SOPC_STATUS_OK == status)
    {
        return SOPC_GoodGenericStatus;
    }
    else
    {
        return OpcUa_BadInternalError;
    }
}
