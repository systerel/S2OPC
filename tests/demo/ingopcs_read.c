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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "util_variant.h"

#include "config.h"
#include "ingopcs_browse.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
StateMachine_Machine* g_pSM = NULL;
/* The start NodeId is global, so that it is accessible to the Print function in the other thread. */
SOPC_NodeId* g_pNid = NULL;
/* So is the Attribute to read */
uint32_t g_iAttr = 0;

/* Event handler of the Read */
void EventDispatcher_Read(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t appCtx);

SOPC_ReturnStatus SendReadRequest(StateMachine_Machine* pSM);
void PrintReadResponse(OpcUa_ReadResponse* pReadResp);

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    printf("INGOPCS read demo.\n");
    /* Read the start node id from the command line */
    if (argc != 3)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status)
    {
        /* argv are always null-terminated */
        g_pNid = SOPC_NodeId_FromCString(argv[1], strlen(argv[1]));
        if (NULL == g_pNid)
        {
            printf("# Error: nodeid not recognized: \"%s\"\n", argv[1]);
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (sscanf(argv[2], "%" SCNu32, &g_iAttr) == 0 || g_iAttr < 1 || g_iAttr > 22)
        {
            printf("# Error: invalid attribute id: \"%s\"\n", argv[2]);
            printf("   Expecting an integer in the range 1..22\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Expects exactly 2 arguments:\n");
        printf("  - the node id XML formatted: [ns=<digits>;]<i, s, g or b>=<nodeid>,\n");
        printf("  - the AttributeId as an int:\n");
        printf("                     NodeId |  1\n");
        printf("                  NodeClass |  2\n");
        printf("                 BrowseName |  3\n");
        printf("                DisplayName |  4\n");
        printf("                Description |  5\n");
        printf("                  WriteMask |  6\n");
        printf("              UserWriteMask |  7\n");
        printf("                 IsAbstract |  8\n");
        printf("                  Symmetric |  9\n");
        printf("                InverseName | 10\n");
        printf("            ContainsNoLoops | 11\n");
        printf("              EventNotifier | 12\n");
        printf("                      Value | 13\n");
        printf("                   DataType | 14\n");
        printf("                  ValueRank | 15\n");
        printf("            ArrayDimensions | 16\n");
        printf("                AccessLevel | 17\n");
        printf("            UserAccessLevel | 18\n");
        printf("    MinimumSamplingInterval | 19\n");
        printf("                Historizing | 20\n");
        printf("                 Executable | 21\n");
        printf("             UserExecutable | 22\n");
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

    /* Active wait */
    while (SOPC_STATUS_OK == status && !StateMachine_IsIdle(g_pSM) && iWait * SLEEP_LENGTH <= SC_LIFETIME)
    {
        iWait += 1;
        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Finish it */
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return status;
}

void EventDispatcher_Read(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t appCtx)
{
    if (StateMachine_EventDispatcher(g_pSM, event, arg, pParam, appCtx))
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

SOPC_ReturnStatus SendReadRequest(StateMachine_Machine* pSM)
{
    OpcUa_ReadRequest* pReq = NULL;
    OpcUa_ReadValueId* lrv = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        pReq = malloc(sizeof(OpcUa_ReadRequest));
        lrv = malloc(1 * sizeof(OpcUa_ReadValueId));
        if (NULL == pReq || NULL == lrv)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Sending ReadRequest.\n");

        /* Fill the Request */
        pReq->encodeableType = &OpcUa_ReadRequest_EncodeableType;
        pReq->MaxAge = 0.;
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

    return status;
}

void PrintReadResponse(OpcUa_ReadResponse* pResp)
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
        free(sNid);
        sNid = NULL;

        pVal = &pResp->Results[i];
        printf("StatusCode: 0x%08X\n", pVal->Status);
        util_variant__print_SOPC_Variant(&(pVal->Value));
    }
}
