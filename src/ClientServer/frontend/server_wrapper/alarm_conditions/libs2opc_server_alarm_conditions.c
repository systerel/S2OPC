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

#include "libs2opc_server_alarm_conditions.h"

#include "libs2opc_internal_alarm_conditions.h"
#include "libs2opc_server_internal.h"

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_event_manager.h"
#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include <string.h>

static void SOPC_InternalCreate_SetEventVarNull_Cb(const char* qnPath,
                                                   SOPC_Variant* var,
                                                   const SOPC_NodeId* dataType,
                                                   int32_t valueRank,
                                                   uintptr_t unusedCtx)
{
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(valueRank);
    SOPC_UNUSED_ARG(unusedCtx);
    SOPC_UNUSED_ARG(dataType);
    SOPC_Variant_Clear(var);
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

SOPC_ReturnStatus SOPC_AlarmCondition_CreateFromEvent(const SOPC_NodeId* notifierNode,
                                                      const SOPC_NodeId* conditionNode,
                                                      SOPC_Event* conditionInst,
                                                      SOPC_AlarmCondition** outAlarmCond)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit() || !SOPC_ServerInternal_IsStarted())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == notifierNode || NULL == conditionNode || NULL == conditionInst || NULL == outAlarmCond)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_AlarmCondition* ac = SOPC_Calloc(1, sizeof(*ac));
    if (NULL == ac)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    // Initialize the AlarmCondition mutex
    SOPC_ReturnStatus status = SOPC_Mutex_Initialization(&ac->mut);
    // Initialize EventId lists for Acknowledge, Confirm and AddComment needed for method calls
    if (SOPC_STATUS_OK == status)
    {
        ac->ackEventIds = SOPC_EventIdList_Create(S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED);
        ac->confEventIds = SOPC_EventIdList_Create(S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED);
        ac->globalEventIds = SOPC_EventIdList_Create(S2OPC_SERVER_ALARM_MAX_EVENT_IDS_RECORDED);
        status = (NULL == ac->ackEventIds || NULL == ac->confEventIds || NULL == ac->globalEventIds)
                     ? SOPC_STATUS_OUT_OF_MEMORY
                     : SOPC_STATUS_OK;
    }
    // Set ConditionNode Id both in event instance and AC instance
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&ac->conditionNode, conditionNode);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetNodeId(conditionInst, conditionNode);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&ac->notifierNode, notifierNode);
    }
    // Set AlarmCondition fields/variables data
    if (SOPC_STATUS_OK == status)
    {
        ac->nodeIds = SOPC_Event_CreateCopy(conditionInst, true);
        if (NULL != ac->nodeIds)
        {
            SOPC_Event_ForEachVar(ac->nodeIds, SOPC_InternalCreate_SetEventVarNull_Cb, (uintptr_t) NULL);
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    // Create structure to store well-known methods Ids
    if (SOPC_STATUS_OK == status)
    {
        ac->methodIds = SOPC_Calloc(SOPC_AC_METHOD_COUNT, sizeof(*ac->methodIds));
        status = (NULL == ac->methodIds) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    // Create structure to store variable changed applicative callbacks
    if (SOPC_STATUS_OK == status)
    {
        ac->varChangedCbDict = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, uintptr_t_free);
        status = (NULL == ac->varChangedCbDict) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    // Inserts the AlarmCondition instance into the global dictionary
    if (SOPC_STATUS_OK == status)
    {
        bool res = SOPC_Dict_Insert(sopc_alarmConditionConfig.g_alarmConditionsDict, (uintptr_t) conditionNode,
                                    (uintptr_t) ac);
        status = res ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }
    // Finalize creation, set default value and set well-known set states with associated text
    if (SOPC_STATUS_OK == status)
    {
        ac->data = conditionInst;
        status = SOPC_InternalAlarmConditionNoLock_InitFields(ac);
    }
    // Try to retrieve the node ids of all the AlarmCondition fields/variables in address space
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Internal_GetAlarmCondition_Vars_NodeIds_And_Write_Values(ac);
        *outAlarmCond = ac;
    }
    else
    {
        SOPC_ServerAlarmCondition_Delete(&ac);
    }
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetEnabledState(SOPC_AlarmCondition* pAlarmCondition,
                                                      bool enabled,
                                                      bool setRetain,
                                                      const SOPC_LocalizedText* optComment)
{
    return SOPC_InternalAlarmCondition_SetEnabledState(pAlarmCondition, enabled, setRetain, optComment, false);
}

bool SOPC_AlarmCondition_GetEnabledState(SOPC_AlarmCondition* pAlarmCondition)
{
    return SOPC_InternalAlarmCondition_GetBoolValueFromStrPath(pAlarmCondition, cEnabledStatePaths.id);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetAutoAcknowledgeable(SOPC_AlarmCondition* pAlarmCondition)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_InternalAlarmCondition_SetAutoAckableOnActive(pAlarmCondition);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetAcknowledgeable(SOPC_AlarmCondition* pAlarmCondition,
                                                         bool acknowledgeable,
                                                         const SOPC_LocalizedText* optComment)
{
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (acknowledgeable && SOPC_InternalAlarmCondition_GetAutoAckableOnActive(pAlarmCondition))
    {
        // Cannot set alarm acknowledgeable manually when AutoAckableOnActive is set
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    bool isEnabled = SOPC_AlarmCondition_GetEnabledState(pAlarmCondition);
    if (acknowledgeable && !isEnabled)
    {
        // Cannot set alarm acknowledgeable when alarm is not enabled
        return SOPC_STATUS_INVALID_STATE;
    }

    // Note: event is triggered only if the condition is enabled
    return SOPC_InternalAlarmCondition_SetAcknowledgeable(pAlarmCondition, acknowledgeable, optComment);
}

SOPC_ReturnStatus SOPC_AlarmCondition_Acknowledge(SOPC_AlarmCondition* pAlarmCondition,
                                                  const SOPC_LocalizedText* optComment)
{
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool isEnabled = SOPC_AlarmCondition_GetEnabledState(pAlarmCondition);
    if (!isEnabled)
    {
        // Cannot change the Acked state when alarm is not enabled
        return SOPC_STATUS_INVALID_STATE;
    }
    // Note: event is triggered only if the condition is enabled
    return SOPC_InternalAlarmCondition_Acknowledge(pAlarmCondition, optComment, NULL, false);
}

bool SOPC_AlarmCondition_GetAckedState(SOPC_AlarmCondition* pAlarmCondition)
{
    return SOPC_InternalAlarmCondition_GetBoolValueFromStrPath(pAlarmCondition, cAckedStatePaths.id);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetAutoConfirmable(SOPC_AlarmCondition* pAlarmCondition)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_InternalAlarmCondition_SetAutoConfOnAcked(pAlarmCondition);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetConfirmable(SOPC_AlarmCondition* pAlarmCondition,
                                                     bool confirmable,
                                                     const SOPC_LocalizedText* optComment)
{
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool isEnabled = SOPC_AlarmCondition_GetEnabledState(pAlarmCondition);
    if (isEnabled && SOPC_InternalAlarmCondition_GetAutoConfOnAcked(pAlarmCondition))
    {
        // Cannot change alarm confirmable manually when AutoConfOnAcked is set
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (confirmable && !isEnabled)
    {
        // Cannot set alarm confirmable when alarm is not enabled
        return SOPC_STATUS_INVALID_STATE;
    }

    // Note: event is triggered only if the condition is enabled
    return SOPC_InternalAlarmCondition_SetConfirmable(pAlarmCondition, confirmable, optComment);
}

SOPC_ReturnStatus SOPC_AlarmCondition_Confirm(SOPC_AlarmCondition* pAlarmCondition,
                                              const SOPC_LocalizedText* optComment)
{
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    bool isEnabled = SOPC_AlarmCondition_GetEnabledState(pAlarmCondition);
    if (!isEnabled)
    {
        // Cannot change the Confirmed state when alarm is not enabled
        return SOPC_STATUS_INVALID_STATE;
    }
    // Note: event is triggered only if the condition is enabled
    return SOPC_InternalAlarmCondition_Confirm(pAlarmCondition, optComment, NULL, false);
}

bool SOPC_AlarmCondition_GetConfirmedState(SOPC_AlarmCondition* pAlarmCondition)
{
    return SOPC_InternalAlarmCondition_GetBoolValueFromStrPath(pAlarmCondition, cConfirmedStatePaths.id);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetActiveState(SOPC_AlarmCondition* pAlarmCondition,
                                                     bool active,
                                                     const SOPC_LocalizedText* optComment)
{
    bool isEnabled = SOPC_AlarmCondition_GetEnabledState(pAlarmCondition);
    if (active && !isEnabled)
    {
        // Cannot change the Active state when alarm is not enabled
        return SOPC_STATUS_INVALID_STATE;
    }
    // Note: event is triggered only if the condition is enabled
    return SOPC_InternalAlarmCondition_SetActiveState(pAlarmCondition, active, optComment, NULL, isEnabled);
}

bool SOPC_AlarmCondition_GetActiveState(SOPC_AlarmCondition* pAlarmCondition)
{
    return SOPC_InternalAlarmCondition_GetBoolValueFromStrPath(pAlarmCondition, cActiveStatePaths.id);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetAutoRetain(SOPC_AlarmCondition* pAlarmCondition)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_InternalAlarmCondition_SetAutoRetain(pAlarmCondition);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetRetain(SOPC_AlarmCondition* pAlarmCondition, bool retain, bool triggerEvent)
{
    if (!SOPC_AlarmCondition_GetEnabledState(pAlarmCondition))
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    const SOPC_Variant retainVar = {.DoNotClear = true,
                                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                                    .BuiltInTypeId = SOPC_Boolean_Id,
                                    .Value.Boolean = retain};

    bool stateTrueOrChanged = true;
    // Retrieve and compare previous value when transition to False
    if (!retain)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        const SOPC_Variant* var = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, cRetainPath);
        SOPC_ReturnStatus tmpStatus = (NULL == var ? SOPC_STATUS_INVALID_STATE : SOPC_STATUS_OK);
        if (SOPC_STATUS_OK == tmpStatus)
        {
            stateTrueOrChanged = var->Value.Boolean == true;
        }
        mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    // Set new value
    SOPC_ReturnStatus status = SOPC_AlarmCondition_SetVariableFromStrPath(pAlarmCondition, cRetainPath, &retainVar);
    // Trigger event if requested and status changed.
    // From part 9 §5.5.2:
    // Events are only generated for Conditions that have their Retain field set to True and for the
    // initial transition of the Retain field from True to False.
    if (SOPC_STATUS_OK == status && triggerEvent && stateTrueOrChanged)
    {
        SOPC_AlarmCondition_TriggerEvent(pAlarmCondition);
    }
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetQuality(SOPC_AlarmCondition* pAlarmCondition, SOPC_StatusCode quality)
{
    const SOPC_Variant qualityVar = {.DoNotClear = true,
                                     .ArrayType = SOPC_VariantArrayType_SingleValue,
                                     .BuiltInTypeId = SOPC_StatusCode_Id,
                                     .Value.Status = quality};
    return SOPC_AlarmCondition_SetConditionVariableFromStrPath(pAlarmCondition, cQualityIdPath, &qualityVar, true);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetSeverity(SOPC_AlarmCondition* pAlarmCondition, uint16_t severity)
{
    const SOPC_Variant newSeverityVar = {.DoNotClear = true,
                                         .ArrayType = SOPC_VariantArrayType_SingleValue,
                                         .BuiltInTypeId = SOPC_UInt16_Id,
                                         .Value.Uint16 = severity};
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Retrieve current Severity and stores it as LastSeverity
    const SOPC_Variant* prevSeverityVar = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, cSeverityIdPath);
    SOPC_ReturnStatus status = NULL != prevSeverityVar ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_InternalAlarmConditionNoLock_SetConditionVariableFromStrPath(pAlarmCondition, cLastSeverityIdPath,
                                                                                   prevSeverityVar, true);
    }
    // Update Severity
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(pAlarmCondition, cSeverityIdPath,
                                                                          &newSeverityVar, true);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus eventStatus = SOPC_InternalAlarmConditionNoLock_TriggerEvent(pAlarmCondition, false);
        if (eventStatus != SOPC_STATUS_INVALID_STATE &&
            (eventStatus != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG))
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            if (eventStatus != SOPC_STATUS_OK)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggering for LastSeverity condition variable change to %" PRIu16
                    "(new Severity=%" PRIu16 ") failed with status %d",
                    conditionIdStr, prevSeverityVar->Value.Uint16, severity, eventStatus);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition %s event triggered for LastSeverity variable change to %" PRIu16
                                       "(new Severity=%" PRIu16 ") with eventId=%s",
                                       conditionIdStr, prevSeverityVar->Value.Uint16, severity, eventIdStr);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(conditionIdStr);
        }
    }
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetComment(SOPC_AlarmCondition* pAlarmCondition,
                                                 const SOPC_LocalizedText* comment)
{
    return SOPC_InternalAlarmCondition_SetComment(pAlarmCondition, comment, NULL);
}

static SOPC_ReturnStatus SOPC_AlarmCondition_CommonSetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                                          const char* varPath,
                                                                          const SOPC_Variant* var,
                                                                          bool conditionVar,
                                                                          bool triggerEvent)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_NOK;
    }
    if (NULL == pAlarmCondition || NULL == varPath || NULL == var)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (conditionVar)
    {
        status = SOPC_InternalAlarmConditionNoLock_SetConditionVariableFromStrPath(pAlarmCondition, varPath, var, true);
    }
    else
    {
        status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(pAlarmCondition, varPath, var, true);
    }
    // Create event copy to be triggered and set the new EventId variable
    if (triggerEvent && SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus eventStatus = SOPC_InternalAlarmConditionNoLock_TriggerEvent(pAlarmCondition, false);
        if (eventStatus != SOPC_STATUS_INVALID_STATE &&
            (eventStatus != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG))
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            char* varStr = SOPC_InternalVarToString(var);
            if (eventStatus != SOPC_STATUS_OK)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition %s event triggering for %s condition variable change to %s "
                                       "failed with status %d",
                                       conditionIdStr, varPath, (NULL == varStr ? "<NULL>" : varStr), eventStatus);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                SOPC_Logger_TraceDebug(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggered for condition variable %s change to %s with eventId=%s",
                    conditionIdStr, varPath, (NULL == varStr ? "<NULL>" : varStr), eventIdStr);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(conditionIdStr);
            SOPC_Free(varStr);
        }
    }
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetVariable(SOPC_AlarmCondition* pAlarmCondition,
                                                  uint16_t nbQnPath,
                                                  const SOPC_QualifiedName* qualifiedNamePathArray,
                                                  const SOPC_Variant* var)
{
    char* qnPath = NULL;
    SOPC_ReturnStatus status = SOPC_EventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, &qnPath);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_AlarmCondition_SetVariableFromStrPath(pAlarmCondition, qnPath, var);
    }
    SOPC_Free(qnPath);
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                             const char* qnPath,
                                                             const SOPC_Variant* var)
{
    return SOPC_AlarmCondition_CommonSetVariableFromStrPath(pAlarmCondition, qnPath, var, false, false);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetConditionVariable(SOPC_AlarmCondition* pAlarmCondition,
                                                           uint16_t nbQnPath,
                                                           const SOPC_QualifiedName* qualifiedNamePathArray,
                                                           const SOPC_Variant* var,
                                                           bool triggerEvent)
{
    char* qnPath = NULL;
    SOPC_ReturnStatus status = SOPC_EventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, &qnPath);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_AlarmCondition_SetConditionVariableFromStrPath(pAlarmCondition, qnPath, var, triggerEvent);
    }
    SOPC_Free(qnPath);
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetConditionVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                                      const char* qnPath,
                                                                      const SOPC_Variant* var,
                                                                      bool triggerEvent)
{
    return SOPC_AlarmCondition_CommonSetVariableFromStrPath(pAlarmCondition, qnPath, var, true, triggerEvent);
}

SOPC_Variant* SOPC_AlarmCondition_GetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition, const char* qnPath)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return NULL;
    }
    if (NULL == pAlarmCondition || NULL == qnPath)
    {
        return NULL;
    }
    SOPC_Variant* varCopy = SOPC_Variant_Create();
    if (NULL == varCopy)
    {
        return NULL;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const SOPC_Variant* var = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, qnPath);
    SOPC_ReturnStatus status = (NULL == var ? SOPC_STATUS_INVALID_STATE : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Variant_Copy(varCopy, var);
    }
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Variant_Delete(varCopy);
        varCopy = NULL;
    }
    return varCopy;
}

SOPC_Variant* SOPC_AlarmCondition_GetVariable(SOPC_AlarmCondition* pAlarmCondition,
                                              uint16_t nbQnPath,
                                              const SOPC_QualifiedName* qualifiedNamePathArray)
{
    char* qnPath = NULL;
    SOPC_Variant* varCopy = NULL;
    SOPC_ReturnStatus status = SOPC_EventManagerUtil_QnPathToCString(nbQnPath, qualifiedNamePathArray, &qnPath);
    if (SOPC_STATUS_OK == status)
    {
        varCopy = SOPC_AlarmCondition_GetVariableFromStrPath(pAlarmCondition, qnPath);
    }
    SOPC_Free(qnPath);
    return varCopy;
}

SOPC_ReturnStatus SOPC_AlarmCondition_TriggerEvent(SOPC_AlarmCondition* pAlarmCondition)
{
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    return SOPC_InternalAlarmConditionWithLock_TriggerEvent(pAlarmCondition, true);
}

static SOPC_ReturnStatus SOPC_AlarmCondition_SetStateChangeCallbackFromStrPath(
    SOPC_AlarmCondition* pAlarmCondition,
    const char* qnPath,
    SOPC_AlarmCondition_StateChanged_Fct* callback,
    uintptr_t userCbCtx)
{
    if (NULL == pAlarmCondition || NULL == callback)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    bool found = false;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    uintptr_t prevVal = SOPC_Dict_Get(pAlarmCondition->varChangedCbDict, (const uintptr_t) qnPath, &found);
    SOPC_UNUSED_RESULT(prevVal);
    if (!found)
    {
        char* strPath = SOPC_strdup(qnPath);
        SOPC_AlarmCondition_StateChanged_Fct_Ctx* fctCtx = SOPC_Calloc(1, sizeof(*fctCtx));
        status = (NULL == fctCtx || NULL == strPath ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
        if (SOPC_STATUS_OK == status)
        {
            fctCtx->ac = pAlarmCondition;
            fctCtx->qnPath = strPath;
            fctCtx->callback = callback;
            fctCtx->userCtx = userCbCtx;
            bool res = SOPC_Dict_Insert(pAlarmCondition->varChangedCbDict, (uintptr_t) strPath, (uintptr_t) fctCtx);
            status = res ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(fctCtx);
            SOPC_Free(strPath);
        }
    }
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetEnabledStateCallback(SOPC_AlarmCondition* pAlarmCondition,
                                                              SOPC_AlarmCondition_StateChanged_Fct* callback,
                                                              uintptr_t userCbCtx)
{
    return SOPC_AlarmCondition_SetStateChangeCallbackFromStrPath(pAlarmCondition, cEnabledStatePaths.self, callback,
                                                                 userCbCtx);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetAckedStateCallback(SOPC_AlarmCondition* pAlarmCondition,
                                                            SOPC_AlarmCondition_StateChanged_Fct* callback,
                                                            uintptr_t userCbCtx)
{
    return SOPC_AlarmCondition_SetStateChangeCallbackFromStrPath(pAlarmCondition, cAckedStatePaths.self, callback,
                                                                 userCbCtx);
}

SOPC_ReturnStatus SOPC_AlarmCondition_SetConfirmedStateCallback(SOPC_AlarmCondition* pAlarmCondition,
                                                                SOPC_AlarmCondition_StateChanged_Fct* callback,
                                                                uintptr_t userCbCtx)
{
    return SOPC_AlarmCondition_SetStateChangeCallbackFromStrPath(pAlarmCondition, cConfirmedStatePaths.self, callback,
                                                                 userCbCtx);
}
