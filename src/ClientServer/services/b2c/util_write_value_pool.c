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

/**
 * \file util_write_value_pool.c
 * \brief Implementation of the shared WriteValue static pool for data-changed events.
 */

#include "util_write_value_pool.h"

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_static_pool.h"
#include "sopc_toolkit_config_constants.h"

static OpcUa_WriteValue sopc_writeValuePoolBuffer[SOPC_WRITE_VALUES_POOL_SIZE];
static SOPC_StaticPool sopc_writeValuePool;

void SOPC_WriteValuePool_Initialize(void)
{
    SOPC_StaticPool_Init(&sopc_writeValuePool, sopc_writeValuePoolBuffer, sizeof(OpcUa_WriteValue),
                         SOPC_WRITE_VALUES_POOL_SIZE);
}

OpcUa_WriteValue* SOPC_WriteValuePool_Alloc(void)
{
    OpcUa_WriteValue* wv = (OpcUa_WriteValue*) SOPC_StaticPool_Alloc(&sopc_writeValuePool);
    if (NULL != wv)
    {
        SOPC_WriteValue_FastInitialize(wv);
    }
    return wv;
}

void SOPC_WriteValuePool_ClearAndFree(OpcUa_WriteValue* wv)
{
    if (NULL == wv)
    {
        return;
    }
    SOPC_WriteValue_FastFreeContent(wv);
    SOPC_StaticPool_Free(&sopc_writeValuePool, wv);
}

size_t SOPC_WriteValuePool_GetNbFallbackAllocsStat(void)
{
    return SOPC_StaticPool_GetNbFallbackAllocsStat(&sopc_writeValuePool);
}

/* Field list and per-field Initialize/Clear taken from OpcUa_WriteValue (sopc_types.h)
 * and its generated field descriptor table WriteValue_Fields[] (sopc_types.c), so that
 * this stays behaviorally identical to OpcUa_WriteValue_Initialize/Clear while avoiding
 * their generic reflective field-descriptor dispatch (SOPC_EncodeableObject_*). Do not
 * "optimize" this into relying on the pool's zero-init instead of calling Initialize:
 * SOPC_String_Initialize sets Length = -1, not 0, which zeroed memory would get wrong. */

void SOPC_WriteValue_FastInitialize(OpcUa_WriteValue* wv)
{
    wv->encodeableType = &OpcUa_WriteValue_EncodeableType;
    SOPC_NodeId_Initialize(&wv->NodeId);
    wv->AttributeId = 0;
    SOPC_String_Initialize(&wv->IndexRange);
    SOPC_DataValue_Initialize(&wv->Value);
    wv->writeByPassPermissionMask = 0;
}

static SOPC_STRONG_INLINE void SOPC_String_FastFreeContent(SOPC_String* string)
{
    SOPC_ASSERT(NULL != string);
    if (!string->DoNotClear)
    {
        SOPC_Free(string->Data);
    }
}

static SOPC_STRONG_INLINE void SOPC_NodeId_FastFreeContent(SOPC_NodeId* nodeId)
{
    SOPC_ASSERT(NULL != nodeId);
    switch (nodeId->IdentifierType)
    {
    case SOPC_IdentifierType_Numeric:
        break;
    case SOPC_IdentifierType_String:
    case SOPC_IdentifierType_ByteString:
        SOPC_String_FastFreeContent(&nodeId->Data.String);
        break;
    case SOPC_IdentifierType_Guid:
        SOPC_Free(nodeId->Data.Guid);
        break;
    default:
        break;
    }
}

static SOPC_STRONG_INLINE void SOPC_DataValue_FastFreeContent(SOPC_DataValue* dataValue)
{
    SOPC_Variant* variant = &dataValue->Value;
    if (variant->DoNotClear)
    {
        return;
    }
    else if (variant->ArrayType != SOPC_VariantArrayType_SingleValue || variant->BuiltInTypeId >= SOPC_LocalizedText_Id)
    {
        // Not optimized due to complexity
        SOPC_Variant_Clear(&dataValue->Value);
    }
    else if (variant->BuiltInTypeId >= SOPC_String_Id)
    {
        // SingleValue && SOPC_String_Id <= BuiltInTypeId < SOPC_LocalizedText_Id
        switch (variant->BuiltInTypeId)
        {
        case SOPC_String_Id:
        case SOPC_ByteString_Id:
        case SOPC_XmlElement_Id:
            SOPC_String_FastFreeContent(&variant->Value.String);
            break;
        case SOPC_DateTime_Id:
        case SOPC_StatusCode_Id:
            break;
        case SOPC_Guid_Id:
            SOPC_Free(variant->Value.Guid);
            break;
        case SOPC_NodeId_Id:
            if (NULL != variant->Value.NodeId)
            {
                SOPC_NodeId_FastFreeContent(variant->Value.NodeId);
                SOPC_Free(variant->Value.NodeId);
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            if (NULL != variant->Value.ExpNodeId)
            {
                SOPC_NodeId_FastFreeContent(&variant->Value.ExpNodeId->NodeId);
                SOPC_String_FastFreeContent(&variant->Value.ExpNodeId->NamespaceUri);
                SOPC_Free(variant->Value.ExpNodeId);
            }
            break;
        case SOPC_QualifiedName_Id:
            if (NULL != variant->Value.Qname)
            {
                SOPC_String_FastFreeContent(&variant->Value.Qname->Name);
                SOPC_Free(variant->Value.Qname);
            }
            break;
        default:
            SOPC_ASSERT(false && "Unexpected case reached !");
            break;
        }
    } // else: BuiltInTypeId < SOPC_String_Id => numeric types only: no op
}

void SOPC_WriteValue_FastFreeContent(OpcUa_WriteValue* wv)
{
    SOPC_NodeId_FastFreeContent(&wv->NodeId);
    SOPC_String_FastFreeContent(&wv->IndexRange);
    SOPC_DataValue_FastFreeContent(&wv->Value);
}
