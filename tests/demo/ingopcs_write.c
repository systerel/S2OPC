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
/* So is the Attribute to write */
static uint32_t g_iAttr = 13;
/* And the value to write */
static SOPC_DataValue g_dv;

/* Event handler of the Write */
static void EventDispatcher_Write(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static SOPC_ReturnStatus SendWriteRequest(StateMachine_Machine* pSM);
static void PrintWriteResponse(OpcUa_WriteResponse* pReadResp);

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;
    double dVal = 0.;
    int64_t iVal = 0;

    printf("INGOPCS write demo (only the Value attribute).\n");
    /* Read the start node id from the command line */
    if (argc != 4)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_STATUS_OK == status)
    {
        assert(strlen(argv[1]) <= INT32_MAX);

        /* argv are always null-terminated */
        g_pNid = SOPC_NodeId_FromCString(argv[1], (int32_t) strlen(argv[1]));
        if (NULL == g_pNid)
        {
            printf("# Error: nodeid not recognized: \"%s\"\n", argv[1]);
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (strstr(argv[2], "-d") == NULL && strstr(argv[2], "-i") == NULL)
        {
            printf("# Error: type qualifier not recognized: \"%s\"\n", argv[2]);
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_DataValue_Initialize(&g_dv);
        if (strstr(argv[2], "-d") != NULL)
        {
            if (sscanf(argv[3], "%lf", &dVal) == 0)
            {
                printf("# Error: failed to read a double \"%s\"\n", argv[3]);
                status = SOPC_STATUS_NOK;
            }
            if (SOPC_STATUS_OK == status)
            {
                g_dv.Value.BuiltInTypeId = SOPC_Double_Id;
                g_dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
                g_dv.Value.Value.Doublev = dVal;
            }
        }
        else if (strstr(argv[2], "-i") != NULL)
        {
            if (sscanf(argv[3], "%" SCNd64, &iVal) == 0)
            {
                printf("# Error: failed to read an integer \"%s\"\n", argv[3]);
                status = SOPC_STATUS_NOK;
            }
            if (SOPC_STATUS_OK == status)
            {
                g_dv.Value.BuiltInTypeId = SOPC_Int64_Id;
                g_dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
                g_dv.Value.Value.Int64 = iVal;
            }
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Expects exactly 3 arguments:\n");
        printf("  - the node id XML formatted: [ns=<digits>;]<i, s, g or b>=<nodeid>,\n");
        printf("  - the type \"-d\" for a floating point value or \"-i\" for signed integer,\n");
        printf("  - the value.\n");
    }

    /* Init */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(EventDispatcher_Write);
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

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}

static void EventDispatcher_Write(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;

    if (StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx))
    {
        switch (event)
        {
        case SE_ACTIVATED_SESSION:
            /* Send message */
            SendWriteRequest(g_pSM);
            break;
        case SE_RCV_SESSION_RESPONSE:
            /* Prints */
            /* It can be long, as the thread is joined by Toolkit_Clear(), it should not be interrupted. */
            PrintWriteResponse((OpcUa_WriteResponse*) pParam);
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

static SOPC_ReturnStatus SendWriteRequest(StateMachine_Machine* pSM)
{
    OpcUa_WriteRequest* pReq = NULL;
    OpcUa_WriteValue* lwv = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pReq = malloc(sizeof(OpcUa_WriteRequest));
    lwv = malloc(1 * sizeof(OpcUa_WriteValue));
    if (NULL == pReq || NULL == lwv)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: Sending WriteRequest.\n");

        OpcUa_WriteRequest_Initialize(pReq);
        OpcUa_WriteValue_Initialize(lwv);

        /* Fill the Request */
        pReq->NoOfNodesToWrite = 1;
        pReq->NodesToWrite = lwv;
        status = SOPC_NodeId_Copy(&lwv[0].NodeId, g_pNid);
        lwv[0].AttributeId = g_iAttr;
        /* lwv[0].IndexRange */
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_DataValue_Copy(&lwv[0].Value, &g_dv);
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
            free(pReq);
        }
        if (NULL != lwv)
        {
            free(lwv);
        }
    }

    return status;
}

static void PrintWriteResponse(OpcUa_WriteResponse* pResp)
{
    int32_t i = 0;
    char* sNid = NULL;

    if (SOPC_GoodGenericStatus != pResp->ResponseHeader.ServiceResult)
    {
        printf("# Error: Write failed with status code %i.\n", pResp->ResponseHeader.ServiceResult);
    }

    /* There should always be 1 result here... */
    for (i = 0; i < pResp->NoOfResults; ++i)
    {
        sNid = SOPC_NodeId_ToCString(g_pNid);
        printf("Write node \"%s\", attribute %i:\n", sNid, g_iAttr);
        free(sNid);
        sNid = NULL;

        printf("  StatusCode: 0x%08X\n", pResp->Results[i]);
    }
}
