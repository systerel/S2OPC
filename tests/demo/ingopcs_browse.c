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

#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "ingopcs_browse.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
StateMachine_Machine* g_pSM = NULL;

/* Event handler of the Browse */
void EventDispatcher_Browse(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t appCtx);

SOPC_ReturnStatus SendBrowseRequest(StateMachine_Machine* pSM);
void PrintBrowseResponse(OpcUa_BrowseResponse* pBwseResp);

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    /* Init */
    printf("INGOPCS browse demo.\n");
    status = SOPC_Toolkit_Initialize(EventDispatcher_Browse);
    g_pSM = StateMachine_Create();
    if (SOPC_STATUS_OK == status && NULL == g_pSM)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Configuration, which include Secure Channel configuration. */
    if (SOPC_STATUS_OK == status)
    {
        status = StateMachine_ConfigureToolkit(g_pSM);
    }

    /* Secure Channel and Session creation */

    /* Active wait */
    while (SOPC_STATUS_OK == status && !StateMachine_IsOver(g_pSM) && iWait * SLEEP_LENGTH <= SC_LIFETIME)
    {
        iWait += 1;
        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Finish it */
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return 0;
}

void EventDispatcher_Browse(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t appCtx)
{
    StateMachine_EventDispatcher(g_pSM, event, arg, pParam, appCtx);

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
        break;
    }
}

SOPC_ReturnStatus SendBrowseRequest(StateMachine_Machine* pSM)
{
    OpcUa_BrowseRequest* pReq = NULL;
    OpcUa_BrowseDescription* pDesc = NULL;

    if (NULL == pSM)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pReq = malloc(sizeof(OpcUa_BrowseRequest));
    pDesc = malloc(1 * sizeof(OpcUa_BrowseDescription));
    if (NULL == pReq || NULL == pDesc)
        return SOPC_STATUS_NOK;

    printf("# Info: Sending BrowseRequest.\n");

    /* Fill the Request */
    pReq->encodeableType = &OpcUa_BrowseRequest_EncodeableType;
    pReq->RequestedMaxReferencesPerNode = 100;
    pReq->NoOfNodesToBrowse = 1;
    pReq->NodesToBrowse = pDesc;
    pDesc[0].NodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    pDesc[0].NodeId.Namespace = 0;
    pDesc[0].NodeId.Data.Numeric = 84;
    pDesc[0].BrowseDirection = OpcUa_BrowseDirection_Both;
    /* pDesc[0].ReferenceTypeId */
    /* pDesc[0].IncludeSubtypes */
    pDesc[0].NodeClassMask = 255; /* First 8 bits set -> all classes */
    pDesc[0].ResultMask = OpcUa_BrowseResultMask_All;

    /* Send the Request */
    SOPC_ToolkitClient_AsyncSendRequestOnSession(pSM->iSessionID, pReq, 42);

    return SOPC_STATUS_OK;
}

void PrintBrowseResponse(OpcUa_BrowseResponse* pResp)
{
    int32_t i = 0;
    int32_t j = 0;
    OpcUa_BrowseResult* pBwse = NULL;
    OpcUa_ReferenceDescription* pRefe = NULL;

    printf("Browsed nodes:\n");
    for (i = 0; i < pResp->NoOfResults; ++i)
    {
        pBwse = &pResp->Results[i];
        if (SOPC_GoodGenericStatus != pBwse->StatusCode)
        {
            printf("# Error: Browse result %i has status code %i\n", i, pBwse->StatusCode);
        }
        else
        {
            for (j = 0; j < pBwse->NoOfReferences; ++j)
            {
                pRefe = &pBwse->References[j];
                printf("- ");
                printf("Start Node");
                if (pRefe->IsForward)
                {
                    printf(" -> ");
                }
                else
                {
                    printf(" <- ");
                }
                printf("End Node, \"%s\"\n", SOPC_String_GetRawCString(&pRefe->BrowseName.Name));
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
}
