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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "state_machine.h"

static StateMachine_Machine* g_pSM = NULL;

static int32_t analysingResult = 0;
static int32_t validResult = 0;

/* Event handler of the Discovery */
static void EventDispatcher_Register(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static void PrintRegisterResponse(OpcUa_RegisterServerResponse* pResp);

static const char* const usage[] = {
    "s2opc_register [options]",
    NULL,
};

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    struct argparse_option options[] = {OPT_HELP(),      CONN_OPTIONS[0], CONN_OPTIONS[1], CONN_OPTIONS[2],
                                        CONN_OPTIONS[3], CONN_OPTIONS[4], CONN_OPTIONS[5], CONN_OPTIONS[6],
                                        CONN_OPTIONS[7], CONN_OPTIONS[8], CONN_OPTIONS[9], CONN_OPTIONS[10],
                                        OPT_END()};
    struct argparse argparse;

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, "S2OPC register demo: send a register server request to server", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC register server demo.\n");
    /* Initialize SOPC Common */
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_register_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    status = SOPC_Common_Initialize(logConfiguration);
    /* Init */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(EventDispatcher_Register);
        g_pSM = StateMachine_Create();
    }

    if (SOPC_STATUS_OK != status || NULL == g_pSM)
    {
        SOPC_Toolkit_Clear();
        if (NULL != g_pSM)
        {
            StateMachine_Delete(&g_pSM);
        }
        return 1;
    }

    /* Configuration, which include Secure Channel configuration. */
    if (SOPC_STATUS_OK == status)
    {
        status = StateMachine_ConfigureMachine(g_pSM, !NONE, ENCRYPT);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Sending RegisterServerRequest.\n");
        StateMachine_StartRegisterServer(g_pSM);

        /* Active wait */
        while (SOPC_STATUS_OK == status &&
               (!StateMachine_IsIdle(g_pSM) || SOPC_Atomic_Int_Get(&analysingResult) == 1) &&
               iWait * SLEEP_LENGTH <= SC_LIFETIME)
        {
            iWait += 1;
            SOPC_Sleep(SLEEP_LENGTH);
        }

        if (1 != SOPC_Atomic_Int_Get(&validResult))
        {
            // Test failed
            status = SOPC_STATUS_NOK;
        }
    }

    /* Finish it */
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}

static void EventDispatcher_Register(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;

    SOPC_Atomic_Int_Set(&analysingResult, 1);
    if (StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx))
    {
        switch (event)
        {
        case SE_RCV_DISCOVERY_RESPONSE:
            PrintRegisterResponse((OpcUa_RegisterServerResponse*) pParam);
            break;
        default:
            break;
        }
    }
    else
    {
        printf("# Error: Received event %i not processed by the machine.\n", event);
        g_pSM->state = stError;
    }
    SOPC_Atomic_Int_Set(&analysingResult, 0);
}

static void PrintRegisterResponse(OpcUa_RegisterServerResponse* pResp)
{
    printf("# Info: Register response received.\n");
    if ((pResp->ResponseHeader.ServiceResult & SOPC_GoodStatusOppositeMask) == 0)
    {
        SOPC_Atomic_Int_Set(&validResult, 1);
    }
}
