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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_scheduler.h"
#include "sopc_sub_target_variable.h"
#include "sopc_threads.h"
#include "sopc_xml_loader.h"

static int32_t stop = 0;
static int32_t stateChanged = 0;

static int returnCode = 0;
static int callIndex = 0;

#define PUB_ID 42
#define PUB_GROUP_ID 14
#define SECURITY_GROUP_ID "1"

static bool SOPC_SetTargetVariables_Test(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues)
{
    SOPC_ASSERT(NULL != nodesToWrite);

    if (!SOPC_Atomic_Int_Get(&stop))
    {
        callIndex++;
        if (callIndex == 1)
        {
            // Only one value in first DSM
            SOPC_ASSERT(1 == nbValues);

            const SOPC_Variant* variant = &(nodesToWrite[0].Value.Value);
            if (SOPC_Boolean_Id != variant->BuiltInTypeId)
            {
                return false;
            }
            if (SOPC_VariantArrayType_SingleValue != variant->ArrayType)
            {
                return false;
            }
            if (true != variant->Value.Boolean)
            {
                return false;
            }
        }
        else if (callIndex == 2)
        {
            // Two variables in second DSM
            SOPC_ASSERT(2 == nbValues);

            const SOPC_Variant* variant = &(nodesToWrite[0].Value.Value);
            if (SOPC_UInt32_Id != variant->BuiltInTypeId)
            {
                return false;
            }
            if (SOPC_VariantArrayType_SingleValue != variant->ArrayType)
            {
                return false;
            }
            if (0x12345678 != variant->Value.Uint32)
            {
                return false;
            }
            variant = &(nodesToWrite[1].Value.Value);
            if (SOPC_UInt16_Id != variant->BuiltInTypeId)
            {
                return false;
            }
            if (SOPC_VariantArrayType_SingleValue != variant->ArrayType)
            {
                return false;
            }
            if (17 != variant->Value.Uint16)
            {
                return false;
            }

            SOPC_Atomic_Int_Set(&stop, true);
        }

        if (returnCode == -1)
        {
            returnCode = 0;
        }
    }
    return true;
}

static void stateChangedCb(const SOPC_Conf_PublisherId* pubId,
                           uint16_t groupId,
                           uint16_t writerId,
                           SOPC_PubSubState state)
{
    SOPC_UNUSED_ARG(writerId);
    SOPC_UNUSED_ARG(groupId);
    if (pubId != NULL)
        return; // Only consider global changes. Not DSM changes.

    stateChanged++;
    printf("[sub]state changed to '%u' !\n", state);
    if (SOPC_Atomic_Int_Get(&stop))
    {
        if (SOPC_PubSubState_Disabled != state)
        {
            returnCode = -2;
        }
    }
    else
    {
        if (SOPC_PubSubState_Operational != state)
        {
            returnCode = -3;
        }
    }
}

// Callback for signature check failure
static void subSignatureCheckFailed(const SOPC_ReaderGroup* group, const char* securityGroupId)
{
    // Check parameters are the one expected
    const SOPC_Conf_PublisherId* pubId = SOPC_ReaderGroup_Get_PublisherId(group);
    if (SOPC_UInteger_PublisherId == pubId->type && PUB_ID == pubId->data.uint &&
        PUB_GROUP_ID == SOPC_ReaderGroup_Get_GroupId(group) && securityGroupId != NULL &&
        strcmp(securityGroupId, SECURITY_GROUP_ID) == 0)
    {
        exit(125);
    }
    else
    {
        exit(142);
    }
}

static void fillSkManagerWithUnexpectedKeys(SOPC_SKManager* skm)
{
    SOPC_SKProvider* skp = SOPC_SKProvider_RandomPubSub_Create(5);

    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;
    SOPC_ReturnStatus status =
        SOPC_SKProvider_GetKeys(skp, skm->securityGroupId, SOPC_SK_MANAGER_CURRENT_TOKEN_ID, UINT32_MAX,
                                &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);

    if (SOPC_STATUS_OK == status && NULL != Keys && 0 < NbToken)
    {
        FirstTokenId = 1;
        TimeToNextKey = 5;
        KeyLifetime = 5;
        SOPC_String SecurityPolicyUri1;
        SOPC_String_Initialize(&SecurityPolicyUri1);
        status = SOPC_String_AttachFromCstring(&SecurityPolicyUri1, SOPC_SecurityPolicy_PubSub_Aes256_URI);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        status =
            SOPC_SKManager_SetKeys(skm, &SecurityPolicyUri1, FirstTokenId, Keys, NbToken, TimeToNextKey, KeyLifetime);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    else
    {
        SOPC_ASSERT(false);
    }

    for (uint32_t i = 0; i < NbToken; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);
    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);

    SOPC_SKProvider_MayDelete(&skp);
}

int main(int argc, char** argv)
{
    bool testWithUnexpectedKeys = false;

    int sleepCount = 20;

    char* filename;
    if (1 < argc)
    {
        filename = argv[1];
        if (2 < argc)
        {
            testWithUnexpectedKeys = strcmp(argv[2], "true") == 0;
        }
    }
    else
    {
        filename = "./config_sub.xml";
    }

    FILE* fd = fopen(filename, "r");
    SOPC_ASSERT(NULL != fd);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_ParseXML(fd);
    int closed = fclose(fd);
    SOPC_ASSERT(0 == closed);

    // Register signature check failed callback for each connection
    for (uint32_t i = 0; i < SOPC_PubSubConfiguration_Nb_SubConnection(config); i++)
    {
        SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, i);
        SOPC_PubSubConfiguration_Set_SubSignatureCheckFailed_Callback(connection, &subSignatureCheckFailed);
    }

    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(&SOPC_SetTargetVariables_Test);

    SOPC_PubSubSKS_Init();

    SOPC_SKManager* skm = SOPC_SKManager_Create(SECURITY_GROUP_ID, 0);
    SOPC_ASSERT(NULL != skm);
    bool res = SOPC_PubSubSKS_AddSkManager(skm);
    SOPC_ASSERT(res);

    if (testWithUnexpectedKeys)
    {
        printf("[sub] test with wrong keys for checking signature\n");
        fillSkManagerWithUnexpectedKeys(skm);
    }

    bool started = SOPC_SubScheduler_Start(config, targetConfig, stateChangedCb, NULL, NULL, 0);

    while (true == started && false == SOPC_Atomic_Int_Get(&stop) && sleepCount > 0)
    {
        SOPC_Sleep(100);
        sleepCount--;
    }

    SOPC_SubScheduler_Stop();

    SOPC_PubSubSKS_Clear();

    if (false == SOPC_Atomic_Int_Get(&stop))
    {
        returnCode = -1;
    }
    else if (2 != stateChanged)
    {
        // We expect 2 state changes
        returnCode = 1;
    }

    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);

    return returnCode;
}
