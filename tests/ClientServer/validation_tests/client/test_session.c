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

/** \fil
 * \brief Test the Toolkit API
 */

#include <check.h>
#include <stdbool.h>

#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"

#include "config.h"
#include "state_machine.h"
#include "test_suite_client.h"
#include "wait_machines.h"

static StateMachine_Machine* g_pSM = NULL;
static int32_t atomicValidatingResult = 0;

static void EventDispatcher_QuitAfterConnect(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

START_TEST(test_username_password)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_session_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert(SOPC_Toolkit_Initialize(EventDispatcher_QuitAfterConnect) == SOPC_STATUS_OK);
    g_pSM = StateMachine_Create();
    ck_assert(NULL != g_pSM);
    ck_assert(StateMachine_ConfigureMachine(g_pSM, true, false) == SOPC_STATUS_OK);

    ck_assert(StateMachine_StartSession_UsernamePassword(g_pSM, "username", "user", "password") == SOPC_STATUS_OK);
    wait_for_machine(&atomicValidatingResult, g_pSM);

    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);
}
END_TEST

void EventDispatcher_QuitAfterConnect(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;
    // Set result is still validating since machine state will change on next instruction
    SOPC_Atomic_Int_Set(&atomicValidatingResult, 1);
    ck_assert(StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx));

    switch (event)
    {
    case SE_ACTIVATED_SESSION:
        StateMachine_StopSession(g_pSM);
        break;
    case SE_CLOSED_SESSION:
        break;
    default:
        ck_assert_msg(false, "Unexpected event");
        break;
    }
    SOPC_Atomic_Int_Set(&atomicValidatingResult, 0);
}

Suite* client_suite_make_session(void)
{
    Suite* s = NULL;
    TCase* tc_user = NULL;

    s = suite_create("Client session");
    /* Anonymous is tested in toolkit_test_client for now */
    tc_user = tcase_create("Username password");

    suite_add_tcase(s, tc_user);
    tcase_add_test(tc_user, test_username_password);

    return s;
}
