/* Copyright (C) Systerel SAS 2019, all rights reserved. */

#include "pubsub_config_static.h"

#include <stdbool.h>
#include <string.h>

#include "sopc_assert.h"

// DO NOT EDIT THIS FILE HAS BEEN GENERATED BY generate-s2opc_pubsub-static-config.py



static SOPC_WriterGroup* SOPC_PubSubConfig_SetPubMessageAt(SOPC_PubSubConnection* connection,
                                                           uint16_t index,
                                                           uint16_t groupId,
                                                           uint32_t groupVersion,
                                                           double interval,
                                                           int32_t offsetUs,
                                                           SOPC_SecurityMode_Type securityMode)
{
    SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, index);
    SOPC_WriterGroup_Set_Id(group, groupId);
    SOPC_WriterGroup_Set_Version(group, groupVersion);
    SOPC_WriterGroup_Set_PublishingInterval(group, interval);
    SOPC_WriterGroup_Set_SecurityMode(group, securityMode);
    if (offsetUs >=0)
    {
        SOPC_WriterGroup_Set_PublishingOffset(group, offsetUs / 1000);
    }

    return group;
}



static SOPC_PublishedDataSet* SOPC_PubSubConfig_InitDataSet(SOPC_PubSubConfiguration* config,
                                                            uint16_t dataSetIndex,
                                                            SOPC_DataSetWriter* writer,
                                                            bool isAcyclic,
                                                            uint16_t dataSetId,
                                                            uint16_t nbVar)
{
    SOPC_PublishedDataSet* dataset = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, dataSetIndex);
    if(isAcyclic)
    {
        SOPC_PublishedDataSet_Init(dataset, SOPC_PublishedDataSetCustomSourceDataType, nbVar);
    }
    else
    {
        SOPC_PublishedDataSet_Init(dataset, SOPC_PublishedDataItemsDataType, nbVar);
    }
    SOPC_DataSetWriter_Set_DataSet(writer, dataset);
    SOPC_DataSetWriter_Set_Id(writer, dataSetId);

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
    SOPC_ASSERT(NULL != publishedVar);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(strNodeId, (int32_t) strlen(strNodeId));
    SOPC_ASSERT(NULL != nodeId);
    SOPC_PublishedVariable_Set_NodeId(publishedVar, nodeId);
    SOPC_PublishedVariable_Set_AttributeId(publishedVar,
                                           13); // Value => AttributeId=13
}


SOPC_PubSubConfiguration* SOPC_PubSubConfig_GetStatic(void)
{
    bool alloc = true;
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    
    SOPC_PubSubConnection* connection = NULL;

    /* 1 publisher connection */
    alloc = SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, 1);
    
    /* 2 Published Datasets */
    alloc = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 2);
    
    /** Publisher connection 0 **/
    
    if (alloc)
    {
        // Set publisher id and address
        connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, 0);
        SOPC_ASSERT(NULL != connection);
        SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, 123);
        alloc = SOPC_PubSubConnection_Set_Address(connection, "opc.udp://232.1.2.100:4840");
        //alloc = SOPC_PubSubConnection_Set_Address(connection, "opc.udp://192.168.1.108:4840");
        //alloc = SOPC_PubSubConnection_Set_Address(connection, "opc.udp://10.41.0.1:4840");
    }
    
    // Set acyclic publisher mode
    SOPC_PubSubConnection_Set_AcyclicPublisher(connection, 0);
    
    if (alloc)
    {
        // Allocate 1 writer groups (messages)
        alloc = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, 1);
    }

    
    SOPC_WriterGroup* writerGroup = NULL;
    /*** Pub Message 14 ***/
    
    if (alloc)
    {
        // GroupId = 14
        // GroupVersion = 1
        // Interval = 2000.000000 ms
        // Offest = -1 us
 
       writerGroup = SOPC_PubSubConfig_SetPubMessageAt(connection, 0, 14, 1, 2000.000000,-1, SOPC_SecurityMode_None);
       alloc = NULL != writerGroup;
    }
 
    if (alloc)
    {
       // 2 data sets for message 14
       alloc = SOPC_WriterGroup_Allocate_DataSetWriter_Array(writerGroup, 2);
    }

    
    /*** DataSetMessage No 1 of message 14 ***/
    
    SOPC_DataSetWriter* writer = NULL;
    SOPC_PublishedDataSet* dataset = NULL;
    if (alloc)
    {
        writer = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, 0);
        SOPC_ASSERT(NULL != writer);
        // WriterId = 50
        dataset = SOPC_PubSubConfig_InitDataSet(config, 0, writer, 0, 50, 1);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, "ns=1;i=5", SOPC_Boolean_Id); // PubBoolean
    }
    
    /*** DataSetMessage No 2 of message 14 ***/
    
    if (alloc)
    {
        writer = SOPC_WriterGroup_Get_DataSetWriter_At(writerGroup, 1);
        SOPC_ASSERT(NULL != writer);
        // WriterId = 51
        dataset = SOPC_PubSubConfig_InitDataSet(config, 1, writer, 0, 51, 2);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, "ns=2;i=6", SOPC_UInt32_Id); // PubUInt32
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 1, "ns=2;i=7", SOPC_UInt16_Id); // PubUInt16
    }
    

    if (!alloc)
    {
        SOPC_PubSubConfiguration_Delete(config);
        return NULL;
    }

    return config;
}
    