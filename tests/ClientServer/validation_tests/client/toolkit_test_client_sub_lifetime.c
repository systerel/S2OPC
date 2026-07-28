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
 * \file
 *
 * Non-regression for subscription LifetimeCounter reset in keep-alive regime
 * (OPC UA Part 4 Table 85 rule #27 / ticket #1802).
 *
 * A subscription with RevisedMaxKeepAliveCount=3 and RevisedLifetimeCount=9 is kept
 * in keepAlive on a static monitored item. PublishRequests are deliberately held
 * back for slightly more than one publishing interval (repeated enough times to
 * exceed the lifetime if LifeCnt never recovers), then resumed. Empty ticks with
 * KeepAliveCounter > 1 stay in keepAlive (transition #16) and decrement LifeCnt.
 * Without the rule #27 reset when PublishingReqQueued is true, LifeCnt only
 * decreases and the subscription is deleted (BadNoSubscription). With the fix,
 * LifeCnt is restored on each cycle that has a queued PublishRequest and the
 * subscription survives.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_request_builder.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"

/* Short timing so the lifetime bug surfaces quickly.
 * MaxKeepAlive must be > 1 so an empty publishing tick stays in keepAlive via
 * transition #16 (KeepAliveCounter > 1). With MaxKeepAlive=1, an empty tick goes
 * LATE (#17) and the PublishRequest arrival path (#11) resets LifeCnt even without
 * the rule #27 keep-alive fix, which would hide the regression. */
#define REQ_PUBLISHING_INTERVAL_MS 200.0
#define REQ_MAX_KEEPALIVE_COUNT 3u
#define REQ_LIFETIME_COUNT 9u /* revised to max(req, 3 * revisedMaxKeepAlive) => 9 */
/* Gap slightly longer than one revised publishing interval. */
#define PUBLISH_GAP_MS 300u
/* Need more empty ticks than RevisedLifetimeCount to delete without LifeCnt reset. */
#define NB_PUBLISH_GAPS 10u
#define NB_WARMUP_PUBLISH 8u
#define NB_RESUME_PUBLISH_PER_GAP 1u
#define NB_STABILITY_PUBLISH 6u

#define STATIC_NODE_ID "ns=1;s=Boolean_001"

static const uint32_t sleepTimeout = 50;
static const uint32_t loopTimeout = 5000;

static int32_t sessionId = 0;
static int32_t sessionClosed = 0;
static int32_t unexpectedEvent = 0;
static int32_t badNoSubscription = 0;
static int32_t pendingResponses = 0;
static int32_t lastServiceResultGood = 0;

static uint32_t subscriptionId = 0;
static double revisedPublishingInterval = 0.0;
static uint32_t revisedLifetimeCount = 0;
static uint32_t revisedMaxKeepAliveCount = 0;

static void set_unexpected(void)
{
    SOPC_Atomic_Int_Set(&unexpectedEvent, 1);
}

static void handle_publish_result(SOPC_StatusCode serviceResult)
{
    if (OpcUa_BadNoSubscription == serviceResult)
    {
        printf(">>Test_Client_Sub_Lifetime: received BadNoSubscription\n");
        SOPC_Atomic_Int_Set(&badNoSubscription, 1);
        SOPC_Atomic_Int_Set(&lastServiceResultGood, 0);
    }
    else if (SOPC_IsGoodStatus(serviceResult))
    {
        SOPC_Atomic_Int_Set(&lastServiceResultGood, 1);
    }
    else
    {
        printf(">>Test_Client_Sub_Lifetime: unexpected Publish ServiceResult 0x%08" PRIX32 "\n", serviceResult);
        set_unexpected();
        SOPC_Atomic_Int_Set(&lastServiceResultGood, 0);
    }
}

static void Test_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    SOPC_UNUSED_ARG(appContext);

    if (SE_ACTIVATED_SESSION == event)
    {
        SOPC_ASSERT(idOrStatus <= INT32_MAX);
        SOPC_Atomic_Int_Set(&sessionId, (int32_t) idOrStatus);
        return;
    }

    if (SE_SESSION_ACTIVATION_FAILURE == event || SE_SND_REQUEST_FAILED == event)
    {
        printf(">>Test_Client_Sub_Lifetime: unexpected event %d\n", (int) event);
        set_unexpected();
        return;
    }

    if (SE_CLOSED_SESSION == event)
    {
        SOPC_Atomic_Int_Set(&sessionClosed, 1);
        return;
    }

    if (SE_RCV_SESSION_RESPONSE != event)
    {
        printf(">>Test_Client_Sub_Lifetime: unexpected event type %d\n", (int) event);
        set_unexpected();
        return;
    }

    if (NULL == param)
    {
        set_unexpected();
        return;
    }

    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;
    SOPC_Atomic_Int_Add(&pendingResponses, -1);

    if (&OpcUa_CreateSubscriptionResponse_EncodeableType == encType)
    {
        OpcUa_CreateSubscriptionResponse* resp = (OpcUa_CreateSubscriptionResponse*) param;
        if (!SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            printf(">>Test_Client_Sub_Lifetime: CreateSubscription failed 0x%08" PRIX32 "\n",
                   resp->ResponseHeader.ServiceResult);
            set_unexpected();
            return;
        }
        subscriptionId = resp->SubscriptionId;
        revisedPublishingInterval = resp->RevisedPublishingInterval;
        revisedLifetimeCount = resp->RevisedLifetimeCount;
        revisedMaxKeepAliveCount = resp->RevisedMaxKeepAliveCount;
        printf(">>Test_Client_Sub_Lifetime: CreateSubscription ok sub=%" PRIu32 " pubItv=%f lifetime=%" PRIu32
               " keepAlive=%" PRIu32 "\n",
               subscriptionId, revisedPublishingInterval, revisedLifetimeCount, revisedMaxKeepAliveCount);
        return;
    }

    if (&OpcUa_CreateMonitoredItemsResponse_EncodeableType == encType)
    {
        OpcUa_CreateMonitoredItemsResponse* resp = (OpcUa_CreateMonitoredItemsResponse*) param;
        if (!SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult) || 1 != resp->NoOfResults ||
            !SOPC_IsGoodStatus(resp->Results[0].StatusCode))
        {
            printf(">>Test_Client_Sub_Lifetime: CreateMonitoredItems failed\n");
            set_unexpected();
        }
        else
        {
            printf(">>Test_Client_Sub_Lifetime: CreateMonitoredItems ok\n");
        }
        return;
    }

    if (&OpcUa_PublishResponse_EncodeableType == encType)
    {
        OpcUa_PublishResponse* resp = (OpcUa_PublishResponse*) param;
        handle_publish_result(resp->ResponseHeader.ServiceResult);
        return;
    }

    if (&OpcUa_ServiceFault_EncodeableType == encType)
    {
        OpcUa_ServiceFault* fault = (OpcUa_ServiceFault*) param;
        handle_publish_result(fault->ResponseHeader.ServiceResult);
        return;
    }

    if (&OpcUa_DeleteSubscriptionsResponse_EncodeableType == encType)
    {
        OpcUa_DeleteSubscriptionsResponse* resp = (OpcUa_DeleteSubscriptionsResponse*) param;
        if (!SOPC_IsGoodStatus(resp->ResponseHeader.ServiceResult))
        {
            printf(">>Test_Client_Sub_Lifetime: DeleteSubscriptions failed 0x%08" PRIX32 "\n",
                   resp->ResponseHeader.ServiceResult);
            set_unexpected();
        }
        return;
    }

    printf(">>Test_Client_Sub_Lifetime: unexpected response type %s\n",
           NULL != encType && NULL != encType->TypeName ? encType->TypeName : "<null>");
    set_unexpected();
}

static bool wait_pending_cleared(const char* what)
{
    uint32_t loopCpt = 0;
    while (SOPC_Atomic_Int_Get(&pendingResponses) > 0 && 0 == SOPC_Atomic_Int_Get(&unexpectedEvent) &&
           0 == SOPC_Atomic_Int_Get(&badNoSubscription) && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }
    if (SOPC_Atomic_Int_Get(&pendingResponses) > 0)
    {
        printf(">>Test_Client_Sub_Lifetime: timeout waiting for %s (pending=%" PRId32 ")\n", what,
               SOPC_Atomic_Int_Get(&pendingResponses));
        return false;
    }
    return 0 == SOPC_Atomic_Int_Get(&unexpectedEvent) && 0 == SOPC_Atomic_Int_Get(&badNoSubscription);
}

static bool send_request(void* request, const char* what)
{
    if (NULL == request)
    {
        printf(">>Test_Client_Sub_Lifetime: failed to build %s\n", what);
        return false;
    }
    SOPC_Atomic_Int_Add(&pendingResponses, 1);
    SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get(&sessionId), request, 1);
    return true;
}

static bool send_publish(void)
{
    OpcUa_PublishRequest* req = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_PublishRequest_EncodeableType, (void**) &req);
    if (SOPC_STATUS_OK != status || NULL == req)
    {
        return false;
    }
    req->NoOfSubscriptionAcknowledgements = 0;
    req->SubscriptionAcknowledgements = NULL;
    return send_request(req, "PublishRequest");
}

static bool send_and_wait_publish(void)
{
    SOPC_Atomic_Int_Set(&lastServiceResultGood, 0);
    if (!send_publish())
    {
        return false;
    }
    if (!wait_pending_cleared("PublishResponse"))
    {
        return false;
    }
    if (1 != SOPC_Atomic_Int_Get(&lastServiceResultGood))
    {
        printf(">>Test_Client_Sub_Lifetime: Publish did not succeed\n");
        return false;
    }
    return true;
}

SOPC_Client_Config clientConfig;

SOPC_SecureChannel_Config scConfig = {.isClientSc = true,
                                      .clientConfigPtr = &clientConfig,
                                      .expectedEndpoints = NULL,
                                      .serverUri = NULL,
                                      .url = DEFAULT_ENDPOINT_URL,
                                      .peerAppCert = NULL,
                                      .reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI,
                                      .requestedLifetime = 20000,
                                      .msgSecurityMode = OpcUa_MessageSecurityMode_None};

int main(void)
{
    int mainResult = EXIT_FAILURE;
    uint32_t channel_config_idx = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool ok = true;

    SOPC_ClientConfig_Initialize(&clientConfig);

    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_sub_lifetime_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    status = SOPC_Common_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK != status)
    {
        printf(">>Test_Client_Sub_Lifetime: Common initialization failed\n");
        return EXIT_FAILURE;
    }

    status = SOPC_Toolkit_Initialize(Test_ComEvent_FctClient);
    if (SOPC_STATUS_OK != status)
    {
        printf(">>Test_Client_Sub_Lifetime: Toolkit initialization failed\n");
        SOPC_Common_Clear();
        return EXIT_FAILURE;
    }

    channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
    if (0 == channel_config_idx)
    {
        printf(">>Test_Client_Sub_Lifetime: Secure channel configuration failed\n");
        SOPC_Toolkit_Clear();
        SOPC_Common_Clear();
        return EXIT_FAILURE;
    }

    /* Activate anonymous session (None). */
    {
        SOPC_EndpointConnectionCfg endpointConnectionCfg = SOPC_EndpointConnectionCfg_CreateClassic(channel_config_idx);
        status = SOPC_ToolkitClient_AsyncActivateSession_Anonymous(endpointConnectionCfg, NULL, 1, "anonymous");
        if (SOPC_STATUS_OK != status)
        {
            ok = false;
        }
    }
    if (ok)
    {
        uint32_t loopCpt = 0;
        while (0 == SOPC_Atomic_Int_Get(&sessionId) && 0 == SOPC_Atomic_Int_Get(&unexpectedEvent) &&
               loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
        if (0 == SOPC_Atomic_Int_Get(&sessionId) || 0 != SOPC_Atomic_Int_Get(&unexpectedEvent))
        {
            printf(">>Test_Client_Sub_Lifetime: session activation failed\n");
            ok = false;
        }
    }

    /* CreateSubscription with short lifetime/keepalive. */
    if (ok)
    {
        OpcUa_CreateSubscriptionRequest* createSubReq = SOPC_CreateSubscriptionRequest_Create(
            REQ_PUBLISHING_INTERVAL_MS, REQ_LIFETIME_COUNT, REQ_MAX_KEEPALIVE_COUNT, 1000, true, 0);
        ok = send_request(createSubReq, "CreateSubscriptionRequest") && wait_pending_cleared("CreateSubscription");
        if (ok && (0 == subscriptionId || 9 != revisedLifetimeCount || 3 != revisedMaxKeepAliveCount))
        {
            printf(
                ">>Test_Client_Sub_Lifetime: unexpected revised params (need lifetime=9 keepAlive=3, got "
                "lifetime=%" PRIu32 " keepAlive=%" PRIu32 ")\n",
                revisedLifetimeCount, revisedMaxKeepAliveCount);
            ok = false;
        }
    }

    /* Monitored item on a static value: settle into keepAlive quickly. */
    if (ok)
    {
        char* nodeIds[1] = {STATIC_NODE_ID};
        OpcUa_CreateMonitoredItemsRequest* createMiReq = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
            subscriptionId, 1, nodeIds, OpcUa_TimestampsToReturn_Both);
        ok = send_request(createMiReq, "CreateMonitoredItemsRequest") && wait_pending_cleared("CreateMonitoredItems");
    }

    /* Warm-up: continuous Publish so the server reaches keepAlive with LifeCnt at max. */
    if (ok)
    {
        printf(">>Test_Client_Sub_Lifetime: warm-up Publish cycles\n");
        for (uint32_t i = 0; ok && i < NB_WARMUP_PUBLISH; i++)
        {
            ok = send_and_wait_publish();
        }
    }

    /*
     * Inject short Publish gaps: each gap lets PublishingTimer expire with an empty
     * request queue (LifeCnt--). Resume then provides a PublishRequest so rule #27
     * must reset LifeCnt. Without the reset, LifeCnt drifts down to deletion.
     */
    if (ok)
    {
        printf(">>Test_Client_Sub_Lifetime: injecting %" PRIu32 " Publish gaps of %" PRIu32 " ms\n", NB_PUBLISH_GAPS,
               PUBLISH_GAP_MS);
        for (uint32_t gap = 0; ok && gap < NB_PUBLISH_GAPS; gap++)
        {
            SOPC_Sleep(PUBLISH_GAP_MS);
            for (uint32_t i = 0; ok && i < NB_RESUME_PUBLISH_PER_GAP; i++)
            {
                ok = send_and_wait_publish();
            }
            if (ok)
            {
                printf(">>Test_Client_Sub_Lifetime: gap %" PRIu32 "/%" PRIu32 " recovered\n", gap + 1, NB_PUBLISH_GAPS);
            }
        }
    }

    /* Stability window: subscription must still answer keep-alives. */
    if (ok)
    {
        printf(">>Test_Client_Sub_Lifetime: stability Publish cycles\n");
        for (uint32_t i = 0; ok && i < NB_STABILITY_PUBLISH; i++)
        {
            ok = send_and_wait_publish();
        }
    }

    if (ok && 0 != SOPC_Atomic_Int_Get(&badNoSubscription))
    {
        ok = false;
    }

    /* Best-effort cleanup (do not fail the test on cleanup errors). */
    if (0 != SOPC_Atomic_Int_Get(&sessionId) && 0 != subscriptionId && ok)
    {
        OpcUa_DeleteSubscriptionsRequest* delReq = NULL;
        if (SOPC_STATUS_OK ==
                SOPC_EncodeableObject_Create(&OpcUa_DeleteSubscriptionsRequest_EncodeableType, (void**) &delReq) &&
            NULL != delReq)
        {
            delReq->NoOfSubscriptionIds = 1;
            delReq->SubscriptionIds = SOPC_Calloc(1, sizeof(uint32_t));
            if (NULL != delReq->SubscriptionIds)
            {
                delReq->SubscriptionIds[0] = subscriptionId;
                if (send_request(delReq, "DeleteSubscriptionsRequest"))
                {
                    (void) wait_pending_cleared("DeleteSubscriptions");
                }
            }
            else
            {
                SOPC_EncodeableObject_Delete(&OpcUa_DeleteSubscriptionsRequest_EncodeableType, (void**) &delReq);
            }
        }
    }
    if (0 != SOPC_Atomic_Int_Get(&sessionId))
    {
        SOPC_ToolkitClient_AsyncCloseSession((uint32_t) SOPC_Atomic_Int_Get(&sessionId));
        uint32_t loopCpt = 0;
        while (0 == SOPC_Atomic_Int_Get(&sessionClosed) && loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
    }

    if (ok && 0 == SOPC_Atomic_Int_Get(&unexpectedEvent) && 0 == SOPC_Atomic_Int_Get(&badNoSubscription))
    {
        printf(">>Test_Client_Sub_Lifetime: SUCCESS\n");
        mainResult = EXIT_SUCCESS;
    }
    else
    {
        printf(">>Test_Client_Sub_Lifetime: FAILURE (badNoSub=%" PRId32 " unexpected=%" PRId32 ")\n",
               SOPC_Atomic_Int_Get(&badNoSubscription), SOPC_Atomic_Int_Get(&unexpectedEvent));
        mainResult = EXIT_FAILURE;
    }

    SOPC_Toolkit_Clear();
    SOPC_Common_Clear();
    return mainResult;
}
