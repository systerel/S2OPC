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
#include "sopc_platform_time.h"
#include "sopc_types.h"

#define QN_PATH_SEPARATOR_CHAR '~'
#define QN_PATH_SEPARATOR_STR "~"

/* Reserved index for variants for any event */
typedef enum SOPC_BaseEventVariantIndexes
{
    SOPC_BaseEventVariantIdx_EventId = 0,
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
    "0:EventId",     "0:EventType", "0:SourceNode", "0:SourceName", "0:Time",
    "0:ReceiveTime", "0:LocalTime", "0:Message",    "0:Severity"};

// Note: cannot be const because MS build does not support &OpcUa_*_EncodeableType reference in definition for library
static SOPC_Variant sopc_baseEventVariants[SOPC_BaseEventVariantIdx_ReservedLength] = {
    {true, SOPC_ByteString_Id, SOPC_VariantArrayType_SingleValue, {.Bstring = {0}}},             // EventId
    {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {.NodeId = (SOPC_NodeId[]){{0}}}}, // EventType
    {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {.NodeId = (SOPC_NodeId[]){{0}}}}, // SourceNode
    {true, SOPC_String_Id, SOPC_VariantArrayType_SingleValue, {.String = {0}}},                  // SourceName
    {true, SOPC_DateTime_Id, SOPC_VariantArrayType_SingleValue, {.Date = 0}},                    // Time
    {true, SOPC_DateTime_Id, SOPC_VariantArrayType_SingleValue, {.Date = 0}},                    // ReceiveTime
    {true,
     SOPC_ExtensionObject_Id,
     SOPC_VariantArrayType_SingleValue,
     {.ExtObject =
          (SOPC_ExtensionObject[]){{{SOPC_NS0_NUMERIC_NODEID(OpcUaId_TimeZoneDataType), {0, 0, NULL}, 0},
                                    SOPC_ExtObjBodyEncoding_Object,
                                    .Body.Object = {(OpcUa_TimeZoneDataType[]){{NULL, 0, 0}}, NULL}}}}}, // LocalTime
    {true,
     SOPC_LocalizedText_Id,
     SOPC_VariantArrayType_SingleValue,
     {.LocalizedText = (SOPC_LocalizedText[]){{{0}, {0}, NULL}}}},            // Message
    {true, SOPC_UInt16_Id, SOPC_VariantArrayType_SingleValue, {.Uint16 = 0}}, // Severity
};

struct _SOPC_Event
{
    SOPC_Dict* qnPathToVariant;
};

static SOPC_NodeId baseEventNodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_BaseEventType);

static uint64_t eventIdCounter = 0;

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
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventType], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_NodeId_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        return eventIdVar->Value.NodeId;
    }
    return NULL;
}

static SOPC_ByteString* SOPC_Event_GetEventId(SOPC_Event* pEvent)
{
    SOPC_ASSERT(NULL != pEvent);
    bool found = false;
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventId], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_ByteString_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        return &eventIdVar->Value.Bstring;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetEventId(SOPC_Event* pEvent, const SOPC_ByteString* pEventId)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant || NULL == pEventId || pEventId->Length <= 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ByteString* bs = SOPC_Event_GetEventId(pEvent);
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
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_SourceName], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_String_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        return &eventIdVar->Value.Bstring;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetSourceName(SOPC_Event* pEvent, const SOPC_String* pSourceName)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant || NULL == pSourceName || pSourceName->Length <= 0)
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
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_SourceNode], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_NodeId_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        return eventIdVar->Value.NodeId;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetSourceNode(SOPC_Event* pEvent, const SOPC_NodeId* pSourceNode)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant || NULL == pSourceNode)
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
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_Time], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_DateTime_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        return eventIdVar->Value.Date;
    }
    return -1;
}

static SOPC_ReturnStatus event_set_date(SOPC_Event* pEvent, SOPC_BaseEventVariantIndexes dateIdx, SOPC_DateTime date)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool found = false;
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(pEvent->qnPathToVariant,
                                                             (uintptr_t) sopc_baseEventVariantsPaths[dateIdx], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_DateTime_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        eventIdVar->Value.Date = date;
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
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_LocalTime], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_ExtensionObject_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == eventIdVar->Value.ExtObject->Encoding);
        SOPC_ASSERT(&OpcUa_TimeZoneDataType_EncodeableType == eventIdVar->Value.ExtObject->Body.Object.ObjType);
        return (OpcUa_TimeZoneDataType*) eventIdVar->Value.ExtObject->Body.Object.Value;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetLocalTime(SOPC_Event* pEvent, const OpcUa_TimeZoneDataType* pLocalTime)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant || NULL == pLocalTime ||
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
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_Message], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_LocalizedText_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        return eventIdVar->Value.LocalizedText;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_Event_SetMessage(SOPC_Event* pEvent, const SOPC_LocalizedText* pMessage)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant || NULL == pMessage)
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
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool found = false;
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(
        pEvent->qnPathToVariant, (uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_Severity], &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_UInt16_Id == eventIdVar->BuiltInTypeId);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == eventIdVar->ArrayType);
        eventIdVar->Value.Uint16 = severity;
    }
    return status;
}

SOPC_ReturnStatus SOPC_Event_SetVariableFromStrPath(SOPC_Event* pEvent, const char* qnPath, const SOPC_Variant* var)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool found = false;
    SOPC_Variant* eventIdVar = (SOPC_Variant*) SOPC_Dict_Get(pEvent->qnPathToVariant, (uintptr_t) qnPath, &found);
    SOPC_ReturnStatus status = found ? SOPC_STATUS_OK : SOPC_STATUS_NOT_SUPPORTED;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Variant_Clear(eventIdVar);
        status = SOPC_Variant_Copy(eventIdVar, var);
    }
    return status;
}

const SOPC_Variant* SOPC_Event_GetVariableFromStrPath(const SOPC_Event* pEvent, const char* qnPath)
{
    if (NULL == pEvent || NULL == pEvent->qnPathToVariant)
    {
        return NULL;
    }
    bool found = false;
    SOPC_Variant* eventVar = (SOPC_Variant*) SOPC_Dict_Get(pEvent->qnPathToVariant, (uintptr_t) qnPath, &found);
    if (found)
    {
        return eventVar;
    }
    return NULL;
}

SOPC_ReturnStatus SOPC_EventManagerUtil_QnPathToCString(uint16_t nbQnPath,
                                                        const SOPC_QualifiedName* qualifiedNamePathArray,
                                                        char** qnPathStr)
{
    SOPC_ASSERT(NULL != qnPathStr);
    SOPC_ASSERT(NULL != qualifiedNamePathArray);
    SOPC_ASSERT(nbQnPath > 0);
    char* prevStr = SOPC_strdup("");
    if (NULL == prevStr)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    char* newStr = NULL;
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

SOPC_ReturnStatus SOPC_Event_SetVariable(SOPC_Event* pEvent,
                                         uint16_t nbQnPath,
                                         SOPC_QualifiedName* qualifiedNamePathArray,
                                         const SOPC_Variant* var)
{
    char* strQnPath = NULL;
    SOPC_ReturnStatus status = SOPC_EventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, &strQnPath);
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

static void variant_uintptr_t_free(uintptr_t elt)
{
    SOPC_Variant_Delete((SOPC_Variant*) elt);
}

static const uintptr_t DICT_TOMBSTONE = UINTPTR_MAX;

// Create a event context to populate with the specific event type variables
static SOPC_Event* SOPC_EventBuilder_CreateCtx(void)
{
    SOPC_Event* result = SOPC_Calloc(1, sizeof(*result));

    bool failed = NULL == result;
    if (!failed)
    {
        result->qnPathToVariant =
            SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, variant_uintptr_t_free);
        failed = (NULL == result->qnPathToVariant);
        if (!failed)
        {
            SOPC_Dict_SetTombstoneKey(result->qnPathToVariant, DICT_TOMBSTONE);
            // Initialize the base event type variables common to all event types
            for (size_t i = 0; !failed && i < SOPC_BaseEventVariantIdx_ReservedLength; i++)
            {
                SOPC_Variant* variantCopy = SOPC_Variant_Create();
                char* qnPathCopy = SOPC_strdup(sopc_baseEventVariantsPaths[i]);
                failed = (NULL == variantCopy || NULL == qnPathCopy);
                if (!failed)
                {
                    SOPC_ReturnStatus status = SOPC_Variant_Copy(variantCopy, &sopc_baseEventVariants[i]);
                    failed = (SOPC_STATUS_OK != status);
                }
                if (!failed)
                {
                    failed =
                        !(SOPC_Dict_Insert(result->qnPathToVariant, (uintptr_t) qnPathCopy, (uintptr_t) variantCopy));
                }
                if (failed)
                {
                    SOPC_Free(qnPathCopy);
                    SOPC_Variant_Delete(variantCopy);
                }
            }
        }
        if (failed)
        {
            SOPC_Dict_Delete(result->qnPathToVariant);
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
    SOPC_Dict_Delete(pEvent->qnPathToVariant);
    pEvent->qnPathToVariant = NULL;
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
    if (NULL != eventDest->qnPathToVariant)
    {
        char* keyCopy = SOPC_strdup((const char*) key);
        bool failed = NULL == keyCopy;
        SOPC_Variant* variantCopy = NULL;
        if (!failed)
        {
            variantCopy = SOPC_Variant_Create();
            failed = NULL == variantCopy;
        }
        if (!failed)
        {
            SOPC_ReturnStatus status = SOPC_Variant_Copy(variantCopy, (const SOPC_Variant*) value);
            failed = SOPC_STATUS_OK != status;
        }
        if (!failed)
        {
            failed = !(SOPC_Dict_Insert(eventDest->qnPathToVariant, (uintptr_t) keyCopy, (uintptr_t) variantCopy));
        }
        if (failed)
        {
            SOPC_Free(keyCopy);
            SOPC_Event_Clear(eventDest);
            SOPC_Variant_Delete(variantCopy);
        }
    }
}

SOPC_Event* SOPC_Event_CreateCopy(const SOPC_Event* pEvent, bool genNewId)
{
    bool failed = (NULL == pEvent || NULL == pEvent->qnPathToVariant);
    SOPC_Event* eventCopy = NULL;
    if (!failed)
    {
        eventCopy = SOPC_Calloc(1, sizeof(*eventCopy));
        failed = NULL == eventCopy;
    }
    if (!failed)
    {
        eventCopy->qnPathToVariant =
            SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, variant_uintptr_t_free);
        failed = NULL == eventCopy->qnPathToVariant;
    }
    if (!failed)
    {
        SOPC_Dict_SetTombstoneKey(eventCopy->qnPathToVariant, DICT_TOMBSTONE);
        SOPC_Dict_ForEach(pEvent->qnPathToVariant, SOPC_Event_Dict_CopyForEach, (uintptr_t) eventCopy);
        failed = NULL == eventCopy->qnPathToVariant;
    }
    if (!failed)
    {
        if (genNewId)
        {
            SOPC_ByteString* eventId = SOPC_Event_GetEventId(eventCopy);
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
        uintptr_t key = SOPC_Dict_GetKey(eventCtx->qnPathToVariant, (uintptr_t) newQnPath, &foundKey);
        SOPC_UNUSED_RESULT(key);
        // Keep already added QNpath->value associated if the type is the base event type
        // or if the QNpath is the EventType Id
        if (foundKey && !is_base_event_type(eventType) &&
            (0 != strcmp(sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventType], newQnPath)))
        {
            // Otherwise remove previous value to set the new default value
            // (it might be a redefinition of variable with more precise data type...)
            SOPC_Dict_Remove(eventCtx->qnPathToVariant, (uintptr_t) newQnPath);
            foundKey = false;
        }
        if (!foundKey)
        {
            // Retrieve default value if set
            SOPC_Variant* defValue = SOPC_AddressSpace_Get_Value(addSpace, currentNode);
            SOPC_ASSERT(NULL != defValue);
            SOPC_Variant* valueCopy = SOPC_Variant_Create();
            status = (NULL == valueCopy ? SOPC_STATUS_OUT_OF_MEMORY : status);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Variant_Copy(valueCopy, defValue);
            }
            if (SOPC_STATUS_OK == status)
            {
            }
            if (SOPC_STATUS_OK == status)
            {
                bool bres = SOPC_Dict_Insert(eventCtx->qnPathToVariant, (uintptr_t) newQnPath, (uintptr_t) valueCopy);
                status = (bres ? status : SOPC_STATUS_NOK);
                deleteNewQnPath = !bres;
            }
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Variant_Delete(valueCopy);
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
    SOPC_Variant* varTypeIdDest = (SOPC_Variant*) SOPC_Dict_Get(
        eventCtx->qnPathToVariant, (const uintptr_t) sopc_baseEventVariantsPaths[SOPC_BaseEventVariantIdx_EventType],
        &found);
    SOPC_ASSERT(found);
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(varTypeIdDest->Value.NodeId, eventTypeId);
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
    SOPC_Variant* localTime = &sopc_baseEventVariants[SOPC_BaseEventVariantIdx_LocalTime];
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
    if (nbQNamePath <= 0 || nbQNamePath > UINT16_MAX)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Event* eventInstDecl = getEventType(eventTypes, eventTypeId);
    if (NULL == eventInstDecl)
    {
        return SOPC_STATUS_NOK;
    }
    char* qnPathStr = NULL;
    SOPC_ReturnStatus status = SOPC_EventManagerUtil_QnPathToCString((uint16_t) nbQNamePath, qNamePath, &qnPathStr);
    if (SOPC_STATUS_OK == status)
    {
        bool found = false;
        SOPC_Variant* eventIdVar =
            (SOPC_Variant*) SOPC_Dict_Get(eventInstDecl->qnPathToVariant, (uintptr_t) qnPathStr, &found);
        SOPC_UNUSED_RESULT(eventIdVar);
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
