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

/** \file
 *
 * \brief Provides various helpers for events fields filling.
 */

#include <string.h>

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_event_helpers.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

#include "opcua_identifiers.h"

/********************/
/* LOCAL VARIABLES  */
/********************/
static const SOPC_NodeId ServerObject_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server);

/********************/
/* EXTERN FUNCTIONS */
/********************/

/*********************************************************************************/
void SOPC_EventHelpers_SetNull(SOPC_Event* const event, const char* qnPath)
{
    if (NULL == event || NULL == qnPath)
    {
        return;
    }
    static const SOPC_Variant nullV = {true, SOPC_Null_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = 0}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &nullV);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach 'NULL' to event at '%s'.", qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetDate(SOPC_Event* const event, const char* qnPath, const SOPC_DateTime value)
{
    SOPC_ASSERT(NULL != event && NULL != qnPath);
    SOPC_Variant var = {false, SOPC_DateTime_Id, SOPC_VariantArrayType_SingleValue, {.Date = value}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach date to event at '%s'.", qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetDouble(SOPC_Event* const event, const char* qnPath, const double value)
{
    SOPC_ASSERT(NULL != event && NULL != qnPath);
    SOPC_Variant var = {false, SOPC_Double_Id, SOPC_VariantArrayType_SingleValue, {.Doublev = value}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach Double value to event at '%s'.",
                                 qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetBool(SOPC_Event* const event, const char* qnPath, const bool value)
{
    SOPC_ASSERT(NULL != event && NULL != qnPath);
    SOPC_Variant var = {false, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = value}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach Bool value to event at '%s'.", qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetUInt32(SOPC_Event* const event, const char* qnPath, const uint32_t value)
{
    if (NULL == event || NULL == qnPath)
    {
        return;
    }
    SOPC_Variant var = {false, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = value}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach UInt32 value to event at '%s'.",
                                 qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetInt32(SOPC_Event* const event, const char* qnPath, const int32_t value)
{
    if (NULL == event || NULL == qnPath)
    {
        return;
    }
    SOPC_Variant var = {false, SOPC_Int32_Id, SOPC_VariantArrayType_SingleValue, {.Int32 = value}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach Int32 value to event at '%s'.",
                                 qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetStatusCode(SOPC_Event* const event, const char* qnPath, const SOPC_StatusCode value)
{
    if (NULL == event || NULL == qnPath)
    {
        return;
    }
    SOPC_Variant var = {false, SOPC_StatusCode_Id, SOPC_VariantArrayType_SingleValue, {.Status = value}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach StatucCode value to event at '%s'.",
                                 qnPath);
    }
}

/*********************************************************************************/
void SOPC_EventHelpers_SetCString(SOPC_Event* const event, const char* qnPath, const char* value)
{
    if (NULL == event || NULL == qnPath)
    {
        return;
    }
    SOPC_Variant var;
    SOPC_Variant_Initialize(&var);
    var.BuiltInTypeId = SOPC_String_Id;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != value)
    {
        status = SOPC_String_CopyFromCString(&var.Value.String, value);
    }
    else
    {
        var.Value.String.Length = -1;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach CString '%s' to event at '%s'.", value,
                                 qnPath);
    }
    SOPC_Variant_Clear(&var);
}

/*********************************************************************************/
void SOPC_EventHelpers_SetString(SOPC_Event* const event, const char* qnPath, const SOPC_String* value)
{
    if (NULL == event || NULL == qnPath || NULL == value)
    {
        return;
    }
    SOPC_Variant var;
    SOPC_Variant_Initialize(&var);
    var.BuiltInTypeId = SOPC_String_Id;
    SOPC_ReturnStatus status = SOPC_String_Copy(&var.Value.String, value);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach String to event at '%s'.", qnPath);
    }
    SOPC_Variant_Clear(&var);
}
/*********************************************************************************/
void SOPC_EventHelpers_SetByteString(SOPC_Event* const event,
                                     const char* qnPath,
                                     const uint8_t* value,
                                     const uint32_t len)
{
    if (NULL == event || NULL == qnPath)
    {
        return;
    }
    SOPC_Variant var;
    SOPC_Variant_Initialize(&var);
    var.BuiltInTypeId = SOPC_ByteString_Id;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != value && len > 0)
    {
        status = SOPC_ByteString_CopyFromBytes(&var.Value.Bstring, value, (int32_t) len);
    }
    else
    {
        var.Value.Bstring.Length = -1; // NULL ByteString
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach ByteString to event at '%s'.", qnPath);
    }
    SOPC_Variant_Clear(&var);
}

/*********************************************************************************/
void SOPC_EventHelpers_SetNodeId(SOPC_Event* const event, const char* qnPath, const SOPC_NodeId* value)
{
    if (NULL == event || NULL == qnPath || NULL == value)
    {
        return;
    }
    SOPC_NodeId nidCopy = *value;
    SOPC_Variant var = {true, SOPC_NodeId_Id, SOPC_VariantArrayType_SingleValue, {.NodeId = &nidCopy}};
    SOPC_ReturnStatus status = SOPC_Event_SetVariableFromStrPath(event, qnPath, &var);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach NodeId value to event at '%s'.",
                                 qnPath);
    }
    // No need to clear variant (No allocated objects)
}

/*********************************************************************************/
void SOPC_EventsHelpers_SetBaseEventType(SOPC_Event* const event,
                                         const char* sourceNameC,
                                         const char* message,
                                         uint16_t severity)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // EventId already set by SOPC_ServerHelper_CreateEvent
    // EventType is set at object creation with SOPC_ServerHelper_CreateEvent
    // SourceNode : not set (null)
    // LocalTime : not supported in current version
    // SourceName (skipped if NULL)
    if (NULL != sourceNameC)
    {
        SOPC_String sourceName;
        SOPC_String_Initialize(&sourceName);
        status = SOPC_String_AttachFromCstring(&sourceName, sourceNameC);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetSourceName(event, &sourceName);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach SourceName '%s' to event.",
                                     sourceNameC);
        }
        SOPC_String_Clear(&sourceName);
    }

    // Time & ReceiveTime
    status = SOPC_Event_SetTime(event, SOPC_Time_GetCurrentTimeUTC());
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach Time to event.");
    }

    // Message
    SOPC_LocalizedText ltMsg;
    SOPC_LocalizedText_Initialize(&ltMsg);
    status = SOPC_String_CopyFromCString(&ltMsg.defaultText, message ? message : "");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetMessage(event, &ltMsg);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach Message '%s' to event.", message);
        }
    }
    SOPC_LocalizedText_Clear(&ltMsg);

    // Severity
    status = SOPC_Event_SetSeverity(event, severity);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to attach Severity to event.");
    }
}

/*********************************************************************************/
void SOPC_EventsHelpers_SetAuditEventType(SOPC_Event* const event,
                                          const char* auditEntryId,
                                          const SOPC_DateTime actionTimestamp,
                                          const char* serverUri,
                                          const char* clientUserId,
                                          const bool bStatus)
{
    // Check ClientAuditEntryId
    if (NULL != auditEntryId)
    {
        SOPC_EventHelpers_SetCString(event, "0:ClientAuditEntryId", auditEntryId);
    }
    else
    {
        SOPC_EventHelpers_SetCString(event, "0:ClientAuditEntryId",
                                     "Unknown client AuditEntryId"); // No available context.
    }
    SOPC_EventHelpers_SetDate(event, "0:ActionTimeStamp", actionTimestamp);
    SOPC_EventHelpers_SetBool(event, "0:Status", bStatus);
    SOPC_EventHelpers_SetCString(event, "0:ServerId", serverUri);
    SOPC_EventHelpers_SetCString(event, "0:ClientUserId", clientUserId);
}

/*********************************************************************************/
/** Set the attributes of an AuditSecurityEventType event. This function does not set inherited attributes */
void SOPC_EventsHelpers_SetAuditSecurityEventType(SOPC_Event* const event, const SOPC_StatusCode statusCode)
{
    SOPC_EventHelpers_SetStatusCode(event, "0:StatusCodeId", statusCode);
}

/*********************************************************************************/
void SOPC_EventsHelpers_SetAuditChannelEventType(SOPC_Event* const event, uint32_t secureChannelId)
{
    char scIdStr[11]; // 10 digits + EOL
    memset(scIdStr, 0, sizeof(scIdStr));
    snprintf(scIdStr, 11, "%" PRIu32, secureChannelId);

    SOPC_EventHelpers_SetCString(event, "0:SecureChannelId", scIdStr);
    // The SourceNode Property for Events of this type shall be assigned to the Server Object.
    SOPC_EventHelpers_SetNodeId(event, "0:SourceNode", &ServerObject_NodeId);
}
