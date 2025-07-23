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

#include <stdbool.h>

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_sub_target_variable.h"

struct _SOPC_SubTargetVariableConfig
{
    SOPC_SetTargetVariables_Func* callback;
};

struct SOPC_TargetVariableCtx
{
    OpcUa_WriteValue* writeValues;
    uint16_t nbValues;
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
                                         SOPC_TargetVariableCtx* targetVariable,
                                         const SOPC_DataSetReader* reader,
                                         SOPC_Dataset_LL_DataSetMessage* dsm)
{
    if (NULL == targetConfig || NULL == targetVariable || NULL == dsm)
    {
        return false;
    }

    uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

    if (nbFields != targetVariable->nbValues)
    {
        return false; // Incoherent parameters
    }

    if (NULL == targetConfig->callback)
    {
        return true; // Nothing to do since there is no callback to call
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint16_t i = 0; SOPC_STATUS_OK == status && i < nbFields; i++)
    {
        OpcUa_WriteValue* value = &targetVariable->writeValues[i];

        SOPC_Variant* variant = NULL;
        SOPC_FieldMetaData* fieldMetaData = NULL;

        variant = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(dsm, i);
        SOPC_ASSERT(NULL != variant);
        fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(reader, i);
        SOPC_ASSERT(NULL != fieldMetaData);

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
                SOPC_Variant_Move(&value->Value.Value, variant);
                // Transmit received timestamp
                // If no timestamp this one should be equal to zero
                value->Value.SourceTimestamp = (SOPC_DateTime) SOPC_Dataset_LL_DataSetMsg_Get_Timestamp(dsm);
            }
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    bool res = false;
    if (SOPC_STATUS_OK == status)
    {
        res = targetConfig->callback(targetVariable->writeValues, nbFields);
    }

    for (uint16_t i = 0; i < nbFields; i++)
    {
        OpcUa_WriteValue* value = &targetVariable->writeValues[i];
        SOPC_Variant_Clear(&value->Value.Value);
    }

    return res;
}

/* Return a list of initialized writeValues. In case of error return NULL */
static OpcUa_WriteValue* createAndInitialize_writeValues(const SOPC_DataSetReader* reader, uint16_t nbValues)
{
    OpcUa_WriteValue* writeValues = SOPC_Calloc(nbValues, sizeof(*writeValues));
    if (NULL == writeValues)
    {
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint16_t i = 0; i < nbValues; i++)
    {
        OpcUa_WriteValue* value = &writeValues[i];
        OpcUa_WriteValue_Initialize(value);

        SOPC_FieldTarget* targetData = NULL;
        SOPC_FieldMetaData* fieldMetaData = NULL;

        if (SOPC_STATUS_OK == status)
        {
            fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(reader, i);
            SOPC_ASSERT(NULL != fieldMetaData);
            targetData = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
            SOPC_ASSERT(NULL != targetData);

            // Fill write value:
            // NodeId
            status = SOPC_NodeId_Copy(&value->NodeId, SOPC_FieldTarget_Get_NodeId(targetData));
        }

        if (SOPC_STATUS_OK == status)
        {
            // AttributeId
            value->AttributeId = SOPC_FieldTarget_Get_AttributeId(targetData);

            // source and target indexes:
            SOPC_ASSERT(NULL == SOPC_FieldTarget_Get_SourceIndexRange(
                                    targetData)); // We do not manage index range on received data

            const char* targetIndexRange = SOPC_FieldTarget_Get_TargetIndexRange(targetData);
            if (NULL != targetIndexRange)
            {
                status = SOPC_String_CopyFromCString(&value->IndexRange,
                                                     targetIndexRange); // But server will manage it on written data
            }
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        for (uint16_t i = 0; i < nbValues; i++)
        {
            OpcUa_WriteValue_Clear(&writeValues[i]);
        }
        SOPC_Free(writeValues);
        writeValues = NULL;
    }
    return writeValues;
}

SOPC_TargetVariableCtx* SOPC_SubTargetVariable_TargetVariablesCtx_Create(const SOPC_DataSetReader* reader)
{
    uint16_t nbFields = SOPC_DataSetReader_Nb_FieldMetaData(reader);
    if (nbFields == 0)
    {
        return NULL;
    }

    SOPC_TargetVariableCtx* variableCtx = SOPC_Malloc(sizeof(*variableCtx));
    if (NULL == variableCtx)
    {
        return NULL;
    }

    OpcUa_WriteValue* writeValues = createAndInitialize_writeValues(reader, nbFields);
    if (NULL == writeValues)
    {
        SOPC_Free(variableCtx);
        variableCtx = NULL;
    }
    else
    {
        variableCtx->writeValues = writeValues;
        variableCtx->nbValues = nbFields;
    }
    return variableCtx;
}

void SOPC_SubTargetVariable_TargetVariableCtx_Delete(SOPC_TargetVariableCtx** subTargetVariable)
{
    if (NULL != subTargetVariable && NULL != *subTargetVariable)
    {
        if (NULL != (*subTargetVariable)->writeValues)
        {
            for (uint16_t i = 0; i < (*subTargetVariable)->nbValues; i++)
            {
                OpcUa_WriteValue_Clear(&(*subTargetVariable)->writeValues[i]);
            }
            SOPC_Free((*subTargetVariable)->writeValues);
        }
        SOPC_Free(*subTargetVariable);
        *subTargetVariable = NULL;
    }
}
