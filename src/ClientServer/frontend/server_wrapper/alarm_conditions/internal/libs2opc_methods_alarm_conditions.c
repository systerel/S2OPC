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
 * \brief Method call implementation for A&C
 */

#include "libs2opc_internal_alarm_conditions.h"

#include "libs2opc_server.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_crypto_decl.h"
#include "sopc_helper_string.h"
#include "sopc_key_manager_lib_itf.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

typedef struct
{
    SOPC_StatusCode resultStatus;
    SOPC_ByteString* failedEventId;
    SOPC_LocalizedText* optComment; /* Optional comment */
    SOPC_String* optClientUserId;   /* Optional ClientUserId: provided if comment is set and user is not anonymous */

} SOPC_MethodCallEvt_Ctx;

/* Managing ConditionRefresh/2 */
typedef struct
{
    SOPC_SessionId sessionId;
    uint32_t subId;
    uint32_t miId;
} SOPC_InternalConditionRefresh_Ctx;

/* OPC UA types node ids used for method calls*/

static const SOPC_NodeId RefreshStartType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_RefreshStartEventType);
static const SOPC_NodeId RefreshEndType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_RefreshEndEventType);

static const SOPC_NodeId ConditionRefreshMethod_NodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType_ConditionRefresh);
static const SOPC_NodeId ConditionRefresh2Method_NodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType_ConditionRefresh2);
static const SOPC_NodeId EnableMethodType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType_Enable);
static const SOPC_NodeId DisableMethodType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType_Disable);
static const SOPC_NodeId AddCommentMethodType_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType_AddComment);
static const SOPC_NodeId AckMethodType_NodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_AcknowledgeableConditionType_Acknowledge);
static const SOPC_NodeId ConfirmMethodType_NodeId =
    SOPC_NODEID_NS0_NUMERIC(OpcUaId_AcknowledgeableConditionType_Confirm);

/* Global variables used for method calls */

static int32_t g_atomic_refreshOnGoing = false; // Atomic accessed value for ConditionRefresh ongoing status

// TODO: factorize with ClientUserId computation from Audit (getClientUserId) ?
static SOPC_String* SOPC_InternalMethodCall_GetClientUserId(const SOPC_CallContext* callContext)
{
    SOPC_ASSERT(NULL != callContext);
    /*********************************************************************************/
    /**
     * Extract the X509 subject name or user Id. The returned value shall be freed.
     *  Return NULL otherwise. */
    SOPC_String* result = SOPC_String_Create();
    if (NULL == result)
    {
        return NULL;
    }
    const SOPC_User* user = SOPC_CallContext_GetUser(callContext);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (SOPC_User_IsAnonymous(user))
    {
        status = SOPC_STATUS_WOULD_BLOCK; // part 5: if an AnonymousIdentityToken was used, the value is null.
    }
    else if (SOPC_User_IsUsername(user))
    {
        status = SOPC_String_Copy(result, SOPC_User_GetUsername(user));
    }
    else if (SOPC_User_IsCertificate(user))
    {
        const SOPC_ByteString* bStringCert = SOPC_User_GetCertificate(user);
        if (bStringCert->Length > 0)
        {
            SOPC_CertificateList* pCrtUser = NULL;
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(bStringCert->Data, (uint32_t) bStringCert->Length,
                                                                    &pCrtUser);
            if (SOPC_STATUS_OK == status)
            {
                uint32_t len;
                char* subject = NULL;
                status = SOPC_KeyManager_Certificate_GetSubjectName(pCrtUser, &subject, &len);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_String_CopyFromCString(result, subject);
                }
                SOPC_Free(subject);
            }
            SOPC_KeyManager_Certificate_Free(pCrtUser);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_String_Delete(result);
        result = NULL;
    }
    return result;
}

static SOPC_AlarmCondition* SOPC_InternalMethodCall_RetrieveAC(void* param, const SOPC_NodeId* conditionNodeId)
{
    SOPC_AlarmCondition* ac = (SOPC_AlarmCondition*) param;
    uintptr_t foundAC = 0;
    bool found = false;
    if (NULL == ac && SOPC_InternalAlarmConditionMgr_IsInit())
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        if (NULL != sopc_alarmConditionConfig.g_alarmConditionsDict)
        {
            foundAC =
                SOPC_Dict_Get(sopc_alarmConditionConfig.g_alarmConditionsDict, (uintptr_t) conditionNodeId, &found);
        }
        mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        if (found)
        {
            ac = (SOPC_AlarmCondition*) foundAC;
        }
    }
    return ac;
}

static void SOPC_InternalConditionRefresh_ComputeNotifiers_ForEach_Fct(const uintptr_t key,
                                                                       uintptr_t value,
                                                                       uintptr_t user_data)
{
    SOPC_UNUSED_ARG(key);
    SOPC_Dict* notifiers = (SOPC_Dict*) user_data;
    SOPC_AlarmCondition* ac = (SOPC_AlarmCondition*) value;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    bool res = SOPC_Dict_Insert(notifiers, (uintptr_t) &ac->notifierNode, true);
    SOPC_UNUSED_RESULT(res);
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

typedef struct
{
    const SOPC_InternalConditionRefresh_Ctx* refreshCtx;
    const SOPC_Event* refreshStartEnd;
    bool isStart;
} SOPC_StartEndNotifier_Ctx;

static void SOPC_InternalConditionRefresh_StartEndNotifiers_ForEach_Fct(const uintptr_t key,
                                                                        uintptr_t value,
                                                                        uintptr_t user_data)
{
    SOPC_UNUSED_ARG(value);
    const SOPC_NodeId* notifierId = (SOPC_NodeId*) key;
    SOPC_StartEndNotifier_Ctx* ctx = (SOPC_StartEndNotifier_Ctx*) user_data;
    // From part 9 §5.5.7 v1.05:
    // A copy of the instance of RefreshStartEventType is queued into the
    // Event stream for every Notifier MonitoredItem in the Subscription.
    // Each of the Event copies shall contain the same EventId.
    SOPC_Event* eventCopy = SOPC_Event_CreateCopy(ctx->refreshStartEnd, false);
    if (NULL != eventCopy)
    {
        SOPC_ReturnStatus status = SOPC_ServerHelper_TriggerEvent(notifierId, eventCopy, ctx->refreshCtx->sessionId,
                                                                  ctx->refreshCtx->subId, ctx->refreshCtx->miId);
        if (SOPC_STATUS_OK != status || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
        {
            const char* startEndStr = (ctx->isStart ? "Start" : "End");
            char* notifierIdStr = SOPC_NodeId_ToCString(notifierId);
            const char* notifierIdNoNull = (NULL != notifierIdStr ? notifierIdStr : "<NULL>");
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Error triggering Refresh%s event for notifier %s with status %d", startEndStr,
                                       notifierIdNoNull, status);
            }
            else
            {
                char* eventIdStr = SOPC_InternalGetEventIdString(ctx->refreshStartEnd);
                const char* eventIdNoNull = (NULL != eventIdStr ? eventIdStr : "<NULL>");
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition Refresh%s event triggered for notifier %s with eventId=%s",
                                       startEndStr, notifierIdNoNull, eventIdNoNull);
                SOPC_Free(eventIdStr);
            }
            SOPC_Free(notifierIdStr);
        }
    }
}

static void SOPC_InternalConditionRefresh_Replay_ForEach_Fct(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    SOPC_InternalConditionRefresh_Ctx* refreshCtx = (SOPC_InternalConditionRefresh_Ctx*) user_data;
    const SOPC_NodeId* condId = (SOPC_NodeId*) key;
    SOPC_AlarmCondition* ac = (SOPC_AlarmCondition*) value;
    SOPC_ASSERT(NULL != ac);
    SOPC_Event* eventCopy = NULL;
    SOPC_Event* event = NULL;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const SOPC_Variant* retainVar = SOPC_Event_GetVariableFromStrPath(ac->data, cRetainPath);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == retainVar->ArrayType);
    if (SOPC_Boolean_Id == retainVar->BuiltInTypeId && retainVar->Value.Boolean)
    {
        bool found = false;
        // Global mutex already locked by caller

        event = (SOPC_Event*) SOPC_Dict_Get(sopc_alarmConditionConfig.g_refreshEventsDict, (uintptr_t) condId, &found);
        if (found)
        {
            eventCopy = SOPC_Event_CreateCopy(event, false);
        }
    }
    if (NULL != eventCopy)
    {
        SOPC_ReturnStatus status = SOPC_ServerHelper_TriggerEvent(&ac->notifierNode, eventCopy, refreshCtx->sessionId,
                                                                  refreshCtx->subId, refreshCtx->miId);
        if (SOPC_STATUS_OK != status || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
        {
            char* notifierIdStr = SOPC_NodeId_ToCString(&ac->notifierNode);
            const char* notifierIdNoNull = (NULL != notifierIdStr ? notifierIdStr : "<NULL>");
            char* eventIdStr = SOPC_InternalGetEventIdString(event);
            const char* eventIdNoNull = (NULL != eventIdStr ? eventIdStr : "<NULL>");
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Error triggering Refresh of EventId %s with status %d", eventIdNoNull, status);
            }
            else
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "AlarmCondition Refreshing event for notifier %s with eventId=%s",
                                       notifierIdNoNull, eventIdNoNull);
            }
            SOPC_Free(notifierIdStr);
            SOPC_Free(eventIdStr);
        }
    }
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

static void SOPC_Internal_ConditionRefresh(SOPC_InternalConditionRefresh_Ctx* refreshCtx, SOPC_StatusCode opcStatus)
{
    SOPC_Event* refreshStart = NULL;
    SOPC_Event* refreshEnd = NULL;
    if (opcStatus != SOPC_GoodGenericStatus)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "Failed call to ConditionRefresh method for SessionId=%" PRIu32 " SubId=%" PRIu32
                              " MIid=%" PRIu32 " with status 0x%08" PRIX32,
                              refreshCtx->sessionId, refreshCtx->subId, refreshCtx->miId, opcStatus);
        SOPC_Free(refreshCtx);
        return;
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Method call ConditionRefresh for SessionId=%" PRIu32 " SubId=%" PRIu32 " MIid=%" PRIu32
                               " on-going",
                               refreshCtx->sessionId, refreshCtx->subId, refreshCtx->miId);
    }

    SOPC_Dict* notifiers = SOPC_NodeId_Dict_Create(false, NULL);
    if (NULL == notifiers)
    {
        return;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Dict_ForEach(sopc_alarmConditionConfig.g_alarmConditionsDict,
                      SOPC_InternalConditionRefresh_ComputeNotifiers_ForEach_Fct, (uintptr_t) notifiers);
    mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Note: types of input arguments already checked by services layer but conversion needed since it is an IntegerId
    SOPC_ReturnStatus status = SOPC_ServerHelper_CreateEvent(&RefreshStartType_NodeId, &refreshStart);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_CreateEvent(&RefreshEndType_NodeId, &refreshEnd);
    }
    SOPC_StartEndNotifier_Ctx startEndNotifierCtx = {
        .refreshCtx = refreshCtx, .refreshStartEnd = refreshStart, .isStart = true};
    // Trigger start event
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Dict_ForEach(notifiers, SOPC_InternalConditionRefresh_StartEndNotifiers_ForEach_Fct,
                          (uintptr_t) &startEndNotifierCtx);
    }
    if (SOPC_STATUS_OK == status && SOPC_InternalAlarmConditionMgr_IsInit())
    {
        // Trigger start event
        mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        SOPC_Dict_ForEach(sopc_alarmConditionConfig.g_alarmConditionsDict,
                          SOPC_InternalConditionRefresh_Replay_ForEach_Fct, (uintptr_t) refreshCtx);
        mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    // Trigger end event
    if (SOPC_STATUS_OK == status)
    {
        startEndNotifierCtx.refreshStartEnd = refreshEnd;
        startEndNotifierCtx.isStart = false;
        SOPC_Dict_ForEach(notifiers, SOPC_InternalConditionRefresh_StartEndNotifiers_ForEach_Fct,
                          (uintptr_t) &startEndNotifierCtx);
    }
    // Refresh is not ongoing anymore
    SOPC_Atomic_Int_Set(&g_atomic_refreshOnGoing, false);
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                           "Method call ConditionRefresh for SessionId=%" PRIu32 " SubId=%" PRIu32 " MIid=%" PRIu32
                           " terminated",
                           refreshCtx->sessionId, refreshCtx->subId, refreshCtx->miId);

    SOPC_Event_Delete(&refreshStart);
    SOPC_Event_Delete(&refreshEnd);
    SOPC_Dict_Delete(notifiers);
    SOPC_Free(refreshCtx);
}

static SOPC_StatusCode SOPC_MethodCall_ConditionRefresh(const SOPC_CallContext* callContextPtr,
                                                        const SOPC_NodeId* objectId,
                                                        uint32_t nbInputArgs,
                                                        const SOPC_Variant* inputArgs,
                                                        uint32_t* nbOutputArgs,
                                                        SOPC_Variant** outputArgs,
                                                        void* param)
{
    // Note: It has already been checked that the objectId has the current method in services layer
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(param);

    SOPC_InternalConditionRefresh_Ctx* refreshCtx = SOPC_Calloc(1, sizeof(*refreshCtx));
    if (NULL == refreshCtx)
    {
        return OpcUa_BadOutOfMemory;
    }
    refreshCtx->sessionId = SOPC_CallContext_GetSessionId(callContextPtr);
    // Note: types of input arguments already checked by services layer but conversion needed since it is an IntegerId
    refreshCtx->subId = inputArgs[0].Value.Uint32;
    if (nbInputArgs > 1) // ConditionRefresh2 management
    {
        refreshCtx->miId = inputArgs[1].Value.Uint32;
    }
    SOPC_StatusCode opcStatus = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (SOPC_InternalAlarmConditionMgr_IsInit())
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        if (true == SOPC_Atomic_Int_Get(&g_atomic_refreshOnGoing))
        {
            opcStatus = OpcUa_BadRefreshInProgress;
        }
        else if (0 == SOPC_Dict_Size(sopc_alarmConditionConfig.g_refreshEventsDict))
        {
            // Note: "The ConditionRefresh Method was called on a SubscriptionId that has no event MonitoredItems."
            //       cannot be precisely verified as it requires introspection in services core.
            //       Only check if A&C instances events exist.
            opcStatus = OpcUa_BadNothingToDo;
        }
        else if (0 == refreshCtx->subId)
        {
            // Note: "The Server shall verify that the SubscriptionId provided is part of the Session"
            //       cannot be precisely verified as it requires introspection in services core.
            //       Only check if subscriptionId is not 0.
            // Note2: "The Method was not called in the context of the Session that owns the Subscription"
            //        cannot be verified at as it requires introspection in services core.
            //        Bad_UserAccessDenied is never returned.
            opcStatus = OpcUa_BadSubscriptionIdInvalid;
        }
        status = SOPC_EventHandler_Post(sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr,
                                        SOPC_AC_EVT_METHOD_CALL_COND_REFRESH, 0, (uintptr_t) refreshCtx,
                                        (uintptr_t) opcStatus);
        mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(refreshCtx);
    }
    else if (SOPC_GoodGenericStatus == opcStatus)
    {
        SOPC_Atomic_Int_Set(&g_atomic_refreshOnGoing, true);
    }
    return opcStatus;
}

/* Managing method calls for A&C States */

static bool SOPC_InternalMethodCall_SetStateToLooper(SOPC_ServerAlarmCondMethods acMethod,
                                                     SOPC_AlarmCondition* ac,
                                                     SOPC_LocalizedText* optComment,
                                                     const SOPC_CallContext* callCtx)
{
    SOPC_MethodCallEvt_Ctx* cmEvtCtx = NULL;
    if (NULL != optComment)
    {
        cmEvtCtx = SOPC_Calloc(1, sizeof(*cmEvtCtx));
        if (NULL != cmEvtCtx)
        {
            // Note: 0 is the Good value for resultStatus
            cmEvtCtx->optComment = optComment;
            cmEvtCtx->optClientUserId = SOPC_InternalMethodCall_GetClientUserId(callCtx);
        } // else: cmEvtCtx is NULL, no comment will be set
    }
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (SOPC_InternalAlarmConditionMgr_IsInit())
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        status = SOPC_EventHandler_Post(sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr,
                                        SOPC_AC_EVT_METHOD_CALL_SET_STATE, (uint32_t) acMethod, (uintptr_t) ac,
                                        (uintptr_t) cmEvtCtx);
        mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_LocalizedText_Clear(optComment);
        SOPC_Free(optComment);
        if (NULL != cmEvtCtx)
        {
            SOPC_String_Delete(cmEvtCtx->optClientUserId);
            SOPC_Free(cmEvtCtx);
        }
    }
    return (SOPC_STATUS_OK == status);
}

static bool SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_ServerAlarmCondMethods acMethod,
                                                         SOPC_AlarmCondition* ac,
                                                         const SOPC_ByteString* optFailedEventId,
                                                         const SOPC_LocalizedText* optComment,
                                                         const SOPC_CallContext* callCtx,
                                                         SOPC_StatusCode resultStatus)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    SOPC_MethodCallEvt_Ctx* cmEvtCtx = SOPC_Calloc(1, sizeof(*cmEvtCtx));
    status = (NULL != cmEvtCtx ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
    if (SOPC_STATUS_OK == status)
    {
        cmEvtCtx->resultStatus = resultStatus;
    }
    if (SOPC_STATUS_OK == status && NULL != optFailedEventId)
    {
        cmEvtCtx->failedEventId = SOPC_Calloc(1, sizeof(*cmEvtCtx->failedEventId));
        status = SOPC_ByteString_Copy(cmEvtCtx->failedEventId, optFailedEventId);
    }
    if (SOPC_STATUS_OK == status && NULL != optComment)
    {
        if (SOPC_STATUS_OK == status)
        {
            cmEvtCtx->optComment = SOPC_Calloc(1, sizeof(*cmEvtCtx->optComment));
            status = SOPC_LocalizedText_Copy(cmEvtCtx->optComment, optComment);
        }
        if (SOPC_STATUS_OK == status)
        {
            cmEvtCtx->optClientUserId = SOPC_InternalMethodCall_GetClientUserId(callCtx);
        }
    }
    if (SOPC_STATUS_OK != status && NULL != cmEvtCtx)
    {
        SOPC_Free(cmEvtCtx->failedEventId);
        SOPC_Free(cmEvtCtx->optComment);
        SOPC_Free(cmEvtCtx);
        cmEvtCtx = NULL;
    }
    status = SOPC_STATUS_INVALID_STATE;
    if (SOPC_InternalAlarmConditionMgr_IsInit())
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        status =
            SOPC_EventHandler_Post(sopc_alarmConditionConfig.g_alarmCondLooperEvtHdlr, SOPC_AC_EVT_METHOD_CALL_FAILURE,
                                   (uint32_t) acMethod, (uintptr_t) ac, (uintptr_t) cmEvtCtx);
        mutStatus = SOPC_Mutex_Unlock(&sopc_alarmConditionConfig.g_mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }
    if (SOPC_STATUS_OK != status && NULL != cmEvtCtx)
    {
        SOPC_Free(cmEvtCtx->failedEventId);
        SOPC_Free(cmEvtCtx->optComment);
        SOPC_String_Delete(cmEvtCtx->optClientUserId);
        SOPC_Free(cmEvtCtx);
        cmEvtCtx = NULL;
    }
    return (SOPC_STATUS_OK == status);
}

static SOPC_StatusCode SOPC_MethodCall_Disable(const SOPC_CallContext* callContextPtr,
                                               const SOPC_NodeId* objectId,
                                               uint32_t nbInputArgs,
                                               const SOPC_Variant* inputArgs,
                                               uint32_t* nbOutputArgs,
                                               SOPC_Variant** outputArgs,
                                               void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_AlarmCondition* ac = SOPC_InternalMethodCall_RetrieveAC(param, objectId);
    if (NULL == ac)
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_AC_METHOD_DISABLE, ac, NULL, NULL, callContextPtr,
                                                     OpcUa_BadNodeIdInvalid);
        // Used to indicate that the specified ObjectId is not valid or that the Method was
        // called on the AcknowledgeableConditionType Node
        return OpcUa_BadNodeIdInvalid;
        // Note: the 1st choice seems preferred by UACTT
        // StatusCode: "The method id does not refer to a method for the specified object."
        // return OpcUa_BadMethodInvalid;
    }
    // Check state is Disabled (not NULL variant && !Enabled)
    SOPC_Mutex_Lock(&ac->mut);
    const SOPC_Variant* enabledVar = SOPC_Event_GetVariableFromStrPath(ac->data, cEnabledStatePaths.id);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == enabledVar->ArrayType);
    bool disabled = (SOPC_Boolean_Id == enabledVar->BuiltInTypeId && !enabledVar->Value.Boolean);
    SOPC_Mutex_Unlock(&ac->mut);

    bool res = !disabled;
    if (res)
    {
        res = SOPC_InternalMethodCall_SetStateToLooper(SOPC_AC_METHOD_DISABLE, ac, NULL, callContextPtr);
    }

    if (!res)
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_AC_METHOD_DISABLE, ac, NULL, NULL, callContextPtr,
                                                     OpcUa_BadConditionAlreadyEnabled);
        return OpcUa_BadConditionAlreadyDisabled;
    }
    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode SOPC_MethodCall_Enable(const SOPC_CallContext* callContextPtr,
                                              const SOPC_NodeId* objectId,
                                              uint32_t nbInputArgs,
                                              const SOPC_Variant* inputArgs,
                                              uint32_t* nbOutputArgs,
                                              SOPC_Variant** outputArgs,
                                              void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_AlarmCondition* ac = SOPC_InternalMethodCall_RetrieveAC(param, objectId);
    if (NULL == ac)
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_AC_METHOD_ENABLE, ac, NULL, NULL, callContextPtr,
                                                     OpcUa_BadNodeIdInvalid);
        // Used to indicate that the specified ObjectId is not valid or that the Method was
        // called on the AcknowledgeableConditionType Node
        return OpcUa_BadNodeIdInvalid;
        // Note: the 1st choice seems preferred by UACTT
        // StatusCode: "The method id does not refer to a method for the specified object."
        // return OpcUa_BadMethodInvalid;
    }
    // Check state is Disabled, otherwise (include NULL variant) it is considered already Enabled
    SOPC_Mutex_Lock(&ac->mut);
    const SOPC_Variant* enabledVar = SOPC_Event_GetVariableFromStrPath(ac->data, cEnabledStatePaths.id);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == enabledVar->ArrayType);
    bool disabled = (SOPC_Boolean_Id == enabledVar->BuiltInTypeId && !enabledVar->Value.Boolean);
    SOPC_Mutex_Unlock(&ac->mut);

    bool res = disabled;
    if (res)
    {
        res = SOPC_InternalMethodCall_SetStateToLooper(SOPC_AC_METHOD_ENABLE, ac, NULL, callContextPtr);
    }

    if (!res)
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_AC_METHOD_ENABLE, ac, NULL, NULL, callContextPtr,
                                                     OpcUa_BadConditionAlreadyEnabled);
        return OpcUa_BadConditionAlreadyEnabled;
    }
    return SOPC_GoodGenericStatus;
}

static bool SOPC_InternalMethodCall_CheckLatestEventIds(SOPC_AlarmCondition* ac, const SOPC_Variant* eventIdvar)
{
    SOPC_ASSERT(NULL != ac);
    SOPC_ASSERT(NULL != eventIdvar);
    SOPC_ASSERT(eventIdvar->BuiltInTypeId == SOPC_ByteString_Id);
    SOPC_ASSERT(eventIdvar->ArrayType == SOPC_VariantArrayType_SingleValue);
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    bool res = SOPC_EventIdList_Contains(ac->globalEventIds, &eventIdvar->Value.Bstring);
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return res;
}

static bool SOPC_InternalMethodCall_CheckAckOrConfEventIds(SOPC_ServerAlarmCondMethods ackOrConfMethodEnum,
                                                           SOPC_AlarmCondition* ac,
                                                           const SOPC_Variant* eventIdvar)
{
    SOPC_ASSERT(NULL != ac);
    SOPC_ASSERT(NULL != eventIdvar);
    SOPC_ASSERT(eventIdvar->BuiltInTypeId == SOPC_ByteString_Id);
    SOPC_ASSERT(eventIdvar->ArrayType == SOPC_VariantArrayType_SingleValue);
    SOPC_ASSERT(SOPC_AC_METHOD_ACKNOWLEDGE == ackOrConfMethodEnum || SOPC_AC_METHOD_CONFIRM == ackOrConfMethodEnum);
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_EventIdList* eventIds =
        (SOPC_AC_METHOD_ACKNOWLEDGE == ackOrConfMethodEnum) ? ac->ackEventIds : ac->confEventIds;
    bool res = SOPC_EventIdList_Contains(eventIds, &eventIdvar->Value.Bstring);
    mutStatus = SOPC_Mutex_Unlock(&ac->mut);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return res;
}

static SOPC_StatusCode SOPC_MethodCall_AddComment(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param)
{
    SOPC_ASSERT(nbInputArgs == 2);
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_LocalizedText* ltComment = NULL;
    SOPC_AlarmCondition* ac = SOPC_InternalMethodCall_RetrieveAC(param, objectId);
    const SOPC_Variant* eventIdVar = &inputArgs[0];
    // Argument type is already verified by services layer
    const SOPC_LocalizedText* ltCommentInput = inputArgs[1].Value.LocalizedText;
    // From part 9: If the comment field is NULL (both locale and text are empty) it will be ignored and any
    // existing comments will remain unchanged. If the comment is to be reset, an empty text with a locale shall be
    // provided.
    if (NULL != ltCommentInput && ltCommentInput->defaultLocale.Length <= 0 && ltCommentInput->defaultText.Length <= 0)
    {
        ltCommentInput = NULL;
    }
    if (NULL == ac)
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_AC_METHOD_ADD_COMMENT, ac, &eventIdVar->Value.Bstring,
                                                     ltCommentInput, callContextPtr, OpcUa_BadNodeIdInvalid);
        // Used to indicate that the specified ObjectId is not valid or that the Method was
        // called on the AcknowledgeableConditionType Node
        return OpcUa_BadNodeIdInvalid;
        // Note: the 1st choice seems preferred by UACTT
        // StatusCode: "The method id does not refer to a method for the specified object."
        // return OpcUa_BadMethodInvalid;
    }
    // Accept latest eventId or any latest state change eventId
    if (!SOPC_InternalMethodCall_CheckLatestEventIds(ac, eventIdVar))
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(SOPC_AC_METHOD_ADD_COMMENT, ac, &eventIdVar->Value.Bstring,
                                                     ltCommentInput, callContextPtr, OpcUa_BadEventIdUnknown);
        return OpcUa_BadEventIdUnknown;
    }
    // Manage non-empty comment
    if (NULL != ltCommentInput)
    {
        ltComment = SOPC_Calloc(1, sizeof(*ltComment));
        if (NULL != ltComment)
        {
            SOPC_LocalizedText_Initialize(ltComment);
            SOPC_ReturnStatus status = SOPC_LocalizedText_Copy(ltComment, ltCommentInput);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(ltComment);
                ltComment = NULL;
            }
        }
    }
    bool res = SOPC_InternalMethodCall_SetStateToLooper(SOPC_AC_METHOD_ADD_COMMENT, ac, ltComment, callContextPtr);
    SOPC_UNUSED_RESULT(res);

    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode SOPC_InternalMethodCall_AckOrConf(const SOPC_CallContext* callContextPtr,
                                                         const SOPC_NodeId* objectId,
                                                         uint32_t nbInputArgs,
                                                         const SOPC_Variant* inputArgs,
                                                         uint32_t* nbOutputArgs,
                                                         SOPC_Variant** outputArgs,
                                                         void* param,
                                                         const char* ackOrConfStateIdPath,
                                                         SOPC_ServerAlarmCondMethods ackOrConfMethodEnum)
{
    SOPC_ASSERT(nbInputArgs == 2);
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_ASSERT(NULL != ackOrConfStateIdPath);
    SOPC_ASSERT(SOPC_AC_METHOD_ACKNOWLEDGE == ackOrConfMethodEnum || SOPC_AC_METHOD_CONFIRM == ackOrConfMethodEnum);
    const SOPC_Variant* eventIdVar = &inputArgs[0];
    // Argument type is already verified by services layer
    const SOPC_LocalizedText* ltCommentInput = inputArgs[1].Value.LocalizedText;
    // From part 9: If the comment field is NULL (both locale and text are empty) it will be ignored and any
    // existing comments will remain unchanged. If the comment is to be reset, an empty text with a locale shall be
    // provided.
    if (NULL != ltCommentInput && ltCommentInput->defaultLocale.Length <= 0 && ltCommentInput->defaultText.Length <= 0)
    {
        ltCommentInput = NULL;
    }
    SOPC_AlarmCondition* ac = SOPC_InternalMethodCall_RetrieveAC(param, objectId);
    if (NULL == ac)
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(ackOrConfMethodEnum, ac, &eventIdVar->Value.Bstring,
                                                     ltCommentInput, callContextPtr, OpcUa_BadNodeIdInvalid);
        // Used to indicate that the specified ObjectId is not valid or that the Method was
        // called on the AcknowledgeableConditionType Node
        return OpcUa_BadNodeIdInvalid;
        // Note: the 1st choice seems preferred by UACTT
        // StatusCode: "The method id does not refer to a method for the specified object."
        // return OpcUa_BadMethodInvalid;
    }
    // Check EventId is in the eventIds since latest transition to false for the concerned state
    // Note: we do not keep track of other eventIds out of this scope
    if (!SOPC_InternalMethodCall_CheckAckOrConfEventIds(ackOrConfMethodEnum, ac, eventIdVar))
    {
        SOPC_InternalMethodCall_TraceFailureToLooper(ackOrConfMethodEnum, ac, &eventIdVar->Value.Bstring,
                                                     ltCommentInput, callContextPtr, OpcUa_BadEventIdUnknown);
        return OpcUa_BadEventIdUnknown;
    }

    // Check state is Enabled and Unacked otherwise (include NULL variant) it is considered already Acked
    SOPC_Mutex_Lock(&ac->mut);
    const SOPC_Variant* enabledVar = SOPC_Event_GetVariableFromStrPath(ac->data, cEnabledStatePaths.id);
    SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == enabledVar->ArrayType);
    bool enabled = (SOPC_Boolean_Id == enabledVar->BuiltInTypeId && enabledVar->Value.Boolean);
    bool unackedOrUnconfirmed = false;

    if (enabled)
    {
        const SOPC_Variant* ackedOrConfirmedVar = SOPC_Event_GetVariableFromStrPath(ac->data, ackOrConfStateIdPath);
        SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == enabledVar->ArrayType);
        unackedOrUnconfirmed =
            (SOPC_Boolean_Id == ackedOrConfirmedVar->BuiltInTypeId && !ackedOrConfirmedVar->Value.Boolean);
    }
    SOPC_Mutex_Unlock(&ac->mut);
    bool res = unackedOrUnconfirmed;
    SOPC_LocalizedText* ltComment = NULL;
    // Copy Comment from input args if necessary
    if (res && NULL != ltCommentInput)
    {
        ltComment = SOPC_Calloc(1, sizeof(*ltComment));
        if (NULL != ltComment)
        {
            SOPC_LocalizedText_Initialize(ltComment);
            SOPC_ReturnStatus status = SOPC_LocalizedText_Copy(ltComment, ltCommentInput);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Free(ltComment);
                ltComment = NULL;
            }
        }
    }
    // Send event to looper to treat the state change
    if (res)
    {
        res = SOPC_InternalMethodCall_SetStateToLooper(ackOrConfMethodEnum, ac, ltComment, callContextPtr);
    }
    if (!res)
    {
        SOPC_StatusCode badAlreadyAckedOrConfirmed = (SOPC_AC_METHOD_ACKNOWLEDGE == ackOrConfMethodEnum)
                                                         ? OpcUa_BadConditionBranchAlreadyAcked
                                                         : OpcUa_BadConditionBranchAlreadyConfirmed;
        SOPC_InternalMethodCall_TraceFailureToLooper(ackOrConfMethodEnum, ac, &eventIdVar->Value.Bstring,
                                                     ltCommentInput, callContextPtr, badAlreadyAckedOrConfirmed);
        return badAlreadyAckedOrConfirmed;
    }
    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode SOPC_MethodCall_Acknowledge(const SOPC_CallContext* callContextPtr,
                                                   const SOPC_NodeId* objectId,
                                                   uint32_t nbInputArgs,
                                                   const SOPC_Variant* inputArgs,
                                                   uint32_t* nbOutputArgs,
                                                   SOPC_Variant** outputArgs,
                                                   void* param)
{
    return SOPC_InternalMethodCall_AckOrConf(callContextPtr, objectId, nbInputArgs, inputArgs, nbOutputArgs, outputArgs,
                                             param, cAckedStatePaths.id, SOPC_AC_METHOD_ACKNOWLEDGE);
}

static SOPC_StatusCode SOPC_MethodCall_Confirm(const SOPC_CallContext* callContextPtr,
                                               const SOPC_NodeId* objectId,
                                               uint32_t nbInputArgs,
                                               const SOPC_Variant* inputArgs,
                                               uint32_t* nbOutputArgs,
                                               SOPC_Variant** outputArgs,
                                               void* param)
{
    return SOPC_InternalMethodCall_AckOrConf(callContextPtr, objectId, nbInputArgs, inputArgs, nbOutputArgs, outputArgs,
                                             param, cConfirmedStatePaths.id, SOPC_AC_METHOD_CONFIRM);
}

/*
 * Configure the method call manager for the alarm condition type and instances
 */

SOPC_ReturnStatus SOPC_Internal_ConfigureMethodCallManager(SOPC_MethodCallManager* mcm)
{
    SOPC_ASSERT(NULL != mcm);
    SOPC_ReturnStatus status = SOPC_MethodCallManager_AddMethod(mcm, &ConditionRefreshMethod_NodeId,
                                                                &SOPC_MethodCall_ConditionRefresh, NULL, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &ConditionRefresh2Method_NodeId,
                                                  &SOPC_MethodCall_ConditionRefresh, NULL, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &DisableMethodType_NodeId, &SOPC_MethodCall_Disable, NULL, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &EnableMethodType_NodeId, &SOPC_MethodCall_Enable, NULL, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &AddCommentMethodType_NodeId, &SOPC_MethodCall_AddComment, NULL,
                                                  NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &AckMethodType_NodeId, &SOPC_MethodCall_Acknowledge, NULL, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &ConfirmMethodType_NodeId, &SOPC_MethodCall_Confirm, NULL, NULL);
    }
    return status;
}

void SOPC_InternalMethod_InitAlarmCondition_Methods(SOPC_MethodCallManager* mcm, SOPC_AlarmCondition* ac)
{
    for (size_t i = 0; i < SOPC_AC_METHOD_COUNT; i++)
    {
        bool isDefined =
            !(SOPC_IdentifierType_Numeric == ac->methodIds[i].IdentifierType && 0 == ac->methodIds[i].Data.Numeric);
        if (isDefined)
        {
            SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
            switch (i)
            {
            case SOPC_AC_METHOD_DISABLE:
                status = SOPC_MethodCallManager_AddMethod(mcm, &ac->methodIds[i], &SOPC_MethodCall_Disable, ac, NULL);
                break;
            case SOPC_AC_METHOD_ENABLE:
                status = SOPC_MethodCallManager_AddMethod(mcm, &ac->methodIds[i], &SOPC_MethodCall_Enable, ac, NULL);
                break;
            case SOPC_AC_METHOD_ADD_COMMENT:
                status =
                    SOPC_MethodCallManager_AddMethod(mcm, &ac->methodIds[i], &SOPC_MethodCall_AddComment, ac, NULL);
                break;
            case SOPC_AC_METHOD_ACKNOWLEDGE:
                status =
                    SOPC_MethodCallManager_AddMethod(mcm, &ac->methodIds[i], &SOPC_MethodCall_Acknowledge, ac, NULL);
                break;
            case SOPC_AC_METHOD_CONFIRM:
                status = SOPC_MethodCallManager_AddMethod(mcm, &ac->methodIds[i], &SOPC_MethodCall_Confirm, ac, NULL);
                break;
            default:
                break;
            }
            if (SOPC_STATUS_OK != status)
            {
                char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Initialization of AlarmCondition %s MethodCallback SOPC_ServerAlarmCondMethods=%" PRIu32
                    " failed with status %d",
                    conditionIdStr, (uint32_t) i, status);
                SOPC_Free(conditionIdStr);
            }
        }
    }
}

/*
 * Manage asynchronous events generated by method calls to do actual method call treatment.
 * The method call only checked for treatment preconditions.
 */

static void onMethodCallSetState(SOPC_ServerAlarmCondMethods acMethod,
                                 SOPC_AlarmCondition* ac,
                                 SOPC_MethodCallEvt_Ctx* optCtx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_LocalizedText* optComment = NULL;
    SOPC_String* optClientUserId = NULL;
    if (NULL != optCtx)
    {
        optComment = optCtx->optComment;
        optClientUserId = optCtx->optClientUserId;
    }
    switch (acMethod)
    {
    case SOPC_AC_METHOD_DISABLE:
        status = SOPC_InternalAlarmCondition_SetEnabledState(ac, false, true, optComment, true);
        break;
    case SOPC_AC_METHOD_ENABLE:
        status = SOPC_InternalAlarmCondition_SetEnabledState(ac, true, false, optComment, true);
        break;
    case SOPC_AC_METHOD_ADD_COMMENT:
        status = SOPC_InternalAlarmCondition_SetComment(ac, optComment, optClientUserId);
        break;
    case SOPC_AC_METHOD_ACKNOWLEDGE:
        status = SOPC_InternalAlarmCondition_Acknowledge(ac, optComment, optClientUserId, true);
        break;
    case SOPC_AC_METHOD_CONFIRM:
        status = SOPC_InternalAlarmCondition_Confirm(ac, optComment, optClientUserId, true);
        break;
    default:
        SOPC_ASSERT(false);
    }
    if (SOPC_STATUS_OK != status || SOPC_Logger_GetTraceLogLevel() >= SOPC_LOG_LEVEL_DEBUG)
    {
        char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
        const char* optUserStr =
            (optClientUserId == NULL || optClientUserId->Length <= 0 ? "<NULL>"
                                                                     : SOPC_String_GetRawCString(optClientUserId));
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Method call %s (optUser=%s) state change application on AlarmCondition %s failed. ",
                                   SOPC_String_GetRawCString(&acMethodsNames[acMethod].Name), optUserStr,
                                   conditionIdStr);
        }
        else if (NULL == optComment)
        {
            SOPC_Logger_TraceDebug(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Method call %s (optUser=%s) state change application on AlarmCondition %s succeeded",
                SOPC_String_GetRawCString(&acMethodsNames[acMethod].Name), optUserStr, conditionIdStr);
        }
        else
        {
            SOPC_Variant commentVar = {.ArrayType = SOPC_VariantArrayType_SingleValue,
                                       .BuiltInTypeId = SOPC_LocalizedText_Id,
                                       .DoNotClear = true,
                                       .Value.LocalizedText = optComment};
            char* commentStr = SOPC_InternalVarToString(&commentVar);
            SOPC_Logger_TraceDebug(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Method call %s (optUser=%s) state change application on AlarmCondition %s succeeded with comment %s",
                SOPC_String_GetRawCString(&acMethodsNames[acMethod].Name), optUserStr, conditionIdStr, commentStr);
            SOPC_Free(commentStr);
        }
        SOPC_Free(conditionIdStr);
    }

    SOPC_LocalizedText_Clear(optComment);
    SOPC_Free(optComment);
    SOPC_String_Delete(optClientUserId);
    SOPC_Free(optCtx);
}

static void onMethodCallFailure(SOPC_ServerAlarmCondMethods acMethod,
                                const SOPC_AlarmCondition* ac,
                                SOPC_MethodCallEvt_Ctx* optCtx)
{
    SOPC_LocalizedText* optComment = NULL;
    SOPC_ByteString* optFailedEventId = NULL;
    SOPC_String* optClientUserId = NULL;
    SOPC_StatusCode resultStatus = OpcUa_BadInternalError;

    char* conditionIdStr = SOPC_InternalConditionIdToString(ac);
    char* eventIdStr = NULL;
    const char* optUserStr = NULL;
    char* commentStr = NULL;

    if (NULL != optCtx)
    {
        optFailedEventId = optCtx->failedEventId;
        optComment = optCtx->optComment;
        optClientUserId = optCtx->optClientUserId;
        resultStatus = optCtx->resultStatus;
    }
    SOPC_Variant commentVar = {0};
    if (NULL != optComment)
    {
        commentVar.ArrayType = SOPC_VariantArrayType_SingleValue;
        commentVar.BuiltInTypeId = SOPC_LocalizedText_Id;
        commentVar.DoNotClear = true;
        commentVar.Value.LocalizedText = optComment;
    }

    switch (acMethod)
    {
    case SOPC_AC_METHOD_DISABLE:
    case SOPC_AC_METHOD_ENABLE:
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "Failed call to %s method of AlarmCondition %s with status 0x%08" PRIX32,
                              SOPC_String_GetRawCString(&acMethodsNames[acMethod].Name), conditionIdStr, resultStatus);

        break;
    case SOPC_AC_METHOD_ADD_COMMENT:
    case SOPC_AC_METHOD_ACKNOWLEDGE:
    case SOPC_AC_METHOD_CONFIRM:
        optUserStr =
            (optClientUserId == NULL || optClientUserId->Length <= 0 ? "<NULL>"
                                                                     : SOPC_String_GetRawCString(optClientUserId));
        commentStr = SOPC_InternalVarToString(&commentVar);
        eventIdStr =
            (NULL == optFailedEventId ? SOPC_strdup("<NULL>") : SOPC_InternalByteStringToString(optFailedEventId));
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "Failed call to %s (optUser=%s) method of AlarmCondition %s with eventId=%s comment=%s "
                              "status: 0x%08" PRIX32,
                              SOPC_String_GetRawCString(&acMethodsNames[acMethod].Name), optUserStr, conditionIdStr,
                              eventIdStr, commentStr, resultStatus);
        break;

    default:
        SOPC_ASSERT(false);
    }
    SOPC_Free(conditionIdStr);
    SOPC_Free(eventIdStr);
    SOPC_Free(commentStr);

    SOPC_ByteString_Clear(optFailedEventId);
    SOPC_Free(optFailedEventId);
    SOPC_LocalizedText_Clear(optComment);
    SOPC_Free(optComment);
    SOPC_String_Delete(optClientUserId);
    SOPC_Free(optCtx);
}

void onMethodCallLibEvt(SOPC_EventHandler* handler, int32_t event, uint32_t eltId, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    switch ((SOPC_ServerAlarmCondLibEvent) event)
    {
    case SOPC_AC_EVT_METHOD_CALL_SET_STATE:
        onMethodCallSetState((SOPC_ServerAlarmCondMethods) eltId, (SOPC_AlarmCondition*) params,
                             (SOPC_MethodCallEvt_Ctx*) auxParam);
        break;
    case SOPC_AC_EVT_METHOD_CALL_FAILURE:
        onMethodCallFailure((SOPC_ServerAlarmCondMethods) eltId, (SOPC_AlarmCondition*) params,
                            (SOPC_MethodCallEvt_Ctx*) auxParam);
        break;
    case SOPC_AC_EVT_METHOD_CALL_COND_REFRESH:
        SOPC_Internal_ConditionRefresh((SOPC_InternalConditionRefresh_Ctx*) params, (SOPC_StatusCode) auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unexpected internal event");
    }
}
