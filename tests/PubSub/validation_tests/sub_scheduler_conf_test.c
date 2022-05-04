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

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include "sopc_atomic.h"
#include "sopc_mem_alloc.h"
#include "sopc_sub_scheduler.h"
#include "sopc_sub_target_variable.h"
#include "sopc_time.h"
#include "sopc_xml_loader.h"

static int32_t stop = 0;
static int32_t stateChanged = 0;

static int returnCode = 0;
static int callIndex = 0;

static bool SOPC_SetTargetVariables_Test(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    assert(NULL != nodesToWrite);

    if (!SOPC_Atomic_Int_Get(&stop))
    {
        callIndex++;
        if (callIndex == 1)
        {
            // Only one value in first DSM
            assert(1 == nbValues);

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
        else if (callIndex == 2)
        {
            // Two variables in second DSM
            assert(2 == nbValues);

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

            SOPC_Atomic_Int_Set(&stop, true);
        }

        if (returnCode == -1)
        {
            returnCode = 0;
        }
    }

    for (int32_t i = 0; i < nbValues; i++)
    {
        OpcUa_WriteValue_Clear(&(nodesToWrite[i]));
    }
    SOPC_Free(nodesToWrite);

    return true;
}

static void stateChangedCb(SOPC_PubSubState state)
{
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

int main(int argc, char** argv)
{
    int sleepCount = 20;

    char* filename;
    if (1 < argc)
    {
        filename = argv[1];
    }
    else
    {
        filename = "./config_sub.xml";
    }

    FILE* fd = fopen(filename, "r");
    assert(NULL != fd);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_ParseXML(fd);
    int closed = fclose(fd);
    assert(0 == closed);

    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(&SOPC_SetTargetVariables_Test);

    bool started = SOPC_SubScheduler_Start(config, targetConfig, stateChangedCb, 0);

    while (true == started && false == SOPC_Atomic_Int_Get(&stop) && sleepCount > 0)
    {
        SOPC_Sleep(100);
        sleepCount--;
    }

    SOPC_SubScheduler_Stop();

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
