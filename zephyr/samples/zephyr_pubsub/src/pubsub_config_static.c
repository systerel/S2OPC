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

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#ifndef CONFIG_SOPC_PUBLISHER_PERIOD_US
#error "CONFIG_SOPC_PUBLISHER_PERIOD_US is not defined!"
#endif

#ifndef CONFIG_SOPC_SUBSCRIBER_PERIOD_US
#error "CONFIG_SOPC_SUBSCRIBER_PERIOD_US is not defined!"
#endif

#ifndef CONFIG_SOPC_PUBLISHER_ADDRESS
#error "CONFIG_SOPC_PUBLISHER_ADDRESS is not defined!"
#endif

#ifndef CONFIG_SOPC_SUBSCRIBER_ADDRESS
#error "CONFIG_SOPC_SUBSCRIBER_ADDRESS is not defined!"
#endif

#define PUBSUB_VAR_STRING "ns=1;s=aString"
#define PUBSUB_VAR_BYTE "ns=1;s=aByte"
#define PUBSUB_VAR_UINT32 "ns=1;s=aUINT32"
#define PUBSUB_VAR_INT16 "ns=1;s=aINT16"
#define PUBSUB_VAR_BOOL "ns=1;s=aBOOL"
#define PUBSUB_VAR_STATUS "ns=1;s=aStatusCode"
#define NB_PUBSUB_VARS 6

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
    SOPC_WriterGroup_Set_PublishingInterval(group, interval);
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
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, builtinType);
    SOPC_PublishedVariable* publishedVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldmetadata);
    assert(NULL != publishedVar);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    assert(NULL != nodeId);
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
    assert(readerGroup != NULL);
    SOPC_ReaderGroup_Set_SecurityMode(readerGroup, securityMode);
    bool allocSuccess = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, 1);
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, publisherId);
    if (allocSuccess)
    {
        SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
        assert(reader != NULL);
        SOPC_DataSetReader_Set_DataSetWriterId(reader, messageId);
        SOPC_DataSetReader_Set_ReceiveTimeout(reader, 2 * interval);
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
    assert(fieldmetadata != NULL);

    /* fieldmetadata: type the field */
    SOPC_FieldMetaData_Set_ValueRank(fieldmetadata, -1);
    SOPC_FieldMetaData_Set_BuiltinType(fieldmetadata, builtinType);

    /* FieldTarget: link to the source/target data */
    SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldmetadata);
    assert(fieldTarget != NULL);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    SOPC_FieldTarget_Set_NodeId(fieldTarget, nodeId);
    SOPC_FieldTarget_Set_AttributeId(fieldTarget, 13); // Value => AttributeId=13
}

SOPC_PubSubConfiguration* SOPC_PubSubConfig_GetStatic(void)
{
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
        // TODO : None or Sign&Encrypt
        writer = SOPC_PubSubConfig_SetPubMessageAt(
            connection, 0, MESSAGE_ID, MESSAGE_VERSION, CONFIG_SOPC_PUBLISHER_PERIOD_US / 1000, SOPC_SecurityMode_None
            // SOPC_SecurityMode_SignAndEncrypt
        );
        alloc = NULL != writer;
    }

    SOPC_PublishedDataSet* dataset = NULL;
    if (alloc)
    {
        dataset = SOPC_PubSubConfig_InitDataSet(config, 0, writer, NB_PUBSUB_VARS);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, PUBSUB_VAR_STRING, SOPC_String_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 1, PUBSUB_VAR_UINT32, SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 2, PUBSUB_VAR_INT16, SOPC_Int16_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 3, PUBSUB_VAR_BOOL, SOPC_Boolean_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 4, PUBSUB_VAR_STATUS, SOPC_StatusCode_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 5, PUBSUB_VAR_BYTE, SOPC_Byte_Id);
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
        // 1 sub message
        alloc = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, 1);
    }

    SOPC_DataSetReader* reader = NULL;
    /*** Sub Message ***/

    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, 0, PUBLISHER_ID, MESSAGE_ID, MESSAGE_VERSION, 1000,
                                                   SOPC_SecurityMode_None);
        alloc = NULL != reader;
    }

    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(reader, NB_PUBSUB_VARS);
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetSubVariableAt(reader, 0, PUBSUB_VAR_STRING, SOPC_String_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 1, PUBSUB_VAR_UINT32, SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 2, PUBSUB_VAR_INT16, SOPC_Int16_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 3, PUBSUB_VAR_BOOL, SOPC_Boolean_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 4, PUBSUB_VAR_STATUS, SOPC_StatusCode_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 5, PUBSUB_VAR_BYTE, SOPC_Byte_Id);
    }

    if (!alloc)
    {
        SOPC_PubSubConfiguration_Delete(config);
        return NULL;
    }

    return config;
}
