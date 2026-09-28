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
 * \brief Tests the server internal write behavior callback API
 *        (::SOPC_ServerInternal_RegisterWriteBehaviorCb and ::SOPC_ServerInternal_GetRegisteredWriteBehaviorCb):
 *        registration only in configuring state, lookup only in started (or stopping) state
 *        and callback call on local write of the node Value attribute.
 */

#include <stdio.h>

#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_threads.h"

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"
#include "libs2opc_server_internal.h"

#include "embedded/sopc_addspace_loader.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost:product"

#define WRITE_BEHAVIOR_CB_AUX_PARAM 1774
#define WRITE_BEHAVIOR_CB_VALUE 42

static const SOPC_NodeId writeBehaviorNodeId = SOPC_NODEID_NUMERIC(1, 1002);   // UInt32 variable
static const SOPC_NodeId noWriteBehaviorNodeId = SOPC_NODEID_NUMERIC(1, 1004); // String variable

static int32_t writeBehaviorCbCalls = 0;
static bool writeBehaviorCbParamsOk = false;
static uint32_t writeBehaviorCbLastValue = 0;
static int32_t serverStopped = 0;

static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&serverStopped, 1);
}

static void test_write_behavior_cb(const SOPC_NodeId* nodeId,
                                   const SOPC_DataValue* prevValue,
                                   const SOPC_DataValue* newValue,
                                   uintptr_t auxParam)
{
    writeBehaviorCbParamsOk = SOPC_NodeId_Equal(&writeBehaviorNodeId, nodeId) && NULL != prevValue &&
                              NULL != newValue && WRITE_BEHAVIOR_CB_AUX_PARAM == auxParam &&
                              SOPC_UInt32_Id == newValue->Value.BuiltInTypeId && SOPC_IsGoodStatus(newValue->Status);
    if (writeBehaviorCbParamsOk)
    {
        writeBehaviorCbLastValue = newValue->Value.Value.Uint32;
    }
    SOPC_Atomic_Int_Add(&writeBehaviorCbCalls, 1);
}

static bool is_write_behavior_cb_registered(void)
{
    SOPC_ServerInternal_WriteBehavior_Fct* callback = NULL;
    uintptr_t auxParam = 0;
    return SOPC_ServerInternal_GetRegisteredWriteBehaviorCb(&writeBehaviorNodeId, &callback, &auxParam) &&
           test_write_behavior_cb == callback && WRITE_BEHAVIOR_CB_AUX_PARAM == auxParam;
}

static SOPC_ReturnStatus configure_server(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config* ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
    // Only local services are used: an unsecured endpoint (no certificate nor PKI needed) is sufficient
    SOPC_SecurityPolicy* sp = (NULL == ep) ? NULL : SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
    if (NULL == sp)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
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
        status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC write behavior test server", NULL,
                                                                   OpcUa_ApplicationType_Server);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_AddressSpace* addressSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
        status = (NULL != addressSpace) ? SOPC_ServerConfigHelper_SetAddressSpace(addressSpace) : SOPC_STATUS_NOK;
    }
    printf("<Test_Server_Write_Behavior_Cb: server configuration: %s\n", SOPC_STATUS_OK == status ? "OK" : "NOK");
    return status;
}

// Shall be called in server configuring state
static SOPC_ReturnStatus check_register_configuring(void)
{
    SOPC_ReturnStatus status = SOPC_ServerInternal_RegisterWriteBehaviorCb(&writeBehaviorNodeId, test_write_behavior_cb,
                                                                           WRITE_BEHAVIOR_CB_AUX_PARAM);
    // Only one callback per node
    if (SOPC_STATUS_OK == status &&
        SOPC_STATUS_INVALID_PARAMETERS !=
            SOPC_ServerInternal_RegisterWriteBehaviorCb(&writeBehaviorNodeId, test_write_behavior_cb, 0))
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status && (SOPC_STATUS_INVALID_PARAMETERS !=
                                         SOPC_ServerInternal_RegisterWriteBehaviorCb(NULL, test_write_behavior_cb, 0) ||
                                     SOPC_STATUS_INVALID_PARAMETERS !=
                                         SOPC_ServerInternal_RegisterWriteBehaviorCb(&noWriteBehaviorNodeId, NULL, 0)))
    {
        status = SOPC_STATUS_NOK;
    }
    // Callback not available before server is started
    if (SOPC_STATUS_OK == status && is_write_behavior_cb_registered())
    {
        status = SOPC_STATUS_NOK;
    }
    printf("<Test_Server_Write_Behavior_Cb: register in configuring state: %s\n",
           SOPC_STATUS_OK == status ? "OK" : "NOK");
    return status;
}

static SOPC_ReturnStatus write_value_sync(const SOPC_NodeId* nodeId, const SOPC_DataValue* dv)
{
    OpcUa_WriteRequest* writeReq = SOPC_WriteRequest_Create(1);
    OpcUa_WriteResponse* writeResp = NULL;
    SOPC_ReturnStatus status = (NULL == writeReq) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_WriteRequest_SetWriteValue(writeReq, 0, nodeId, SOPC_AttributeId_Value, NULL, dv);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(writeReq, (void**) &writeResp);
        writeReq = NULL; // freed by toolkit
    }
    if (SOPC_STATUS_OK == status)
    {
        status = (&OpcUa_WriteResponse_EncodeableType == writeResp->encodeableType &&
                  SOPC_IsGoodStatus(writeResp->ResponseHeader.ServiceResult) && 1 == writeResp->NoOfResults &&
                  SOPC_IsGoodStatus(writeResp->Results[0]))
                     ? SOPC_STATUS_OK
                     : SOPC_STATUS_NOK;
    }
    if (NULL != writeResp)
    {
        SOPC_EncodeableObject_Delete(writeResp->encodeableType, (void**) &writeResp);
    }
    SOPC_EncodeableObject_Delete(&OpcUa_WriteRequest_EncodeableType, (void**) &writeReq);
    return status;
}

// Shall be called in server started state
static SOPC_ReturnStatus check_write_started(void)
{
    // Registration not possible once server is started
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE == SOPC_ServerInternal_RegisterWriteBehaviorCb(
                                                                &noWriteBehaviorNodeId, test_write_behavior_cb, 0)
                                   ? SOPC_STATUS_OK
                                   : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status && !is_write_behavior_cb_registered())
    {
        status = SOPC_STATUS_NOK;
    }

    // Write of node with callback: callback called once with new value
    if (SOPC_STATUS_OK == status)
    {
        SOPC_DataValue dv;
        SOPC_DataValue_Initialize(&dv);
        dv.Value.BuiltInTypeId = SOPC_UInt32_Id;
        dv.Value.Value.Uint32 = WRITE_BEHAVIOR_CB_VALUE;
        status = write_value_sync(&writeBehaviorNodeId, &dv);
    }
    if (SOPC_STATUS_OK == status && (1 != SOPC_Atomic_Int_Get(&writeBehaviorCbCalls) || !writeBehaviorCbParamsOk ||
                                     WRITE_BEHAVIOR_CB_VALUE != writeBehaviorCbLastValue))
    {
        status = SOPC_STATUS_NOK;
    }

    // Write of node without callback: no callback call
    if (SOPC_STATUS_OK == status)
    {
        SOPC_DataValue dv;
        SOPC_DataValue_Initialize(&dv);
        dv.Value.BuiltInTypeId = SOPC_String_Id;
        status = SOPC_String_AttachFromCstring(&dv.Value.Value.String, "No write behavior callback");
        if (SOPC_STATUS_OK == status)
        {
            status = write_value_sync(&noWriteBehaviorNodeId, &dv);
        }
    }
    if (SOPC_STATUS_OK == status && 1 != SOPC_Atomic_Int_Get(&writeBehaviorCbCalls))
    {
        status = SOPC_STATUS_NOK;
    }
    printf("<Test_Server_Write_Behavior_Cb: write in started state: %s\n", SOPC_STATUS_OK == status ? "OK" : "NOK");
    return status;
}

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    const uint32_t sleepTimeout = 50;
    const uint32_t loopTimeout = 5000;
    uint32_t loopCpt = 0;

    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_write_behavior_cb_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);

    // Registration not possible before server configuration is initialized
    if (SOPC_STATUS_OK == status &&
        SOPC_STATUS_INVALID_STATE != SOPC_ServerInternal_RegisterWriteBehaviorCb(
                                         &writeBehaviorNodeId, test_write_behavior_cb, WRITE_BEHAVIOR_CB_AUX_PARAM))
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = configure_server();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = check_register_configuring();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = check_write_started();
    }

    // Stop the server and check callback is not available anymore once stopped
    SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();
    while (SOPC_STATUS_OK == stopStatus && 0 == SOPC_Atomic_Int_Get(&serverStopped) &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }
    if (SOPC_STATUS_OK == stopStatus && 0 == SOPC_Atomic_Int_Get(&serverStopped))
    {
        stopStatus = SOPC_STATUS_TIMEOUT;
    }
    if (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus && is_write_behavior_cb_registered())
    {
        status = SOPC_STATUS_NOK;
    }

    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    printf("<Test_Server_Write_Behavior_Cb: final result: %s\n",
           SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus ? "OK" : "NOK");
    return (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus) ? 0 : 1;
}
