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

#include <stdio.h>
#include <stdlib.h>

#include <vm.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_time.h"

#include "../unit_test/unit_test_include.h"
#include "pubsub_config_static.h"
#include "static_security_data.h"

#define NB_VARS 8

SOPC_Variant varArr[NB_VARS] = {
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 12071982}},    // 0
    {true, SOPC_Byte_Id, SOPC_VariantArrayType_SingleValue, {.Byte = 239}},             // 1
    {true, SOPC_Int16_Id, SOPC_VariantArrayType_SingleValue, {.Int16 = 5462}},          // 2
    {true, SOPC_Float_Id, SOPC_VariantArrayType_SingleValue, {.Floatv = (float) 0.12}}, // 3
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 369852}},      // 4
    {true, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = true}},      // 5
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 0x12345678}},  // 6
    {true, SOPC_UInt16_Id, SOPC_VariantArrayType_SingleValue, {.Uint16 = 17}}           // 7
};

static int32_t gStopAtomic = 0;

/*---------------------------------------------------------------------------
 *                            Common utililities
 *---------------------------------------------------------------------------*/

static void log_UserCallback(const char* context, const char* text)
{
    SOPC_UNUSED_ARG(context);
    if (NULL != text)
    {
        vm_cprintf("%s\r\n", text);
    }
}

/*---------------------------------------------------------------------------
 *                            Security Initialization
 *---------------------------------------------------------------------------*/

static SOPC_SKManager* createSKmanager(void)
{
    /* Create Service Keys manager and set constant keys */
    SOPC_SKManager* skm = SOPC_SKManager_Create();
    SOPC_ASSERT(NULL != skm && "SOPC_SKManager_Create failed");
    uint32_t nbKeys = 0;
    SOPC_Buffer* keysBuffer =
        SOPC_Buffer_Create(sizeof(pubSub_keySign) + sizeof(pubSub_keyEncrypt) + sizeof(pubSub_keyNonce));
    SOPC_ReturnStatus status = (NULL == keysBuffer ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, pubSub_keySign, (uint32_t) sizeof(pubSub_keySign));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, pubSub_keyEncrypt, (uint32_t) sizeof(pubSub_keyEncrypt));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, pubSub_keyNonce, (uint32_t) sizeof(pubSub_keyNonce));
    }
    SOPC_ByteString keys;
    SOPC_ByteString_Initialize(&keys);
    SOPC_String securityPolicyUri;
    SOPC_String_Initialize(&securityPolicyUri);
    if (SOPC_STATUS_OK == status)
    {
        nbKeys = 1;
        // Set buffer as a byte string for API compatibility
        keys.DoNotClear = true;
        keys.Length = (int32_t) keysBuffer->length;
        keys.Data = (SOPC_Byte*) keysBuffer->data;
        // Set security policy
        status = SOPC_String_AttachFromCstring(&securityPolicyUri, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKManager_SetKeys(skm, &securityPolicyUri, 1, &keys, nbKeys, UINT32_MAX, UINT32_MAX);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_SKManager_Clear(skm);
        SOPC_Free(skm);
        skm = NULL;
    }
    SOPC_Buffer_Delete(keysBuffer);

    return skm;
}

/*---------------------------------------------------------------------------
 *                             Publisher Callback
 *---------------------------------------------------------------------------*/

SOPC_DataValue* SOPC_GetSourceVariables_TestFunc(OpcUa_ReadValueId* nodesToRead, int32_t nbValues);

SOPC_DataValue* SOPC_GetSourceVariables_TestFunc(OpcUa_ReadValueId* nodesToRead, int32_t nbValues)

{
    SOPC_ASSERT(nbValues <= NB_VARS);
    SOPC_ASSERT(0 < nbValues);
    SOPC_DataValue* dataValues = SOPC_Calloc((size_t) nbValues, sizeof(*dataValues));
    SOPC_ASSERT(NULL != dataValues);

    for (int32_t i = 0; i < nbValues; i++)
    {
        SOPC_DataValue* dataValue = &dataValues[i];

        SOPC_DataValue_Initialize(dataValue);

        OpcUa_ReadValueId* readValue = &nodesToRead[i];

        uint32_t index = readValue->NodeId.Data.Numeric;
        SOPC_ASSERT(13 == readValue->AttributeId); // Value => AttributeId=13
        SOPC_ASSERT(SOPC_IdentifierType_Numeric == readValue->NodeId.IdentifierType);

        // index node id
        SOPC_ASSERT(NB_VARS > index);
        dataValue->Value.ArrayType = varArr[index].ArrayType;
        dataValue->Value.BuiltInTypeId = varArr[index].BuiltInTypeId;
        dataValue->Value.Value = varArr[index].Value;

        OpcUa_ReadValueId_Clear(nodesToRead);
    }
    SOPC_Free(nodesToRead);

    return dataValues;
}

/*---------------------------------------------------------------------------
 *                             Subscriber Callback
 *---------------------------------------------------------------------------*/

static bool SOPC_SetTargetVariables_Test(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    SOPC_ASSERT(NULL != nodesToWrite);

    if (!SOPC_Atomic_Int_Get(&gStopAtomic))
    {
        if (1 == nbValues)
        {
            SOPC_Variant* variant = &(nodesToWrite[0].Value.Value);
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
        else if (2 == nbValues)
        {
            SOPC_Variant* variant = &(nodesToWrite[0].Value.Value);
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

            SOPC_Atomic_Int_Set(&gStopAtomic, true);
        }
    }

    for (int32_t i = 0; i < nbValues; i++)
    {
        OpcUa_WriteValue_Clear(&(nodesToWrite[i]));
    }
    SOPC_Free(nodesToWrite);

    return true;
}

/*---------------------------------------------------------------------------
 *                             Main program
 *---------------------------------------------------------------------------*/

void suite_test_publisher_subsriber(int* index)
{
    vm_cprintf("\nTEST %d: validation publisher subscriber\n", *index);

    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logLevel = SOPC_LOG_LEVEL_ERROR;
    logConfiguration.logSystem = SOPC_LOG_SYSTEM_USER;
    logConfiguration.logSysConfig.userSystemLogConfig.doLog = &log_UserCallback;

    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_GetStatic();
    SOPC_ASSERT(NULL != config);

    /* Set publisher callback */
    SOPC_PubSourceVariableConfig* sourceConfig = SOPC_PubSourceVariableConfig_Create(&SOPC_GetSourceVariables_TestFunc);
    SOPC_ASSERT(NULL != sourceConfig);

    /* Set subscriber callback */
    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(&SOPC_SetTargetVariables_Test);
    SOPC_ASSERT(NULL != targetConfig);

    /* PubSub Security Keys configuration */
    SOPC_SKManager* skmanager = createSKmanager();
    SOPC_ASSERT(NULL != skmanager);

    SOPC_PubSubSKS_Init();
    SOPC_PubSubSKS_SetSkManager(skmanager);

    const uint32_t nbPublisher = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    SOPC_ASSERT(0 != nbPublisher);
    const uint32_t nbSubscriber = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    SOPC_ASSERT(0 != nbSubscriber);

    P4_prio_t myPriority = 0;
    SOPC_ASSERT(P4_E_OK == p4_thread_get_priority(P4_THREAD_MYSELF, &myPriority));
    int threadPrio = myPriority / 2;
    bool res = SOPC_PubScheduler_Start(config, sourceConfig, threadPrio);
    if (res)
    {
        vm_cprintf("Pub scheduler start\n");
    }
    else
    {
        vm_cprintf("Pub scheduler failed start\n");
    }
    SOPC_ASSERT(res);
    res = SOPC_SubScheduler_Start(config, targetConfig, NULL, NULL, NULL, threadPrio);
    if (res)
    {
        vm_cprintf("Sub scheduler start\n");
    }
    else
    {
        vm_cprintf("Sub scheduler failed start\n");
    }
    SOPC_ASSERT(res);

    while (SOPC_STATUS_OK == status && res && 0 == SOPC_Atomic_Int_Get(&gStopAtomic))
    {
        SOPC_Sleep(100);
    }

    SOPC_PubScheduler_Stop();
    SOPC_SubScheduler_Stop();
    SOPC_SKManager_Clear(skmanager);
    SOPC_Free(skmanager);
    skmanager = NULL;
    SOPC_PubSourceVariableConfig_Delete(sourceConfig);
    sourceConfig = NULL;
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    targetConfig = NULL;
    SOPC_PubSubConfiguration_Delete(config);
    config = NULL;
    SOPC_Common_Clear();
    *index += 1;
    vm_cprintf("Test 1: ok\n");
}
