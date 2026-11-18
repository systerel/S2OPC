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

/** \file sopc_test_services_queue_busy.c
 *
 * \brief Implementation of shared helpers for Services-queue busy refusal tests.
 */

#include "sopc_test_services_queue_busy.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "embedded/sopc_addspace_loader.h"
#include "libs2opc_common_config.h"
#include "libs2opc_common_monitoring.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_services_api.h"
#include "sopc_threads.h"

void SOPC_TestBusy_Stats_Init(SOPC_TestBusy_Stats* stats, SOPC_CommonMonitoring_QueueType queueType)
{
    SOPC_ASSERT(NULL != stats);
    memset(stats, 0, sizeof(*stats));
    stats->queueType = queueType;
}

void SOPC_TestBusy_Stats_MarkFloodStart(SOPC_TestBusy_Stats* stats)
{
    SOPC_ASSERT(NULL != stats);
    stats->tFloodStart = SOPC_TimeReference_GetCurrent();
}

bool SOPC_TestBusy_IsTooBusyStatus(SOPC_StatusCode status)
{
    return (OpcUa_BadServerTooBusy == status);
}

bool SOPC_TestBusy_IsTooBusyResponse(const SOPC_EncodeableType* const type, const void* const response)
{
    SOPC_StatusCode serviceResult = SOPC_GoodGenericStatus;

    if (NULL == type || NULL == response)
    {
        return false;
    }

    if (&OpcUa_ServiceFault_EncodeableType == type)
    {
        serviceResult = ((const OpcUa_ServiceFault*) response)->ResponseHeader.ServiceResult;
    }
    else if (&OpcUa_WriteResponse_EncodeableType == type)
    {
        serviceResult = ((const OpcUa_WriteResponse*) response)->ResponseHeader.ServiceResult;
    }
    else
    {
        return false;
    }

    return SOPC_TestBusy_IsTooBusyStatus(serviceResult);
}

void SOPC_TestBusy_Stats_OnResponse(SOPC_TestBusy_Stats* stats, bool isBusy)
{
    SOPC_ASSERT(NULL != stats);

    stats->nbResp++;
    if (isBusy)
    {
        stats->nbBusy++;
        if (0 == stats->tFirstBusy)
        {
            stats->tFirstBusy = SOPC_TimeReference_GetCurrent();
            stats->nbSentAtFirstBusy = stats->nbSent;
            stats->queueAtFirstBusy = SOPC_TestBusy_GetServicesQueueSize(stats->queueType);
        }
    }
    else
    {
        stats->nbOk++;
        /* tFirstOk is set only by ::SOPC_TestBusy_WaitUntilNotBusy (sustained recovery). */
    }
}

SOPC_ReturnStatus SOPC_TestBusy_ConfigureServer(const char* const applicationName, bool secureEndpoint)
{
    SOPC_SecurityPolicy_URI secuPolicy = SOPC_SecurityPolicy_None;
    SOPC_SecurityModeMask secuModes = SOPC_SecurityModeMask_None;
    SOPC_Endpoint_Config* ep = NULL;
    SOPC_SecurityConfig* sp = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_AddressSpace* address_space = NULL;
    SOPC_PKIProvider* pkiProvider = NULL;

    if (NULL == applicationName)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (secureEndpoint)
    {
        secuPolicy = SOPC_SecurityPolicy_Basic256Sha256;
        secuModes = SOPC_SecurityModeMask_SignAndEncrypt;
    }

    // Endpoint in static memory of the configuration helper: never freed here
    ep = SOPC_ServerConfigHelper_CreateEndpoint(SOPC_TEST_BUSY_ENDPOINT_URL, true);
    sp = SOPC_EndpointConfig_AddSecurityConfig(ep, secuPolicy);
    if (NULL == ep || NULL == sp)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_SecurityConfig_SetSecurityModes(sp, secuModes);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromPath(SOPC_TEST_BUSY_SERVER_CERT_PATH,
                                                                SOPC_TEST_BUSY_SERVER_KEY_PATH, true);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(SOPC_TEST_BUSY_PKI_PATH, &pkiProvider);
        if (SOPC_STATUS_OK == status)
        {
            // Owned by the configuration helper once set
            status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_PKIProvider_Free(&pkiProvider);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(SOPC_TEST_BUSY_APPLICATION_URI,
                                                                   SOPC_TEST_BUSY_PRODUCT_URI, applicationName, NULL,
                                                                   OpcUa_ApplicationType_Server);
    }
    if (SOPC_STATUS_OK == status)
    {
        address_space = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
        status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        // Owned by the configuration helper once set
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_AddressSpace_Delete(address_space);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetShutdownCountdown(0);
    }

    return status;
}

OpcUa_WriteRequest* SOPC_TestBusy_CreateHeavyWriteRequest(void)
{
    OpcUa_WriteRequest* req = SOPC_WriteRequest_Create(SOPC_TEST_BUSY_WRITE_NB_VALUES);
    SOPC_DataValue dv;
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    uint32_t i = 0;

    SOPC_DataValue_Initialize(&dv);
    dv.Value.BuiltInTypeId = SOPC_UInt64_Id;
    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.Value.Uint64 = 1;

    for (i = 0; SOPC_STATUS_OK == status && i < SOPC_TEST_BUSY_WRITE_NB_VALUES; i++)
    {
        status = SOPC_WriteRequest_SetWriteValueFromStrings(req, i, SOPC_TEST_BUSY_NODE_ID_STR, SOPC_AttributeId_Value,
                                                            NULL, &dv);
    }

    SOPC_DataValue_Clear(&dv);

    if (SOPC_STATUS_OK != status)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &req);
        req = NULL;
    }

    return req;
}

uint32_t SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType queueType)
{
    return SOPC_CommonMonitoring_GetQueueSize(queueType);
}

uint32_t SOPC_TestBusy_GetFloodCap(uint32_t highThreshold)
{
    return highThreshold * SOPC_TEST_BUSY_FLOOD_CAP_FACTOR;
}

void SOPC_TestBusy_PrintBusyDuration(const SOPC_TestBusy_Stats* const stats)
{
    SOPC_ASSERT(NULL != stats);

    if (0 == stats->tFirstBusy || 0 == stats->tFirstOk || stats->tFirstOk < stats->tFirstBusy)
    {
        printf("Busy duration: n/a (tFirstBusy=%" PRIu64 " tFirstOk=%" PRIu64 ")\n", (uint64_t) stats->tFirstBusy,
               (uint64_t) stats->tFirstOk);
        return;
    }

    const uint64_t durationMs = (uint64_t)(stats->tFirstOk - stats->tFirstBusy);
    printf("Busy duration: %" PRIu64 " s (%" PRIu64 " ms)\n", durationMs / 1000u, durationMs);
}

void SOPC_TestBusy_PrintStats(const char* const testTag, const SOPC_TestBusy_Stats* const stats)
{
    uint64_t untilBusyMs = 0;

    SOPC_ASSERT(NULL != testTag);
    SOPC_ASSERT(NULL != stats);

    if (0 != stats->tFloodStart && 0 != stats->tFirstBusy && stats->tFirstBusy >= stats->tFloodStart)
    {
        untilBusyMs = (uint64_t)(stats->tFirstBusy - stats->tFloodStart);
    }

    printf("<%s: stats before busy: sent=%" PRIu32 " queue=%" PRIu32 " time=%" PRIu64 " s (%" PRIu64 " ms)\n", testTag,
           stats->nbSentAtFirstBusy, stats->queueAtFirstBusy, untilBusyMs / 1000u, untilBusyMs);
    printf("<%s: stats global: sent=%" PRIu32 " resp=%" PRIu32 " busy=%" PRIu32 " ok=%" PRIu32 "\n", testTag,
           stats->nbSent, stats->nbResp, stats->nbBusy, stats->nbOk);
    printf("<%s: stats after busy: queue=%" PRIu32 " drain=%" PRIu64 " s (%" PRIu64 " ms)\n", testTag,
           stats->queueToDrain, stats->drainMs / 1000u, stats->drainMs);
}

SOPC_ReturnStatus SOPC_TestBusy_WaitUntilNotBusy(SOPC_TestBusy_Probe_Fct probeCb,
                                                 void* userCtx,
                                                 SOPC_TestBusy_Stats* stats)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    bool isBusy = true;

    if (NULL == probeCb || NULL == stats)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Elapsed time measured on the monotonic clock: a probe might take long under load
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    while (SOPC_TimeReference_GetCurrent() - tStart <= (SOPC_TimeReference) SOPC_TEST_BUSY_MAX_WAIT_SECS * 1000u)
    {
        status = probeCb(userCtx, &isBusy);
        /*
         * Transient probe failures (OOM / timeout under residual flood load) are treated as
         * still-busy: keep waiting rather than aborting the recovery loop.
         */
        if (SOPC_STATUS_OK == status && !isBusy)
        {
            if (0 == stats->tFirstOk)
            {
                stats->tFirstOk = SOPC_TimeReference_GetCurrent();
            }
            return SOPC_STATUS_OK;
        }

        SOPC_Sleep(SOPC_TEST_BUSY_PROBE_PERIOD_MS);
    }

    return SOPC_STATUS_TIMEOUT;
}

/* Services thread blocking: set by SOPC_TestBusy_BlockServices, reset by SOPC_TestBusy_ReleaseServices */
static int32_t gBlockServices = 0;
/* Set by the Services thread while blocked in SOPC_TestBusy_BlockingHook */
static int32_t gServicesBlocked = 0;
/* Set when the response of the trigger request of SOPC_TestBusy_BlockServices is received */
static int32_t gTriggerResp = 0;
/* Number of calls of SOPC_TestBusy_BlockingHook, incremented at the end of each call */
static int32_t gHookCalls = 0;

/* Called by the Services thread after each event treatment: blocks it while gBlockServices is set */
static void SOPC_TestBusy_BlockingHook(void)
{
    if (0 != SOPC_Atomic_Int_Get(&gBlockServices))
    {
        SOPC_Atomic_Int_Set(&gServicesBlocked, 1);
        while (0 != SOPC_Atomic_Int_Get(&gBlockServices))
        {
            SOPC_Sleep(1);
        }
        SOPC_Atomic_Int_Set(&gServicesBlocked, 0);
    }
    SOPC_Atomic_Int_Add(&gHookCalls, 1);
}

static SOPC_Services_Hooks gBlockingHook = {.eventTreatedCb = SOPC_TestBusy_BlockingHook, .serverStoppedCb = NULL};

void SOPC_TestBusy_SetBlockingHook(bool install)
{
    if (!install)
    {
        SOPC_TestBusy_ReleaseServices();
    }
    SOPC_Services_SetHooks(install ? &gBlockingHook : NULL);
}

/* Waits until the atomic *value equals expected, returns false on timeout */
static bool SOPC_TestBusy_WaitAtomicValue(int32_t* value, int32_t expected, uint32_t timeoutMs)
{
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    while (expected != SOPC_Atomic_Int_Get(value) && SOPC_TimeReference_GetCurrent() - tStart <= timeoutMs)
    {
        SOPC_Sleep(1);
    }
    return expected == SOPC_Atomic_Int_Get(value);
}

/* Waits until the blocking hook is called nbCalls times since hookCalls value, returns false on timeout */
static bool SOPC_TestBusy_WaitHookCalls(int32_t hookCalls, uint32_t nbCalls, uint32_t timeoutMs)
{
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    bool done = false;
    while (!done && SOPC_TimeReference_GetCurrent() - tStart <= timeoutMs)
    {
        done = (uint32_t) SOPC_Atomic_Int_Get(&gHookCalls) - (uint32_t) hookCalls >= nbCalls;
        if (!done)
        {
            SOPC_Sleep(1);
        }
    }
    return done;
}

SOPC_ReturnStatus SOPC_TestBusy_WaitServicesIdle(uint32_t timeoutMs)
{
    OpcUa_WriteRequest* req = SOPC_TestBusy_CreateHeavyWriteRequest();
    void* response = NULL;
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    if (SOPC_STATUS_OK == status)
    {
        // Treated after the local requests already pending (e.g. server startup Writes)
        status = SOPC_ServerHelper_LocalServiceSync(req, &response);
    }
    if (NULL != response)
    {
        SOPC_EncodeableObject_Delete(*(SOPC_EncodeableType**) response, &response);
    }
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    while (SOPC_STATUS_OK == status &&
           0 != SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType_Services))
    {
        if (SOPC_TimeReference_GetCurrent() - tStart > timeoutMs)
        {
            status = SOPC_STATUS_TIMEOUT;
        }
        SOPC_Sleep(1);
    }
    return status;
}

static void SOPC_TestBusy_TriggerRespCb(SOPC_EncodeableType* type, void* response, uintptr_t userContext)
{
    SOPC_UNUSED_ARG(type);
    SOPC_UNUSED_ARG(response);
    SOPC_UNUSED_ARG(userContext);
    SOPC_Atomic_Int_Set(&gTriggerResp, 1);
}

/* Arms the blocking hook and posts the trigger request, returns once the Services thread is blocked (*step set to the
 * failed step otherwise) */
static SOPC_ReturnStatus SOPC_TestBusy_TriggerBlock(uint32_t timeoutMs, const char** step)
{
    SOPC_Atomic_Int_Set(&gTriggerResp, 0);
    SOPC_Atomic_Int_Set(&gBlockServices, 1);

    OpcUa_WriteRequest* req = SOPC_TestBusy_CreateHeavyWriteRequest();
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceAsyncCustom(&SOPC_TestBusy_TriggerRespCb, req, 0);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &req);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        *step = "trigger request post";
    }
    else if (!SOPC_TestBusy_WaitAtomicValue(&gServicesBlocked, 1, timeoutMs))
    {
        *step = "Services thread blocking";
        status = SOPC_STATUS_TIMEOUT;
    }
    return status;
}

SOPC_ReturnStatus SOPC_TestBusy_BlockServices(uint32_t timeoutMs)
{
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool triggerTreated = false;
    // Failed step and hook calls at start reported for diagnosis
    const char* step = NULL;
    const int32_t hookCallsAtStart = SOPC_Atomic_Int_Get(&gHookCalls);

    while (SOPC_STATUS_OK == status && !triggerTreated)
    {
        status = SOPC_TestBusy_TriggerBlock(timeoutMs, &step);
        /*
         * The Services thread might be blocked by the hook call of an event treated before the trigger request (e.g.
         * the hook call of the previous request not done yet when armed): the trigger request is still pending.
         * Then release it and retry once the hook is called for the trigger request (ends of the current hook call
         * and of the trigger request one).
         */
        const int32_t hookCalls = SOPC_Atomic_Int_Get(&gHookCalls);
        triggerTreated = (SOPC_STATUS_OK == status && 0 == SOPC_TestBusy_GetServicesQueueSize(
                                                               SOPC_CommonMonitoring_QueueType_ServicesLocalRequests));
        if (SOPC_STATUS_OK == status && !triggerTreated)
        {
            printf("<SOPC_TestBusy: Services thread blocked before the trigger request treatment, retry\n");
            SOPC_TestBusy_ReleaseServices();
            if (!SOPC_TestBusy_WaitAtomicValue(&gTriggerResp, 1, timeoutMs) ||
                !SOPC_TestBusy_WaitHookCalls(hookCalls, 2u, timeoutMs))
            {
                step = "retry: trigger request treatment";
                status = SOPC_STATUS_TIMEOUT;
            }
            else if (SOPC_TimeReference_GetCurrent() - tStart > timeoutMs)
            {
                step = "retry: global timeout";
                status = SOPC_STATUS_TIMEOUT;
            }
        }
    }
    // The trigger response is sent by the Services thread before the hook call
    if (SOPC_STATUS_OK == status && !SOPC_TestBusy_WaitAtomicValue(&gTriggerResp, 1, timeoutMs))
    {
        step = "trigger response reception";
        status = SOPC_STATUS_TIMEOUT;
    }
    if (SOPC_STATUS_OK != status)
    {
        // State captured before the release for diagnosis
        printf(
            "<SOPC_TestBusy: Services thread not blocked after the trigger request treatment: failed step '%s' "
            "(status=%d), blocked=%" PRIi32 " triggerResp=%" PRIi32 " hookCalls=%" PRIi32 "->%" PRIi32
            " pendingLocal=%" PRIu32 " pendingExternal=%" PRIu32 "\n",
            (NULL != step) ? step : "unknown", (int) status, SOPC_Atomic_Int_Get(&gServicesBlocked),
            SOPC_Atomic_Int_Get(&gTriggerResp), hookCallsAtStart, SOPC_Atomic_Int_Get(&gHookCalls),
            SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType_ServicesLocalRequests),
            SOPC_TestBusy_GetServicesQueueSize(SOPC_CommonMonitoring_QueueType_ServicesExternalRequests));
        SOPC_TestBusy_ReleaseServices();
    }
    return status;
}

void SOPC_TestBusy_ReleaseServices(void)
{
    SOPC_Atomic_Int_Set(&gBlockServices, 0);
}

uint32_t SOPC_TestBusy_BatchSize(uint32_t high, bool aboveHigh)
{
    return aboveHigh ? high + SOPC_TEST_BUSY_BATCH_ABOVE_HIGH : high;
}

uint32_t SOPC_TestBusy_ExpectedRefusedInBatch(uint32_t nbRequests, uint32_t high, uint32_t low)
{
    // Busy state evaluated at the dequeue of the request i (1-based) with the remaining size nbRequests - i
    return (nbRequests > high) ? nbRequests - low - 1u : 0u;
}

void SOPC_TestBusy_SyncFileName(char* name, size_t size, bool serverBlocked, uint32_t batchIdx)
{
    int res = serverBlocked ? snprintf(name, size, SOPC_TEST_BUSY_EXT_BLOCKED_FILE_FMT, batchIdx)
                            : snprintf(name, size, SOPC_TEST_BUSY_EXT_READY_FILE_FMT, batchIdx);
    SOPC_ASSERT(res > 0 && (size_t) res < size);
}

SOPC_ReturnStatus SOPC_TestBusy_CreateFile(const char* path)
{
    FILE* fd = fopen(path, "w");
    if (NULL == fd)
    {
        return SOPC_STATUS_NOK;
    }
    return (0 == fclose(fd)) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_TestBusy_WaitFile(const char* path, uint32_t timeoutMs, int32_t* stop)
{
    const SOPC_TimeReference tStart = SOPC_TimeReference_GetCurrent();
    while (SOPC_TimeReference_GetCurrent() - tStart <= timeoutMs && (NULL == stop || 0 == SOPC_Atomic_Int_Get(stop)))
    {
        FILE* fd = fopen(path, "r");
        if (NULL != fd)
        {
            (void) fclose(fd);
            return SOPC_STATUS_OK;
        }
        SOPC_Sleep(10);
    }
    return SOPC_STATUS_TIMEOUT;
}
