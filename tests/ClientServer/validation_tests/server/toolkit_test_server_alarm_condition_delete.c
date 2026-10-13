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
 * \brief Validates ::SOPC_AlarmCondition_Delete and A&C instance lifecycle robustness:
 *        - nominal lifecycle: Create, TriggerEvent, Delete, recreate
 *        - duplicate creation on the same condition node is refused
 *        - Create then immediate Delete while async responses are in flight
 *        - TriggerEvent right after Create (A&C mutexes lock order, deadlock detected by CTest timeout)
 *        - Write responses in error treated after Delete (no access to the deleted instance data)
 *        - client method calls (Enable / Disable) on a deleted instance and state change events
 *          still queued in the A&C looper when the instance is deleted (in-process OPC UA client)
 *
 * Uses demo alarms address space (EqtNoConf / AlarmNoConf, ns=1;i=5008/5009).
 * Run from build/bin with demo XML env vars (see CMake test properties).
 *
 * CreateFromEvent returns before async TranslateBrowsePathsToNodeIds completes.
 * TriggerEvent needs 0:EventId in ac->nodeIds; calling it too early logs
 * SOPC_STATUS_INVALID_STATE on the EventId node write. The nominal lifecycle test
 * uses fixed sleeps (test harness only, not library code) until a readiness API exists,
 * see AC_CREATE_INIT_MS and sibling constants below. Stress tests deliberately do not wait.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_alarm_conditions.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_encodeabletype.h"
#include "sopc_enums.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"

/* EqtNoConf (EventNotifier) and AlarmNoConf from s2opc_demo_alarms_origin.xml */
static const SOPC_NodeId g_notifierNode = SOPC_NODEID_NUMERIC(1, 5008);
static const SOPC_NodeId g_conditionNode = SOPC_NODEID_NUMERIC(1, 5009);
static const SOPC_NodeId g_alarmConditionTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_AlarmConditionType);
/* AlarmNoConf instance methods and ConditionType methods */
static const SOPC_NodeId g_disableInstMethodId = SOPC_NODEID_NUMERIC(1, 7064);
static const SOPC_NodeId g_enableInstMethodId = SOPC_NODEID_NUMERIC(1, 7065);
static const SOPC_NodeId g_disableTypeMethodId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType_Disable);

/*
 * Fixed delays for async server/A&C work (test harness only).
 *
 * SERVER_START_SETTLE_MS: StartServer returns before address space and event
 *   types are fully usable
 *
 * AC_CREATE_INIT_MS: CreateFromEvent posts TBP on the A&C looper and returns
 *   immediately. TriggerEvent writes 0:EventId via GetNodeId(ac->nodeIds);
 *   without this wait, EventId node update fails with INVALID_STATE (status 3).
 *
 * AFTER_TRIGGER_MS: TriggerEvent posts async Write requests for EventId and
 *   related fields. Brief pause before Delete avoids in-flight write callbacks
 *   on the instance being torn down (TryLockAc drops them, but may still log).
 *
 * LOOPER_DRAIN_MS: Let pending TBP/Write responses on the A&C looper finish
 *   before Mgr_Clear so shutdown stays quiet under ASan/CI.
 */
static const uint32_t SERVER_START_SETTLE_MS = 200;
static const uint32_t AC_CREATE_INIT_MS = 500;
static const uint32_t AFTER_TRIGGER_MS = 200;
static const uint32_t LOOPER_DRAIN_MS = 500;

/*
 * Stress loops run without any sleep, so that async TBP / Write responses are
 * still in flight when the instance is triggered or deleted.
 *
 * NB_CREATE_DELETE_ITERATIONS: CreateFromEvent then immediate Delete on the same
 *   condition node (TBP response must be dropped as stale, the same address may
 *   be reused by the next creation).
 *
 * NB_CREATE_TRIGGER_ITERATIONS / NB_TRIGGER_PER_CREATE: TriggerEvent in a tight
 *   loop right after CreateFromEvent, while the looper treats the TBP response.
 *   Regression for the lock order between the A&C global mutex and ac->mut: a
 *   deadlock makes the test hang until the CTest timeout. Also run with a wrongly typed
 *   condition field so that Write responses in error are treated after Delete.
 */
static const uint32_t NB_CREATE_DELETE_ITERATIONS = 100;
static const uint32_t NB_CREATE_TRIGGER_ITERATIONS = 20;
static const uint32_t NB_TRIGGER_PER_CREATE = 50;

/*
 * STATE_CB_BLOCK_MS: duration for which the EnabledState callback blocks the A&C looper,
 *   so that a method call state change is still queued when the instance is deleted.
 * STATE_CB_WAIT_MS: maximum wait for the EnabledState callback to be called.
 */
static const uint32_t STATE_CB_BLOCK_MS = 500;
static const uint32_t STATE_CB_WAIT_MS = 5000;

static int32_t g_stopped = 0;

static void stop_cb(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&g_stopped, 1);
}

static SOPC_ReturnStatus load_config(void)
{
    const char* xml_server = getenv("TEST_SERVER_XML_CONFIG");
    const char* xml_as = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users = getenv("TEST_USERS_XML_CONFIG");

    if (NULL == xml_server || NULL == xml_as || NULL == xml_users)
    {
        fprintf(stderr,
                "Missing XML env vars. Example (from build/bin):\n"
                "  TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml \\\n"
                "  TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_Alarms_NodeSet.xml \\\n"
                "  TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml \\\n"
                "  TEST_PASSWORD_PRIVATE_KEY=password \\\n"
                "  toolkit_test_server_alarm_condition_delete\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_ServerConfigHelper_ConfigureFromXML(xml_server, xml_as, xml_users, NULL);
}

/*
 * Builds the condition event. When \p wrongMessageType is set, 0:Message is a String
 * whereas the node DataType is LocalizedText: the Write of this field fails with
 * Bad_TypeMismatch, which exercises the A&C Write response error path.
 */
static SOPC_Event* make_condition_event(bool wrongMessageType)
{
    SOPC_Event* event = NULL;
    SOPC_ReturnStatus status = SOPC_ServerHelper_CreateEvent(&g_alarmConditionTypeId, &event);
    if (SOPC_STATUS_OK != status || NULL == event)
    {
        return NULL;
    }

    const SOPC_Variant quality = SOPC_VARIANT_STATUSCODE(SOPC_GoodGenericStatus);
    const SOPC_Variant comment = SOPC_VARIANT_LOCALTEXT("", "");
    const SOPC_Variant suppressed = SOPC_VARIANT_BOOL(false);
    const SOPC_Variant message = SOPC_VARIANT_LOCALTEXT("en-US", "toolkit_test_server_alarm_condition_delete");
    const SOPC_Variant wrongTypeMessage = SOPC_VARIANT_STRING("toolkit_test_server_alarm_condition_delete");

    status = SOPC_Event_SetVariableFromStrPath(event, "0:Quality", &quality);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(event, "0:Comment", &comment);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(event, "0:SuppressedOrShelved", &suppressed);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetVariableFromStrPath(event, "0:Message", wrongMessageType ? &wrongTypeMessage : &message);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Event_Delete(&event);
        return NULL;
    }
    return event;
}

static SOPC_ReturnStatus create_condition_ext(bool wrongMessageType, SOPC_AlarmCondition** outAc)
{
    SOPC_Event* event = make_condition_event(wrongMessageType);
    if (NULL == event)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_AlarmCondition_CreateFromEvent(&g_notifierNode, &g_conditionNode, event, outAc);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Event_Delete(&event);
    }
    return status;
}

static SOPC_ReturnStatus create_condition(SOPC_AlarmCondition** outAc)
{
    return create_condition_ext(false, outAc);
}

static SOPC_ReturnStatus run_delete_lifecycle_test(void)
{
    SOPC_AlarmCondition* ac = NULL;
    SOPC_ReturnStatus status = create_condition(&ac);
    printf("[1] CreateFromEvent status=%d\n", (int) status);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    /* Wait for TBP to fill ac->nodeIds before first TriggerEvent (see AC_CREATE_INIT_MS). */
    SOPC_Sleep(AC_CREATE_INIT_MS);

    status = SOPC_AlarmCondition_TriggerEvent(ac);
    printf("[2] TriggerEvent (refresh dict) status=%d\n", (int) status);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    /* Let TriggerEvent async Writes drain before Delete (see AFTER_TRIGGER_MS). */
    SOPC_Sleep(AFTER_TRIGGER_MS);

    status = SOPC_AlarmCondition_Delete(&ac);
    printf("[3] Delete status=%d ac=%p\n", (int) status, (void*) ac);
    if (SOPC_STATUS_OK != status || NULL != ac)
    {
        return (SOPC_STATUS_OK != status) ? status : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_INVALID_PARAMETERS != SOPC_AlarmCondition_Delete(&ac))
    {
        printf("[4] Double Delete expected INVALID_PARAMETERS\n");
        return SOPC_STATUS_NOK;
    }
    printf("[4] Double Delete returned INVALID_PARAMETERS as expected\n");

    status = create_condition(&ac);
    printf("[5] CreateFromEvent after Delete status=%d\n", (int) status);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    /* Same TBP race as step [1]: second CreateFromEvent also returns before nodeIds ready. */
    SOPC_Sleep(AC_CREATE_INIT_MS);

    status = SOPC_AlarmCondition_TriggerEvent(ac);
    printf("[6] TriggerEvent after recreate status=%d\n", (int) status);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    status = SOPC_AlarmCondition_Delete(&ac);
    printf("[7] Final Delete status=%d\n", (int) status);
    if (SOPC_STATUS_OK != status || NULL != ac)
    {
        return (SOPC_STATUS_OK != status) ? status : SOPC_STATUS_NOK;
    }

    /* Drain A&C looper before process exit (see LOOPER_DRAIN_MS). */
    SOPC_Sleep(LOOPER_DRAIN_MS);
    return SOPC_STATUS_OK;
}

/* A second creation on a condition node already used by a live instance shall be refused */
static SOPC_ReturnStatus run_duplicate_create_test(void)
{
    SOPC_AlarmCondition* ac = NULL;
    SOPC_ReturnStatus status = create_condition(&ac);
    if (SOPC_STATUS_OK != status)
    {
        printf("[dup] first CreateFromEvent status=%d\n", (int) status);
        return status;
    }

    SOPC_AlarmCondition* dupAc = NULL;
    SOPC_ReturnStatus dupStatus = create_condition(&dupAc);
    printf("[dup] second CreateFromEvent status=%d ac=%p\n", (int) dupStatus, (void*) dupAc);

    status = SOPC_AlarmCondition_Delete(&ac);
    if (SOPC_STATUS_INVALID_STATE != dupStatus || NULL != dupAc)
    {
        return SOPC_STATUS_NOK;
    }
    return status;
}

/* CreateFromEvent then immediate Delete, the TBP response of each instance being still in flight */
static SOPC_ReturnStatus run_create_delete_loop_test(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < NB_CREATE_DELETE_ITERATIONS; i++)
    {
        SOPC_AlarmCondition* ac = NULL;
        status = create_condition(&ac);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AlarmCondition_Delete(&ac);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("[create/delete] iteration %" PRIu32 " failed with status=%d\n", i, (int) status);
        }
    }
    printf("[create/delete] %" PRIu32 " iterations status=%d\n", NB_CREATE_DELETE_ITERATIONS, (int) status);
    SOPC_Sleep(LOOPER_DRAIN_MS);
    return status;
}

/*
 * TriggerEvent in a tight loop right after CreateFromEvent, then Delete (see NB_CREATE_TRIGGER_ITERATIONS).
 * With \p wrongMessageType, Write responses in error are treated after Delete: their error
 * logging shall not access variable paths owned by the deleted instance.
 */
static SOPC_ReturnStatus run_trigger_right_after_create_test(bool wrongMessageType)
{
    const char* testName = wrongMessageType ? "[create/trigger/write error]" : "[create/trigger]";
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < NB_CREATE_TRIGGER_ITERATIONS; i++)
    {
        SOPC_AlarmCondition* ac = NULL;
        status = create_condition_ext(wrongMessageType, &ac);
        for (uint32_t j = 0; SOPC_STATUS_OK == status && j < NB_TRIGGER_PER_CREATE; j++)
        {
            status = SOPC_AlarmCondition_TriggerEvent(ac);
        }
        if (NULL != ac)
        {
            SOPC_ReturnStatus delStatus = SOPC_AlarmCondition_Delete(&ac);
            status = (SOPC_STATUS_OK == status ? delStatus : status);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("%s iteration %" PRIu32 " failed with status=%d\n", testName, i, (int) status);
        }
    }
    printf("%s %" PRIu32 " iterations status=%d\n", testName, NB_CREATE_TRIGGER_ITERATIONS, (int) status);
    SOPC_Sleep(LOOPER_DRAIN_MS);
    return status;
}

/*---------------------------------------------------------------------------
 *         In-process client used to call A&C methods (Call is not a local service)
 *---------------------------------------------------------------------------*/

static void client_conn_event_cb(SOPC_ClientConnection* config,
                                 SOPC_ClientConnectionEvent event,
                                 SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    printf("[client] unexpected connection event\n");
}

/* Same client configuration as toolkit_test_client_alarms for the demo server (4k certificates, user "me") */
static SOPC_ReturnStatus client_configure(SOPC_SecureConnection_Config** outScConfig)
{
    const char* password = getenv("TEST_PASSWORD_USER_ME");
    if (NULL == password)
    {
        printf("[client] TEST_PASSWORD_USER_ME not set\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath("client_public/client_4k_cert.der",
                                                                "client_private/encrypted_client_4k_key.pem", true);
    }
    SOPC_PKIProvider* clientPKI = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore("./S2OPC_Demo_PKI_Client", &clientPKI);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(clientPKI);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
    SOPC_SecureConnection_Config* scConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        scConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
            "0", "opc.tcp://localhost:4841", OpcUa_MessageSecurityMode_Sign, SOPC_SecurityPolicy_Basic256Sha256);
        status = (NULL == scConfig ? SOPC_STATUS_NOK : SOPC_STATUS_OK);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(scConfig, "server_public/server_4k_cert.der");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetUserName(scConfig, "username_Basic256Sha256", "me", password);
    }
    if (SOPC_STATUS_OK == status)
    {
        *outScConfig = scConfig;
    }
    return status;
}

/* Calls a method without arguments on the condition node, returns the method call result status code */
static SOPC_StatusCode client_call_method(SOPC_ClientConnection* conn, const SOPC_NodeId* methodId)
{
    SOPC_StatusCode result = OpcUa_BadInternalError;
    OpcUa_CallResponse* resp = NULL;
    OpcUa_CallRequest* req = SOPC_CallRequest_Create(1);
    SOPC_ReturnStatus status = (NULL == req ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CallRequest_SetMethodToCall(req, 0, &g_conditionNode, methodId, 0, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(conn, req, (void**) &resp);
        req = NULL; // Memory managed by previous callee
    }
    if (SOPC_STATUS_OK == status && NULL != resp && 1 == resp->NoOfResults)
    {
        result = resp->Results[0].StatusCode;
    }
    if (NULL != req)
    {
        SOPC_ReturnStatus delStatus = SOPC_EncodeableObject_Delete(req->encodeableType, (void**) &req);
        SOPC_UNUSED_RESULT(delStatus);
    }
    if (NULL != resp)
    {
        SOPC_ReturnStatus delStatus = SOPC_EncodeableObject_Delete(resp->encodeableType, (void**) &resp);
        SOPC_UNUSED_RESULT(delStatus);
    }
    return result;
}

static bool check_call(SOPC_ClientConnection* conn,
                       const char* stepName,
                       const SOPC_NodeId* methodId,
                       SOPC_StatusCode expected)
{
    SOPC_StatusCode result = client_call_method(conn, methodId);
    printf("[methods] %s: result=0x%08" PRIX32 " (expected 0x%08" PRIX32 ")\n", stepName, result, expected);
    return expected == result;
}

static int32_t g_enabledCbCalls = 0;

/*
 * EnabledState callback called by the A&C looper. The first call blocks the looper
 * (see STATE_CB_BLOCK_MS). Callback parameters are not used after the wait since the
 * instance may have been deleted meanwhile.
 */
static void enabled_state_cb(uintptr_t userCbCtx,
                             SOPC_AlarmCondition* alarmCond,
                             const char* qnPath,
                             const SOPC_Variant* prevValue,
                             const SOPC_Variant* newValue)
{
    SOPC_UNUSED_ARG(userCbCtx);
    SOPC_UNUSED_ARG(alarmCond);
    SOPC_UNUSED_ARG(qnPath);
    SOPC_UNUSED_ARG(prevValue);
    SOPC_UNUSED_ARG(newValue);
    if (0 == SOPC_Atomic_Int_Add(&g_enabledCbCalls, 1)) // first call (returns the previous value)
    {
        SOPC_Sleep(STATE_CB_BLOCK_MS);
    }
}

static bool wait_enabled_cb_call(void)
{
    for (uint32_t elapsed = 0; elapsed < STATE_CB_WAIT_MS && 0 == SOPC_Atomic_Int_Get(&g_enabledCbCalls); elapsed += 10)
    {
        SOPC_Sleep(10);
    }
    return 0 != SOPC_Atomic_Int_Get(&g_enabledCbCalls);
}

/*
 * Method calls and Delete:
 * - a method call state change still queued in the A&C looper when the instance is deleted
 *   shall be dropped (the looper is blocked in the EnabledState callback meanwhile)
 * - instance and type method calls on a deleted instance shall fail with Bad_NodeIdInvalid
 * - instance methods shall be usable again after re-creation of the instance
 */
static SOPC_ReturnStatus run_method_call_after_delete_test(SOPC_ClientConnection* conn)
{
    SOPC_AlarmCondition* ac = NULL;
    SOPC_ReturnStatus status = create_condition(&ac);
    if (SOPC_STATUS_OK == status)
    {
        /* Wait for TBP to fill nodeIds and register instance methods (see AC_CREATE_INIT_MS). */
        SOPC_Sleep(AC_CREATE_INIT_MS);
        status = SOPC_AlarmCondition_SetEnabledStateCallback(ac, &enabled_state_cb, 0);
    }
    bool res = (SOPC_STATUS_OK == status);
    res = res && check_call(conn, "Disable", &g_disableInstMethodId, SOPC_GoodGenericStatus);
    // Looper is now blocked in the EnabledState callback of the Disable state change
    res = res && wait_enabled_cb_call();
    // Queued in the looper behind the blocked callback
    res = res && check_call(conn, "Enable (queued)", &g_enableInstMethodId, SOPC_GoodGenericStatus);
    if (NULL != ac)
    {
        // While enable call is queued delete the AC instance
        SOPC_ReturnStatus delStatus = SOPC_AlarmCondition_Delete(&ac);
        res = res && (SOPC_STATUS_OK == delStatus);
    }
    // Let the looper treat the queued Enable state change on the deleted instance
    SOPC_Sleep(STATE_CB_BLOCK_MS + LOOPER_DRAIN_MS);
    printf("[methods] EnabledState callback calls=%" PRId32 "\n", SOPC_Atomic_Int_Get(&g_enabledCbCalls));
    // Queued Enable state change dropped: no callback call for it
    res = res && (1 == SOPC_Atomic_Int_Get(&g_enabledCbCalls));

    res =
        res && check_call(conn, "Disable instance method after Delete", &g_disableInstMethodId, OpcUa_BadNodeIdInvalid);
    res = res && check_call(conn, "Disable type method after Delete", &g_disableTypeMethodId, OpcUa_BadNodeIdInvalid);

    if (res)
    {
        status = create_condition(&ac);
        res = (SOPC_STATUS_OK == status);
    }
    if (res)
    {
        SOPC_Sleep(AC_CREATE_INIT_MS);
        res = check_call(conn, "Disable after re-creation", &g_disableInstMethodId, SOPC_GoodGenericStatus);
    }
    if (NULL != ac)
    {
        SOPC_ReturnStatus delStatus = SOPC_AlarmCondition_Delete(&ac);
        res = res && (SOPC_STATUS_OK == delStatus);
    }
    SOPC_Sleep(LOOPER_DRAIN_MS);
    return res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

static SOPC_ReturnStatus run_client_tests(void)
{
    SOPC_SecureConnection_Config* scConfig = NULL;
    SOPC_ClientConnection* conn = NULL;
    SOPC_ReturnStatus status = client_configure(&scConfig);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(scConfig, &client_conn_event_cb, &conn);
        printf("[client] Connect status=%d\n", (int) status);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_method_call_after_delete_test(conn);
        SOPC_ReturnStatus discStatus = SOPC_ClientHelper_Disconnect(&conn);
        SOPC_UNUSED_RESULT(discStatus);
    }
    SOPC_ClientConfigHelper_Clear();
    return status;
}

static SOPC_ReturnStatus run_all_tests(void)
{
    SOPC_ReturnStatus status = run_delete_lifecycle_test();
    if (SOPC_STATUS_OK == status)
    {
        status = run_duplicate_create_test();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_create_delete_loop_test();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_trigger_right_after_create_test(false);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_trigger_right_after_create_test(true);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = run_client_tests();
    }
    return status;
}

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    SOPC_Log_Configuration logConfig = SOPC_Common_GetDefaultLogConfiguration();
    logConfig.logLevel = SOPC_LOG_LEVEL_WARNING;
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_alarm_condition_delete_logs/";

    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = load_config();
    }

    SOPC_MethodCallManager* mcm = NULL;
    if (SOPC_STATUS_OK == status)
    {
        mcm = SOPC_MethodCallManager_Create();
        status = (NULL != mcm) ? SOPC_ServerConfigHelper_SetMethodCallManager(mcm) : SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerAlarmConditionMgr_Initialize(mcm);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(&stop_cb);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Address space / event types ready before first CreateFromEvent. */
        SOPC_Sleep(SERVER_START_SETTLE_MS);
        status = run_all_tests();
        (void) SOPC_ServerHelper_StopServer();
        while (0 == SOPC_Atomic_Int_Get(&g_stopped))
        {
            SOPC_Sleep(50); /* wait for stop_cb after StopServer */
        }
    }
    else
    {
        fprintf(stderr, "Server setup failed status=%d\n", (int) status);
    }

    SOPC_ServerAlarmConditionMgr_Clear();
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    return (SOPC_STATUS_OK == status) ? 0 : 1;
}
