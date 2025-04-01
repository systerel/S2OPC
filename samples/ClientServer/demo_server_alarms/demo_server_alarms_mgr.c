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

#include "demo_server_alarms_mgr.h"

#include <string.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_enums.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_types.h"

#include "sopc_logger.h"

#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_alarm_conditions.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

/*----------------------------------------------------------------------------
 *                            Equipments With Alarms management
 *----------------------------------------------------------------------------*/

#define DEFAULT_UPDATE_COND_PERIOD_MS 5000

typedef struct SOPC_EquipmentWithAlarm
{
    SOPC_Mutex mutex;                   // Mutex to modify equipment states
    SOPC_NodeId eqtId;                  // Identifier of the equipment object
    SOPC_QualifiedName eqtName;         // QualifiedName of the equipment object
    SOPC_NodeId conditionSourceId;      // Identifier of the condition source variable
    bool lastConditionSourceValue;      // The condition source that triggers the alarm activation
    SOPC_NodeId alarmId;                // Identifier of the alarm object
    char* alarmIdStr;                   // String representation of the alarm object identifier
    SOPC_AlarmCondition* alarmInstance; // The alarm condition instance
    bool alarmHasConfirmation;          // True if the alarm has a confirmation step
} SOPC_EquipmentWithAlarm;

static void SOPC_EquipmentWithAlarm_Clear(void* eqtAux)
{
    SOPC_EquipmentWithAlarm* eqt = (SOPC_EquipmentWithAlarm*) eqtAux;
    SOPC_Mutex_Lock(&eqt->mutex);
    SOPC_NodeId_Clear(&eqt->eqtId);
    SOPC_QualifiedName_Clear(&eqt->eqtName);
    SOPC_NodeId_Clear(&eqt->conditionSourceId);
    SOPC_NodeId_Clear(&eqt->alarmId);
    SOPC_Free(eqt->alarmIdStr);
    eqt->alarmIdStr = NULL;
    eqt->alarmInstance = NULL;
    SOPC_Mutex_Unlock(&eqt->mutex);
    SOPC_Mutex_Clear(&eqt->mutex);
}

#define MY_EQUIPMENT_TYPE_NS 1
static const SOPC_NodeId MyEquipmentWithConfTypeId = SOPC_NODEID_NUMERIC(MY_EQUIPMENT_TYPE_NS, 1004);
static const SOPC_NodeId MyEquipmentNoConfTypeId = SOPC_NODEID_NUMERIC(MY_EQUIPMENT_TYPE_NS, 1006);

static const SOPC_NodeId AlarmConditionTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_AlarmConditionType);
static const SOPC_NodeId HasTypeDefinitionId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasTypeDefinition);

static SOPC_QualifiedName qnConditionSource = SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "ConditionSource");
static SOPC_QualifiedName qnAlarmNoConf = SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "AlarmNoConf");
static SOPC_QualifiedName qnAlarmWithConf = SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "AlarmWithConf");

static const SOPC_Variant defaultMessage = SOPC_VARIANT_STRING("Ai + FALSE");
static const SOPC_Variant messageActiveTrue = SOPC_VARIANT_STRING("Aa + TRUE");
static const SOPC_Variant messageActiveFalse = SOPC_VARIANT_STRING("Aa + FALSE");
static const SOPC_Variant messageAckedTrue = SOPC_VARIANT_STRING("Ak + TRUE");
static const SOPC_Variant messageAckedFalse = SOPC_VARIANT_STRING("Ak + FALSE");

// Initialize context of the managed ::SOPC_EquipmentWithAlarm
static SOPC_ReturnStatus SOPC_EquipmentsWithAlarm_CreateCtx(size_t* nbEqt, SOPC_EquipmentWithAlarm** eqtArr)
{
    SOPC_ASSERT(NULL != nbEqt);
    SOPC_ASSERT(NULL != eqtArr);
    size_t nbResEqt = 0;
    SOPC_EquipmentWithAlarm* resEqtArr = NULL;

    // Browse MyEquipmentNoConfTypeId / MyEquipmentWithConfTypeId nodes to get all
    // their Object node instances in address space (reverse HasTypeDefinition reference to)
    OpcUa_BrowseRequest* browseReq = SOPC_BrowseRequest_Create(2, 0, NULL);
    OpcUa_BrowseResponse* browseResp = NULL;
    SOPC_ReturnStatus status = (NULL != browseReq) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_BrowseRequest_SetBrowseDescription(browseReq, 0, &MyEquipmentNoConfTypeId,
                                                         OpcUa_BrowseDirection_Inverse, &HasTypeDefinitionId, false,
                                                         OpcUa_NodeClass_Object, OpcUa_BrowseResultMask_BrowseName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_BrowseRequest_SetBrowseDescription(browseReq, 1, &MyEquipmentWithConfTypeId,
                                                         OpcUa_BrowseDirection_Inverse, &HasTypeDefinitionId, false,
                                                         OpcUa_NodeClass_Object, OpcUa_BrowseResultMask_BrowseName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(browseReq, (void**) &browseResp);
    }

    // Create array of SOPC_EquipmentWithAlarm for the equipment instances
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult) &&
            (uint64_t)(browseResp->Results[0].NoOfReferences + browseResp->Results[1].NoOfReferences) <= SIZE_MAX)
        {
            // 1 result for each equipment type MyEquipmentNoConfTypeId / MyEquipmentWithConfTypeId
            SOPC_ASSERT(2 == browseResp->NoOfResults);
            resEqtArr = SOPC_Calloc(
                (size_t) browseResp->Results[0].NoOfReferences + (size_t) browseResp->Results[1].NoOfReferences,
                sizeof(SOPC_EquipmentWithAlarm));
            status = (NULL != resEqtArr) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

            for (int32_t eqtTypIdx = 0; SOPC_STATUS_OK == status && eqtTypIdx < 2; eqtTypIdx++)
            {
                for (int32_t eqtInstanceIdx = 0;
                     SOPC_STATUS_OK == status && eqtInstanceIdx < browseResp->Results[eqtTypIdx].NoOfReferences;
                     eqtInstanceIdx++)
                {
                    SOPC_EquipmentWithAlarm* eqt = &resEqtArr[nbResEqt];
                    // Set no conf / conf property depending on alarm type
                    eqt->alarmHasConfirmation =
                        (1 == eqtTypIdx); // MyEquipmentNoConfTypeId idx == 0 / MyEquipmentWithConfTypeId idx == 1
                    SOPC_ExpandedNodeId* expNid = &browseResp->Results[eqtTypIdx].References[eqtInstanceIdx].NodeId;
                    // Set equipment NodeId
                    if (expNid->NamespaceUri.Length <= 0 && 0 == expNid->ServerIndex)
                    {
                        SOPC_NodeId_Initialize(&eqt->eqtId);
                        status = SOPC_NodeId_Copy(
                            &eqt->eqtId, &browseResp->Results[eqtTypIdx].References[eqtInstanceIdx].NodeId.NodeId);
                        // Increment
                        nbResEqt = (SOPC_STATUS_OK == status ? nbResEqt + 1 : nbResEqt);
                    }
                    else
                    {
                        status = SOPC_STATUS_ENCODING_ERROR;
                    }
                    // Initialize mutex
                    if (SOPC_STATUS_OK == status)
                    {
                        SOPC_Mutex_Initialization(&eqt->mutex);
                    }
                    // Set equipment qualified name
                    if (SOPC_STATUS_OK == status)
                    {
                        status = SOPC_QualifiedName_Copy(
                            &eqt->eqtName, &browseResp->Results[eqtTypIdx].References[eqtInstanceIdx].BrowseName);
                    }
                }
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        (void) SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp);
    }

    // Use TranslateBrowsePath to get the NodeId of the condition and alarm variables
    if (SOPC_STATUS_OK == status)
    {
        // For each equipment instance retrieve node ids of condition node and alarm node
        OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpReq = SOPC_TranslateBrowsePathsRequest_Create(2 * nbResEqt);
        OpcUa_TranslateBrowsePathsToNodeIdsResponse* tbpResp = NULL;

        // Define paths (array of 1 element <=> pointer on unique element)
        OpcUa_RelativePathElement conditionPath;
        OpcUa_RelativePathElement_Initialize(&conditionPath);
        conditionPath.ReferenceTypeId = (SOPC_NodeId) SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
        conditionPath.IsInverse = false;
        conditionPath.TargetName = qnConditionSource;

        OpcUa_RelativePathElement alarmNoConfPath;
        OpcUa_RelativePathElement_Initialize(&alarmNoConfPath);
        alarmNoConfPath.ReferenceTypeId = (SOPC_NodeId) SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
        alarmNoConfPath.IsInverse = false;
        alarmNoConfPath.TargetName = qnAlarmNoConf;

        OpcUa_RelativePathElement alarmWithConfPath;
        OpcUa_RelativePathElement_Initialize(&alarmWithConfPath);
        alarmWithConfPath.ReferenceTypeId = (SOPC_NodeId) SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
        alarmWithConfPath.IsInverse = false;
        alarmWithConfPath.TargetName = qnAlarmWithConf;

        status = (NULL != tbpReq) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        for (size_t i = 0; i < nbResEqt; i++)
        {
            // ConditionSource path is the same for both alarm types
            OpcUa_RelativePathElement* relPathEltsCopy = NULL;
            status = SOPC_EncodeableObject_Create(&OpcUa_RelativePathElement_EncodeableType, (void*) &relPathEltsCopy);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EncodeableObject_Copy(&OpcUa_RelativePathElement_EncodeableType, relPathEltsCopy,
                                                    &conditionPath);
                if (SOPC_STATUS_OK == status)
                {
                    status =
                        SOPC_TranslateBrowsePathRequest_SetPath(tbpReq, i * 2, &resEqtArr[i].eqtId, 1, relPathEltsCopy);
                    SOPC_ASSERT(SOPC_STATUS_OK == status);
                }
            }
            // Alarm path is different depending on alarm type
            relPathEltsCopy = NULL;
            status = SOPC_EncodeableObject_Create(&OpcUa_RelativePathElement_EncodeableType, (void*) &relPathEltsCopy);
            if (SOPC_STATUS_OK == status)
            {
                OpcUa_RelativePathElement* alarmPath =
                    (resEqtArr[i].alarmHasConfirmation ? &alarmWithConfPath : &alarmNoConfPath);
                status =
                    SOPC_EncodeableObject_Copy(&OpcUa_RelativePathElement_EncodeableType, relPathEltsCopy, alarmPath);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_TranslateBrowsePathRequest_SetPath(tbpReq, i * 2 + 1, &resEqtArr[i].eqtId, 1,
                                                                     relPathEltsCopy);
                    SOPC_ASSERT(SOPC_STATUS_OK == status);
                }
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerHelper_LocalServiceSync(tbpReq, (void**) &tbpResp);
            status =
                (SOPC_STATUS_OK == status && SOPC_IsGoodStatus(tbpResp->ResponseHeader.ServiceResult) ? SOPC_STATUS_OK
                                                                                                      : status);
        }
        // Fill the condition and alarm NodeId in the equipment structure using TBP response
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ASSERT(nbResEqt * 2 == (size_t) tbpResp->NoOfResults);
            for (size_t i = 0; i < nbResEqt; i++)
            {
                SOPC_ASSERT(1 == tbpResp->Results[i * 2].NoOfTargets);
                SOPC_NodeId* condNid = &tbpResp->Results[i * 2].Targets->TargetId.NodeId;
                SOPC_ASSERT(1 == tbpResp->Results[i * 2 + 1].NoOfTargets);
                SOPC_NodeId* alarmNid = &tbpResp->Results[i * 2 + 1].Targets->TargetId.NodeId;
                // Copy ConditionSource NodeId
                status = SOPC_NodeId_Copy(&resEqtArr[i].conditionSourceId, condNid);
                if (SOPC_STATUS_OK != status)
                {
                    char* cstrNid = SOPC_NodeId_ToCString(condNid);
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Demo_Server_A&C: Failed to copy node id %s for managing condition variable",
                                           cstrNid);
                    SOPC_Free(cstrNid);
                }
                // Copy Alarm NodeId
                status = SOPC_NodeId_Copy(&resEqtArr[i].alarmId, alarmNid);
                resEqtArr[i].alarmIdStr = SOPC_NodeId_ToCString(alarmNid);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Demo_Server_A&C: Failed to copy node id %s for managing alarm variable",
                                           resEqtArr[i].alarmIdStr);
                }
            }
        }
        if (NULL != tbpResp)
        {
            (void) SOPC_EncodeableObject_Delete(tbpResp->encodeableType, (void**) &tbpResp);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *nbEqt = nbResEqt;
        *eqtArr = resEqtArr;
    }
    else
    {
        int32_t arrSize = (int32_t) nbResEqt;
        SOPC_Clear_Array(&arrSize, (void**) &resEqtArr, sizeof(SOPC_EquipmentWithAlarm), SOPC_EquipmentWithAlarm_Clear);
    }
    return status;
}

// Initialize the equipment alarm initial state in output alarm
static SOPC_ReturnStatus SOPC_InternalEqtInitEvents_Create(const SOPC_Event* alarmEventBase,
                                                           SOPC_EquipmentWithAlarm* eqt,
                                                           SOPC_Event** alarmEvent)
{
    SOPC_Event* alarmRes = SOPC_Event_CreateCopy(alarmEventBase, true);
    SOPC_ReturnStatus status = (NULL != alarmRes) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    // Set SourceName: "<eqtName>.ConditionSource"
    if (SOPC_STATUS_OK == status)
    {
        char* sourceName = NULL;
        status = SOPC_StrConcat(SOPC_String_GetRawCString(&eqt->eqtName.Name), ".ConditionSource", &sourceName);
        SOPC_ASSERT(SOPC_STATUS_OK == status);

        SOPC_String stringSourceName;
        SOPC_String_Initialize(&stringSourceName);
        status = SOPC_String_AttachFromCstring(&stringSourceName, sourceName);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetSourceName(alarmRes, &stringSourceName);
        }
        SOPC_Free(sourceName);
    }

    // Set the condition source Node as source and input for Alarm instance
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetSourceNode(alarmRes, &eqt->conditionSourceId);
    }
    SOPC_Variant sourceNodeVar = SOPC_VARIANT_NODEID(SOPC_NODEID_NS0_NUMERIC(0));
    sourceNodeVar.Value.NodeId = &eqt->conditionSourceId;
    if (SOPC_STATUS_OK == status)

    {
        status = SOPC_Event_SetVariableFromStrPath(alarmRes, "0:InputNode", &sourceNodeVar);
    }
    // Set the Active alarm state to False, Enabled state is True by default
    SOPC_Variant activeVar = SOPC_VARIANT_BOOL(false);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmRes, "0:ActiveState" QN_PATH_SEPARATOR_STR "0:Id", &activeVar);
    }
    // Set both AckedState (and ConfirmState) to True by default (not ackable/confirmable)
    SOPC_Variant trueVar = SOPC_VARIANT_BOOL(true);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmRes, "0:AckedState" QN_PATH_SEPARATOR_STR "0:Id", &trueVar);
    }
    if (eqt->alarmHasConfirmation && SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmRes, "0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:Id", &trueVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        *alarmEvent = alarmRes;
    }
    else
    {
        SOPC_Event_Delete(&alarmRes);
    }
    return status;
}

// Callback executed after an EnableState change through Enable()/Disable() method calls
static void SOPC_EnabledStateChanged(uintptr_t userCbCtx,
                                     SOPC_AlarmCondition* alarmCond,
                                     const char* qnPath,
                                     const SOPC_Variant* prevValue,
                                     const SOPC_Variant* newValue)
{
    SOPC_EquipmentWithAlarm* eqt = (SOPC_EquipmentWithAlarm*) userCbCtx;
    SOPC_Mutex_Lock(&eqt->mutex);
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == newValue->ArrayType);
    bool enabled = false;
    if (SOPC_Boolean_Id == newValue->BuiltInTypeId)
    {
        enabled = newValue->Value.Boolean;
    }

    if (enabled)
    {
        // Check condition source state and activate alarm if needed
        if (eqt->lastConditionSourceValue)
        {
            // Set retain property to true
            SOPC_ReturnStatus retainStatus = SOPC_AlarmCondition_SetRetain(alarmCond, true, false);
            // Alarm becomes Active
            SOPC_ReturnStatus status = SOPC_AlarmCondition_SetActiveState(alarmCond, true, NULL);
            if (SOPC_STATUS_OK != status || SOPC_STATUS_OK != retainStatus)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Demo_Server_A&C: Failed to update active state on EnabledStateChanged for "
                                       "alarm %s (retainCode=%d, activeCode=%d)",
                                       eqt->alarmIdStr, retainStatus, status);
            }
        }
    }
    else
    {
        // Reset alarm states to default values whatever the condition source state is
        if (eqt->alarmHasConfirmation)
        {
            (void) SOPC_AlarmCondition_SetConfirmable(alarmCond, false, NULL);
        }
        (void) SOPC_AlarmCondition_SetAcknowledgeable(alarmCond, false, NULL);
        (void) SOPC_AlarmCondition_SetActiveState(alarmCond, false, NULL);
    }
    SOPC_Mutex_Unlock(&eqt->mutex);
}

// Internal utility function to update alarm Message field to display current state
// (see specific alarm states machine in toolkit_demo_server_alarms.c header documentation)
static void SOPC_InternalUpdateAlarmMessage(SOPC_EquipmentWithAlarm* eqt,
                                            const SOPC_Variant* message,
                                            bool triggerEvent)
{
    SOPC_ReturnStatus msgStatus = SOPC_AlarmCondition_SetVariableFromStrPath(eqt->alarmInstance, "0:Message", message);
    SOPC_UNUSED_RESULT(msgStatus);
    if (triggerEvent)
    {
        // Trigger event to change Message as Alarm states did not change
        SOPC_ReturnStatus evtStatus = SOPC_AlarmCondition_TriggerEvent(eqt->alarmInstance);
        SOPC_UNUSED_RESULT(evtStatus);
    }
}

// Internal utility function to set the alarm to inactive state
static void SOPC_InternalSetInactiveAlarmState(SOPC_EquipmentWithAlarm* eqt, const char* srcOfInactiveTransition)
{
    // Set message property to inactive
    SOPC_InternalUpdateAlarmMessage(eqt, &defaultMessage, false);
    // Set retain property to false
    SOPC_ReturnStatus retainStatus = SOPC_AlarmCondition_SetRetain(eqt->alarmInstance, false, false);
    // Alarm becomes InActive
    SOPC_ReturnStatus status = SOPC_AlarmCondition_SetActiveState(eqt->alarmInstance, false, NULL);
    if (SOPC_STATUS_OK != status || SOPC_STATUS_OK != retainStatus)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Demo_Server_A&C: Failed to update active state on %s for "
                               "alarm %s (retainCode=%d, activeCode=%d)",
                               srcOfInactiveTransition, eqt->alarmIdStr, retainStatus, status);
    }
}

// Callback executed after an AckedState change through Acknowledge() method call
static void SOPC_AckedStateChanged(uintptr_t userCbCtx,
                                   SOPC_AlarmCondition* alarmCond,
                                   const char* qnPath,
                                   const SOPC_Variant* prevValue,
                                   const SOPC_Variant* newValue)
{
    SOPC_EquipmentWithAlarm* eqt = (SOPC_EquipmentWithAlarm*) userCbCtx;
    SOPC_Mutex_Lock(&eqt->mutex);
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == newValue->ArrayType);
    bool acked = false;
    if (SOPC_Boolean_Id == newValue->BuiltInTypeId)
    {
        acked = newValue->Value.Boolean;
    }

    // If the alarm is acknowledged and the condition source is false
    if (acked && !eqt->lastConditionSourceValue)
    {
        if (eqt->alarmHasConfirmation)
        {
            // Set message to Acked+FALSE
            SOPC_InternalUpdateAlarmMessage(eqt, &messageAckedFalse, false);
            // Alarm becomes confirmable
            SOPC_ReturnStatus status = SOPC_AlarmCondition_SetConfirmable(alarmCond, true, NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Demo_Server_A&C: Failed to update confirmable state on AckedStateChanged for "
                                       "alarm %s (confirmableCode=%d)",
                                       eqt->alarmIdStr, status);
            }
        }
        else
        {
            // Alarms becomes inactive
            SOPC_InternalSetInactiveAlarmState(eqt, "AckedStateChanged");
        }
    }
    else if (acked) // wait for condition source to become false
    {
        // Set message to Acked+TRUE and trigger notification
        SOPC_InternalUpdateAlarmMessage(eqt, &messageAckedTrue, true);
    }
    SOPC_Mutex_Unlock(&eqt->mutex);
}

// Callback executed after an AckedState change through Confirm() method call
static void SOPC_ConfirmedStateChanged(uintptr_t userCbCtx,
                                       SOPC_AlarmCondition* alarmCond,
                                       const char* qnPath,
                                       const SOPC_Variant* prevValue,
                                       const SOPC_Variant* newValue)
{
    SOPC_EquipmentWithAlarm* eqt = (SOPC_EquipmentWithAlarm*) userCbCtx;
    SOPC_Mutex_Lock(&eqt->mutex);
    SOPC_UNUSED_ARG(alarmCond);
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == newValue->ArrayType);
    bool confirmed = false;
    if (SOPC_Boolean_Id == newValue->BuiltInTypeId)
    {
        confirmed = newValue->Value.Boolean;
    }
    if (confirmed && !eqt->lastConditionSourceValue)
    {
        // Alarms becomes inactive
        SOPC_InternalSetInactiveAlarmState(eqt, "ConfirmedStateChanged");
    } // else: ignore since condition source became true again
    SOPC_Mutex_Unlock(&eqt->mutex);
}

// Internal utility function to create the base event containing common init data for any equipment
static SOPC_Event* SOPC_InternalCreateBaseEventData(void)
{
    SOPC_Event* alarmEventBase = NULL;
    SOPC_ReturnStatus status = SOPC_ServerHelper_CreateEvent(&AlarmConditionTypeId, &alarmEventBase);
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }

    // Set the ConditionClassId to the alarm event
    SOPC_Variant conditionClassId =
        SOPC_VARIANT_NODEID(SOPC_NODEID_NUMERIC(MY_EQUIPMENT_TYPE_NS, OpcUaId_MaintenanceConditionClassType));
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmEventBase, "0:ConditionClassId", &conditionClassId);
    }

    // Set the ConditionClassName to the alarm event
    SOPC_Variant conditionClassName = SOPC_VARIANT_LOCALTEXT("", "MaintenanceConditionClassType");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmEventBase, "0:ConditionClassName", &conditionClassName);
    }

    // Set initial Quality value
    const SOPC_Variant defaultQuality = SOPC_VARIANT_STATUSCODE(SOPC_GoodGenericStatus);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmEventBase, "0:Quality", &defaultQuality);
    }

    // Set initial Quality Timestamp value
    const SOPC_Variant defaultQualityTs = SOPC_VARIANT_DATE(SOPC_Time_GetCurrentTimeUTC());
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(
            alarmEventBase, "0:Quality" QN_PATH_SEPARATOR_STR "0:SourceTimestamp", &defaultQualityTs);
    }

    // Set initial Comment value
    const SOPC_Variant defaultComment = SOPC_VARIANT_LOCALTEXT("", "");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmEventBase, "0:Comment", &defaultComment);
    }

    // Set initial SuppressedOrShelved value
    const SOPC_Variant defaultSuppressed = SOPC_VARIANT_BOOL(false);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmEventBase, "0:SuppressedOrShelved", &defaultSuppressed);
    }

    // Set initial Message value
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(alarmEventBase, "0:Message", &defaultMessage);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Event_Delete(&alarmEventBase);
        alarmEventBase = NULL;
    }
    return alarmEventBase;
}

// Internal utility to write the condition source values specified in write request into the address space
static SOPC_ReturnStatus SOPC_InternalWriteConditionSourceValues(OpcUa_WriteRequest* conditionSrcWriteReq)
{
    OpcUa_WriteResponse* conditionSrcWriteResp = NULL;
    OpcUa_WriteRequest conditionSrcWriteReqCopy;
    OpcUa_WriteRequest_Initialize(&conditionSrcWriteReqCopy);
    SOPC_ReturnStatus status =
        SOPC_EncodeableObject_Copy(&OpcUa_WriteRequest_EncodeableType, &conditionSrcWriteReqCopy, conditionSrcWriteReq);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(conditionSrcWriteReq, (void**) &conditionSrcWriteResp);
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_IsGoodStatus(conditionSrcWriteResp->ResponseHeader.ServiceResult) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        for (int32_t i = 0; i < conditionSrcWriteResp->NoOfResults && SOPC_STATUS_OK == status; i++)
        {
            if (!SOPC_IsGoodStatus(conditionSrcWriteResp->Results[i]))
            {
                char* nodeIdStr = SOPC_NodeId_ToCString(&conditionSrcWriteReqCopy.NodesToWrite[i].NodeId);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to update value of condition source %s",
                                         nodeIdStr);
                SOPC_Free(nodeIdStr);
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Failed to update value of condition source (service failure)");
        }
        (void) SOPC_EncodeableObject_Clear(conditionSrcWriteReqCopy.encodeableType, &conditionSrcWriteReqCopy);
        (void) SOPC_EncodeableObject_Delete(conditionSrcWriteResp->encodeableType, (void**) &conditionSrcWriteResp);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to update value of condition source");
        (void) SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &conditionSrcWriteReq);
    }
    return status;
}

static SOPC_ReturnStatus SOPC_EquipmentsWithAlarm_Initialize(size_t nbEqt, SOPC_EquipmentWithAlarm* eqtArr)
{
    // Create base event reused to initialized each alarm instance
    SOPC_Event* alarmEventBase = SOPC_InternalCreateBaseEventData();
    if (NULL == alarmEventBase)
    {
        return SOPC_STATUS_NOK;
    }

    // Fill a write request to set initial value of the equipments ConditionSource values
    OpcUa_WriteRequest* conditionSrcWriteReq = SOPC_WriteRequest_Create(nbEqt);
    SOPC_ReturnStatus status = (NULL != conditionSrcWriteReq) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    SOPC_Event* alarmEvent = NULL;
    SOPC_AlarmCondition* alarm = NULL;

    // For each equipment create an alarm instance from dedicated event values and set condition source value
    for (size_t i = 0; i < nbEqt && SOPC_STATUS_OK == status; i++)
    {
        SOPC_EquipmentWithAlarm* eqt = &eqtArr[i];

        // Set initial values expected for the alarms
        status = SOPC_InternalEqtInitEvents_Create(alarmEventBase, eqt, &alarmEvent);

        // Create the alarms instance
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AlarmCondition_CreateFromEvent(&eqt->eqtId, &eqt->alarmId, alarmEvent, &alarm);
        }

        // Configure the alarms behavior on Activation (=> Acknowledgeable)
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AlarmCondition_SetAutoAcknowledgeable(alarm);
        }

        // Configure the alarms callbacks on enable
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AlarmCondition_SetEnabledStateCallback(alarm, &SOPC_EnabledStateChanged, (uintptr_t) eqt);
        }

        // Configure the alarms callbacks on acked / confirmed
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AlarmCondition_SetAckedStateCallback(alarm, &SOPC_AckedStateChanged, (uintptr_t) eqt);
        }
        if (eqt->alarmHasConfirmation && SOPC_STATUS_OK == status)
        {
            status = SOPC_AlarmCondition_SetConfirmedStateCallback(alarm, &SOPC_ConfirmedStateChanged, (uintptr_t) eqt);
        }

        // Set the initial value of the condition source
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Variant conditionSrcVar = SOPC_VARIANT_BOOL(eqt->lastConditionSourceValue);
            SOPC_DataValue conditionSrcDv;
            SOPC_DataValue_Initialize(&conditionSrcDv);
            conditionSrcDv.Value = conditionSrcVar;
            conditionSrcDv.Status = OpcUa_BadResourceUnavailable;
            status = SOPC_WriteRequest_SetWriteValue(conditionSrcWriteReq, i, &eqt->conditionSourceId,
                                                     SOPC_AttributeId_Value, NULL, &conditionSrcDv);
        }

        if (SOPC_STATUS_OK == status)
        {
            eqt->alarmInstance = alarm;
        }
        else
        {
            SOPC_Event_Delete(&alarmEvent);
        }
    }
    // Write the equipment condition source values in address space
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_InternalWriteConditionSourceValues(conditionSrcWriteReq);
    }
    else
    {
        (void) SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &conditionSrcWriteReq);
    }
    SOPC_Event_Delete(&alarmEventBase);

    return status;
}

// Update the condition source value and the alarm state:
// ConditionSource = !(ConditionSource)
// ConditionSource TRUE:
// Set ActiveState to TRUE if not already active and reset AckedState (and ConfirmedState)
// ConditionSource FALSE:
// Set ActiveState to FALSE if already active and AckedState (and ConfirmedState) are TRUE
static void UpdateConditionSourceAndAlarmStates(OpcUa_WriteRequest* wr, size_t wrIdx, SOPC_EquipmentWithAlarm* eqt)
{
    SOPC_Mutex_Lock(&eqt->mutex);
    // Set the new value and fill the write value request item

    // ConditionSource = !(previousConditionSource)
    bool conditionSourceVal = !eqt->lastConditionSourceValue;
    eqt->lastConditionSourceValue = conditionSourceVal;

    // Make a DataValue to fill the write request
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    dv.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
    dv.Value.BuiltInTypeId = SOPC_Boolean_Id;
    dv.Value.Value.Boolean = conditionSourceVal;

    SOPC_ReturnStatus status =
        SOPC_WriteRequest_SetWriteValue(wr, (size_t) wrIdx, &eqt->conditionSourceId, SOPC_AttributeId_Value, NULL, &dv);
    assert(SOPC_STATUS_OK == status);

    if (!SOPC_AlarmCondition_GetEnabledState(eqt->alarmInstance))
    {
        SOPC_Mutex_Unlock(&eqt->mutex);
        return; // No alarm update, only update the ConditionSource value through write request
    }

    bool activeState = SOPC_AlarmCondition_GetActiveState(eqt->alarmInstance);
    bool ackedState = SOPC_AlarmCondition_GetAckedState(eqt->alarmInstance);
    bool confirmedState = SOPC_AlarmCondition_GetConfirmedState(eqt->alarmInstance);

    // Update the alarm state to Active if condition source is true and alarm is not already active
    if (conditionSourceVal)
    {
        if (activeState && ackedState && confirmedState) // note: only to manage concurrent execution possible issue
        {
            // Confirmed callback was not called yet: alarms becomes first inactive
            SOPC_InternalSetInactiveAlarmState(eqt, "ConfirmedChanged(bis)");
            activeState = false;
        }
        if (!activeState) // Alarm was inactive
        {
            // Set message property to active TRUE
            SOPC_InternalUpdateAlarmMessage(eqt, &messageActiveTrue, false);

            status = SOPC_AlarmCondition_SetRetain(eqt->alarmInstance, true, false);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to set Retain TRUE property for alarm: %s",
                                       eqt->alarmIdStr);
            }
            // Set alarm Acitve: automatically reset the Acked/Confirmed states
            status = SOPC_AlarmCondition_SetActiveState(eqt->alarmInstance, true, NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to set active state TRUE for alarm: %s",
                                       eqt->alarmIdStr);
            }
        }
        else // Alarm is already active
        {
            if (eqt->alarmHasConfirmation) // Alarm with a confirmation case
            {
                // Alarm becomes unconfirmable as the condition source is true
                status = SOPC_AlarmCondition_SetConfirmable(eqt->alarmInstance, false, NULL);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
            }
            // Set message property to acked/active + TRUE, trigger event to notify message change
            SOPC_InternalUpdateAlarmMessage(eqt, ackedState ? &messageAckedTrue : &messageActiveTrue, true);
        }
    }

    // Update the alarm state if condition source became false
    if (!conditionSourceVal)
    {
        if (activeState && ackedState) // Alarm was active and acknowledged
        {
            if (eqt->alarmHasConfirmation) // Alarm with a confirmation case
            {
                // Set message property to acked and false
                SOPC_InternalUpdateAlarmMessage(eqt, &messageAckedFalse, false);

                // Alarm becomes confirmable as it is acknowledged and the condition source is false
                status = SOPC_AlarmCondition_SetConfirmable(eqt->alarmInstance, true, NULL);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Failed to set confirmable state for alarm: %s", eqt->alarmIdStr);
                }
            }
            else
            {
                // Alarms becomes inactive
                SOPC_InternalSetInactiveAlarmState(eqt, "ConditionSourceChanged");
            }
        }
        else if (activeState) // Alarm is not Acked
        {
            // Set message property to active and unacked, trigger event to notify change
            SOPC_InternalUpdateAlarmMessage(eqt, &messageActiveFalse, true);
        } // Not active: default state
    }
    SOPC_Mutex_Unlock(&eqt->mutex);
}

// Function periodically called to update the condition source values and update the alarms
static void SOPC_LooperEventHandler_UpdateEqtConditionSource(SOPC_EventHandler* handler,
                                                             int32_t event,
                                                             uint32_t eltId,
                                                             uintptr_t params,
                                                             uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(auxParam);
    uint32_t nbEqt = eltId;
    SOPC_EquipmentWithAlarm* eqtArr = (SOPC_EquipmentWithAlarm*) params;

    OpcUa_WriteRequest* wr = SOPC_WriteRequest_Create(nbEqt);
    if (NULL == wr)
    {
        return;
    }

    // Update the condition source values and associated alarms
    for (uint32_t i = 0; i < nbEqt; i++)
    {
        SOPC_EquipmentWithAlarm* eqt = &eqtArr[i];
        UpdateConditionSourceAndAlarmStates(wr, i, eqt);
    }

    // Actually write the new values into server address space
    SOPC_ReturnStatus status = SOPC_InternalWriteConditionSourceValues(wr);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
}

// Looper thread in charge of updating variables and alarms
static SOPC_Looper* g_updateVarsEventLooper = NULL;
// Update timer identifier (to stop timer)
static uint32_t g_updateVarsTimerId = 0;
// Array of equipments with an alarm managed
static SOPC_EquipmentWithAlarm* g_eqtArr = NULL;
size_t g_nbEqt = 0;

SOPC_ReturnStatus Demo_Server_InitializeAlarms(void)
{
    // Get configuration for equipment objects with alarms
    SOPC_ReturnStatus status = SOPC_EquipmentsWithAlarm_CreateCtx(&g_nbEqt, &g_eqtArr);

    // Iinitialize Alarm and Condition instances
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EquipmentsWithAlarm_Initialize(g_nbEqt, g_eqtArr);
    }

    // Start looper to manage updates of variables and alarms
    // Create looper and create timer events for periodic update of variables
    if (SOPC_STATUS_OK == status)
    {
        SOPC_EventHandler* updateVarsEH = NULL;
        g_updateVarsEventLooper = SOPC_Looper_Create("UpdateEquipmentWithAlarm");
        status = (NULL != g_updateVarsEventLooper) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        if (SOPC_STATUS_OK == status)
        {
            updateVarsEH =
                SOPC_EventHandler_Create(g_updateVarsEventLooper, &SOPC_LooperEventHandler_UpdateEqtConditionSource);
            status = (NULL != updateVarsEH) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_LooperEvent event = {0, (uint32_t) g_nbEqt, (uintptr_t) g_eqtArr, 0};
            // Only 1 update for initial values
            g_updateVarsTimerId = SOPC_EventTimer_CreatePeriodic(updateVarsEH, event, DEFAULT_UPDATE_COND_PERIOD_MS);
            status = (0 != g_updateVarsTimerId) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        Demo_Server_ClearAlarms();
    }
    return status;
}

void Demo_Server_PreStopAlarms(void)
{
    // Stop timer for periodic updates
    SOPC_EventTimer_Cancel(g_updateVarsTimerId);
    g_updateVarsTimerId = 0;
    // Stop event handler + looper
    SOPC_Looper_Delete(g_updateVarsEventLooper);
    g_updateVarsEventLooper = NULL;
}

void Demo_Server_ClearAlarms(void)
{
    int32_t arrSize = (int32_t) g_nbEqt;
    SOPC_Clear_Array(&arrSize, (void**) &g_eqtArr, sizeof(SOPC_EquipmentWithAlarm), SOPC_EquipmentWithAlarm_Clear);
}
