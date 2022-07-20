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

#if 0
    This file provides a software-generated Pub-Sub configuration corresponding to the
    following XMl configuration:


    <PubSub publisherId="42" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="s2opc_pubsub_config.xsd">
        <!-- one to many -->
        <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
            <!-- one to many -->
            <message id="20" version="1" publishingInterval="100." securityMode="signAndEncrypt">
              <!-- one to many -->
                <variable nodeId="ns=1;s=PubVarLoops" displayName="SubVarLoops" dataType="UInt32"/>
                <variable nodeId="ns=1;s=PubVarUpTime" displayName="SubVarUpTime" dataType="UInt32"/>
                <variable nodeId="ns=1;s=PubVarFill64" displayName="PubVarFill64" dataType="UInt64"/>
                <!-- the nodeId is used to retrieve the variable in the adresse space -->
            </message>
        </connection>
        <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
            <!-- one to many -->
            <message id="14" version="1" publishingInterval="100." publisherId="42" securityMode="signAndEncrypt">
                <!-- one to many -->
                <variable nodeId="ns=1;s=SubBool" displayName="varBool" dataType="Boolean"/>
                <variable nodeId="ns=1;s=SubString" displayName="varString" dataType="String"/>
                <!-- the nodeId is used to retrieve the variable in the adresse space -->
            </message>
            <message id="15" version="1" publishingInterval="1000." publisherId="42">
                <!-- one to many -->
                <variable nodeId="ns=1;s=SubInt" displayName="varInt" dataType="Int64"/>
                <variable nodeId="ns=1;s=SubUInt" displayName="varUInt" dataType="UInt64"/>
            </message>
        </connection>
    </PubSub>

#endif

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
        SOPC_DataSetReader_Set_DataSetWriterId(reader,  messageId);
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
        SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, 42);
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

    /*** Pub Message 20 ***/

    SOPC_DataSetWriter* writer = NULL;
    if (alloc)
    {
        // TODO : None or Sign&Encrypt
        writer = SOPC_PubSubConfig_SetPubMessageAt(connection, 0, 20, 1, CONFIG_SOPC_PUBLISHER_PERIOD_US/1000, SOPC_SecurityMode_None
                                                   //                    SOPC_SecurityMode_SignAndEncrypt
        );
        alloc = NULL != writer;
    }

    SOPC_PublishedDataSet* dataset = NULL;
    if (alloc)
    {
        dataset = SOPC_PubSubConfig_InitDataSet(config, 0, writer, 3);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, SERVER_STATUS_PUB_VAR_LOOP, SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 1, SERVER_CMD_PUBSUB_PERIOD_MS, SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 2, "ns=1;s=PubVarFill64", SOPC_UInt64_Id);
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
        // 2 sub messages
        alloc = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, 2);
    }

    SOPC_DataSetReader* reader = NULL;
    /*** Sub Message 21 ***/

    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, 0, 42, 21, 1, 1000, SOPC_SecurityMode_None);
        alloc = NULL != reader;
    }

    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(reader, 4);
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetSubVariableAt(reader, 0, "ns=1;s=SubVarResult", SOPC_Byte_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 1, "ns=1;s=SubVarInfo", SOPC_String_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 2, "ns=1;s=SubVarLoops", SOPC_UInt32_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 3, "ns=1;s=SubVarUpTime", SOPC_UInt32_Id);
    }

    /*** Sub Message 15 ***/

    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, 1, 42, 15, 1, CONFIG_SOPC_SUBSCRIBER_PERIOD_US / 1000,
                                                   SOPC_SecurityMode_None);
        alloc = NULL != reader;
    }

    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(reader, 2);
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetSubVariableAt(reader, 0, "ns=1;s=SubInt", SOPC_Int64_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 1, "ns=1;s=SubUInt", SOPC_UInt64_Id);
    }

    if (!alloc)
    {
        SOPC_PubSubConfiguration_Delete(config);
        return NULL;
    }

    return config;
}
