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
#include <stdbool.h>

#include "opcua_statuscodes.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_sub_target_variable.h"

struct _SOPC_SubTargetVariableConfig
{
    SOPC_SetTargetVariables_Func* callback;
};

SOPC_SubTargetVariableConfig* SOPC_SubTargetVariableConfig_Create(SOPC_SetTargetVariables_Func* callback)
{
    SOPC_SubTargetVariableConfig* targetConfig = SOPC_Calloc(1, sizeof(*targetConfig));
    if (NULL != targetConfig)
    {
        targetConfig->callback = callback;
    }
    return targetConfig;
}

void SOPC_SubTargetVariableConfig_Delete(SOPC_SubTargetVariableConfig* targetConfig)
{
    SOPC_Free(targetConfig);
}

bool SOPC_SubTargetVariable_SetVariables(SOPC_SubTargetVariableConfig* targetConfig,
                                         const SOPC_DataSetReader* reader,
                                         const SOPC_Dataset_LL_DataSetMessage* dsm)
{
    if (NULL == targetConfig || NULL == reader || NULL == dsm)
    {
        return false;
    }

    uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
    uint16_t nbFieldsMetadata = SOPC_DataSetReader_Nb_FieldMetaData(reader);

    if (nbFields != nbFieldsMetadata)
    {
        return false; // Incoherent parameters
    }

    if (NULL == targetConfig->callback)
    {
        return true; // Nothing to do since there is no callback to call
    }

    OpcUa_WriteValue* writeValues = SOPC_Calloc(nbFields, sizeof(*writeValues));
    if (NULL == writeValues)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint16_t i = 0; i < nbFields; i++)
    {
        OpcUa_WriteValue* value = &writeValues[i];
        OpcUa_WriteValue_Initialize(value);

        const SOPC_Variant* variant = NULL;
        SOPC_FieldTarget* targetData = NULL;
        SOPC_FieldMetaData* fieldMetaData = NULL;

        if (SOPC_STATUS_OK == status)
        {
            variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, i);
            assert(NULL != variant);
            fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(reader, i);
            assert(NULL != fieldMetaData);
            targetData = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
            assert(NULL != targetData);

            // Fill write value:
            // NodeId
            status = SOPC_NodeId_Copy(&value->NodeId, SOPC_FieldTarget_Get_NodeId(targetData));
        }

        if (SOPC_STATUS_OK == status)
        {
            // AttributeId
            value->AttributeId = SOPC_FieldTarget_Get_AttributeId(targetData);

            // source and target indexes:
            assert(NULL ==
                   SOPC_FieldTarget_Get_SourceIndexRange(targetData)); // We do not manage index range on received data

            const char* targetIndexRange = SOPC_FieldTarget_Get_TargetIndexRange(targetData);
            if (NULL != targetIndexRange)
            {
                status = SOPC_String_CopyFromCString(&value->IndexRange,
                                                     targetIndexRange); // But server will manage it on written data
            }
        }

        // Fill value
        if (SOPC_STATUS_OK == status)
        {
            bool isBad = false;
            bool isCompatibleType = SOPC_PubSubHelpers_IsCompatibleVariant(fieldMetaData, variant, &isBad);

            if (isCompatibleType)
            {
                if (isBad)
                {
                    // Bad status code received instead of value, set it as status and keep value Null (default)
                    value->Value.Status = variant->Value.Status;
                }
                else
                {
                    // Nominal case
                    status = SOPC_Variant_Copy(&value->Value.Value, variant);
                }
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        for (uint16_t i = 0; i < nbFields; i++)
        {
            OpcUa_WriteValue_Clear(&writeValues[i]);
        }
        SOPC_Free(writeValues);

        return false;
    }

    return targetConfig->callback(writeValues, nbFields);
}
