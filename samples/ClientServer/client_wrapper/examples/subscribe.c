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
 * \brief A subscribe example using the high-level client API
 *
 * Requires the toolkit_demo_server to be running.
 * Connect to the server, create a subscription and monitors the value of the given node.
 * Then disconnect and closes the toolkit.
 *
 */

#include <stdio.h>

#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_new_client.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include <signal.h>
#include <stdlib.h>

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"
#define DEFAULT_CONFIG_ID "read"

// Period used to check for catch signal
#define UPDATE_TIMEOUT_MS 500
#define SET_SUBSCRIBE_TIMEOUT_ENV "SET_SUBSCRIBE_TIMEOUT"

// Flag used on sig stop
static int32_t stop = 0;

/*
 * Management of Ctrl-C to stop the client (callback on stop signal)
 */
static void StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    SOPC_UNUSED_ARG(sig);

    /*
     * Signal steps:
     * - 1st signal: wait for stop flag to be seen and gentle stop
     * - 2rd signal: abrupt exit with error code '1'
     */
    if (stop > 0)
    {
        exit(1);
    }
    else
    {
        stop++;
    }
}

static bool AskKeyPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Password for client key:", outPassword);
}

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);

    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    printf("ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event, status);
}

static void SubscriptionNotification_Cb(const SOPC_ClientHelper_Subscription* subscription,
                                        SOPC_StatusCode status,
                                        SOPC_EncodeableType* notificationType,
                                        uint32_t nbNotifElts,
                                        const void* notification,
                                        uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);

    if (SOPC_IsGoodStatus(status) && &OpcUa_DataChangeNotification_EncodeableType == notificationType)
    {
        const OpcUa_DataChangeNotification* notifs = (const OpcUa_DataChangeNotification*) notification;
        for (uint32_t i = 0; i < nbNotifElts; i++)
        {
            printf("Value change notification for node %s:\n", (char*) monitoredItemCtxArray[i]);
            SOPC_Variant_Print(&notifs->MonitoredItems[i].Value.Value);
            printf("\n");
        }
    }
}

int main(int argc, char* const argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <nodeId> [<nodeId>] (e.g. %s \"ns=1;i=1012\").\nThe '" DEFAULT_CONFIG_ID
               "' connection configuration "
               "from " DEFAULT_CLIENT_CONFIG_XML " is used.\n",
               argv[0], argv[0]);
        return -2;
    }
    int res = 0;

    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, StopSignal);
    signal(SIGTERM, StopSignal);

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_subscribe_logs/";
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
        status = SOPC_ClientConfigHelper_ConfigureFromXML(DEFAULT_CLIENT_CONFIG_XML, NULL, &nbConfigs, &scConfigArray);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_subscribe: failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* readConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        readConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == readConnCfg)
        {
            printf("<Example_wrapper_subscribe: failed to load configuration id '" DEFAULT_CONFIG_ID
                   "' from XML config file %s\n",
                   DEFAULT_CLIENT_CONFIG_XML);

            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&AskKeyPass_FromTerminal);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_Connect(readConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_subscribe: Failed to connect\n");
        }
    }

    /* create a subscription */
    SOPC_ClientHelper_Subscription* subscription = NULL;
    if (SOPC_STATUS_OK == status)
    {
        subscription =
            SOPC_ClientHelperNew_CreateSubscription(secureConnection, SOPC_CreateSubscriptionRequest_CreateDefault(),
                                                    SubscriptionNotification_Cb, (uintptr_t) NULL);
        if (NULL == subscription)
        {
            status = SOPC_STATUS_NOK;
            printf("<Example_wrapper_subscribe: Failed to create subscription\n");
        }
        else
        {
            double revisedPublishingInterval = 0;
            uint32_t revisedLifetimeCount = 0;
            uint32_t revisedMaxKeepAliveCount = 0;
            SOPC_ReturnStatus localStatus = SOPC_ClientHelperNew_Subscription_GetRevisedParameters(
                subscription, &revisedPublishingInterval, &revisedLifetimeCount, &revisedMaxKeepAliveCount);
            if (SOPC_STATUS_OK == localStatus)
            {
                printf("Creation of subscription succeeded with revised parameters pubItv=%f lifetimeCpt=%" PRIu32
                       " keepAliveCpt=%" PRIu32 ".\n",
                       revisedPublishingInterval, revisedLifetimeCount, revisedMaxKeepAliveCount);
            }
            else
            {
                printf("<Example_wrapper_subscribe: Failed to retrieve revised parameters of created subscription\n");
            }
        }
    }

    // Prepare the creation request for monitored items
    OpcUa_CreateMonitoredItemsRequest* createMIreq = NULL;
    if (SOPC_STATUS_OK == status)
    {
        createMIreq = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(0, (size_t)(argc - 1), &argv[1],
                                                                                OpcUa_TimestampsToReturn_Both);
        if (NULL == createMIreq)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // Create the monitored items
    // Response is necessary to know if creation succeeded or not
    OpcUa_CreateMonitoredItemsResponse createMIresp;
    OpcUa_CreateMonitoredItemsResponse_Initialize(&createMIresp);
    if (SOPC_STATUS_OK == status)
    {
        // Our context is the array of node ids
        const uintptr_t* nodeIdsCtxArray = (const uintptr_t*) &argv[1];
        status = SOPC_ClientHelperNew_Subscription_CreateMonitoredItems(subscription, createMIreq, nodeIdsCtxArray,
                                                                        &createMIresp);
        if (SOPC_STATUS_OK != status)
        {
            OpcUa_CreateMonitoredItemsRequest_Clear(createMIreq);
            SOPC_Free(createMIreq);
            printf("<Example_wrapper_subscribe: Failed to create monitored items\n");
        }
        else
        {
            bool oneSucceeded = false;
            for (int32_t i = 0; i < createMIresp.NoOfResults; i++)
            {
                if (SOPC_IsGoodStatus(createMIresp.Results[i].StatusCode))
                {
                    oneSucceeded = true;
                    printf("Creation of monitored item for node %s succeeded\n", argv[i + 1]);
                }
                else
                {
                    printf("Creation of monitored item for node %s: failed with StatusCode: 0x%08" PRIX32 "\n",
                           argv[i + 1], createMIresp.Results[i].StatusCode);
                }
            }
            if (!oneSucceeded)
            {
                status = SOPC_STATUS_WOULD_BLOCK;
            }
        }
    }

    // Check the timeout flag for from environment variable used for test purpose
    const char* val = getenv(SET_SUBSCRIBE_TIMEOUT_ENV);
    unsigned int SUBSCRIBE_TIMEOUT_CYCLES = 0;
    if (NULL != val)
    {
        SUBSCRIBE_TIMEOUT_CYCLES = 4;
    }

    // Wait for stop signal
    unsigned int cpt = 0;
    while (SOPC_STATUS_OK == status && !stop && (0 == SUBSCRIBE_TIMEOUT_CYCLES || cpt < SUBSCRIBE_TIMEOUT_CYCLES))
    {
        cpt++;
        SOPC_Sleep(UPDATE_TIMEOUT_MS);
    }

    // Prepare the delete request for monitored items
    OpcUa_DeleteMonitoredItemsRequest* deleteMIreq = NULL;
    if (SOPC_STATUS_OK == status)
    {
        deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, (size_t) createMIresp.NoOfResults, NULL);
        if (NULL != deleteMIreq)
        {
            for (size_t i = 0; SOPC_STATUS_OK == status && i < (size_t) createMIresp.NoOfResults; i++)
            {
                status = SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId(deleteMIreq, i,
                                                                             createMIresp.Results[i].MonitoredItemId);
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    // Clear the create monitored items response kept for the monitored item ids
    OpcUa_CreateMonitoredItemsResponse_Clear(&createMIresp);

    // Delete the monitored items
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_DeleteMonitoredItemsResponse deleteMIresp;
        OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);

        // Our context is the array of node ids
        status = SOPC_ClientHelperNew_Subscription_DeleteMonitoredItems(subscription, deleteMIreq, &deleteMIresp);
        if (SOPC_STATUS_OK != status)
        {
            OpcUa_DeleteMonitoredItemsRequest_Clear(deleteMIreq);
            SOPC_Free(deleteMIreq);
            printf("<Example_wrapper_subscribe: Failed to delete monitored items\n");
        }
        else
        {
            for (int32_t i = 0; i < deleteMIresp.NoOfResults; i++)
            {
                if (!SOPC_IsGoodStatus(deleteMIresp.Results[i]))
                {
                    printf("Deletion of monitored item for node %s: failed with StatusCode: 0x%08" PRIX32 "\n",
                           argv[i + 1], deleteMIresp.Results[i]);
                }
            }
        }

        OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);
    }

    // Close the subscription
    if (NULL != subscription)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelperNew_DeleteSubscription(&subscription);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_subscribe: Failed to delete subscription\n");
        }
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelperNew_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_subscribe: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
