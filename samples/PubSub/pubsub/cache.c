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

#include "sopc_mem_alloc.h"
#include "sopc_logger.h"
#include "sopc_time.h"

#include "cache.h"


SOPC_Dict* g_cache = NULL;


static void free_datavalue(void* value)
{
    SOPC_DataValue_Clear(value);
    SOPC_Free(value);
}


/* Create a new datavalue with a default false/0/now/empty value of given scalar built-in type */
static SOPC_DataValue* new_datavalue(SOPC_BuiltinId type)
{
    SOPC_DataValue *dv = SOPC_Calloc(1, sizeof(SOPC_DataValue));
    if (NULL == dv)
    {
        return NULL;
    }

    /* Note: for now, only the Variant part of the DataValue is used by the publisher */
    SOPC_Variant *var = &dv->Value;
    var->BuiltInTypeId = type;
    var->ArrayType = SOPC_VariantArrayType_SingleValue;

    switch(type)
    {
    case SOPC_Null_Id:
        break;
    case SOPC_Boolean_Id:
        var->Value.Boolean = false;
        break;
    case SOPC_SByte_Id:
        var->Value.Sbyte = 0;
        break;
    case SOPC_Byte_Id:
        var->Value.Byte = 0;
        break;
    case SOPC_Int16_Id:
        var->Value.Int16 = 0;
        break;
    case SOPC_UInt16_Id:
        var->Value.Uint16 = 0;
        break;
    case SOPC_Int32_Id:
        var->Value.Int32 = 0;
        break;
    case SOPC_UInt32_Id:
        var->Value.Uint32 = 0;
        break;
    case SOPC_Int64_Id:
        var->Value.Int64 = 0;
        break;
    case SOPC_UInt64_Id:
        var->Value.Uint64 = 0;
        break;
    case SOPC_Float_Id:
        var->Value.Floatv = 0.f;
        break;
    case SOPC_Double_Id:
        var->Value.Doublev = 0.;
        break;
    case SOPC_String_Id:
        SOPC_String_Initialize(&var->Value.String);
        break;
    case SOPC_DateTime_Id:
        var->Value.Date = SOPC_Time_GetCurrentTimeUTC();
        break;
    case SOPC_ByteString_Id:
        SOPC_ByteString_Initialize(&var->Value.Bstring);
        break;
    case SOPC_StatusCode_Id:
        var->Value.Status = SOPC_GoodGenericStatus;
        break;
    case SOPC_Guid_Id:
    case SOPC_XmlElement_Id:
    case SOPC_NodeId_Id:
    case SOPC_ExpandedNodeId_Id:
    case SOPC_QualifiedName_Id:
    case SOPC_LocalizedText_Id:
    case SOPC_ExtensionObject_Id:
    case SOPC_DataValue_Id:
    case SOPC_DiagnosticInfo_Id:
    case SOPC_Variant_Id:
    default:
        assert(false && "Cannot create default empty value for complex types");
        break;
    }

    return dv;
}


bool Cache_Initialize(SOPC_PubSubConfiguration* config)
{
    if (NULL == config)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cache initialization with NULL configuration");
        return false;
    }

    if (NULL != g_cache)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Cache already initialized");
        return false;
    }

    g_cache =  SOPC_NodeId_Dict_Create(true, free_datavalue);
    bool res = NULL != g_cache;

    /* Parse configuration and fill the cache with default values */
    /* NodeId is in the field metadata (target variable for subscriber and published variable for publisher)
     *  and fields are in DataSetMetaData, which is found in reader groups of subscriber connections,
     *  and in published data set for publishers */
    const uint32_t nbPubConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    for (uint32_t i=0; res && i<nbPubConnection; ++i)
    {
        /* TODO: it might be shorter to directly Get_PublishedDataSet_At */
        const SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, i);
        const uint16_t nbWriterGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
        for (uint16_t j=0; res && j<nbWriterGroup; ++j)
        {
            const SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);
            const uint8_t nbDataSetWriter = SOPC_WriterGroup_Nb_DataSetWriter(group);
            for (uint8_t k=0; res && k<nbDataSetWriter; ++k)
            {
                const SOPC_DataSetWriter* writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, k);
                const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
                const uint16_t nbFields = SOPC_PublishedDataSet_Nb_FieldMetaData(dataset);
                for (uint16_t l=0; res && l<nbFields; ++l)
                {
                    const SOPC_FieldMetaData* metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, l);
                    SOPC_BuiltinId type = SOPC_FieldMetaData_Get_BuiltinType(metadata);

                    /* Retrieve and copy the NodeId for insertion in the Cache */
                    const SOPC_PublishedVariable* target = SOPC_FieldMetaData_Get_PublishedVariable(metadata);
                    const SOPC_NodeId* src = SOPC_PublishedVariable_Get_NodeId(target);
                    SOPC_NodeId *nid = SOPC_Calloc(1, sizeof(SOPC_NodeId));
                    res &= (NULL != nid);

                    if (res)
                    {
                        SOPC_ReturnStatus status = SOPC_NodeId_Copy(nid, src);
                        if (SOPC_STATUS_OK != status)
                        {
                            res = false;
                        }
                    }

                    /* Insert a DataValue initialized to a default value */
                    if (res)
                    {
                        SOPC_DataValue *dv = new_datavalue(type);
                        res &= Cache_Set(nid, dv);  /* Both will be freed by the cache */
                    }

                    if (res)
                    {
                        char *nid_str = SOPC_NodeId_ToCString(nid);
                        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Cache: inserted %s of type %d", nid_str, type);
                        SOPC_Free(nid_str);
                    }
                }
            }
        }
    }

    const uint32_t nbSubConnection = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    for (uint32_t i=0; res && i<nbSubConnection; ++i)
    {
        const SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, i);
        const uint16_t nbReaderGroup = SOPC_PubSubConnection_Nb_ReaderGroup(connection);
        for (uint16_t j=0; res && j<nbReaderGroup; ++j)
        {
            const SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, j);
            const uint8_t nbDataSetReader = SOPC_ReaderGroup_Nb_DataSetReader(readerGroup);
            for (uint8_t k=0; res && k<nbDataSetReader; ++k)
            {
                const SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, k);
                const uint16_t nbFields = SOPC_DataSetReader_Nb_FieldMetaData(dataSetReader);
                for (uint16_t l=0; res && l<nbFields; ++l)
                {
                    const SOPC_FieldMetaData* metadata = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, l);
                    SOPC_BuiltinId type = SOPC_FieldMetaData_Get_BuiltinType(metadata);

                    /* Retrieve and copy the NodeId for insertion in the Cache */
                    const SOPC_FieldTarget* target = SOPC_FieldMetaData_Get_TargetVariable(metadata);
                    const SOPC_NodeId* src = SOPC_FieldTarget_Get_NodeId(target);
                    SOPC_NodeId *nid = SOPC_Calloc(1, sizeof(SOPC_NodeId));
                    res &= (NULL != nid);

                    if (res)
                    {
                        SOPC_ReturnStatus status = SOPC_NodeId_Copy(nid, src);
                        if (SOPC_STATUS_OK != status)
                        {
                            res = false;
                        }
                    }

                    /* Insert a DataValue initialized to a default value */
                    if (res)
                    {
                        SOPC_DataValue *dv = new_datavalue(type);
                        res &= Cache_Set(nid, dv);  /* Both will be freed by the cache */
                    }

                    if (res)
                    {
                        char *nid_str = SOPC_NodeId_ToCString(nid);
                        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Cache: inserted %s of type %d", nid_str, type);
                        SOPC_Free(nid_str);
                    }
                }
            }
        }

    }

    return res;
}


SOPC_DataValue* Cache_Get(const SOPC_NodeId *nid)
{
    return SOPC_Dict_Get(g_cache, nid, NULL);
}

bool Cache_Set(SOPC_NodeId *nid, SOPC_DataValue *dv)
{
    return SOPC_Dict_Insert(g_cache, nid, dv);
}

/* nodesToRead shall be freed by the callee, and returned DataValues will be freed by the caller */
SOPC_DataValue* Cache_GetSourceVariables(OpcUa_ReadValueId* nodesToRead, int32_t nbValues)
{
    assert(NULL != nodesToRead && nbValues > 0);
    assert(INT32_MAX < SIZE_MAX || nbValues <= SIZE_MAX);

    SOPC_DataValue *dvs = SOPC_Calloc((size_t)nbValues, sizeof(SOPC_DataValue));
    if (NULL == dvs)
    {
        return dvs;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (int32_t i=0; SOPC_STATUS_OK == status && i < nbValues; ++i)
    {
        OpcUa_ReadValueId *rv = &nodesToRead[i];
        SOPC_DataValue *dv = &dvs[i];

        /* As ownership is given to the caller, we have to copy all values */
        /* TODO: IndexRange and DataEncoding */
        const SOPC_DataValue *src = SOPC_Dict_Get(g_cache, &rv->NodeId, NULL);
        status = SOPC_DataValue_Copy(dv, src);

        /* As we have ownership of the rv, clear it */
        OpcUa_ReadValueId_Clear(rv);
    }
    SOPC_Free(nodesToRead);

    if (SOPC_STATUS_OK != status)
    {
        for (int32_t i=0; i < nbValues; ++i)
        {
            SOPC_DataValue_Clear(&dvs[i]);
        }
        SOPC_Free(dvs);
        dvs = NULL;
    }

    return dvs;
}

/* nodesToWrite shall be freed by the callee */
bool Cache_SetTargetVariables(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    assert(NULL != nodesToWrite && nbValues > 0);
    assert(INT32_MAX < SIZE_MAX || nbValues <= SIZE_MAX);

    bool ok = true;
    for (int32_t i=0; ok && i < nbValues; ++i)
    {
        /* TODO: IndexRange */
        /* Note: the cache frees both the key and the value, so we have to give it new ones */
        OpcUa_WriteValue *wv = &nodesToWrite[i];
        SOPC_NodeId *nid = &wv->NodeId;
        SOPC_DataValue *dv = &wv->Value;
        /* Divert the NodeId and the DataValue (avoid a complete copy) from the OpcUa_WriteValue and give them to our dict */
        SOPC_NodeId *key = SOPC_Malloc(sizeof(SOPC_NodeId));
        SOPC_DataValue *item = SOPC_Malloc(sizeof(SOPC_DataValue));
        ok &= NULL != key && NULL != item;
        if (ok)
        {
            *key = *nid;
            SOPC_NodeId_Initialize(nid);
            assert(NULL != item);
            *item = *dv;
            SOPC_DataValue_Initialize(dv);

            ok = Cache_Set(key, item);
        }

        /* As we have ownership of the wv, clear it */
        OpcUa_WriteValue_Clear(wv);
    }
    SOPC_Free(nodesToWrite);

    return ok;
}

void Cache_Clear(void)
{
    SOPC_Dict_Delete(g_cache);
    g_cache = NULL;
}
