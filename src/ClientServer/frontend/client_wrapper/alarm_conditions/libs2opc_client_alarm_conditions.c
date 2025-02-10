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

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "libs2opc_client.h"
#include "libs2opc_client_alarm_conditions.h"
#include "libs2opc_request_builder.h"

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_event.h"
#include "sopc_hash.h"
#include "sopc_helper_encode.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

// Internal SOPC_MonitoredAlarm structure
struct _SOPC_MonitoredAlarm
{
    // the last event associated with the alarm
    SOPC_Event* lastEvent;
    SOPC_Mutex eventMutex;
    // the group the alarm is associated with
    SOPC_MonitoredAlarmsGroup* alarmGroup;
};

// Internal SOPC_MonitoredAlarmsGroup structure
struct _SOPC_MonitoredAlarmsGroup
{
    // Subscription associated to the MI
    SOPC_ClientHelper_Subscription* subscription;
    // The MonitoredItemId corresponding to the MonitoredItem of the alarms group.
    // 1 alarms group = 1 MonitoredItem
    uint32_t monitoredItemId;
    // Dictionnary of ConditionId (NodeId) <-> MonitoredAlarm
    SOPC_Dict* monitoredAlarms;
    SOPC_Mutex dictMutex;
    // The select clauses in the MI
    char** alarm_selectClauses;
    size_t alarm_selectClauses_len;
};

static SOPC_SLinkedList* monitored_alarm_groups = NULL;
#define MAX_ALARM_GROUPS 50

static SOPC_MonitoredAlarm_Event_Fct* userCallback = NULL;

static const SOPC_NodeId conditionTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType);
static const char* conditionTypeIdStr = "i=2782";

SOPC_Looper* userCallback_looper = NULL;
SOPC_EventHandler* userCallback_eventHandler = NULL;

typedef enum
{
    UPDATE_ALARM_WITH_USER_CALLBACK,
} eventType;

// qnPaths indicating state of the event
static const char* qnEnabledStateId = "0:EnabledState~0:Id";
static const char* qnAckedStateId = "0:AckedState~0:Id";
static const char* qnConfirmedStateId = "0:ConfirmedState~0:Id";
static const char* qnActiveStateId = "0:ActiveState~0:Id";

// Method Call ids
static const char* acknowledgeMethodId = "i=9111";
static const char* enableMethodId = "i=9027";
static const char* disableMethodId = "i=9028";
static const char* addCommentMethodId = "i=9029";
static const char* confirmMethodId = "i=9113";
static const char* conditionRefreshMethodId = "i=3875";
static const char* conditionRefresh2MethodId = "i=12912";

/*---------------------------------------------------------------------------
 *                                FUNCTIONS
 *---------------------------------------------------------------------------*/

static void maGroup_set_single_selectClause_from_event(const char* qnPath,
                                                       SOPC_Variant* var,
                                                       const SOPC_NodeId* dataType,
                                                       int32_t valueRank,
                                                       uintptr_t qnPathsCtx)
{
    SOPC_UNUSED_ARG(var);
    SOPC_UNUSED_ARG(dataType);
    SOPC_UNUSED_ARG(valueRank);

    SOPC_ASSERT(NULL != qnPath);
    char* qnPath_of_maGroup = SOPC_Malloc(strlen(qnPath) + 1);
    SOPC_ASSERT(NULL != qnPath_of_maGroup);
    strcpy(qnPath_of_maGroup, qnPath);

    SOPC_MonitoredAlarmsGroup* maGroup = (SOPC_MonitoredAlarmsGroup*) qnPathsCtx;
    maGroup->alarm_selectClauses[maGroup->alarm_selectClauses_len] = qnPath_of_maGroup;
    maGroup->alarm_selectClauses_len += 1;
}

static OpcUa_CreateMonitoredItemsResponse* create_monitored_item_event(
    const SOPC_ClientHelper_Subscription* subscription,
    const SOPC_NodeId* notifierId,
    SOPC_MonitoredAlarmsGroup* maGroup,
    const SOPC_Event* optAlarmFields,
    uint32_t queueSizeRequested)
{
    SOPC_ASSERT(NULL != subscription);
    SOPC_UNUSED_ARG(optAlarmFields);

    OpcUa_CreateMonitoredItemsRequest* createMonItReq =
        SOPC_CreateMonitoredItemsRequest_Create(0, 1, OpcUa_TimestampsToReturn_Both);
    if (NULL == createMonItReq)
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(
        createMonItReq, 0, notifierId, SOPC_AttributeId_EventNotifier, NULL);

    /* Make eventFilter: ask for fields of AlarmConditionType + ConditionType events or subtypes */
    OpcUa_EventFilter* eventFilter = NULL;
    SOPC_Event* alarmEvent = NULL;
    size_t nb_select_clauses_alarm = 0;
    if (SOPC_STATUS_OK == status)
    {
        alarmEvent = SOPC_Event_GetInstanceAlarmConditionType();
        if (NULL == alarmEvent)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            nb_select_clauses_alarm = SOPC_Event_GetNbVariables(alarmEvent);
            SOPC_ASSERT(0 < nb_select_clauses_alarm);
            size_t nbWhereClauseElts = 1;
            eventFilter = SOPC_MonitoredItem_CreateEventFilter(nb_select_clauses_alarm, nbWhereClauseElts);
            if (NULL == eventFilter)
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        // WhereClause of filter: ConditionType or subtype
        // Careful: events RefreshStart/End/Required ignore the filter and will always be received.
        // (OPC UA v1.05 part9 4.5)
        status = SOPC_EventFilter_SetOfTypeWhereClause(eventFilter, 0, &conditionTypeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        // SelectClause of filter: fields of AlarmConditionType.
        // The clause paths are resolved from BaseEventType

        // ConditionId clause (v1.05.03 part 9 Table 12)
        status = SOPC_EventFilter_SetSelectClauseFromStringPath(eventFilter, 0, NULL, '~', "", SOPC_AttributeId_NodeId,
                                                                NULL);
        // Other clauses are "normal" clauses
        maGroup->alarm_selectClauses = SOPC_Malloc(nb_select_clauses_alarm * sizeof(char*));
        SOPC_ASSERT(NULL != maGroup->alarm_selectClauses);
        maGroup->alarm_selectClauses_len = 0;
        SOPC_Event_ForEachVar(alarmEvent, &maGroup_set_single_selectClause_from_event, (uintptr_t) maGroup);
        for (size_t i = 1; i < nb_select_clauses_alarm && SOPC_STATUS_OK == status; i++)
        {
            status = SOPC_EventFilter_SetSelectClauseFromStringPath(
                eventFilter, i, NULL, '~', maGroup->alarm_selectClauses[i - 1], SOPC_AttributeId_Value, NULL);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ExtensionObject* extObj = SOPC_MonitoredItem_EventFilter(eventFilter);
        if (NULL != extObj)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
                createMonItReq, 0, OpcUa_MonitoringMode_Reporting, 0, -1, extObj, queueSizeRequested, true);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    // Create the monitored items
    OpcUa_CreateMonitoredItemsResponse* createMonItResp = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_EncodeableObject_Create(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, (void**) &createMonItResp);
    }
    if (SOPC_STATUS_OK == status)
    {
        // MI context is alarm group, used on receive events
        const SOPC_MonitoredAlarmsGroup* monitoredItemCtxtAlarmsGroup[1] = {maGroup};
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
            subscription, createMonItReq, (const uintptr_t*) monitoredItemCtxtAlarmsGroup, createMonItResp);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, (void**) &createMonItResp);
        createMonItResp = NULL;
    }

    SOPC_Event_Delete(&alarmEvent);
    return createMonItResp;
}

static SOPC_ReturnStatus check_mi_response(OpcUa_CreateMonitoredItemsResponse* createMonItResp, char** selectClausesReq)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (SOPC_IsGoodStatus(createMonItResp->ResponseHeader.ServiceResult))
    {
        status = SOPC_STATUS_OK;
        if (createMonItResp->NoOfResults == 1)
        {
            if (!SOPC_IsGoodStatus(createMonItResp->Results[0].StatusCode))
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "A&C CLIENT: CreateMonitoredItemsResponse[0] result not good: 0x%08" PRIX32 "\n",
                                       createMonItResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: Unexpected number of MI in "
                                   "CreateMonitoredItemsResponse %" PRIi32 " != 1\n",
                                   createMonItResp->NoOfResults);
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: CreateMonitoredItemsResponse "
                               "global result not good: 0x%08" PRIX32 "\n",
                               createMonItResp->ResponseHeader.ServiceResult);
    }

    // Check filterResult if it exists
    if (SOPC_ExtObjBodyEncoding_None != createMonItResp->Results[0].FilterResult.Encoding)
    {
        // For the moment:
        // - Where clause is fixed by API
        // - Select clauses fixed by API but will not be true for future version (parameter optAlarmFields
        //   given by client)
        OpcUa_EventFilterResult* filterResult =
            (OpcUa_EventFilterResult*) createMonItResp->Results[0].FilterResult.Body.Object.Value;

        // Check the only Where clause
        SOPC_ASSERT(1 == filterResult->WhereClauseResult.NoOfElementResults);
        OpcUa_ContentFilterElementResult* whereEltRes = &filterResult->WhereClauseResult.ElementResults[0];
        if (!SOPC_IsGoodStatus(whereEltRes->StatusCode))
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: filterResult WhereClauseResult result unexpected: 0x%08" PRIX32 "",
                                   whereEltRes->StatusCode);
            status = SOPC_STATUS_NOK;
        }

        // Check the Select clauses
        for (int32_t i = 0; i < filterResult->NoOfSelectClauseResults; i++)
        {
            if (!SOPC_IsGoodStatus(filterResult->SelectClauseResults[i]))
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "A&C CLIENT: filterResult SelectClauseResult result unexpected: 0x%08" PRIX32
                                       ""
                                       "for clause %s",
                                       filterResult->SelectClauseResults[i], selectClausesReq[i]);
                status = SOPC_STATUS_NOK;
            }
        }
    }

    return status;
}

static void monitored_alarm_free(uintptr_t elt)
{
    SOPC_MonitoredAlarm* ma = (SOPC_MonitoredAlarm*) elt;

    // Delete event
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&ma->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Event_Delete(&ma->lastEvent);
    mutStatus = SOPC_Mutex_Unlock(&ma->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    // Delete mutex
    SOPC_UNUSED_RESULT(SOPC_Mutex_Clear(&ma->eventMutex));

    // Dereference alarm group associated with the alarm
    ma->alarmGroup = NULL;

    // Delete alarm
    SOPC_Free(ma);
}

static void SOPC_MonitoredAlarm_TriggerSubscriptionNotification(const SOPC_ClientHelper_Subscription* subscription,
                                                                SOPC_StatusCode status,
                                                                SOPC_EncodeableType* notificationType,
                                                                uint32_t nbNotifElts,
                                                                const void* notification,
                                                                uintptr_t* monitoredItemCtxArray)
{
    SOPC_ASSERT(NULL != subscription);
    SOPC_UNUSED_ARG(status);
    if (&OpcUa_EventNotificationList_EncodeableType != notificationType)
    {
        // Ignore notifications that are not events.
        return;
    }

    // Verify client API context
    SOPC_ASSERT(12 == SOPC_ClientHelper_Subscription_GetUserParam(subscription));

    SOPC_Buffer* buf = SOPC_Buffer_CreateResizable(1024, 1024 * 16);
    if (NULL == buf)
    {
        return; // OOM
    }
    const OpcUa_EventNotificationList* eventList = (const OpcUa_EventNotificationList*) notification;
    bool localExpectedContentReceived = ((int32_t) nbNotifElts == eventList->NoOfEvents);

    // Retrieve alarm group associated with the MI that received the notification
    SOPC_MonitoredAlarmsGroup* MAgroup = (SOPC_MonitoredAlarmsGroup*) monitoredItemCtxArray[0];

    for (int32_t i = 0; localExpectedContentReceived && i < eventList->NoOfEvents; i++)
    {
        const OpcUa_EventFieldList* event = &eventList->Events[i];

        // Create SOPC_Event that will be stored in alarm (field lastEvent)
        SOPC_Event* alarmConditionTypeEvent = SOPC_Event_GetInstanceAlarmConditionType();
        SOPC_ReturnStatus stat = (NULL != alarmConditionTypeEvent) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

        // Fill event fields
        for (int32_t iField = 0; SOPC_STATUS_OK == stat && iField < event->NoOfEventFields; iField++)
        {
            const SOPC_Variant* var = &event->EventFields[iField];
            SOPC_Buffer_Reset(buf);
            SOPC_Variant_Dump(buf, var);
            // Ensure that there is a ZERO char at the end.
            if (buf->length < buf->maximum_size)
            {
                // Nominal case
                buf->data[buf->length] = 0;
            }
            else
            {
                SOPC_ASSERT(buf->maximum_size > 0);
                // Robustness: replace last char by a NULL char
                buf->data[buf->maximum_size - 1] = 0;
            }

            if (0 == iField)
            {
                // Position 0 is ConditionId
                if (SOPC_NodeId_Id != var->BuiltInTypeId)
                {
                    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                             "A&C CLIENT: No ConditionId in received event. Event will be ignored.");
                    stat = SOPC_STATUS_NOK;
                }
                else
                {
                    stat = SOPC_Event_SetVariableFromStrPath(alarmConditionTypeEvent, "OwnNodeId", var);
                }
            }
            else
            {
                const char* clauseStr = MAgroup->alarm_selectClauses[iField - 1];
                stat = SOPC_Event_SetVariableFromStrPath(alarmConditionTypeEvent, clauseStr, var);
            }
        }

        // If all the fields of the event were filled correctly, make additionnal checks on event
        // before updating the alarm:
        // - event time must be greater than current alarm event (check if alarm exists already)
        // - BranchId must be NULL (BranchId != NULL not managed for the moment, they are just ignored)
        if (SOPC_STATUS_OK == stat)
        {
            // Check BranchId there
            const SOPC_Variant* branchId = SOPC_Event_GetVariableFromStrPath(alarmConditionTypeEvent, "0:BranchId");
            if (SOPC_Null_Id != branchId->BuiltInTypeId)
            {
                stat = SOPC_STATUS_NOK;
            }
        }

        // TODO: special case for event RefreshStart/End/Required, which are not alarms ? Because it does not
        // make sense to call update alarm's callback on them.

        if (SOPC_STATUS_OK == stat)
        {
            bool alarmUpdated = false;
            const SOPC_NodeId* conditionId = SOPC_Event_GetNodeId(alarmConditionTypeEvent);
            // ConditionId must not be null if we arrived here
            SOPC_ASSERT(NULL != conditionId);
            char* conditionIdStr = SOPC_NodeId_ToCString(conditionId);
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: New event received for MonitoredAlarm on node %s", conditionIdStr);
            // Get dictionnary of the alarm group
            SOPC_ReturnStatus dictMutStatus = SOPC_Mutex_Lock(&MAgroup->dictMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == dictMutStatus);
            SOPC_Dict* alarmDict = MAgroup->monitoredAlarms;

            // Retrieve alarm associated with the ConditionId of the event (in this group) or create one
            bool found = false;
            SOPC_MonitoredAlarm* MAToAddOrUpdate = NULL;
            SOPC_MonitoredAlarm* monitoredAlarmFromDict =
                (SOPC_MonitoredAlarm*) SOPC_Dict_Get(alarmDict, (const uintptr_t) conditionId, &found);
            if (found)
            {
                MAToAddOrUpdate = monitoredAlarmFromDict;
                SOPC_ReturnStatus eventMutStatus = SOPC_Mutex_Lock(&MAToAddOrUpdate->eventMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == eventMutStatus);
                // Check event time because alarm is existing
                SOPC_DateTime newEventTime = SOPC_Event_GetTime(alarmConditionTypeEvent);
                SOPC_DateTime currentEventTime = SOPC_Event_GetTime(MAToAddOrUpdate->lastEvent);
                int32_t comparison = -2;
                SOPC_ASSERT(SOPC_STATUS_OK == SOPC_DateTime_Compare(&currentEventTime, &newEventTime, &comparison));
                if (comparison <= 0 || (-1 == newEventTime && -1 == currentEventTime))
                {
                    // Replace if:
                    // - current time <= new time OR
                    // - if server does not generate Time clause for events (in this case SOPC_Event_GetTime returns -1)
                    SOPC_Event_Clear(MAToAddOrUpdate->lastEvent);
                    SOPC_Free(MAToAddOrUpdate->lastEvent);
                    MAToAddOrUpdate->lastEvent = alarmConditionTypeEvent;
                    alarmUpdated = true;
                }
                else
                {
                    // else: ignore event and log warning
                    SOPC_Logger_TraceWarning(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "A&C CLIENT: Event will be ignored because occuration time of the new event is "
                        "less than current event time");
                }
                eventMutStatus = SOPC_Mutex_Unlock(&MAToAddOrUpdate->eventMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == eventMutStatus);
            }
            else
            {
                // Create key for new alarm
                SOPC_NodeId* conditionIdCpy = SOPC_Calloc(1, sizeof(SOPC_NodeId));
                SOPC_ASSERT(NULL != conditionIdCpy);
                stat = SOPC_NodeId_Copy(conditionIdCpy, conditionId);
                SOPC_ASSERT(SOPC_STATUS_OK == stat);
                // Create new value alarm
                MAToAddOrUpdate = SOPC_Calloc(1, sizeof(SOPC_MonitoredAlarm));
                SOPC_ASSERT(NULL != MAToAddOrUpdate);
                MAToAddOrUpdate->lastEvent = alarmConditionTypeEvent;
                MAToAddOrUpdate->alarmGroup = MAgroup;
                SOPC_ReturnStatus eventMutStatus = SOPC_Mutex_Initialization(&MAToAddOrUpdate->eventMutex);
                SOPC_ASSERT(SOPC_STATUS_OK == eventMutStatus);
                // Add (key, value) in the dictionnary
                bool res = SOPC_Dict_Insert(alarmDict, (uintptr_t) conditionIdCpy, (uintptr_t) MAToAddOrUpdate);
                if (res)
                {
                    alarmUpdated = true;
                }
                else
                {
                    // Fail at insert the (key, value) in the dictionnary. Log and free the value + key
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "A&C CLIENT: Failed at creating MonitoredAlarm on node %s", conditionIdStr);
                    SOPC_NodeId_Clear(conditionIdCpy);
                    SOPC_Free(conditionIdCpy);
                    eventMutStatus = SOPC_Mutex_Clear(&MAToAddOrUpdate->eventMutex);
                    SOPC_ASSERT(SOPC_STATUS_OK == eventMutStatus);
                    MAToAddOrUpdate->lastEvent = NULL;
                    MAToAddOrUpdate->alarmGroup = NULL;
                    SOPC_Free(MAToAddOrUpdate);
                }
            }
            SOPC_Free(conditionIdStr);

            dictMutStatus = SOPC_Mutex_Unlock(&MAgroup->dictMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == dictMutStatus);

            if (alarmUpdated)
            {
                // Call user-callback in a new thread so we finish this one
                SOPC_Event* eventCopy = SOPC_Event_CreateCopy(alarmConditionTypeEvent, false);
                status = SOPC_EventHandler_Post(userCallback_eventHandler, UPDATE_ALARM_WITH_USER_CALLBACK, 0,
                                                (uintptr_t) MAToAddOrUpdate, (uintptr_t) eventCopy);
                SOPC_ASSERT(SOPC_STATUS_OK == status);
            }
            else
            {
                // No alarm was updated with event. Free the event.
                SOPC_Event_Delete(&alarmConditionTypeEvent);
            }
        }
        else
        {
            SOPC_Event_Delete(&alarmConditionTypeEvent);
        }
    }
    SOPC_Buffer_Delete(buf);
}

static void OnInputEvent(SOPC_EventHandler* pHandler,
                         int32_t event,
                         uint32_t eltId,
                         uintptr_t pParams,
                         uintptr_t pAuxParam)
{
    SOPC_UNUSED_ARG(pHandler);
    SOPC_UNUSED_ARG(eltId);

    SOPC_MonitoredAlarm* ma = NULL;
    SOPC_Event* eventNotif = NULL;
    switch (event)
    {
    case UPDATE_ALARM_WITH_USER_CALLBACK:
        ma = (SOPC_MonitoredAlarm*) pParams;
        eventNotif = (SOPC_Event*) pAuxParam;
        userCallback(ma->alarmGroup, ma, eventNotif);
        break;
    default:
        SOPC_ASSERT(false);
    }
}

// Initialize MonitoredAlarmMgr
SOPC_ReturnStatus SOPC_MonitoredAlarmMgr_Initialize(SOPC_MonitoredAlarm_Event_Fct* eventCb, bool ignoreRetain)
{
    if (NULL == eventCb)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: Initialization failed. Please provide a valid callback function "
                               "for handling alarm updates.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_UNUSED_ARG(ignoreRetain);

    // 1) Set user callback
    userCallback = eventCb;

    // 2) Create user callback thread and looper
    const char* userCallback_thread = "USER_CALLBACK_THREAD";
    SOPC_ASSERT(NULL == userCallback_looper);
    userCallback_looper = SOPC_Looper_Create(userCallback_thread);
    if (NULL != userCallback_looper)
    {
        userCallback_eventHandler = SOPC_EventHandler_Create(userCallback_looper, OnInputEvent);
    }

    // 3) Create list of monitored alarm groups
    monitored_alarm_groups = SOPC_SLinkedList_Create(MAX_ALARM_GROUPS);

    return (NULL != userCallback_eventHandler && NULL != monitored_alarm_groups) ? SOPC_STATUS_OK
                                                                                 : SOPC_STATUS_OUT_OF_MEMORY;
}

static SOPC_ReturnStatus delete_monitored_items(const SOPC_ClientHelper_Subscription* subscription,
                                                uint32_t monitoredItemId)
{
    SOPC_ASSERT(NULL != subscription);

    // Prepare delete monitored items request
    OpcUa_DeleteMonitoredItemsResponse delMonItResp;
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&delMonItResp);
    OpcUa_DeleteMonitoredItemsRequest* delMonItReq = SOPC_DeleteMonitoredItemsRequest_Create(0, 1, &monitoredItemId);
    SOPC_ReturnStatus status = (NULL != delMonItReq) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, delMonItReq, &delMonItResp);
    }

    // Check DeleteMonitoredItems response
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(delMonItResp.ResponseHeader.ServiceResult))
        {
            if (!SOPC_IsGoodStatus(delMonItResp.Results[0]))
            {
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    OpcUa_DeleteMonitoredItemsResponse_Clear(&delMonItResp);
    return status;
}

static bool search_for_same_subscription_in_groups(const SOPC_ClientHelper_Subscription* subscription)
{
    uint32_t subscriptionId = 0;
    uint32_t counter = 0;
    bool hasSubcriptionUniqueUsage = true;
    SOPC_ReturnStatus status = SOPC_ClientHelper_GetSubscriptionId(subscription, &subscriptionId);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    // Iterate on alarm groups
    SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(monitored_alarm_groups);
    while (SOPC_SLinkedList_HasNext(&it))
    {
        uint32_t subscriptionId_it = 0;
        SOPC_MonitoredAlarmsGroup* alarmGroup_it = (SOPC_MonitoredAlarmsGroup*) SOPC_SLinkedList_Next(&it);
        status = SOPC_ClientHelper_GetSubscriptionId(alarmGroup_it->subscription, &subscriptionId_it);
        if (SOPC_STATUS_OK == status && subscriptionId == subscriptionId_it)
        {
            counter += 1;
        }
    }

    if (counter > 1)
    {
        hasSubcriptionUniqueUsage = false;
    }
    return hasSubcriptionUniqueUsage;
}

void SOPC_MonitoredAlarm_ClearAlarmsGroup(SOPC_MonitoredAlarmsGroup* group)
{
    if (NULL == group)
    {
        return;
    }

    // Delete MI and maybe subscription
    if (NULL != group->subscription && 0 < group->monitoredItemId)
    {
        // Delete MI
        SOPC_ASSERT(SOPC_STATUS_OK == delete_monitored_items(group->subscription, group->monitoredItemId));
        group->monitoredItemId = 0;
        // Delete the subscription ONLY if it is not used in another alarms group, because
        // subscription is necessary for deleting MI.
        bool hasSubcriptionUniqueUsage = search_for_same_subscription_in_groups(group->subscription);
        if (hasSubcriptionUniqueUsage)
        {
            SOPC_ASSERT(SOPC_STATUS_OK == SOPC_ClientHelper_DeleteSubscription(&group->subscription));
        }
    }

    // Delete qnPaths of the alarm type
    if (0 < group->alarm_selectClauses_len)
    {
        for (size_t i = 0; i < group->alarm_selectClauses_len; i++)
        {
            SOPC_Free(group->alarm_selectClauses[i]);
        }
        SOPC_Free(group->alarm_selectClauses);
        group->alarm_selectClauses_len = 0;
    }

    // Delete dictionnary of alarms and their content
    if (NULL != group->monitoredAlarms)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&group->dictMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        SOPC_Dict_Delete(group->monitoredAlarms);
        mutStatus = SOPC_Mutex_Unlock(&group->dictMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    }

    // Delete mutex
    SOPC_UNUSED_RESULT(SOPC_Mutex_Clear(&group->dictMutex));

    // Remove entry in the list and final object
    SOPC_SLinkedList_RemoveFromValuePtr(monitored_alarm_groups, (uintptr_t) group);
    SOPC_Free(group);
}

void SOPC_MonitoredAlarmMgr_Clear(void)
{
    if (NULL != monitored_alarm_groups)
    {
        // Iterate on alarm groups
        SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(monitored_alarm_groups);
        while (SOPC_SLinkedList_HasNext(&it))
        {
            // Using SOPC_SLinkedList_Apply with clear function is tricky here because
            // the current clear function also iterates on the list elements (search for same subscription)
            SOPC_MonitoredAlarmsGroup* alarmGroup_it = (SOPC_MonitoredAlarmsGroup*) SOPC_SLinkedList_Next(&it);
            SOPC_MonitoredAlarm_ClearAlarmsGroup(alarmGroup_it);
        }
        SOPC_SLinkedList_Delete(monitored_alarm_groups);
    }

    // Dereference user callback
    userCallback = NULL;

    // Delete looper
    userCallback_eventHandler = NULL;
    SOPC_Looper_Delete(userCallback_looper);
}

static SOPC_ClientHelper_Subscription* get_connection_subscription_in_groups(
    const SOPC_ClientConnection* secureConnection)
{
    SOPC_ClientHelper_Subscription* sub = NULL;

    // Iterate on alarm groups and check for equality of pointer secureConnection
    SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(monitored_alarm_groups);
    bool stop = false;
    while (SOPC_SLinkedList_HasNext(&it) && !stop)
    {
        SOPC_MonitoredAlarmsGroup* alarmGroup_it = (SOPC_MonitoredAlarmsGroup*) SOPC_SLinkedList_Next(&it);
        const SOPC_ClientConnection* secureConnection_it =
            SOPC_ClientHelper_GetSecureConnection(alarmGroup_it->subscription);
        if (secureConnection_it == secureConnection)
        {
            sub = alarmGroup_it->subscription;
            stop = true;
        }
    }
    return sub;
}

SOPC_MonitoredAlarmsGroup* SOPC_MonitoredAlarm_CreateAlarmsGroup(SOPC_ClientConnection* secureConnection,
                                                                 OpcUa_CreateSubscriptionRequest* createSubReq,
                                                                 const SOPC_NodeId* notifierId,
                                                                 const SOPC_Event* optAlarmFields)
{
    SOPC_UNUSED_ARG(optAlarmFields);
    SOPC_ASSERT(NULL != monitored_alarm_groups);

    SOPC_ClientHelper_Subscription* sub = get_connection_subscription_in_groups(secureConnection);
    if (NULL == sub)
    {
        // Create subscription with user parameters
        if (NULL == createSubReq)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "A&C CLIENT: Create Monitored alarm group: please "
                "provide subcription parameters for the creation of the monitored item of the alarm group.");
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: Create Monitored alarm group: creating new "
                                   "subscription associated to that connection.");
            sub = SOPC_ClientHelper_CreateSubscription(secureConnection, createSubReq,
                                                       SOPC_MonitoredAlarm_TriggerSubscriptionNotification, 12);
        }
    }
    else
    {
        SOPC_Logger_TraceDebug(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "A&C CLIENT: Create Monitored alarm group: There is "
            "already one subscription associated to this connection. Subscription parameters will be ignored and "
            "this subscription will be used for creating new MonitoredItem.");
        SOPC_UNUSED_ARG(createSubReq);
    }
    SOPC_ReturnStatus status = (NULL != sub ? SOPC_STATUS_OK : SOPC_STATUS_NOK);

    SOPC_MonitoredAlarmsGroup* ma_group = NULL;
    if (SOPC_STATUS_OK == status)
    {
        ma_group = SOPC_Calloc(1, sizeof(SOPC_MonitoredAlarmsGroup));
        SOPC_ASSERT(NULL != ma_group);

        // Create MI on notifierId with WHERE clause ConditionType or subtype and SELECT Clause AlarmConditionType
        // clauses (optAlarmFields later). The alarms group is given as parameter because alarm select clauses are put
        // in the structure + the structure is given as context of the CreateMI request.
        ma_group->monitoredItemId = 0;
        OpcUa_CreateMonitoredItemsResponse* miRespServer =
            create_monitored_item_event(sub, notifierId, ma_group, NULL, UINT32_MAX);
        if (NULL != miRespServer)
        {
            // Check response
            status = check_mi_response(miRespServer, ma_group->alarm_selectClauses);
            if (SOPC_STATUS_OK == status)
            {
                ma_group->monitoredItemId = miRespServer->Results[0].MonitoredItemId;
            }
            OpcUa_CreateMonitoredItemsResponse_Clear(miRespServer);
            SOPC_Free(miRespServer);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    char* notifierNodeIdString = SOPC_NodeId_ToCString(notifierId);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Dict* dictionnary = SOPC_NodeId_Dict_Create(true, monitored_alarm_free);
        if (NULL != dictionnary)
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: Create Monitored alarm group: successfully created "
                                   "on notifier node %s",
                                   notifierNodeIdString);
            ma_group->subscription = sub;
            ma_group->monitoredAlarms = dictionnary;
            SOPC_ReturnStatus dictMutStatus = SOPC_Mutex_Initialization(&ma_group->dictMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == dictMutStatus);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: Create Monitored alarm group: failed creation on notifier node %s",
                               notifierNodeIdString);
        // Alarms group may have been partially filled (during create_monitored_item_event), free it.
        SOPC_MonitoredAlarm_ClearAlarmsGroup(ma_group);
        ma_group = NULL;
    }
    else
    {
        if (0 == SOPC_SLinkedList_Append(monitored_alarm_groups, 0, (uintptr_t) ma_group))
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: Create Monitored alarm group: "
                                   "Out of memory fail.");
            SOPC_MonitoredAlarm_ClearAlarmsGroup(ma_group);
            ma_group = NULL;
        }
    }

    SOPC_Free(notifierNodeIdString);
    return ma_group;
}

SOPC_ReturnStatus SOPC_MonitoredAlarm_GetSubscriptionRevisedParams(SOPC_MonitoredAlarmsGroup* alarmGroup,
                                                                   double* revisedPublishingInterval,
                                                                   uint32_t* revisedLifetimeCount,
                                                                   uint32_t* revisedMaxKeepAliveCount)
{
    if (NULL == alarmGroup || NULL == alarmGroup->subscription)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_ClientHelper_Subscription_GetRevisedParameters(alarmGroup->subscription, revisedPublishingInterval,
                                                               revisedLifetimeCount, revisedMaxKeepAliveCount);
}

/*---------------------------------------------------------------------------*/

static bool check_parameter_alarm(const SOPC_MonitoredAlarm* alarmCond)
{
    bool res = true;
    if (NULL == alarmCond)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: Impossible to get alarm state because no alarm was provided as argument.");
        res = false;
    }
    else if (NULL == alarmCond->lastEvent)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "A&C CLIENT: Impossible to get alarm state because no event has been received on that alarm.");
        res = false;
    }
    return res;
}

static SOPC_StatusCode check_call_response_generic(const OpcUa_CallResponse* callResponse,
                                                   const char* optEventIdParamStr,
                                                   const char* method_name)
{
    SOPC_ASSERT(NULL != callResponse);
    SOPC_StatusCode returnSc = callResponse->ResponseHeader.ServiceResult;
    if (SOPC_IsGoodStatus(returnSc))
    {
        OpcUa_CallMethodResult* result = &callResponse->Results[0];
        returnSc = result->StatusCode;
        if (1 != callResponse->NoOfResults)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: %s method call wrong number of response items %s%s", method_name,
                                   optEventIdParamStr ? "on event with eventId " : "",
                                   optEventIdParamStr ? optEventIdParamStr : "");
        }
        else if (!SOPC_IsGoodStatus(result->StatusCode))
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "A&C CLIENT: %s method call bad result 0x%08" PRIX32 " %s%s", method_name, returnSc,
                                   optEventIdParamStr ? "on event with eventId " : "",
                                   optEventIdParamStr ? optEventIdParamStr : "");
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "A&C CLIENT: %s method call success result %s%s",
                                   method_name, optEventIdParamStr ? "on event with eventId " : "",
                                   optEventIdParamStr ? optEventIdParamStr : "");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: %s method call service bad status 0x%08" PRIX32 " %s%s", method_name,
                               returnSc, optEventIdParamStr ? "on event with eventId " : "",
                               optEventIdParamStr ? optEventIdParamStr : "");
    }

    return returnSc;
}

SOPC_StatusCode SOPC_MonitoredAlarmCall_Enable(SOPC_MonitoredAlarm* alarmCond, bool enable)
{
    // Check params
    if (!check_parameter_alarm(alarmCond))
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_StatusCode returnSc = OpcUa_BadInternalError;
    // Get the SC of the client
    SOPC_ClientConnection* secureConnection =
        SOPC_ClientHelper_GetSecureConnection(alarmCond->alarmGroup->subscription);
    SOPC_ASSERT(NULL != secureConnection);

    // Prepare CallRequest
    OpcUa_CallResponse* callResponse = NULL;
    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = (NULL != callRequest) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        char* conditionId = SOPC_NodeId_ToCString(SOPC_Event_GetNodeId(alarmCond->lastEvent));
        mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, conditionId,
                                                             enable ? enableMethodId : disableMethodId, 0, NULL);
        SOPC_Free(conditionId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, callRequest, (void**) &callResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        returnSc = check_call_response_generic(callResponse, NULL, enable ? "Enable" : "Disable");
        SOPC_EncodeableObject_Delete(callResponse->encodeableType, (void**) &callResponse);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: Fail at creation or at sending the method call request %s "
                               "to the server",
                               enable ? "Enable" : "Disable");
    }

    return returnSc;
}

static SOPC_StatusCode call_request_with_params_eventId_and_comment(SOPC_MonitoredAlarm* alarmCond,
                                                                    SOPC_LocalizedText* optComment,
                                                                    const char* methodId,
                                                                    const char* methodName)
{
    // Check params
    if (!check_parameter_alarm(alarmCond))
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_StatusCode returnSc = OpcUa_BadInternalError;
    // Get the SC of the client
    SOPC_ClientConnection* secureConnection =
        SOPC_ClientHelper_GetSecureConnection(alarmCond->alarmGroup->subscription);
    SOPC_ASSERT(NULL != secureConnection);

    // Prepare CallRequest
    char* eventIdStr = NULL; // For future use in logs.
    SOPC_Variant* inputParameters = NULL;
    OpcUa_CallResponse* callResponse = NULL;
    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = (NULL != callRequest) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    if (SOPC_STATUS_OK == status)
    {
        inputParameters = SOPC_Calloc(2, sizeof(SOPC_Variant));
        if (NULL == inputParameters)
        {
            SOPC_EncodeableObject_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &callRequest);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // parameter 1: EventId
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        const SOPC_ByteString* bsEventId = SOPC_Event_GetEventId(alarmCond->lastEvent);
        eventIdStr = SOPC_Event_GetCstringEventId(alarmCond->lastEvent);
        mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        SOPC_ByteString bsEventIdCopy;
        SOPC_ByteString_Initialize(&bsEventIdCopy);
        if (NULL != bsEventId)
        {
            // Need copy of the eventId because the request parameters memory is managed by service API
            status = SOPC_ByteString_Copy(&bsEventIdCopy, bsEventId);
            if (SOPC_STATUS_OK == status)
            {
                inputParameters[0].BuiltInTypeId = SOPC_ByteString_Id;
                inputParameters[0].ArrayType = SOPC_VariantArrayType_SingleValue;
                inputParameters[0].Value.Bstring = bsEventIdCopy;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    // parameter 2: optionnal Comment
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != optComment)
        {
            inputParameters[1].BuiltInTypeId = SOPC_LocalizedText_Id;
            inputParameters[1].ArrayType = SOPC_VariantArrayType_SingleValue;
            inputParameters[1].Value.LocalizedText = optComment;
        }
        else
        {
            SOPC_Variant_Initialize(&inputParameters[1]);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        char* conditionId = SOPC_NodeId_ToCString(SOPC_Event_GetNodeId(alarmCond->lastEvent));
        mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, conditionId, methodId, 2, inputParameters);
        SOPC_Free(conditionId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, callRequest, (void**) &callResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        returnSc = check_call_response_generic(callResponse, eventIdStr, methodName);
        SOPC_EncodeableObject_Delete(callResponse->encodeableType, (void**) &callResponse);
    }
    else
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "A&C CLIENT: Fail at creation or at sending the method call request %s to the server on event with "
            "eventId %s.",
            methodName, eventIdStr);
    }

    SOPC_Variant_Delete(inputParameters);
    SOPC_Free(eventIdStr);
    return returnSc;
}

SOPC_StatusCode SOPC_MonitoredAlarmCall_Acknowledge(SOPC_MonitoredAlarm* alarmCond, SOPC_LocalizedText* optComment)
{
    return call_request_with_params_eventId_and_comment(alarmCond, optComment, acknowledgeMethodId, "Acknowledge");
}

SOPC_StatusCode SOPC_MonitoredAlarmCall_Confirm(SOPC_MonitoredAlarm* alarmCond, SOPC_LocalizedText* optComment)
{
    return call_request_with_params_eventId_and_comment(alarmCond, optComment, confirmMethodId, "Confirm");
}

SOPC_StatusCode SOPC_MonitoredAlarmCall_AddComment(SOPC_MonitoredAlarm* alarmCond, SOPC_LocalizedText* comment)
{
    return call_request_with_params_eventId_and_comment(alarmCond, comment, addCommentMethodId, "AddComment");
}

SOPC_StatusCode SOPC_MonitoredAlarmCall_Refresh(const SOPC_MonitoredAlarmsGroup* group)
{
    // Check params
    if (NULL == group)
    {
        return OpcUa_BadInvalidArgument;
    }
    SOPC_ASSERT(NULL != group->subscription);

    SOPC_StatusCode returnSc = OpcUa_BadInternalError;
    uint32_t subscriptionId = 0;
    SOPC_ReturnStatus status = SOPC_ClientHelper_GetSubscriptionId(group->subscription, &subscriptionId);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    uint32_t MIid = group->monitoredItemId;

    // Try ConditionRefresh2
    bool tryConditionRefresh = false;
    // Get the SC of the client
    SOPC_ClientConnection* secureConnection = SOPC_ClientHelper_GetSecureConnection(group->subscription);
    SOPC_ASSERT(NULL != secureConnection);

    // Prepare CallRequest
    SOPC_Variant* inputParameters = NULL;
    OpcUa_CallResponse* callResponse = NULL;
    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    if (NULL != callRequest)
    {
        inputParameters = SOPC_Calloc(2, sizeof(SOPC_Variant));
        if (NULL == inputParameters)
        {
            SOPC_EncodeableObject_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &callRequest);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        inputParameters[0].BuiltInTypeId = SOPC_UInt32_Id; /* IntegerId */
        inputParameters[0].ArrayType = SOPC_VariantArrayType_SingleValue;
        inputParameters[0].Value.Uint32 = subscriptionId;
        inputParameters[1].BuiltInTypeId = SOPC_UInt32_Id;
        inputParameters[1].ArrayType = SOPC_VariantArrayType_SingleValue;
        inputParameters[1].Value.Uint32 = MIid;
        status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, conditionTypeIdStr,
                                                             conditionRefresh2MethodId, 2, inputParameters);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, callRequest, (void**) &callResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        returnSc = check_call_response_generic(callResponse, NULL, "ConditionRefresh2");
        if (OpcUa_BadMethodInvalid == returnSc)
        {
            // Try ConditionRefresh if OPC UA method returned Bad_MethodInvalid,.
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "A&C CLIENT: Method ConditionRefresh2 on subscription with id %" PRIu32
                                     " failed. Trying "
                                     "ConditionRefresh method instead.",
                                     subscriptionId);
            tryConditionRefresh = true;
        }
        SOPC_EncodeableObject_Delete(callResponse->encodeableType, (void**) &callResponse);
    }
    else
    {
        // Also try ConditionRefresh if we failed before (at creation / sending the request)
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "A&C CLIENT: Fail at creation or at sending the method call request ConditionRefresh2 to the server.");
        tryConditionRefresh = true;
    }

    if (tryConditionRefresh)
    {
        // Clean 2nd parameter of input parameters only
        SOPC_Variant_Delete(inputParameters);
        callRequest = SOPC_CallRequest_Create(1);
        if (NULL != callRequest)
        {
            inputParameters = SOPC_Calloc(1, sizeof(SOPC_Variant));
            if (NULL == inputParameters)
            {
                SOPC_EncodeableObject_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &callRequest);
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }

        if (SOPC_STATUS_OK == status)
        {
            inputParameters[0].BuiltInTypeId = SOPC_UInt32_Id; /* IntegerId */
            inputParameters[0].ArrayType = SOPC_VariantArrayType_SingleValue;
            inputParameters[0].Value.Uint32 = subscriptionId;
            status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, conditionTypeIdStr,
                                                                 conditionRefreshMethodId, 1, inputParameters);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ClientHelper_ServiceSync(secureConnection, callRequest, (void**) &callResponse);
        }

        if (SOPC_STATUS_OK == status)
        {
            returnSc = check_call_response_generic(callResponse, NULL, "ConditionRefresh");
            SOPC_EncodeableObject_Delete(callResponse->encodeableType, (void**) &callResponse);
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "A&C CLIENT: Fail at creation or at sending the method call request ConditionRefresh to the server.");
        }
    }

    SOPC_Variant_Delete(inputParameters);
    return returnSc;
}

SOPC_StatusCode SOPC_MonitoredAlarmCall_EnableFromId(const SOPC_MonitoredAlarmsGroup* group,
                                                     const SOPC_NodeId* conditionId,
                                                     bool enable)
{
    // Check params
    if (NULL == group || NULL == conditionId)
    {
        return OpcUa_BadInvalidArgument;
    }

    SOPC_StatusCode returnSc = OpcUa_BadInternalError;
    // Get the SC of the client
    SOPC_ClientConnection* secureConnection = SOPC_ClientHelper_GetSecureConnection(group->subscription);
    SOPC_ASSERT(NULL != secureConnection);

    // Prepare CallRequest
    OpcUa_CallResponse* callResponse = NULL;
    OpcUa_CallRequest* callRequest = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = (NULL != callRequest) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        char* conditionIdStr = SOPC_NodeId_ToCString(conditionId);
        status = SOPC_CallRequest_SetMethodToCallFromStrings(callRequest, 0, conditionIdStr,
                                                             enable ? enableMethodId : disableMethodId, 0, NULL);
        SOPC_Free(conditionIdStr);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, callRequest, (void**) &callResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        returnSc = check_call_response_generic(callResponse, NULL, enable ? "Enable" : "Disable");
        SOPC_EncodeableObject_Delete(callResponse->encodeableType, (void**) &callResponse);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "A&C CLIENT: Fail at creation or at sending the method call request %s "
                               "to the server",
                               enable ? "Enable" : "Disable");
    }
    return returnSc;
}

const SOPC_NodeId* SOPC_MonitoredAlarm_GetConditionId(SOPC_MonitoredAlarm* alarmCond)
{
    if (!check_parameter_alarm(alarmCond))
    {
        return NULL;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const SOPC_NodeId* conditionId = SOPC_Event_GetNodeId(alarmCond->lastEvent);
    mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return conditionId;
}

const SOPC_ByteString* SOPC_MonitoredAlarm_GetLastEventId(SOPC_MonitoredAlarm* alarmCond)
{
    if (!check_parameter_alarm(alarmCond))
    {
        return NULL;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const SOPC_ByteString* eventId = SOPC_Event_GetEventId(alarmCond->lastEvent);
    mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return eventId;
}

static SOPC_ReturnStatus alarm_getState_generic(SOPC_MonitoredAlarm* alarmCond, bool* state, const char* qnStateId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    const SOPC_Event* currentState = alarmCond->lastEvent;
    SOPC_ASSERT(NULL != currentState);
    const SOPC_Variant* stateVariableValue = SOPC_Event_GetVariableFromStrPath(currentState, qnStateId);
    if (NULL != stateVariableValue && SOPC_Boolean_Id == stateVariableValue->BuiltInTypeId)
    {
        *state = stateVariableValue->Value.Boolean;
        status = SOPC_STATUS_OK;
    }
    mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_MonitoredAlarm_GetEnabledState(SOPC_MonitoredAlarm* alarmCond, bool* state)
{
    if (!check_parameter_alarm(alarmCond) || NULL == state)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return alarm_getState_generic(alarmCond, state, qnEnabledStateId);
}

SOPC_ReturnStatus SOPC_MonitoredAlarm_GetAckedState(SOPC_MonitoredAlarm* alarmCond, bool* state)
{
    if (!check_parameter_alarm(alarmCond) || NULL == state)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return alarm_getState_generic(alarmCond, state, qnAckedStateId);
}

SOPC_ReturnStatus SOPC_MonitoredAlarm_GetConfirmedState(SOPC_MonitoredAlarm* alarmCond, bool* state)
{
    if (!check_parameter_alarm(alarmCond) || NULL == state)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return alarm_getState_generic(alarmCond, state, qnConfirmedStateId);
}

SOPC_ReturnStatus SOPC_MonitoredAlarm_GetActiveState(SOPC_MonitoredAlarm* alarmCond, bool* state)
{
    if (!check_parameter_alarm(alarmCond) || NULL == state)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return alarm_getState_generic(alarmCond, state, qnActiveStateId);
}

SOPC_Event* SOPC_MonitoredAlarm_GetLastEvent(SOPC_MonitoredAlarm* alarmCond)
{
    if (!check_parameter_alarm(alarmCond))
    {
        return NULL;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Event* lastEvent = SOPC_Event_CreateCopy(alarmCond->lastEvent, true);
    mutStatus = SOPC_Mutex_Unlock(&alarmCond->eventMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return lastEvent;
}

SOPC_MonitoredAlarmsGroup* SOPC_MonitoredAlarm_GetGroup(SOPC_MonitoredAlarm* alarmCond)
{
    if (NULL != alarmCond)
    {
        return alarmCond->alarmGroup;
    }
    return NULL;
}
