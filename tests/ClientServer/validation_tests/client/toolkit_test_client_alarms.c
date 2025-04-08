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
#include "libs2opc_client_alarm_conditions.h"
#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_askpass.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"

/* Test the toolkit demo and test servers alarms. */

static const SOPC_NodeId serverObjectId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server);

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 200;
// Loop timeout in milliseconds
static const uint32_t loopTimeout = 50000;

// Boolean flags for TC 1
static int32_t nbrOfCyclesConfirmationCompleted = 0;   // read by main thread, written by callback thread
static int32_t nbrOfCyclesNoConfirmationCompleted = 0; // read by main thread, written by callback thread
static bool playTest1 = false;                         // written by main thread, read only by callback thread

static int32_t nbrOfDisabledAlarms = 0; // read by main thread, written by callback thread
static bool playTest2Part1 = false;     // written by main thread, read only by callback thread

static bool oneAlarmUpdated = false; // read by main thread, written by callback thread
static bool playTest2Part2 = false;  // written by main thread, read only by callback thread

static int32_t nbrOfEnabledAlarms = 0;   // read by main thread, written by callback thread
static bool playTest3 = false;           // written by main thread, read only by callback thread
static bool clearArrayOfEnabled = false; // written by main thread, read only by callback thread
static bool arrayCleared = false;        // read by main thread, written by callback thread

// ConditionsIds of the servers alarms, used for enabling from ConditionId
static const SOPC_NodeId alarm1_demo_server = SOPC_NODEID_NUMERIC(1, 5009);
static const SOPC_NodeId alarm2_demo_server = SOPC_NODEID_NUMERIC(1, 5011);
static const SOPC_NodeId alarm1_test_server = SOPC_NODEID_NUMERIC(1, 15010);
static const SOPC_NodeId alarm2_test_server = SOPC_NODEID_NUMERIC(1, 15011);
static const SOPC_NodeId alarm3_test_server = SOPC_NODEID_NUMERIC(1, 15016);
static const SOPC_NodeId alarm4_test_server = SOPC_NODEID_NUMERIC(1, 15017);

// Special events: Refresh event types
static SOPC_NodeId refreshStartTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_RefreshStartEventType);
static SOPC_NodeId refreshEndTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_RefreshEndEventType);
static SOPC_NodeId refreshRequiredTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_RefreshRequiredEventType);

#define MAX_DIFFERENT_ALARMS 4
static SOPC_NodeId null_NodeId = SOPC_NODEID_NS0_NUMERIC(0);
static SOPC_NodeId* enabledAlarmsArray[] = {&null_NodeId, &null_NodeId, &null_NodeId, &null_NodeId};

/*---------------------------------------------------------------------------
 *                          Client initialization
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
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_alarm_conditions_logs/";
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

static SOPC_ReturnStatus Client_LoadClientConfiguration(bool is_demo_server, SOPC_SecureConnection_Config** scConfig)
{
    const char* clientCertPath = "client_public/client_2k_cert.der";
    const char* clientKeyPath = "client_private/encrypted_client_2k_key.pem";
    const char* serverCertPath = "server_public/server_2k_cert.der";
    if (is_demo_server)
    {
        // 4k for demo server
        clientCertPath = "client_public/client_4k_cert.der";
        clientKeyPath = "client_private/encrypted_client_4k_key.pem";
        serverCertPath = "server_public/server_4k_cert.der";
    }

    bool encrypted = true;
    const char* clientPKIStorePath = "./S2OPC_Demo_PKI_Client";

    SOPC_PKIProvider* clientPKI = NULL;
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(clientCertPath, clientKeyPath, encrypted);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(clientPKIStorePath, &clientPKI);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(clientPKI);
    }

    /* Create the secure connection and set parameters */
    SOPC_SecureConnection_Config* scConfigBuilt = NULL;
    if (SOPC_STATUS_OK == status)
    {
        scConfigBuilt = SOPC_ClientConfigHelper_CreateSecureConnection(
            "0", "opc.tcp://localhost:4841", OpcUa_MessageSecurityMode_Sign, SOPC_SecurityPolicy_Basic256Sha256);
        if (NULL == scConfigBuilt)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(scConfigBuilt, serverCertPath);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (is_demo_server)
        {
            // username "me" for the user rights for demo server
            const char* password = getenv("TEST_PASSWORD_USER_ME");
            if (NULL == password)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
                printf("TEST_PASSWORD_USER_ME not set!\n");
            }
            else
            {
                status =
                    SOPC_SecureConnectionConfig_SetUserName(scConfigBuilt, "username_Basic256Sha256", "me", password);
            }
        }
        else
        {
            // username "user1" for the user rights for demo server
            const char* password = getenv("TEST_PASSWORD_USER");
            if (NULL == password)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
                printf("TEST_PASSWORD_USER not set!\n");
            }
            else
            {
                status = SOPC_SecureConnectionConfig_SetUserName(scConfigBuilt, "username_Basic256Sha256", "user1",
                                                                 password);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    if (SOPC_STATUS_OK == status)
    {
        *scConfig = scConfigBuilt;
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                  Callback when condition events received
 *---------------------------------------------------------------------------*/

// Add the NodeId in the list if it was not already present.
// Returns boolean indicating if is was present or not.
static bool add_nodeId_in_list(SOPC_NodeId** array_of_NodeIds, SOPC_NodeId* item_to_add)
{
    bool was_in_list = false;
    bool added_in_list = false;
    for (size_t i = 0; was_in_list == false && added_in_list == false && i < 4; i++)
    {
        SOPC_NodeId* current_item = array_of_NodeIds[i];
        bool isNullId = SOPC_NodeId_Equal(&null_NodeId, current_item);
        if (!isNullId)
        {
            bool isEqual = SOPC_NodeId_Equal(item_to_add, current_item);
            if (isEqual)
            {
                was_in_list = true;
            }
        }
        else
        {
            // add
            array_of_NodeIds[i] = item_to_add;
            added_in_list = true;
        }
    }
    return was_in_list;
}

static void SOPC_MonitoredAlarm_Event_Cb(SOPC_MonitoredAlarmsGroup* group,
                                         SOPC_MonitoredAlarm* alarmCond,
                                         SOPC_Event* rcvEvent)
{
    SOPC_UNUSED_ARG(group);
    const SOPC_NodeId* eventTypeId = SOPC_Event_GetEventTypeId(rcvEvent);
    // Ignore Refresh events. Will not be handled here in future version.
    if (SOPC_NodeId_Equal(eventTypeId, &refreshStartTypeId) || SOPC_NodeId_Equal(eventTypeId, &refreshEndTypeId) ||
        SOPC_NodeId_Equal(eventTypeId, &refreshRequiredTypeId))
    {
        SOPC_Event_Delete(&rcvEvent);
        return;
    }

    const SOPC_NodeId* conditionIdNodeId = SOPC_Event_GetNodeId(rcvEvent);
    char* conditionIdStr = SOPC_NodeId_ToCString(conditionIdNodeId);
    SOPC_CONSOLE_PRINTF("Received new event for alarm %s. States of the alarm:\n", conditionIdStr);
    SOPC_Free(conditionIdStr);
    // Get states for display
    bool confirmedState = false;
    bool confirmableAlarm = false;
    SOPC_ReturnStatus status = SOPC_MonitoredAlarm_GetConfirmedState(alarmCond, &confirmedState);
    if (SOPC_STATUS_OK == status)
    {
        printf("- confirmable alarm. Confirmed: %s\n", confirmedState ? "true" : "false");
        confirmableAlarm = true;
    }
    else
    {
        printf("- non-confirmable alarm.\n");
    }
    bool ackedState = false;
    status = SOPC_MonitoredAlarm_GetAckedState(alarmCond, &ackedState);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    printf("- acked: %s\n", ackedState ? "true" : "false");
    bool activeState = false;
    status = SOPC_MonitoredAlarm_GetActiveState(alarmCond, &activeState);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    printf("- active: %s\n", activeState ? "true" : "false");

    if (playTest1)
    {
        if (!confirmableAlarm)
        {
            // It is a no-confirmation alarm.
            if (!ackedState)
            {
                // Non acked. Acknowledge.
                SOPC_StatusCode opcua_status = SOPC_MonitoredAlarmCall_Acknowledge(alarmCond, NULL);
                printf("Called Acknowledge method. Return status of the method: 0x%08" PRIX32 "\n", opcua_status);
            }
            else
            {
                // Acked
                if (!activeState)
                {
                    // Alarm must have returned inactive.
                    printf("Alarm has turned inactive!\n");
                    SOPC_Atomic_Int_Add(&nbrOfCyclesNoConfirmationCompleted, 1);
                }
            }
        }
        else
        {
            // It is a confirmation alarm.
            if (!ackedState)
            {
                // Non acked. Acknowledge.
                SOPC_StatusCode opcua_status = SOPC_MonitoredAlarmCall_Acknowledge(alarmCond, NULL);
                printf("Called Acknowledge method. Return status of the method: 0x%08" PRIX32 "\n", opcua_status);
            }
            else
            {
                // Acked
                if (!confirmedState)
                {
                    // Acked but non confirmed. Confirm.
                    SOPC_StatusCode opcua_status = SOPC_MonitoredAlarmCall_Confirm(alarmCond, NULL);
                    printf("Called Confirm method. Return status of the method: 0x%08" PRIX32 "\n", opcua_status);
                }
                else
                {
                    // Acked + Confirmed
                    if (!activeState)
                    {
                        // Alarm must have returned inactive.
                        printf("Alarm has turned inactive!\n");
                        SOPC_Atomic_Int_Add(&nbrOfCyclesConfirmationCompleted, 1);
                    }
                }
            }
        }
    }

    if (playTest2Part1)
    {
        // Test if the alarm is a good candidate for the test
        bool enabledState = false;
        status = SOPC_MonitoredAlarm_GetEnabledState(alarmCond, &enabledState);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        if (enabledState)
        {
            SOPC_StatusCode opcua_status = SOPC_MonitoredAlarmCall_Enable(alarmCond, false);
            printf("Status of Disabling method: 0x%08" PRIX32 "\n", opcua_status);
            if (SOPC_IsGoodStatus(opcua_status))
            {
                printf("New alarm disabled.\n");
                SOPC_Atomic_Int_Add(&nbrOfDisabledAlarms, 1);
            }
        }
    }

    if (playTest2Part2)
    {
        oneAlarmUpdated = true;
    }

    if (playTest3)
    {
        SOPC_NodeId* conditionIdNodeIdCpy = SOPC_Malloc(sizeof(SOPC_NodeId));
        status = SOPC_NodeId_Copy(conditionIdNodeIdCpy, conditionIdNodeId);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        // If we did not count the alarm as a new enabled alarm, do it
        bool was_in_list = add_nodeId_in_list(enabledAlarmsArray, conditionIdNodeIdCpy);
        if (!was_in_list)
        {
            printf("Notification received for a new alarm, the alarm has been added to the enabled alarms.\n");
            SOPC_Atomic_Int_Add(&nbrOfEnabledAlarms, 1);
        }
        else
        {
            SOPC_NodeId_Clear(conditionIdNodeIdCpy);
            SOPC_Free(conditionIdNodeIdCpy);
        }
    }

    if (clearArrayOfEnabled)
    {
        for (size_t i = 0; i < MAX_DIFFERENT_ALARMS; i++)
        {
            SOPC_NodeId_Clear(enabledAlarmsArray[i]);
        }
        arrayCleared = true;
    }

    SOPC_Event_Delete(&rcvEvent);
    printf("\n");
}

/*---------------------------------------------------------------------------
 *                                 MAIN
 *---------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    if (3 != argc)
    {
        printf(
            "Usage: please mention the binary server on which you wish the test to be performed "
            "('toolkit_test_server' or 'toolkit_demo_server_alarms') and the number of alarms.\n"
            "For example ./toolkit_test_client_alarms test_server 3.\nNote that the server needs to have "
            "at least this number of different alarms (different ConditionIds).\n");
        return -2;
    }
    bool is_demo_server = false;
    if (0 == strcmp("toolkit_demo_server_alarms", argv[1]))
    {
        // Demo server needs user "me" for the alarms method call
        is_demo_server = true;
    }
    int32_t numberOfAlarmsToTest = atoi(argv[2]);

    bool startTests = false;
    bool globTestResult = false;
    SOPC_SecureConnection_Config* scConfig = NULL;
    SOPC_ClientConnection* secureConnection = NULL;

    // Initialize A&C module
    SOPC_ReturnStatus status = SOPC_MonitoredAlarmMgr_Initialize(SOPC_MonitoredAlarm_Event_Cb, true);

    if (SOPC_STATUS_OK == status)
    {
        status = Client_Initialize();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Client_LoadClientConfiguration(is_demo_server, &scConfig);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != scConfig);
        status = SOPC_ClientHelper_Connect(scConfig, SOPC_Client_ConnEventCb, &secureConnection);
    }

    bool connCreated = false;
    SOPC_MonitoredAlarmsGroup* MAGroup = NULL;
    OpcUa_CreateSubscriptionRequest* createSubReq = NULL;
    // Create a subscription
    if (SOPC_STATUS_OK == status)
    {
        connCreated = true;
        createSubReq = SOPC_CreateSubscriptionRequest_Create(500, 6, 2, 1000, true, 0);
        if (NULL != createSubReq)
        {
            /* Create MonitoredAlarm with notifier serverObject and default API clauses */
            MAGroup = SOPC_MonitoredAlarm_CreateAlarmsGroup(secureConnection, createSubReq, &serverObjectId, NULL);
        }
    }

    if (NULL != MAGroup)
    {
        // Start with a refresh
        SOPC_CONSOLE_PRINTF("MonitoredAlarms group created. Refreshing.\n");
        SOPC_StatusCode sc = SOPC_MonitoredAlarmCall_Refresh(MAGroup);
        printf("Status of Refreshing: 0x%08" PRIX32 "\n", sc);
        startTests = true;
    }

    if (startTests)
    {
        globTestResult = true;
        /*---------------------------------------------------------------------------
         *                                 TC 1
         *---------------------------------------------------------------------------*/
        // Wait for alarms completing full cycles
        uint32_t loopCpt = 0;
        int32_t nbrOfCompleteCyclesWanted = numberOfAlarmsToTest;
        SOPC_CONSOLE_PRINTF("\n=========== TC 1: Complete %d full cycles ===========\n", nbrOfCompleteCyclesWanted);
        playTest1 = true;
        /* Wait until the wished number of complete cycles (nbrOfCompleteCyclesWanted) has been reached */
        // X full no-confirmation alarm cycles + X full confirmation cycles for demo server
        // X full confirmation cycles for test server
        while (((SOPC_Atomic_Int_Get(&nbrOfCyclesConfirmationCompleted) < nbrOfCompleteCyclesWanted) ||
                (is_demo_server ? (SOPC_Atomic_Int_Get(&nbrOfCyclesNoConfirmationCompleted) < nbrOfCompleteCyclesWanted)
                                : 0)) &&
               loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
        playTest1 = false;
        if (loopCpt * sleepTimeout > loopTimeout)
        {
            SOPC_CONSOLE_PRINTF(
                "TC 1 Failed. Completed %d full cycles on confirmation alarm and %d full cycles "
                " on non-confirmation alarm, was expecting %d cumulated.\n",
                SOPC_Atomic_Int_Get(&nbrOfCyclesConfirmationCompleted),
                SOPC_Atomic_Int_Get(&nbrOfCyclesNoConfirmationCompleted), nbrOfCompleteCyclesWanted);
            globTestResult = false;
        }
        else
        {
            SOPC_CONSOLE_PRINTF("TC 1 Success. Full cycle completed on %d alarms.\n", nbrOfCompleteCyclesWanted);
        }

        /*---------------------------------------------------------------------------
         *                                 TC 2
         *---------------------------------------------------------------------------*/
        // Disable all alarms of the server (ie if receive enabled, disable).
        // Check that at least numberOfAlarmsToTest that alarms have been disabled.
        SOPC_CONSOLE_PRINTF(
            "\n=========== TC 2: Disable all alarms. No further update should be received  ===========\n");
        playTest2Part1 = true;
        loopCpt = 0;
        int32_t nbrOfDisabledAlarmsExpected = numberOfAlarmsToTest;
        /* Wait until the wished number of disabled alarms (nbrOfDisabledAlarmsExpected) has been reached */
        while (SOPC_Atomic_Int_Get(&nbrOfDisabledAlarms) < nbrOfDisabledAlarmsExpected &&
               loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
        playTest2Part1 = false;
        if (loopCpt * sleepTimeout > loopTimeout)
        {
            SOPC_CONSOLE_PRINTF("TC 2 Part 1 Failed. Disabled %d alarms, was expecting %d disabled alarms.\n",
                                SOPC_Atomic_Int_Get(&nbrOfDisabledAlarms), nbrOfDisabledAlarmsExpected);
            globTestResult = false;
        }
        else
        {
            SOPC_CONSOLE_PRINTF("TC 2 Part 1 Success. %d alarms disabled.\n", nbrOfDisabledAlarmsExpected);
            SOPC_StatusCode sc = SOPC_MonitoredAlarmCall_Refresh(MAGroup);
            printf("Status of Refreshing after disabling all alarms: 0x%08" PRIX32 "\n", sc);
            if (!SOPC_IsGoodStatus(sc))
            {
                SOPC_CONSOLE_PRINTF("TC 2 Part 2 Failed at refreshing alarms.\n");
                globTestResult = false;
            }
            // Wait for refresh to take effect on received alarm updates.
            // TODO: wait for the event ConditionRefreshEnd instead.
            SOPC_Sleep(sleepTimeout * 10);
            playTest2Part2 = true;
            SOPC_Sleep(sleepTimeout * 10); // wait to see if we receive some updates
            playTest2Part2 = false;
            if (!oneAlarmUpdated)
            {
                SOPC_CONSOLE_PRINTF(
                    "TC 2 Part 2 Success. No more alarm notification received after disabling all of them.\n");
            }
            else
            {
                SOPC_CONSOLE_PRINTF("TC 2 Part 2 Failed: received one alarm update after disabling all of them.\n");
                globTestResult = false;
            }
        }

        /*---------------------------------------------------------------------------
         *                                 TC 3
         *---------------------------------------------------------------------------*/
        // Enable all alarms of the server.
        // Check that at least numberOfAlarmsToTest alarms are enabled.
        SOPC_CONSOLE_PRINTF(
            "\n=========== TC 3: Enable all alarms. Waiting for at least one update on each. ===========\n");
        if (is_demo_server)
        {
            SOPC_StatusCode enableSc = SOPC_MonitoredAlarmCall_EnableFromId(MAGroup, &alarm1_demo_server, true);
            SOPC_ASSERT(SOPC_IsGoodStatus(enableSc));
            enableSc = SOPC_MonitoredAlarmCall_EnableFromId(MAGroup, &alarm2_demo_server, true);
            SOPC_ASSERT(SOPC_IsGoodStatus(enableSc));
        }
        else
        {
            SOPC_StatusCode enableSc = SOPC_MonitoredAlarmCall_EnableFromId(MAGroup, &alarm1_test_server, true);
            SOPC_ASSERT(SOPC_IsGoodStatus(enableSc));
            enableSc = SOPC_MonitoredAlarmCall_EnableFromId(MAGroup, &alarm2_test_server, true);
            SOPC_ASSERT(SOPC_IsGoodStatus(enableSc));
            enableSc = SOPC_MonitoredAlarmCall_EnableFromId(MAGroup, &alarm3_test_server, true);
            SOPC_ASSERT(SOPC_IsGoodStatus(enableSc));
            enableSc = SOPC_MonitoredAlarmCall_EnableFromId(MAGroup, &alarm4_test_server, true);
            SOPC_ASSERT(SOPC_IsGoodStatus(enableSc));
        }

        loopCpt = 0;
        int32_t nbrOfEnabledAlarmsExpected = numberOfAlarmsToTest;
        playTest3 = true;
        /* Wait until the wished number of enabled alarms (nbrOfEnabledAlarmsExpected) has been reached */
        while (SOPC_Atomic_Int_Get(&nbrOfEnabledAlarms) < nbrOfEnabledAlarmsExpected &&
               loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
        playTest3 = false;
        if (loopCpt * sleepTimeout > loopTimeout)
        {
            SOPC_CONSOLE_PRINTF("TC 3 Failed. Only %d alarms enabled, was expecting %d.\n",
                                SOPC_Atomic_Int_Get(&nbrOfEnabledAlarms), nbrOfEnabledAlarmsExpected);
            globTestResult = false;
        }
        else
        {
            loopCpt = 0;
            clearArrayOfEnabled = true;
            /* Wait for the clear of ConditionIds array necessary for previous test */
            // NOTE: if the client does not receive any notification from now, it will be memory leak.
            // But since we just enabled all alarms and server changes states frequently, we should
            // receive new notifications (it may be considered as part of the test to receive new notifications).
            while (!arrayCleared && loopCpt * sleepTimeout <= loopTimeout)
            {
                loopCpt++;
                SOPC_Sleep(sleepTimeout);
            }
            clearArrayOfEnabled = false;
            if (loopCpt * sleepTimeout > loopTimeout)
            {
                SOPC_CONSOLE_PRINTF("TC 3 Failed. Timeout when waiting for new updates after enabling alarms.");
                globTestResult = false;
            }
            else
            {
                SOPC_CONSOLE_PRINTF("TC 3 Success. Success on enabling %d alarms.\n", nbrOfEnabledAlarmsExpected);
            }
        }

        /*---------------------------------------------------------------------------
         *                                 TC 3
         *---------------------------------------------------------------------------*/
        // Test creating a new alarm group + the manual clear and global module clear.
        SOPC_CONSOLE_PRINTF(
            "\n=========== TC 4: Create a new alarms group. Manually clear the first group and make "
            "a global module clear for clearing the second one. ===========\n\n");
        SOPC_MonitoredAlarmsGroup* MAGroup2 =
            SOPC_MonitoredAlarm_CreateAlarmsGroup(secureConnection, NULL, &serverObjectId, NULL);
        if (NULL == MAGroup2)
        {
            SOPC_CONSOLE_PRINTF("TC 4 Failed. Fail at creating additionnal alarm group.\n");
            globTestResult = false;
        }
        else
        {
            SOPC_CONSOLE_PRINTF("TC 4 Success. New alarm group successfully created.\n");
        }
        SOPC_MonitoredAlarm_ClearAlarmsGroup(MAGroup);
        SOPC_MonitoredAlarmMgr_Clear();
        MAGroup = NULL;
        MAGroup2 = NULL;
    }
    else
    {
        SOPC_CONSOLE_PRINTF("Tests did not play because fail at creating monitored alarms.\n");
    }

    /* Close the connection */
    if (connCreated)
    {
        SOPC_ReturnStatus tmpStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != tmpStatus)
        {
            printf(">>Test_Client_Toolkit_Alarm_Conditions: fail at disconnection.\n");
            globTestResult = false;
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (globTestResult)
    {
        printf("\n>>Test_Client_Toolkit_Alarm_Conditions final result: OK\n");
    }
    else
    {
        printf("\n>>Test_Client_Toolkit_Alarm_Conditions final result: NOK (BAD status: %" PRIu32 ")\n", status);
    }
    return (globTestResult ? EXIT_SUCCESS : EXIT_FAILURE);
}
