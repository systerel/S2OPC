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

#include "sopc_mem_alloc.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_helpers.h"

struct _SOPC_PubSourceVariableConfig
{
    SOPC_GetSourceVariables_Func callback;
    SOPC_GetSourceVariablesRequest_Func callbackRequest;   // Request callback, invoked by pub scheduler
    SOPC_GetSourceVariablesResponse_Func callbackResponse; // Response callback, invoked by pub scheduler
};

SOPC_PubSourceVariableConfig* SOPC_PubSourceVariableConfig_Create(SOPC_GetSourceVariables_Func callback,
                                                                  SOPC_GetSourceVariablesRequest_Func callbackRequest,
                                                                  SOPC_GetSourceVariablesResponse_Func callbackResponse)
{
    SOPC_PubSourceVariableConfig* sourceConfig = NULL;
    if (NULL != callback)
    {
        sourceConfig = SOPC_Calloc(1, sizeof(*sourceConfig));
    }
    if (NULL != sourceConfig)
    {
        sourceConfig->callback = callback;
        sourceConfig->callbackRequest = callbackRequest;
        sourceConfig->callbackResponse = callbackResponse;
    }
    return sourceConfig;
}

void SOPC_PubSourceVariableConfig_Delete(SOPC_PubSourceVariableConfig* sourceConfig)
{
    SOPC_Free(sourceConfig);
}

SOPC_DataValue* SOPC_PubSourceVariable_GetVariablesSync(const SOPC_PubSourceVariableConfig* sourceConfig,
                                                    const SOPC_PublishedDataSet* pubDataset)
{
    if (NULL == sourceConfig || NULL == pubDataset)
    {
        return NULL;
    }

    assert(SOPC_PublishedDataItemsDataType == SOPC_PublishedDataSet_Get_DataSet_SourceType(pubDataset));
    uint16_t nbFieldsMetadata = SOPC_PublishedDataSet_Nb_FieldMetaData(pubDataset);

    OpcUa_ReadValueId* readValues = SOPC_Calloc(nbFieldsMetadata, sizeof(*readValues));
    if (NULL == readValues)
    {
        return NULL;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Build the Read request using PublishedVariable property of each FieldMetaData
    for (uint16_t i = 0; i < nbFieldsMetadata; i++)
    {
        OpcUa_ReadValueId* readValue = &readValues[i];
        OpcUa_ReadValueId_Initialize(readValue);

        SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataset, i);
        assert(NULL != fieldData);

        SOPC_PublishedVariable* sourceData = SOPC_FieldMetaData_Get_PublishedVariable(fieldData);
        assert(NULL != sourceData);

        readValue->AttributeId = SOPC_PublishedVariable_Get_AttributeId(sourceData);

        if (SOPC_STATUS_OK == status) // possibly set to NOK in previous iteration
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
        for (uint16_t i = 0; i < nbFieldsMetadata; i++)
        {
            OpcUa_ReadValueId_Clear(&readValues[i]);
        }
        SOPC_Free(readValues);

        return NULL;
    }

    return sourceConfig->callback(readValues, nbFieldsMetadata);
}

SOPC_ReturnStatus SOPC_PubSourceVariable_GetVariables(SOPC_EventHandler* eventHandler,
                                                      uintptr_t msgCtxt,
                                                      const SOPC_PubSourceVariableConfig* sourceConfig,
                                                      const SOPC_PublishedDataSet* pubDataset)
{
    // Note: Return result is mandatory. If SOPC_STATUS_OK is not returned, then
    // READY status is set to messageCtx. So, request can be performed.
    // Else, BUSY is set to messageCtx. Next request will be ignored.
    // This is important to avoid memory issue in case of
    // treatment of request by services thread takes a long time.

    if (NULL == sourceConfig || NULL == pubDataset)
    {
        return SOPC_STATUS_NOK;
    }

    assert(SOPC_PublishedDataItemsDataType == SOPC_PublishedDataSet_Get_DataSet_SourceType(pubDataset));
    uint16_t nbFieldsMetadata = SOPC_PublishedDataSet_Nb_FieldMetaData(pubDataset);

    OpcUa_ReadValueId* readValues = SOPC_Calloc(nbFieldsMetadata, sizeof(*readValues));
    if (NULL == readValues)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Build the Read request using PublishedVariable property of each FieldMetaData
    for (uint16_t i = 0; i < nbFieldsMetadata; i++)
    {
        OpcUa_ReadValueId* readValue = &readValues[i];
        OpcUa_ReadValueId_Initialize(readValue);

        SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataset, i);
        assert(NULL != fieldData);

        SOPC_PublishedVariable* sourceData = SOPC_FieldMetaData_Get_PublishedVariable(fieldData);
        assert(NULL != sourceData);

        readValue->AttributeId = SOPC_PublishedVariable_Get_AttributeId(sourceData);

        if (SOPC_STATUS_OK == status) // possibly set to NOK in previous iteration
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
        for (uint16_t i = 0; i < nbFieldsMetadata; i++)
        {
            OpcUa_ReadValueId_Clear(&readValues[i]);
        }
        SOPC_Free(readValues);

        return status;
    }

    status = sourceConfig->callbackRequest(eventHandler, // Event handle where application send response
                                           msgCtxt, // Message context used by pubscheduler when a response is received
                                           readValues,        //
                                           nbFieldsMetadata); //

    return status;
}

SOPC_DataValue* SOPC_PubSourceVariable_GetVariablesResponse(SOPC_PubSheduler_GetVariableRequestContext* requestContext,
                                                            const SOPC_PubSourceVariableConfig* sourceConfig)
{
    if (NULL == sourceConfig)
    {
        return NULL;
    }

    return sourceConfig->callbackResponse(requestContext); // Request where has been set response
}
