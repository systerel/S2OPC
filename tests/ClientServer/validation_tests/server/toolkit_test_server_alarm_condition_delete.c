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
#include <stdio.h>
#include <stdlib.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_alarm_conditions.h"
#include "opcua_identifiers.h"
#include "sopc_atomic.h"
#include "sopc_enums.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_threads.h"

/* EqtNoConf (EventNotifier) and AlarmNoConf from s2opc_demo_alarms_origin.xml */
static const SOPC_NodeId g_notifierNode = SOPC_NODEID_NUMERIC(1, 5008);
static const SOPC_NodeId g_conditionNode = SOPC_NODEID_NUMERIC(1, 5009);
static const SOPC_NodeId g_alarmConditionTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_AlarmConditionType);

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
 *   deadlock makes the test hang until the CTest timeout.
 */
static const uint32_t NB_CREATE_DELETE_ITERATIONS = 100;
static const uint32_t NB_CREATE_TRIGGER_ITERATIONS = 20;
static const uint32_t NB_TRIGGER_PER_CREATE = 50;

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

static SOPC_Event* make_condition_event(void)
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
        status = SOPC_Event_SetVariableFromStrPath(event, "0:Message", &message);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Event_Delete(&event);
        return NULL;
    }
    return event;
}

static SOPC_ReturnStatus create_condition(SOPC_AlarmCondition** outAc)
{
    SOPC_Event* event = make_condition_event();
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

/* TriggerEvent in a tight loop right after CreateFromEvent (see NB_CREATE_TRIGGER_ITERATIONS) */
static SOPC_ReturnStatus run_trigger_right_after_create_test(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < NB_CREATE_TRIGGER_ITERATIONS; i++)
    {
        SOPC_AlarmCondition* ac = NULL;
        status = create_condition(&ac);
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
            printf("[create/trigger] iteration %" PRIu32 " failed with status=%d\n", i, (int) status);
        }
    }
    printf("[create/trigger] %" PRIu32 " iterations status=%d\n", NB_CREATE_TRIGGER_ITERATIONS, (int) status);
    SOPC_Sleep(LOOPER_DRAIN_MS);
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
        status = run_trigger_right_after_create_test();
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
