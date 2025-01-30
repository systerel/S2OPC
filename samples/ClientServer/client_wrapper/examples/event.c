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
 * \brief An event reader example using the high-level client API
 *
 * Requires any event-supporteed server to be running (e.g.toolkit_demo_server).
 * Connect to the server and reads some events for a given duration.
 * Then disconnect and closes the toolkit.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

#include "opcua_identifiers.h"

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"
#define DEFAULT_CONFIG_ID "event"

#define APP_NAME "s2opc_wrapper_event"
#define LOG_DIR "./" APP_NAME "_logs/"

#define TEST_PASSWORD_PRIVATE_KEY "TEST_PASSWORD_PRIVATE_KEY"
#define TEST_USERNAME "TEST_USERNAME"
#define TEST_PASSWORD_USER "TEST_PASSWORD_USER"
#define TEST_CLIENT_CONFIG_XML "TEST_CLIENT_CONFIG_XML"
#define TEST_CLIENT_CONFIG_ID "TEST_CLIENT_CONFIG_ID"

#define MAX_ITEMS 256

static int32_t gQuit = 0;

static const SOPC_NodeId serverObjectId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server);
static const SOPC_NodeId hasProperty = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasProperty);

typedef struct
{
    char* path;
    SOPC_NodeId* typeId;
} SelectClause_Cfg;

static SOPC_ClientHelper_Subscription* sub;
typedef struct
{
    SelectClause_Cfg selectClauses[MAX_ITEMS];
    OpcUa_CreateMonitoredItemsResponse* miRespServer;
    size_t nbClauses;
} MonitoredItemCtxt;

static MonitoredItemCtxt monitoredItemCtxt;

static const char* monitoredItemNames[MAX_ITEMS] = {0};
static size_t nbMINames = 0;

/******************************************************************************
 *  CALLBACKS
 ******************************************************************************/
static bool askUserPass_FromEnv(const SOPC_SecureConnection_Config* secConnConfig,
                                char** outUserName,
                                char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    SOPC_ASSERT(NULL != outUserName);
    SOPC_ASSERT(NULL != outPassword);

    char* _outUser = getenv(TEST_USERNAME);
    if (NULL == _outUser)
    {
        printf("<" APP_NAME ": The following environment variable is missing: %s\n", TEST_USERNAME);
        return false;
    }

    char* _outPassword = getenv(TEST_PASSWORD_USER);
    if (NULL == _outPassword)
    {
        printf("<" APP_NAME ": The following environment variable is missing: %s\n", TEST_PASSWORD_USER);
        return false;
    }

    printf("Used user from environment variable %s/%s\n", TEST_USERNAME, TEST_PASSWORD_USER);
    *outUserName = SOPC_strdup(_outUser);     // Do a copy
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    return true;
}

static bool askPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        We have to make a copy here because in any case, we will free the password and not distinguish if it come
        from environement or terminal after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
    */
    char* _outPassword = getenv(TEST_PASSWORD_PRIVATE_KEY);
    if (NULL == _outPassword)
    {
        printf("<" APP_NAME ": The following environment variable is missing: %s\n", TEST_PASSWORD_PRIVATE_KEY);
        return false;
    }
    *outPassword = SOPC_strdup(_outPassword); // Do a copy

    return NULL != *outPassword;
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

    if (!SOPC_IsGoodStatus(status))
    {
        return;
    }

    SOPC_Buffer* buf = SOPC_Buffer_CreateResizable(1024, 1024 * 16);

    const MonitoredItemCtxt* userCtx =
        (const MonitoredItemCtxt*) SOPC_ClientHelper_Subscription_GetUserParam(subscription);
    SOPC_ASSERT(NULL == userCtx);

    if (&OpcUa_EventNotificationList_EncodeableType == notificationType)
    {
        const OpcUa_EventNotificationList* eventList = (const OpcUa_EventNotificationList*) notification;
        bool localExpectedContentReceived = ((int32_t) nbNotifElts == eventList->NoOfEvents);

        for (int32_t i = 0; localExpectedContentReceived && i < eventList->NoOfEvents; i++)
        {
            const OpcUa_EventFieldList* event = &eventList->Events[i];

            SOPC_CONSOLE_PRINTF("----- New event -----\n");

            // Preformat the event output in a new entry of sEvtReceived
            for (int32_t iField = 0; iField < event->NoOfEventFields; iField++)
            {
                const SOPC_Variant* var = &event->EventFields[iField];
                if (var->BuiltInTypeId == SOPC_Null_Id)
                    continue;
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

                const char* clauseStr = monitoredItemCtxt.selectClauses[iField].path;
                SOPC_CONSOLE_PRINTF("  {\"%s\", \"%s\"},\n", (clauseStr ? clauseStr : "<null>"),
                                    (const char*) buf->data);
            }
        }
    }
    SOPC_Buffer_Delete(buf);
}

/******************************************************************************
 *  BROWSE MANAGEMENT
 ******************************************************************************/
static SOPC_ReturnStatus browse(SOPC_ClientConnection* secureConnection, const char* name)
{
    OpcUa_BrowseRequest* browseRequest = NULL;
    OpcUa_BrowseResponse* browseResponse = NULL;
    SOPC_ReturnStatus status;
    browseRequest = SOPC_BrowseRequest_Create(1, 0, NULL);

    printf("Browsing node'%s' ...\n", name);

    if (NULL != browseRequest)
    {
        status = SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
            browseRequest, 0, name, OpcUa_BrowseDirection_Forward, NULL, true, 0,
            OpcUa_BrowseResultMask_ReferenceTypeId | OpcUa_BrowseResultMask_BrowseName);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, browseRequest, (void**) &browseResponse);
    }

    // Extract results
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(browseResponse->ResponseHeader.ServiceResult) && 1 == browseResponse->NoOfResults)
        {
            OpcUa_BrowseResult* result = &browseResponse->Results[0];

            if (SOPC_IsGoodStatus(result->StatusCode))
            {
                for (int32_t i = 0; i < result->NoOfReferences; i++)
                {
                    OpcUa_ReferenceDescription* ref = &result->References[i];
                    if (SOPC_NodeId_Equal(&hasProperty, &ref->ReferenceTypeId))
                    {
                        SelectClause_Cfg* cfg = &monitoredItemCtxt.selectClauses[monitoredItemCtxt.nbClauses];
                        char* nid = SOPC_NodeId_ToCString(&ref->NodeId.NodeId);
                        cfg->path = SOPC_QualifiedName_ToCString(&ref->BrowseName);
                        SOPC_NodeId_Copy(cfg->typeId, &ref->NodeId.NodeId);
                        printf("- clause: %s (%s) \n", nid, cfg->path);
                        monitoredItemCtxt.nbClauses++;
                        SOPC_Free(nid);
                    }
                }
            }
        }
        else
        {
            printf("Unexpected BROWSE answer content!\n");
            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != browseResponse)
    {
        SOPC_EncodeableObject_Delete(browseResponse->encodeableType, (void**) &browseResponse);
    }

    return status;
}

/******************************************************************************
 *  SUBSCRIPTION MANAGEMENT
 ******************************************************************************/

static SOPC_ClientHelper_Subscription* create_subscription(SOPC_ClientConnection* connection)
{
    OpcUa_CreateSubscriptionRequest* createSubReq = SOPC_CreateSubscriptionRequest_Create(500, 6, 2, 1000, true, 0);
    if (NULL == createSubReq)
    {
        return NULL;
    }
    /* In case the subscription service shall not be supported, check service response is unsupported service*/
    SOPC_ClientHelper_Subscription* subscription =
        SOPC_ClientHelper_CreateSubscription(connection, createSubReq, SOPC_Client_SubscriptionNotification_Cb, 0);
    return subscription;
}

static OpcUa_CreateMonitoredItemsResponse* create_monitored_item_event(void)
{
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
        eventFilter = SOPC_MonitoredItem_CreateEventFilter(monitoredItemCtxt.nbClauses, nbWhereClauseElts);
    }

    if (NULL != eventFilter)
    {
        for (size_t i = 0; i < monitoredItemCtxt.nbClauses && SOPC_STATUS_OK == status; i++)
        {
            const SelectClause_Cfg* clause = &monitoredItemCtxt.selectClauses[i];
            char* path = SOPC_NodeId_ToCString(clause->typeId);
            status = SOPC_EventFilter_SetSelectClauseFromStringPath(eventFilter, i, path, '~', clause->path,
                                                                    SOPC_AttributeId_Value, NULL);
            SOPC_Free(path);
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
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(sub, createMonItReq, 0, createMonItResp);
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
                SOPC_CONSOLE_PRINTF(">>" APP_NAME ": CreateMonitoredItemsResponse[%" PRIi32
                                    "] result not good: 0x%08" PRIX32 "\n",
                                    0, createMonItResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            SOPC_CONSOLE_PRINTF(">>" APP_NAME ": Unexpected number of MI in CreateMonitoredItemsResponse %" PRIi32
                                " != 1\n",
                                createMonItResp->NoOfResults);
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        SOPC_CONSOLE_PRINTF(">>" APP_NAME ": CreateMonitoredItemsResponse global result not good: 0x%08" PRIX32 "\n",
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
            SOPC_CONSOLE_PRINTF(">>" APP_NAME
                                ": Unexpected CreateMonitoredItemsResponse[0] filter encoding type: "
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
    if (NULL == createMonItResp)
    {
        return SOPC_STATUS_OK;
    }
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

static SOPC_ReturnStatus creatEventSubscription(SOPC_ClientConnection* secureConnection)
{
    SOPC_ASSERT(NULL != secureConnection);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Create a subscription
    if (sub == NULL)
    {
        sub = create_subscription(secureConnection);
        status = (NULL == sub ? SOPC_STATUS_NOK : status);
    }

    // Create a MI for events on Server node with all clauses
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL == monitoredItemCtxt.miRespServer);
        monitoredItemCtxt.miRespServer = create_monitored_item_event();
        status = (NULL == monitoredItemCtxt.miRespServer ? SOPC_STATUS_NOK : status);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Check response content + revised queue size
        status = check_monitored_items(monitoredItemCtxt.miRespServer);
    }

    return status;
}

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);

    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    printf("ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event, status);
    SOPC_Atomic_Int_Set(&gQuit, 1);
}

/******************************************************************************
 *  MAIN ENTRY
 ******************************************************************************/
int main(int argc, char* const argv[])
{
    /* Get Use XML config file */
    char* xmlCfg = getenv(TEST_CLIENT_CONFIG_XML);
    if (NULL == xmlCfg)
    {
        xmlCfg = DEFAULT_CLIENT_CONFIG_XML;
    }
    char* xmlSectionId = getenv(TEST_CLIENT_CONFIG_ID);
    if (NULL == xmlSectionId)
    {
        xmlSectionId = DEFAULT_CONFIG_ID;
    }

    if (argc < 2)
    {
        printf(
            "Usage: %s <EventTypeNodeId> [-<MaxWaitSec>]\nThe <TEST_CLIENT_CONFIG_ID>='%s"
            "' connection configuration from <TEST_CLIENT_CONFIG_XML>='%s' is used. Default values: '" DEFAULT_CONFIG_ID
            "'/'" DEFAULT_CLIENT_CONFIG_XML
            "'\n"
            " - <EventTypeNodeId> is the base event type to listen events from (multiple values supported).\n"
            "- If <MaxWaitSec> is skipped or zero, the process will never stop.\n"
            "E.g. %s 'i=2041' 'i=2052' 'i=2058' -10   (for BaseEventType + AuditEventType "
            "+ AuditSecurityEventType for 10 seconds)\n",
            argv[0], xmlSectionId, xmlCfg, argv[0]);
        return -2;
    }

    printf("< User configuration file '%s', section'%s' \n", xmlCfg, xmlSectionId);

    nbMINames = 0;

    uint64_t maxWaitS = 0;

    for (int argi = 1; argi < argc; argi++)
    {
        const char* arg = argv[argi];
        if (*arg == 0)
            continue;
        if (*arg == '-')
        {
            arg++; // Skip the '-' char
            maxWaitS = (uint64_t) strtol(arg, NULL, 10);
        }
        else
        {
            monitoredItemNames[nbMINames] = arg;
            nbMINames++;
        }
    }

    SOPC_Atomic_Int_Set(&gQuit, 0);
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = LOG_DIR;
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;

    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    size_t nbConfigs = 0;
    SOPC_SecureConnection_Config** scConfigArray = NULL;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_ConfigureFromXML(xmlCfg, NULL, &nbConfigs, &scConfigArray);

        if (SOPC_STATUS_OK != status)
        {
            printf("<" APP_NAME ": failed to load XML config file %s\n", xmlCfg);
        }
    }

    SOPC_SecureConnection_Config* connCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        connCfg = SOPC_ClientConfigHelper_GetConfigFromId(xmlSectionId);

        if (NULL == connCfg)
        {
            printf("<" APP_NAME ": failed to load configuration id '%s' from XML config file %s\n", xmlSectionId,
                   xmlCfg);

            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&askPass_FromEnv);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&askUserPass_FromEnv);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(connCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<" APP_NAME ": Failed to connect, status = %d.\n", (int) status);
        }
    }

    /* Search for events to monitor, based on NodeId provided */
    for (size_t i = 0; SOPC_STATUS_OK == status && i < nbMINames; i++)
    {
        status = browse(secureConnection, monitoredItemNames[i]);
        if (SOPC_STATUS_OK != status)
        {
            printf("<" APP_NAME ": Failed to Find Events from node '%s'.\n", monitoredItemNames[i]);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("Found %u events to monitor.\n", (unsigned) monitoredItemCtxt.nbClauses);
        status = creatEventSubscription(secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<" APP_NAME ": Failed to Create subscription, status = %d.\n", (int) status);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("Client connected. Waiting for Audit events\n");
        if (maxWaitS > 0)
        {
            printf("(Process will stop after %d seconds).\n", (int) maxWaitS);
        }

        SOPC_TimeReference t0 = SOPC_TimeReference_GetCurrent();
        while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&gQuit) == 0)
        {
            if (maxWaitS > 0)
            {
                SOPC_TimeReference now = SOPC_TimeReference_GetCurrent();
                if (now > t0 + maxWaitS * 1000)
                {
                    SOPC_Atomic_Int_Set(&gQuit, 1);
                }
            }
            SOPC_Sleep(50);
        }
    }

    /* Clear internal objects */
    delete_monitored_items(sub, &monitoredItemCtxt.miRespServer);
    for (size_t iCl = 0; iCl < monitoredItemCtxt.nbClauses; iCl++)
    {
        SelectClause_Cfg* sel = &monitoredItemCtxt.selectClauses[iCl];
        SOPC_Free(sel->path);
        SOPC_NodeId_Clear(sel->typeId);
        SOPC_Free(sel->typeId);
    }

    if (NULL != sub)
    {
        SOPC_ClientHelper_DeleteSubscription(&sub);
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_read: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
