/* Copyright (C) Systerel SAS 2019, all rights reserved. */

#include "pubsub_config_static.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>



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
    SOPC_PublishedVariable_Set_AttributeId(publishedVar, 13); // Value => AttributeId=13
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
        alloc = SOPC_PubSubConnection_Set_Address(connection, "opc.udp://232.1.2.100:4840");
    }

    if (alloc)
    {
        // 2 pub messages
        alloc = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, 2);
    }

    if (alloc)
    {
        // 2 published data sets
        alloc = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 2);
    }

        
    /*** Pub Message 14 ***/
    
    SOPC_DataSetWriter* writer = NULL;
        
    if (alloc)
    {
        writer = SOPC_PubSubConfig_SetPubMessageAt(connection, 0, 14, 1, 100, SOPC_SecurityMode_SignAndEncrypt);
        alloc = NULL != writer;
    }
    
    SOPC_PublishedDataSet* dataset = NULL;
    
    if (alloc)
    {
        dataset = SOPC_PubSubConfig_InitDataSet(config, 0, writer, 2);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, "ns=1;s=PubBool", SOPC_Boolean_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 1, "ns=1;s=PubString", SOPC_String_Id);
    }
    
    /*** Pub Message 15 ***/
    
    if (alloc)
    {
        writer = SOPC_PubSubConfig_SetPubMessageAt(connection, 1, 15, 1, 1000, SOPC_SecurityMode_None);
        alloc = NULL != writer;
    }
    
    if (alloc)
    {
        dataset = SOPC_PubSubConfig_InitDataSet(config, 1, writer, 2);
        alloc = NULL != dataset;
    }
    if (alloc)
    {
        
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 0, "ns=1;s=PubInt", SOPC_Int64_Id);
        SOPC_PubSubConfig_SetPubVariableAt(dataset, 1, "ns=1;s=PubUInt", SOPC_UInt64_Id);
    }
    

    if (!alloc)
    {
        SOPC_PubSubConfiguration_Delete(config);
        return NULL;
    }

    return config;
}
    