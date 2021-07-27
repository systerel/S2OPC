/* Copyright (C) Systerel SAS 2019, all rights reserved. */

#include "pubsub_config_static.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>


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
    if (allocSuccess)
    {
        SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
        assert(reader != NULL);
        SOPC_DataSetReader_Set_WriterGroupVersion(reader, version);
        SOPC_DataSetReader_Set_WriterGroupId(reader, messageId);
        SOPC_DataSetReader_Set_DataSetWriterId(reader, messageId); // Same as WriterGroup
        SOPC_DataSetReader_Set_ReceiveTimeout(reader, 2 * interval);
        SOPC_DataSetReader_Set_PublisherId_UInteger(reader, publisherId);
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
    

    /* 1 connection Sub */
    alloc = SOPC_PubSubConfiguration_Allocate_SubConnection_Array(config, 1);
    
    /** connection sub 0 **/
    
    if (alloc)
    {
        // Set subscriber id and address
        connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
        alloc = SOPC_PubSubConnection_Set_Address(connection, "opc.udp://232.1.2.100:4840");
    }

    if (alloc)
    {
        // 2 sub messages
        alloc = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, 2);
    }

        
    SOPC_DataSetReader* reader = NULL;
        
    /*** Sub Message 14 ***/
    
    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, 0, 42, 14, 1, 100, SOPC_SecurityMode_SignAndEncrypt);
        alloc = NULL != reader;
    }
    
    if (alloc)
    {
        alloc = SOPC_PubSubConfig_SetSubNbVariables(reader, 2);
    }
    if (alloc)
    {
        
        SOPC_PubSubConfig_SetSubVariableAt(reader, 0, "ns=1;s=SubBool", SOPC_Boolean_Id);
        SOPC_PubSubConfig_SetSubVariableAt(reader, 1, "ns=1;s=SubString", SOPC_String_Id);
    }
    
    /*** Sub Message 15 ***/
    
    if (alloc)
    {
        reader = SOPC_PubSubConfig_SetSubMessageAt(connection, 1, 42, 15, 1, 1000, SOPC_SecurityMode_None);
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
    