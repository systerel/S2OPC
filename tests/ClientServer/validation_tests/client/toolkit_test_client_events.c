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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_assert.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "sopc_helper_askpass.h"

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 200;
// Loop timeout in milliseconds
static const uint32_t loopTimeout = 10000;
static const uint32_t failureLoopTimeout = 3000;

static const char* testObjectNodeIdStr = "ns=1;s=TestObject";
static const char* genEventMethodIdStr = "ns=1;s=GenEventMethod";

static const SOPC_NodeId serverObjectId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server);
static const SOPC_NodeId baseEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_BaseEventType);
static const SOPC_NodeId conditionTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_ConditionType);
static const SOPC_NodeId alarmConditionTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AlarmConditionType);
static const SOPC_NodeId baseModelChangeEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_BaseModelChangeEventType);

#define NB_MONITORED_ITEMS 2
static const char* monitoredItemNames[NB_MONITORED_ITEMS] = {"TestObject", "Server"};
static uint32_t nbMIs = 0;

#define NB_SELECT_CLAUSES_IN_ARRAY 6
#define NB_SELECT_CLAUSES 4 + NB_SELECT_CLAUSES_IN_ARRAY
// selectClause[0]
static const SOPC_QualifiedName qnEventId = {0, {sizeof("EventId") - 1, 1, (SOPC_Byte*) "EventId"}};
// selectClause[1]
#define CONDITION_TYPE_FIELD_IDX 1
static const SOPC_QualifiedName qnConditionName = {0, {sizeof("ConditionName") - 1, 1, (SOPC_Byte*) "ConditionName"}};
// selectClause[2] is invalid path for EventType and selectClause[3] is invalid attribute NodeId
// selectClause[4..9] are valid paths (unchecked due to base event type selected)
#define EVENT_TYPE_FIELD_IDX 4
#define ALARM_CONDITION_TYPE_FIELD_IDX 9
static const char* selectClauses_4[NB_SELECT_CLAUSES - NB_SELECT_CLAUSES_IN_ARRAY] = {
    "0:EventId", "0:ConditionName", "invalid 0:ConditionName", "invalid NodeId attribute"}; // For traces only
static const char* selectClauses_6[NB_SELECT_CLAUSES_IN_ARRAY] = {
    "0:EventType", "0:Message", "0:Severity", "0:SourceName", "0:Time", "0:ShelvingState~0:CurrentState"};
static const SOPC_StatusCode selectClausesResult[NB_SELECT_CLAUSES] = {
    SOPC_GoodGenericStatus, SOPC_GoodGenericStatus, OpcUa_BadNodeIdUnknown, OpcUa_BadAttributeIdInvalid,
    SOPC_GoodGenericStatus, SOPC_GoodGenericStatus, SOPC_GoodGenericStatus, SOPC_GoodGenericStatus,
    SOPC_GoodGenericStatus, SOPC_GoodGenericStatus,
};

static bool notifEventFieldSetResult[NB_SELECT_CLAUSES] = {
    true, false, false, false, true, true, true, true, true, false,
};

#define NB_SELECT_CLAUSES_INDEX_RANGE 3
/*
 * 1. Invalid for variable type
 * 2. Valid index range
 * 3. Invalid syntax
 */
static const char* selectClausesIndexRange[NB_SELECT_CLAUSES_INDEX_RANGE] = {"0:EventType", "0:SourceName",
                                                                             "0:SourceName"};
static const char* selectClausesIndexRangeValue[NB_SELECT_CLAUSES_INDEX_RANGE] = {"1:2", "3:4", "3;4"};
static const SOPC_StatusCode selectClausesIndexRangeResult[NB_SELECT_CLAUSES_INDEX_RANGE] = {
    OpcUa_BadTypeMismatch, SOPC_GoodGenericStatus, OpcUa_BadIndexRangeInvalid};
static const bool notifEventFieldSetIndexRangeResult[NB_SELECT_CLAUSES_INDEX_RANGE] = {false, true, false};

static bool eventNotifReceived = false;
static bool eventNotifReceived2 = false;
static bool expectedContentReceived = false;
static bool expectedContentReceived2 = false;

static bool expectOverflow = false;
static bool overflowReceived = false;
static const uint32_t nbEventsBeforeOverflow = SOPC_MIN_EVENT_NOTIFICATION_QUEUE_SIZE;
static uint32_t nbEventsReceived = 0;
static bool expNbEventsReceivedWhenOverflow = false;

static bool is_eventQueueOverflowTypeId(SOPC_Variant* var)
{
    return (SOPC_NodeId_Id == var->BuiltInTypeId && SOPC_IdentifierType_Numeric == var->Value.NodeId->IdentifierType &&
            0 == var->Value.NodeId->Namespace &&
            OpcUaId_EventQueueOverflowEventType == var->Value.NodeId->Data.Numeric);
}

static void SOPC_Client_SubscriptionNotification_Cb(const SOPC_ClientHelper_Subscription* subscription,
                                                    SOPC_StatusCode status,
                                                    SOPC_EncodeableType* notificationType,
                                                    uint32_t nbNotifElts,
                                                    const void* notification,
                                                    uintptr_t* monitoredItemCtxArray)
{
    uintptr_t userCtx = SOPC_ClientHelper_Subscription_GetUserParam(subscription);
    const char* currentSelectClause = NULL;
    SOPC_ASSERT(12 == userCtx);
    if (&OpcUa_EventNotificationList_EncodeableType == notificationType)
    {
        if (SOPC_IsGoodStatus(status))
        {
            const OpcUa_EventNotificationList* eventList = (const OpcUa_EventNotificationList*) notification;
            bool localExpectedContentReceived = ((int32_t) nbNotifElts == eventList->NoOfEvents);
            const OpcUa_EventFieldList* event = NULL;
            SOPC_CONSOLE_PRINTF("\nEvent notification values:\n");
            for (int32_t i = 0; localExpectedContentReceived && i < eventList->NoOfEvents; i++)
            {
                SOPC_CONSOLE_PRINTF("\n-------------------------\nEventNotif[%" PRIi32
                                    "]('%s'):\n-------------------------",
                                    i, (const char*) monitoredItemCtxArray[i]);
                event = &eventList->Events[i];
                localExpectedContentReceived = (NB_SELECT_CLAUSES == event->NoOfEventFields);
                // all clauses present case
                if (localExpectedContentReceived)
                {
                    for (int32_t j = 0; j < event->NoOfEventFields; j++)
                    {
                        currentSelectClause =
                            (j < 4 ? selectClauses_4[j]
                                   : selectClauses_6[j - (NB_SELECT_CLAUSES - NB_SELECT_CLAUSES_IN_ARRAY)]);
                        SOPC_CONSOLE_PRINTF("\nEventField[%" PRIi32 "]('%s'):\n", j, currentSelectClause);
                        SOPC_Variant_Print(&event->EventFields[j]);
                        // Check if event field is NULL only when expected not to be set
                        if (!(notifEventFieldSetResult[j] == (event->EventFields[j].BuiltInTypeId != SOPC_Null_Id)) &&
                            (!expectOverflow || CONDITION_TYPE_FIELD_IDX != j))
                        {
                            localExpectedContentReceived = false;
                            SOPC_CONSOLE_PRINTF(
                                "ERROR: unexpected event field [idx=%" PRIi32 "] state (set/unset): %s instead of %s\n",
                                j, (event->EventFields[j].BuiltInTypeId != SOPC_Null_Id ? "SET" : "UNSET"),
                                (notifEventFieldSetResult[j] ? "SET" : "UNSET"));
                        }
                    }
                }
                // index range clauses present case
                else if (NB_SELECT_CLAUSES_INDEX_RANGE == event->NoOfEventFields)
                {
                    localExpectedContentReceived = true;
                    for (int32_t j = 0; j < event->NoOfEventFields; j++)
                    {
                        currentSelectClause = selectClausesIndexRange[j];
                        SOPC_CONSOLE_PRINTF("\nEventField[%" PRIi32 "]('%s'):\n", j, currentSelectClause);
                        SOPC_Variant_Print(&event->EventFields[j]);
                        // Check if event field is NULL only when expected not to be set
                        if (!(notifEventFieldSetIndexRangeResult[j] ==
                              (event->EventFields[j].BuiltInTypeId != SOPC_Null_Id)))
                        {
                            localExpectedContentReceived = false;
                            SOPC_CONSOLE_PRINTF(
                                "ERROR: unexpected event field [idx=%" PRIi32 "] state (set/unset): %s instead of %s\n",
                                j, (event->EventFields[j].BuiltInTypeId != SOPC_Null_Id ? "SET" : "UNSET"),
                                (notifEventFieldSetResult[j] ? "SET" : "UNSET"));
                        }
                    }
                }
                else
                {
                    SOPC_CONSOLE_PRINTF("ERROR: unexpected number of event field: %" PRIi32 " instead of %d\n",
                                        event->NoOfEventFields, (int) NB_SELECT_CLAUSES);
                }

                if (localExpectedContentReceived)
                {
                    if (monitoredItemNames[0] == (char*) monitoredItemCtxArray[i])
                    {
                        // Nominal case for first MI: record received flag + expected content flag
                        if (!expectOverflow)
                        {
                            expectedContentReceived = localExpectedContentReceived;
                            eventNotifReceived = true;
                        }
                        // EventQueueOverflowCase case for first MI: record overflow Event and number of events received
                        else if (0 == nbEventsReceived &&
                                 is_eventQueueOverflowTypeId(&event->EventFields[EVENT_TYPE_FIELD_IDX]))
                        {
                            overflowReceived = true;
                            nbEventsReceived++;
                        }
                        // EventQueueOverflowCase case for first MI: record number of events received which are not
                        // overflow event
                        else if (!is_eventQueueOverflowTypeId(&event->EventFields[EVENT_TYPE_FIELD_IDX]))
                        {
                            nbEventsReceived++;
                        }
                    }
                    else if (monitoredItemNames[1] == (char*) monitoredItemCtxArray[i])
                    {
                        // Nominal case for second MI: record received flag + expected content flag
                        expectedContentReceived2 = localExpectedContentReceived;
                        eventNotifReceived2 = true;
                    }
                }
            }
            expNbEventsReceivedWhenOverflow = (nbEventsBeforeOverflow == nbEventsReceived);
        }
    }
}

// Connection event callback (only for unexpected events)
static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    SOPC_ASSERT(false && "Unexpected connection event");
}

static SOPC_ReturnStatus Client_LoadClientConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    /* Retrieve XML configuration file path from environment variables TEST_CLIENT_XML_CONFIG,
     *
     * In case of success returns the file path.
     */

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const char* xml_client_config_path = getenv("TEST_CLIENT_XML_CONFIG");

    if (NULL != xml_client_config_path)
    {
        status = SOPC_ClientConfigHelper_ConfigureFromXML(xml_client_config_path, NULL, nbSecConnCfgs,
                                                          secureConnConfigArray);
    }
    else
    {
        printf("Error: TEST_CLIENT_XML_CONFIG environment variable is not defined !\n");
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("Error: an XML client configuration file path provided parsing failed\n");
    }

    // Set callback necessary to retrieve client key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit_Events: Client configured\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit_Events: Client configuration failed\n");
    }

    return status;
}

static SOPC_ClientHelper_Subscription* create_subscription(SOPC_ClientConnection* connection)
{
    OpcUa_CreateSubscriptionRequest* createSubReq = SOPC_CreateSubscriptionRequest_Create(500, 6, 2, 1000, true, 0);
    if (NULL == createSubReq)
    {
        return NULL;
    }
    /* In case the subscription service shall not be supported, check service response is unsupported service*/
    SOPC_ClientHelper_Subscription* subscription =
        SOPC_ClientHelper_CreateSubscription(connection, createSubReq, SOPC_Client_SubscriptionNotification_Cb, 12);
    return subscription;
}

static OpcUa_CreateMonitoredItemsResponse* create_monitored_item_event(SOPC_ClientHelper_Subscription* subscription,
                                                                       bool isTestObject,
                                                                       uint32_t queueSizeRequested,
                                                                       const SOPC_NodeId* whereTypeId,
                                                                       bool validSelectClausesOnly,
                                                                       bool invalidSelectClausesOnly)
{
    SOPC_ASSERT(NULL != subscription);
    SOPC_ASSERT(nbMIs < NB_MONITORED_ITEMS);
    size_t initSelectArray = 4;
    size_t nbSelectClauses = NB_SELECT_CLAUSES;
    if (validSelectClausesOnly)
    {
        initSelectArray = 0;
        nbSelectClauses = NB_SELECT_CLAUSES_IN_ARRAY;
    }
    else if (invalidSelectClausesOnly)
    {
        nbSelectClauses = 2;
    }

    OpcUa_CreateMonitoredItemsRequest* createMonItReq =
        SOPC_CreateMonitoredItemsRequest_Create(0, 1, OpcUa_TimestampsToReturn_Both);
    if (NULL == createMonItReq)
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (isTestObject)
    {
        // TestObject shall always be the first MI since we use monitoredItemNames[0] == "TestObject" to determine it is
        // the first MI in SOPC_Client_SubscriptionNotification_Cb
        SOPC_ASSERT(0 == nbMIs);
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemIdFromStrings(createMonItReq, 0, testObjectNodeIdStr,
                                                                                SOPC_AttributeId_EventNotifier, NULL);
    }
    else
    {
        // Server shall always be the second MI since we use monitoredItemNames[1] == "Server" to determine it is
        // the second MI in SOPC_Client_SubscriptionNotification_Cb
        SOPC_ASSERT(1 == nbMIs);
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(createMonItReq, 0, &serverObjectId,
                                                                     SOPC_AttributeId_EventNotifier, NULL);
    }

    OpcUa_EventFilter* eventFilter = NULL;
    if (SOPC_STATUS_OK == status)
    {
        size_t nbWhereClauseElts = 0;
        if (whereTypeId != NULL)
        {
            nbWhereClauseElts = 1;
        }
        eventFilter = SOPC_MonitoredItem_CreateEventFilter(nbSelectClauses, nbWhereClauseElts);
    }
    if (NULL != eventFilter)
    {
        if (NULL != whereTypeId)
        {
            status = SOPC_EventFilter_SetOfTypeWhereClause(eventFilter, 0, whereTypeId);
        }
        if (!validSelectClausesOnly && !invalidSelectClausesOnly)
        {
            /* select clause 0: eventId browse path with base event type */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EventFilter_SetSelectClause(eventFilter, 0, &baseEventTypeId, 1, &qnEventId,
                                                          SOPC_AttributeId_Value, NULL);
            }
            /* select clause 1: specific type of event */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EventFilter_SetSelectClause(eventFilter, 1, &conditionTypeId, 1, &qnConditionName,
                                                          SOPC_AttributeId_Value, NULL);
            }
            /* select clause 2: specific type of event with invalid path */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EventFilter_SetSelectClause(eventFilter, 2, &baseModelChangeEventTypeId, 1,
                                                          &qnConditionName, SOPC_AttributeId_Value, NULL);
            }
            /* select clause 3: node id attribute */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EventFilter_SetSelectClauseFromStringPath(eventFilter, 3, NULL, '~', "",
                                                                        SOPC_AttributeId_NodeId, NULL);
            }
        }
        /* 3-N select clauses: browses path as a string + NULL type id (base event type) */
        for (size_t i = initSelectArray; !invalidSelectClausesOnly && SOPC_STATUS_OK == status && i < nbSelectClauses;
             i++)
        {
            status = SOPC_EventFilter_SetSelectClauseFromStringPath(
                eventFilter, i, NULL, '~', selectClauses_6[i - initSelectArray], SOPC_AttributeId_Value, NULL);
        }

        if (invalidSelectClausesOnly)
        {
            /* select clause 0: specific type of event with invalid path */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EventFilter_SetSelectClause(eventFilter, 0, &baseModelChangeEventTypeId, 1,
                                                          &qnConditionName, SOPC_AttributeId_Value, NULL);
            }
            /* select clause 1: node id attribute */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_EventFilter_SetSelectClauseFromStringPath(eventFilter, 1, NULL, '~', "",
                                                                        SOPC_AttributeId_NodeId, NULL);
            }
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ExtensionObject* extObj = SOPC_MonitoredItem_EventFilter(eventFilter);
        SOPC_ASSERT(NULL != extObj);
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
            createMonItReq, 0, OpcUa_MonitoringMode_Reporting, 0, -1, extObj, queueSizeRequested, true);
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
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
            subscription, createMonItReq, (const uintptr_t*) &monitoredItemNames[nbMIs], createMonItResp);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, (void**) &createMonItResp);
        createMonItResp = NULL;
    }
    return createMonItResp;
}

static OpcUa_CreateMonitoredItemsResponse* create_monitored_item_event_index_ranges(
    SOPC_ClientHelper_Subscription* subscription)
{
    SOPC_ASSERT(NULL != subscription);
    SOPC_ASSERT(nbMIs < NB_MONITORED_ITEMS);
    size_t nbSelectClauses = NB_SELECT_CLAUSES_INDEX_RANGE;
    size_t nbWhereClauseElts = 0;

    OpcUa_CreateMonitoredItemsRequest* createMonItReq =
        SOPC_CreateMonitoredItemsRequest_Create(0, 1, OpcUa_TimestampsToReturn_Both);
    if (NULL == createMonItReq)
    {
        return NULL;
    }

    // Server shall always be the second MI since we use monitoredItemNames[1] == "Server" to determine it is
    // the second MI in SOPC_Client_SubscriptionNotification_Cb
    SOPC_ASSERT(1 == nbMIs);
    SOPC_ReturnStatus status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(
        createMonItReq, 0, &serverObjectId, SOPC_AttributeId_EventNotifier, NULL);

    OpcUa_EventFilter* eventFilter = NULL;
    if (SOPC_STATUS_OK == status)
    {
        eventFilter = SOPC_MonitoredItem_CreateEventFilter(nbSelectClauses, nbWhereClauseElts);
    }
    if (NULL != eventFilter)
    {
        for (size_t i = 0; SOPC_STATUS_OK == status && i < nbSelectClauses; i++)
        {
            status =
                SOPC_EventFilter_SetSelectClauseFromStringPath(eventFilter, i, NULL, '~', selectClausesIndexRange[i],
                                                               SOPC_AttributeId_Value, selectClausesIndexRangeValue[i]);
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ExtensionObject* extObj = SOPC_MonitoredItem_EventFilter(eventFilter);
        SOPC_ASSERT(NULL != extObj);
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
            createMonItReq, 0, OpcUa_MonitoringMode_Reporting, 0, -1, extObj, 0, true);
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
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
            subscription, createMonItReq, (const uintptr_t*) &monitoredItemNames[nbMIs], createMonItResp);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(createMonItResp);
        createMonItResp = NULL;
    }
    return createMonItResp;
}

static SOPC_ReturnStatus check_mi_resp_header(OpcUa_CreateMonitoredItemsResponse* createMonItResp)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (SOPC_IsGoodStatus(createMonItResp->ResponseHeader.ServiceResult))
    {
        status = SOPC_STATUS_OK;
        if (createMonItResp->NoOfResults == 1)
        {
            if (!SOPC_IsGoodStatus(createMonItResp->Results[0].StatusCode))
            {
                printf(">>Test_Client_Toolkit_Events: CreateMonitoredItemsResponse[%" PRIi32
                       "] result not good: 0x%08" PRIX32 "\n",
                       0, createMonItResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
            else
            {
                nbMIs++;
            }
        }
        else
        {
            printf(">>Test_Client_Toolkit_Events: Unexpected number of MI in CreateMonitoredItemsResponse %" PRIi32
                   " != 1\n",
                   createMonItResp->NoOfResults);
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        printf(">>Test_Client_Toolkit_Events: CreateMonitoredItemsResponse global result not good: 0x%08" PRIX32 "\n",
               createMonItResp->ResponseHeader.ServiceResult);
    }
    return status;
}

static SOPC_ReturnStatus check_monitored_items(OpcUa_CreateMonitoredItemsResponse* createMonItResp,
                                               uint32_t queueSizeRevised)
{
    SOPC_ReturnStatus status = check_mi_resp_header(createMonItResp);
    if (SOPC_STATUS_OK == status)
    {
        if (queueSizeRevised != createMonItResp->Results[0].RevisedQueueSize)
        {
            printf(">>Test_Client_Toolkit_Events: Unexpected revised queue size: %" PRIu32 " instead of %" PRIu32 "\n",
                   queueSizeRevised, createMonItResp->Results[0].RevisedQueueSize);
            status = SOPC_STATUS_NOK;
        }

        SOPC_ReturnStatus filterStatus = SOPC_STATUS_OK;
        if (SOPC_ExtObjBodyEncoding_Object != createMonItResp->Results[0].FilterResult.Encoding ||
            &OpcUa_EventFilterResult_EncodeableType != createMonItResp->Results[0].FilterResult.Body.Object.ObjType)
        {
            char* encTypId = SOPC_NodeId_ToCString(&createMonItResp->Results[0].FilterResult.TypeId.NodeId);
            printf(
                ">>Test_Client_Toolkit_Events: Unexpected CreateMonitoredItemsResponse[0] filter result type: "
                "%s\n",
                encTypId);
            SOPC_Free(encTypId);
            filterStatus = SOPC_STATUS_NOK;
        }

        OpcUa_EventFilterResult* filterResult = NULL;
        if (SOPC_STATUS_OK == filterStatus)
        {
            filterResult = (OpcUa_EventFilterResult*) createMonItResp->Results[0].FilterResult.Body.Object.Value;
        }

        // Check where clause elements result
        for (int32_t i = 0; SOPC_STATUS_OK == filterStatus && i < filterResult->WhereClauseResult.NoOfElementResults;
             i++)
        {
            OpcUa_ContentFilterElementResult* whereEltRes = &filterResult->WhereClauseResult.ElementResults[i];
            if (!SOPC_IsGoodStatus(whereEltRes->StatusCode))
            {
                printf(
                    ">>Test_Client_Toolkit_Events: "
                    "filterResult->WhereClauseResult.ElementResults[%" PRIi32 "] result unexpected: 0x%08" PRIX32 "\n",
                    i, whereEltRes->StatusCode);
                filterStatus = SOPC_STATUS_NOK;
            }
            else
            {
                OpcUa_ContentFilterElementResult* contentFilterElt = &filterResult->WhereClauseResult.ElementResults[i];
                for (int32_t j = 0; j < contentFilterElt->NoOfOperandStatusCodes; j++)
                {
                    if (!SOPC_IsGoodStatus(contentFilterElt->OperandStatusCodes[j]))
                    {
                        printf(
                            ">>Test_Client_Toolkit_Events: "
                            "filterResult->WhereClauseResult.ElementResults[%" PRIi32 "].OperandStatusCodes[%" PRIi32
                            "] result unexpected: 0x%08" PRIX32 "\n",
                            i, j, contentFilterElt->OperandStatusCodes[j]);
                        filterStatus = SOPC_STATUS_NOK;
                    }
                }
            }
        }

        // Check select clauses result
        if (SOPC_STATUS_OK == filterStatus)
        {
            if (filterResult->NoOfSelectClauseResults != NB_SELECT_CLAUSES)
            {
                filterStatus = SOPC_STATUS_NOK;
            }
            else
            {
                for (int32_t i = 0; i < filterResult->NoOfSelectClauseResults; i++)
                {
                    if (selectClausesResult[i] != filterResult->SelectClauseResults[i])
                    {
                        printf(
                            ">>Test_Client_Toolkit_Events: "
                            "CreateMonitoredItemsResponse[0].FilterResult.SelectClauseResult[%" PRIi32
                            "] result unexpected: 0x%08" PRIX32 " (expected: 0x%08" PRIX32 ")\n",
                            i, filterResult->SelectClauseResults[i], selectClausesResult[i]);
                        filterStatus = SOPC_STATUS_NOK;
                    }
                }
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = filterStatus;
        }
    }
    return status;
}

static SOPC_ReturnStatus check_monitored_items_index_ranges(OpcUa_CreateMonitoredItemsResponse* createMonItResp)
{
    SOPC_ReturnStatus status = check_mi_resp_header(createMonItResp);

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ReturnStatus filterStatus = SOPC_STATUS_OK;
        if (SOPC_ExtObjBodyEncoding_Object != createMonItResp->Results[0].FilterResult.Encoding ||
            &OpcUa_EventFilterResult_EncodeableType != createMonItResp->Results[0].FilterResult.Body.Object.ObjType)
        {
            char* encTypId = SOPC_NodeId_ToCString(&createMonItResp->Results[0].FilterResult.TypeId.NodeId);
            printf(
                ">>Test_Client_Toolkit_Events: Unexpected CreateMonitoredItemsResponse[0] filter result type: "
                "%s\n",
                encTypId);
            SOPC_Free(encTypId);
            filterStatus = SOPC_STATUS_NOK;
        }

        OpcUa_EventFilterResult* filterResult = NULL;
        if (SOPC_STATUS_OK == filterStatus)
        {
            filterResult = (OpcUa_EventFilterResult*) createMonItResp->Results[0].FilterResult.Body.Object.Value;
        }
        // Check select clauses result
        if (SOPC_STATUS_OK == filterStatus)
        {
            for (int32_t i = 0; i < filterResult->NoOfSelectClauseResults; i++)
            {
                if (filterResult->NoOfSelectClauseResults != NB_SELECT_CLAUSES_INDEX_RANGE)
                {
                    filterStatus = SOPC_STATUS_NOK;
                }
                else
                {
                    if (selectClausesIndexRangeResult[i] != filterResult->SelectClauseResults[i])
                    {
                        printf(
                            ">>Test_Client_Toolkit_Events: "
                            "CreateMonitoredItemsResponse[0].FilterResult.SelectClauseResult[%" PRIi32
                            "] result unexpected: 0x%08" PRIX32 " (expected: 0x%08" PRIX32 ")\n",
                            i, filterResult->SelectClauseResults[i], selectClausesIndexRangeResult[i]);
                        filterStatus = SOPC_STATUS_NOK;
                    }
                }
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = filterStatus;
        }
    }
    return status;
}

static SOPC_ReturnStatus check_monitored_items_special_cases(OpcUa_CreateMonitoredItemsResponse* createMonItResp,
                                                             uint32_t queueSizeRevised,
                                                             bool expectSuccess,
                                                             bool validWhereClauseExp,
                                                             bool validSelectClausesOnly)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool deleteResp = true;
    if (SOPC_IsGoodStatus(createMonItResp->ResponseHeader.ServiceResult))
    {
        status = SOPC_STATUS_OK;
        if (createMonItResp->NoOfResults == 1)
        {
            if (expectSuccess && !SOPC_IsGoodStatus(createMonItResp->Results[0].StatusCode))
            {
                printf(">>Test_Client_Toolkit_Events: CreateMonitoredItemsResponse[%" PRIi32
                       "] result not good: 0x%08" PRIX32 "\n",
                       0, createMonItResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
            else if (!expectSuccess)
            {
                if (OpcUa_BadMonitoredItemFilterInvalid != createMonItResp->Results[0].StatusCode &&
                    OpcUa_BadMonitoredItemFilterUnsupported != createMonItResp->Results[0].StatusCode)
                {
                    printf(">>Test_Client_Toolkit_Events: CreateMonitoredItemsResponse[%" PRIi32
                           "] result not bad or unexpected code: "
                           "0x%08" PRIX32 "\n",
                           0, createMonItResp->Results[0].StatusCode);
                }
            }
            else
            {
                deleteResp = false; // resp needed to call delete MI
                nbMIs++;
            }
        }
        else
        {
            printf(">>Test_Client_Toolkit_Events: Unexpected number of MI in CreateMonitoredItemsResponse %" PRIi32
                   " != 1\n",
                   createMonItResp->NoOfResults);
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        printf(">>Test_Client_Toolkit_Events: CreateMonitoredItemsResponse global result not good: 0x%08" PRIX32 "\n",
               createMonItResp->ResponseHeader.ServiceResult);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (queueSizeRevised != createMonItResp->Results[0].RevisedQueueSize)
        {
            printf(">>Test_Client_Toolkit_Events: Unexpected revised queue size: %" PRIu32 " instead of %" PRIu32 "\n",
                   queueSizeRevised, createMonItResp->Results[0].RevisedQueueSize);
            status = SOPC_STATUS_NOK;
        }

        SOPC_ReturnStatus filterStatus = SOPC_STATUS_OK;
        if (validSelectClausesOnly && validWhereClauseExp)
        {
            if (SOPC_ExtObjBodyEncoding_None != createMonItResp->Results[0].FilterResult.Encoding)
            {
                printf(
                    ">>Test_Client_Toolkit_Events: Unexpected CreateMonitoredItemsResponse[0] filter non-empty "
                    "result "
                    "when all clauses are valid\n");
                filterStatus = SOPC_STATUS_NOK;
            }
        }
        else
        {
            if (SOPC_ExtObjBodyEncoding_Object != createMonItResp->Results[0].FilterResult.Encoding ||
                &OpcUa_EventFilterResult_EncodeableType != createMonItResp->Results[0].FilterResult.Body.Object.ObjType)
            {
                char* encTypId = SOPC_NodeId_ToCString(&createMonItResp->Results[0].FilterResult.TypeId.NodeId);
                printf(
                    ">>Test_Client_Toolkit_Events: Unexpected CreateMonitoredItemsResponse[0] filter result type: "
                    "%s\n",
                    encTypId);
                SOPC_Free(encTypId);
                filterStatus = SOPC_STATUS_NOK;
            }

            OpcUa_EventFilterResult* filterResult = NULL;
            if (SOPC_STATUS_OK == filterStatus)
            {
                filterResult = (OpcUa_EventFilterResult*) createMonItResp->Results[0].FilterResult.Body.Object.Value;
            }

            // Check where clause elements result
            for (int32_t i = 0;
                 SOPC_STATUS_OK == filterStatus && i < filterResult->WhereClauseResult.NoOfElementResults; i++)
            {
                OpcUa_ContentFilterElementResult* whereEltRes = &filterResult->WhereClauseResult.ElementResults[i];
                if (!SOPC_IsGoodStatus(whereEltRes->StatusCode))
                {
                    if (validWhereClauseExp)
                    {
                        printf(
                            ">>Test_Client_Toolkit_Events: "
                            "filterResult->WhereClauseResult.ElementResults[%" PRIi32
                            "] result unexpected: 0x%08" PRIX32 "\n",
                            i, whereEltRes->StatusCode);
                        filterStatus = SOPC_STATUS_NOK;
                    }
                }
                else
                {
                    OpcUa_ContentFilterElementResult* contentFilterElt =
                        &filterResult->WhereClauseResult.ElementResults[i];
                    for (int32_t j = 0; j < contentFilterElt->NoOfOperandStatusCodes; j++)
                    {
                        if (validWhereClauseExp && !SOPC_IsGoodStatus(contentFilterElt->OperandStatusCodes[j]))
                        {
                            printf(
                                ">>Test_Client_Toolkit_Events: "
                                "filterResult->WhereClauseResult.ElementResults[%" PRIi32
                                "].OperandStatusCodes[%" PRIi32 "] result unexpected: 0x%08" PRIX32 "\n",
                                i, j, contentFilterElt->OperandStatusCodes[j]);
                            filterStatus = SOPC_STATUS_NOK;
                        }
                    }
                }
            }

            // Check select clauses result
            if (SOPC_STATUS_OK == filterStatus)
            {
                for (int32_t i = 0; i < filterResult->NoOfSelectClauseResults; i++)
                {
                    if (validSelectClausesOnly && selectClausesResult[i] != filterResult->SelectClauseResults[i])
                    {
                        printf(
                            ">>Test_Client_Toolkit_Events: "
                            "CreateMonitoredItemsResponse[0].FilterResult.SelectClauseResult[%" PRIi32
                            "] result unexpected: 0x%08" PRIX32 " (expected: 0x%08" PRIX32 ")\n",
                            i, filterResult->SelectClauseResults[i], selectClausesResult[i]);
                        filterStatus = SOPC_STATUS_NOK;
                    }
                }
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            status = filterStatus;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_CONSOLE_PRINTF("\n===============> OK\n");
    }
    else
    {
        SOPC_CONSOLE_PRINTF("\n///////////////> NOK\n");
    }
    if (deleteResp)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, (void**) &createMonItResp);
    }
    return status;
}

static SOPC_ReturnStatus delete_monitored_items(SOPC_ClientHelper_Subscription* subscription,
                                                OpcUa_CreateMonitoredItemsResponse** ppCreateMonItResp)
{
    // Check CreateMonitoredItems response and prepare delete monitored items request
    if (NULL == ppCreateMonItResp)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_CreateMonitoredItemsResponse* createMonItResp = *ppCreateMonItResp;
    OpcUa_DeleteMonitoredItemsRequest* delMonItReq = NULL;
    OpcUa_DeleteMonitoredItemsResponse delMonItResp;
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&delMonItResp);
    bool deleteMonitoredItems = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    size_t nbMItoDelete = 0;
    if (SOPC_IsGoodStatus(createMonItResp->ResponseHeader.ServiceResult))
    {
        for (int32_t i = 0; i < createMonItResp->NoOfResults; i++)
        {
            if (SOPC_IsGoodStatus(createMonItResp->Results[i].StatusCode))
            {
                nbMItoDelete++;
            }
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    // Check CreateMonitoredItems response
    if (SOPC_STATUS_OK == status)
    {
        size_t nbMItoDeleteIdx = 0;
        delMonItReq = SOPC_DeleteMonitoredItemsRequest_Create(0, nbMItoDelete, NULL);
        for (int32_t i = 0; SOPC_STATUS_OK == status && i < createMonItResp->NoOfResults; i++)
        {
            status = SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId(delMonItReq, nbMItoDeleteIdx,
                                                                         createMonItResp->Results[i].MonitoredItemId);
            nbMItoDeleteIdx++;
            deleteMonitoredItems = true;
        }
    }

    if (deleteMonitoredItems)
    {
        status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, delMonItReq, &delMonItResp);
        SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, (void**) ppCreateMonItResp);

        // Check DeleteMonitoredItems response
        if (SOPC_STATUS_OK == status)
        {
            if (SOPC_IsGoodStatus(delMonItResp.ResponseHeader.ServiceResult))
            {
                for (int32_t i = 0; i < delMonItResp.NoOfResults; i++)
                {
                    if (!SOPC_IsGoodStatus(delMonItResp.Results[i]))
                    {
                        status = SOPC_STATUS_NOK;
                        SOPC_ASSERT(nbMIs > 0);
                    }
                    else
                    {
                        nbMIs--;
                    }
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
        OpcUa_DeleteMonitoredItemsResponse_Clear(&delMonItResp);
    }
    return status;
}

static SOPC_ReturnStatus call_gen_events_method(SOPC_ClientConnection* secureConnection,
                                                uint32_t optNbEvents,
                                                const SOPC_NodeId* optEventTypeId,
                                                uint32_t optSub,
                                                uint32_t optMIid)
{
    eventNotifReceived = false;
    eventNotifReceived2 = false;
    expectedContentReceived = false;
    expectedContentReceived2 = false;

    SOPC_Variant* inputArgs = NULL;
    OpcUa_CallRequest* callMethod = SOPC_CallRequest_Create(1);
    OpcUa_CallResponse* callMethodResp = NULL;
    SOPC_ReturnStatus status = (NULL == callMethod ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        inputArgs = SOPC_Calloc(4, sizeof(*inputArgs));
        status = (NULL == inputArgs ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Variant_Initialize(&inputArgs[1]);
        inputArgs[1].BuiltInTypeId = SOPC_NodeId_Id;
        inputArgs[1].Value.NodeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        if (NULL != inputArgs[1].Value.NodeId)
        {
            SOPC_NodeId_Initialize(inputArgs[1].Value.NodeId);
            if (NULL != optEventTypeId)
            {
                status = SOPC_NodeId_Copy(inputArgs[1].Value.NodeId, optEventTypeId);
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Variant_Initialize(&inputArgs[0]);
        inputArgs[0].BuiltInTypeId = SOPC_UInt32_Id;
        inputArgs[0].Value.Uint32 = optNbEvents;

        SOPC_Variant_Initialize(&inputArgs[2]);
        inputArgs[2].BuiltInTypeId = SOPC_UInt32_Id;
        inputArgs[2].Value.Uint32 = optSub;

        SOPC_Variant_Initialize(&inputArgs[3]);
        inputArgs[3].BuiltInTypeId = SOPC_UInt32_Id;
        inputArgs[3].Value.Uint32 = optMIid;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CallRequest_SetMethodToCallFromStrings(callMethod, 0, testObjectNodeIdStr, genEventMethodIdStr, 4,
                                                             inputArgs);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, callMethod, (void**) &callMethodResp);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(callMethodResp->ResponseHeader.ServiceResult))
        {
            SOPC_ASSERT(1 == callMethodResp->NoOfResults);
            status = SOPC_IsGoodStatus(callMethodResp->Results[0].StatusCode) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        SOPC_EncodeableObject_Delete(callMethodResp->encodeableType, (void**) &callMethodResp);
    }
    if (NULL != inputArgs)
    {
        SOPC_Variant_Clear(&inputArgs[0]);
        SOPC_Variant_Clear(&inputArgs[1]);
        SOPC_Variant_Clear(&inputArgs[2]);
        SOPC_Variant_Clear(&inputArgs[3]);
        SOPC_Free(inputArgs);
    }
    return status;
}

static SOPC_ReturnStatus Client_Initialize(void)
{
    // Print Toolkit Configuration
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_events_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    return status;
}

static int32_t waitEventReceivedAndResult(const bool expectSuccessFirstMI,
                                          const bool hasSecondMI,
                                          const bool expectSuccessSecondMI)
{
    SOPC_ASSERT(hasSecondMI || !expectSuccessSecondMI); // cannot expect success on second MI if no second MI
    /* Wait until event received */
    uint32_t loopCpt = 0;
    uint32_t localLoopTimeoutMs = loopTimeout;
    if (!expectSuccessFirstMI || (hasSecondMI && !expectSuccessSecondMI))
    {
        localLoopTimeoutMs = failureLoopTimeout;
    }
    while ((!eventNotifReceived || (hasSecondMI && !eventNotifReceived2)) &&
           loopCpt * sleepTimeout <= localLoopTimeoutMs)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (loopCpt * sleepTimeout > localLoopTimeoutMs)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    if (SOPC_STATUS_OK == status)
    {
        if (expectSuccessFirstMI != expectedContentReceived ||
            (hasSecondMI && expectSuccessSecondMI != expectedContentReceived2))
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else if (SOPC_STATUS_TIMEOUT == status)
    {
        if (expectSuccessFirstMI == expectedContentReceived &&
            (!hasSecondMI || expectSuccessSecondMI == expectedContentReceived2))
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_CONSOLE_PRINTF("\n===============> OK\n");
    }
    else
    {
        SOPC_CONSOLE_PRINTF("\n///////////////> NOK (%s, %s)\n",
                            (expectSuccessFirstMI == expectedContentReceived ? "true" : "false"),
                            (!hasSecondMI || expectSuccessSecondMI == expectedContentReceived2) ? "true" : "false");
    }
    eventNotifReceived = false;
    eventNotifReceived2 = false;
    expectedContentReceived = false;
    expectedContentReceived2 = false;
    return (SOPC_STATUS_OK == status ? 0 : 1);
}

static int32_t waitOverflowEventReceived(void)
{
    /* Wait until event received */
    uint32_t loopCpt = 0;
    if (!expectOverflow)
    {
        return 1;
    }
    while (!overflowReceived && !expNbEventsReceivedWhenOverflow && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_CONSOLE_PRINTF("\n///////////////> NOK (overflowReceived: %s, nbEventsReceived: %" PRIu32 ")\n",
                            overflowReceived ? "true" : "false", nbEventsReceived);
    }
    expectOverflow = false;
    nbEventsReceived = 0;
    return (SOPC_STATUS_OK == status ? 0 : 1);
}

int main(void)
{
    SOPC_SecureConnection_Config** secureConnConfigArray = NULL;
    size_t nbSecConnCfgs = 0;

    SOPC_ClientConnection* secureConnection = NULL;

    SOPC_ReturnStatus status = Client_Initialize();

    if (SOPC_STATUS_OK == status)
    {
        status = Client_LoadClientConfiguration(&nbSecConnCfgs, &secureConnConfigArray);
    }

    /* Create anonymous user connection */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(secureConnConfigArray[0], SOPC_Client_ConnEventCb, &secureConnection);
    }

    bool connCreated = false;
    bool subCreated = false;
    bool firstMIcreated = false;
    bool secondMIcreated = false;
    uint32_t firstMIid = 0;
    uint32_t secondMIid = 0;
    SOPC_ReturnStatus tmpStatus = SOPC_STATUS_NOK;
    SOPC_ClientHelper_Subscription* sub = NULL;
    OpcUa_CreateMonitoredItemsResponse* miRespTestObj = NULL;
    OpcUa_CreateMonitoredItemsResponse* miRespServer = NULL;
    uint32_t revisedQueueSizeExp = 0;
    bool validSelectClausesOnly = false;
    bool invalidSelectClausesOnly = false;
    bool validWhereClauseExp = false;

    int32_t unexpectedResults = 0;
    // Create a subscription
    if (SOPC_STATUS_OK == status)
    {
        connCreated = true;
        sub = create_subscription(secureConnection);
        status = (NULL == sub ? SOPC_STATUS_NOK : status);
    }

    // Create the first MI for events on TestObject node with all clauses (valid and invalid clauses)
    if (SOPC_STATUS_OK == status)
    {
        subCreated = true;
        // Requested queue size 0 => default queue size
        miRespTestObj =
            create_monitored_item_event(sub, true, 0, NULL, validSelectClausesOnly, invalidSelectClausesOnly);
        revisedQueueSizeExp = SOPC_DEFAULT_EVENT_NOTIFICATION_QUEUE_SIZE;
        status = (NULL == miRespTestObj ? SOPC_STATUS_NOK : status);
    }

    if (SOPC_STATUS_OK == status)
    {
        firstMIcreated = true;
        // Check response content + revised queue size
        status = check_monitored_items(miRespTestObj, revisedQueueSizeExp);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 1: base event type ===========\n");
    // Generate events with default options
    if (SOPC_STATUS_OK == status)
    {
        firstMIid = miRespTestObj->Results[0].MonitoredItemId;
        status = call_gen_events_method(secureConnection, 0, NULL, 0, 0);
    }

    // Check events received for the first MI
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, false, false);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 2: condition event type ===========\n");

    // Generate events with specific type
    if (SOPC_STATUS_OK == status)
    {
        // activate condition type event field expected
        notifEventFieldSetResult[CONDITION_TYPE_FIELD_IDX] = true;
        status = call_gen_events_method(secureConnection, 1, &conditionTypeId, 0, 0);
    }

    // Check events received for the first MI
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, false, false);
    }

    /* Create a second MI for Server object with all clauses (valid and invalid clauses) */
    if (SOPC_STATUS_OK == status)
    {
        // Requested queue size 1 => minimum queue size
        miRespServer =
            create_monitored_item_event(sub, false, 1, NULL, validSelectClausesOnly, invalidSelectClausesOnly);
        revisedQueueSizeExp = SOPC_MIN_EVENT_NOTIFICATION_QUEUE_SIZE;
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Check response content + revised queue size
        secondMIcreated = true;
        status = check_monitored_items(miRespServer, revisedQueueSizeExp);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 3: alarm condition type (TestObject + Server) ===========\n");
    // Generate events with specific type
    if (SOPC_STATUS_OK == status)
    {
        secondMIid = miRespServer->Results[0].MonitoredItemId;
        // activate alarm condition type event field expected (in addition to condition with is parent type)
        notifEventFieldSetResult[ALARM_CONDITION_TYPE_FIELD_IDX] = true;
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId, 0, 0);
    }

    // Check events received for the first and second MIs
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, true);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 4: alarm type + valid subscription id (TestObject + Server) ===========\n");
    uint32_t subId = 0;
    // Retrieve the subscription Id
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_GetSubscriptionId(sub, &subId);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    // Generate events with specific type only for the created subscription
    if (SOPC_STATUS_OK == status)
    {
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId, subId, 0);
    }
    // Check events received for the first and second MIs
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, true);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 5: alarm type + invalid subscription id ===========\n");
    // Generate events with specific type and a subscription Id which IS NOT the created subscriptionId
    if (SOPC_STATUS_OK == status)
    {
        // By default use UINT32_MAX as subId unless it is the actual current subId, then use 1 which is not...
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId,
                                        (subId == UINT32_MAX ? 1 : UINT32_MAX), 0);
    }
    // Check NO event is received for both MIs
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(false, true, false);
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 6: alarm type + valid subscription id + valid MI id  (TestObject) ===========\n");
    // Generate events with specific type only for the created subscription and only for the first MI
    if (SOPC_STATUS_OK == status)
    {
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId, subId, firstMIid);
    }
    // Check events received for the first MI only and none received for the second MI
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, false);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 7: alarm type + valid subscription id + valid MI id  (Server) ===========\n");
    // Generate events with specific type only for the created subscription and only for the second MI
    if (SOPC_STATUS_OK == status)
    {
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId, subId, secondMIid);
    }
    // Check events received for the second MI only and none received for the first MI
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(false, true, true);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 8: alarm type + valid subscription id + invalid MI id ===========\n");
    // Generate events with specific type only for the created subscription and using a NOT expected MI id
    if (SOPC_STATUS_OK == status)
    {
        // By default use UINT32_MAX as "invalid" MIid, unless it is the actual first or second MIid,
        // then use 1 unless it is the second MIid already used, then use 2 which cannot be used.
        status = call_gen_events_method(
            secureConnection, 0, &alarmConditionTypeId, subId,
            (firstMIid == UINT32_MAX || secondMIid == UINT32_MAX ? (firstMIid == 1 || secondMIid == 1 ? 2 : 1)
                                                                 : UINT32_MAX));
    }
    // Check NO event is received for both MIs
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(false, true, false);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 9: alarm type + invalid subscription id + invalid MI id ===========\n");
    // Generate events with specific type with NOT the created subscription and using a NOT expected MI id
    if (SOPC_STATUS_OK == status)
    {
        // Note : see the 2 previous invalid ids cases for id choice logic
        status = call_gen_events_method(
            secureConnection, 0, &alarmConditionTypeId, (subId == UINT32_MAX ? 1 : UINT32_MAX),
            (firstMIid == UINT32_MAX || secondMIid == UINT32_MAX ? (firstMIid == 1 || secondMIid == 1 ? 2 : 1)
                                                                 : UINT32_MAX));
    }
    // Check NO event is received for both MIs
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(false, true, false);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 10: alarm type + valid MI id  (TestObject) ===========\n");
    // Generate events with specific type only on first MI (but without subscription id provided)
    if (SOPC_STATUS_OK == status)
    {
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId, 0, firstMIid);
    }
    // Check events received for the first MI only and none received for the second MI
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, false);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 11: alarm type + invalid MI id ===========\n");
    // Generate events with specific type with "invalid" MI id (but without subscription id provided)
    if (SOPC_STATUS_OK == status)
    {
        // By default use UINT32_MAX as "invalid" MIid, unless it is the actual first or second MIid,
        // then use 1 unless it is the second MIid already used, then use 2 which cannot be used.
        status = call_gen_events_method(
            secureConnection, 0, &alarmConditionTypeId, 0,
            (firstMIid == UINT32_MAX || secondMIid == UINT32_MAX ? (firstMIid == 1 || secondMIid == 1 ? 2 : 1)
                                                                 : UINT32_MAX));
    }
    // Check NO event is received for both MIs
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(false, true, false);
    }

    /* Delete second MI  */
    if (SOPC_STATUS_OK == status && secondMIcreated)
    {
        status = delete_monitored_items(sub, &miRespServer);
        if (SOPC_STATUS_OK == status)
        {
            secondMIcreated = false;
        }
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 12: base event type (TestObject + !Server (WHERE=AlarmConditionType)) ===========\n");

    /* Create a second MI for Server object with WHERE clause element filter on Alarm&Condition type */
    if (SOPC_STATUS_OK == status)
    {
        // Requested queue size INT32_MAX => maximum queue size
        miRespServer = create_monitored_item_event(sub, false, INT32_MAX, &alarmConditionTypeId, validSelectClausesOnly,
                                                   invalidSelectClausesOnly);
        revisedQueueSizeExp = SOPC_MAX_NOTIFICATION_QUEUE_SIZE;
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Check response content + revised queue size
        secondMIcreated = true;
        status = check_monitored_items(miRespServer, revisedQueueSizeExp);
    }

    if (SOPC_STATUS_OK == status)
    {
        secondMIid = miRespServer->Results[0].MonitoredItemId;
        // de-activate condition type and alarm condition type event field
        notifEventFieldSetResult[CONDITION_TYPE_FIELD_IDX] = false;
        notifEventFieldSetResult[ALARM_CONDITION_TYPE_FIELD_IDX] = false;
        // Generate events without specific type and with default parameters
        status = call_gen_events_method(secureConnection, 0, NULL, 0, 0);
    }
    // Check event is received for first MI
    // but not for the second since it filters AlarmConditionType using WHERE clause
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, false);
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 13: condition event type (TestObject + !Server (WHERE=AlarmConditionType)) "
        "===========\n");

    if (SOPC_STATUS_OK == status)
    {
        // activate condition type event field expected
        notifEventFieldSetResult[CONDITION_TYPE_FIELD_IDX] = true;
        // Generate events with specific type ConditionType
        status = call_gen_events_method(secureConnection, 0, &conditionTypeId, 0, 0);
    }
    // Check event is received for first MI
    // but not for the second since it filters AlarmConditionType using WHERE clause
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, false);
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 14: alarm condition type (TestObject + Server (WHERE=AlarmConditionType)) ===========\n");
    if (SOPC_STATUS_OK == status)
    {
        // activate alarm condition type event field expected
        notifEventFieldSetResult[ALARM_CONDITION_TYPE_FIELD_IDX] = true;
        // Generate events with specific type AlarmConditionType
        status = call_gen_events_method(secureConnection, 0, &alarmConditionTypeId, 0, 0);
    }
    // Check event is received for first MI and second MI
    // (since it filters AlarmConditionType using WHERE clause)
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(true, true, true);
    }
    // Delete the first MI
    if (SOPC_STATUS_OK == status && firstMIcreated)
    {
        status = delete_monitored_items(sub, &miRespTestObj);
        if (SOPC_STATUS_OK == status)
        {
            firstMIcreated = false;
        }
    }
    // Delete the second MI
    if (SOPC_STATUS_OK == status && secondMIcreated)
    {
        status = delete_monitored_items(sub, &miRespServer);
        if (SOPC_STATUS_OK == status)
        {
            secondMIcreated = false;
        }
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 15: base event type (TestObject): EventQueueOverflow (even with WHERE for condition "
        "type) ===========\n");
    // Create the first MI for events on TestObject node with all clauses (valid and invalid clauses) and ConditionType
    // for WHERE clause
    if (SOPC_STATUS_OK == status)
    {
        miRespTestObj = create_monitored_item_event(sub, true, 1, &conditionTypeId, validSelectClausesOnly,
                                                    invalidSelectClausesOnly);
        // Requested queue size 1 => minimum queue size
        revisedQueueSizeExp = SOPC_MIN_EVENT_NOTIFICATION_QUEUE_SIZE;
        status = (NULL == miRespTestObj ? SOPC_STATUS_NOK : status);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Check response content + revised queue size
        status = check_monitored_items(miRespTestObj, revisedQueueSizeExp);
    }

    // Generate a lot of events to trigger the EventQueueOverflow event
    if (SOPC_STATUS_OK == status)
    {
        firstMIcreated = true;
        firstMIid = miRespTestObj->Results[0].MonitoredItemId;
        // activate condition type event field expected and deactivate alarm condition type event field
        notifEventFieldSetResult[CONDITION_TYPE_FIELD_IDX] = true;
        notifEventFieldSetResult[ALARM_CONDITION_TYPE_FIELD_IDX] = false;
        expectOverflow = true;
        nbEventsReceived = 0;
        expNbEventsReceivedWhenOverflow = false;
        status = call_gen_events_method(secureConnection, nbEventsBeforeOverflow + 1, &conditionTypeId, 0, 0);
    }

    // Check overflow event is received with the expected number of events (queue max size + overflow)
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitOverflowEventReceived();
        expectOverflow = true;
        nbEventsReceived = 0;
        expNbEventsReceivedWhenOverflow = false;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Expecting no event after overflow
        unexpectedResults += waitEventReceivedAndResult(false, false, false);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 16: create MI: only valid select clauses (no filter result) ===========\n");
    // Create the second MI for events on Server node with valid clauses only and ConditionType for WHERE clause
    if (SOPC_STATUS_OK == status)
    {
        validSelectClausesOnly = true;
        validWhereClauseExp = true;
        miRespServer = create_monitored_item_event(sub, false, 1, &conditionTypeId, validSelectClausesOnly,
                                                   invalidSelectClausesOnly);
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }

    // Expecting creation SUCCESS
    if (SOPC_STATUS_OK == status)
    {
        secondMIcreated = true;
        status = check_monitored_items_special_cases(miRespServer, revisedQueueSizeExp, true, validWhereClauseExp,
                                                     validSelectClausesOnly);
        unexpectedResults = (SOPC_STATUS_OK != status ? unexpectedResults + 1 : unexpectedResults);
    }

    /* Delete second MI */
    if (SOPC_STATUS_OK == status)
    {
        status = delete_monitored_items(sub, &miRespServer);
    }

    SOPC_CONSOLE_PRINTF("\n=========== TC 17: create MI: only invalid select clauses ===========\n");

    /* Create second MI with only invalid clauses and AlarmConditionType for WHERE clause */
    if (SOPC_STATUS_OK == status)
    {
        secondMIcreated = false;
        validSelectClausesOnly = false;
        invalidSelectClausesOnly = true;
        miRespServer = create_monitored_item_event(sub, false, 1, &alarmConditionTypeId, validSelectClausesOnly,
                                                   invalidSelectClausesOnly);
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }
    // Expecting creation FAILURE
    if (SOPC_STATUS_OK == status)
    {
        status = check_monitored_items_special_cases(miRespServer, revisedQueueSizeExp, false, validWhereClauseExp,
                                                     validSelectClausesOnly);
        if (SOPC_STATUS_OK != status)
        {
            secondMIcreated = true;
            unexpectedResults++;
        }
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 18: create MI: only invalid select clauses and invalid where clause ===========\n");

    /* Create second MI with only invalid clauses and ServerObject for WHERE clause which is invalid => not a type */
    if (SOPC_STATUS_OK == status)
    {
        secondMIcreated = false;
        validSelectClausesOnly = false;
        invalidSelectClausesOnly = true;
        validWhereClauseExp = false;
        miRespServer = create_monitored_item_event(sub, false, 1, &serverObjectId, validSelectClausesOnly,
                                                   invalidSelectClausesOnly);
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }

    // Expecting creation FAILURE
    if (SOPC_STATUS_OK == status)
    {
        status = check_monitored_items_special_cases(miRespServer, revisedQueueSizeExp, false, validWhereClauseExp,
                                                     validSelectClausesOnly);
        if (SOPC_STATUS_OK != status)
        {
            secondMIcreated = true;
            unexpectedResults++;
        }
    }

    SOPC_CONSOLE_PRINTF(
        "\n=========== TC 19: create MI: select clause with invalid and valid index ranges ===========\n");

    /* Create second MI with index range select clauses for Server object */
    if (SOPC_STATUS_OK == status)
    {
        secondMIcreated = false;
        validSelectClausesOnly = false;
        invalidSelectClausesOnly = false;
        validWhereClauseExp = true;
        miRespServer = create_monitored_item_event_index_ranges(sub);
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }
    // Expecting creation SUCCESS
    if (SOPC_STATUS_OK == status)
    {
        secondMIid = miRespServer->Results[0].MonitoredItemId;
        secondMIcreated = true;
        status = check_monitored_items_index_ranges(miRespServer);
        if (SOPC_STATUS_OK != status)
        {
            unexpectedResults++;
        }
    }

    // Generate events only on second MI
    if (SOPC_STATUS_OK == status)
    {
        status = call_gen_events_method(secureConnection, 0, NULL, subId, secondMIid);
    }

    // Expect only events on second MI
    if (SOPC_STATUS_OK == status)
    {
        unexpectedResults += waitEventReceivedAndResult(false, true, true);
    }

    // TODO: add more degraded cases: unsupported/invalid where operator, empty select clauses, etc.

    // Delete MIs at the end of test if flagged as needed (even in case of unexpected failure)
    if (firstMIcreated)
    {
        tmpStatus = delete_monitored_items(sub, &miRespTestObj);
        if (SOPC_STATUS_OK == tmpStatus)
        {
            firstMIcreated = false;
        }
        status = (SOPC_STATUS_OK == status ? tmpStatus : status);
    }

    if (secondMIcreated)
    {
        tmpStatus = delete_monitored_items(sub, &miRespServer);
        if (SOPC_STATUS_OK == tmpStatus)
        {
            secondMIcreated = false;
        }
        status = (SOPC_STATUS_OK == status ? tmpStatus : status);
    }

    if (subCreated)
    {
        tmpStatus = SOPC_ClientHelper_DeleteSubscription(&sub);
        status = (SOPC_STATUS_OK == status ? tmpStatus : status);
    }

    /* Close the connection */
    if (connCreated)
    {
        tmpStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        status = (SOPC_STATUS_OK == status ? tmpStatus : status);
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK == status && 0 == unexpectedResults)
    {
        printf(">>Test_Client_Toolkit_Events final result: OK\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit_Events final result: NOK (BAD status: %" PRIu32
               " or incorrect result(s): %" PRIi32 ")\n",
               status, unexpectedResults);
    }
    return (0 == unexpectedResults ? (int) status : unexpectedResults);
}
