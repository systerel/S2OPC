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

/**
 * \file toolkit_test_server_busy_external.c
 *
 * \brief Server of the external busy validation test (client toolkit_test_server_client_busy): for each request batch
 *        of the client, blocks its Services thread so that the batch requests are pending in its Services queue, then
 *        releases it (see ::SOPC_TEST_BUSY_EXT_NB_BATCHES for the synchronization with the client).
 *
 * Runs until SIGINT / SIGTERM (stopped by with-opc-server.py once the client test is done).
 *
 * \note The Server.ServerStatus.CurrentTime refresh (local Write) is deactivated to keep the batches deterministic.
 */

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_threads.h"

#include "sopc_test_services_queue_busy.h"

/* Maximum waiting time for a client synchronization file */
#define BUSY_EXT_SERVER_SYNC_TIMEOUT_MS 60000u

static int32_t gStopSignal = 0;
static int32_t gServerStopped = 0;

static void BusyExtServer_StopSignal(int sig)
{
    SOPC_UNUSED_ARG(sig);
    SOPC_Atomic_Int_Set(&gStopSignal, 1);
}

static void BusyExtServer_StoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&gServerStopped, 1);
}

static void BusyExtServer_RemoveSyncFiles(void)
{
    char name[128];
    for (uint32_t i = 0; i < SOPC_TEST_BUSY_EXT_NB_BATCHES; i++)
    {
        SOPC_TestBusy_SyncFileName(name, sizeof(name), false, i);
        (void) remove(name);
        SOPC_TestBusy_SyncFileName(name, sizeof(name), true, i);
        (void) remove(name);
    }
}

/* Blocks the Services thread until the batch requests are pending in the Services queue */
static SOPC_ReturnStatus BusyExtServer_HoldBatch(uint32_t batchIdx)
{
    char name[128];
    const uint32_t nbRequests =
        SOPC_TestBusy_BatchSize((uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL, 1u <= batchIdx);

    SOPC_TestBusy_SyncFileName(name, sizeof(name), false, batchIdx);
    SOPC_ReturnStatus status = SOPC_TestBusy_WaitFile(name, BUSY_EXT_SERVER_SYNC_TIMEOUT_MS, &gStopSignal);
    if (SOPC_STATUS_OK == status)
    {
        // The blocking hook is called after the trigger request treatment only if the Services thread is idle
        status = SOPC_TestBusy_WaitServicesIdle(SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TestBusy_BlockServices(SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TestBusy_SyncFileName(name, sizeof(name), true, batchIdx);
        status = SOPC_TestBusy_CreateFile(name);
    }
    // Wait for the batch requests received by the SecureChannels layer
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    uint32_t queueSize = 0;
    while (SOPC_STATUS_OK == status && queueSize < nbRequests)
    {
        if (SOPC_TimeReference_GetCurrent() - tStart > SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS ||
            0 != SOPC_Atomic_Int_Get(&gStopSignal))
        {
            status = SOPC_STATUS_TIMEOUT;
        }
        else
        {
            SOPC_Sleep(1);
            queueSize = SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType_ServicesExternalRequests);
        }
    }
    SOPC_TestBusy_ReleaseServices();
    printf("<Test_Server_Busy_External: batch %" PRIu32 " of %" PRIu32 " requests released with %" PRIu32
           " pending external requests (status=%d)\n",
           batchIdx, nbRequests, queueSize, (int) status);
    return status;
}

int main(void)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    bool serverStarted = false;

    if (NULL == getenv(PASSWORD_ENV_NAME))
    {
        printf("<Test_Server_Busy_External: FAIL missing environment variable %s\n", PASSWORD_ENV_NAME);
        return 1;
    }
    BusyExtServer_RemoveSyncFiles();

    logConfiguration.logLevel = SOPC_LOG_LEVEL_ERROR;
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_busy_external_logs/";
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TestBusy_ConfigureServer("S2OPC external busy test server", true);
    }
    if (SOPC_STATUS_OK == status)
    {
        // No periodic local Write of the CurrentTime: deterministic batches
        status = SOPC_ServerConfigHelper_SetCurrentTimeRefreshInterval(0);
    }
    if (SOPC_STATUS_OK == status)
    {
        signal(SIGINT, BusyExtServer_StopSignal);
        signal(SIGTERM, BusyExtServer_StopSignal);
        status = SOPC_ServerHelper_StartServer(BusyExtServer_StoppedCallback);
        serverStarted = (SOPC_STATUS_OK == status);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TestBusy_SetBlockingHook(true);
    }
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < SOPC_TEST_BUSY_EXT_NB_BATCHES; i++)
    {
        status = BusyExtServer_HoldBatch(i);
    }

    // Serve until stopped by the test harness
    while (serverStarted && 0 == SOPC_Atomic_Int_Get(&gStopSignal) && 0 == SOPC_Atomic_Int_Get(&gServerStopped))
    {
        SOPC_Sleep(10);
    }

    SOPC_TestBusy_SetBlockingHook(false);
    if (serverStarted)
    {
        SOPC_ServerHelper_StopServer();
        while (0 == SOPC_Atomic_Int_Get(&gServerStopped))
        {
            SOPC_Sleep(10);
        }
    }
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
    BusyExtServer_RemoveSyncFiles();

    printf("<Test_Server_Busy_External: %s\n", (SOPC_STATUS_OK == status) ? "PASSED" : "FAILED");
    return (SOPC_STATUS_OK == status) ? 0 : 1;
}
