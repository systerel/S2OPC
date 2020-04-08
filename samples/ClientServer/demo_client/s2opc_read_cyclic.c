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

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "util_variant.h"

#include "argparse.h"
#include "config.h"
#include "config_cyclic.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
static StateMachine_Machine* g_pSM = NULL;
/* So is the Attribute to write */
static uint32_t g_iAttr = 13;

/* Event handler of the Read */
static void EventDispatcher_Read(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static SOPC_ReturnStatus SendReadRequest(StateMachine_Machine* pSM);
static void PrintReadResponse(OpcUa_ReadResponse* pReadResp);

static const char* const usage[] = {
    "s2opc_read [options]",
    NULL,
};

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    struct argparse_option options[] = {OPT_HELP(),       OPT_GROUP("Read options"), CONN_OPTIONS[0],  CONN_OPTIONS[1],
                                        CONN_OPTIONS[2],  CONN_OPTIONS[3],           CONN_OPTIONS[4],  CONN_OPTIONS[5],
                                        CONN_OPTIONS[6],  CONN_OPTIONS[7],           CONN_OPTIONS[8],  CONN_OPTIONS[9],
                                        CONN_OPTIONS[10], CONN_OPTIONS[11],          CONN_OPTIONS[12], OPT_END()};

    struct argparse argparse;

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, "\nS2OPC periodic read demo: send Read Request cyclically and print received values.",
                      "\nLook the config_cyclic.c C file to configure the Node to Read.");
    argc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC periodic read demo.\n");

    if (SOPC_STATUS_OK != status)
    {
        argparse_usage(&argparse);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = conf_initialize_nodeid_array();
    }

    /* Initialize SOPC_Common */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_read_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_Common_Initialize(logConfiguration);
    }
    /* Init */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(EventDispatcher_Read);
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

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Configured();
        if (SOPC_STATUS_OK == status)
        {
            printf("# Info: Toolkit configuration done.\n");
            printf("# Info: Opening Session.\n");
        }
        else
        {
            printf("# Error: Toolkit configuration failed.\n");
        }
    }

    /* Secure Channel and Session creation */
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != USER_NAME)
        {
            status = StateMachine_StartSession_UsernamePassword(g_pSM, USER_POLICY_ID, USER_NAME, USER_PWD);
        }
        else
        {
            status = StateMachine_StartSession_Anonymous(g_pSM, ANONYMOUS_POLICY_ID);
        }
    }

    uint32_t tempo = CONF_SEND_MS_PERIOD;

    /* Active wait */
    while (SOPC_STATUS_OK == status && !StateMachine_IsIdle(g_pSM) && iWait * SLEEP_LENGTH <= CONF_LIFETIME)
    {
        iWait += 1;

        /* Send request */
        if (tempo - SLEEP_LENGTH > SLEEP_LENGTH)
        {
            tempo = tempo - SLEEP_LENGTH;
        }
        else if (stActivated == g_pSM->state)
        {
            SendReadRequest(g_pSM);
            tempo = CONF_SEND_MS_PERIOD;
        }

        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Finish it */
    for (uint32_t i = 0; i < CONF_NB_NODE_TO_WRITE; i++)
    {
        if (NULL != g_nodeIdArray[i])
        {
            SOPC_Free(g_nodeIdArray[i]);
        }
    }
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}

static void EventDispatcher_Read(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;

    if (StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx))
    {
        switch (event)
        {
        case SE_ACTIVATED_SESSION:
            break;
        case SE_RCV_SESSION_RESPONSE:
            /* Prints */
            /* It can be long, as the thread is joined by Toolkit_Clear(), it should not be interrupted. */
            PrintReadResponse((OpcUa_ReadResponse*) pParam);
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

static SOPC_ReturnStatus SendReadRequest(StateMachine_Machine* pSM)
{
    OpcUa_ReadRequest* pReq = NULL;
    OpcUa_ReadValueId* lrv = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pReq = SOPC_Malloc(sizeof(OpcUa_ReadRequest));
    lrv = SOPC_Calloc(CONF_NB_NODE_TO_WRITE, sizeof(OpcUa_ReadValueId));
    if (NULL == pReq || NULL == lrv)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Sending ReadRequest.\n");

        OpcUa_ReadRequest_Initialize(pReq);
        OpcUa_ReadValueId_Initialize(lrv);

        /* Fill the Request */
        pReq->TimestampsToReturn = OpcUa_TimestampsToReturn_Neither;
        pReq->NoOfNodesToRead = CONF_NB_NODE_TO_WRITE;
        pReq->NodesToRead = lrv;

        for (uint32_t i = 0; i < CONF_NB_NODE_TO_WRITE; i++)
        {
            OpcUa_ReadValueId_Initialize(&lrv[i]);
            status = SOPC_NodeId_Copy(&lrv[i].NodeId, g_nodeIdArray[i]);
            lrv[i].AttributeId = g_iAttr;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Send the Request */
        SOPC_ToolkitClient_AsyncSendRequestOnSession(pSM->iSessionID, pReq, 42);
    }

    if (SOPC_STATUS_NOK == status)
    {
        printf("# Error: Send request creation failed. Abort.\n");
        g_pSM->state = stError;
    }

    /* Free resources when message was not sent to the Toolkit */
    if (SOPC_STATUS_OK != status)
    {
        if (NULL != pReq)
        {
            OpcUa_ReadRequest_Clear(pReq);
            SOPC_Free(pReq);
        }
        if (NULL != lrv)
        {
            SOPC_Free(lrv);
        }
    }

    return status;
}

static void PrintReadResponse(OpcUa_ReadResponse* pResp)
{
    int32_t i = 0;
    SOPC_DataValue* pVal = NULL;
    char* sNid = NULL;

    if (SOPC_GoodGenericStatus != pResp->ResponseHeader.ServiceResult)
    {
        printf("# Error: Read failed with status code %i.\n", pResp->ResponseHeader.ServiceResult);
    }

    printf("Receives Read Response for attribute %i:\n", g_iAttr);
    for (i = 0; i < pResp->NoOfResults; ++i)
    {
        pVal = &pResp->Results[i];
        sNid = SOPC_NodeId_ToCString(g_nodeIdArray[i]);
        printf("Read Response StatusCode for \"%s\" is 0x%08X\n", sNid, pVal->Status);
        SOPC_Free(sNid);
        sNid = NULL;
        SOPC_Variant_Print(&(pVal->Value));
    }
}
