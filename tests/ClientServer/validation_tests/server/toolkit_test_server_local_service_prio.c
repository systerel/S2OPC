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
 * \brief Checks that ::SOPC_ServerHelper_LocalServicePrioAsyncCustom request is treated
 *        before already pending local service requests.
 *
 * To be deterministic, the local service requests are sent from a method call callback,
 * which is executed by the services thread: no request can be treated before the callback returns.
 * The method is called by a client connected without security (None).
 */

#include <inttypes.h>
#include <stdio.h>

#include "opcua_identifiers.h"
#include "sopc_atomic.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"

#define NB_NORMAL_REQS 3
#define NB_REQS (NB_NORMAL_REQS + 1)
#define PRIO_REQ_CTX NB_NORMAL_REQS

#define WAIT_STEP_MS 50
#define WAIT_TIMEOUT_MS 5000

static const SOPC_NodeId testObjectId = SOPC_NODEID_STRING(1, "TestObject");
static const SOPC_NodeId methodNoArgId = SOPC_NODEID_STRING(1, "MethodNoArg");
static const SOPC_NodeId serverStateId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_ServerStatus_State);

// Written by the application thread only (local service responses are received sequentially)
static uintptr_t respOrder[NB_REQS];
static int32_t nbResps = 0;
static int32_t sendFailure = false;

static void LocalServiceRespCb(SOPC_EncodeableType* encType, void* response, uintptr_t userContext)
{
    SOPC_UNUSED_ARG(encType);
    SOPC_UNUSED_ARG(response);
    int32_t idx = SOPC_Atomic_Int_Get(&nbResps);
    if (idx < NB_REQS)
    {
        respOrder[idx] = userContext;
    }
    SOPC_Atomic_Int_Add(&nbResps, 1);
}

static bool SendLocalRead(bool isPrio, uintptr_t userContext)
{
    OpcUa_ReadRequest* req = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Neither);
    SOPC_ReturnStatus status = (NULL == req) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ReadRequest_SetReadValue(req, 0, &serverStateId, SOPC_AttributeId_Value, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = isPrio ? SOPC_ServerHelper_LocalServicePrioAsyncCustom(&LocalServiceRespCb, req, userContext)
                        : SOPC_ServerHelper_LocalServiceAsyncCustom(&LocalServiceRespCb, req, userContext);
        req = (SOPC_STATUS_OK == status) ? NULL : req;
    }
    if (NULL != req)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &req);
    }
    return SOPC_STATUS_OK == status;
}

// Executed by the services thread: requests are only treated once it returns
static SOPC_StatusCode MethodSendLocalReqs(const SOPC_CallContext* callContextPtr,
                                           const SOPC_NodeId* objectId,
                                           uint32_t nbInputArgs,
                                           const SOPC_Variant* inputArgs,
                                           uint32_t* nbOutputArgs,
                                           SOPC_Variant** outputArgs,
                                           void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    bool res = true;
    for (uintptr_t i = 0; res && i < NB_NORMAL_REQS; i++)
    {
        res = SendLocalRead(false, i);
    }
    res = res && SendLocalRead(true, PRIO_REQ_CTX);
    if (!res)
    {
        SOPC_Atomic_Int_Set(&sendFailure, true);
    }
    return SOPC_GoodGenericStatus;
}

static void ClientConnectionEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
}

static void ServerStoppedCb(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
}

static SOPC_ReturnStatus Server_Configure(void)
{
    SOPC_ReturnStatus status = SOPC_ServerConfigHelper_Initialize();
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(APPLICATION_URI, APPLICATION_URI,
                                                                   "S2OPC local service prio test server", NULL,
                                                                   OpcUa_ApplicationType_Server);
    }
    SOPC_Endpoint_Config* ep = NULL;
    if (SOPC_STATUS_OK == status)
    {
        ep = SOPC_ServerConfigHelper_CreateEndpoint(ENDPOINT_URL, true);
        status = (NULL == ep) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    SOPC_SecurityPolicy* sp = NULL;
    if (SOPC_STATUS_OK == status)
    {
        sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
        status = (NULL == sp) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_None);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_AddressSpace* addSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
        status = (NULL == addSpace) ? SOPC_STATUS_NOK : SOPC_ServerConfigHelper_SetAddressSpace(addSpace);
    }
    SOPC_MethodCallManager* mcm = NULL;
    if (SOPC_STATUS_OK == status)
    {
        mcm = SOPC_MethodCallManager_Create();
        status = (NULL == mcm) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(mcm, &methodNoArgId, &MethodSendLocalReqs, NULL, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Reduce shutdown phase duration for test
        status = SOPC_ServerConfigHelper_SetShutdownCountdown(1);
    }
    return status;
}

static SOPC_ReturnStatus Client_Configure(SOPC_SecureConnection_Config** outSecConnConfig)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetApplicationDescription(APPLICATION_URI, APPLICATION_URI,
                                                                   "S2OPC local service prio test client", NULL,
                                                                   OpcUa_ApplicationType_Client);
    }
    if (SOPC_STATUS_OK == status)
    {
        *outSecConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
            "1", ENDPOINT_URL, OpcUa_MessageSecurityMode_None, SOPC_SecurityPolicy_None);
        status = (NULL == *outSecConnConfig) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    return status;
}

// Calls the test method through a client session and waits for all local service responses
static SOPC_ReturnStatus CallMethodAndWaitResps(SOPC_SecureConnection_Config* secConnConfig)
{
    SOPC_ClientConnection* connection = NULL;
    SOPC_ReturnStatus status = SOPC_ClientHelper_Connect(secConnConfig, &ClientConnectionEventCb, &connection);
    OpcUa_CallRequest* callReq = NULL;
    OpcUa_CallResponse* callResp = NULL;
    if (SOPC_STATUS_OK == status)
    {
        callReq = SOPC_CallRequest_Create(1);
        status = (NULL == callReq) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CallRequest_SetMethodToCall(callReq, 0, &testObjectId, &methodNoArgId, 0, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Request is deallocated by the client helper
        status = SOPC_ClientHelper_ServiceSync(connection, callReq, (void**) &callResp);
        callReq = NULL;
    }
    if (SOPC_STATUS_OK == status && (&OpcUa_CallResponse_EncodeableType != callResp->encodeableType ||
                                     1 != callResp->NoOfResults || !SOPC_IsGoodStatus(callResp->Results[0].StatusCode)))
    {
        printf("<Test_Server_Local_Service_Prio: method call failed\n");
        status = SOPC_STATUS_NOK;
    }
    if (NULL != callReq)
    {
        SOPC_EncodeableObject_Delete(&OpcUa_CallRequest_EncodeableType, (void**) &callReq);
    }
    if (NULL != callResp)
    {
        SOPC_EncodeableObject_Delete(callResp->encodeableType, (void**) &callResp);
    }

    uint32_t waitedMs = 0;
    while (SOPC_STATUS_OK == status && NB_REQS != SOPC_Atomic_Int_Get(&nbResps) && waitedMs < WAIT_TIMEOUT_MS)
    {
        SOPC_Sleep(WAIT_STEP_MS);
        waitedMs += WAIT_STEP_MS;
    }

    if (NULL != connection)
    {
        SOPC_ReturnStatus discStatus = SOPC_ClientHelper_Disconnect(&connection);
        status = (SOPC_STATUS_OK == status) ? discStatus : status;
    }
    return status;
}

// Expected order: priority request first, then the normal requests in FIFO order
static bool CheckRespOrder(void)
{
    bool res = !SOPC_Atomic_Int_Get(&sendFailure) && NB_REQS == SOPC_Atomic_Int_Get(&nbResps);
    for (uintptr_t i = 0; res && i < NB_REQS; i++)
    {
        uintptr_t expected = (0 == i) ? PRIO_REQ_CTX : i - 1;
        res = (expected == respOrder[i]);
    }
    printf("<Test_Server_Local_Service_Prio: %" PRId32 " responses received in order:", SOPC_Atomic_Int_Get(&nbResps));
    for (int32_t i = 0; i < SOPC_Atomic_Int_Get(&nbResps) && i < NB_REQS; i++)
    {
        printf(" %" PRIuPTR, respOrder[i]);
    }
    printf(" (expected: %d then 0..%d)\n", PRIO_REQ_CTX, NB_NORMAL_REQS - 1);
    return res;
}

int main(void)
{
    SOPC_Log_Configuration logConfig = SOPC_Common_GetDefaultLogConfiguration();
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_local_service_prio_logs/";
    logConfig.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig, NULL);

    SOPC_SecureConnection_Config* secConnConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = Client_Configure(&secConnConfig);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = Server_Configure();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(&ServerStoppedCb);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = CallMethodAndWaitResps(secConnConfig);
        SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();
        status = (SOPC_STATUS_OK == status) ? stopStatus : status;
    }

    bool res = (SOPC_STATUS_OK == status) && CheckRespOrder();

    SOPC_ClientConfigHelper_Clear();
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (res)
    {
        printf("<Test_Server_Local_Service_Prio: OK\n");
    }
    else
    {
        printf("<Test_Server_Local_Service_Prio: NOK (status=%d)\n", (int) status);
    }
    return res ? 0 : 1;
}
