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

/** \brief Test description.
 *
 * This test aims at checking the Security audit events (SecureChannel, Session and Certificate)
 * that are generated on server-side for client related events.
 *
 * This test application :
 * - Acts as on OPCUA client and subscribes to Server Events.
 * - Loads its configuration from file provided by environment variable 'TEST_CLIENT_XML_CONFIG'.
 * - Connects to the server with S&E and subscribes to the "Server" Node to receive Audit events.
 * - Execute different remote process that connect as client on the server.
 * - Check the received events regarding these actions.
 *
 * \note jokers can be used in expected values:
 * - '?' will match any character
 * - '*' (as ending) will match any remaining string
 * - '*<c>' (with <c> = any char), will match any remaining string on first occurrence, then will
 * check the same value on next occurrences.
 *
 * See file "toolkit_test_client_audit_test.h" for actual content of events checked
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toolkit_test_client_audit.h"

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_assert.h"
#include "sopc_encodeabletype.h"
#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_singly_linked_list.h"
#include "sopc_threads.h"

#include "sopc_helper_askpass.h"

#define XML_AUDIT_CONFIG_FILE_PATH "S2OPC_Client_Audit_Config.xml"

#define SECUADMIN_PASSWORD_ENV_NAME "TEST_PASSWORD_USER_SECUADMIN"

#define DUMP_EVENTS_ON_CONSOLE 0

#if DUMP_EVENTS_ON_CONSOLE
#define DEBUG_PRINT SOPC_CONSOLE_PRINTF
#define DEBUG_JOKER SOPC_CONSOLE_PRINTF
#else
#define DEBUG_PRINT(...)
#define DEBUG_JOKER(...)

#endif

static const SOPC_NodeId serverObjectId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server);
static const SOPC_NodeId baseEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_BaseEventType);
static const SOPC_NodeId auditEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditEventType);
static const SOPC_NodeId auditSecuEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditSecurityEventType);
static const SOPC_NodeId auditCertEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateEventType);
static const SOPC_NodeId auditCertDataMismatchEventTypeId =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCertificateDataMismatchEventType);
static const SOPC_NodeId auditChannelEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditChannelEventType);
static const SOPC_NodeId auditOpenSCEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditOpenSecureChannelEventType);
static const SOPC_NodeId auditSessionEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditSessionEventType);
static const SOPC_NodeId auditCreateSessionEventTypeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditCreateSessionEventType);
static const SOPC_NodeId auditActivateSessionEventTypeId =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_AuditActivateSessionEventType);
static const char* monitoredItemCtxt[1] = {"Server"};

static uint32_t nbFailures = 0;
typedef struct
{
    const char* path;
    const SOPC_NodeId* typeId;
} SelectClause_Cfg;

static const SelectClause_Cfg selectClauses[] = {
    // SecureChannel specific audit events fields
    {"0:ClientCertificate", &auditOpenSCEventTypeId},
    {"0:ClientCertificateThumbprint", &auditOpenSCEventTypeId},
    {"0:RequestType", &auditOpenSCEventTypeId},
    {"0:SecurityPolicyUri", &auditOpenSCEventTypeId},
    {"0:SecurityMode", &auditOpenSCEventTypeId},
    {"0:RequestedLifetime", &auditOpenSCEventTypeId},
    {"0:CertificateErrorEventId", &auditOpenSCEventTypeId},
    {"0:SecureChannelId", &auditChannelEventTypeId},

    // Session specific audit events fields
    {"0:SessionId", &auditSessionEventTypeId},
    // Session create
    {"0:SecureChannelId", &auditCreateSessionEventTypeId},
    {"0:ClientCertificate", &auditCreateSessionEventTypeId},
    {"0:ClientCertificateThumbprint", &auditCreateSessionEventTypeId},
    {"0:RevisedSessionTimeout", &auditCreateSessionEventTypeId},
    // Session activate
    {"0:ClientSoftwareCertificates", &auditActivateSessionEventTypeId},
    {"0:UserIdentityToken", &auditActivateSessionEventTypeId},
    {"0:SecureChannelId", &auditActivateSessionEventTypeId},

    // Certificate validation specific audit events fields
    {"0:InvalidHostname", &auditCertDataMismatchEventTypeId},
    {"0:InvalidUri", &auditCertDataMismatchEventTypeId},
    {"0:Certificate", &auditCertEventTypeId},

    // Security audit events fields
    {"0:StatusCodeId", &auditSecuEventTypeId},

    // Common audit events fields
    {"0:ActionTimeStamp", &auditEventTypeId},
    {"0:Status", &auditEventTypeId},
    {"0:ServerId", &auditEventTypeId},
    {"0:ClientAuditEntryId", &auditEventTypeId},
    {"0:ClientUserId", &auditEventTypeId},

    // Base event fields
    {"0:EventId", &baseEventTypeId},
    {"0:EventType", &baseEventTypeId},
    {"0:SourceNode", &baseEventTypeId},
    {"0:SourceName", &baseEventTypeId},
    {"0:Message", &baseEventTypeId},
    {"0:Severity", &baseEventTypeId},
};

#define NB_SELECT_CLAUSES (sizeof(selectClauses) / sizeof(*selectClauses))

typedef struct EventContentItem
{
    const char* clause;
    char* value; ///< Must  be freed after use
} EventContentItem;

typedef struct EventContent
{
    EventContentItem at[NB_SELECT_CLAUSES];
} EventContent;

static void EventContent_Clear(EventContent* evt)
{
    for (unsigned i = 0; evt != NULL && i < NB_SELECT_CLAUSES; i++)
    {
        SOPC_Free(evt->at[i].value);
    }
    SOPC_Free(evt);
}
// Current list of received event notifications (Contains some EventContent*)
static SOPC_SLinkedList* sEvtReceived;

// Current memorized jokers values
static char* sJokers[256] = {0};

static void jokersClear(void)
{
    for (size_t i = 0; i < 256; i++)
    {
        SOPC_Free(sJokers[i]);
        sJokers[i] = NULL;
    }
}

static void SOPC_Client_SubscriptionNotification_Cb(const SOPC_ClientHelper_Subscription* subscription,
                                                    SOPC_StatusCode status,
                                                    SOPC_EncodeableType* notificationType,
                                                    uint32_t nbNotifElts,
                                                    const void* notification,
                                                    uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(notificationType);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);
    SOPC_ASSERT(NULL != sEvtReceived);

    SOPC_Buffer* buf = SOPC_Buffer_CreateResizable(1024, 1024 * 16);

    uintptr_t userCtx = SOPC_ClientHelper_Subscription_GetUserParam(subscription);
    SOPC_UNUSED_ARG(userCtx);

    DEBUG_PRINT("Notif CB status = 0x%08X, notif = %p, userCtx=%p \n", (unsigned) status, notification,
                (void*) userCtx);

    if (!SOPC_IsGoodStatus(status))
        return;

    if (&OpcUa_EventNotificationList_EncodeableType == notificationType)
    {
        const OpcUa_EventNotificationList* eventList = (const OpcUa_EventNotificationList*) notification;
        bool localExpectedContentReceived = ((int32_t) nbNotifElts == eventList->NoOfEvents);

        for (int32_t i = 0; localExpectedContentReceived && i < eventList->NoOfEvents; i++)
        {
            const OpcUa_EventFieldList* event = &eventList->Events[i];
            DEBUG_PRINT("- EventNotif[%" PRIi32 "]('%s')\n", i, (const char*) monitoredItemCtxArray[i]);

            EventContent* newEvt = (EventContent*) SOPC_Calloc(1, sizeof(*newEvt));
            SOPC_ASSERT(NULL != newEvt);

            // Preformat the event output in a new entry of sEvtReceived
            for (int32_t iField = 0; iField < event->NoOfEventFields; iField++)
            {
                SOPC_Buffer_Reset(buf);
                const SOPC_Variant* var = &event->EventFields[iField];
                if (var->BuiltInTypeId == SOPC_Null_Id)
                    continue;
                SOPC_Variant_Dump(buf, var);
                // Ensure that there is a ZERO char at the end.
                uint32_t pos = (buf->position < buf->maximum_size ? buf->position : buf->length - 1);
                buf->data[pos] = 0;

                DEBUG_PRINT("  {\"%s\", \"%s\"},\n", selectClauses[iField].path, (const char*) buf->data);

                newEvt->at[iField].clause = selectClauses[iField].path; // Shallow copy
                newEvt->at[iField].value = SOPC_strdup((const char*) buf->data);
            }
            SOPC_SLinkedList_Append(sEvtReceived, 0, (uintptr_t) newEvt);
        }
    }
    SOPC_Buffer_Delete(buf);
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

static bool SOPC_GetClientUserSecuAdminPassword(const SOPC_SecureConnection_Config* secConnConfig,
                                                char** outUserName,
                                                char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    const char* secuAdmin = "secuAdmin";
    char* userName = SOPC_Calloc(strlen(secuAdmin) + 1, sizeof(*userName));
    if (NULL == userName)
    {
        return false;
    }
    memcpy(userName, secuAdmin, strlen(secuAdmin) + 1);
    bool bres = false;
    char* _outPassword = getenv(SECUADMIN_PASSWORD_ENV_NAME);
    if (NULL != _outPassword)
    {
        *outPassword = SOPC_strdup(_outPassword); // Do a copy
        bres = (NULL != *outPassword);
    }
    if (!bres)
    {
        printf("<SOPC_TestHelper_AskPass_FromEnv: The following environment variable is missing (or copy failed): %s\n",
               SECUADMIN_PASSWORD_ENV_NAME);
        SOPC_Free(userName);
        return false;
    }
    *outUserName = userName;
    return true;
}

static SOPC_ReturnStatus Client_LoadClientConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    /* Retrieve XML configuration file S2OPC_Client_Audit_Config.xml
     *
     * In case of success returns the file path.
     */

    SOPC_CONSOLE_PRINTF("Using configuration file '%s'\n", XML_AUDIT_CONFIG_FILE_PATH);
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_ConfigureFromXML(XML_AUDIT_CONFIG_FILE_PATH, NULL, nbSecConnCfgs,
                                                                        secureConnConfigArray);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_CONSOLE_PRINTF("Error: an XML client configuration file path provided parsing failed\n");
    }

    // Set callback necessary to retrieve client key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    // Set callback necessary to retrieve user password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&SOPC_GetClientUserSecuAdminPassword);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_CONSOLE_PRINTF(">>Test_Client_Toolkit_Audit: Client configured\n");
    }
    else
    {
        SOPC_CONSOLE_PRINTF(">>Test_Client_Toolkit_Audit: Client configuration failed\n");
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

static OpcUa_CreateMonitoredItemsResponse* create_monitored_item_event(SOPC_ClientHelper_Subscription* subscription)
{
    SOPC_ASSERT(NULL != subscription);

    OpcUa_CreateMonitoredItemsRequest* createMonItReq =
        SOPC_CreateMonitoredItemsRequest_Create(0, 1, OpcUa_TimestampsToReturn_Both);
    if (NULL == createMonItReq)
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(createMonItReq, 0, &serverObjectId,
                                                                 SOPC_AttributeId_EventNotifier, NULL);

    OpcUa_EventFilter* eventFilter = NULL;
    if (SOPC_STATUS_OK == status)
    {
        size_t nbWhereClauseElts = 0;
        eventFilter = SOPC_MonitoredItem_CreateEventFilter(NB_SELECT_CLAUSES, nbWhereClauseElts);
    }

    if (NULL != eventFilter)
    {
        for (size_t i = 0; i < NB_SELECT_CLAUSES && SOPC_STATUS_OK == status; i++)
        {
            const SelectClause_Cfg* clause = &selectClauses[i];
            char* typeIdStr = SOPC_NodeId_ToCString(clause->typeId);
            status = SOPC_EventFilter_SetSelectClauseFromStringPath(eventFilter, i, typeIdStr, '~', clause->path,
                                                                    SOPC_AttributeId_Value, NULL);
            SOPC_Free(typeIdStr);
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        static const uint32_t queueSizeRequested = 1;
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
            subscription, createMonItReq, (const uintptr_t*) monitoredItemCtxt, createMonItResp);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsResponse_EncodeableType, (void**) &createMonItResp);
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
                SOPC_CONSOLE_PRINTF(">>Test_Client_Toolkit_Audit: CreateMonitoredItemsResponse[%" PRIi32
                                    "] result not good: 0x%08" PRIX32 "\n",
                                    0, createMonItResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            SOPC_CONSOLE_PRINTF(
                ">>Test_Client_Toolkit_Audit: Unexpected number of MI in CreateMonitoredItemsResponse %" PRIi32
                " != 1\n",
                createMonItResp->NoOfResults);
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        SOPC_CONSOLE_PRINTF(
            ">>Test_Client_Toolkit_Audit: CreateMonitoredItemsResponse global result not good: 0x%08" PRIX32 "\n",
            createMonItResp->ResponseHeader.ServiceResult);
    }
    return status;
}

static SOPC_ReturnStatus check_monitored_items(OpcUa_CreateMonitoredItemsResponse* createMonItResp)
{
    SOPC_ReturnStatus status = check_mi_resp_header(createMonItResp);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ExtensionObject* result0 = &createMonItResp->Results[0].FilterResult;

        // No invalid filters, so that no Filter response is expected.
        if (SOPC_ExtObjBodyEncoding_None != result0->Encoding)
        {
            SOPC_CONSOLE_PRINTF(
                ">>Test_Client_Toolkit_Audit: Unexpected CreateMonitoredItemsResponse[0] filter encoding type: "
                "%d\n",
                (int) result0->Encoding);
            status = SOPC_STATUS_NOK;
        }
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
static SOPC_ReturnStatus Client_Initialize(void)
{
    // Print Toolkit Configuration
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    SOPC_CONSOLE_PRINTF("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
                        build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
                        build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    SOPC_CONSOLE_PRINTF("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
                        build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
                        build_info.clientServerBuildInfo.buildDockerId,
                        build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_audit_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    return status;
}

static bool jokerMatch(const char* a, const char* b)
{
    if (strcmp(a, b) == 0)
    {
        // simple match!
        return true;
    }

    // Check any joker (*) in 'b'
    const size_t lenA = strlen(a);
    const size_t lenB = strlen(b);
    for (size_t pos = 0; pos < lenB && pos < lenA; pos++)
    {
        if (b[pos] == '*')
        {
            if (pos + 1 < lenB)
            {
                unsigned char jokerId = (unsigned char) b[pos + 1];
                if (sJokers[jokerId] != NULL)
                {
                    // Existing Joker, continue with remaining string.
                    DEBUG_JOKER("Note : Checking existing joker '%c' ='%s' vs '%s'\n", b[pos + 1], &a[pos],
                                sJokers[jokerId]);
                    return strcmp(&a[pos], sJokers[jokerId]) == 0;
                }
                else
                {
                    // register a new joker and accept result.
                    sJokers[jokerId] = SOPC_strdup(&a[pos]);
                    DEBUG_JOKER("Creation joker '%c' ='%s'\n", b[pos + 1], sJokers[jokerId]);
                }
            }
            return true; // Ignore all after '*'
        }
        if (b[pos] == '?')
            continue; // Ignore only this char '?'
        if (b[pos] != a[pos])
            return false; // Stop on mismatch
    }
    return (lenB == lenA);
}

static uint64_t str_hash(const uintptr_t str)
{
    // We only hash on the first 3 chars to allow easy conflicts
    size_t hash_len = strlen((const char*) str);

    if (hash_len > 3)
    {
        hash_len = 3;
    }

    return SOPC_DJBHash((const uint8_t*) str, hash_len);
}

static bool str_equal(const uintptr_t a, const uintptr_t b)
{
    return strcmp((const char*) a, (const char*) b) == 0;
}

static SOPC_Dict* getRemainingExpFields(const Test_ExpectedValue* expFieldsList)
{
    SOPC_Dict* remainingFields = SOPC_Dict_Create((uintptr_t) "", str_hash, str_equal, NULL, NULL);
    SOPC_ASSERT(NULL != remainingFields);
    SOPC_Dict_SetTombstoneKey(remainingFields, (uintptr_t) NULL);
    for (; expFieldsList->clause != NULL; expFieldsList++)
    {
        bool res = SOPC_Dict_Insert(remainingFields, (uintptr_t) expFieldsList->clause, (uintptr_t) NULL);
        SOPC_ASSERT(res);
    }
    return remainingFields;
}

static void print_missingClause(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    if (NULL == (void*) key)
    {
        return;
    }
    SOPC_UNUSED_ARG(value);
    SOPC_UNUSED_ARG(user_data);
    SOPC_CONSOLE_PRINTF("Missing expected clause: '%s'\n", (const char*) key);
}

static bool checkExpectedEvent(const EventContent* evt, const Test_ExpectedValue* expFieldsList)
{
    SOPC_Dict* remainingFieldsToFind = getRemainingExpFields(expFieldsList);
    const char* context = expFieldsList->value;
    DEBUG_PRINT("\n=======\nSearching for evt ('%s'='%s')\n", expFieldsList->clause, context);
    bool result = false;
    for (; expFieldsList->clause != NULL; expFieldsList++)
    {
        for (unsigned iEvt = 0; iEvt < NB_SELECT_CLAUSES; iEvt++)
        {
            const EventContentItem* evtItem = &evt->at[iEvt];
            if (evtItem->clause == NULL)
                continue;
            if (strcmp(evtItem->clause, expFieldsList->clause) == 0)
            {
                SOPC_Dict_Remove(remainingFieldsToFind, (uintptr_t) expFieldsList->clause);
                // Clause received in evt. Check result.
                if (!jokerMatch(evtItem->value, expFieldsList->value))
                {
                    SOPC_CONSOLE_PRINTF("Mismatching clause %s = '%s' vs '%s' in '%s'\n", evtItem->clause,
                                        evtItem->value, expFieldsList->value, context);
                    SOPC_Dict_Delete(remainingFieldsToFind);
                    return false; // Mismatching
                }
                result = true; // At least one matching field.
            }
        }
    }
    if (SOPC_Dict_Size(remainingFieldsToFind) > 0)
    {
        SOPC_CONSOLE_PRINTF("Missing expected clause(s) not found in '%s'\n", context);
        SOPC_Dict_ForEach(remainingFieldsToFind, print_missingClause, (uintptr_t) NULL);
        result = false;
    }
    SOPC_Dict_Delete(remainingFieldsToFind);
    return result;
}

static SOPC_ReturnStatus exec_audit_test(const Test_ExpectedValue** expectedEventsList, const uint32_t maxWait_ms)
{
    jokersClear();

    uint32_t nb100ms = maxWait_ms / 100;

    for (const Test_ExpectedValue** iter = expectedEventsList; NULL != *iter; iter++)
    {
        const Test_ExpectedValue* expected = (*iter);
        bool found = false;
        while (!found && nb100ms > 0)
        {
            EventContent* evt = (EventContent*) SOPC_SLinkedList_PopHead(sEvtReceived);
            if (evt == NULL)
            {
                SOPC_Sleep(100);
                nb100ms -= 1;
            }
            else
            {
                found = checkExpectedEvent(evt, expected);
                if (!found)
                {
                    SOPC_CONSOLE_PRINTF("Skipping mismatching event...");
                }
                EventContent_Clear(evt);
            }
        };

        if (!found)
        {
            nbFailures++;
            SOPC_CONSOLE_PRINTF("Event '%s'='%s' not found!\n", expected->clause, expected->value);
        }
    }

    jokersClear();
    return SOPC_STATUS_OK;
}

int main(void)
{
    SOPC_SecureConnection_Config** secureConnConfigArray = NULL;
    size_t nbSecConnCfgs = 0;

    SOPC_ClientConnection* secureConnection = NULL;

    SOPC_ReturnStatus status = Client_Initialize();

    sEvtReceived = SOPC_SLinkedList_Create(256);

    if (SOPC_STATUS_OK == status)
    {
        status = Client_LoadClientConfiguration(&nbSecConnCfgs, &secureConnConfigArray);
    }

    /* Create Secured user connection */
    if (SOPC_STATUS_OK == status)
    {
        static const char* cfgid = "2";
        SOPC_SecureConnection_Config* cfg = SOPC_ClientConfigHelper_GetConfigFromId(cfgid);
        if (NULL == cfg)
        {
            SOPC_CONSOLE_PRINTF("[EE] Could not load client configuration. Missing id='%s'\n", cfgid);
            status = SOPC_STATUS_NOK;
        }
        status = SOPC_ClientHelper_Connect(cfg, SOPC_Client_ConnEventCb, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_CONSOLE_PRINTF("[EE] Could not connect client using configuration id='%s'. Is the server running?\n",
                                cfgid);
        }
    }

    bool connCreated = false;
    bool subCreated = false;
    bool MIcreated = false;
    OpcUa_CreateMonitoredItemsResponse* miRespServer = NULL;
    SOPC_ClientHelper_Subscription* sub = NULL;

    // Create a subscription
    if (SOPC_STATUS_OK == status)
    {
        connCreated = true;
        sub = create_subscription(secureConnection);
        status = (NULL == sub ? SOPC_STATUS_NOK : status);
    }

    // Create a MI for events on Server node with all clauses
    if (SOPC_STATUS_OK == status)
    {
        subCreated = true;
        miRespServer = create_monitored_item_event(sub);
        status = (NULL == miRespServer ? SOPC_STATUS_NOK : status);
    }

    if (SOPC_STATUS_OK == status)
    {
        MIcreated = true;
        // Check response content + revised queue size
        status = check_monitored_items(miRespServer);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_CONSOLE_PRINTF("\n=========== Test setup complete. Starting tests\n");

        nbFailures = 0;
        SOPC_CONSOLE_PRINTF("\n=========== TC 1: Normal 'None' connection (Success case) ===========\n");
        // launch a test.
        system("./s2opc_read --none -n i=2259 -a 13");

        status = exec_audit_test(&audit_NoneOk[0], 2000);

        SOPC_CONSOLE_PRINTF("\n=========== TC 2: connection with user/pass (Success case) ===========\n");
        // launch a test.
        system("TEST_USERNAME=user1 TEST_PASSWORD_USER=password ./s2opc_wrapper_connect 2");

        status = exec_audit_test(&audit_User1Ok[0], 2000);

        SOPC_CONSOLE_PRINTF("\n=========== TC 3: connection with bad user/pass (ActivateSession failed) ===========\n");
        // launch a test.
        system("TEST_USERNAME=user1 TEST_PASSWORD_USER=PASSword ./s2opc_wrapper_connect 2");

        status = exec_audit_test(&audit_User1_BadPass[0], 2000);

        SOPC_CONSOLE_PRINTF(
            "\n=========== TC 4: connection with bad server certificate (OpenSecureChannel failed) ===========\n");
        // launch a test.
        system("TEST_USERNAME=user1 TEST_PASSWORD_USER=password ./s2opc_wrapper_connect 4");

        status = exec_audit_test(&audit_User1_BadServerCertificate[0], 2000);

        SOPC_CONSOLE_PRINTF("\n=========== End of tests. Tear down\n");
    }
    else
    {
        SOPC_CONSOLE_PRINTF("\n=========== Test setup FAILED. Tests will NOT be executed\n");
    }

    SOPC_ReturnStatus tmpStatus = SOPC_STATUS_NOK;
    // Delete MIs at the end of test if flagged as needed (even in case of unexpected failure)
    if (MIcreated)
    {
        tmpStatus = delete_monitored_items(sub, &miRespServer);
        if (SOPC_STATUS_OK == tmpStatus)
        {
            MIcreated = false;
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

    // Clean up registered events (sEvtReceived)
    while (true)
    {
        EventContent* evt = (EventContent*) SOPC_SLinkedList_PopHead(sEvtReceived);
        if (evt == NULL)
            break;
        EventContent_Clear(evt);
    }
    SOPC_SLinkedList_Clear(sEvtReceived);

    if (SOPC_STATUS_OK == status && 0 == nbFailures)
    {
        printf(">>Test_Client_Toolkit_Audit final result: OK\n");
    }
    else
    {
        SOPC_CONSOLE_PRINTF(">>Test_Client_Toolkit_Audit final result: NOK (BAD status: %" PRIu32
                            " or incorrect result(s): %" PRIi32 ")\n",
                            status, nbFailures);
    }
    return (0 == nbFailures ? (int) status : 1);
}
