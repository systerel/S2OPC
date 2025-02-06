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

#include "sopc_event_manager.h"

#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_address_space_utils_internal.h"
#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time_reference.h"
#include "sopc_types.h"

/* A reserved entry for empty path is defined to store the event NodeId
 * which shall be the event/node instance
 * NodeId if it exists in address space (see ConditionId in part 9) */
#define QN_EMPTY_PATH_NODEID "OwnNodeId"

typedef struct SOPC_Event_Variable
{
    SOPC_Variant data;
    SOPC_NodeId dataType;
    int32_t valueRank;
} SOPC_Event_Variable;

struct _SOPC_Event
{
    SOPC_Dict* qnPathToEventVar;
};

/* Reserved index for variants for any event */
typedef enum SOPC_BaseEventVariantIndexes
{
    SOPC_BaseEventVariantIdx_NodeId = 0,
    SOPC_BaseEventVariantIdx_EventId,
    SOPC_BaseEventVariantIdx_EventType,
    SOPC_BaseEventVariantIdx_SourceNode,
    SOPC_BaseEventVariantIdx_SourceName,
    SOPC_BaseEventVariantIdx_Time,
    SOPC_BaseEventVariantIdx_ReceiveTime,
    SOPC_BaseEventVariantIdx_LocalTime,
    SOPC_BaseEventVariantIdx_Message,
    SOPC_BaseEventVariantIdx_Severity,
    SOPC_BaseEventVariantIdx_ReservedLength // first free index
} SOPC_BaseEventVariantIndexes;

/* Statically defined variants for any event */
static const char* sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_ReservedLength] = {
    QN_EMPTY_PATH_NODEID, "0:EventId",   "0:EventType", "0:SourceNode", "0:SourceName", "0:Time",
    "0:ReceiveTime",      "0:LocalTime", "0:Message",   "0:Severity"};

// Notes:
// - On Windows: can't be const because MS build does not support &OpcUa_*_EncodeableType ref in definition for library
// - On PikeOS: ignore cast const necessary due to macros with empty strings
SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
static SOPC_Event_Variable sopc_baseEventVariants[SOPC_BaseEventVariantIdx_ReservedLength] = {
    {SOPC_VARIANT_NODEID(SOPC_NODEID_NS0_NUMERIC(0)), SOPC_NODEID_NS0_NUMERIC(OpcUaId_NodeId),
     -1}, // "OwnNodeId": placeholder to store the actual event/node instance NodeId (see ConditionId in part 9)
    {SOPC_VARIANT_BYTESTRING({0}), SOPC_NODEID_NS0_NUMERIC(OpcUaId_ByteString), -1},                // EventId
    {SOPC_VARIANT_NODEID(SOPC_NODEID_NS0_NUMERIC(0)), SOPC_NODEID_NS0_NUMERIC(OpcUaId_NodeId), -1}, // EventType
    {SOPC_VARIANT_NODEID(SOPC_NODEID_NS0_NUMERIC(0)), SOPC_NODEID_NS0_NUMERIC(OpcUaId_NodeId), -1}, // SourceNode
    {SOPC_VARIANT_STRING(""), SOPC_NODEID_NS0_NUMERIC(OpcUaId_String), -1},                         // SourceName
    {SOPC_VARIANT_DATE(0), SOPC_NODEID_NS0_NUMERIC(OpcUaId_UtcTime), -1},                           // Time
    {SOPC_VARIANT_DATE(0), SOPC_NODEID_NS0_NUMERIC(OpcUaId_UtcTime), -1},                           // ReceiveTime
    {{true,
      SOPC_ExtensionObject_Id,
      SOPC_VariantArrayType_SingleValue,
      {.ExtObject = (SOPC_ExtensionObject[]){{{SOPC_NODEID_NS0_NUMERIC(OpcUaId_TimeZoneDataType), {0, 0, NULL}, 0},
                                              SOPC_ExtObjBodyEncoding_Object,
                                              .Body.Object = {(OpcUa_TimeZoneDataType[]){{NULL, 0, 0}}, NULL}}}}},
     SOPC_NODEID_NS0_NUMERIC(OpcUaId_TimeZoneDataType),
     -1},                                                                                 // LocalTime
    {SOPC_VARIANT_LOCALTEXT("", ""), SOPC_NODEID_NS0_NUMERIC(OpcUaId_LocalizedText), -1}, // Message
    {SOPC_VARIANT_UINT16(0), SOPC_NODEID_NS0_NUMERIC(OpcUaId_UInt16), -1},                // Severity
};
SOPC_GCC_DIAGNOSTIC_RESTORE

static SOPC_NodeId baseEventNodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_BaseEventType);

static uint64_t eventIdCounter = 0;

static SOPC_ReturnStatus SOPC_InternalEventManagerUtil_QnPathToCString(uint16_t nbQnPath,
                                                                       const SOPC_QualifiedName* qualifiedNamePathArray,
                                                                       char** qnPathStr)
{
    if (NULL == qnPathStr || (NULL == qualifiedNamePathArray && 0 != nbQnPath))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Specific management of internal empty path which always represents event/node instance NodeId
    if (0 == nbQnPath)
    {
        *qnPathStr = SOPC_strdup(QN_EMPTY_PATH_NODEID);
        if (NULL == *qnPathStr)
        {
            return SOPC_STATUS_OUT_OF_MEMORY;
        }
        return SOPC_STATUS_OK;
    }
    return SOPC_EventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, qnPathStr);
}

static void SOPC_EventVariable_Delete(SOPC_Event_Variable** ppEventVar)
{
    if (NULL != ppEventVar && NULL != *ppEventVar)
    {
        SOPC_Event_Variable* evar = *ppEventVar;
        SOPC_Variant_Clear(&evar->data);
        SOPC_NodeId_Clear(&evar->dataType);
        SOPC_Free(evar);
        *ppEventVar = NULL;
    }
}

static void SOPC_EventVariable_Initialize(SOPC_Event_Variable* pEventVar)
{
    SOPC_NodeId_Initialize(&pEventVar->dataType);
    SOPC_Variant_Initialize(&pEventVar->data);
    pEventVar->valueRank = -1; // SCALAR
}

size_t SOPC_Event_GetNbVariables(SOPC_Event* event)
{
    if (NULL == event || NULL == event->qnPathToEventVar)
    {
        return 0;
    }
    return SOPC_Dict_Size(event->qnPathToEventVar);
}

static SOPC_Event_Variable* SOPC_EventVariable_CreateFrom(const SOPC_Variant* data,
                                                          const SOPC_NodeId* dataType,
                                                          int32_t valueRank)
{
    if (NULL == data || NULL == dataType || valueRank < -3)
    {
        return NULL;
    }
    SOPC_Event_Variable* eventVar = SOPC_Calloc(1, sizeof(*eventVar));
    if (NULL != eventVar)
    {
        SOPC_EventVariable_Initialize(eventVar);
        eventVar->valueRank = valueRank;
        SOPC_ReturnStatus status = SOPC_NodeId_Copy(&eventVar->dataType, dataType);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Variant_Copy(&eventVar->data, data);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_EventVariable_Delete(&eventVar);
        }
    }
    return eventVar;
}

static SOPC_Event_Variable* SOPC_EventVariable_CreateCopy(const SOPC_Event_Variable* source)
{
    return SOPC_EventVariable_CreateFrom(&source->data, &source->dataType, source->valueRank);
}

static SOPC_ReturnStatus generate_unique_event_id(SOPC_ByteString* eventId)
{
    SOPC_TimeReference tref = SOPC_TimeReference_GetCurrent();
    SOPC_ByteString_Clear(eventId);
    eventId->DoNotClear = false;
    eventId->Data = SOPC_Calloc(1, sizeof(tref) + sizeof(eventIdCounter) + 8);
    eventId->Length = (int32_t)(sizeof(tref) + sizeof(eventIdCounter) + 8);
    SOPC_ReturnStatus status = (NULL == eventId->Data ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    SOPC_CryptoProvider* pCrypto = NULL;
    SOPC_ExposedBuffer* expBuffer = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pCrypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_GenerateRandomBytes(pCrypto, 8, &expBuffer);
    }
    if (SOPC_STATUS_OK == status)
    {
        memcpy(eventId->Data, &tref, sizeof(tref));
        eventIdCounter++;
        memcpy(eventId->Data + sizeof(tref), &eventIdCounter, sizeof(eventIdCounter));
        memcpy(eventId->Data + sizeof(tref) + sizeof(eventIdCounter), expBuffer, 8);
    }
    else
    {
        SOPC_ByteString_Clear(eventId);
    }
    SOPC_CryptoProvider_Free(pCrypto);
    SOPC_Free(expBuffer);

    return status;
}

const SOPC_NodeId* SOPC_Event_GetEventTypeId(const SOPC_Event* pEvent)
{
    if (NULL == pEvent)
    {
        return NULL;
    }
    bool found = false;
    SOPC_Event_Variable* eventIdVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventType],
        &found);

    if (found && SOPC_NodeId_Id == eventIdVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == eventIdVarInfo->data.ArrayType)
    {
        return eventIdVarInfo->data.Value.NodeId;
    }
    return NULL;
}

static SOPC_ByteString* SOPC_InternalEvent_GetEventId(const SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* eventIdVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventId],
        &found);

    if (found && SOPC_ByteString_Id == eventIdVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == eventIdVarInfo->data.ArrayType)
    {
        return &eventIdVarInfo->data.Value.Bstring;
    }
    return NULL;
}

const SOPC_ByteString* SOPC_Event_GetEventId(const SOPC_Event* pEvent)
{
    return SOPC_InternalEvent_GetEventId(pEvent);
}

SOPC_ReturnStatus SOPC_Event_SetEventId(SOPC_Event* pEvent, const SOPC_ByteString* pEventId)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar || NULL == pEventId || pEventId->Length <= 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ByteString* bs = SOPC_InternalEvent_GetEventId(pEvent);
    SOPC_ReturnStatus status = NULL != bs ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ByteString_Clear(bs);
        status = SOPC_ByteString_Copy(bs, pEventId);
    }
    return status;
}

static SOPC_String* SOPC_Event_GetSourceName(SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* sourceNameVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_SourceName],
        &found);

    if (found && SOPC_String_Id == sourceNameVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == sourceNameVarInfo->data.ArrayType)
    {
        return &sourceNameVarInfo->data.Value.Bstring;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetSourceName(SOPC_Event* pEvent, const SOPC_String* pSourceName)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar || NULL == pSourceName || pSourceName->Length <= 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_String* sn = SOPC_Event_GetSourceName(pEvent);
    SOPC_ReturnStatus status = NULL != sn ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Clear(sn);
        status = SOPC_String_Copy(sn, pSourceName);
    }
    return status;
}

static SOPC_NodeId* SOPC_Event_GetSourceNode(SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* sourceNodeVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_SourceNode],
        &found);

    if (found && SOPC_NodeId_Id == sourceNodeVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == sourceNodeVarInfo->data.ArrayType)
    {
        return sourceNodeVarInfo->data.Value.NodeId;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetSourceNode(SOPC_Event* pEvent, const SOPC_NodeId* pSourceNode)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar || NULL == pSourceNode)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_NodeId* sn = SOPC_Event_GetSourceNode(pEvent);
    SOPC_ReturnStatus status = NULL != sn ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_NodeId_Clear(sn);
        status = SOPC_NodeId_Copy(sn, pSourceNode);
    }
    return status;
}

SOPC_DateTime SOPC_Event_GetTime(const SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* timeVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_Time], &found);

    if (found && SOPC_DateTime_Id == timeVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == timeVarInfo->data.ArrayType)
    {
        return timeVarInfo->data.Value.Date;
    }
    return -1;
}

static SOPC_ReturnStatus event_set_date(SOPC_Event* pEvent, SOPC_BaseEventVariantIndexes dateIdx, SOPC_DateTime date)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool found = false;
    SOPC_Event_Variable* dateVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[dateIdx], &found);

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (found && SOPC_DateTime_Id == dateVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == dateVarInfo->data.ArrayType)
    {
        status = SOPC_STATUS_OK;
        dateVarInfo->data.Value.Date = date;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Event_SetTime(SOPC_Event* pEvent, SOPC_DateTime time)
{
    return event_set_date(pEvent, SOPC_BaseEventVariantIdx_Time, time);
}

SOPC_ReturnStatus SOPC_Event_SetReceiveTime(SOPC_Event* pEvent, SOPC_DateTime receiveTime)
{
    return event_set_date(pEvent, SOPC_BaseEventVariantIdx_ReceiveTime, receiveTime);
}

static OpcUa_TimeZoneDataType* SOPC_Event_GetLocalTime(SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* localTimeVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_LocalTime],
        &found);

    if (found && SOPC_ExtensionObject_Id == localTimeVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == localTimeVarInfo->data.ArrayType &&
        SOPC_ExtObjBodyEncoding_Object == localTimeVarInfo->data.Value.ExtObject->Encoding &&
        &OpcUa_TimeZoneDataType_EncodeableType == localTimeVarInfo->data.Value.ExtObject->Body.Object.ObjType)
    {
        return (OpcUa_TimeZoneDataType*) localTimeVarInfo->data.Value.ExtObject->Body.Object.Value;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetLocalTime(SOPC_Event* pEvent, const OpcUa_TimeZoneDataType* pLocalTime)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar || NULL == pLocalTime ||
        &OpcUa_TimeZoneDataType_EncodeableType != pLocalTime->encodeableType)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_TimeZoneDataType* localTime = SOPC_Event_GetLocalTime(pEvent);
    SOPC_ReturnStatus status = NULL != localTime ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_TimeZoneDataType_Clear(localTime);
        status = SOPC_EncodeableObject_Copy(localTime->encodeableType, localTime, pLocalTime);
    }
    return status;
}

static SOPC_LocalizedText* SOPC_Event_GetMessage(SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* messageVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_Message],
        &found);

    if (found && SOPC_LocalizedText_Id == messageVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == messageVarInfo->data.ArrayType)
    {
        return messageVarInfo->data.Value.LocalizedText;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetMessage(SOPC_Event* pEvent, const SOPC_LocalizedText* pMessage)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar || NULL == pMessage)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_LocalizedText* msg = SOPC_Event_GetMessage(pEvent);
    SOPC_ReturnStatus status = NULL != msg ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_LocalizedText_Clear(msg);
        status = SOPC_LocalizedText_Copy(msg, pMessage);
    }
    return status;
}

SOPC_ReturnStatus SOPC_Event_SetSeverity(SOPC_Event* pEvent, uint16_t severity)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool found = false;
    SOPC_Event_Variable* severityVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_Severity],
        &found);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    if (found && SOPC_UInt16_Id == severityVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == severityVarInfo->data.ArrayType)
    {
        severityVarInfo->data.Value.Uint16 = severity;
        status = SOPC_STATUS_OK;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Event_SetVariableFromStrPath(SOPC_Event* pEvent, const char* qnPath, const SOPC_Variant* var)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool found = false;
    SOPC_Event_Variable* eventVarInfo =
        (SOPC_Event_Variable*) SOPC_Dict_Get(pEvent->qnPathToEventVar, (uintptr_t) qnPath, &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_NOT_SUPPORTED;
    if (SOPC_STATUS_OK == status)
    {
        /* TODO: set with typechecking ? */
        SOPC_Variant_Clear(&eventVarInfo->data);
        status = SOPC_Variant_Copy(&eventVarInfo->data, var);
    }
    return status;
}

const SOPC_Variant* SOPC_Event_GetVariableAndTypeFromStrPath(const SOPC_Event* pEvent,
                                                             const char* qnPath,
                                                             const SOPC_NodeId** outDataType,
                                                             int32_t* outValueRank)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar)
    {
        return NULL;
    }
    bool found = false;
    SOPC_Event_Variable* eventVarInfo =
        (SOPC_Event_Variable*) SOPC_Dict_Get(pEvent->qnPathToEventVar, (uintptr_t) qnPath, &found);

    if (found && NULL != outDataType)
    {
        *outDataType = &eventVarInfo->dataType;
    }
    if (found && NULL != outValueRank)
    {
        *outValueRank = eventVarInfo->valueRank;
    }
    if (found)
    {
        return &eventVarInfo->data;
    }
    return NULL;
}

const SOPC_Variant* SOPC_Event_GetVariableAndType(const SOPC_Event* pEvent,
                                                  uint16_t nbQnPath,
                                                  SOPC_QualifiedName* qualifiedNamePathArray,
                                                  const SOPC_NodeId** outDataType,
                                                  int32_t* outValueRank)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar)
    {
        return NULL;
    }
    const SOPC_Variant* result = NULL;
    char* qnPathStr = NULL;
    SOPC_ReturnStatus status =
        SOPC_InternalEventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, &qnPathStr);
    if (SOPC_STATUS_OK == status)
    {
        result = SOPC_Event_GetVariableAndTypeFromStrPath(pEvent, qnPathStr, outDataType, outValueRank);
    }
    SOPC_Free(qnPathStr);
    return result;
}

const SOPC_Variant* SOPC_Event_GetVariableFromStrPath(const SOPC_Event* pEvent, const char* qnPath)
{
    if (NULL == qnPath)
    {
        return NULL;
    }
    const char* qnPathWithEmpty = NULL;
    if ('\0' == *qnPath)
    {
        qnPathWithEmpty = QN_EMPTY_PATH_NODEID;
    }
    else
    {
        qnPathWithEmpty = qnPath;
    }
    return SOPC_Event_GetVariableAndTypeFromStrPath(pEvent, qnPathWithEmpty, NULL, NULL);
}

static SOPC_NodeId* SOPC_Event_GetNodeId(SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Event_Variable* nodeIdVarInfo = (SOPC_Event_Variable*) SOPC_Dict_Get(
        pEvent->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_NodeId],
        &found);

    if (found && SOPC_NodeId_Id == nodeIdVarInfo->data.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == nodeIdVarInfo->data.ArrayType)
    {
        return nodeIdVarInfo->data.Value.NodeId;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetNodeId(SOPC_Event* pEvent, const SOPC_NodeId* pNodeId)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToEventVar || NULL == pNodeId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_NodeId* nId = SOPC_Event_GetNodeId(pEvent);
    SOPC_ReturnStatus status = NULL != nId ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_NodeId_Clear(nId);
        status = SOPC_NodeId_Copy(nId, pNodeId);
    }
    return status;
}

typedef struct SOPC_Dict_ForEachEventVar_LocalData
{
    SOPC_Event_ForEachVar_Fct* func;
    uintptr_t user_data;
} SOPC_Dict_ForEachEventVar_LocalData;

static void SOPC_Dict_ForEachEventVar_Fct(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    SOPC_Dict_ForEachEventVar_LocalData* localData = (SOPC_Dict_ForEachEventVar_LocalData*) user_data;
    SOPC_Event_Variable* eventVar = (SOPC_Event_Variable*) value;
    // Avoid specific case of internal empty path which is the event/node instance NodeId
    if (0 != SOPC_strcmp_ignore_case(QN_EMPTY_PATH_NODEID, (const char*) key))
    {
        localData->func((const char*) key, &eventVar->data, &eventVar->dataType, eventVar->valueRank,
                        localData->user_data);
    }
}

void SOPC_Event_ForEachVar(SOPC_Event* event, SOPC_Event_ForEachVar_Fct* func, uintptr_t user_data)
{
    SOPC_Dict_ForEachEventVar_LocalData localData = {func, user_data};
    SOPC_Dict_ForEach(event->qnPathToEventVar, SOPC_Dict_ForEachEventVar_Fct, (uintptr_t) &localData);
}

SOPC_ReturnStatus SOPC_EventManagerUtil_QnPathToCString(uint16_t nbQnPath,
                                                        const SOPC_QualifiedName* qualifiedNamePathArray,
                                                        char** qnPathStr)
{
    if (NULL == qnPathStr || (NULL == qualifiedNamePathArray && 0 != nbQnPath))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* prevStr = SOPC_strdup("");
    if (NULL == prevStr)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    char* newStr = NULL;
    // Manage empty path case
    if (0 == nbQnPath)
    {
        newStr = prevStr;
        prevStr = NULL;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint16_t i = 0; i < nbQnPath && SOPC_STATUS_OK == status; i++)
    {
        char* qnStr = SOPC_QualifiedName_ToCString(&qualifiedNamePathArray[i]);
        if (NULL != qnStr)
        {
            // Check separator is not present in the qualified name
            char* sepInQnStr = strchr(qnStr, QN_PATH_SEPARATOR_CHAR);
            if (NULL != sepInQnStr)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
            // Concatenate separator if not the first element in the string path
            char* tmpPrevStr = NULL;
            if (SOPC_STATUS_OK == status)
            {
                if (0 != i)
                {
                    status = SOPC_StrConcat(prevStr, QN_PATH_SEPARATOR_STR, &tmpPrevStr);
                }
                else
                {
                    tmpPrevStr = prevStr;
                }
            }
            // Concatenate element to the string path
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_StrConcat(tmpPrevStr, qnStr, &newStr);
                if (0 != i)
                {
                    SOPC_Free(tmpPrevStr);
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                // Note: if SOPC_STATUS_OK != status => prevStr == newStr which will be freed after loop
                SOPC_Free(prevStr);
                prevStr = newStr;
            }
            SOPC_Free(qnStr);
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        if (prevStr != newStr)
        {
            SOPC_Free(prevStr);
            prevStr = NULL;
        }
        SOPC_Free(newStr);
        newStr = NULL;
    }
    else
    {
        *qnPathStr = newStr;
    }
    return status;
}

SOPC_ReturnStatus SOPC_EventManagerUtil_cStringPathToQnPath(char qnPathSep,
                                                            const char* qnPathStr,
                                                            int32_t* nbQnPath,
                                                            SOPC_QualifiedName** qNamePath)
{
    if ('\0' == qnPathSep || NULL == qnPathStr || NULL == nbQnPath || NULL == qNamePath || NULL != *qNamePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    size_t nbElts = 1;
    SOPC_QualifiedName* resultPath = NULL;
    char* pathCopy = NULL;
    const size_t pathLength = strlen(qnPathStr);
    if (0 == pathLength)
    {
        // Empty path
        *nbQnPath = 0;
        return SOPC_STATUS_OK;
    }
    // Compute the number of elements if no error occurs
    for (size_t i = 0; i < pathLength; i++)
    {
        if (qnPathSep == qnPathStr[i] && i != 0 && i != pathLength - 1 && qnPathStr[i - 1] != '\\')
        {
            // Note: i=0 and i=pathLength cases considered invalid since it means empty element
            //       and will lead to parsing failure anyway.
            nbElts++;
        }
    }
    if (nbElts > INT32_MAX)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Allocate the result qualified name array
    resultPath = SOPC_Calloc(nbElts, sizeof(*resultPath));
    SOPC_ReturnStatus status = (NULL == resultPath ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);

    // Make a copy of input string to manage removal of escape characters
    if (SOPC_STATUS_OK == status)
    {
        pathCopy = SOPC_strdup(qnPathStr);
        status = (NULL == pathCopy ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    }
    char* pathEltStart = pathCopy;
    char* pathEltCurrent = pathCopy;
    char* pathEltEnd = pathCopy;
    bool foundEnd = false;
    size_t eltIdx = 0;
    while (SOPC_STATUS_OK == status && NULL != pathEltEnd)
    {
        if (foundEnd)
        {
            // Prepare parsing of next path element
            pathEltStart = pathEltEnd + 1;
            pathEltCurrent = pathEltStart;
            foundEnd = false;
        }
        // Search for next separator
        pathEltEnd = strchr(pathEltCurrent, qnPathSep);

        if (pathEltStart == pathEltEnd)
        {
            // Empty element is not allowed
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        // End of element: end of full path string reached (NULL) or non-escaped separator
        else if (NULL == pathEltEnd || *(pathEltEnd - 1) != '\\')
        {
            if (NULL != pathEltEnd)
            {
                *pathEltEnd = '\0'; // terminate element as a C string for QN parsing
            }                       // else: end of element is already end of full path string
            // Parse the qualified name
            status = SOPC_QualifiedName_ParseCString(&resultPath[eltIdx], pathEltStart);
            eltIdx++;
            foundEnd = true;
        }
        // escaped character found before the separator => do not consider as separator
        else
        {
            // Remove escape character by shifting characters to the left
            memmove(pathEltEnd - 1, pathEltEnd, strlen(pathEltEnd) + 1);
            pathEltCurrent = pathEltEnd;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        // In case of success the number of elements shall be equal to the index value
        SOPC_ASSERT(eltIdx == nbElts);
        *qNamePath = resultPath;
        *nbQnPath = (int32_t) nbElts;
    }
    else
    {
        for (size_t i = 0; i < eltIdx; i++)
        {
            SOPC_QualifiedName_Clear(&resultPath[i]);
        }
        SOPC_Free(resultPath);
    }
    SOPC_Free(pathCopy);
    return status;
}

SOPC_ReturnStatus SOPC_Event_SetVariable(SOPC_Event* pEvent,
                                         uint16_t nbQnPath,
                                         SOPC_QualifiedName* qualifiedNamePathArray,
                                         const SOPC_Variant* var)
{
    char* strQnPath = NULL;
    SOPC_ReturnStatus status =
        SOPC_InternalEventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, &strQnPath);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(pEvent, strQnPath, var);
    }
    SOPC_Free(strQnPath);
    return status;
}

static uint64_t str_hash(const uintptr_t data)
{
    return SOPC_DJBHash((const uint8_t*) data, strlen((const char*) data));
}

static bool str_equal(const uintptr_t a, const uintptr_t b)
{
    return strcmp((const char*) a, (const char*) b) == 0;
}

static void uintptr_t_free(uintptr_t elt)
{
    SOPC_Free((void*) elt);
}

static void event_var_uintptr_t_free(uintptr_t elt)
{
    SOPC_Event_Variable* evar = (SOPC_Event_Variable*) elt;
    SOPC_EventVariable_Delete(&evar);
}

static const uintptr_t DICT_TOMBSTONE = UINTPTR_MAX;

// Create a event context to populate with the specific event type variables
static SOPC_Event* SOPC_EventBuilder_CreateCtx(void)
{
    SOPC_Event* result = SOPC_Calloc(1, sizeof(*result));

    bool failed = (NULL == result);
    if (!failed)
    {
        result->qnPathToEventVar =
            SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, event_var_uintptr_t_free);
        failed = (NULL == result->qnPathToEventVar);
        if (!failed)
        {
            SOPC_Dict_SetTombstoneKey(result->qnPathToEventVar, DICT_TOMBSTONE);
            // Initialize the base event type variables common to all event types
            for (size_t i = 0; !failed && i < SOPC_BaseEventVariantIdx_ReservedLength; i++)
            {
                SOPC_Event_Variable* eventVarCopy = NULL;
                char* qnPathCopy = SOPC_strdup(sopc_baseEventVariantsPaths[i]);
                failed = (NULL == qnPathCopy);
                if (!failed)
                {
                    eventVarCopy = SOPC_EventVariable_CreateCopy(&sopc_baseEventVariants[i]);
                    failed = (NULL == eventVarCopy);
                }
                if (!failed)
                {
                    failed =
                        !(SOPC_Dict_Insert(result->qnPathToEventVar, (uintptr_t) qnPathCopy, (uintptr_t) eventVarCopy));
                }
                if (failed)
                {
                    SOPC_Free(qnPathCopy);
                    SOPC_EventVariable_Delete(&eventVarCopy);
                }
            }
        }
        if (failed)
        {
            SOPC_Dict_Delete(result->qnPathToEventVar);
            SOPC_Free(result);
            result = NULL;
        }
    }
    return result;
}

void SOPC_Event_Clear(SOPC_Event* pEvent)
{
    if (NULL == pEvent)
    {
        return;
    }
    SOPC_Dict_Delete(pEvent->qnPathToEventVar);
    pEvent->qnPathToEventVar = NULL;
}

void SOPC_Event_Delete(SOPC_Event** ppEvent)
{
    if (NULL == ppEvent)
    {
        return;
    }
    SOPC_Event_Clear(*ppEvent);
    SOPC_Free(*ppEvent);
    *ppEvent = NULL;
}

static void SOPC_Event_Dict_CopyForEach(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    SOPC_Event* eventDest = (SOPC_Event*) user_data;
    if (NULL != eventDest->qnPathToEventVar)
    {
        char* keyCopy = SOPC_strdup((const char*) key);
        bool failed = (NULL == keyCopy);
        SOPC_Event_Variable* eventVarCopy = NULL;
        if (!failed)
        {
            eventVarCopy = SOPC_EventVariable_CreateCopy((const SOPC_Event_Variable*) value);
            failed = (NULL == eventVarCopy);
        }
        if (!failed)
        {
            failed = !(SOPC_Dict_Insert(eventDest->qnPathToEventVar, (uintptr_t) keyCopy, (uintptr_t) eventVarCopy));
        }
        if (failed)
        {
            SOPC_Free(keyCopy);
            SOPC_Event_Clear(eventDest);
            SOPC_EventVariable_Delete(&eventVarCopy);
        }
    }
}

SOPC_Event* SOPC_Event_CreateCopy(const SOPC_Event* pEvent, bool genNewId)
{
    bool failed = (NULL == pEvent || NULL == pEvent->qnPathToEventVar);
    SOPC_Event* eventCopy = NULL;
    if (!failed)
    {
        eventCopy = SOPC_Calloc(1, sizeof(*eventCopy));
        failed = (NULL == eventCopy);
    }
    if (!failed)
    {
        eventCopy->qnPathToEventVar =
            SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, event_var_uintptr_t_free);
        failed = (NULL == eventCopy->qnPathToEventVar);
    }
    if (!failed)
    {
        SOPC_Dict_SetTombstoneKey(eventCopy->qnPathToEventVar, DICT_TOMBSTONE);
        SOPC_Dict_ForEach(pEvent->qnPathToEventVar, SOPC_Event_Dict_CopyForEach, (uintptr_t) eventCopy);
        failed = (NULL == eventCopy->qnPathToEventVar);
    }
    if (!failed)
    {
        if (genNewId)
        {
            SOPC_ByteString* eventId = SOPC_InternalEvent_GetEventId(eventCopy);
            SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
            if (NULL != eventId)
            {
                status = generate_unique_event_id(eventId);
            }
            if (SOPC_STATUS_OK != status)
            {
                failed = true;
            }
        }
    }
    if (failed)
    {
        SOPC_Event_Delete(&eventCopy);
    }
    return eventCopy;
}

static bool is_base_event_type(SOPC_AddressSpace_Node* eventType)
{
    return (OPCUA_NAMESPACE_INDEX == eventType->data.object.NodeId.Namespace &&
            SOPC_IdentifierType_Numeric == eventType->data.object.NodeId.IdentifierType &&
            OpcUaId_BaseEventType == eventType->data.object.NodeId.Data.Numeric);
}

static SOPC_ReturnStatus build_new_browsePath(const char* baseQnPath,
                                              SOPC_AddressSpace* addSpace,
                                              SOPC_AddressSpace_Node* currentNode,
                                              char** newQnPath)
{
    char* tmpNewQnPath = NULL;
    SOPC_QualifiedName* bn = SOPC_AddressSpace_Get_BrowseName(addSpace, currentNode);
    SOPC_ASSERT(NULL != bn);
    char* qnStr = SOPC_QualifiedName_ToCString(bn);
    SOPC_ReturnStatus status = (NULL == qnStr ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        char* sepInQnStr = strchr(qnStr, QN_PATH_SEPARATOR_CHAR);
        if (NULL != sepInQnStr)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != baseQnPath)
        {
            status = SOPC_StrConcat(baseQnPath, QN_PATH_SEPARATOR_STR, &tmpNewQnPath);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_StrConcat(tmpNewQnPath, qnStr, newQnPath);
            }
            SOPC_Free(qnStr);
            SOPC_Free(tmpNewQnPath);
        }
        else
        {
            *newQnPath = qnStr;
        }
    }
    else
    {
        SOPC_Free(qnStr);
    }
    return status;
}

// Build recursively the event type reference content
static SOPC_ReturnStatus SOPC_EventBuilder_RecPopulateEventTypeVariables(const char* baseQnPath,
                                                                         SOPC_AddressSpace* addSpace,
                                                                         SOPC_AddressSpace_Node* eventType,
                                                                         SOPC_AddressSpace_Node* currentNode,
                                                                         SOPC_Event* eventCtx,
                                                                         uint16_t depth)
{
    if (depth >= RECURSION_LIMIT)
    {
        return SOPC_STATUS_WOULD_BLOCK;
    }
    depth++;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_NodeClass* nodeClass = SOPC_AddressSpace_Get_NodeClass(addSpace, currentNode);

    // Update browse path
    char* newQnPath = NULL;
    bool deleteNewQnPath = true;
    if (depth > 1)
    {
        status = build_new_browsePath(baseQnPath, addSpace, currentNode, &newQnPath);
    } // First node is root node for the path, do not add its browse name

    // Register QNpath -> Value if the node is a variable
    if (SOPC_STATUS_OK == status && OpcUa_NodeClass_Variable == *nodeClass)
    {
        SOPC_ASSERT(NULL != newQnPath);
        bool foundKey = false;
        uintptr_t key = SOPC_Dict_GetKey(eventCtx->qnPathToEventVar, (uintptr_t) newQnPath, &foundKey);
        SOPC_UNUSED_RESULT(key);
        // Keep already added QNpath->value associated if the type is the base event type
        // or if the QNpath is the EventType Id
        if (foundKey && !is_base_event_type(eventType) &&
            (0 != strcmp(sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventType], newQnPath)))
        {
            // Otherwise remove previous value to set the new default value
            // (it might be a redefinition of variable with more precise data type...)
            SOPC_Dict_Remove(eventCtx->qnPathToEventVar, (uintptr_t) newQnPath);
            foundKey = false;
        }
        if (!foundKey)
        {
            // Retrieve default value if set
            SOPC_Variant* defValue = SOPC_AddressSpace_Get_Value(addSpace, currentNode);
            SOPC_NodeId* dataType = SOPC_AddressSpace_Get_DataType(addSpace, currentNode);
            int32_t* pValueRank = SOPC_AddressSpace_Get_ValueRank(addSpace, currentNode);
            SOPC_ASSERT(NULL != defValue);
            SOPC_Event_Variable* eventVar = SOPC_EventVariable_CreateFrom(defValue, dataType, *pValueRank);
            status = (NULL == eventVar ? SOPC_STATUS_OUT_OF_MEMORY : status);
            if (SOPC_STATUS_OK == status)
            {
                bool bres = SOPC_Dict_Insert(eventCtx->qnPathToEventVar, (uintptr_t) newQnPath, (uintptr_t) eventVar);
                status = (bres ? status : SOPC_STATUS_NOK);
                deleteNewQnPath = !bres;
            }
            if (SOPC_STATUS_OK != status)
            {
                SOPC_EventVariable_Delete(&eventVar);
            }
        }
    }

    // Recursively iterate on object/variable children node
    // Iterate on child nodes to populate the event browse path and associated variables
    SOPC_AddressSpace_Node* childNode = NULL;
    OpcUa_NodeClass* childNodeClass = NULL;
    bool foundNode = false;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, currentNode);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, currentNode);
    for (int32_t i = 0; SOPC_STATUS_OK == status && i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];
        foundNode = false;
        if (SOPC_AddressSpaceUtil_IsComponent(ref) || SOPC_AddressSpaceUtil_IsProperty(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                childNode = SOPC_AddressSpace_Get_Node(addSpace, &ref->TargetId.NodeId, &foundNode);
            }
            if (foundNode)
            {
                childNodeClass = SOPC_AddressSpace_Get_NodeClass(addSpace, childNode);
                // Ignores method nodes (and invalid node classes regarding part 3)
                if (OpcUa_NodeClass_Object == *childNodeClass || OpcUa_NodeClass_Variable == *childNodeClass)
                {
                    status = SOPC_EventBuilder_RecPopulateEventTypeVariables(newQnPath, addSpace, eventType, childNode,
                                                                             eventCtx, depth);
                }
            }
            else
            {
                status = SOPC_STATUS_WOULD_BLOCK;
            }
        }
    }
    if (deleteNewQnPath)
    {
        SOPC_Free(newQnPath);
    }
    return status;
}

// Create the event type reference for the given EventType node
static SOPC_ReturnStatus SOPC_EventBuilder_PopulateEventTypeVariables(SOPC_AddressSpace* addSpace,
                                                                      SOPC_AddressSpace_Node* eventTypeNode,
                                                                      SOPC_Event* eventCtx)
{
    // Retrieve and set the EventTypeId value
    SOPC_NodeId* eventTypeId = SOPC_AddressSpace_Get_NodeId(addSpace, eventTypeNode);
    bool found = false;
    SOPC_Event_Variable* varTypeIdDest = (SOPC_Event_Variable*) SOPC_Dict_Get(
        eventCtx->qnPathToEventVar, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventType],
        &found);
    SOPC_ASSERT(found);
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(varTypeIdDest->data.Value.NodeId, eventTypeId);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    status = SOPC_EventBuilder_RecPopulateEventTypeVariables(NULL, addSpace, eventTypeNode, eventTypeNode, eventCtx, 0);

    if (SOPC_STATUS_OK != status)
    {
        char* strNodeId = SOPC_NodeId_ToCString(eventTypeId);
        if (SOPC_STATUS_WOULD_BLOCK == status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Event ObjectType %s is defined using unknown nodes in the local address "
                                     "space or has too many node levels. "
                                     "It will not be possible to create event instances of this type.",
                                     strNodeId);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Building EventType %s failed: %d", strNodeId, status);
        }
        SOPC_Free(strNodeId);
    }
    return status;
}

// Create all event types references starting from the given type
static SOPC_ReturnStatus SOPC_EventBuilder_RecPopulateAllEventTypes(SOPC_AddressSpace* addSpace,
                                                                    SOPC_NodeId* eventTypeNodeId,
                                                                    SOPC_Event* eventCtx,
                                                                    SOPC_Server_Event_Types* eventTypesDict,
                                                                    uint16_t depth)
{
    SOPC_ASSERT(NULL != addSpace);
    SOPC_ASSERT(NULL != eventTypeNodeId);
    SOPC_ASSERT(NULL != eventCtx);
    SOPC_ASSERT(NULL != eventTypesDict);
    if (depth >= RECURSION_LIMIT)
    {
        SOPC_Event_Delete(&eventCtx);
        return SOPC_STATUS_WOULD_BLOCK;
    }
    depth++;
    bool found = false;
    SOPC_AddressSpace_Node* eventTypeNode = SOPC_AddressSpace_Get_Node(addSpace, eventTypeNodeId, &found);
    if (!found)
    {
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "SOPC_OpcUaEventManager: impossible to initialize event types, EventType node i=%d was not found",
            OpcUaId_BaseEventType);
        SOPC_Event_Delete(&eventCtx);
        return SOPC_STATUS_OK;
    }

    // Populate the event type content
    SOPC_ReturnStatus status = SOPC_EventBuilder_PopulateEventTypeVariables(addSpace, eventTypeNode, eventCtx);

    // Register the event type
    if (SOPC_STATUS_OK == status)
    {
        bool bres = SOPC_Dict_Insert(eventTypesDict, (uintptr_t) eventTypeNodeId, (uintptr_t) eventCtx);
        status = bres ? status : SOPC_STATUS_NOK;
    }

    // Recursively iterate on event type subtypes nodes
    if (SOPC_STATUS_OK == status)
    {
        int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, eventTypeNode);
        OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, eventTypeNode);
        for (int32_t i = 0; SOPC_STATUS_OK == status && i < *n_refs; ++i)
        {
            // Context is incremental for subtypes (inheritance of parent variables)
            OpcUa_ReferenceNode* ref = &(*refs)[i];
            if (SOPC_AddressSpaceUtil_IsHasSubtype(ref, false))
            {
                if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
                { // Shall be on same server and shall use only NodeId
                    SOPC_Event* incrEventCtx = SOPC_Event_CreateCopy(eventCtx, false);
                    if (NULL != incrEventCtx)
                    {
                        status = SOPC_EventBuilder_RecPopulateAllEventTypes(addSpace, &ref->TargetId.NodeId,
                                                                            incrEventCtx, eventTypesDict, depth);
                    }
                    else
                    {
                        status = SOPC_STATUS_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    char* strNodeId = SOPC_NodeId_ToCString(eventTypeNodeId);
                    char* strSubtypeId = SOPC_NodeId_ToCString(&ref->TargetId.NodeId);
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                             "EventType %s has unknown subtype node (SrvIdx=%" PRIu32
                                             ", Uri=%s, %s) in the local address space"
                                             "It will not be possible to create event instances of this subtype.",
                                             strNodeId, ref->TargetId.ServerIndex,
                                             SOPC_String_GetRawCString(&ref->TargetId.NamespaceUri), strSubtypeId);
                    SOPC_Free(strNodeId);
                    SOPC_Free(strSubtypeId);
                }
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        if (SOPC_STATUS_WOULD_BLOCK == status)
        {
            // Current type not added but keep going (warning was logged)
            status = SOPC_STATUS_OK;
        }
        SOPC_Event_Delete(&eventCtx);
    }

    return status;
}

// Create all event types references starting from the BaseEventTypes
static SOPC_ReturnStatus SOPC_EventBuilder_PopulateAllEventTypes(SOPC_AddressSpace* addSpace,
                                                                 SOPC_Server_Event_Types* eventTypesDict)
{
    SOPC_Event* eventDefaultCtx = SOPC_EventBuilder_CreateCtx();
    SOPC_ReturnStatus status = (NULL == eventDefaultCtx ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        bool found = false;
        SOPC_AddressSpace_Node* baseEventTypeNode = SOPC_AddressSpace_Get_Node(addSpace, &baseEventNodeId, &found);
        SOPC_UNUSED_RESULT(baseEventTypeNode); // only check existence
        if (found)
        {
            // Note: manage deallocation of eventDefaultCtx in case of failure
            status = SOPC_EventBuilder_RecPopulateAllEventTypes(addSpace, &baseEventNodeId, eventDefaultCtx,
                                                                eventTypesDict, 0);
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "SOPC_OpcUaEventManager: impossible to initialize event types, BaseEventType node "
                                     "i=%d was not found. Adding default BaseEventType only.",
                                     OpcUaId_BaseEventType);

            // Add default baseEventType as unique event type managed
            bool bres = SOPC_Dict_Insert(eventTypesDict, (uintptr_t) &baseEventNodeId, (uintptr_t) eventDefaultCtx);
            status = bres ? status : SOPC_STATUS_NOK;
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Event_Delete(&eventDefaultCtx);
            }
        }
    }
    return status;
}

static void SOPC_Event_Free_Fct(uintptr_t data)
{
    SOPC_Event_Clear((SOPC_Event*) data);
    SOPC_Free((SOPC_Event*) data);
}

SOPC_ReturnStatus SOPC_EventManager_CreateEventTypes(SOPC_AddressSpace* addSpace,
                                                     SOPC_Server_Event_Types** outEventTypesDict)
{
    if (NULL == outEventTypesDict || NULL == addSpace)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Initialize the LocalTime ExtensionObject object value since it was not done statically
    SOPC_Variant* localTime = &sopc_baseEventVariants[SOPC_BaseEventVariantIdx_LocalTime].data;
    OpcUa_TimeZoneDataType_Initialize(localTime->Value.ExtObject->Body.Object.Value);
    localTime->Value.ExtObject->Body.Object.ObjType = &OpcUa_TimeZoneDataType_EncodeableType;

    SOPC_Server_Event_Types* eventTypesDict = SOPC_NodeId_Dict_Create(false, SOPC_Event_Free_Fct);
    SOPC_ReturnStatus status = (NULL == eventTypesDict ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EventBuilder_PopulateAllEventTypes(addSpace, eventTypesDict);
    }
    if (SOPC_STATUS_OK == status)
    {
        *outEventTypesDict = eventTypesDict;
    }
    else
    {
        SOPC_Dict_Delete(eventTypesDict);
    }
    return status;
}

static SOPC_Event* getEventType(const SOPC_Server_Event_Types* eventTypes, const SOPC_NodeId* eventTypeId)
{
    SOPC_Event* eventInstDecl = NULL;
    if (NULL != eventTypes && NULL != eventTypeId)
    {
        bool found = false;
        eventInstDecl = (SOPC_Event*) SOPC_Dict_Get(eventTypes, (uintptr_t) eventTypeId, &found);
        SOPC_UNUSED_RESULT(found);
    }
    return eventInstDecl;
}

bool SOPC_EventManager_HasEventType(const SOPC_Server_Event_Types* eventTypes, const SOPC_NodeId* eventTypeId)
{
    SOPC_Event* eventInstDecl = getEventType(eventTypes, eventTypeId);
    return NULL != eventInstDecl;
}

SOPC_ReturnStatus SOPC_EventManager_HasEventTypeAndBrowsePath(const SOPC_Server_Event_Types* eventTypes,
                                                              const SOPC_NodeId* eventTypeId,
                                                              int32_t nbQNamePath,
                                                              const SOPC_QualifiedName* qNamePath)
{
    if (nbQNamePath > UINT16_MAX)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (nbQNamePath < 0)
    {
        nbQNamePath = 0;
    }
    SOPC_Event* eventInstDecl = getEventType(eventTypes, eventTypeId);
    if (NULL == eventInstDecl)
    {
        return SOPC_STATUS_NOK;
    }
    char* qnPathStr = NULL;
    SOPC_ReturnStatus status =
        SOPC_InternalEventManagerUtil_QnPathToCString((uint16_t) nbQNamePath, qNamePath, &qnPathStr);
    if (SOPC_STATUS_OK == status)
    {
        bool found = false;
        SOPC_Event_Variable* eventVar =
            (SOPC_Event_Variable*) SOPC_Dict_Get(eventInstDecl->qnPathToEventVar, (uintptr_t) qnPathStr, &found);
        SOPC_UNUSED_RESULT(eventVar);
        SOPC_Free(qnPathStr);
        status = found ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }
    return status;
}

SOPC_Event* SOPC_EventManager_CreateEventInstance(const SOPC_Server_Event_Types* eventTypes,
                                                  const SOPC_NodeId* eventTypeId)
{
    bool found = false;
    SOPC_Event* eventInst = NULL;
    SOPC_Event* eventInstDecl = (SOPC_Event*) SOPC_Dict_Get(eventTypes, (uintptr_t) eventTypeId, &found);
    if (found)
    {
        eventInst = SOPC_Event_CreateCopy(eventInstDecl, true);
    }

    return eventInst;
}

void SOPC_EventManager_Delete(SOPC_Server_Event_Types** eventTypes)
{
    if (NULL != eventTypes)
    {
        if (NULL != *eventTypes)
        {
            SOPC_Dict_Delete(*eventTypes);
        }
        *eventTypes = NULL;
    }
}
