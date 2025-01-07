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

#include "libs2opc_internal_alarm_conditions.h"

#include "libs2opc_request_builder.h"
#include "libs2opc_server_internal.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_event_handler.h"
#include "sopc_event_manager.h"
#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "util_variant.h"

#include <string.h>

typedef struct
{
    SOPC_AlarmCondition* ac;
    const char** varPathArr;
} SOPC_InternalLocalService_Ctx;

#define QN_SRC_TS "0:SourceTimestamp"

const SOPC_InternalTwoStateVariableFieldsPaths cEnabledStatePaths = {
    .self = "0:EnabledState",
    .id = "0:EnabledState" QN_PATH_SEPARATOR_STR "0:Id",
    .transitionTime = "0:EnabledState" QN_PATH_SEPARATOR_STR "0:TransitionTime",
    .falseTrueStates = {"0:EnabledState" QN_PATH_SEPARATOR_STR "0:FalseState",
                        "0:EnabledState" QN_PATH_SEPARATOR_STR "0:TrueState"}};

const SOPC_InternalTwoStateVariableFieldsPaths cAckedStatePaths = {
    .self = "0:AckedState",
    .id = "0:AckedState" QN_PATH_SEPARATOR_STR "0:Id",
    .transitionTime = "0:AckedState" QN_PATH_SEPARATOR_STR "0:TransitionTime",
    .falseTrueStates = {"0:AckedState" QN_PATH_SEPARATOR_STR "0:FalseState",
                        "0:AckedState" QN_PATH_SEPARATOR_STR "0:TrueState"}};

const SOPC_InternalTwoStateVariableFieldsPaths cConfirmedStatePaths = {
    .self = "0:ConfirmedState",
    .id = "0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:Id",
    .transitionTime = "0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:TransitionTime",
    .falseTrueStates = {"0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:FalseState",
                        "0:ConfirmedState" QN_PATH_SEPARATOR_STR "0:TrueState"}};

const SOPC_InternalTwoStateVariableFieldsPaths cActiveStatePaths = {
    .self = "0:ActiveState",
    .id = "0:ActiveState" QN_PATH_SEPARATOR_STR "0:Id",
    .transitionTime = "0:ActiveState" QN_PATH_SEPARATOR_STR "0:TransitionTime",
    .falseTrueStates = {"0:ActiveState" QN_PATH_SEPARATOR_STR "0:FalseState",
                        "0:ActiveState" QN_PATH_SEPARATOR_STR "0:TrueState"}};

const char* cSeverityIdPath = "0:Severity";
const char* cLastSeverityIdPath = "0:LastSeverity";
const char* cQualityIdPath = "0:Quality";
const char* cCommentIdPath = "0:Comment";
const char* cClientUserIdPath = "0:ClientUserId";
const char* cEventIdPath = "0:EventId";
const char* cRetainPath = "0:Retain";

// The looper used to manage interaction between S2OPC thread (at least method call) and application callbacks ()
struct _SOPC_AlarmConditionConfig sopc_alarmConditionConfig = {
    .g_mutex = SOPC_INVALID_MUTEX,
    .g_alarmCondLooperEvtHdlr = NULL,
    .g_alarmConditionsDict = NULL, // Condition NodeId => AC
    .g_refreshEventsDict = NULL,   // Condition NodeId => latest event triggered by condition
};
static int32_t g_init = false;
static SOPC_MethodCallManager* g_mcm = NULL;
static SOPC_Looper* g_alarmCondLooper = NULL;

static const char* SourceTimestampPath = QN_PATH_SEPARATOR_STR QN_SRC_TS;

static const SOPC_NodeId AggregatesRefType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Aggregates);
static const SOPC_NodeId HasComponentRefType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);

bool SOPC_InternalAlarmConditionMgr_IsInit(void)
{
    return SOPC_Atomic_Int_Get(&g_init);
}

bool SOPC_InternalAlarmCondition_GetAutoConfOnAcked(SOPC_AlarmCondition* ac)
{
    if (NULL != ac)
    {
        return (bool) SOPC_Atomic_Int_Get(&ac->autoConfOnAckedFlag);
    }
    return false;
}

void SOPC_InternalAlarmCondition_SetAutoConfOnAcked(SOPC_AlarmCondition* ac)
{
    if (NULL != ac)
    {
        SOPC_Atomic_Int_Set(&ac->autoConfOnAckedFlag, true);
    }
}

bool SOPC_InternalAlarmCondition_GetAutoAckableOnActive(SOPC_AlarmCondition* ac)
{
    if (NULL != ac)
    {
        return (bool) SOPC_Atomic_Int_Get(&ac->autoAckOnActiveFlag);
    }
    return false;
}

void SOPC_InternalAlarmCondition_SetAutoAckableOnActive(SOPC_AlarmCondition* ac)
{
    if (NULL != ac)
    {
        SOPC_Atomic_Int_Set(&ac->autoAckOnActiveFlag, true);
    }
}

bool SOPC_InternalAlarmCondition_GetAutoRetain(SOPC_AlarmCondition* ac)
{
    if (NULL != ac)
    {
        return (bool) SOPC_Atomic_Int_Get(&ac->autoRetainFlag);
    }
    return false;
}

void SOPC_InternalAlarmCondition_SetAutoRetain(SOPC_AlarmCondition* ac)
{
    if (NULL != ac)
    {
        SOPC_Atomic_Int_Set(&ac->autoRetainFlag, true);
    }
}

static void SOPC_RefreshEvent_Free(uintptr_t data)
{
    SOPC_Event_Delete((SOPC_Event**) &data);
}

static void SOPC_AlarmConditionValue_Free(uintptr_t ac)
{
    SOPC_ServerAlarmCondition_Delete((SOPC_AlarmCondition**) &ac);
}

char* SOPC_InternalVarToString(const SOPC_Variant* var)
{
    SOPC_ASSERT(NULL != var);
    SOPC_ASSERT(var->ArrayType == SOPC_VariantArrayType_SingleValue);
    SOPC_Buffer* buf = SOPC_Buffer_Create(200);
    SOPC_ReturnStatus dumpSt = SOPC_Variant_Dump(buf, var);
    if (SOPC_STATUS_OK == dumpSt)
    {
        dumpSt = SOPC_Buffer_Write(buf, (const uint8_t*) "", 1);
    }
    char* result = NULL;
    if (SOPC_STATUS_OK == dumpSt)
    {
        result = (char*) buf->data;
        buf->data = NULL;
    }
    SOPC_Buffer_Delete(buf);
    return result;
}

char* SOPC_InternalByteStringToString(const SOPC_ByteString* bs)
{
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Variant bsVar = {.ArrayType = SOPC_VariantArrayType_SingleValue,
                          .BuiltInTypeId = SOPC_ByteString_Id,
                          .DoNotClear = true,
                          .Value.Bstring = *(SOPC_ByteString*) bs};
    SOPC_GCC_DIAGNOSTIC_RESTORE

    return SOPC_InternalVarToString(&bsVar);
}

char* SOPC_InternalGetEventIdString(const SOPC_Event* event)
{
    const SOPC_ByteString* bs = SOPC_Event_GetEventId(event);
    if (NULL == bs || bs->Length <= 0)
    {
        return NULL;
    }
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Variant bsVar = {.ArrayType = SOPC_VariantArrayType_SingleValue,
                          .BuiltInTypeId = SOPC_ByteString_Id,
                          .DoNotClear = true,
                          .Value.Bstring = *(SOPC_ByteString*) bs};
    SOPC_GCC_DIAGNOSTIC_RESTORE

    return SOPC_InternalVarToString(&bsVar);
}

static void onTranslateBPresp(SOPC_InternalLocalService_Ctx* lsCtx, OpcUa_TranslateBrowsePathsToNodeIdsResponse* resp);

static void onStateChangeCbTrigger(SOPC_AlarmCondition_StateChanged_Fct_Ctx* fctCtx,
                                   const SOPC_Variant* prevVal,
                                   const SOPC_Variant* newVal);

// Treats (S2OPC) library events
static void onLibEvt(SOPC_EventHandler* handler, int32_t event, uint32_t eltId, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    SOPC_UNUSED_ARG(eltId);
    SOPC_InternalLocalService_Ctx* lsCtx = NULL;
    OpcUa_WriteResponse* writeResp = NULL;
    char* conditionIdStr = NULL;

    switch ((SOPC_ServerAlarmCondLibEvent) event)
    {
    case SOPC_AC_EVT_TBP_RESP:
        lsCtx = (SOPC_InternalLocalService_Ctx*) auxParam;
        onTranslateBPresp(lsCtx, (OpcUa_TranslateBrowsePathsToNodeIdsResponse*) params);
        break;
    case SOPC_AC_EVT_WRITE_RESP:
        writeResp = (OpcUa_WriteResponse*) params;
        lsCtx = (SOPC_InternalLocalService_Ctx*) auxParam;
        conditionIdStr = SOPC_NodeId_ToCString(&lsCtx->ac->conditionNode);
        for (int32_t i = 0; i < writeResp->NoOfResults; i++)
        {
            if (!SOPC_IsGoodStatus(writeResp->Results[i]))
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Error writing AlarmCondition %s variable %s with status 0x%08" PRIX32,
                                       conditionIdStr, lsCtx->varPathArr[i], writeResp->Results[i]);
            }
        }
        SOPC_Free(conditionIdStr);
        SOPC_Free((void*) lsCtx->varPathArr); // MSVC 15 considers array is const otherwise
        SOPC_Free(lsCtx);
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) params, (void**) &params);
        break;
    case SOPC_AC_EVT_METHOD_CALL_SET_STATE:
    case SOPC_AC_EVT_METHOD_CALL_COND_REFRESH:
    case SOPC_AC_EVT_METHOD_CALL_FAILURE:
        onMethodCallLibEvt(handler, event, eltId, params, auxParam);
        break;
    case SOPC_AC_EVT_STATE_CHANGED_CB:
        onStateChangeCbTrigger((SOPC_AlarmCondition_StateChanged_Fct_Ctx*) params, &((SOPC_Variant*) auxParam)[0],
                               &((SOPC_Variant*) auxParam)[1]);
        SOPC_Variant_Clear(&((SOPC_Variant*) auxParam)[0]);
        SOPC_Variant_Clear(&((SOPC_Variant*) auxParam)[1]);
        SOPC_Free((SOPC_Variant*) auxParam); // Free copy of variants
        break;
    default:
        SOPC_ASSERT(false);
    }
}

// Callback managing local service calls: mainly redispatch to A&C looper
static void SOPC_ServerAlarmCondition_LocalServiceAsyncResp_Fct(SOPC_EncodeableType* type,
                                                                void* response,
                                                                uintptr_t userContext)
{
    SOPC_HelperConfigInternal_Ctx* helperCtx = (SOPC_HelperConfigInternal_Ctx*) userContext;
    SOPC_ServerAlarmCondLibEvent libEvt = 0;
    bool postToLooper = false;
    bool moveRespToParams = false;
    uintptr_t params = (uintptr_t) NULL;

    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return;
    }

    if (&OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType == type)
    {
        postToLooper = true;
        moveRespToParams = true;
        libEvt = SOPC_AC_EVT_TBP_RESP;
    }
    else if (&OpcUa_WriteResponse_EncodeableType == type)
    {
        libEvt = SOPC_AC_EVT_WRITE_RESP;
        OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) response;
        if (SOPC_IsGoodStatus(writeResp->ResponseHeader.ServiceResult))
        {
            for (int32_t i = 0; i < writeResp->NoOfResults; i++)
            {
                if (!SOPC_IsGoodStatus(writeResp->Results[i]))
                {
                    postToLooper = true;
                    moveRespToParams = true;
                }
            }
        }
        else
        {
            postToLooper = true;
            moveRespToParams = true;
        }
        // postToLooper == moveRespToParams == FALSE => all write succeeded, no need to do anything
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Unexpected local service response type %s with status code 0x%08" PRIX32
                               ". Error message is: %s",
                               (NULL != type ? type->TypeName : "<NULL>"),
                               (NULL != response ? ((OpcUa_ResponseHeader*) response)->ServiceResult : 0),
                               helperCtx->eventCtx.localService.internalErrorMsg);
    }
    if (postToLooper && moveRespToParams)
    {
        SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(type, (void**) &params);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_EncodeableObject_Move((void*) params, response);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Unexpected local service write response copy failure %d", status);

            postToLooper = false;
        }
    }
    bool res = postToLooper;
    if (postToLooper)
    {
        res = SOPC_InternalAlarmConditionMgr_IsInit();
        if (res)
        {
            SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            SOPC_ReturnStatus status = SOPC_EventHandler_Post(sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr,
                                                              (int32_t) libEvt, 0, params, helperCtx->userContext);
            res = (SOPC_STATUS_OK == status);
            mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        }
        if (!res)
        {
            SOPC_ReturnStatus ignoredStatus = SOPC_EncodeableObject_Delete(type, (void**) &params);
            SOPC_UNUSED_RESULT(ignoredStatus);
        }
    }
    if (!res)
    {
        // In case there is no post delete context (response is automatically deleted after call)
        SOPC_InternalLocalService_Ctx* lsCtx = (SOPC_InternalLocalService_Ctx*) helperCtx->userContext;
        if (NULL != lsCtx)
        {
            SOPC_Free((void*) lsCtx->varPathArr); // MSVC 15 considers array is const otherwise
            SOPC_Free(lsCtx);
        }
    }
}

SOPC_ReturnStatus SOPC_ServerAlarmConditionMgr_Initialize(SOPC_MethodCallManager* mcm)
{
    if (NULL == mcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (!SOPC_ServerInternal_IsConfiguring() || SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_ReturnStatus status = SOPC_Mutex_Initialization(&sopc_alarmConditionConfig.g_mutex);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    SOPC_ASSERT(NULL == g_alarmCondLooper);
    g_alarmCondLooper = SOPC_Looper_Create("ServerAlarmConditions");
    if (NULL == g_alarmCondLooper)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr = SOPC_EventHandler_Create(g_alarmCondLooper, onLibEvt);
        sopc_alarmConditionConfig.g_alarmConditionsDict =
            SOPC_NodeId_Dict_Create(false, &SOPC_AlarmConditionValue_Free);
        sopc_alarmConditionConfig.g_refreshEventsDict = SOPC_NodeId_Dict_Create(false, &SOPC_RefreshEvent_Free);
    }
    if (NULL == sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr ||
        NULL == sopc_alarmConditionConfig.g_alarmConditionsDict ||
        NULL == sopc_alarmConditionConfig.g_refreshEventsDict)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Internal_ConfigureMethodCallManager(mcm);
    }
    if (SOPC_STATUS_OK == status)
    {
        g_mcm = mcm;
        SOPC_Atomic_Int_Set(&g_init, true);
    }
    else
    {
        sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr = NULL;
        SOPC_Looper_Delete(g_alarmCondLooper);
        SOPC_Dict_Delete(sopc_alarmConditionConfig.g_alarmConditionsDict);
        SOPC_Dict_Delete(sopc_alarmConditionConfig.g_refreshEventsDict);
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Clear(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    return status;
}

void SOPC_ServerAlarmConditionMgr_Clear(void)
{
    if (!SOPC_ServerInternal_IsConfigClearable() || false == SOPC_Atomic_Int_Get(&g_init))
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Cannot clear Alarm & Conditions manager: server configuration not clearable or manager not initialized");
        return;
    }
    SOPC_Atomic_Int_Set(&g_init, false);
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    // SOPC_ServerInternal_IsConfigClearable ? => seems a good idea to avoid possible method call trying to use the
    // deallocated looper
    sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr = NULL;
    SOPC_Looper_Delete(g_alarmCondLooper);
    SOPC_Dict_Delete(sopc_alarmConditionConfig.g_alarmConditionsDict);
    SOPC_Dict_Delete(sopc_alarmConditionConfig.g_refreshEventsDict);
    g_mcm = NULL;
    mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Clear(&sopc_alarmConditionConfig.g_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

void SOPC_ServerAlarmCondition_Delete(SOPC_AlarmCondition** ppAlarmCond)
{
    if (NULL != ppAlarmCond && NULL != *ppAlarmCond)
    {
        SOPC_AlarmCondition* pAlarmCond = *ppAlarmCond;
        SOPC_Mutex_Clear(&pAlarmCond->mut);
        SOPC_NodeId_Clear(&pAlarmCond->notifierNode);
        SOPC_NodeId_Clear(&pAlarmCond->conditionNode);
        SOPC_Event_Delete(&pAlarmCond->data);
        SOPC_EventIdList_Delete(&pAlarmCond->ackEventIds);
        SOPC_EventIdList_Delete(&pAlarmCond->confEventIds);
        SOPC_EventIdList_Delete(&pAlarmCond->globalEventIds);
        SOPC_Event_Delete(&pAlarmCond->nodeIds);
        int32_t methodsCount = SOPC_AC_METHOD_COUNT;
        SOPC_Clear_Array(&methodsCount, (void**) &pAlarmCond->methodIds, sizeof(*pAlarmCond->methodIds),
                         &SOPC_NodeId_ClearAux);
        SOPC_Dict_Delete(pAlarmCond->varChangedCbDict);
        SOPC_Free(pAlarmCond);
        *ppAlarmCond = NULL;
    }
}

/* Write use for init management */

// Create a Write request and set value of event field value into each variable NodeId
static void SOPC_InternalWrite_InitAlarmCondition_Node_Values(SOPC_AlarmCondition* ac,
                                                              size_t noOfValidNodeIds,
                                                              size_t arrSize,
                                                              const SOPC_NodeId* nodeIdArr,
                                                              const char** prevVarPathArr)
{
    SOPC_ASSERT(NULL != ac);
    OpcUa_WriteRequest* writeReq = SOPC_WriteRequest_Create(noOfValidNodeIds);
    SOPC_InternalLocalService_Ctx* tbpCtx = SOPC_Calloc(1, sizeof(*tbpCtx));
    const char** varPathArr = SOPC_Calloc(noOfValidNodeIds, sizeof(*varPathArr));
    if (NULL == writeReq || NULL == tbpCtx || NULL == varPathArr)
    {
        SOPC_Free(writeReq);
        SOPC_Free(tbpCtx);
        SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
        return;
    }
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);
    size_t index = 0;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    for (size_t i = 0; i < arrSize; i++)
    {
        SOPC_ASSERT(index <= noOfValidNodeIds);
        if (nodeIdArr[i].IdentifierType != SOPC_IdentifierType_Numeric || nodeIdArr[i].Data.Numeric != 0)
        {
            const SOPC_Variant* var = SOPC_Event_GetVariableFromStrPath(ac->data, prevVarPathArr[i]);
            dv.Value = *var;
            SOPC_ReturnStatus status =
                SOPC_WriteRequest_SetWriteValue(writeReq, index, &nodeIdArr[i], SOPC_AttributeId_Value, NULL, &dv);
            if (SOPC_STATUS_OK != status)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Initialization of AlarmCondition %s SetWriteValue on variable %s failed with status %d",
                    conditionIdStr, prevVarPathArr[i], status);
                SOPC_Free(conditionIdStr);
            }
            varPathArr[index] = prevVarPathArr[i];
            index++;
        } // else it means it is not a valid Id (zeroed id)
    }
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    tbpCtx->ac = ac;
    tbpCtx->varPathArr = varPathArr;
    bool res = SOPC_ServerInternal_LocalServiceAsync(&SOPC_ServerAlarmCondition_LocalServiceAsyncResp_Fct, writeReq,
                                                     (uintptr_t) tbpCtx,
                                                     "Failure to treat Write to initialize A&C nodes from event.");
    if (!res)
    {
        // Delete context
        SOPC_Free(tbpCtx);
        SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
        tbpCtx = NULL;
        varPathArr = NULL;
    }
}

/* TranslateBrowsePath use for init management */

static void onTranslateBPresp(SOPC_InternalLocalService_Ctx* lsCtx, OpcUa_TranslateBrowsePathsToNodeIdsResponse* resp)
{
    // nodeId array to prepare the write request to initialize the node values
    SOPC_NodeId* nodeIdArr = NULL;

    if ((size_t) resp->NoOfResults > SOPC_AC_METHOD_COUNT)
    {
        nodeIdArr = SOPC_Calloc((size_t) resp->NoOfResults - SOPC_AC_METHOD_COUNT, sizeof(*nodeIdArr));
    }
    size_t noOfVarNodeIds = 0;

    // Variant to be store the node id in event-like form
    SOPC_Variant nodeIdVar;
    SOPC_Variant_Initialize(&nodeIdVar);
    nodeIdVar.ArrayType = SOPC_VariantArrayType_SingleValue;
    nodeIdVar.BuiltInTypeId = SOPC_NodeId_Id;

    SOPC_AlarmCondition* ac = lsCtx->ac;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    // Retrieve NodeId
    for (int32_t i = 0; NULL != nodeIdArr && i < resp->NoOfResults - SOPC_AC_METHOD_COUNT; i++)
    {
        if (SOPC_IsGoodStatus(resp->Results[i].StatusCode) && resp->Results[i].NoOfTargets > 0 &&
            0 == resp->Results[i].Targets[0].TargetId.ServerIndex)
        {
            // Stores the variable NodeId to write
            noOfVarNodeIds++;
            nodeIdArr[i] = resp->Results[i].Targets[0].TargetId.NodeId;

            // Record the NodeId for event fields
            // TODO: log if more than 1 target / serverIndex != 0
            nodeIdVar.Value.NodeId = &resp->Results[i].Targets[0].TargetId.NodeId;
            status = SOPC_Event_SetVariableFromStrPath(ac->nodeIds, lsCtx->varPathArr[i], &nodeIdVar);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
            // Reset temporary Variant for next iteration
            nodeIdVar.Value.NodeId = NULL;
            char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
            if (resp->Results[i].NoOfTargets > 1)
            {
                char* varIdStr = SOPC_NodeId_ToCString(&nodeIdArr[i]);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "TBPtoNodeId of AlarmCondition %s variable %s returned several target, only "
                                         "the first result %s was kept.",
                                         conditionIdStr, lsCtx->varPathArr[i], varIdStr);
                SOPC_Free(varIdStr);
            }
            else
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "TBPtoNodeId of AlarmCondition %s variable %s found.", conditionIdStr,
                                       lsCtx->varPathArr[i]);
            }
            SOPC_Free(conditionIdStr);
        }
        else if (resp->Results[i].NoOfTargets > 0 && 0 != resp->Results[i].Targets[0].TargetId.ServerIndex)
        {
            char* varIdStr = SOPC_NodeId_ToCString(&resp->Results[i].Targets[0].TargetId.NodeId);
            char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "TBPtoNodeId of AlarmCondition %s variable %s returned an external node serverIdx=%" PRIu32
                " nodeId=%s as result.  It will not be functional.",
                conditionIdStr, lsCtx->varPathArr[i], resp->Results[i].Targets[0].TargetId.ServerIndex, varIdStr);
            SOPC_Free(varIdStr);
            SOPC_Free(conditionIdStr);
        }
    }
    // Note: after this steps there are noOfVarNodeIds valid NodeIds in nodeIdVar, others are zeroed NodeId from alloc

    size_t methodIdx = 0;
    for (int32_t i = resp->NoOfResults - SOPC_AC_METHOD_COUNT; i < resp->NoOfResults; i++)
    {
        if (SOPC_IsGoodStatus(resp->Results[i].StatusCode) && resp->Results[i].NoOfTargets > 0 &&
            0 == resp->Results[i].Targets[0].TargetId.ServerIndex)
        {
            ac->methodIds[methodIdx] = resp->Results[i].Targets[0].TargetId.NodeId;
            SOPC_NodeId_Initialize(&resp->Results[i].Targets[0].TargetId.NodeId);
            char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
            if (resp->Results[i].NoOfTargets > 1)
            {
                char* varIdStr = SOPC_NodeId_ToCString(&ac->methodIds[methodIdx]);
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "TBPtoNodeId of AlarmCondition %s method %s returned several target, only "
                                         "the first result %s was kept.",
                                         conditionIdStr, SOPC_String_GetRawCString(&acMethodsNames[methodIdx].Name),
                                         varIdStr);
                SOPC_Free(varIdStr);
            }
            else
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "TBPtoNodeId of AlarmCondition %s method %s found.", conditionIdStr,
                                       SOPC_String_GetRawCString(&acMethodsNames[methodIdx].Name));
            }
            SOPC_Free(conditionIdStr);
        }
        else if (resp->Results[i].NoOfTargets > 0 && 0 != resp->Results[i].Targets[0].TargetId.ServerIndex)
        {
            char* varIdStr = SOPC_NodeId_ToCString(&resp->Results[i].Targets[0].TargetId.NodeId);
            char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "TBPtoNodeId of AlarmCondition %s method %s returned an external node serverIdx=%" PRIu32
                " nodeId=%s as result. It will not be functional.",
                conditionIdStr, SOPC_String_GetRawCString(&acMethodsNames[methodIdx].Name),
                resp->Results[i].Targets[0].TargetId.ServerIndex, varIdStr);
            SOPC_Free(varIdStr);
            SOPC_Free(conditionIdStr);
        }
        methodIdx++;
    }
    SOPC_InternalMethod_InitAlarmCondition_Methods(g_mcm, ac);
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (noOfVarNodeIds > SOPC_AC_METHOD_COUNT && NULL != nodeIdArr)
    {
        SOPC_InternalWrite_InitAlarmCondition_Node_Values(
            lsCtx->ac, noOfVarNodeIds, (size_t) resp->NoOfResults - SOPC_AC_METHOD_COUNT, nodeIdArr, lsCtx->varPathArr);
    }
    SOPC_Free(nodeIdArr);
    SOPC_Free((void*) lsCtx->varPathArr); // MSVC 15 considers array is const otherwise
    SOPC_Free(lsCtx);
    SOPC_EncodeableObject_Delete(resp->encodeableType, (void**) &resp);
}

// Ephemeral structure needed to initialize the TBP request items
typedef struct
{
    SOPC_AlarmCondition* ac;
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* req;
    size_t index;
    const char** varPathArr;
} _SOPC_InternalTBP_FillTBP_Ctx;

static void SOPC_InternalTBP_SetEventVarTBP_Cb(const char* qnPath,
                                               SOPC_Variant* var,
                                               const SOPC_NodeId* dataType,
                                               int32_t valueRank,
                                               uintptr_t fillTBPctx)
{
    SOPC_UNUSED_ARG(var);
    SOPC_UNUSED_ARG(dataType);
    SOPC_UNUSED_ARG(valueRank);
    _SOPC_InternalTBP_FillTBP_Ctx* ctx = (_SOPC_InternalTBP_FillTBP_Ctx*) fillTBPctx;
    int32_t nbPaths = 0;
    SOPC_QualifiedName* qnPathArr = NULL;
    OpcUa_RelativePathElement* relPathArr = NULL;
    SOPC_ReturnStatus status =
        SOPC_EventManagerUtil_cStringPathToQnPath(QN_PATH_SEPARATOR_CHAR, qnPath, &nbPaths, &qnPathArr);
    if (0 == nbPaths)
    {
        status = SOPC_STATUS_WOULD_BLOCK;
    }
    if (SOPC_STATUS_OK == status)
    {
        relPathArr = SOPC_Calloc((size_t) nbPaths, sizeof(*relPathArr));
        status = (NULL == relPathArr ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    }
    for (int32_t iPath = 0; SOPC_STATUS_OK == status && iPath < nbPaths; iPath++)
    {
        OpcUa_RelativePathElement* pe = &relPathArr[iPath];
        OpcUa_RelativePathElement_Initialize(pe);
        pe->ReferenceTypeId = AggregatesRefType_NodeId;
        pe->IncludeSubtypes = true;
        pe->IsInverse = false;
        pe->TargetName = qnPathArr[iPath];
        // reset source content => moved content
        SOPC_QualifiedName_Initialize(&qnPathArr[iPath]);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TranslateBrowsePathRequest_SetPath(ctx->req, ctx->index, &ctx->ac->conditionNode,
                                                         (size_t) nbPaths, relPathArr);
    }
    if (SOPC_STATUS_OK == status)
    {
        ctx->varPathArr[ctx->index] = qnPath;
        ctx->index++;
    }
    else
    {
        int32_t nbRpe = nbPaths;
        SOPC_Clear_Array(&nbRpe, (void**) &relPathArr, sizeof(*relPathArr), &OpcUa_RelativePathElement_Clear);
    }
    SOPC_Clear_Array(&nbPaths, (void**) &qnPathArr, sizeof(*qnPathArr), &SOPC_QualifiedName_ClearAux);
}

static void SOPC_InternalTBP_GetAlarmCondition_Method_NodeIds(size_t indexOffset,
                                                              const SOPC_AlarmCondition* ac,
                                                              OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpReq)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (size_t i = 0; i < SOPC_AC_METHOD_COUNT; i++)
    {
        OpcUa_RelativePathElement* relPathArr = NULL;
        if (SOPC_STATUS_OK == status)
        {
            relPathArr = SOPC_Calloc(1, sizeof(*relPathArr));
            status = (NULL == relPathArr ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
        }
        if (SOPC_STATUS_OK == status)
        {
            OpcUa_RelativePathElement_Initialize(relPathArr);
            relPathArr->ReferenceTypeId = HasComponentRefType_NodeId;
            relPathArr->IncludeSubtypes = false;
            relPathArr->IsInverse = false;
            status = SOPC_QualifiedName_Copy(&relPathArr->TargetName, &acMethodsNames[i]);
        }
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_TranslateBrowsePathRequest_SetPath(tbpReq, indexOffset + i, &ac->conditionNode, 1, relPathArr);
        } // else, empty TBP item request will fail and be ignored
    }
}

// Create a TranslateBrowsePath request and call local services to obtain node ids of nodes,
// on TBP response, write address space variables values
void SOPC_Internal_GetAlarmCondition_Vars_NodeIds_And_Write_Values(SOPC_AlarmCondition* ac)
{
    // Note: no need of using the AC mutex as it has not been returned to application yet !
    SOPC_ASSERT(NULL != ac);
    size_t nbVars = SOPC_Event_GetNbVariables(ac->data);
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* tbpReq =
        SOPC_TranslateBrowsePathsRequest_Create(nbVars + SOPC_AC_METHOD_COUNT);
    SOPC_InternalLocalService_Ctx* tbpCtx = SOPC_Calloc(1, sizeof(*tbpCtx));
    const char** varPathArr = SOPC_Calloc(nbVars, sizeof(*varPathArr));
    bool reqNeedDealloc = true;
    if (NULL == tbpReq || NULL == tbpCtx || NULL == varPathArr)
    {
        SOPC_Free(tbpReq);
        SOPC_Free(tbpCtx);
        SOPC_Free(varPathArr);
        return;
    }
    _SOPC_InternalTBP_FillTBP_Ctx fillCtx = {
        .ac = ac,
        .req = tbpReq,
        .index = 0,
        .varPathArr = varPathArr,
    };
    // Fill TBP request for each event field present
    SOPC_Event_ForEachVar(ac->data, &SOPC_InternalTBP_SetEventVarTBP_Cb, (uintptr_t) &fillCtx);
    if (fillCtx.index > 0)
    {
        if (fillCtx.index < nbVars) // adapt number of filled TBP for variables
        {
            tbpReq->NoOfBrowsePaths = (int32_t) fillCtx.index + SOPC_AC_METHOD_COUNT;
        }
        // Fill TBP request for each supported AandC method
        SOPC_InternalTBP_GetAlarmCondition_Method_NodeIds(fillCtx.index, ac, tbpReq);
        tbpCtx->ac = ac;
        tbpCtx->varPathArr = varPathArr;
        reqNeedDealloc = false;
        bool res = SOPC_ServerInternal_LocalServiceAsync(&SOPC_ServerAlarmCondition_LocalServiceAsyncResp_Fct, tbpReq,
                                                         (uintptr_t) tbpCtx,
                                                         "Failure to treat TBP to initialize A&C nodes from event.");
        if (!res)
        {
            // Delete context
            SOPC_Free(tbpCtx);
            SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
            tbpCtx = NULL;
            varPathArr = NULL;
        }
    } // else: log error
    if (reqNeedDealloc)
    {
        SOPC_EncodeableObject_Delete(tbpReq->encodeableType, (void**) &tbpReq);
        SOPC_Free(tbpCtx);
        SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
    }
}

SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_InitFields(SOPC_AlarmCondition* ac)
{
    // ENABLED
    // From part 9 §5.5.2 (v1.05.02):
    // [In the event of a restart of an AlarmManager ...].
    // If the system can not determine if the Condition is Enabled or Disabled, it shall be Enabled.
    bool enabled = true;
    const SOPC_Variant* varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cEnabledStatePaths.id);
    if (NULL == varVal)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    enabled = (SOPC_Boolean_Id == varVal->BuiltInTypeId ? varVal->Value.Boolean : enabled);
    SOPC_ReturnStatus status =
        SOPC_InternalAlarmCondition_SetBoolState(ac, &cEnabledStatePaths, enabled, NULL, NULL, false, false, false);

    SOPC_Variant setVarVal = {.ArrayType = SOPC_VariantArrayType_SingleValue,
                              .BuiltInTypeId = SOPC_Boolean_Id,
                              .DoNotClear = true,
                              .Value.Boolean = false};
    // RETAIN
    // From same part of specification:
    // When the Condition instance enters the Disabled state, the Retain Property of this
    // Condition shall be set to False by the Server
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cRetainPath);
        if (NULL == varVal)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (!enabled || SOPC_Boolean_Id != varVal->BuiltInTypeId)
        {
            status = SOPC_Event_SetVariableFromStrPath(ac->data, cRetainPath, &setVarVal);
        }
    }

    // QUALITY
    // From same part of specification:
    // A Server that supports no quality information shall return Good.
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cQualityIdPath);
        if (NULL == varVal)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        // If no value is defined, set it as Good
        if (SOPC_StatusCode_Id != varVal->BuiltInTypeId)
        {
            setVarVal.BuiltInTypeId = SOPC_StatusCode_Id;
            setVarVal.Value.Status = SOPC_GoodGenericStatus;
            status = SOPC_Event_SetVariableFromStrPath(ac->data, cQualityIdPath, &setVarVal);
        }
    }
    // SEVERITY
    // From same part of specification:
    // LastSeverity: Initially this Variable contains a zero value
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cLastSeverityIdPath);
        if (NULL == varVal)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        // If no value is defined, set it as 0
        if (SOPC_UInt16_Id != varVal->BuiltInTypeId)
        {
            setVarVal.BuiltInTypeId = SOPC_UInt16_Id;
            setVarVal.Value.Uint16 = 0;
            status = SOPC_Event_SetVariableFromStrPath(ac->data, cLastSeverityIdPath, &setVarVal);
        }
    }
    // Note: following variables might not exist (not in ConditionType) or might not be defined,
    //       only set the defined State/Id value to automatically set the associated State text value.
    // ACK: only set if it exists and value is set to automatically define associated text value
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cAckedStatePaths.id);
        if (NULL != varVal && SOPC_Boolean_Id == varVal->BuiltInTypeId)
        {
            status = SOPC_InternalAlarmCondition_SetBoolState(ac, &cAckedStatePaths, varVal->Value.Boolean, NULL, NULL,
                                                              false, false, false);
        }
    }
    // CONF: only set if it exists and value is set to automatically define associated text value
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cConfirmedStatePaths.id);
        if (NULL != varVal && SOPC_Boolean_Id == varVal->BuiltInTypeId)
        {
            status = SOPC_InternalAlarmCondition_SetBoolState(ac, &cConfirmedStatePaths, varVal->Value.Boolean, NULL,
                                                              NULL, false, false, false);
        }
    }
    // ACTIVE: only set if it exists and value is set to automatically define associated text value
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cActiveStatePaths.id);
        if (NULL != varVal && SOPC_Boolean_Id == varVal->BuiltInTypeId)
        {
            status = SOPC_InternalAlarmCondition_SetBoolState(ac, &cActiveStatePaths, varVal->Value.Boolean, NULL, NULL,
                                                              false, false, false);
        }
    }
    // CLIENTUSERID
    // ClientUserId: set it to the ClientUserId empty string value if NULL
    if (SOPC_STATUS_OK == status)
    {
        varVal = SOPC_Event_GetVariableFromStrPath(ac->data, cClientUserIdPath);
        if (NULL != varVal && SOPC_Null_Id == varVal->BuiltInTypeId)
        {
            setVarVal.BuiltInTypeId = SOPC_String_Id;
            SOPC_String_Initialize(&setVarVal.Value.String);
            status = SOPC_Event_SetVariableFromStrPath(ac->data, cClientUserIdPath, &setVarVal);
        }
    }
    return status;
}

static const SOPC_NodeId* SOPC_InternalAlarmConditionNoLock_GetNodeId(const SOPC_AlarmCondition* ac,
                                                                      const char* varPath)
{
    SOPC_ASSERT(NULL != ac);
    SOPC_ASSERT(NULL != varPath);
    const SOPC_NodeId* nodeId = NULL;
    const SOPC_Variant* varNodeId = NULL;
    varNodeId = SOPC_Event_GetVariableFromStrPath(ac->nodeIds, varPath);
    SOPC_ReturnStatus status =
        (NULL != varNodeId && SOPC_NodeId_Id == varNodeId->BuiltInTypeId ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == varNodeId->ArrayType);
        nodeId = varNodeId->Value.NodeId;
    }
    return nodeId;
}

static SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_UpdateVariableNode(SOPC_AlarmCondition* ac,
                                                                              const char* varPath,
                                                                              const SOPC_Variant* val,
                                                                              const SOPC_DateTime optSrcTs)

{
    SOPC_ASSERT(NULL != ac);
    SOPC_ASSERT(NULL != varPath);
    SOPC_ASSERT(NULL != val);
    // Get the node NodeId for event variable path
    const SOPC_NodeId* nodeId = SOPC_InternalAlarmConditionNoLock_GetNodeId(ac, varPath);
    SOPC_ReturnStatus status = (NULL != nodeId ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_STATE);
    // Write the event variable value into the matching node
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_WriteRequest* writeReq = SOPC_WriteRequest_Create(1);
        SOPC_InternalLocalService_Ctx* tbpCtx = SOPC_Calloc(1, sizeof(*tbpCtx));
        const char** varPathArr = SOPC_Calloc(1, sizeof(*varPathArr));
        if (NULL == writeReq || NULL == tbpCtx || NULL == varPathArr)
        {
            SOPC_Free(writeReq);
            SOPC_Free(tbpCtx);
            SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
            return SOPC_STATUS_OUT_OF_MEMORY;
        }
        SOPC_DataValue dv;
        SOPC_DataValue_Initialize(&dv);
        dv.Value = *val;
        // Set the source timestamp if requested
        {
            dv.SourceTimestamp = optSrcTs;
        }
        status = SOPC_WriteRequest_SetWriteValue(writeReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dv);
        bool res = false;
        if (SOPC_STATUS_OK == status)
        {
            tbpCtx->ac = ac;
            *varPathArr = varPath;
            tbpCtx->varPathArr = varPathArr;
            res = SOPC_ServerInternal_LocalServiceAsync(&SOPC_ServerAlarmCondition_LocalServiceAsyncResp_Fct, writeReq,
                                                        (uintptr_t) tbpCtx,
                                                        "Failure to treat Write to update value of variable A&C node");
        }
        if (!res)
        {
            // Delete context
            SOPC_Free(tbpCtx);
            SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
            tbpCtx = NULL;
            varPathArr = NULL;
            status = (SOPC_STATUS_OK == status ? SOPC_STATUS_NOK : status);
        }
    }
    return status;
}

static SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_UpdateTwoStateVariableNodes(
    SOPC_AlarmCondition* ac,
    const SOPC_InternalTwoStateVariableFieldsPaths* stateVarPaths,
    const SOPC_Variant* boolVal,
    const SOPC_Variant* textVal,
    const SOPC_Variant* transTimeVal)
{
    SOPC_ASSERT(NULL != boolVal);
    const SOPC_NodeId* stateNodeId = NULL;
    const SOPC_NodeId* stateIdNodeId = NULL;
    const SOPC_NodeId* stateTimeNodeId = NULL;

    size_t nbWrites = 0;
    // Retrieve variables: State value / Id / TransitionTime
    stateIdNodeId = SOPC_InternalAlarmConditionNoLock_GetNodeId(ac, stateVarPaths->id);
    SOPC_ReturnStatus status = (NULL == stateIdNodeId ? SOPC_STATUS_NOT_SUPPORTED : SOPC_STATUS_OK);
    // At least the Id node should be modifiable (actual state value)
    if (SOPC_STATUS_OK == status)
    {
        nbWrites++;
        if (NULL != textVal)
        {
            stateNodeId = SOPC_InternalAlarmConditionNoLock_GetNodeId(ac, stateVarPaths->self);
            nbWrites = (NULL != stateNodeId ? nbWrites + 1 : nbWrites);
        }
        if (NULL != transTimeVal)
        {
            stateTimeNodeId = SOPC_InternalAlarmConditionNoLock_GetNodeId(ac, stateVarPaths->transitionTime);
            nbWrites = (NULL != stateTimeNodeId ? nbWrites + 1 : nbWrites);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_WriteRequest* writeReq = SOPC_WriteRequest_Create(nbWrites);
        SOPC_InternalLocalService_Ctx* tbpCtx = SOPC_Calloc(1, sizeof(*tbpCtx));
        const char** varPathArr = SOPC_Calloc(nbWrites, sizeof(*varPathArr));
        if (NULL == writeReq || NULL == tbpCtx || NULL == varPathArr)
        {
            SOPC_Free(writeReq);
            SOPC_Free(tbpCtx);
            SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
            return SOPC_STATUS_OUT_OF_MEMORY;
        }
        size_t nbWriteIndex = 0;
        SOPC_DataValue dv;
        SOPC_DataValue_Initialize(&dv);
        dv.Value = *boolVal;
        status =
            SOPC_WriteRequest_SetWriteValue(writeReq, nbWriteIndex, stateIdNodeId, SOPC_AttributeId_Value, NULL, &dv);
        varPathArr[nbWriteIndex] = stateVarPaths->id;
        nbWriteIndex = (SOPC_STATUS_OK == status ? nbWriteIndex + 1 : nbWriteIndex);

        if (SOPC_STATUS_OK == status && NULL != stateNodeId)
        {
            dv.Value = *textVal;
            status =
                SOPC_WriteRequest_SetWriteValue(writeReq, nbWriteIndex, stateNodeId, SOPC_AttributeId_Value, NULL, &dv);
            varPathArr[nbWriteIndex] = stateVarPaths->self;
            nbWriteIndex = (SOPC_STATUS_OK == status ? nbWriteIndex + 1 : nbWriteIndex);
        }
        if (SOPC_STATUS_OK == status && NULL != stateTimeNodeId)
        {
            dv.Value = *transTimeVal;
            status = SOPC_WriteRequest_SetWriteValue(writeReq, nbWriteIndex, stateTimeNodeId, SOPC_AttributeId_Value,
                                                     NULL, &dv);
            varPathArr[nbWriteIndex] = stateVarPaths->transitionTime;
            nbWriteIndex = (SOPC_STATUS_OK == status ? nbWriteIndex + 1 : nbWriteIndex);
        }
        SOPC_UNUSED_RESULT(nbWriteIndex); // no further use

        bool res = false;
        if (SOPC_STATUS_OK == status)
        {
            tbpCtx->ac = ac;
            tbpCtx->varPathArr = varPathArr;
            res = SOPC_ServerInternal_LocalServiceAsync(&SOPC_ServerAlarmCondition_LocalServiceAsyncResp_Fct, writeReq,
                                                        (uintptr_t) tbpCtx,
                                                        "Failure to treat Write to update TwoState variable A&C nodes");
        }
        if (!res)
        {
            // Delete context
            SOPC_Free(tbpCtx);
            SOPC_Free((void*) varPathArr); // MSVC 15 considers array is const otherwise
            tbpCtx = NULL;
            varPathArr = NULL;
            status = (SOPC_STATUS_OK == status ? SOPC_STATUS_NOK : status);
        }
    }
    return status;
}

static SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_CommonSetVariableFromStrPath(
    SOPC_AlarmCondition* pAlarmCondition,
    const char* varPath,
    const SOPC_Variant* val,
    bool setTsVal,
    bool updateNodeVal)
{
    SOPC_ASSERT(NULL != pAlarmCondition);
    SOPC_ASSERT(NULL != varPath);
    SOPC_ASSERT(NULL != val);
    // Generate a sourceTimestamp and a variant with it
    SOPC_DateTime sourceTs = SOPC_Time_GetCurrentTimeUTC();
    SOPC_Variant valTs = {.ArrayType = SOPC_VariantArrayType_SingleValue,
                          .BuiltInTypeId = SOPC_DateTime_Id,
                          .DoNotClear = true,
                          .Value.Date = sourceTs};
    char* varSourceTsPath = NULL;
    const SOPC_Variant* valWithLTupdate = val;
    SOPC_Variant* prevValueCopy = NULL;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_LocalizedText_Id == val->BuiltInTypeId)
    {
        // Specific treatment for LocalizedText which shall be updated and not simply replaced
        const SOPC_Variant* prevValue = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, varPath);
        prevValueCopy = util_variant__new_Variant_from_Variant(prevValue, true);
        char** supportedLocales = SOPC_ServerConfigHelper_GetLocaleIds();
        if (NULL != supportedLocales && NULL != prevValueCopy && SOPC_VariantArrayType_SingleValue == val->ArrayType &&
            SOPC_LocalizedText_Id == prevValue->BuiltInTypeId)
        {
            status = util_variant__update_applying_supported_locales(prevValueCopy, val, supportedLocales);
            if (SOPC_STATUS_OK == status)
            {
                valWithLTupdate = prevValueCopy;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        // In case of failure, replace the value by the new LT without managing previous locales
        if (SOPC_STATUS_OK != status)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Update of AlarmCondition %s LocalizedText variable %s failed with status %d. It "
                                     "will be replaced by new value only.",
                                     conditionIdStr, varPath, status);
            SOPC_Free(conditionIdStr);
            status = SOPC_STATUS_OK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(pAlarmCondition->data, varPath, valWithLTupdate);
    }
    if (setTsVal && SOPC_STATUS_OK == status)
    {
        status = SOPC_StrConcat(varPath, SourceTimestampPath, &varSourceTsPath);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Event_SetVariableFromStrPath(pAlarmCondition->data, varSourceTsPath, &valTs);
        }
    }
    if (updateNodeVal && SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus nodeStatus =
            SOPC_InternalAlarmConditionNoLock_UpdateVariableNode(pAlarmCondition, varPath, val, sourceTs);
        if (setTsVal && SOPC_STATUS_OK == nodeStatus)
        {
            nodeStatus = SOPC_InternalAlarmConditionNoLock_UpdateVariableNode(pAlarmCondition, varSourceTsPath, &valTs,
                                                                              sourceTs);
            if (SOPC_STATUS_OK != nodeStatus)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Update of AlarmCondition %s variable %s failed with status %d", conditionIdStr,
                                       varSourceTsPath, nodeStatus);
                SOPC_Free(conditionIdStr);
            }
        }
        else if (SOPC_STATUS_OK != nodeStatus)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Update of AlarmCondition %s variable %s failed with status %d", conditionIdStr,
                                   varPath, nodeStatus);
            SOPC_Free(conditionIdStr);
        }
    }
    SOPC_Free(varSourceTsPath);
    SOPC_Variant_Delete(prevValueCopy);
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(SOPC_AlarmCondition* pAlarmCondition,
                                                                           const char* varPath,
                                                                           const SOPC_Variant* val,
                                                                           bool updateNodeVal)
{
    return SOPC_InternalAlarmConditionNoLock_CommonSetVariableFromStrPath(pAlarmCondition, varPath, val, false,
                                                                          updateNodeVal);
}

/* ConditionVariable have a SourceTimestamp */
SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_SetConditionVariableFromStrPath(
    SOPC_AlarmCondition* pAlarmCondition,
    const char* varPath,
    const SOPC_Variant* val,
    bool updateNodeVal)
{
    return SOPC_InternalAlarmConditionNoLock_CommonSetVariableFromStrPath(pAlarmCondition, varPath, val, true,
                                                                          updateNodeVal);
}

// Clear the latest EventId for the given Ack/Conf state variable as it is set to False
// note: Ack/Conf state event notification that can be used for method calls are only ones since latest change to False
static void SOPC_InternalAlarmConditionNoLock_ClearLatestEventId(
    SOPC_AlarmCondition* pAlarmCondition,
    const SOPC_InternalTwoStateVariableFieldsPaths* stateVarPaths,
    bool newState)
{
    if (!newState && stateVarPaths == &cAckedStatePaths)
    {
        SOPC_EventIdList_Clear(pAlarmCondition->ackEventIds);
    }
    else if (!newState && stateVarPaths == &cConfirmedStatePaths)
    {
        SOPC_EventIdList_Clear(pAlarmCondition->confEventIds);
    }
    else if (newState && stateVarPaths == &cActiveStatePaths &&
             SOPC_InternalAlarmCondition_GetAutoAckableOnActive(pAlarmCondition))
    {
        // If AckedState is linked to the Active state, reset AckedState valid EventId list on activation
        SOPC_EventIdList_Clear(pAlarmCondition->ackEventIds);
    }
}

// Call previous but with a lock on the AlarmCondition
static void SOPC_InternalAlarmCondition_ClearLatestEventId(
    SOPC_AlarmCondition* pAlarmCondition,
    const SOPC_InternalTwoStateVariableFieldsPaths* stateVarPaths,
    bool newState)
{
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_InternalAlarmConditionNoLock_ClearLatestEventId(pAlarmCondition, stateVarPaths, newState);
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

/* Manage EventId lists */

static void SOPC_InternalAlarmConditionNoLock_UpdateEventIdLists_Error(const SOPC_AlarmCondition* pAlarmCondition,
                                                                       const SOPC_ByteString* eventId,
                                                                       const char* contextStr,
                                                                       SOPC_ReturnStatus status)
{
    char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
    char* eventIdStr = SOPC_InternalByteStringToString(eventId);
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                           "AlarmCondition %s update latest EventId %s in %s context failed with status %d",
                           conditionIdStr, eventIdStr, contextStr, status);
    SOPC_Free(conditionIdStr);
    SOPC_Free(eventIdStr);
}

static void SOPC_InternalAlarmConditionNoLock_UpdateEventIdLists(SOPC_AlarmCondition* pAlarmCondition,
                                                                 const SOPC_ByteString* eventId)
{
    SOPC_ReturnStatus status = SOPC_EventIdList_Add(pAlarmCondition->ackEventIds, eventId);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_InternalAlarmConditionNoLock_UpdateEventIdLists_Error(pAlarmCondition, eventId, "Acknowledge", status);
    }
    status = SOPC_EventIdList_Add(pAlarmCondition->confEventIds, eventId);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_InternalAlarmConditionNoLock_UpdateEventIdLists_Error(pAlarmCondition, eventId, "Confirm", status);
    }
    status = SOPC_EventIdList_Add(pAlarmCondition->globalEventIds, eventId);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_InternalAlarmConditionNoLock_UpdateEventIdLists_Error(pAlarmCondition, eventId, "Global", status);
    }
}

/* Trigger event */

SOPC_ReturnStatus SOPC_InternalAlarmConditionNoLock_TriggerEvent(SOPC_AlarmCondition* pAlarmCondition, bool force)
{
    SOPC_ASSERT(NULL != pAlarmCondition);
    SOPC_Event* event = SOPC_Event_CreateCopy(pAlarmCondition->data, true);
    SOPC_ReturnStatus status = (NULL == event ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);

    // If not in force mode, check if the condition is enabled
    if (SOPC_STATUS_OK == status && !force)
    {
        const SOPC_Variant* var = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, cEnabledStatePaths.id);
        status = (NULL == var ? SOPC_STATUS_INVALID_STATE : SOPC_STATUS_OK);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == var->ArrayType);
            status = (var->BuiltInTypeId == SOPC_Boolean_Id && var->Value.Boolean ? SOPC_STATUS_OK
                                                                                  : SOPC_STATUS_INVALID_STATE);
        }
    }

    const SOPC_ByteString* eventId = NULL;
    if (SOPC_STATUS_OK == status)
    {
        eventId = SOPC_Event_GetEventId(event);
        if (NULL != eventId)
        {
            const SOPC_Variant newEventId = {.DoNotClear = true,
                                             .ArrayType = SOPC_VariantArrayType_SingleValue,
                                             .BuiltInTypeId = SOPC_ByteString_Id,
                                             .Value.Bstring = *eventId};

            status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(pAlarmCondition, cEventIdPath,
                                                                              &newEventId, true);
            if (SOPC_STATUS_OK != status)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition %s EventId node  value update failed with status %d",
                                       conditionIdStr, status);
                SOPC_Free(conditionIdStr);
            }
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK != status)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AlarmCondition %s new event generation failed with status %d", conditionIdStr,
                                   status);
            SOPC_Free(conditionIdStr);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        // Keep an exact copy which can be provided on ConditionRefresh calls
        SOPC_Event* eventCopy = SOPC_Event_CreateCopy(event, false);
        if (NULL == eventCopy)
        {
            char* eventIdStr = SOPC_InternalGetEventIdString(event);
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "AlarmCondition %s event %s copy allocation for ConditionRefresh failed with status %d", conditionIdStr,
                eventIdStr, status);
            SOPC_Free(eventIdStr);
            SOPC_Free(conditionIdStr);
        }
        // Record new EventId in lists
        // (note: scope of eventId variable stops after TriggerEvent call)
        SOPC_InternalAlarmConditionNoLock_UpdateEventIdLists(pAlarmCondition, eventId);
        // Actually trigger event in Server address space
        bool insertRes = false;
        status = SOPC_ServerHelper_TriggerEvent(&pAlarmCondition->notifierNode, event, 0, 0, 0);
        event = NULL;
        if (NULL != eventCopy)
        {
            // No lock on alarm condition, but lock the global mutex for refresh dict update
            SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            insertRes = SOPC_Dict_Insert(sopc_alarmConditionConfig.g_refreshEventsDict,
                                         (uintptr_t) &pAlarmCondition->conditionNode, (uintptr_t) eventCopy);
            mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        }
        status = (SOPC_STATUS_OK == status && insertRes) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        if (SOPC_STATUS_OK != status)
        {
            char* eventIdStr = SOPC_InternalGetEventIdString(eventCopy);
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AlarmCondition %s event %s copy storage for ConditionRefresh failed with status %d",
                                   conditionIdStr, eventIdStr, status);
            SOPC_Free(eventIdStr);
            SOPC_Free(conditionIdStr);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmConditionWithLock_TriggerEvent(SOPC_AlarmCondition* pAlarmCondition, bool traceLog)
{
    if (NULL == pAlarmCondition)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_ReturnStatus status = SOPC_InternalAlarmConditionNoLock_TriggerEvent(pAlarmCondition, false);
    if (traceLog && (status != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG))
    {
        char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
        if (status != SOPC_STATUS_OK)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AlarmCondition %s event triggering failed with status %d", conditionIdStr, status);
        }
        else
        {
            char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "AlarmCondition %s event triggered with eventId=%s",
                                   conditionIdStr, eventIdStr);
            SOPC_Free(eventIdStr);
        }
        SOPC_Free(conditionIdStr);
    }
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

/* Manage states change  */

static void onStateChangeCbTrigger(SOPC_AlarmCondition_StateChanged_Fct_Ctx* fctCtx,
                                   const SOPC_Variant* prevVal,
                                   const SOPC_Variant* newVal)
{
    SOPC_ASSERT(NULL != fctCtx);
    fctCtx->callback(fctCtx->userCtx, fctCtx->ac, fctCtx->qnPath, prevVal, newVal);
}

static void SOPC_InternalAlarmConditionNoLock_StateGenCallback(const SOPC_AlarmCondition* pAlarmCondition,
                                                               const char* cVarPath,
                                                               const SOPC_Variant* prevVar,
                                                               const SOPC_Variant* newVar)
{
    bool found = false;
    SOPC_AlarmCondition_StateChanged_Fct_Ctx* fctCtx = (SOPC_AlarmCondition_StateChanged_Fct_Ctx*) SOPC_Dict_Get(
        pAlarmCondition->varChangedCbDict, (const uintptr_t) cVarPath, &found);
    if (!found)
    {
        return; // Nothing to do
    }
    SOPC_Variant* prevNewVarCopy = SOPC_Calloc(2, sizeof(*prevNewVarCopy));
    SOPC_ReturnStatus status = ((NULL == prevNewVarCopy) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Variant_Copy(&prevNewVarCopy[0], prevVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Variant_Copy(&prevNewVarCopy[1], newVar);
    }
    if (SOPC_STATUS_OK == status)
    {
        // No need to global mutex lock as looper is thread safe, thus only check g_init for ongoing Clear call
        if (SOPC_InternalAlarmConditionMgr_IsInit())
        {
            SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
            status =
                SOPC_EventHandler_Post(sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr, SOPC_AC_EVT_STATE_CHANGED_CB,
                                       0, (uintptr_t) fctCtx, (uintptr_t) prevNewVarCopy);
            mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
            SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(prevNewVarCopy);
    }
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetBoolState(
    SOPC_AlarmCondition* pAlarmCondition,
    const SOPC_InternalTwoStateVariableFieldsPaths* stateVarPaths,
    bool newState,
    const SOPC_LocalizedText* optComment,
    const SOPC_String* optClientUserId,
    bool updateNodeVal,
    bool triggerEvent,
    bool triggerCb)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return SOPC_STATUS_NOK;
    }
    if (NULL == pAlarmCondition || NULL == stateVarPaths)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    // First check state is not already the new value
    const SOPC_Variant* var = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, stateVarPaths->id);
    SOPC_ReturnStatus status = (NULL == var ? SOPC_STATUS_INVALID_PARAMETERS : SOPC_STATUS_OK);
    // Check if transition is valid (if NULL shall be set, any previous state is valid)
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == var->ArrayType);
        // Either previous state is valid for transition (false => true or true => false)
        status = (SOPC_Boolean_Id == var->BuiltInTypeId && var->Value.Boolean != newState ? SOPC_STATUS_OK
                                                                                          : SOPC_STATUS_INVALID_STATE);
        // Either transition from NULL value
        status = (SOPC_STATUS_OK != status && SOPC_Null_Id == var->BuiltInTypeId ? SOPC_STATUS_OK : status);

        // Either bypass verification for INIT case <=> trigger nothing and do not update nodes
        status = (SOPC_STATUS_OK != status && !updateNodeVal && !triggerEvent && !triggerCb ? SOPC_STATUS_OK : status);
    }
    // Then, set the state Id / text value ∕ TransitionTime
    const SOPC_Variant newVar = {.DoNotClear = true,
                                 .ArrayType = SOPC_VariantArrayType_SingleValue,
                                 .BuiltInTypeId = SOPC_Boolean_Id,
                                 .Value.Boolean = newState};
    const SOPC_Variant newVarNullText = {.DoNotClear = true,
                                         .ArrayType = SOPC_VariantArrayType_SingleValue,
                                         .BuiltInTypeId = SOPC_Null_Id,
                                         .Value.LocalizedText = NULL};

    const SOPC_Variant* newVarText = &newVarNullText;

    SOPC_Variant newVarTransitionTime = {.DoNotClear = true,
                                         .ArrayType = SOPC_VariantArrayType_SingleValue,
                                         .BuiltInTypeId = SOPC_DateTime_Id,
                                         .Value.Date = SOPC_Time_GetCurrentTimeUTC()};

    bool invalidTransition = SOPC_STATUS_OK != status;
    bool resId = false;
    bool resText = false;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(pAlarmCondition, stateVarPaths->id, &newVar,
                                                                          false);
        resId = (SOPC_STATUS_OK == status);
    }
    // Updates the state text value and transition time
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(0 == newState || 1 == newState); // stdbool definition used
        newVarText = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, stateVarPaths->falseTrueStates[newState]);
        status = (NULL == newVarText || SOPC_LocalizedText_Id != newVarText->BuiltInTypeId ||
                          newVarText->ArrayType != SOPC_VariantArrayType_SingleValue
                      ? SOPC_STATUS_WOULD_BLOCK
                      : SOPC_STATUS_OK);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(pAlarmCondition, stateVarPaths->self,
                                                                              newVarText, false);
            resText = (SOPC_STATUS_OK == status);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(
                pAlarmCondition, stateVarPaths->transitionTime, &newVarTransitionTime, false);
        }
        if (SOPC_STATUS_OK != status)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            const char* conditionStrNotNull = conditionIdStr;
            const char* setStateStr = (newState ? "true" : "false");
            if (resText)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "AlarmCondition %s state %s change for %s failed with status %d",
                                         conditionStrNotNull, stateVarPaths->self, setStateStr, status);
            }
            else
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "AlarmCondition %s state %s change to %s failed with status %d",
                                         conditionStrNotNull, stateVarPaths->transitionTime, setStateStr, status);
            }
            SOPC_Free(conditionIdStr);
        }
        status = SOPC_STATUS_OK;
    }
    else if (!resId)
    {
        char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
        const char* setStateStr = (newState ? "true" : "false");
        if (invalidTransition)
        {
            // Only trace at info level as failure is due to current state
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                  "AlarmCondition %s state %s change to %s failed with status %d", conditionIdStr,
                                  stateVarPaths->id, setStateStr, status);
        }
        else
        {
            // Trace at error level as failure is unexpected
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AlarmCondition %s state %s change to %s failed with status %d", conditionIdStr,
                                   stateVarPaths->id, setStateStr, status);
        }
        SOPC_Free(conditionIdStr);
    }
    // Clear the latest EventId for the given Ack/Conf state variables if it is set to False
    // (and for Ack state if Active state is set to True with autoAckOnActiveFlag set)
    if (SOPC_STATUS_OK == status)
    {
        SOPC_InternalAlarmConditionNoLock_ClearLatestEventId(pAlarmCondition, stateVarPaths, newState);
    }
    // Finally update the corresponding nodes consequently
    if (updateNodeVal && SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus writeStatus = SOPC_InternalAlarmConditionNoLock_UpdateTwoStateVariableNodes(
            pAlarmCondition, stateVarPaths, &newVar, newVarText, &newVarTransitionTime);
        if (writeStatus != SOPC_STATUS_OK)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            const char* setStateStr = (newState ? "true" : "false");
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AlarmCondition %s state %s change for %s failed to UPDATE NODES with status %d",
                                   conditionIdStr, stateVarPaths->self, setStateStr, writeStatus);
            SOPC_Free(conditionIdStr);
        }
    }
    // If Comment is provided, update its value and ClientUserId consequently
    if (SOPC_STATUS_OK == status && NULL != optComment)
    {
        // Note: variant is then copied and thus LT never modified
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        const SOPC_Variant commentVar = {.DoNotClear = true,
                                         .ArrayType = SOPC_VariantArrayType_SingleValue,
                                         .BuiltInTypeId = SOPC_LocalizedText_Id,
                                         .Value.LocalizedText = (SOPC_LocalizedText*) optComment};
        SOPC_GCC_DIAGNOSTIC_RESTORE
        SOPC_Variant userIdVar = {.DoNotClear = true,
                                  .ArrayType = SOPC_VariantArrayType_SingleValue,
                                  .BuiltInTypeId = SOPC_String_Id,
                                  .Value.String = {0}};
        SOPC_String_Initialize(&userIdVar.Value.String);
        SOPC_ReturnStatus userIdStatus = SOPC_STATUS_OK;
        if (NULL != optClientUserId)
        {
            userIdStatus = SOPC_String_Copy(&userIdVar.Value.String, optClientUserId);
        }
        if (SOPC_STATUS_OK == userIdStatus)
        {
            userIdStatus = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(pAlarmCondition, cClientUserIdPath,
                                                                                    &userIdVar, true);
        }
        if (userIdStatus != SOPC_STATUS_OK)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER, "AlarmCondition %s ClientUserId change to %s failed with status %d",
                conditionIdStr,
                (userIdVar.Value.String.Length <= 0 ? "<NULL>" : SOPC_String_GetRawCString(&userIdVar.Value.String)),
                userIdStatus);
            SOPC_Free(conditionIdStr);
        }
        SOPC_String_Clear(&userIdVar.Value.String);

        SOPC_ReturnStatus commentStatus = SOPC_InternalAlarmConditionNoLock_SetConditionVariableFromStrPath(
            pAlarmCondition, cCommentIdPath, &commentVar, true);
        if (commentStatus != SOPC_STATUS_OK)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER, "AlarmCondition %s Comment change to %s failed with status %d",
                conditionIdStr,
                (optComment->defaultText.Length <= 0 ? "<NULL>" : SOPC_String_GetRawCString(&optComment->defaultText)),
                commentStatus);
            SOPC_Free(conditionIdStr);
        }
    }
    // Create event copy to be triggered and set the new EventId variable
    if (triggerEvent && SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus eventStatus = SOPC_InternalAlarmConditionNoLock_TriggerEvent(pAlarmCondition, false);
        if (eventStatus != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            const char* setStateStr = (newState ? "true" : "false");

            if (eventStatus != SOPC_STATUS_OK && eventStatus != SOPC_STATUS_INVALID_STATE)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggering for %s state change to %s failed with status %d",
                    conditionIdStr, stateVarPaths->self, setStateStr, eventStatus);
            }
            else if (eventStatus == SOPC_STATUS_INVALID_STATE)
            {
                SOPC_Logger_TraceDebug(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event not triggered for %s state change to %s because it is disabled",
                    conditionIdStr, stateVarPaths->self, setStateStr);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition %s event triggered for %s state change to %s with eventId=%s",
                                       conditionIdStr, stateVarPaths->self, setStateStr, eventIdStr);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(conditionIdStr);
        }
    }
    // Triggers the application callback
    if (triggerCb && SOPC_STATUS_OK == status)
    {
        SOPC_InternalAlarmConditionNoLock_StateGenCallback(pAlarmCondition, stateVarPaths->self, var, &newVar);
    }
    mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

static void SOPC_InternalAlarmCondition_SetRetainState(SOPC_AlarmCondition* pAlarmCondition,
                                                       bool retain,
                                                       bool checkActive,
                                                       bool checkAckConf)
{
    SOPC_ASSERT(checkActive || checkAckConf); // At least one check shall be done
    const SOPC_Variant retainVar = {.DoNotClear = true,
                                    .ArrayType = SOPC_VariantArrayType_SingleValue,
                                    .BuiltInTypeId = SOPC_Boolean_Id,
                                    .Value.Boolean = retain};

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    {
        // Determine if Retain value shall be set depending on Active/Acked/Confirmed states and check requested
        bool setRetain = !checkActive; // If no Active check, set Retain value by default
        bool active = false;           // Actual active state init
        if (checkActive)
        {
            const SOPC_Variant* activeVar =
                SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, cActiveStatePaths.id);
            if (NULL != activeVar && SOPC_Boolean_Id == activeVar->BuiltInTypeId)
            {
                SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == activeVar->ArrayType);
                // If an Active state exists, retrieve its value otherwise consider consistent to Retain value
                active = (activeVar->Value.Boolean);
                // Active state and Retain value shall be the same
                setRetain = (retain == active);
            }
            else
            {
                checkActive = false; // State not available, ignore it
                setRetain = true;
            }
        }
        if (setRetain && checkAckConf)
        {
            // Retrieve if Acked state is FALSE (alarm is acknowledgeable)
            const SOPC_Variant* varAck = SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, cAckedStatePaths.id);
            // Acked state exists and value is FALSE
            bool ackable = (NULL != varAck && SOPC_Boolean_Id == varAck->BuiltInTypeId && !varAck->Value.Boolean);
            // Acked state does not exist or value is TRUE
            bool acked = (NULL == varAck || SOPC_Boolean_Id != varAck->BuiltInTypeId || varAck->Value.Boolean);
            // Retrieve if Conf state is FALSE (alarm is confirmable)
            const SOPC_Variant* varConf =
                SOPC_Event_GetVariableFromStrPath(pAlarmCondition->data, cConfirmedStatePaths.id);
            // Confirmed state exists and value is FALSE
            bool confirmable =
                (NULL != varConf && SOPC_Boolean_Id == varConf->BuiltInTypeId && !varConf->Value.Boolean);
            // Confirmed state does not exist or value is TRUE
            bool confirmed = (NULL == varConf || SOPC_Boolean_Id != varConf->BuiltInTypeId || varConf->Value.Boolean);
            if (retain)
            {
                // If Retain is set to TRUE, alarm shall be active or acknowledgeable or confirmable
                setRetain = ((checkActive && active) || ackable || confirmable);
            }
            else
            {
                // If Retain is set to FALSE, alarm shall be acknowledged and confirmed
                // Note: !checkActive => !active
                setRetain = (!active && acked && confirmed);
            }
        }
        if (setRetain)
        {
            SOPC_ReturnStatus status = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(
                pAlarmCondition, cRetainPath, &retainVar, true);
            if (status != SOPC_STATUS_OK)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition %s Retain change to %s failed with status %d", conditionIdStr,
                                       (retain ? "true" : "false"), status);
                SOPC_Free(conditionIdStr);
            }
        } // else: do not change Retain value as Active state does not match
        mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetEnabledState(SOPC_AlarmCondition* pAlarmCondition,
                                                              bool enabled,
                                                              bool setRetain,
                                                              const SOPC_LocalizedText* optComment,
                                                              bool triggerCb)
{
    // Note: always accept to set from NULL value as it should never be NULL (init should ensure that)
    // Note2: do not trigger event in callee since we need to update Retain value before
    SOPC_ReturnStatus status = SOPC_InternalAlarmCondition_SetBoolState(pAlarmCondition, &cEnabledStatePaths, enabled,
                                                                        optComment, NULL, true, false, triggerCb);
    if (SOPC_STATUS_OK == status)
    {
        const SOPC_Variant retainVar = {.DoNotClear = true,
                                        .ArrayType = SOPC_VariantArrayType_SingleValue,
                                        .BuiltInTypeId = SOPC_Boolean_Id,
                                        .Value.Boolean = enabled};

        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pAlarmCondition->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        // From part 9 §5.5.2 (v1.05.02):
        // When the Condition instance enters the Disabled state, the Retain Property of this
        // Condition shall be set to False by the Server
        if (!enabled || setRetain)
        {
            SOPC_ReturnStatus retStatus = SOPC_InternalAlarmConditionNoLock_SetVariableFromStrPath(
                pAlarmCondition, cRetainPath, &retainVar, true);
            if (retStatus != SOPC_STATUS_OK)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggering for %s state change to %s failed with status %d",
                    conditionIdStr, cRetainPath, (enabled ? "true" : "false"), retStatus);
                SOPC_Free(conditionIdStr);
            }
        }
        else
        {
            // If automatically managed, set Retain state accordingly (if consistent depending on Ack/Conf states)
            if (SOPC_InternalAlarmCondition_GetAutoRetain(pAlarmCondition))
            {
                /* Set Retain value to TRUE
                   (check in function that Active/Ack/Conf state is consistent for setting it) */
                SOPC_InternalAlarmCondition_SetRetainState(pAlarmCondition, true, true, true);
            }
        }
        SOPC_ReturnStatus eventStatus = SOPC_InternalAlarmConditionNoLock_TriggerEvent(pAlarmCondition, true);
        if (eventStatus != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);

            if (eventStatus != SOPC_STATUS_OK)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggering for %s state change to %s failed with status %d",
                    conditionIdStr, cEnabledStatePaths.self, (enabled ? "true" : "false"), eventStatus);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition %s event triggered for %s state change to %s with eventId=%s",
                                       conditionIdStr, cEnabledStatePaths.self, (enabled ? "true" : "false"),
                                       eventIdStr);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(conditionIdStr);
        }
        mutStatus = SOPC_Mutex_Unlock(&pAlarmCondition->mut);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_Confirm(SOPC_AlarmCondition* pAlarmCondition,
                                                      const SOPC_LocalizedText* optComment,
                                                      const SOPC_String* optClientUserId,
                                                      bool triggerCb)
{
    if (SOPC_InternalAlarmCondition_GetAutoRetain(pAlarmCondition))
    {
        /* Set Retain to FALSE upon Confirmation
           (check in function that Active state is FALSE, do not check Conf state that is not set yet) */
        SOPC_InternalAlarmCondition_SetRetainState(pAlarmCondition, false, true, false);
    }

    return SOPC_InternalAlarmCondition_SetBoolState(pAlarmCondition, &cConfirmedStatePaths, true, optComment,
                                                    optClientUserId, true, true, triggerCb);
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetConfirmable(SOPC_AlarmCondition* pAlarmCondition,
                                                             bool confirmable,
                                                             const SOPC_LocalizedText* optComment)
{
    // Set the Confirm state to !confirmable value
    // (might fail with SOPC_STATUS_INVALID_STATE if already set to the same value)
    SOPC_ReturnStatus status = SOPC_InternalAlarmCondition_SetBoolState(
        pAlarmCondition, &cConfirmedStatePaths, !confirmable, optComment, NULL, true, false, false);

    // Specific treatment if already in expected state
    if (SOPC_STATUS_INVALID_STATE == status)
    {
        // Acked state is already in expected value for acknowledgeable property which is OK
        status = SOPC_STATUS_OK;
        // In case we reset the confirmable state to TRUE, we need to reset the EventId list
        if (confirmable)
        {
            // note: newState param here is ConfirmedState == false
            SOPC_InternalAlarmCondition_ClearLatestEventId(pAlarmCondition, &cConfirmedStatePaths, false);
        }
    }

    // Triggers an event in case we set confirmable state
    if (confirmable && SOPC_STATUS_OK == status)
    {
        status = SOPC_InternalAlarmConditionWithLock_TriggerEvent(pAlarmCondition, false);

        if (status != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            if (status != SOPC_STATUS_OK)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggering for ConfirmedState change to false failed with status %d",
                    conditionIdStr, status);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                SOPC_Logger_TraceDebug(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggered for ConfirmedState change to false with eventId=%s",
                    conditionIdStr, eventIdStr);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(conditionIdStr);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_Acknowledge(SOPC_AlarmCondition* pAlarmCondition,
                                                          const SOPC_LocalizedText* optComment,
                                                          const SOPC_String* optClientUserId,
                                                          bool triggerCb)
{
    // Set the Acked state to given value (might fail with SOPC_STATUS_INVALID_STATE if already set to the same
    // value)
    SOPC_ReturnStatus status = SOPC_InternalAlarmCondition_SetBoolState(
        pAlarmCondition, &cAckedStatePaths, true, optComment, optClientUserId, true, false, triggerCb);

    if (SOPC_STATUS_OK == status)
    {
        // If automatically managed, set "confirmable" state accordingly (triggers an event)
        if (SOPC_InternalAlarmCondition_GetAutoConfOnAcked(pAlarmCondition))
        {
            // Set state to Confirmed = FALSE
            // note: in this case we do not trigger the callback as behavior is requested by application
            SOPC_ReturnStatus confStatus = SOPC_InternalAlarmCondition_SetConfirmable(pAlarmCondition, true, NULL);
            SOPC_ASSERT(confStatus == SOPC_STATUS_OK);
        }
        else // else: triggers an event for acknowledgement
        {
            if (SOPC_InternalAlarmCondition_GetAutoRetain(pAlarmCondition))
            {
                /* Set Retain to FALSE upon Acknowledgement if AutoConfOnAcked flag is not set
                   (check in function that Active state and Ack/Conf states are consistent) */
                SOPC_InternalAlarmCondition_SetRetainState(pAlarmCondition, false, true, true);
            }

            status = SOPC_InternalAlarmConditionWithLock_TriggerEvent(pAlarmCondition, false);

            if (status != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
                if (status != SOPC_STATUS_OK)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "AlarmCondition %s event triggering for Acknowledged failed with status %d",
                                           conditionIdStr, status);
                }
                else
                {
                    char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "AlarmCondition %s event triggered for Acknowledged with eventId=%s",
                                           conditionIdStr, eventIdStr);
                    SOPC_Free(eventIdStr);
                }
                SOPC_Free(conditionIdStr);
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetAcknowledgeable(SOPC_AlarmCondition* pAlarmCondition,
                                                                 bool acknowledgeable,
                                                                 const SOPC_LocalizedText* optComment)
{
    // Set the Acked state to !acknowledgeable value
    // (might fail with SOPC_STATUS_INVALID_STATE if already set to the same value)
    SOPC_ReturnStatus status = SOPC_InternalAlarmCondition_SetBoolState(
        pAlarmCondition, &cAckedStatePaths, !acknowledgeable, optComment, NULL, true, false, false);

    // Specific treatment if already in expected state
    if (SOPC_STATUS_INVALID_STATE == status)
    {
        // Acked state is already in expected value for acknowledgeable property which is OK
        status = SOPC_STATUS_OK;
        // In case we reset the acknowledgeable state to TRUE, we need to reset the EventId list
        if (acknowledgeable)
        {
            // note: newState param here is AckedState == false
            SOPC_InternalAlarmCondition_ClearLatestEventId(pAlarmCondition, &cAckedStatePaths, false);
        }
    }

    // If automatically managed, set "confirmable" state accordingly
    if (SOPC_STATUS_OK == status && SOPC_InternalAlarmCondition_GetAutoConfOnAcked(pAlarmCondition))
    {
        // In both cases set "unconfirmable" state (which do not trigger an event)
        SOPC_ReturnStatus confStatus = SOPC_InternalAlarmCondition_SetConfirmable(pAlarmCondition, false, NULL);
        SOPC_UNUSED_RESULT(confStatus); // ignore result as only failure possible is expected state
    }
    // Triggers an event in case we set acknowledgeable state
    if (acknowledgeable && SOPC_STATUS_OK == status)
    {
        status = SOPC_InternalAlarmConditionWithLock_TriggerEvent(pAlarmCondition, false);

        if (status != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            if (status != SOPC_STATUS_OK)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggering for AckedState change to false failed with status %d",
                    conditionIdStr, status);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                SOPC_Logger_TraceDebug(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "AlarmCondition %s event triggered for AckedState change to false with eventId=%s", conditionIdStr,
                    eventIdStr);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(conditionIdStr);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetActiveState(SOPC_AlarmCondition* pAlarmCondition,
                                                             bool active,
                                                             const SOPC_LocalizedText* optComment,
                                                             const SOPC_String* optClientUserId,
                                                             bool triggerEvent)
{
    bool autoAckedOnActive = SOPC_InternalAlarmCondition_GetAutoAckableOnActive(pAlarmCondition);
    // Set the Active state to given value (might fail with SOPC_STATUS_INVALID_STATE if already set to the same value)
    SOPC_ReturnStatus status = SOPC_InternalAlarmCondition_SetBoolState(
        pAlarmCondition, &cActiveStatePaths, active, optComment, optClientUserId, true, false, false);

    if (SOPC_STATUS_OK == status)
    {
        // If automatically managed, set Retain state accordingly (if consistent depending on Ack/Conf states)
        if (SOPC_InternalAlarmCondition_GetAutoRetain(pAlarmCondition))
        {
            /* Set Retain to Active value (check in function that Ack/Conf state is consistent for setting it) */
            SOPC_InternalAlarmCondition_SetRetainState(pAlarmCondition, active, false, true);
        }

        // If automatically managed, set "acknowledgeable" state on alarm activation (triggers an event)
        if (active && autoAckedOnActive)
        {
            SOPC_ASSERT(triggerEvent); // when active is set to true, triggerEvent is always true
            // note: in this case we do not trigger the callback as behavior is requested by application
            SOPC_ReturnStatus ackedStatus =
                SOPC_InternalAlarmCondition_SetAcknowledgeable(pAlarmCondition, true, optComment);
            SOPC_UNUSED_ARG(ackedStatus);
        }
        else if (triggerEvent) // else: triggers an event for activation state change if needed
        {
            status = SOPC_InternalAlarmConditionWithLock_TriggerEvent(pAlarmCondition, false);

            if (status != SOPC_STATUS_OK || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
                if (status != SOPC_STATUS_OK)
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "AlarmCondition %s event triggering for Active state to %s failed with status %d",
                        conditionIdStr, active ? "true" : "false", status);
                }
                else
                {
                    char* eventIdStr = SOPC_InternalGetEventIdString(pAlarmCondition->data);
                    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "AlarmCondition %s event triggered for Active state to %s with eventId=%s",
                                           conditionIdStr, active ? "true" : "false", eventIdStr);
                    SOPC_Free(eventIdStr);
                }
                SOPC_Free(conditionIdStr);
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_InternalAlarmCondition_SetComment(SOPC_AlarmCondition* pAlarmCondition,
                                                         const SOPC_LocalizedText* comment,
                                                         const SOPC_String* userId)
{
    if (NULL == comment)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    // Note: variant is then copied and thus LT never modified
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    const SOPC_Variant commentVar = {.DoNotClear = true,
                                     .ArrayType = SOPC_VariantArrayType_SingleValue,
                                     .BuiltInTypeId = SOPC_LocalizedText_Id,
                                     .Value.LocalizedText = (SOPC_LocalizedText*) comment};
    SOPC_GCC_DIAGNOSTIC_RESTORE
    SOPC_Variant userIdVar = {.DoNotClear = true,
                              .ArrayType = SOPC_VariantArrayType_SingleValue,
                              .BuiltInTypeId = SOPC_String_Id,
                              .Value.String = {0}};
    SOPC_String_Initialize(&userIdVar.Value.String);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != userId)
    {
        status = SOPC_String_Copy(&userIdVar.Value.String, userId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_AlarmCondition_SetVariableFromStrPath(pAlarmCondition, cClientUserIdPath, &userIdVar);
        if (SOPC_STATUS_OK != status)
        {
            char* conditionIdStr = SOPC_InternalConditionIdToString(pAlarmCondition);
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER, "AlarmCondition %s ClientUserId change to %s failed with status %d",
                conditionIdStr,
                (userIdVar.Value.String.Length <= 0 ? "<NULL>" : SOPC_String_GetRawCString(&userIdVar.Value.String)),
                status);
            SOPC_Free(conditionIdStr);
        }
        SOPC_String_Clear(&userIdVar.Value.String);
    }

    return SOPC_AlarmCondition_SetConditionVariableFromStrPath(pAlarmCondition, cCommentIdPath, &commentVar, true);
}

bool SOPC_InternalAlarmCondition_GetBoolValueFromStrPath(SOPC_AlarmCondition* ac, const char* strPath)
{
    if (!SOPC_InternalAlarmConditionMgr_IsInit())
    {
        return false;
    }
    if (NULL == ac || NULL == strPath)
    {
        return false;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const SOPC_Variant* boolVar = SOPC_Event_GetVariableFromStrPath(ac->data, strPath);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == boolVar->ArrayType);
    bool res = (SOPC_Boolean_Id == boolVar->BuiltInTypeId && boolVar->Value.Boolean);
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return res;
}

char* SOPC_InternalConditionIdToString(const SOPC_AlarmCondition* ac)
{
    if (NULL == ac)
    {
        return NULL;
    }

    // Get the condition node ID from the alarm condition
    const SOPC_NodeId* conditionId = &ac->conditionNode;
    // Convert NodeId to string using SOPC_NodeId_ToString
    char* result = SOPC_NodeId_ToCString(conditionId);
    if (NULL == result)
    {
        // Allocate and copy "<NULL>" string
        result = SOPC_Calloc(strlen("<NULL>") + 1, sizeof(char));
        if (NULL != result)
        {
            strcpy(result, "<NULL>");
        }
    }

    return result;
}
