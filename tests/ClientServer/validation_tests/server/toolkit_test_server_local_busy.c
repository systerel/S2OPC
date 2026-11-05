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
 * \file toolkit_test_server_local_busy.c
 *
 * \brief Validation test: local service requests are refused with OpcUa_BadServerTooBusy when the number of local
 *        requests pending in the Services queue reaches ::SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL.
 *
 * Scenarios:
 * - hysteresis (deterministic): the Services thread is blocked by an event treated hook while a batch of local Writes
 *   is posted, then released. The busy state is evaluated at each request dequeue with the remaining pending requests:
 * a batch of HIGH requests is never refused, a batch of N > HIGH requests has its N - LOW - 1 first requests refused
 *   and its LOW + 1 last requests accepted.
 * - flood: flood local Writes while the Services thread is blocked, probe until recovered, drain, and print stats.
 *
 * \note The Server.ServerStatus.CurrentTime refresh (local Write) is deactivated to keep the batches deterministic.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

#include "sopc_test_services_queue_busy.h"

static SOPC_TestBusy_Stats gStats;
static SOPC_Mutex gStatsMutex;
static SOPC_Condition gRespCond;
static int32_t gStopServer = 0;

static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&gStopServer, 1);
}

/* User context of the local requests: flood scenario or hysteresis batch request */
#define LOCAL_BUSY_CTX_FLOOD 0u
#define LOCAL_BUSY_CTX_BATCH 1u

/* Hysteresis batch results (protected by gStatsMutex) */
static uint32_t gBatchResp = 0;
static uint32_t gBatchBusy = 0;
static uint32_t gBatchOk = 0;
static bool gBatchBusyAfterOk = false;

static void LocalBusy_RespCb(SOPC_EncodeableType* type, void* response, uintptr_t userContext)
{
    /* Response memory is freed by the toolkit after this callback returns. */
    const bool isBusy = SOPC_TestBusy_IsTooBusyResponse(type, response);

    SOPC_Mutex_Lock(&gStatsMutex);
    if (LOCAL_BUSY_CTX_FLOOD == userContext)
    {
        SOPC_TestBusy_Stats_OnResponse(&gStats, isBusy);
    }
    else if (LOCAL_BUSY_CTX_BATCH == userContext)
    {
        gBatchResp++;
        if (isBusy)
        {
            gBatchBusy++;
            gBatchBusyAfterOk = gBatchBusyAfterOk || gBatchOk > 0;
        }
        else
        {
            gBatchOk++;
        }
    }
    SOPC_Condition_SignalAll(&gRespCond);
    SOPC_Mutex_Unlock(&gStatsMutex);
}

static SOPC_ReturnStatus LocalBusy_SendAsyncWriteCtx(uintptr_t userContext)
{
    OpcUa_WriteRequest* req = SOPC_TestBusy_CreateHeavyWriteRequest();
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceAsyncCustom(&LocalBusy_RespCb, req, userContext);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &req);
        }
    }

    return status;
}

static SOPC_ReturnStatus LocalBusy_SendAsyncWrite(void)
{
    return LocalBusy_SendAsyncWriteCtx(LOCAL_BUSY_CTX_FLOOD);
}

/*
 * Posts nbRequests local Writes while the Services thread is blocked, releases it and checks the number of refused
 * requests (all refused requests shall be answered before the accepted ones).
 */
static SOPC_ReturnStatus LocalBusy_CheckBatch(uint32_t nbRequests, uint32_t expectedBusy)
{
    SOPC_Mutex_Lock(&gStatsMutex);
    gBatchResp = 0;
    gBatchBusy = 0;
    gBatchOk = 0;
    gBatchBusyAfterOk = false;
    SOPC_Mutex_Unlock(&gStatsMutex);

    SOPC_ReturnStatus status = SOPC_TestBusy_BlockServices(SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < nbRequests; i++)
    {
        status = LocalBusy_SendAsyncWriteCtx(LOCAL_BUSY_CTX_BATCH);
    }
    // All the batch requests are pending in the queues measured by the local busy category
    const uint32_t queueSize =
        SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType_ServicesLocalRequests);
    if (SOPC_STATUS_OK == status && nbRequests != queueSize)
    {
        printf("<Test_Server_Local_Busy: FAIL batch of %" PRIu32 " requests but %" PRIu32 " pending events\n",
               nbRequests, queueSize);
        status = SOPC_STATUS_NOK;
    }
    SOPC_TestBusy_ReleaseServices();

    // Wait for all the batch responses
    SOPC_Mutex_Lock(&gStatsMutex);
    SOPC_ReturnStatus waitStatus = SOPC_STATUS_OK;
    while (SOPC_STATUS_OK == status && SOPC_STATUS_OK == waitStatus && gBatchResp < nbRequests)
    {
        waitStatus = SOPC_Mutex_UnlockAndTimedWaitCond(&gRespCond, &gStatsMutex, SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    }
    if (SOPC_STATUS_OK == status && gBatchResp < nbRequests)
    {
        printf("<Test_Server_Local_Busy: FAIL batch responses %" PRIu32 "/%" PRIu32 "\n", gBatchResp, nbRequests);
        status = SOPC_STATUS_TIMEOUT;
    }
    if (SOPC_STATUS_OK == status && (expectedBusy != gBatchBusy || gBatchBusyAfterOk))
    {
        printf("<Test_Server_Local_Busy: FAIL batch of %" PRIu32 " requests: %" PRIu32 " refused (expected %" PRIu32
               "), refused after accepted: %d\n",
               nbRequests, gBatchBusy, expectedBusy, (int) gBatchBusyAfterOk);
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Busy: batch of %" PRIu32 " requests: %" PRIu32 " refused, %" PRIu32 " accepted\n",
               nbRequests, gBatchBusy, gBatchOk);
    }
    SOPC_Mutex_Unlock(&gStatsMutex);

    return status;
}

/*
 * Hysteresis: a batch of HIGH requests is never refused (remaining size < HIGH at each dequeue). A batch of
 * N = HIGH + SOPC_TEST_BUSY_BATCH_ABOVE_HIGH requests enters busy at the first dequeue (remaining N - 1 >= HIGH) and
 * leaves it when the remaining size reaches LOW: N - LOW - 1 refused requests then LOW + 1 accepted ones.
 */
static SOPC_ReturnStatus LocalBusy_CheckHysteresis(void)
{
    const uint32_t high = (uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL;
    const uint32_t low = (uint32_t) SOPC_SERVICES_QUEUE_BUSY_LOW_LOCAL;
    const uint32_t nbAboveHigh = SOPC_TestBusy_BatchSize(high, true);

    // The blocking hook is called after the trigger request treatment only if the Services thread is idle
    SOPC_ReturnStatus status = SOPC_TestBusy_WaitServicesIdle(SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Local_Busy: FAIL Services queues not empty before hysteresis check\n");
    }
    SOPC_TestBusy_SetBlockingHook(true);
    if (SOPC_STATUS_OK == status)
    {
        status = LocalBusy_CheckBatch(SOPC_TestBusy_BatchSize(high, false),
                                      SOPC_TestBusy_ExpectedRefusedInBatch(high, high, low));
    }
    if (SOPC_STATUS_OK == status)
    {
        status = LocalBusy_CheckBatch(nbAboveHigh, SOPC_TestBusy_ExpectedRefusedInBatch(nbAboveHigh, high, low));
    }
    // The blocking hook remains set for the flood scenario
    return status;
}

static SOPC_ReturnStatus LocalBusy_ProbeSync(void* userCtx, bool* isBusy)
{
    SOPC_UNUSED_ARG(userCtx);
    OpcUa_WriteRequest* req = SOPC_TestBusy_CreateHeavyWriteRequest();
    void* response = NULL;
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    SOPC_EncodeableType* type = NULL;

    SOPC_ASSERT(NULL != isBusy);
    *isBusy = false;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(req, &response);
    }
    if (SOPC_STATUS_OK == status && NULL != response)
    {
        type = *(SOPC_EncodeableType**) response;
        *isBusy = SOPC_TestBusy_IsTooBusyResponse(type, response);
        SOPC_Mutex_Lock(&gStatsMutex);
        gStats.nbSent++;
        SOPC_TestBusy_Stats_OnResponse(&gStats, *isBusy);
        SOPC_Mutex_Unlock(&gStatsMutex);
        SOPC_EncodeableObject_Delete(type, &response);
    }

    return status;
}

static SOPC_ReturnStatus LocalBusy_WaitFirstBusy(uint32_t timeoutMs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_Mutex_Lock(&gStatsMutex);
    while (SOPC_STATUS_OK == status && 0 == gStats.tFirstBusy)
    {
        status = SOPC_Mutex_UnlockAndTimedWaitCond(&gRespCond, &gStatsMutex, timeoutMs);
    }
    if (0 != gStats.tFirstBusy)
    {
        status = SOPC_STATUS_OK;
    }
    SOPC_Mutex_Unlock(&gStatsMutex);

    return status;
}

static SOPC_ReturnStatus LocalBusy_WaitPendingResponses(uint32_t timeoutMs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_Mutex_Lock(&gStatsMutex);
    while (SOPC_STATUS_OK == status && gStats.nbResp < gStats.nbSent)
    {
        status = SOPC_Mutex_UnlockAndTimedWaitCond(&gRespCond, &gStatsMutex, timeoutMs);
    }
    if (gStats.nbResp >= gStats.nbSent)
    {
        status = SOPC_STATUS_OK;
    }
    SOPC_Mutex_Unlock(&gStatsMutex);

    return status;
}

int main(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus drainStatus = SOPC_STATUS_OK;
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    const uint32_t high = (uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_LOCAL;
    const uint32_t floodCap = SOPC_TestBusy_GetFloodCap(high);
    bool serverStarted = false;

    if (NULL == getenv(PASSWORD_ENV_NAME))
    {
        printf(
            "<Test_Server_Local_Busy: FAIL missing environment variable %s\n"
            "  e.g. %s=password ./toolkit_test_server_local_busy\n",
            PASSWORD_ENV_NAME, PASSWORD_ENV_NAME);
        return 1;
    }

    SOPC_TestBusy_Stats_Init(&gStats, SOPC_CommonMonitoring_QueueType_ServicesLocalRequests);
    status = SOPC_Mutex_Initialization(&gStatsMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Condition_Init(&gRespCond);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    logConfiguration.logLevel = SOPC_LOG_LEVEL_ERROR;
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_local_busy_logs/";
    status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TestBusy_ConfigureServer("S2OPC local busy test server", false);
    }
    if (SOPC_STATUS_OK == status)
    {
        // No periodic local Write of the CurrentTime: deterministic hysteresis batches
        status = SOPC_ServerConfigHelper_SetCurrentTimeRefreshInterval(0);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
        if (SOPC_STATUS_OK == status)
        {
            serverStarted = true;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = LocalBusy_CheckHysteresis();
    }

    /*
     * Flood local Writes (floodCap requests) while the Services thread is blocked, so that the busy state is
     * reached whatever the relative speed of the application and Services threads, then release it.
     */
    SOPC_TestBusy_Stats_MarkFloodStart(&gStats);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TestBusy_BlockServices(SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    }
    while (SOPC_STATUS_OK == status && gStats.nbSent < floodCap)
    {
        status = LocalBusy_SendAsyncWrite();
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Mutex_Lock(&gStatsMutex);
            gStats.nbSent++;
            SOPC_Mutex_Unlock(&gStatsMutex);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Busy: flooded nbSent=%" PRIu32 " queue=%" PRIu32 " HIGH=%" PRIu32 "\n",
               gStats.nbSent, SOPC_TestBusy_GetServicesQueueSize(gStats.queueType), high);
    }
    SOPC_TestBusy_ReleaseServices();

    if (SOPC_STATUS_OK == status)
    {
        status = LocalBusy_WaitFirstBusy(SOPC_TEST_BUSY_FLOOD_WAIT_MS);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Busy: FAIL no BadServerTooBusy during flood\n");
        }
    }

    /* Wait recovery (or confirm already recovered). */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_TestBusy_WaitUntilNotBusy(LocalBusy_ProbeSync, NULL, &gStats);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Busy: FAIL did not recover from busy within timeout\n");
        }
    }

    /* Drain remaining async responses; duration is measured from tFirstOk. */
    gStats.queueToDrain = SOPC_TestBusy_GetServicesQueueSize(gStats.queueType);
    drainStatus = LocalBusy_WaitPendingResponses(SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    if (0 != gStats.tFirstOk)
    {
        gStats.drainMs = (uint64_t)(SOPC_TimeReference_GetCurrent() - gStats.tFirstOk);
    }
    if (SOPC_STATUS_OK != drainStatus)
    {
        printf("<Test_Server_Local_Busy: FAIL pending responses not drained (sent=%" PRIu32 " resp=%" PRIu32
               " status=%d)\n",
               gStats.nbSent, gStats.nbResp, (int) drainStatus);
        if (SOPC_STATUS_OK == status)
        {
            status = drainStatus;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        if (0 == gStats.tFirstOk)
        {
            printf("<Test_Server_Local_Busy: FAIL missing recovery timestamp\n");
            status = SOPC_STATUS_NOK;
        }
        else
        {
            SOPC_TestBusy_PrintBusyDuration(&gStats);
            SOPC_TestBusy_PrintStats("Test_Server_Local_Busy", &gStats);
        }
    }

    SOPC_TestBusy_SetBlockingHook(false);
    if (serverStarted)
    {
        SOPC_ServerHelper_StopServer();
        while (0 == SOPC_Atomic_Int_Get(&gStopServer))
        {
            SOPC_Sleep(10);
        }
    }

    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
    SOPC_Condition_Clear(&gRespCond);
    SOPC_Mutex_Clear(&gStatsMutex);

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Busy: PASSED\n");
        return 0;
    }

    printf("<Test_Server_Local_Busy: FAILED status=%d\n", (int) status);
    return 1;
}
