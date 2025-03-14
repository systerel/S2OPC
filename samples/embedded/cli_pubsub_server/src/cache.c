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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "samples_platform_dep.h"

#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

// project includes
#include "cache.h"
#include "test_config.h"

#define SAFE_STRING(s) ((NULL == (s)) ? "<NULL>" : (s))
/**
 * The cache is a simple dictionnary { SOPC_NodeId* : SOPC_DataValue*}
 * The elements and keys are not referenced anywhere else, thus references must be cleared
 * when an item is removed.
 * Of course, keys and elements MUST be MALLOCed.
 */
SOPC_Dict* g_cache = NULL;
SOPC_Mutex g_lock;

static void free_datavalue(uintptr_t value)
{
    SOPC_DataValue_Clear((void*) value);
    SOPC_Free((void*) value);
}

static SOPC_VariantArrayType valueRankToArrayType(const int32_t valueRank)
{
    SOPC_VariantArrayType result = SOPC_VariantArrayType_SingleValue;
    if (valueRank < 0)
    {
        // Use Single value if possible
        result = SOPC_VariantArrayType_SingleValue;
    }
    else if (valueRank < 2)
    {
        // Use Array if possible
        result = SOPC_VariantArrayType_Array;
    }
    else if (valueRank == 2)
    {
        // Use Matrix
        result = SOPC_VariantArrayType_Matrix;
    }
    else
    {
        SOPC_ASSERT(false && "Cannot create variables with dimensions > 2");
    }
    return result;
}

bool Cache_UpdateVariant(SOPC_BuiltinId type, SOPC_VariantValue* variant, const char* value)
{
    SOPC_ASSERT(NULL != variant && NULL != value);

    bool result = true;
    int64_t iValue;
    if (value[0] == 0)
    {
        iValue = 0;
    }
    else if (SOPC_strncmp_ignore_case(value, "0x", 2) == 0)
    {
        iValue = (int64_t) strtoll(value + 2, NULL, 0x10);
    }
    else
    {
        iValue = (int64_t) strtoll(value, NULL, 10);
    }
    const float fValue = (float) atof(value);

    switch (type)
    {
    case SOPC_Null_Id:
        result = false;
        break;
    case SOPC_Boolean_Id:
        variant->Boolean = (SOPC_Boolean) iValue;
        break;
    case SOPC_SByte_Id:
        variant->Sbyte = (SOPC_SByte) iValue;
        break;
    case SOPC_Byte_Id:
        variant->Byte = (SOPC_Byte) iValue;
        break;
    case SOPC_Int16_Id:
        variant->Int16 = (int16_t) iValue;
        break;
    case SOPC_UInt16_Id:
        variant->Uint16 = (uint16_t) iValue;
        break;
    case SOPC_Int32_Id:
        variant->Int32 = (int32_t) iValue;
        break;
    case SOPC_UInt32_Id:
        variant->Uint32 = (uint32_t) iValue;
        break;
    case SOPC_Int64_Id:
        variant->Int64 = (int64_t) iValue;
        break;
    case SOPC_UInt64_Id:
        variant->Uint64 = (uint64_t) iValue;
        break;
    case SOPC_Float_Id:
        variant->Floatv = (float) fValue;
        break;
    case SOPC_Double_Id:
        variant->Doublev = (double) fValue;
        break;
    case SOPC_String_Id:
        result = (SOPC_STATUS_OK == SOPC_String_InitializeFromCString(&variant->String, value));
        break;
    case SOPC_DateTime_Id:
        variant->Date = SOPC_Time_GetCurrentTimeUTC();
        break;
    case SOPC_ByteString_Id:
        result = (SOPC_STATUS_OK ==
                  SOPC_ByteString_CopyFromBytes(&variant->Bstring, (const SOPC_Byte*) value, (int32_t) strlen(value)));
        break;
    case SOPC_StatusCode_Id:
        variant->Status = (SOPC_StatusCode) iValue;
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
        result = false;
        break;
    }

    return result;
}

/* Create a new datavalue with a default false/0/now/empty value of given scalar built-in type */
static SOPC_DataValue* new_datavalue(SOPC_BuiltinId type, const SOPC_VariantArrayType arrayType)
{
    SOPC_DataValue* dv = SOPC_Calloc(1, sizeof(SOPC_DataValue));
    if (NULL == dv)
    {
        return NULL;
    }

    /* Note: for now, only the Variant part of the DataValue is used by the publisher */
    SOPC_DataValue_Initialize(dv);
    SOPC_Variant* var = &dv->Value;
    SOPC_Variant_Initialize(var);
    var->BuiltInTypeId = type;
    var->ArrayType = arrayType;

    switch (arrayType)
    {
    case SOPC_VariantArrayType_SingleValue:
        Cache_UpdateVariant(type, &var->Value, "");
        break;

    case SOPC_VariantArrayType_Array:
        // Set "Empty array"
        var->Value.Array.Length = 0;
        break;

    case SOPC_VariantArrayType_Matrix:
        var->Value.Matrix.Dimensions = 2;
        var->Value.Matrix.ArrayDimensions = SOPC_Calloc(2, sizeof(uint32_t));
        SOPC_ASSERT(var->Value.Matrix.ArrayDimensions != NULL);
        var->Value.Matrix.ArrayDimensions[0] = 0;
        var->Value.Matrix.ArrayDimensions[1] = 0;
        break;
    default:
        SOPC_ASSERT(false && "Cannot create default empty value for Matrixes with Dimensions > 2");
        break;
    }

    return dv;
}

bool Cache_Initialize(SOPC_PubSubConfiguration* config, bool noSubCache)
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

    SOPC_ReturnStatus status = SOPC_Mutex_Initialization(&g_lock);
    g_cache = SOPC_NodeId_Dict_Create(true, free_datavalue);
    bool res = SOPC_STATUS_OK == status && NULL != g_cache;

    /* Parse configuration and fill the cache with default values */
    /* NodeId is in the field metadata (target variable for subscriber and published variable for publisher)
     *  and fields are in DataSetMetaData, which is found in reader groups of subscriber connections,
     *  and in published data set for publishers */
    const uint32_t nbPubConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    for (uint32_t i = 0; res && i < nbPubConnection; ++i)
    {
        /* TODO: it might be shorter to directly Get_PublishedDataSet_At */
        const SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, i);
        const uint16_t nbWriterGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
        for (uint16_t j = 0; res && j < nbWriterGroup; ++j)
        {
            const SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);
            const uint8_t nbDataSetWriter = SOPC_WriterGroup_Nb_DataSetWriter(group);
            for (uint8_t k = 0; res && k < nbDataSetWriter; ++k)
            {
                const SOPC_DataSetWriter* writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, k);
                const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
                const uint16_t nbFields = SOPC_PublishedDataSet_Nb_FieldMetaData(dataset);
                for (uint16_t l = 0; res && l < nbFields; ++l)
                {
                    const SOPC_FieldMetaData* metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, l);
                    const SOPC_BuiltinId type = SOPC_FieldMetaData_Get_BuiltinType(metadata);
                    const int32_t valueRank = SOPC_FieldMetaData_Get_ValueRank(metadata);

                    /* Retrieve and copy the NodeId for insertion in the Cache */
                    const SOPC_PublishedVariable* target = SOPC_FieldMetaData_Get_PublishedVariable(metadata);
                    const SOPC_NodeId* src = SOPC_PublishedVariable_Get_NodeId(target);
                    SOPC_NodeId* nid = SOPC_Calloc(1, sizeof(SOPC_NodeId));
                    res &= (NULL != nid);

                    if (res)
                    {
                        status = SOPC_NodeId_Copy(nid, src);
                        if (SOPC_STATUS_OK != status)
                        {
                            res = false;
                        }
                    }

                    /* Insert a DataValue initialized to a default value */
                    if (res)
                    {
                        SOPC_DataValue* dv = new_datavalue(type, valueRankToArrayType(valueRank));
                        res &= Cache_Set(nid, dv); /* Both will be freed by the cache */
                    }

                    if (res)
                    {
                        char* nid_str = SOPC_NodeId_ToCString(nid);
                        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Cache: inserted %s of type %d", nid_str, type);
                        SOPC_Free(nid_str);
                    }
                    else
                    {
                        SOPC_Free(nid);
                    }
                }
            }
        }
    }

    const uint32_t nbSubConnection = SOPC_PubSubConfiguration_Nb_SubConnection(config);
    for (uint32_t i = 0; res && i < nbSubConnection && false == noSubCache; ++i)
    {
        const SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, i);
        const uint16_t nbReaderGroup = SOPC_PubSubConnection_Nb_ReaderGroup(connection);
        for (uint16_t j = 0; res && j < nbReaderGroup; ++j)
        {
            const SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, j);
            const uint8_t nbDataSetReader = SOPC_ReaderGroup_Nb_DataSetReader(readerGroup);
            for (uint8_t k = 0; res && k < nbDataSetReader; ++k)
            {
                const SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, k);
                const uint16_t nbFields = SOPC_DataSetReader_Nb_FieldMetaData(dataSetReader);
                for (uint16_t l = 0; res && l < nbFields; ++l)
                {
                    const SOPC_FieldMetaData* metadata = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, l);
                    const SOPC_BuiltinId type = SOPC_FieldMetaData_Get_BuiltinType(metadata);
                    const int32_t valueRank = SOPC_FieldMetaData_Get_ValueRank(metadata);

                    /* Retrieve and copy the NodeId for insertion in the Cache */
                    const SOPC_FieldTarget* target = SOPC_FieldMetaData_Get_TargetVariable(metadata);
                    const SOPC_NodeId* src = SOPC_FieldTarget_Get_NodeId(target);
                    SOPC_NodeId* nid = SOPC_Calloc(1, sizeof(SOPC_NodeId));
                    res &= (NULL != nid);

                    if (res)
                    {
                        status = SOPC_NodeId_Copy(nid, src);
                        if (SOPC_STATUS_OK != status)
                        {
                            res = false;
                        }
                    }

                    /* Insert a DataValue initialized to a default value */
                    if (res)
                    {
                        SOPC_DataValue* dv = new_datavalue(type, valueRankToArrayType(valueRank));
                        res &= Cache_Set(nid, dv); /* Both will be freed by the cache */
                    }

                    if (res)
                    {
                        char* nid_str = SOPC_NodeId_ToCString(nid);
                        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Cache: inserted %s of type %d", nid_str, type);
                        SOPC_Free(nid_str);
                    }
                    else
                    {
                        SOPC_Free(nid);
                    }
                }
            }
        }
    }

    return res;
}

SOPC_DataValue* Cache_Get(const SOPC_NodeId* nid)
{
    SOPC_ASSERT(NULL != g_cache);
    return (SOPC_DataValue*) SOPC_Dict_Get(g_cache, (uintptr_t) nid, NULL);
}

bool Cache_Set(SOPC_NodeId* nid, SOPC_DataValue* dv)
{
    SOPC_ASSERT(NULL != g_cache);
    return SOPC_Dict_Insert(g_cache, (uintptr_t) nid, (uintptr_t) dv);
}

/* Returned DataValues will be freed by the caller */
SOPC_DataValue* Cache_GetSourceVariables(const OpcUa_ReadValueId* nodesToRead, const int32_t nbValues)
{
    SOPC_ASSERT(NULL != nodesToRead && nbValues > 0);
    SOPC_ASSERT(INT32_MAX < SIZE_MAX || nbValues <= SIZE_MAX);

    SOPC_DataValue* dvs = SOPC_Calloc((size_t) nbValues, sizeof(SOPC_DataValue));
    if (NULL == dvs)
    {
        return dvs;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    Cache_Lock();
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < nbValues; ++i)
    {
        const OpcUa_ReadValueId* rv = &nodesToRead[i];
        SOPC_DataValue* dv = &dvs[i];

        /* IndexRange and DataEncoding are not supported in this cache and should be empty */
        SOPC_ASSERT(rv->IndexRange.Length <= 0 && "IndexRange not supported");
        SOPC_ASSERT(rv->DataEncoding.NamespaceIndex == 0 && rv->DataEncoding.Name.Length <= 0 &&
                    "DataEncoding not supported");

        /* As ownership is given to the caller, we have to copy all values */
        SOPC_DataValue* src = (SOPC_DataValue*) SOPC_Dict_Get(g_cache, (uintptr_t) &rv->NodeId, NULL);
        status = SOPC_DataValue_Copy(dv, src);

        /* As we have ownership of the rv, clear it */
    }
    Cache_Unlock();

    if (SOPC_STATUS_OK != status)
    {
        for (int32_t i = 0; i < nbValues; ++i)
        {
            SOPC_DataValue_Clear(&dvs[i]);
        }
        SOPC_Free(dvs);
        dvs = NULL;
    }

    return dvs;
}

/* nodesToWrite shall be freed by the callee */
bool Cache_SetTargetVariables(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues)
{
    SOPC_ASSERT(NULL != nodesToWrite && nbValues > 0);
    SOPC_ASSERT(INT32_MAX < SIZE_MAX || nbValues <= SIZE_MAX);

    bool ok = true;
    Cache_Lock();
    for (int32_t i = 0; ok && i < nbValues; ++i)
    {
        /* Note: the cache frees both the key and the value, so we have to give it new ones */
        const OpcUa_WriteValue* wv = &nodesToWrite[i];
        const SOPC_NodeId* nid = &wv->NodeId;
        const SOPC_DataValue* dv = &wv->Value;

        /* IndexRange is not supported in this cache and should be empty */
        SOPC_ASSERT(wv->IndexRange.Length <= 0 && "IndexRange not supported");

        /* Copy the NodeId and the DataValue from the OpcUa_WriteValue and give them to our dict */
        SOPC_NodeId* key = SOPC_Malloc(sizeof(SOPC_NodeId));
        SOPC_DataValue* item = SOPC_Malloc(sizeof(SOPC_DataValue));
        ok &= NULL != key && NULL != item;
        if (ok)
        {
            SOPC_ReturnStatus status = SOPC_NodeId_Copy(key, nid);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
            status = SOPC_DataValue_Copy(item, dv);
            SOPC_ASSERT(SOPC_STATUS_OK == status);

            ok = Cache_Set(key, item);
        }
        else
        {
            SOPC_Free(key);
            SOPC_Free(item);
        }
    }
    Cache_Unlock();

    return ok;
}

void Cache_Dump_VarValue(const SOPC_NodeId* nid, const SOPC_DataValue* dv)
{
    char* nidStr = SOPC_NodeId_ToCString(nid);
    SOPC_ASSERT(NULL != nid && NULL != nidStr);

    PRINT("- %.25s", nidStr);

    if (NULL != dv)
    {
        int result = -1;
        static char status[22];
        if (dv->Status & SOPC_BadStatusMask)
        {
            result = sprintf(status, "BAD 0x%08" PRIX32, dv->Status);
        }
        else if (dv->Status & SOPC_UncertainStatusMask)
        {
            result = sprintf(status, "UNCERTAIN 0x%08" PRIX32, dv->Status);
        }
        else
        {
            result = sprintf(status, "GOOD");
        }
        SOPC_ASSERT(result > 0);

        const char* type = "";
        switch (dv->Value.ArrayType)
        {
        case SOPC_VariantArrayType_Matrix:
            type = " ; [MAT]";
            break;
        case SOPC_VariantArrayType_Array:
            type = " ; [ARR]";
            break;
        case SOPC_VariantArrayType_SingleValue:
        default:
            break;
        }
        PRINT(" ; Status = %s%s", status, type);
        if (type[0] == 0)
        {
            static const char* typeName[] = {
                "Null",           "Boolean",       "SByte",         "Byte",          "Int16",           "UInt16",
                "Int32",          "UInt32",        "Int64",         "UInt64",        "Float",           "Double",
                "String",         "DateTime",      "Guid",          "ByteString",    "XmlElement",      "NodeId",
                "ExpandedNodeId", "StatusCode",    "QualifiedName", "LocalizedText", "ExtensionObject", "DataValue",
                "Variant",        "DiagnosticInfo"};

            const SOPC_BuiltinId typeId = dv->Value.BuiltInTypeId;
            if (typeId <= sizeof(typeName) / sizeof(*typeName) - 1)
            {
                PRINT(" ; Type=%.12s ; Val = ", typeName[typeId]);
            }
            else
            {
                PRINT(" ; Type=%.12s ; Val = ", "<Invalid>");
            }

            switch (typeId)
            {
            case SOPC_Boolean_Id:
                PRINT("%" PRId32, (int32_t) dv->Value.Value.Boolean);
                break;
            case SOPC_SByte_Id:
                PRINT("%" PRId32, (uint32_t) dv->Value.Value.Sbyte);
                break;
            case SOPC_Byte_Id:
                PRINT("%" PRId32, (int32_t) dv->Value.Value.Byte);
                break;
            case SOPC_UInt16_Id:
                PRINT("%" PRIu16, dv->Value.Value.Uint16);
                break;
            case SOPC_Int16_Id:
                PRINT("%" PRId16, dv->Value.Value.Int16);
                break;
            case SOPC_Int32_Id:
                PRINT("%" PRId32, dv->Value.Value.Int32);
                break;
            case SOPC_UInt32_Id:
                PRINT("%" PRIu32, dv->Value.Value.Uint32);
                break;
            case SOPC_Int64_Id:
                PRINT("%" PRId64, dv->Value.Value.Int64);
                break;
            case SOPC_UInt64_Id:
                PRINT("%" PRIu64, dv->Value.Value.Uint64);
                break;
            case SOPC_Float_Id:
                PRINT("%f", (double) dv->Value.Value.Floatv);
                break;
            case SOPC_Double_Id:
                PRINT("%f", (double) dv->Value.Value.Doublev);
                break;
            case SOPC_String_Id:
                PRINT("<%s>", SAFE_STRING(SOPC_String_GetRawCString(&dv->Value.Value.String)));
                break;
            case SOPC_StatusCode_Id:
                PRINT("0x%08" PRIX32, dv->Value.Value.Status);
                break;
            default:
                PRINT("(...)");
                break;
            }
        }
    }
    else
    {
        PRINT("<no value>");
    }
    PRINT("\n");

    SOPC_Free(nidStr);
}

static void forEach_DoExec(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    SOPC_ASSERT(NULL != (void*) user_data);
    Cache_ForEach_Exec* exec = (Cache_ForEach_Exec*) user_data;
    exec->pExec((const SOPC_NodeId*) key, (SOPC_DataValue*) value);
}

static void Cache_ForEach_Dump(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    (void) user_data;
    Cache_Dump_VarValue((const SOPC_NodeId*) key, (SOPC_DataValue*) value);
}

void Cache_Dump_NodeId(const SOPC_NodeId* pNid)
{
    SOPC_ASSERT(NULL != g_cache);
    Cache_Lock();
    Cache_Dump_VarValue(pNid, Cache_Get(pNid));
    Cache_Unlock();
}

void Cache_ForEach(Cache_ForEach_Exec* exec)
{
    Cache_Lock();
    SOPC_Dict_ForEach(g_cache, &forEach_DoExec, (uintptr_t) exec);
    Cache_Unlock();
}

void Cache_Dump(void)
{
    SOPC_ASSERT(NULL != g_cache);
    PRINT("Warning: Dumping the cache may break real-time operations!\n");
    Cache_Lock();
    SOPC_Dict_ForEach(g_cache, &Cache_ForEach_Dump, (uintptr_t) NULL);
    Cache_Unlock();
}

void Cache_Lock(void)
{
    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&g_lock);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
}

void Cache_Unlock(void)
{
    SOPC_ReturnStatus status = SOPC_Mutex_Unlock(&g_lock);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
}

void Cache_Clear(void)
{
    SOPC_Mutex_Clear(&g_lock);
    SOPC_Dict_Delete(g_cache);
    g_cache = NULL;
}
