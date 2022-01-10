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

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_crypto_provider.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
static StateMachine_Machine* g_pSM = NULL;

/* Event handler of the Discovery */
static void EventDispatcher_Discovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static void PrintServers(OpcUa_FindServersResponse* pResp);

static const char* const usage[] = {
    "s2opc_findserver [options]",
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
    argparse_describe(&argparse, "\nS2OPC find server: send a find server request to server", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC find server demo.\n");

    /* Initialize SOPC_Common */
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_findserver_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    status = SOPC_Common_Initialize(logConfiguration);
    /* Init */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(EventDispatcher_Discovery);
        g_pSM = StateMachine_Create();
    }

    if (SOPC_STATUS_OK == status && NULL == g_pSM)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Configuration, which include Secure Channel configuration. */
    if (SOPC_STATUS_OK == status)
    {
        status = StateMachine_ConfigureMachine(g_pSM, !NONE, ENCRYPT);
    }

    /* Starts Discovery */
    if (SOPC_STATUS_OK == status)
    {
        status = StateMachine_StartFindServers(g_pSM);
    }

    /* Active wait */
    while (SOPC_STATUS_OK == status && !StateMachine_IsIdle(g_pSM) && iWait * SLEEP_LENGTH <= SC_LIFETIME)
    {
        iWait += 1;
        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Finish it */
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}

static void EventDispatcher_Discovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;

    if (StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx))
    {
        switch (event)
        {
        case SE_RCV_DISCOVERY_RESPONSE:
            PrintServers((OpcUa_FindServersResponse*) pParam);
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
}

static void PrintServers(OpcUa_FindServersResponse* pResp)
{
    int32_t i = 0;
    int32_t j = 0;
    OpcUa_ApplicationDescription* pAppDesc = NULL;

    if (SOPC_GoodGenericStatus != pResp->ResponseHeader.ServiceResult)
    {
        printf("# Error: FindServers failed with status code %i.\n", pResp->ResponseHeader.ServiceResult);
    }
    else
    {
        printf("%" PRIi32 " servers through endpoint %s:\n", pResp->NoOfServers, ENDPOINT_URL);
    }

    for (i = 0; i < pResp->NoOfServers; ++i)
    {
        pAppDesc = &pResp->Servers[i];
        printf("- Server\n");
        printf("  ApplicationUri: %s\n", SOPC_String_GetRawCString(&pAppDesc->ApplicationUri));
        printf("  ProductUri: %s\n", SOPC_String_GetRawCString(&pAppDesc->ProductUri));
        printf("  ApplicationName: %s\n", SOPC_String_GetRawCString(&pAppDesc->ApplicationName.defaultText));
        printf("  ApplicationType: ");
        switch (pAppDesc->ApplicationType)
        {
        case OpcUa_ApplicationType_Server:
            printf("Server");
            break;
        case OpcUa_ApplicationType_Client:
            printf("Client");
            break;
        case OpcUa_ApplicationType_ClientAndServer:
            printf("ClientAndServer");
            break;
        case OpcUa_ApplicationType_DiscoveryServer:
            printf("DiscoveryServer");
            break;
        default:
            printf("<err>");
            break;
        }
        printf("\n  GatewayServerUri: %s\n", SOPC_String_GetRawCString(&pAppDesc->GatewayServerUri));
        printf("  DiscoveryProfileUri: %s\n", SOPC_String_GetRawCString(&pAppDesc->DiscoveryProfileUri));
        printf("  DiscoveryUrls\n");
        for (j = 0; j < pAppDesc->NoOfDiscoveryUrls; ++j)
        {
            printf("  -  %s\n", SOPC_String_GetRawCString(&pAppDesc->DiscoveryUrls[j]));
        }
    }
}
