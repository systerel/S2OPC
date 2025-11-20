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
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

/*---------------------------------------------------------------------------
 *                           Global variables
 *---------------------------------------------------------------------------*/

static const char* SERVER_URL = "opc.tcp://localhost:4841";

// Handle timeout on receive events
static const uint32_t sleepTime = 200;
static const uint32_t timeout = 10000;

// Flags and counter of received notifs
static int32_t nbrNotifsOnChangingNode = 0;
static bool receivedNotifOnSecondNode = false;
static bool receivedNotifOnThirdNode = false;

// NodeIds monitored
static char* nodeId1[1] = {"s=Counter"};
static char* nodeId2[1] = {"i=1009"};
static char* nodeId3[1] = {"i=1003"};

/*---------------------------------------------------------------------------
 *                       Client config and callbacks
 *---------------------------------------------------------------------------*/

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
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_multi_sub_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    return status;
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

static void SubscriptionNotification_Cb1(const SOPC_ClientHelper_Subscription* subscription,
                                         SOPC_StatusCode status,
                                         SOPC_EncodeableType* notificationType,
                                         uint32_t nbNotifElts,
                                         const void* notification,
                                         uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);

    printf("Received notif on callback1 ! (used for subscription1)\n");
    SOPC_Atomic_Int_Add(&nbrNotifsOnChangingNode, 1);

    if (SOPC_IsGoodStatus(status) && &OpcUa_DataChangeNotification_EncodeableType == notificationType)
    {
        const OpcUa_DataChangeNotification* notifs = (const OpcUa_DataChangeNotification*) notification;
        for (uint32_t i = 0; i < nbNotifElts; i++)
        {
            printf("Value change notification. New value:\n");
            SOPC_Variant_Print(&notifs->MonitoredItems[i].Value.Value);
            printf("\n");
        }
    }
}

static void SubscriptionNotification_Cb2(const SOPC_ClientHelper_Subscription* subscription,
                                         SOPC_StatusCode status,
                                         SOPC_EncodeableType* notificationType,
                                         uint32_t nbNotifElts,
                                         const void* notification,
                                         uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);

    printf("Received notif on callback2 ! (used for subscription2)\n");
    receivedNotifOnSecondNode = true;
    if (SOPC_IsGoodStatus(status) && &OpcUa_DataChangeNotification_EncodeableType == notificationType)
    {
        const OpcUa_DataChangeNotification* notifs = (const OpcUa_DataChangeNotification*) notification;
        for (uint32_t i = 0; i < nbNotifElts; i++)
        {
            printf("Value change notification. New value:\n");
            SOPC_Variant_Print(&notifs->MonitoredItems[i].Value.Value);
            printf("\n");
        }
    }
}

static void SubscriptionNotification_Cb3(const SOPC_ClientHelper_Subscription* subscription,
                                         SOPC_StatusCode status,
                                         SOPC_EncodeableType* notificationType,
                                         uint32_t nbNotifElts,
                                         const void* notification,
                                         uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);

    printf("Received notif on callback3 ! (used for subscription3)\n");
    receivedNotifOnThirdNode = true;
    if (SOPC_IsGoodStatus(status) && &OpcUa_DataChangeNotification_EncodeableType == notificationType)
    {
        const OpcUa_DataChangeNotification* notifs = (const OpcUa_DataChangeNotification*) notification;
        for (uint32_t i = 0; i < nbNotifElts; i++)
        {
            printf("Value change notification. New value:\n");
            SOPC_Variant_Print(&notifs->MonitoredItems[i].Value.Value);
            printf("\n");
        }
    }
}

static SOPC_ReturnStatus Client_LoadAnonClientConfiguration(SOPC_SecureConnection_Config** scConfig)
{
    /* Create the secure connection and set parameters */
    SOPC_SecureConnection_Config* scConfigBuilt = SOPC_ClientConfigHelper_CreateSecureConnection(
        "0", SERVER_URL, OpcUa_MessageSecurityMode_None, SOPC_SecurityPolicy_None);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (NULL != scConfigBuilt)
    {
        status = SOPC_SecureConnectionConfig_SetAnonymous(scConfigBuilt, "anonymous");
        if (SOPC_STATUS_OK == status)
        {
            *scConfig = scConfigBuilt;
        }
    }
    return status;
}

static SOPC_ReturnStatus clear_mi(SOPC_ClientHelper_Subscription* sub, uint32_t MIid)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    // Prepare the delete request for monitored items
    OpcUa_DeleteMonitoredItemsRequest* deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, 1, &MIid);
    if (NULL != deleteMIreq)
    {
        OpcUa_DeleteMonitoredItemsResponse deleteMIresp;
        OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
        status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(sub, deleteMIreq, &deleteMIresp);
        if (SOPC_STATUS_OK != status)
        {
            printf("Failed to delete monitored item\n");
        }
        else
        {
            if (!SOPC_IsGoodStatus(deleteMIresp.Results[0]))
            {
                status = SOPC_STATUS_NOK;
                printf("Deletion of monitored item failed with StatusCode: 0x%08" PRIX32 "\n", deleteMIresp.Results[0]);
            }
            else
            {
                status = SOPC_STATUS_OK;
                printf("Success deleting MI\n");
            }
        }
        OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);
    }
    return status;
}

static uint32_t create_mi(SOPC_ClientHelper_Subscription* sub, char* const* nodeIdsToMonitor)
{
    uint32_t MIid = 0;
    // Prepare the creation request for monitored items
    OpcUa_CreateMonitoredItemsRequest* createMIreq = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
        0, 1, nodeIdsToMonitor, OpcUa_TimestampsToReturn_Both);

    SOPC_ReturnStatus status = (NULL != createMIreq ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);

    // Create the monitored items
    // Response is necessary to know if creation succeeded or not
    OpcUa_CreateMonitoredItemsResponse createMIresp;
    OpcUa_CreateMonitoredItemsResponse_Initialize(&createMIresp);
    if (SOPC_STATUS_OK == status)
    {
        // Our context is the array of node ids
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(sub, createMIreq, 0, &createMIresp);
        if (SOPC_STATUS_OK != status)
        {
            printf("Failed to create monitored item.\n");
        }
        else
        {
            SOPC_ASSERT(1 == createMIresp.NoOfResults);
            if (SOPC_IsGoodStatus(createMIresp.Results[0].StatusCode))
            {
                printf("Creation of monitored item succeeded.\n");
                MIid = createMIresp.Results[0].MonitoredItemId;
            }
            else
            {
                printf("Creation of monitored item failed with StatusCode: 0x%08" PRIX32 "\n",
                       createMIresp.Results[0].StatusCode);
            }
        }
    }
    OpcUa_CreateMonitoredItemsResponse_Clear(&createMIresp);
    return MIid;
}

static SOPC_ClientHelper_Subscription* create_sub(SOPC_ClientConnection* secureConnection,
                                                  double reqPublishingInterval,
                                                  SOPC_ClientSubscriptionNotification_Fct* subNotifCb)
{
    double revisedPublishingInterval = 0;
    uint32_t revisedLifetimeCount = 0;
    uint32_t revisedMaxKeepAliveCount = 0;

    SOPC_ClientHelper_Subscription* sub = NULL;
    OpcUa_CreateSubscriptionRequest* createSubReq =
        SOPC_CreateSubscriptionRequest_Create(reqPublishingInterval, 6, 2, 1000, true, 0);
    if (NULL != createSubReq)
    {
        sub = SOPC_ClientHelper_CreateSubscription(secureConnection, createSubReq, subNotifCb, (uintptr_t) NULL);
        if (NULL == sub)
        {
            printf("Failed to create subscription\n");
        }
        else
        {
            SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Subscription_GetRevisedParameters(
                sub, &revisedPublishingInterval, &revisedLifetimeCount, &revisedMaxKeepAliveCount);
            if (SOPC_STATUS_OK == localStatus)
            {
                printf("Creation of subscription succeeded with revised parameters pubItv=%f lifetimeCpt=%" PRIu32
                       " keepAliveCpt=%" PRIu32 ".\n",
                       revisedPublishingInterval, revisedLifetimeCount, revisedMaxKeepAliveCount);
            }
            else
            {
                printf("Fail to get revised parameters of the subscription.\n");
            }
        }
    }
    return sub;
}

static bool clear_mi_and_sub(uint32_t MIid1,
                             uint32_t MIid2,
                             SOPC_ClientHelper_Subscription* subscription1,
                             SOPC_ClientHelper_Subscription* subscription2,
                             SOPC_ClientHelper_Subscription* subscription3)
{
    bool res = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (0 < MIid1)
    {
        status = clear_mi(subscription1, MIid1);
        if (SOPC_STATUS_OK != status)
        {
            res = false;
            printf("Clear MI 1 failed with status code status %d.\n", status);
        }
    }
    if (NULL != subscription1)
    {
        status = SOPC_ClientHelper_DeleteSubscription(&subscription1);
        if (SOPC_STATUS_OK != status)
        {
            res = false;
            printf("Clear Subscription 1 failed with status code status %d.\n", status);
        }
        else
        {
            printf("Clear Subscription 1 success.\n");
        }
    }

    if (0 < MIid2)
    {
        status = clear_mi(subscription2, MIid2);
        if (SOPC_STATUS_OK != status)
        {
            res = false;
            printf("Clear MI 2 failed with status code status %d.\n", status);
        }
    }
    if (NULL != subscription2)
    {
        status = SOPC_ClientHelper_DeleteSubscription(&subscription2);
        if (SOPC_STATUS_OK != status)
        {
            res = false;
            printf("Clear Subscription 2 failed with status code status %d.\n", status);
        }
        else
        {
            printf("Clear Subscription 2 success.\n");
        }
    }

    if (NULL != subscription3)
    {
        // Delete sub without deling before its MIs.
        status = SOPC_ClientHelper_DeleteSubscription(&subscription3);
        if (SOPC_STATUS_OK != status)
        {
            res = false;
            printf("Clear Subscription 3 failed with status code status %d.\n", status);
        }
        else
        {
            printf("Clear Subscription 3 success.\n");
        }
    }
    return res;
}

/*---------------------------------------------------------------------------
 *                                 MAIN
 *---------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    SOPC_SecureConnection_Config* scConfig = NULL;
    SOPC_ClientConnection* secureConnection = NULL;
    SOPC_ReturnStatus status = Client_Initialize();

    if (SOPC_STATUS_OK == status)
    {
        status = Client_LoadAnonClientConfiguration(&scConfig);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != scConfig);
        status = SOPC_ClientHelper_Connect(scConfig, SOPC_Client_ConnEventCb, &secureConnection);
    }

    uint32_t MIid1 = 0;
    uint32_t MIid2 = 0;
    uint32_t MIid3 = 0;
    SOPC_ClientHelper_Subscription* subscription1 = NULL;
    SOPC_ClientHelper_Subscription* subscription2 = NULL;
    SOPC_ClientHelper_Subscription* subscription3 = NULL;
    // Create subscriptions and MIs
    if (SOPC_STATUS_OK == status)
    {
        subscription1 = create_sub(secureConnection, 400, SubscriptionNotification_Cb1);
        subscription2 = create_sub(secureConnection, 500, SubscriptionNotification_Cb2);
        subscription3 = create_sub(secureConnection, 800, SubscriptionNotification_Cb3);
        if (NULL == subscription1 || NULL == subscription2 || NULL == subscription3)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("Creating one MI on each subscription.\n");
        MIid1 = create_mi(subscription1, nodeId1);
        MIid2 = create_mi(subscription2, nodeId2);
        MIid3 = create_mi(subscription3, nodeId3);
        if (0 == MIid1 || 0 == MIid2 || 0 == MIid3)
        {
            printf("Failed to retrieve revised parameters of created subscription\n");
            status = SOPC_STATUS_NOK;
        }
    }

    /* Wait until event received */
    uint32_t loopCounter = 0;
    while ((SOPC_Atomic_Int_Get(&nbrNotifsOnChangingNode) < 5 || !receivedNotifOnSecondNode ||
            !receivedNotifOnThirdNode) &&
           loopCounter * sleepTime <= timeout)
    {
        loopCounter++;
        SOPC_Sleep(sleepTime);
    }
    if (SOPC_STATUS_OK == status && loopCounter * sleepTime > timeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    // Clear
    bool deleteRes = clear_mi_and_sub(MIid1, MIid2, subscription1, subscription2, subscription3);
    if (!deleteRes)
    {
        printf(">>Test_Client_Multi_Sub: fail at clearing MIs and subscriptions.\n");
    }

    /* Close the connection */
    SOPC_ReturnStatus discoStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
    if (SOPC_STATUS_OK != discoStatus)
    {
        printf(">>Test_Client_Multi_Sub: fail at disconnection.\n");
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    return (SOPC_STATUS_OK == status && deleteRes && SOPC_STATUS_OK == discoStatus) ? EXIT_SUCCESS : EXIT_FAILURE;
}
