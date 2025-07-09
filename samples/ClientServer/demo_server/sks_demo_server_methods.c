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

#include "sks_demo_server_methods.h"

#include <stdio.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_manager.h"

#include "sopc_sk_secu_group_managers.h"

/*---------------------------------------------------------------------------
 *                 SKS Demo Methods for Call service definition
 *---------------------------------------------------------------------------*/

#define GETSECURITYKEYS_EXPECTED_NB_INPUT_ARGS 3

/*
 * Implementation of GetSecurityKeys method for PubSub Security Key Service
 */
SOPC_StatusCode SOPC_Method_Func_PublishSubscribe_GetSecurityKeys(const SOPC_CallContext* callContextPtr,
                                                                  const SOPC_NodeId* objectId,
                                                                  uint32_t nbInputArgs,
                                                                  const SOPC_Variant* inputArgs,
                                                                  uint32_t* nbOutputArgs,
                                                                  SOPC_Variant** outputArgs,
                                                                  void* param)
{
    SOPC_UNUSED_ARG(objectId); /* Should be OpcUaId_PublishSubscribe */
    SOPC_UNUSED_ARG(param);
    /* Check Call Context */

    printf("Enter method GetSecurityKeys with param SecurityGroupId = %s\n",
           SOPC_String_GetRawCString(&inputArgs[0].Value.String));

    // SecureChannel shall use encryption to keep the provided keys secret
    OpcUa_MessageSecurityMode msm = SOPC_CallContext_GetSecurityMode(callContextPtr);
    if (OpcUa_MessageSecurityMode_SignAndEncrypt != msm)
    {
        return OpcUa_BadSecurityModeInsufficient;
    }

    // User shall be authorized to call the GetSecurityKeys method
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    /* Check if the user is authorized to call the method for this Security Group
     *
     * IMPORTANT NOTE: this shall now be enforced using RolePermissions in the address space
     *                 by obtaining SecurityAdmin role on session activation
     *                 (see s2opc_base_sks_origin.xml)
     */
    if (SOPC_User_IsUsername(user))
    {
        const SOPC_String* username = SOPC_User_GetUsername(user);
        if (0 != strcmp("secuAdmin", SOPC_String_GetRawCString(username)))
        {
            /* Only secuAdmin is allowed to call getSecurityKeys() */
            return OpcUa_BadUserAccessDenied;
        }
    }
    else // TODO: accept particular X509 thumbprint
    {
        return OpcUa_BadUserAccessDenied;
    }

    /* Check Input Object */

    if (GETSECURITYKEYS_EXPECTED_NB_INPUT_ARGS != nbInputArgs || NULL == inputArgs)
    {
        /* Should not happen if method is well defined in address space */
        return OpcUa_BadInvalidArgument;
    }

    /* Check Security Group argument */
    if (SOPC_String_Id != inputArgs[0].BuiltInTypeId || SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Check requestedStartingTokenId input argument type is an UInt32 single value
    if (SOPC_UInt32_Id != inputArgs[1].BuiltInTypeId || SOPC_VariantArrayType_SingleValue != inputArgs[1].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }
    uint32_t requestedStartingTokenId = inputArgs[1].Value.Uint32;

    // Check requestedNbKeys input argument type is an UInt32 single value
    if (SOPC_UInt32_Id != inputArgs[2].BuiltInTypeId || SOPC_VariantArrayType_SingleValue != inputArgs[2].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }
    uint32_t requestedNbKeys = inputArgs[2].Value.Uint32;

    *nbOutputArgs = 5;
    *outputArgs = SOPC_Calloc(5, sizeof(SOPC_Variant));
    if (NULL == *outputArgs)
    {
        return OpcUa_BadOutOfMemory;
    }

    SOPC_Variant* variant;
    for (uint32_t i = 0; i < 5; i++)
    {
        variant = &((*outputArgs)[i]);
        SOPC_Variant_Initialize(variant);
    }

    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;

    SOPC_SKManager* skm = SOPC_SK_SecurityGroup_GetSkManager(SOPC_String_GetRawCString(&inputArgs[0].Value.String));
    SOPC_ReturnStatus status =
        SOPC_SKManager_GetKeys(skm, requestedStartingTokenId, requestedNbKeys, &SecurityPolicyUri, &FirstTokenId, &Keys,
                               &NbToken, &TimeToNextKey, &KeyLifetime);
    bool keysValid = (NULL != Keys && 0 < NbToken && INT32_MAX >= NbToken);
    if (SOPC_STATUS_OK != status || !keysValid)
    {
        printf("<Security Key Service: Error in SK Manager when get keys\n");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (0 == FirstTokenId)
        {
            printf("<Security Key Service Error: First Token id is not valid\n");
        }
        if (0 == TimeToNextKey)
        {
            printf("<Security Key Service Error: TimeToNextKey is not valid\n");
        }
        if (0 == KeyLifetime)
        {
            printf("<Security Key Service Error: KeyLifetime is not valid\n");
        }

        if (SOPC_STATUS_OK == status)
        {
            /* SecurityPolicyUri */
            variant = &((*outputArgs)[0]);
            variant->BuiltInTypeId = SOPC_String_Id;
            variant->ArrayType = SOPC_VariantArrayType_SingleValue;
            status = SOPC_String_Copy(&variant->Value.String, SecurityPolicyUri);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* FirstTokenId */
        variant = &((*outputArgs)[1]);
        variant->BuiltInTypeId = SOPC_UInt32_Id; /* IntegerId */
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Uint32 = FirstTokenId;

        /* Keys */
        variant = &((*outputArgs)[2]);
        variant->BuiltInTypeId = SOPC_ByteString_Id;
        variant->ArrayType = SOPC_VariantArrayType_Array;
        // SigningKey + EncryptingKey + KeyNonce
        variant->Value.Array.Content.BstringArr = SOPC_Calloc(NbToken, sizeof(SOPC_ByteString));
        if (NULL == variant->Value.Array.Content.BstringArr)
        {
            SOPC_Free(*outputArgs);
            return OpcUa_BadOutOfMemory;
        }
        else
        {
            for (uint32_t i = 0; i < NbToken && SOPC_STATUS_OK == status; i++)
            {
                SOPC_ByteString_Clear(&variant->Value.Array.Content.BstringArr[i]);
                status = SOPC_ByteString_Copy(&variant->Value.Array.Content.BstringArr[i], &Keys[i]);
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            variant->Value.Array.Length = (int32_t) NbToken;
        }
        else
        {
            printf("<Security Key Service Error: Cannot save Keys\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* TimeToNextKey */
        variant = &((*outputArgs)[3]);
        variant->BuiltInTypeId = SOPC_Double_Id; /* Duration */
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Doublev = (double) TimeToNextKey;

        /* KeyLifetime */
        variant = &((*outputArgs)[4]);
        variant->BuiltInTypeId = SOPC_Double_Id; /* Duration */
        variant->ArrayType = SOPC_VariantArrayType_SingleValue;
        variant->Value.Doublev = (double) KeyLifetime;
    }
    else
    {
        for (uint32_t i = 0; i < 5; i++)
        {
            variant = &((*outputArgs)[i]);
            SOPC_Variant_Clear(variant);
        }
        SOPC_Free(*outputArgs);
        *outputArgs = NULL;
        *nbOutputArgs = 0;
    }

    /* Delete Keys */
    for (uint32_t i = 0; i < NbToken; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);
    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);

    if (SOPC_STATUS_OK == status)
    {
        return SOPC_GoodGenericStatus;
    }
    else
    {
        return OpcUa_BadInternalError;
    }
}
