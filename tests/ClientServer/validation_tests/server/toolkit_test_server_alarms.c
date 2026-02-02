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

#include "toolkit_test_server_alarms.h"

#include <string.h>

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
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

#define DEFAULT_UPDATE_SENSORS_PERIOD_MS 500
typedef struct SOPC_EquipmentSensor
{
    bool increasing;                // True if the sensor value is increasing
    double value;                   // Current value of the sensor variable
    SOPC_NodeId measureId;          // Identifier of the sensor variable
    SOPC_NodeId lowAlarmId;         // Identifier of the low limit alarm
    SOPC_NodeId highAlarmId;        // Identifier of the high limit alarm
    SOPC_AlarmCondition* lowAlarm;  // Low limit alarm condition
    SOPC_AlarmCondition* highAlarm; // High limit alarm condition
} SOPC_EquipmentSensor;

typedef struct SOPC_EquipmentWithAlarm
{
    SOPC_NodeId eqtId; // Identifier of the equipment object
    SOPC_QualifiedName eqtName;
    SOPC_EquipmentSensor lvl;
    SOPC_EquipmentSensor press;
    SOPC_EquipmentSensor temp;
} SOPC_EquipmentWithAlarm;

static void SOPC_EquipmentWithAlarm_Clear(void* eqtAux)
{
    SOPC_EquipmentWithAlarm* eqt = (SOPC_EquipmentWithAlarm*) eqtAux;
    SOPC_NodeId_Clear(&eqt->eqtId);
    SOPC_QualifiedName_Clear(&eqt->eqtName);
    SOPC_NodeId_Clear(&eqt->lvl.measureId);
    SOPC_NodeId_Clear(&eqt->lvl.lowAlarmId);
    SOPC_NodeId_Clear(&eqt->lvl.highAlarmId);
    SOPC_NodeId_Clear(&eqt->press.measureId);
    SOPC_NodeId_Clear(&eqt->press.lowAlarmId);
    SOPC_NodeId_Clear(&eqt->press.highAlarmId);
    SOPC_NodeId_Clear(&eqt->temp.measureId);
    SOPC_NodeId_Clear(&eqt->temp.lowAlarmId);
    SOPC_NodeId_Clear(&eqt->temp.highAlarmId);
}

#define MY_EQUIPMENT_TYPE_NS 1
static const SOPC_NodeId MyEquipmentTypeId = SOPC_NODEID_NUMERIC(MY_EQUIPMENT_TYPE_NS, 11004);
static const SOPC_NodeId MyHighAlarmTypeId = SOPC_NODEID_NUMERIC(MY_EQUIPMENT_TYPE_NS, 11002);
static const SOPC_NodeId MyLowAlarmTypeId = SOPC_NODEID_NUMERIC(MY_EQUIPMENT_TYPE_NS, 11003);

static const SOPC_NodeId HasTypeDefinitionId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasTypeDefinition);

#define NB_EQT_SENSORS 3
#define NB_SENSOR_VARS 3

static const SOPC_QualifiedName QN_EQT_SENSORS[NB_EQT_SENSORS] = {
    SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "Level"), SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "Pressure"),
    SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "Temperature")};

static const SOPC_QualifiedName QN_SENSOR_VARS[NB_SENSOR_VARS] = {
    SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "Measure"), SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "LowLimit"),
    SOPC_QUALIFIED_NAME(MY_EQUIPMENT_TYPE_NS, "HighLimit")};

// from min to max in 6 steps
#define DEFAULT_LEVEL_MIN 0.0
#define DEFAULT_LEVEL_MAX 1500.0
#define DEFAULT_LEVEL_DELTA 250.0
#define DEFAULT_LEVEL_VALUE 500.0
#define DEFAULT_LEVEL_LOW 500.0
#define DEFAULT_LEVEL_HIGH 1000.0

// from min to max in 5 steps
#define DEFAULT_PRESSURE_MIN 0.0
#define DEFAULT_PRESSURE_MAX 4.5
#define DEFAULT_PRESSURE_DELTA 0.5
#define DEFAULT_PRESSURE_VALUE 1.5
#define DEFAULT_PRESSURE_LOW 1.5
#define DEFAULT_PRESSURE_HIGH 3.0

// from min to max in 9 steps
#define DEFAULT_TEMPERATURE_MIN -20.0
#define DEFAULT_TEMPERATURE_MAX 70.0
#define DEFAULT_TEMPERATURE_DELTA 10.0
#define DEFAULT_TEMPERATURE_VALUE 20.0
#define DEFAULT_TEMPERATURE_LOW 10.0
#define DEFAULT_TEMPERATURE_HIGH 40.0

static const double DEFAULT_MIN_EQT_SENSORS[NB_EQT_SENSORS] = {DEFAULT_LEVEL_MIN, DEFAULT_PRESSURE_MIN,
                                                               DEFAULT_TEMPERATURE_MIN};
static const double DEFAULT_MAX_EQT_SENSORS[NB_EQT_SENSORS] = {DEFAULT_LEVEL_MAX, DEFAULT_PRESSURE_MAX,
                                                               DEFAULT_TEMPERATURE_MAX};
static const double DEFAULT_DELTA_EQT_SENSORS[NB_EQT_SENSORS] = {DEFAULT_LEVEL_DELTA, DEFAULT_PRESSURE_DELTA,
                                                                 DEFAULT_TEMPERATURE_DELTA};

static const double DEFAULT_VALUE_EQT_SENSORS[NB_EQT_SENSORS] = {DEFAULT_LEVEL_VALUE, DEFAULT_PRESSURE_VALUE,
                                                                 DEFAULT_TEMPERATURE_VALUE};
static const double LIMIT_LOW_EQT_SENSORS[NB_EQT_SENSORS] = {DEFAULT_LEVEL_LOW, DEFAULT_PRESSURE_LOW,
                                                             DEFAULT_TEMPERATURE_LOW};
static const double LIMIT_HIGH_EQT_SENSORS[NB_EQT_SENSORS] = {DEFAULT_LEVEL_HIGH, DEFAULT_PRESSURE_HIGH,
                                                              DEFAULT_TEMPERATURE_HIGH};

static SOPC_EquipmentSensor* GetEqtSensorFromIndex(SOPC_EquipmentWithAlarm* eqt, size_t idx)
{
    SOPC_EquipmentSensor* sensor = NULL;
    switch (idx)
    {
    case 0:
        sensor = &eqt->lvl;
        break;
    case 1:
        sensor = &eqt->press;
        break;
    case 2:
        sensor = &eqt->temp;
        break;
    default:
        SOPC_ASSERT(false);
        break;
    }

    return sensor;
}

static SOPC_NodeId* GetSensorVarIdFromIndex(SOPC_EquipmentSensor* eqtSensor, size_t idx)
{
    SOPC_NodeId* nid = NULL;
    switch (idx)
    {
    case 0:
        nid = &eqtSensor->measureId;
        break;
    case 1:
        nid = &eqtSensor->lowAlarmId;
        break;
    case 2:
        nid = &eqtSensor->highAlarmId;
        break;
    default:
        SOPC_ASSERT(false);
        break;
    }

    return nid;
}

static SOPC_ReturnStatus SOPC_EquipmentsWithAlarm_CreateCtx(size_t* nbEqt, SOPC_EquipmentWithAlarm** eqtArr)
{
    SOPC_ASSERT(NULL != nbEqt);
    SOPC_ASSERT(NULL != eqtArr);
    size_t nbResEqt = 0;
    SOPC_EquipmentWithAlarm* resEqtArr = NULL;

    // Browse MyEquipmentType node to get all (reverse HasTypeDefinition reference to) object instances in address space
    OpcUa_BrowseRequest* browseReq = SOPC_BrowseRequest_Create(1, 0, NULL);
    OpcUa_BrowseResponse* browseResp = NULL;
    SOPC_ReturnStatus status = (NULL != browseReq) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_BrowseRequest_SetBrowseDescription(browseReq, 0, &MyEquipmentTypeId,
                                                         OpcUa_BrowseDirection_Inverse, &HasTypeDefinitionId, false,
                                                         OpcUa_NodeClass_Object, OpcUa_BrowseResultMask_BrowseName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(browseReq, (void**) &browseResp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult))
        {
            resEqtArr = SOPC_Calloc((size_t) browseResp->Results[0].NoOfReferences, sizeof(SOPC_EquipmentWithAlarm));
            status = (NULL != resEqtArr) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
            if (SOPC_STATUS_OK == status)
            {
                for (int32_t i = 0; i < browseResp->Results[0].NoOfReferences; i++)
                {
                    SOPC_ExpandedNodeId* expNid = &browseResp->Results[0].References[i].NodeId;
                    SOPC_EquipmentWithAlarm* eqt = &resEqtArr[i];
                    if (expNid->NamespaceUri.Length <= 0 && 0 == expNid->ServerIndex)
                    {
                        SOPC_NodeId_Initialize(&eqt->eqtId);
                        status = SOPC_NodeId_Copy(&eqt->eqtId, &browseResp->Results[0].References[i].NodeId.NodeId);
                        nbResEqt = (SOPC_STATUS_OK == status ? nbResEqt + 1 : nbResEqt);
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        eqt->eqtName = browseResp->Results[0].References[i].BrowseName;
                        // equivalent to moving memory ownership
                        SOPC_QualifiedName_Initialize(&browseResp->Results[0].References[i].BrowseName);
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

    status = (SOPC_STATUS_OK == status
                  ? (((uint64_t) nbResEqt) * NB_EQT_SENSORS * NB_SENSOR_VARS <= SIZE_MAX ? SOPC_STATUS_OK
                                                                                         : SOPC_STATUS_OUT_OF_MEMORY)
                  : status);
    if (SOPC_STATUS_OK == status)
    {
        // For each equipment instance retrieve node ids of level, pressure and temperature variables and alarms
        OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpReq =
            SOPC_TranslateBrowsePathsRequest_Create(nbResEqt * NB_EQT_SENSORS * NB_SENSOR_VARS);
        OpcUa_TranslateBrowsePathsToNodeIdsResponse* tbpResp = NULL;
// Path element array init
#define nbRelPathElts 2
        OpcUa_RelativePathElement relPathElts[nbRelPathElts];
        SOPC_Initialize_Array((int32_t) nbRelPathElts, relPathElts, sizeof(*relPathElts),
                              OpcUa_RelativePathElement_Initialize);
        relPathElts[0].IsInverse = false;
        relPathElts[1].IsInverse = false;
        status = (NULL != tbpReq) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        size_t tbpIdx = 0;
        for (size_t i = 0; SOPC_STATUS_OK == status && i < nbResEqt; i++)
        {
            for (size_t j = 0; j < NB_EQT_SENSORS; j++)
            {
                relPathElts[0].TargetName = QN_EQT_SENSORS[j];
                for (size_t k = 0; k < NB_SENSOR_VARS; k++)
                {
                    relPathElts[1].TargetName = QN_SENSOR_VARS[k];
                    OpcUa_RelativePathElement* relPathEltsCopy = SOPC_Malloc(sizeof(relPathElts));
                    if (NULL != relPathEltsCopy)
                    {
                        relPathEltsCopy = memcpy(relPathEltsCopy, relPathElts, sizeof(relPathElts));
                        status = SOPC_TranslateBrowsePathRequest_SetPath(tbpReq, tbpIdx, &resEqtArr[i].eqtId,
                                                                         nbRelPathElts, relPathEltsCopy);
                    }
                    tbpIdx = (SOPC_STATUS_OK == status ? tbpIdx + 1 : tbpIdx);
                }
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ASSERT(tbpIdx == nbResEqt * NB_EQT_SENSORS * NB_SENSOR_VARS);
            status = SOPC_ServerHelper_LocalServiceSync(tbpReq, (void**) &tbpResp);
            status =
                (SOPC_STATUS_OK == status && SOPC_IsGoodStatus(tbpResp->ResponseHeader.ServiceResult) ? SOPC_STATUS_OK
                                                                                                      : status);
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ASSERT(nbResEqt * NB_EQT_SENSORS * NB_SENSOR_VARS == (size_t) tbpResp->NoOfResults);
            for (size_t i = 0; i < nbResEqt; i++)
            {
                SOPC_EquipmentWithAlarm* eqt = &resEqtArr[i];
                for (size_t j = 0; j < NB_EQT_SENSORS; j++)
                {
                    SOPC_EquipmentSensor* eqtSensor = GetEqtSensorFromIndex(eqt, j);
                    // Set a default value for the sensor measure
                    eqtSensor->value = DEFAULT_VALUE_EQT_SENSORS[j];
                    for (size_t k = 0; k < NB_SENSOR_VARS; k++)
                    {
                        tbpIdx = i * NB_EQT_SENSORS * NB_SENSOR_VARS + j * NB_SENSOR_VARS + k;
                        SOPC_ASSERT(1 == tbpResp->Results[tbpIdx].NoOfTargets);
                        SOPC_ASSERT(UINT32_MAX == tbpResp->Results[tbpIdx].Targets[0].RemainingPathIndex);
                        SOPC_ASSERT(0 >= tbpResp->Results[tbpIdx].Targets[0].TargetId.NamespaceUri.Length);
                        SOPC_ASSERT(0 == tbpResp->Results[tbpIdx].Targets[0].TargetId.ServerIndex);

                        SOPC_NodeId* nid = &tbpResp->Results[tbpIdx].Targets->TargetId.NodeId;
                        SOPC_NodeId* sensorVarId = GetSensorVarIdFromIndex(eqtSensor, k);
                        status = SOPC_NodeId_Copy(sensorVarId, nid);
                        if (SOPC_STATUS_OK != status)
                        {
                            char* cstrNid = SOPC_NodeId_ToCString(nid);
                            SOPC_Logger_TraceError(
                                SOPC_LOG_MODULE_CLIENTSERVER,
                                "Demo_Server_A&C: Failed to copy node id %s for managing sensor variable", cstrNid);
                            SOPC_Free(cstrNid);
                            // Variable will not be managed
                            status = SOPC_STATUS_OK;
                        }
                    }
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
        SOPC_Free(resEqtArr);
    }
    return status;
}

static SOPC_ReturnStatus SOPC_InternalSensorInitEvents_Create(bool enabled,
                                                              const SOPC_Event* lowAlarmEventBase,
                                                              const SOPC_Event* highAlarmEventBase,
                                                              const SOPC_EquipmentWithAlarm* eqt,
                                                              SOPC_EquipmentSensor* sensor,
                                                              const SOPC_QualifiedName* sensorName,
                                                              SOPC_Event** lowAlarmEvent,
                                                              SOPC_Event** highAlarmEvent)
{
    SOPC_UNUSED_ARG(eqt);
    SOPC_Event* lowAlarmRes = SOPC_Event_CreateCopy(lowAlarmEventBase, true);
    SOPC_Event* highAlarmRes = SOPC_Event_CreateCopy(highAlarmEventBase, true);
    SOPC_ReturnStatus status =
        (NULL != lowAlarmRes && NULL != highAlarmRes) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    if (SOPC_STATUS_OK == status)
    {
        char* prevSourceName = NULL;
        char* newSourceName = NULL;
        status = SOPC_StrConcat(SOPC_String_GetRawCString(&eqt->eqtName.Name), ".", &prevSourceName);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        status = SOPC_StrConcat(prevSourceName, SOPC_String_GetRawCString(&sensorName->Name), &newSourceName);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_Free(prevSourceName);
        prevSourceName = newSourceName;
        status = SOPC_StrConcat(prevSourceName, ".Measure", &newSourceName);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        SOPC_Free(prevSourceName);
        prevSourceName = newSourceName;

        SOPC_String stringSourceName;
        SOPC_String_Initialize(&stringSourceName);
        status = SOPC_String_AttachFromCstring(&stringSourceName, newSourceName);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetSourceName(lowAlarmRes, &stringSourceName);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetSourceName(highAlarmRes, &stringSourceName);
        }
        SOPC_Free(newSourceName);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Note: by default alarms are enabled, it is actually only necessary when enable = false
        SOPC_Variant enabledVar = SOPC_VARIANT_BOOL(enabled);
        status =
            SOPC_Event_SetVariableFromStrPath(lowAlarmRes, "0:EnabledState" QN_PATH_SEPARATOR_STR "0:Id", &enabledVar);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetVariableFromStrPath(highAlarmRes, "0:EnabledState" QN_PATH_SEPARATOR_STR "0:Id",
                                                       &enabledVar);
        }
    }

    // Set the sensor measure Node as source and input for Alarm instance
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetSourceNode(lowAlarmRes, &sensor->measureId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetSourceNode(highAlarmRes, &sensor->measureId);
    }
    SOPC_Variant sourceNodeVar = SOPC_VARIANT_NODEID(SOPC_NODEID_NS0_NUMERIC(0));
    sourceNodeVar.Value.NodeId = &sensor->measureId;
    if (SOPC_STATUS_OK == status)

    {
        status = SOPC_Event_SetVariableFromStrPath(lowAlarmRes, "0:InputNode", &sourceNodeVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(highAlarmRes, "0:InputNode", &sourceNodeVar);
    }
    // Set the Active alarm state to False, Enabled state is True by default
    SOPC_Variant activeVar = SOPC_VARIANT_BOOL(false);
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_Event_SetVariableFromStrPath(lowAlarmRes, "0:ActiveState" QN_PATH_SEPARATOR_STR "0:Id", &activeVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_Event_SetVariableFromStrPath(highAlarmRes, "0:ActiveState" QN_PATH_SEPARATOR_STR "0:Id", &activeVar);
    }
    // Set both AckedState and ConfirmState to True by default
    SOPC_Variant trueVar = SOPC_VARIANT_BOOL(true);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(lowAlarmRes, "0:AckedState" QN_PATH_SEPARATOR_STR "0:Id", &trueVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(highAlarmRes, "0:AckedState" QN_PATH_SEPARATOR_STR "0:Id", &trueVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_Event_SetVariableFromStrPath(lowAlarmRes, "0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:Id", &trueVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_Event_SetVariableFromStrPath(highAlarmRes, "0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:Id", &trueVar);
    }

    if (SOPC_STATUS_OK == status)
    {
        *lowAlarmEvent = lowAlarmRes;
        *highAlarmEvent = highAlarmRes;
    }
    else
    {
        SOPC_Event_Delete(&lowAlarmRes);
        SOPC_Event_Delete(&highAlarmRes);
    }
    return status;
}

static void SOPC_EnabledStateChanged(uintptr_t userCbCtx,
                                     SOPC_AlarmCondition* alarmCond,
                                     const char* qnPath,
                                     const SOPC_Variant* prevValue,
                                     const SOPC_Variant* newValue)
{
    SOPC_UNUSED_ARG(userCbCtx);
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == newValue->ArrayType);
    bool enabled = false;
    if (SOPC_Boolean_Id == newValue->BuiltInTypeId)
    {
        enabled = newValue->Value.Boolean;
    }
    if (!enabled)
    {
        (void) SOPC_AlarmCondition_SetAcknowledgeable(alarmCond, false, NULL);
        (void) SOPC_AlarmCondition_SetActiveState(alarmCond, false, NULL);
    }
}

static void SOPC_ConfirmedStateChanged(uintptr_t userCbCtx,
                                       SOPC_AlarmCondition* alarmCond,
                                       const char* qnPath,
                                       const SOPC_Variant* prevValue,
                                       const SOPC_Variant* newValue)
{
    SOPC_UNUSED_ARG(userCbCtx);
    SOPC_UNUSED_ARG(alarmCond);
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == newValue->ArrayType);
    bool confirmed = false;
    if (SOPC_Boolean_Id == newValue->BuiltInTypeId)
    {
        confirmed = newValue->Value.Boolean;
    }
    SOPC_UNUSED_RESULT(confirmed);
}

static SOPC_ReturnStatus SOPC_EquipmentsWithAlarm_Initialize(size_t nbEqt, SOPC_EquipmentWithAlarm* eqtArr)
{
    // Create base events reused to initialized each alarm instance
    SOPC_Event* lowAlarmEventBase = NULL;
    SOPC_Event* highAlarmEventBase = NULL;
    SOPC_ReturnStatus status = SOPC_ServerHelper_CreateEvent(&MyLowAlarmTypeId, &lowAlarmEventBase);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_CreateEvent(&MyHighAlarmTypeId, &highAlarmEventBase);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Event_Delete(&lowAlarmEventBase);
        SOPC_Event_Delete(&highAlarmEventBase);
        return status;
    }
    const SOPC_Variant defaultQuality = SOPC_VARIANT_STATUSCODE(SOPC_GoodGenericStatus);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(lowAlarmEventBase, "0:Quality", &defaultQuality);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(highAlarmEventBase, "0:Quality", &defaultQuality);
    }

    const SOPC_Variant defaultQualityTs = SOPC_VARIANT_DATE(SOPC_Time_GetCurrentTimeUTC());
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(
            lowAlarmEventBase, "0:Quality" QN_PATH_SEPARATOR_STR "0:SourceTimestamp", &defaultQualityTs);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(
            highAlarmEventBase, "0:Quality" QN_PATH_SEPARATOR_STR "0:SourceTimestamp", &defaultQualityTs);
    }

    const SOPC_Variant defaultComment = SOPC_VARIANT_LOCALTEXT("", "");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(lowAlarmEventBase, "0:Comment", &defaultComment);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(highAlarmEventBase, "0:Comment", &defaultComment);
    }

    const SOPC_Variant defaultSuppressed = SOPC_VARIANT_BOOL(false);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(lowAlarmEventBase, "0:SuppressedOrShelved", &defaultSuppressed);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(highAlarmEventBase, "0:SuppressedOrShelved", &defaultSuppressed);
    }

    // Set Message field for low and high limit alarms
    SOPC_LocalizedText lowLimitText = SOPC_LOCALIZED_TEXT("", "LowLimit");
    SOPC_LocalizedText highLimitText = SOPC_LOCALIZED_TEXT("", "HighLimit");
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetMessage(lowAlarmEventBase, &lowLimitText);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetMessage(highAlarmEventBase, &highLimitText);
    }

    // Fill a write request to set initial value of the sensors measure
    OpcUa_WriteRequest* measureWriteReq = NULL;
    OpcUa_WriteResponse* measureWriteResp = NULL;
    if (SOPC_STATUS_OK == status)
    {
        measureWriteReq = SOPC_WriteRequest_Create(nbEqt * NB_EQT_SENSORS);
        status = (NULL != measureWriteReq) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_Event* lowAlarmEvent = NULL;
    SOPC_Event* highAlarmEvent = NULL;
    SOPC_AlarmCondition* lowAlarm = NULL;
    SOPC_AlarmCondition* highAlarm = NULL;
    // For each equipment
    for (size_t i = 0; i < nbEqt && SOPC_STATUS_OK == status; i++)
    {
        SOPC_EquipmentWithAlarm* eqt = &eqtArr[i];
        SOPC_EquipmentSensor* sensors[3] = {&eqt->lvl, &eqt->press, &eqt->temp};
        // For each sensor
        for (size_t j = 0; j < NB_EQT_SENSORS && SOPC_STATUS_OK == status; j++)
        {
            SOPC_EquipmentSensor* sensor = sensors[j];

            // Set initial values expected for the low and high alarms
            status = SOPC_InternalSensorInitEvents_Create(0 == j, lowAlarmEventBase, highAlarmEventBase, eqt, sensor,
                                                          &QN_EQT_SENSORS[j], &lowAlarmEvent, &highAlarmEvent);

            // Create the alarms instances
            if (SOPC_STATUS_OK == status)
            {
                status =
                    SOPC_AlarmCondition_CreateFromEvent(&eqt->eqtId, &sensor->lowAlarmId, lowAlarmEvent, &lowAlarm);
            }
            if (SOPC_STATUS_OK == status)
            {
                status =
                    SOPC_AlarmCondition_CreateFromEvent(&eqt->eqtId, &sensor->highAlarmId, highAlarmEvent, &highAlarm);
            }

            // Configure the alarms behavior on Activation (=> Acknowledgeable)
            // and on Acknowledge states (Acknowledged => Confirmable, Acknowledgeable => Not confirmable)
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetAutoAcknowledgeable(lowAlarm);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetAutoConfirmable(lowAlarm);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetAutoRetain(lowAlarm);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetAutoAcknowledgeable(highAlarm);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetAutoConfirmable(highAlarm);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetAutoRetain(highAlarm);
            }

            // Configure the alarms callbacks on enable
            if (SOPC_STATUS_OK == status)
            {
                status =
                    SOPC_AlarmCondition_SetEnabledStateCallback(lowAlarm, &SOPC_EnabledStateChanged, (uintptr_t) NULL);
            }
            if (SOPC_STATUS_OK == status)
            {
                status =
                    SOPC_AlarmCondition_SetEnabledStateCallback(highAlarm, &SOPC_EnabledStateChanged, (uintptr_t) NULL);
            }

            // Configure the alarms callbacks on confirmed
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetConfirmedStateCallback(lowAlarm, &SOPC_ConfirmedStateChanged,
                                                                       (uintptr_t) NULL);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_AlarmCondition_SetConfirmedStateCallback(highAlarm, &SOPC_ConfirmedStateChanged,
                                                                       (uintptr_t) NULL);
            }

            // Set the initial value of the sensor measure
            if (SOPC_STATUS_OK == status)
            {
                SOPC_Variant measureVar = SOPC_VARIANT_DOUBLE(sensor->value);
                SOPC_DataValue measureDv;
                SOPC_DataValue_Initialize(&measureDv);
                measureDv.Value = measureVar;
                measureDv.Status = OpcUa_BadResourceUnavailable;
                status = SOPC_WriteRequest_SetWriteValue(measureWriteReq, i * NB_EQT_SENSORS + j, &sensor->measureId,
                                                         SOPC_AttributeId_Value, NULL, &measureDv);
            }

            if (SOPC_STATUS_OK == status)
            {
                sensor->lowAlarm = lowAlarm;
                sensor->highAlarm = highAlarm;
            }
            else
            {
                SOPC_Event_Delete(&lowAlarmEvent);
                SOPC_Event_Delete(&highAlarmEvent);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_WriteRequest measureWriteReqCopy;
        OpcUa_WriteRequest_Initialize(&measureWriteReqCopy);
        status = SOPC_EncodeableObject_Copy(&OpcUa_WriteRequest_EncodeableType, &measureWriteReqCopy, measureWriteReq);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerHelper_LocalServiceSync(measureWriteReq, (void**) &measureWriteResp);
        }
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_IsGoodStatus(measureWriteResp->ResponseHeader.ServiceResult) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            for (int32_t i = 0; i < measureWriteResp->NoOfResults && SOPC_STATUS_OK == status; i++)
            {
                if (!SOPC_IsGoodStatus(measureWriteResp->Results[i]))
                {
                    char* nodeIdStr = SOPC_NodeId_ToCString(&measureWriteReqCopy.NodesToWrite[i].NodeId);
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                             "Failed to set initial value of sensor measure %s", nodeIdStr);
                    SOPC_Free(nodeIdStr);
                }
            }
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Failed to set initial values of sensors measures");
            }
            (void) SOPC_EncodeableObject_Clear(measureWriteReqCopy.encodeableType, &measureWriteReqCopy);
            (void) SOPC_EncodeableObject_Delete(measureWriteResp->encodeableType, (void**) &measureWriteResp);
        }
    }
    else if (SOPC_STATUS_TIMEOUT != status) // in timeout case, deallocated by LocalServiceSync
    {
        (void) SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &measureWriteReq);
    }
    SOPC_Event_Delete(&lowAlarmEventBase);
    SOPC_Event_Delete(&highAlarmEventBase);

    return status;
}

static void UpdateSensorValue(OpcUa_WriteRequest* wr,
                              size_t wrIdx,
                              SOPC_EquipmentSensor* eqtSensor,
                              double min,
                              double max,
                              double low,
                              double high,
                              double delta)
{
    // Compute new value and fill the write value request item
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    dv.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
    dv.Value.BuiltInTypeId = SOPC_Double_Id;
    if (eqtSensor->increasing && eqtSensor->value + delta > max)
    {
        eqtSensor->increasing = false;
    }
    else if (!eqtSensor->increasing && eqtSensor->value - delta < min)
    {
        eqtSensor->increasing = true;
    }

    if (eqtSensor->increasing)
    {
        // Prev value + delta
        dv.Value.Value.Doublev = eqtSensor->value + delta;
        eqtSensor->value = dv.Value.Value.Doublev;
    }
    else
    {
        // Prev value - delta
        dv.Value.Value.Doublev = eqtSensor->value - delta;
        eqtSensor->value = dv.Value.Value.Doublev;
    }

    SOPC_ReturnStatus status =
        SOPC_WriteRequest_SetWriteValue(wr, (size_t) wrIdx, &eqtSensor->measureId, SOPC_AttributeId_Value, NULL, &dv);
    assert(SOPC_STATUS_OK == status);

    // Update the alarms states
    // Low limit newly reached triggers the low limit alarm
    if (SOPC_AlarmCondition_GetEnabledState(eqtSensor->lowAlarm) && eqtSensor->value <= low &&
        !SOPC_AlarmCondition_GetActiveState(eqtSensor->lowAlarm))
    {
        status = SOPC_AlarmCondition_SetActiveState(eqtSensor->lowAlarm, true, NULL);
        if (SOPC_STATUS_OK != status)
        {
            char* lowAlarmIdStr = SOPC_NodeId_ToCString(&eqtSensor->lowAlarmId);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to set active state of low alarm: %s",
                                   lowAlarmIdStr);
            SOPC_Free(lowAlarmIdStr);
        }
    }
    // High limit newly reached triggers the high limit alarm
    else if (SOPC_AlarmCondition_GetEnabledState(eqtSensor->highAlarm) && eqtSensor->value >= high &&
             !SOPC_AlarmCondition_GetActiveState(eqtSensor->highAlarm))
    {
        status = SOPC_AlarmCondition_SetActiveState(eqtSensor->highAlarm, true, NULL);
        if (SOPC_STATUS_OK != status)
        {
            char* highAlarmIdStr = SOPC_NodeId_ToCString(&eqtSensor->highAlarmId);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to set active state of high alarm: %s",
                                   highAlarmIdStr);
            SOPC_Free(highAlarmIdStr);
        }
    }
    // Low limit is not triggered anymore
    if (eqtSensor->value > low && SOPC_AlarmCondition_GetActiveState(eqtSensor->lowAlarm))
    {
        status = SOPC_AlarmCondition_SetActiveState(eqtSensor->lowAlarm, false, NULL);
        // Keep previous active alarm acknowledged / confirmed state until next activation
    }
    // High limit is not triggered anymore
    else if (eqtSensor->value < high && SOPC_AlarmCondition_GetActiveState(eqtSensor->highAlarm))
    {
        status = SOPC_AlarmCondition_SetActiveState(eqtSensor->highAlarm, false, NULL);
        // Keep previous active alarm acknowledged / confirmed state until next activation
    }
}

static void SOPC_LooperEventHandler_UpdateEqtSensors(SOPC_EventHandler* handler,
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

    OpcUa_WriteRequest* wr = SOPC_WriteRequest_Create(nbEqt * NB_EQT_SENSORS);
    OpcUa_WriteResponse* resp = NULL;
    if (NULL == wr)
    {
        return;
    }

    // Update the sensors values in context structure and associated low and high limits alarms states
    for (uint32_t i = 0; i < nbEqt; i++)
    {
        for (uint32_t j = 0; j < NB_EQT_SENSORS; j++)
        {
            SOPC_EquipmentSensor* eqtSensor = GetEqtSensorFromIndex(&eqtArr[i], j);
            SOPC_ASSERT(eqtSensor != NULL);
            UpdateSensorValue(wr, i * NB_EQT_SENSORS + j, eqtSensor, DEFAULT_MIN_EQT_SENSORS[j],
                              DEFAULT_MAX_EQT_SENSORS[j], LIMIT_LOW_EQT_SENSORS[j], LIMIT_HIGH_EQT_SENSORS[j],
                              DEFAULT_DELTA_EQT_SENSORS[j]);
        }
    }

    // Actually write the new values into server address space
    OpcUa_WriteRequest measureWriteReqCopy;
    OpcUa_WriteRequest_Initialize(&measureWriteReqCopy);
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Copy(&OpcUa_WriteRequest_EncodeableType, &measureWriteReqCopy, wr);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(wr, (void**) &resp);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        for (int32_t i = 0; i < resp->NoOfResults && SOPC_STATUS_OK == status; i++)
        {
            if (!SOPC_IsGoodStatus(resp->Results[i]))
            {
                char* nodeIdStr = SOPC_NodeId_ToCString(&measureWriteReqCopy.NodesToWrite[i].NodeId);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to update value of sensor measure %s",
                                         nodeIdStr);
                SOPC_Free(nodeIdStr);
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to update sensors values (service failure)");
        }
        (void) SOPC_EncodeableObject_Delete(resp->encodeableType, (void**) &resp);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to update sensors values");
        if (SOPC_STATUS_TIMEOUT != status) // in timeout case, deallocated by LocalServiceSync
        {
            (void) SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &wr);
        }
    }
    (void) SOPC_EncodeableObject_Clear(measureWriteReqCopy.encodeableType, &measureWriteReqCopy);
}

// Looper thread in charge of updating variables and alarms
static SOPC_Looper* g_updateVarsEventLooper = NULL;
static uint32_t g_updateVarsTimerId = 0;
static SOPC_EquipmentWithAlarm* g_eqtArr = NULL;
size_t g_nbEqt = 0;

SOPC_ReturnStatus Test_Server_InitializeAlarms(void)
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
        g_updateVarsEventLooper = SOPC_Looper_Create("UpdateEquipmentSensors");
        status = (NULL != g_updateVarsEventLooper) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        if (SOPC_STATUS_OK == status)
        {
            updateVarsEH = SOPC_EventHandler_Create(g_updateVarsEventLooper, &SOPC_LooperEventHandler_UpdateEqtSensors);
            status = (NULL != updateVarsEH) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_LooperEvent event = {0, (uint32_t) g_nbEqt, (uintptr_t) g_eqtArr, 0};
            // Only 1 update for initial values
            g_updateVarsTimerId = SOPC_EventTimer_CreatePeriodic(updateVarsEH, event, DEFAULT_UPDATE_SENSORS_PERIOD_MS);
            status = (0 != g_updateVarsTimerId) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    return status;
}

void Test_Server_PreStopAlarms(void)
{
    // Stop timer for periodic updates
    SOPC_EventTimer_Cancel(g_updateVarsTimerId);
    g_updateVarsTimerId = 0;
    // Stop event handler + looper
    SOPC_Looper_Delete(g_updateVarsEventLooper);
    g_updateVarsEventLooper = NULL;
}

void Test_Server_ClearAlarms(void)
{
    int32_t arrSize = (int32_t) g_nbEqt;
    SOPC_Clear_Array(&arrSize, (void**) &g_eqtArr, sizeof(SOPC_EquipmentWithAlarm), SOPC_EquipmentWithAlarm_Clear);
}
