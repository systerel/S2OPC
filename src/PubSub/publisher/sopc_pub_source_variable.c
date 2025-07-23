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

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_helpers.h"

struct SOPC_PubSourceVariableConfig
{
    SOPC_GetSourceVariables_Func* callback;
};

struct SOPC_SourceVariableCtx
{
    OpcUa_ReadValueId* readValues;
    uint16_t nbValues;
};

/* Return a list of initialized readValues. In case of error return NULL */
static OpcUa_ReadValueId* createAndInitialize_readValues(const SOPC_PublishedDataSet* pubDataset, uint16_t nbValues)
{
    /* Create and initialize a list of readValues used in publisher user callback */
    OpcUa_ReadValueId* readValues = SOPC_Calloc(nbValues, sizeof(*readValues));
    if (NULL == readValues)
    {
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Build the Read request using PublishedVariable property of each FieldMetaData
    for (uint16_t i = 0; i < nbValues; i++)
    {
        OpcUa_ReadValueId* readValue = &readValues[i];
        OpcUa_ReadValueId_Initialize(readValue);

        SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataset, i);
        SOPC_ASSERT(NULL != fieldData);

        SOPC_PublishedVariable* sourceData = SOPC_FieldMetaData_Get_PublishedVariable(fieldData);
        SOPC_ASSERT(NULL != sourceData);

        readValue->AttributeId = SOPC_PublishedVariable_Get_AttributeId(sourceData);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(&readValue->NodeId, SOPC_PublishedVariable_Get_NodeId(sourceData));
        }

        if (SOPC_STATUS_OK == status)
        {
            const char* indexRange = SOPC_PublishedVariable_Get_IndexRange(sourceData);
            if (NULL != indexRange)
            {
                status = SOPC_String_CopyFromCString(&readValue->IndexRange, indexRange);
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        for (uint16_t i = 0; i < nbValues; i++)
        {
            OpcUa_ReadValueId_Clear(&readValues[i]);
        }
        SOPC_Free(readValues);
        readValues = NULL;
    }
    return readValues;
}

SOPC_SourceVariableCtx* SOPC_PubSourceVariable_SourceVariablesCtx_Create(const SOPC_PublishedDataSet* pubDataset)
{
    uint16_t nbFieldsMetadata = SOPC_PublishedDataSet_Nb_FieldMetaData(pubDataset);
    if (nbFieldsMetadata == 0)
    {
        return NULL;
    }
    SOPC_SourceVariableCtx* variableCtx = SOPC_Malloc(sizeof(*variableCtx));
    if (NULL == variableCtx)
    {
        return NULL;
    }

    OpcUa_ReadValueId* readValues = createAndInitialize_readValues(pubDataset, nbFieldsMetadata);
    if (NULL == readValues)
    {
        SOPC_Free(variableCtx);
        variableCtx = NULL;
    }
    else
    {
        variableCtx->readValues = readValues;
        variableCtx->nbValues = nbFieldsMetadata;
    }

    return variableCtx;
}

void SOPC_PubSourceVariable_SourceVariableCtx_Delete(SOPC_SourceVariableCtx** pubSourceVariable)
{
    if (NULL != pubSourceVariable && NULL != *pubSourceVariable)
    {
        if (NULL != (*pubSourceVariable)->readValues)
        {
            for (uint16_t i = 0; i < (*pubSourceVariable)->nbValues; i++)
            {
                OpcUa_ReadValueId_Clear(&(*pubSourceVariable)->readValues[i]);
            }
            SOPC_Free((*pubSourceVariable)->readValues);
        }
        SOPC_Free(*pubSourceVariable);
        *pubSourceVariable = NULL;
    }
}

SOPC_PubSourceVariableConfig* SOPC_PubSourceVariableConfig_Create(SOPC_GetSourceVariables_Func* callback)
{
    SOPC_PubSourceVariableConfig* sourceConfig = NULL;
    if (NULL != callback)
    {
        sourceConfig = SOPC_Calloc(1, sizeof(*sourceConfig));
    }
    if (NULL != sourceConfig)
    {
        sourceConfig->callback = callback;
    }
    return sourceConfig;
}

void SOPC_PubSourceVariableConfig_Delete(SOPC_PubSourceVariableConfig* sourceConfig)
{
    SOPC_Free(sourceConfig);
}

SOPC_DataValue* SOPC_PubSourceVariable_GetVariables(const SOPC_PubSourceVariableConfig* sourceConfig,
                                                    const SOPC_SourceVariableCtx* sourceVariable)
{
    if (NULL == sourceConfig || NULL == sourceVariable || NULL == sourceVariable->readValues)
    {
        return NULL;
    }
    return sourceConfig->callback(sourceVariable->readValues, sourceVariable->nbValues);
}
