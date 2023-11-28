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

#include "pubsub_config_static.h"

#include <stdbool.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_profiles_lib_itf.h"

#include "samples_platform_dep.h"
#include "test_config.h"

#define PUB_CNX_NAME "PubCnx#1"
#define SUB_CNX_NAME "SubCnx#1"

// These nodeIds must exist in AddressSpace!
#define PUB_VAR_STRING "ns=1;s=PubString"
#define PUB_VAR_BYTE "ns=1;s=PubByte"
#define PUB_VAR_UINT32 "ns=1;s=PubUInt32"
#define PUB_VAR_INT16 "ns=1;s=PubInt16"
#define PUB_VAR_BOOL "ns=1;s=PubBool"
#define PUB_VAR_STATUS "ns=1;s=PubStatusCode"
#define NB_PUB_VARS 6

#define SUB_VAR_STRING "ns=1;s=SubString"
#define SUB_VAR_BYTE "ns=1;s=SubByte"
#define SUB_VAR_UINT32 "ns=1;s=SubUInt32"
#define SUB_VAR_INT16 "ns=1;s=SubInt16"
#define SUB_VAR_BOOL "ns=1;s=SubBool"
#define SUB_VAR_STATUS "ns=1;s=SubStatusCode"
#define NB_SUB_VARS 6

#define PUBLISHER_ID 42
#define MESSAGE_ID 20
#define MESSAGE_VERSION 1

static SOPC_DataSetWriter* SOPC_PubSubConfig_SetPubMessageAt(SOPC_PubSubConnection* connection,
                                                             uint16_t index,
                                                             uint16_t messageId,
                                                             uint32_t version,
                                                             uint64_t interval,
                                                             SOPC_SecurityMode_Type securityMode)

{
    SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, index);
    SOPC_WriterGroup_Set_Id(group, messageId);
    SOPC_WriterGroup_Set_Version(group, version);
    SOPC_WriterGroup_Set_PublishingInterval(group, (double) interval);
    SOPC_WriterGroup_Set_SecurityMode(group, securityMode);

    // Create one DataSet Writer
    SOPC_WriterGroup_Allocate_DataSetWriter_Array(group, 1);
    SOPC_DataSetWriter* writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);
    SOPC_DataSetWriter_Set_Id(writer, messageId);
    return writer;
}

static SOPC_PublishedDataSet* SOPC_PubSubConfig_InitDataSet(SOPC_PubSubConfiguration* config,
                                                            uint16_t dataSetIndex,
                                                            SOPC_DataSetWriter* writer,
                                                            uint16_t nbVar)
{
    SOPC_PublishedDataSet* dataset = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, dataSetIndex);
    SOPC_PublishedDataSet_Init(dataset, SOPC_PublishedDataItemsDataType, nbVar);
    SOPC_DataSetWriter_Set_DataSet(writer, dataset);

    return dataset;
}

static void SOPC_PubSubConfig_SetPubVariableAt(SOPC_PublishedDataSet* dataset,
                                               uint16_t index,
                                               char* strNodeId,
                                               SOPC_BuiltinId builtinType)
{
    SOPC_FieldMetaData* fieldmetadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, index);
    SOPC_PubSub_ArrayDimension arrayDimension = {.valueRank = -1, .arrayDimensions = NULL};
    SOPC_FieldMetaData_ArrayDimension_Move(fieldmetadata, &arrayDimension);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, builtinType);
    SOPC_PublishedVariable* publishedVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldmetadata);
    SOPC_ASSERT(NULL != publishedVar);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    SOPC_ASSERT(NULL != nodeId);
    SOPC_PublishedVariable_Set_NodeId(publishedVar, nodeId);
    SOPC_PublishedVariable_Set_AttributeId(publishedVar,
                                           13); // Value => AttributeId=13
}

static SOPC_DataSetReader* SOPC_PubSubConfig_SetSubMessageAt(SOPC_PubSubConnection* connection,
                                                             uint16_t index,
                                                             uint32_t publisherId,
                                                             uint16_t messageId,
                                                             uint32_t version,
                                                             uint64_t interval,
                                                             SOPC_SecurityMode_Type securityMode)
{
    SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, index);
    SOPC_ASSERT(readerGroup != NULL);
    SOPC_ReaderGroup_Set_SecurityMode(readerGroup, securityMode);
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, version);
    SOPC_ReaderGroup_Set_GroupId(readerGroup, messageId);
    bool allocSuccess = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, 1);
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, publisherId);
    if (allocSuccess)
    {
        SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
        SOPC_ASSERT(reader != NULL);
        SOPC_DataSetReader_Set_DataSetWriterId(reader, messageId);
        SOPC_DataSetReader_Set_ReceiveTimeout(reader, 2.0 * (double) interval);
        return reader;
    }
    return NULL;
}

static bool SOPC_PubSubConfig_SetSubNbVariables(SOPC_DataSetReader* reader, uint16_t nbVar)
{
    return SOPC_DataSetReader_Allocate_FieldMetaData_Array(reader, SOPC_TargetVariablesDataType, nbVar);
}

static void SOPC_PubSubConfig_SetSubVariableAt(SOPC_DataSetReader* reader,
                                               uint16_t index,
                                               char* strNodeId,
                                               SOPC_BuiltinId builtinType)
{
    SOPC_FieldMetaData* fieldmetadata = SOPC_DataSetReader_Get_FieldMetaData_At(reader, index);
    SOPC_ASSERT(fieldmetadata != NULL);

    /* fieldmetadata: type the field */
    SOPC_PubSub_ArrayDimension arrayDimension = {.valueRank = -1, .arrayDimensions = NULL};
    SOPC_FieldMetaData_ArrayDimension_Move(fieldmetadata, &arrayDimension);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, builtinType);

    /* FieldTarget: link to the source/target data */
    SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldmetadata);
    SOPC_ASSERT(fieldTarget != NULL);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    SOPC_FieldTarget_Set_NodeId(fieldTarget, nodeId);
    SOPC_FieldTarget_Set_AttributeId(fieldTarget, 13); // Value => AttributeId=13
}

SOPC_PubSubConfiguration* SOPC_PubSubConfig_GetStatic(void)
{
    const SOPC_SecurityMode_Type security_Mode =
        SOPC_CryptoProfile_Is_Implemented() ? CONFIG_SOPC_PUBSUB_SECURITY_MODE : SOPC_SecurityMode_None;
    bool alloc = true;
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    SOPC_PubSubConnection* connection;

    /* 1 connection pub */
    alloc = SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, 1);

    /** connection pub 0 **/

    if (alloc)
    {
        // Set publisher id and address
        connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, 0);
        SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, PUBLISHER_ID);
        alloc = SOPC_PubSubConnection_Set_Address(connection, CONFIG_SOPC_PUBLISHER_ADDRESS);
    }

    if (alloc)
    {
        // Set connection name
        alloc = SOPC_PubSubConnection_Set_Name(connection, PUB_CNX_NAME);
    }

    if (alloc)
    {
        const char* itf_name = CONFIG_SOPC_PUBLISHER_ITF_NAME;
        if (CONFIG_SOPC_PUBLISHER_ITF_NAME[0] == '\0')
        {
            // Force default interface if not specified
            itf_name = SOPC_Platform_Get_Default_Net_Itf();
        }
        if (itf_name[0] != '\0')
        {
            alloc = SOPC_PubSubConnection_Set_InterfaceName(connection, itf_name);
        }
    }

    if (alloc)
    {
        // 2 pub messages
        alloc = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, 1);
    }

    if (alloc)
    {
        // 2 published data sets
        alloc = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 1);
    }

    /*** Pub Message ***/

    SOPC_DataSetWriter* writer = NULL;
    if (alloc)
    {
        writer = SOPC_PubSubConfig_SetPubMessageAt(connection, 0, MESSAGE_ID, MESSAGE_VERSION,
                                                   CONFIG_SOPC_PUBLISHER_PERIOD_US / 1000, security_Mode);
        alloc = NULL != writer;
    }

    SOPC_PublishedDataSet* dataset = NULL;
    if (alloc)
    {
        dataset = SOPC_PubSubConfig_InitDataSet(config, 0, writer, NB_PUB_VARS);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, PUB_VAR_STRING, SOPC_String_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 1, PUB_VAR_UINT32, SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 2, PUB_VAR_INT16, SOPC_Int16_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 3, PUB_VAR_BOOL, SOPC_Boolean_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 4, PUB_VAR_STATUS, SOPC_StatusCode_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 5, PUB_VAR_BYTE, SOPC_Byte_Id);
    }

    /* 1 connection Sub */
    alloc = SOPC_PubSubConfiguration_Allocate_SubConnection_Array(config, 1);

    /** connection sub 0 **/

    if (alloc)
    {
        // Set subscriber id and address
        connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
        alloc = SOPC_PubSubConnection_Set_Address(connection, CONFIG_SOPC_SUBSCRIBER_ADDRESS);
    }

    if (alloc)
    {
        // Set connection name
        alloc = SOPC_PubSubConnection_Set_Name(connection, SUB_CNX_NAME);
    }

    if (alloc)
    {
        const char* itf_name = CONFIG_SOPC_PUBLISHER_ITF_NAME;
        if (CONFIG_SOPC_PUBLISHER_ITF_NAME[0] == '\0')
        {
            // Force default interface if not specified
            itf_name = SOPC_Platform_Get_Default_Net_Itf();
        }
        if (itf_name[0] != '\0')
        {
            alloc = SOPC_PubSubConnection_Set_InterfaceName(connection, itf_name);
        }
    }

    if (alloc)
    {
        // 1 sub message
        alloc = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, 1);
    }

    SOPC_DataSetReader* reader = NULL;
    /*** Sub Message ***/

    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, 0, PUBLISHER_ID, MESSAGE_ID, MESSAGE_VERSION, 1000,
                                                   security_Mode);
        alloc = NULL != reader;
    }

    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(reader, NB_SUB_VARS);
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetSubVariableAt(reader, 0, SUB_VAR_STRING, SOPC_String_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 1, SUB_VAR_UINT32, SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 2, SUB_VAR_INT16, SOPC_Int16_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 3, SUB_VAR_BOOL, SOPC_Boolean_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 4, SUB_VAR_STATUS, SOPC_StatusCode_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 5, SUB_VAR_BYTE, SOPC_Byte_Id);
    }

    if (!alloc)
    {
        SOPC_PubSubConfiguration_Delete(config);
        return NULL;
    }

    return config;
}
