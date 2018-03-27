/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_builtintypes.h"
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
void EventDispatcher_Browse(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

SOPC_ReturnStatus SendBrowseRequest(StateMachine_Machine* pSM);
void PrintBrowseResponse(OpcUa_BrowseResponse* pBwseResp);

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    printf("INGOPCS browse demo.\n");
    /* Read the start node id from the command line */
    if (argc != 2)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status)
    {
        /* argv are always null-terminated */
        g_pNid = SOPC_NodeId_FromCString(argv[1], strlen(argv[1]));
        if (NULL == g_pNid)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Expects exactly 1 argument, the node id as the XML format\n");
        printf("    [ns=<digits>;]<i, s, g or b>=<nodeid>\n");
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
        status = StateMachine_ConfigureMachine(g_pSM);
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
        status = StateMachine_StartSession(g_pSM);
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
        free(g_pNid);
    }
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return status;
}

void EventDispatcher_Browse(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
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

SOPC_ReturnStatus SendBrowseRequest(StateMachine_Machine* pSM)
{
    OpcUa_BrowseRequest* pReq = NULL;
    OpcUa_BrowseDescription* pDesc = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        pReq = malloc(sizeof(OpcUa_BrowseRequest));
        pDesc = malloc(1 * sizeof(OpcUa_BrowseDescription));
        if (NULL == pReq || NULL == pDesc)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Sending BrowseRequest.\n");

        /* Fill the Request */
        pReq->encodeableType = &OpcUa_BrowseRequest_EncodeableType;
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

    return status;
}

void PrintBrowseResponse(OpcUa_BrowseResponse* pResp)
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
                free(sNidB);
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

    free(sNidA);
}
