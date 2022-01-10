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
#include "sopc_helper_uri.h"
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
/* So is the Attribute to write */
static uint32_t g_iAttr = 13;
/* The builtin id defining type of value to write */
static SOPC_BuiltinId g_builtInId = SOPC_Null_Id;
/* And the value to write */
static SOPC_DataValue g_dv;

/* Event handler of the Write */
static void EventDispatcher_Write(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static SOPC_ReturnStatus SendWriteRequest(StateMachine_Machine* pSM);
static void PrintWriteResponse(OpcUa_WriteResponse* pReadResp);

static bool ParseValue(const char* val);

static const char* const usage[] = {
    "s2opc_write [options] <value>",
    NULL,
};

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    char* nid = NULL;
    int builtInId = 0;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Write options"),
        OPT_STRING('n', "node_id", &nid, "node id to read", NULL, 0, 0),
        OPT_INTEGER('t', "builtin_id", &builtInId, "OPC UA built in id type to write", NULL, 0, 0),
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
    argparse_describe(&argparse, "\nS2OPC write demo: write a node Value attribute with numerical type",
                      "\nExpects at least 2 arguments:"
                      "\n -n: the Node id XML formatted [ns=<digits>;]<i, s, g or b>=<nodeid>,"
                      "\n -t: the BuiltInType id: "
                      "\n       SOPC_Boolean_Id | 1"
                      "\n       SOPC_SByte_Id   | 2"
                      "\n       SOPC_Byte_Id    | 3"
                      "\n       SOPC_Int16_Id   | 4"
                      "\n       SOPC_UInt16_Id  | 5"
                      "\n       SOPC_Int32_Id   | 6"
                      "\n       SOPC_UInt32_Id  | 7"
                      "\n       SOPC_Int64_Id   | 8"
                      "\n       SOPC_UInt64_Id  | 9"
                      "\n       SOPC_Float_Id   | 10"
                      "\n       SOPC_Double_Id  | 11"
                      "\n       SOPC_String_Id  | 12"

                      "\n E.g.: ./s2opc_write -u user1 -p password -n \"ns=1;s=Byte_001\" -t 3 42");
    int restArgc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC write demo\n");

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
        if (builtInId >= SOPC_Boolean_Id && builtInId <= SOPC_String_Id)
        {
            g_builtInId = (SOPC_BuiltinId) builtInId;
        }
        else
        {
            printf("# Error: builtin id not recognized: \"%d\"\n", builtInId);
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (1 != restArgc)
        {
            if (0 == restArgc)
            {
                printf("# Error: no value provided\n");
            }
            else
            {
                printf("# Error: too many arguments provided for value: \"%d\"\n", restArgc);
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        if (!ParseValue(argv[argc - 1]))
        {
            status = SOPC_STATUS_NOK;
            printf("# Error: failed parsing value provided\n");
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        argparse_usage(&argparse);
    }

    /* Initialize SOPC Common */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_write_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_Common_Initialize(logConfiguration);
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

    /* Clear */
    if (NULL != g_pNid)
    {
        SOPC_NodeId_Clear(g_pNid);
        SOPC_Free(g_pNid);
    }
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}

static bool ParseValue(const char* val)
{
    SOPC_DataValue_Initialize(&g_dv);
    g_dv.Value.BuiltInTypeId = g_builtInId;
    g_dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    int scanRes = 0;
    /* Note: SCNu8 / SCNi8 not managed by mingw, use int and uint and then check max values */
    int i8 = 0;
    unsigned int u8 = 0;
    switch (g_builtInId)
    {
    case SOPC_Boolean_Id:
    case SOPC_Byte_Id:
        scanRes = sscanf(val, "%u", &u8);
        if (0 != scanRes && u8 <= UINT8_MAX)
        {
            if (SOPC_Byte_Id == g_builtInId)
            {
                g_dv.Value.Value.Byte = (SOPC_Byte) u8;
            }
            else
            {
                g_dv.Value.Value.Boolean = (SOPC_Boolean) u8;
            }
        }
        else
        {
            scanRes = 0;
        }
        break;
    case SOPC_SByte_Id:
        scanRes = sscanf(val, "%d", &i8);
        if (0 != scanRes && i8 <= INT8_MAX && i8 >= INT8_MIN)
        {
            g_dv.Value.Value.Sbyte = (SOPC_SByte) i8;
        }
        else
        {
            scanRes = 0;
        }
        break;
    case SOPC_Int16_Id:
        scanRes = sscanf(val, "%" SCNi16, &g_dv.Value.Value.Int16);
        break;
    case SOPC_UInt16_Id:
        scanRes = sscanf(val, "%" SCNu16, &g_dv.Value.Value.Uint16);
        break;
    case SOPC_Int32_Id:
        scanRes = sscanf(val, "%" SCNi32, &g_dv.Value.Value.Int32);
        break;
    case SOPC_UInt32_Id:
        scanRes = sscanf(val, "%" SCNu32, &g_dv.Value.Value.Uint32);
        break;
    case SOPC_Int64_Id:
        scanRes = sscanf(val, "%" SCNi64, &g_dv.Value.Value.Int64);
        break;
    case SOPC_UInt64_Id:
        scanRes = sscanf(val, "%" SCNu64, &g_dv.Value.Value.Uint64);
        break;
    case SOPC_Float_Id:
        scanRes = sscanf(val, "%f", &g_dv.Value.Value.Floatv);
        break;
    case SOPC_Double_Id:
        scanRes = sscanf(val, "%lf", &g_dv.Value.Value.Doublev);
        break;
    case SOPC_String_Id:
        SOPC_String_Initialize(&g_dv.Value.Value.String);
        if (SOPC_STATUS_OK == SOPC_String_CopyFromCString(&g_dv.Value.Value.String, val))
        {
            scanRes = true;
        }
        break;
    default:
        assert(false);
    }

    g_dv.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();

    return 0 != scanRes;
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

    pReq = SOPC_Malloc(sizeof(OpcUa_WriteRequest));
    lwv = SOPC_Malloc(1 * sizeof(OpcUa_WriteValue));
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
            SOPC_Free(pReq);
        }
        if (NULL != lwv)
        {
            SOPC_Free(lwv);
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
        SOPC_Free(sNid);
        sNid = NULL;

        printf("  StatusCode: 0x%08X\n", pResp->Results[i]);
    }
}
