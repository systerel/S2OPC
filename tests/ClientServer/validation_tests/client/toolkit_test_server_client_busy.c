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
 * \file toolkit_test_server_client_busy.c
 *
 * \brief Validation test: external (client) service requests are refused with OpcUa_BadServerTooBusy when the
 *        number of requests pending in the Services queue of the server reaches
 *        ::SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL.
 *
 * The server is a separate process (toolkit_test_server_busy_external, started by with-opc-server.py): a client
 * of the same process would send its requests through the same Services thread than the server one, the number of
 * pending received requests would then depend on the relative speed of their sending and of their treatment.
 *
 * Hysteresis (deterministic): for each batch (see ::SOPC_TEST_BUSY_EXT_NB_BATCHES), the server blocks its Services
 * thread, the client sends the batch, the server releases its Services thread once the batch is pending in its Services
 * queue. A batch of HIGH requests is never refused, a batch of N > HIGH requests has its N - LOW - 1 first requests
 * refused and its LOW + 1 last requests accepted. A last synchronous request checks the server is no longer busy.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"
#include "sopc_toolkit_config_constants.h"

#include "sopc_test_services_queue_busy.h"

#define DEFAULT_APPLICATION_NAME "Test_Client_Busy_S2OPC"
#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256
#define CLI_CERT_PATH "./client_public/client_2k_cert.der"
#define CLI_KEY_PATH "./client_private/encrypted_client_2k_key.pem"

/* Maximum waiting time for a server synchronization file */
#define CLIENT_BUSY_SYNC_TIMEOUT_MS 60000u

static SOPC_ClientConnection* gConnection = NULL;

/* Batch results (protected by gBatchMutex) */
static SOPC_Mutex gBatchMutex;
static SOPC_Condition gBatchCond;
static uint32_t gBatchResp = 0;
static uint32_t gBatchBusy = 0;
static uint32_t gBatchOk = 0;
static bool gBatchBusyAfterOk = false;

static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
}

static void ClientBusy_RespCb(SOPC_EncodeableType* type, const void* response, uintptr_t userContext)
{
    SOPC_UNUSED_ARG(userContext);
    const bool isBusy = SOPC_TestBusy_IsTooBusyResponse(type, response);

    SOPC_Mutex_Lock(&gBatchMutex);
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
    SOPC_Condition_SignalAll(&gBatchCond);
    SOPC_Mutex_Unlock(&gBatchMutex);
}

static SOPC_ReturnStatus Client_Configure(SOPC_SecureConnection_Config** outSecureConnConfig)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    SOPC_PKIProvider* pkiProvider = NULL;
    SOPC_SecureConnection_Config* secureConnConfig = NULL;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetApplicationDescription(SOPC_TEST_BUSY_APPLICATION_URI,
                                                                   SOPC_TEST_BUSY_PRODUCT_URI, DEFAULT_APPLICATION_NAME,
                                                                   "en-US", OpcUa_ApplicationType_Client);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(CLI_CERT_PATH, CLI_KEY_PATH, true);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(SOPC_TEST_BUSY_PKI_PATH, &pkiProvider);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);
    }
    if (SOPC_STATUS_OK == status)
    {
        secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("BusyTest", SOPC_TEST_BUSY_ENDPOINT_URL,
                                                                          MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
        status = (NULL != secureConnConfig) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_SecureConnectionConfig_SetServerCertificateFromPath(secureConnConfig, SOPC_TEST_BUSY_SERVER_CERT_PATH);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetAnonymous(secureConnConfig, "anonymous");
    }
    if (SOPC_STATUS_OK == status)
    {
        *outSecureConnConfig = secureConnConfig;
    }

    return status;
}

static SOPC_ReturnStatus ClientBusy_SendAsyncWrite(void)
{
    OpcUa_WriteRequest* req = SOPC_TestBusy_CreateHeavyWriteRequest();
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    if (SOPC_STATUS_OK == status)
    {
        // The request is managed by the client helper even in case of error
        status = SOPC_ClientHelper_ServiceAsyncCustom(gConnection, req, 0, &ClientBusy_RespCb);
    }

    return status;
}

/* Sends the batch batchIdx once the server Services thread is blocked and checks the number of refused requests */
static SOPC_ReturnStatus ClientBusy_CheckBatch(uint32_t batchIdx)
{
    const uint32_t nbRequests =
        SOPC_TestBusy_BatchSize((uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL, 1u <= batchIdx);
    const uint32_t expectedBusy =
        SOPC_TestBusy_ExpectedRefusedInBatch(nbRequests, (uint32_t) SOPC_SERVICES_QUEUE_BUSY_HIGH_EXTERNAL,
                                             (uint32_t) SOPC_SERVICES_QUEUE_BUSY_LOW_EXTERNAL);
    char name[128];
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (nbRequests > (uint32_t) SOPC_MAX_PENDING_REQUESTS)
    {
        printf("<Test_Server_Client_Busy: FAIL batch of %" PRIu32 " requests above SOPC_MAX_PENDING_REQUESTS=%d\n",
               nbRequests, (int) SOPC_MAX_PENDING_REQUESTS);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_Mutex_Lock(&gBatchMutex);
    gBatchResp = 0;
    gBatchBusy = 0;
    gBatchOk = 0;
    gBatchBusyAfterOk = false;
    SOPC_Mutex_Unlock(&gBatchMutex);

    SOPC_TestBusy_SyncFileName(name, sizeof(name), false, batchIdx);
    status = SOPC_TestBusy_CreateFile(name);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_TestBusy_SyncFileName(name, sizeof(name), true, batchIdx);
        status = SOPC_TestBusy_WaitFile(name, CLIENT_BUSY_SYNC_TIMEOUT_MS, NULL);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client_Busy: FAIL server Services thread not blocked for batch %" PRIu32 "\n",
                   batchIdx);
        }
    }
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < nbRequests; i++)
    {
        status = ClientBusy_SendAsyncWrite();
    }

    // Wait for all the batch responses
    SOPC_Mutex_Lock(&gBatchMutex);
    SOPC_ReturnStatus waitStatus = SOPC_STATUS_OK;
    while (SOPC_STATUS_OK == status && SOPC_STATUS_OK == waitStatus && gBatchResp < nbRequests)
    {
        waitStatus = SOPC_Mutex_UnlockAndTimedWaitCond(&gBatchCond, &gBatchMutex, SOPC_TEST_BUSY_DRAIN_TIMEOUT_MS);
    }
    if (SOPC_STATUS_OK == status && gBatchResp < nbRequests)
    {
        printf("<Test_Server_Client_Busy: FAIL batch responses %" PRIu32 "/%" PRIu32 "\n", gBatchResp, nbRequests);
        status = SOPC_STATUS_TIMEOUT;
    }
    if (SOPC_STATUS_OK == status && (expectedBusy != gBatchBusy || gBatchBusyAfterOk))
    {
        printf("<Test_Server_Client_Busy: FAIL batch of %" PRIu32 " requests: %" PRIu32 " refused (expected %" PRIu32
               "), refused after accepted: %d\n",
               nbRequests, gBatchBusy, expectedBusy, (int) gBatchBusyAfterOk);
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Client_Busy: batch of %" PRIu32 " requests: %" PRIu32 " refused, %" PRIu32 " accepted\n",
               nbRequests, gBatchBusy, gBatchOk);
    }
    SOPC_Mutex_Unlock(&gBatchMutex);

    return status;
}

/* A last synchronous request shall be accepted: the server is no longer busy */
static SOPC_ReturnStatus ClientBusy_CheckNotBusy(void)
{
    OpcUa_WriteRequest* req = SOPC_TestBusy_CreateHeavyWriteRequest();
    void* response = NULL;
    SOPC_ReturnStatus status = (NULL != req) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(gConnection, req, &response);
    }
    if (SOPC_STATUS_OK == status && NULL != response)
    {
        SOPC_EncodeableType* type = *(SOPC_EncodeableType**) response;
        if (SOPC_TestBusy_IsTooBusyResponse(type, response))
        {
            printf("<Test_Server_Client_Busy: FAIL request refused after the batches\n");
            status = SOPC_STATUS_NOK;
        }
        SOPC_EncodeableObject_Delete(type, &response);
    }
    else if (SOPC_STATUS_OK == status)
    {
        status = SOPC_STATUS_NOK;
    }

    return status;
}

int main(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    SOPC_SecureConnection_Config* secConnConfig = NULL;

    if (NULL == getenv(PASSWORD_ENV_NAME))
    {
        printf(
            "<Test_Server_Client_Busy: FAIL missing environment variable %s\n"
            "  e.g. %s=password ./toolkit_test_server_client_busy\n",
            PASSWORD_ENV_NAME, PASSWORD_ENV_NAME);
        return 1;
    }

    status = SOPC_Mutex_Initialization(&gBatchMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Condition_Init(&gBatchCond);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    logConfiguration.logLevel = SOPC_LOG_LEVEL_ERROR;
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_client_busy_logs/";
    status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = Client_Configure(&secConnConfig);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(secConnConfig, &SOPC_Client_ConnEventCb, &gConnection);
    }
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < SOPC_TEST_BUSY_EXT_NB_BATCHES; i++)
    {
        status = ClientBusy_CheckBatch(i);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = ClientBusy_CheckNotBusy();
    }

    if (NULL != gConnection)
    {
        (void) SOPC_ClientHelper_Disconnect(&gConnection);
    }
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
    SOPC_Condition_Clear(&gBatchCond);
    SOPC_Mutex_Clear(&gBatchMutex);

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Client_Busy: PASSED\n");
        return 0;
    }

    printf("<Test_Server_Client_Busy: FAILED status=%d\n", (int) status);
    return 1;
}
