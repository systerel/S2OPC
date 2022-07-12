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

#include "config.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
static StateMachine_Machine* g_pSM = NULL;
/* The start NodeId is global, so that it is accessible to the Print function in the other thread. */
static SOPC_NodeId* g_pNid = NULL;

/* Event handler of the Browse */
static void EventDispatcher_Browse(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static SOPC_ReturnStatus SendBrowseRequest(StateMachine_Machine* pSM);
static void PrintBrowseResponse(OpcUa_BrowseResponse* pBwseResp);

static const char* const usage[] = {
    "s2opc_browse [options]",
    NULL,
};

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    char* nid = NULL;

    struct argparse_option options[] = {OPT_HELP(),
                                        OPT_GROUP("Browse options"),
                                        OPT_STRING('n', "node_id", &nid, "node id to browse", NULL, 0, 0),
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
    argparse_describe(&argparse, "\nS2OPC browse demo: browse a node in address space",
                      "\nExpects at least 1 argument:"
                      "\n -n: the Node id XML formatted [ns=<digits>;]<i, s, g or b>=<nodeid>,"
                      "\n E.g.: ./s2opc_browse -n i=85");
    argc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC browse demo.\n");

    if (NULL != nid)
    {
        g_pNid = SOPC_NodeId_FromCString(nid, (int32_t) strlen(nid));
    }
    if (NULL == g_pNid)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK != status)
    {
        argparse_usage(&argparse);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_browse_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_Common_Initialize(logConfiguration);
    }
    /* Init */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(EventDispatcher_Browse);
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
        if (NULL != USER_NAME)
        {
            status = StateMachine_StartSession_UsernamePassword(g_pSM, USER_POLICY_ID, USER_NAME, USER_PWD);
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

static void EventDispatcher_Browse(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;

    if (StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx))
    {
        switch (event)
        {
        case SE_ACTIVATED_SESSION:
            /* Send message */
            SendBrowseRequest(g_pSM);
            break;
        case SE_RCV_SESSION_RESPONSE:
            /* Prints */
            /* It can be long, as the thread is joined by Toolkit_Clear(), it should not be interrupted. */
            PrintBrowseResponse((OpcUa_BrowseResponse*) pParam);
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

static SOPC_ReturnStatus SendBrowseRequest(StateMachine_Machine* pSM)
{
    OpcUa_BrowseRequest* pReq = NULL;
    OpcUa_BrowseDescription* pDesc = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pReq = SOPC_Malloc(sizeof(OpcUa_BrowseRequest));
    pDesc = SOPC_Malloc(1 * sizeof(OpcUa_BrowseDescription));
    if (NULL == pReq || NULL == pDesc)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Sending BrowseRequest.\n");

        OpcUa_BrowseRequest_Initialize(pReq);
        OpcUa_BrowseDescription_Initialize(pDesc);

        /* Fill the Request */
        pReq->RequestedMaxReferencesPerNode = 100;
        pReq->NoOfNodesToBrowse = 1;
        pReq->NodesToBrowse = pDesc;
        status = SOPC_NodeId_Copy(&pDesc[0].NodeId, g_pNid);
        pDesc[0].BrowseDirection = OpcUa_BrowseDirection_Both;
        /* pDesc[0].ReferenceTypeId */
        /* pDesc[0].IncludeSubtypes */
        pDesc[0].NodeClassMask = 255; /* First 8 bits set -> all classes */
        pDesc[0].ResultMask = OpcUa_BrowseResultMask_All;
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
            OpcUa_BrowseRequest_Clear(pReq);
            SOPC_Free(pReq);
        }
        if (NULL != pDesc)
        {
            SOPC_Free(pDesc);
        }
    }

    return status;
}

static void PrintBrowseResponse(OpcUa_BrowseResponse* pResp)
{
    int32_t i = 0;
    int32_t j = 0;
    OpcUa_BrowseResult* pBwse = NULL;
    OpcUa_ReferenceDescription* pRefe = NULL;
    char* sNidA = NULL;
    char* sNidB = NULL;

    sNidA = SOPC_NodeId_ToCString(g_pNid);

    if (SOPC_GoodGenericStatus != pResp->ResponseHeader.ServiceResult)
    {
        printf("# Error: Browse failed with status code %i.\n", pResp->ResponseHeader.ServiceResult);
    }
    else
    {
        printf("Browsed nodes:\n");
    }

    for (i = 0; i < pResp->NoOfResults; ++i)
    {
        pBwse = &pResp->Results[i];
        if (SOPC_GoodGenericStatus != pBwse->StatusCode)
        {
            printf("# Error: Browse result %i has status code %i.\n", i, pBwse->StatusCode);
        }
        else
        {
            for (j = 0; j < pBwse->NoOfReferences; ++j)
            {
                pRefe = &pBwse->References[j];
                printf("- %s", sNidA);
                if (pRefe->IsForward)
                {
                    printf(" -> ");
                }
                else
                {
                    printf(" <- ");
                }
                sNidB = SOPC_NodeId_ToCString(&pRefe->NodeId.NodeId);
                printf("%s \"%s\"\n", sNidB, SOPC_String_GetRawCString(&pRefe->BrowseName.Name));
                SOPC_Free(sNidB);
            }

            if (pBwse->ContinuationPoint.Length <= 0)
            {
                printf("# Info: Continuation point null, no more References.\n");
            }
            else
            {
                printf("# Info: Continuation point non null, more References could have been fetched.\n");
            }
        }
    }

    SOPC_Free(sNidA);
}
