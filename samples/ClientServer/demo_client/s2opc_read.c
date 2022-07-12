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
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
static StateMachine_Machine* g_pSM = NULL;
/* The start NodeId is global, so that it is accessible to the Print function in the other thread. */
static SOPC_NodeId* g_pNid = NULL;
/* So is the Attribute to read */
static uint32_t g_iAttr = 0;

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

    char* nid = NULL;
    int attr = 0;

    struct argparse_option options[] = {OPT_HELP(),
                                        OPT_GROUP("Read options"),
                                        OPT_STRING('n', "node_id", &nid, "node id to read", NULL, 0, 0),
                                        OPT_INTEGER('a', "attribute_id", &attr, "attribute id to read", NULL, 0, 0),
                                        CONN_OPTIONS[0],
                                        CONN_OPTIONS[1],
                                        CONN_OPTIONS[2],
                                        CONN_OPTIONS[3],
                                        CONN_OPTIONS[4],
                                        CONN_OPTIONS[5],
                                        CONN_OPTIONS[6],
                                        CONN_OPTIONS[7],
                                        CONN_OPTIONS[8],
                                        CONN_OPTIONS[9],
                                        CONN_OPTIONS[10],
                                        CONN_OPTIONS[11],
                                        CONN_OPTIONS[12],
                                        CONN_OPTIONS[13],
                                        CONN_OPTIONS[14],
                                        OPT_END()};

    struct argparse argparse;

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, "\nS2OPC read demo: read a node attribute",
                      "\nExpects at least 2 arguments:"
                      "\n -n: the Node id XML formatted [ns=<digits>;]<i, s, g or b>=<nodeid>,"
                      "\n -a: the Attribute id as an int: "
                      "\n                     NodeId |  1"
                      "\n                  NodeClass |  2"
                      "\n                 BrowseName |  3"
                      "\n                DisplayName |  4"
                      "\n                Description |  5"
                      "\n                  WriteMask |  6"
                      "\n              UserWriteMask |  7"
                      "\n                 IsAbstract |  8"
                      "\n                  Symmetric |  9"
                      "\n                InverseName | 10"
                      "\n            ContainsNoLoops | 11"
                      "\n              EventNotifier | 12"
                      "\n                      Value | 13"
                      "\n                   DataType | 14"
                      "\n                  ValueRank | 15"
                      "\n            ArrayDimensions | 16"
                      "\n                AccessLevel | 17"
                      "\n            UserAccessLevel | 18"
                      "\n    MinimumSamplingInterval | 19"
                      "\n                Historizing | 20"
                      "\n                 Executable | 21"
                      "\n             UserExecutable | 22"
                      "\n E.g.: ./s2opc_read -n i=2259 -a 13");
    argc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC read demo.\n");

    if (NULL != nid)
    {
        g_pNid = SOPC_NodeId_FromCString(nid, (int32_t) strlen(nid));
    }
    if (NULL == g_pNid)
    {
        printf("# Error: nodeid not recognized: \"%s\"\n", nid);
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (attr < 1 || attr > 22)
        {
            printf("# Error: invalid attribute id: \"%d\"\n", attr);
            printf("   Expecting an integer in the range 1..22\n");
            status = SOPC_STATUS_NOK;
        }
        else
        {
            g_iAttr = (uint32_t) attr;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        argparse_usage(&argparse);
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

    /* Secure Channel and Session creation */
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != USER_NAME || NULL != USER_PWD)
        {
            if (NULL == USER_NAME || NULL == USER_PWD)
            {
                printf("# Error: username AND password must be either both or none set\n");
                status = SOPC_STATUS_NOK;
            }
            else
            {
                status = StateMachine_StartSession_UsernamePassword(g_pSM, USER_POLICY_ID, USER_NAME, USER_PWD);
            }
        }
        else
        {
            status = StateMachine_StartSession_Anonymous(g_pSM, ANONYMOUS_POLICY_ID);
        }
    }

    /* Active wait */
    while (SOPC_STATUS_OK == status && !StateMachine_IsIdle(g_pSM) && iWait * SLEEP_LENGTH <= SC_LIFETIME)
    {
        iWait += 1;
        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Finish it */
    if (NULL != g_pNid)
    {
        SOPC_NodeId_Clear(g_pNid);
        SOPC_Free(g_pNid);
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
            /* Send message */
            SendReadRequest(g_pSM);
            break;
        case SE_RCV_SESSION_RESPONSE:
            /* Prints */
            /* It can be long, as the thread is joined by Toolkit_Clear(), it should not be interrupted. */
            PrintReadResponse((OpcUa_ReadResponse*) pParam);
            StateMachine_StopSession(g_pSM);
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
    lrv = SOPC_Malloc(1 * sizeof(OpcUa_ReadValueId));
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
        pReq->NoOfNodesToRead = 1;
        pReq->NodesToRead = lrv;
        status = SOPC_NodeId_Copy(&lrv[0].NodeId, g_pNid);
        lrv[0].AttributeId = g_iAttr;
        /* lrv[0].IndexRange */
        /* lrv[0].DataEncoding */
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

    /* There should always be 1 result here... */
    for (i = 0; i < pResp->NoOfResults; ++i)
    {
        sNid = SOPC_NodeId_ToCString(g_pNid);
        printf("Read node \"%s\", attribute %i:\n", sNid, g_iAttr);
        SOPC_Free(sNid);
        sNid = NULL;

        pVal = &pResp->Results[i];
        printf("StatusCode: 0x%08X\n", pVal->Status);
        SOPC_Variant_Print(&(pVal->Value));
    }
}
