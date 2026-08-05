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

#include "util_fast_init_clear.h"

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_types.h"

void SOPC_WriteValue_FastInitialize(OpcUa_WriteValue* wv)
{
    SOPC_ASSERT(NULL != wv);
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

void SOPC_NotificationMessage_FastFreeContent(OpcUa_NotificationMessage* notifMsg)
{
    for (int32_t i = 0; i < notifMsg->NoOfNotificationData; i++)
    {
        SOPC_ExtensionObject* notifData = &notifMsg->NotificationData[i];
        if (notifData->Encoding == SOPC_ExtObjBodyEncoding_Object &&
            notifData->Body.Object.ObjType == &OpcUa_DataChangeNotification_EncodeableType)
        {
            // Fast free content
            OpcUa_DataChangeNotification* dataChangeNotif =
                (OpcUa_DataChangeNotification*) notifData->Body.Object.Value;
            SOPC_ASSERT(NULL == dataChangeNotif->DiagnosticInfos); // Not managed yet
            for (int32_t j = 0; j < dataChangeNotif->NoOfMonitoredItems; j++)
            {
                OpcUa_MonitoredItemNotification* mi = &dataChangeNotif->MonitoredItems[j];
                SOPC_DataValue_FastFreeContent(&mi->Value);
            }
            SOPC_Free(dataChangeNotif->MonitoredItems);
            SOPC_Free(dataChangeNotif);
            notifData->Encoding = SOPC_ExtObjBodyEncoding_None;
            notifData->Body.Object.ObjType = NULL;
            notifData->Body.Object.Value = NULL;
        }
        // Finalize clear (or realize it)
        SOPC_ExtensionObject_Clear(notifData);
    }
    SOPC_Free(notifMsg->NotificationData);
}
