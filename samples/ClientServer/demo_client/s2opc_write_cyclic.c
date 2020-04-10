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

#include "argparse.h"
#include "config.h"
#include "state_machine.h"

/* Configuration :
 * The configuration is composed of 3 elements :
 * - Lifetime of the execution
 * - Period to send write request
 * - Number of elements to write
 * - Array of Builtin Type Id of the elements to write
 * - Array of Node Id of the elements to write
 */

// time in MS
#define CONF_LIFETIME 1000000

// time in MS. 1s
#define CONF_SEND_MS_PERIOD 1000

#define CONF_NB_NODE_TO_WRITE 8

// Data Values Type of the Node to write
static SOPC_BuiltinId CONF_DV_TYPE_ARRAY[CONF_NB_NODE_TO_WRITE] = {SOPC_SByte_Id,  SOPC_Byte_Id,  SOPC_Int16_Id,
                                                                   SOPC_UInt16_Id, SOPC_Int32_Id, SOPC_UInt32_Id,
                                                                   SOPC_Int64_Id,  SOPC_UInt64_Id};

// Node ID the Node to write.
static char* CONF_NODE_ID_ARRAY[CONF_NB_NODE_TO_WRITE] = {
    "ns=1;i=1007", // SByte
    "ns=1;i=1008", // Byte
    "ns=1;i=1009", // Int16
    "ns=1;i=1010", // UInt16
    "ns=1;i=1011", // Int32
    "ns=1;i=1002", // UInt32
    "ns=1;i=1001", // Int64
    "ns=1;i=1012"  // UInt64
};

/* The Attribute to write */
static uint32_t g_iAttr = 13;

/* Commom Variables
   Uses by Read and Write.
   Do not touch
 */

/* The start NodeId is global, so that it is accessible to the Print function in the other thread. */
SOPC_NodeId* g_nodeIdArray[CONF_NB_NODE_TO_WRITE];

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
static StateMachine_Machine* g_pSM = NULL;

/* And the value to write */
static SOPC_DataValue g_dvArray[CONF_NB_NODE_TO_WRITE];

/* Event handler of the Write */
static void EventDispatcher_Write(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static SOPC_ReturnStatus SendWriteRequest(StateMachine_Machine* pSM);
static void PrintWriteResponse(OpcUa_WriteResponse* pReadResp);

static const char* const usage[] = {
    "s2opc_write_cyclic [options] <value>",
    NULL,
};

static SOPC_ReturnStatus conf_initialize_nodeid_array(void)
{
    memset(g_nodeIdArray, 0, CONF_NB_NODE_TO_WRITE * sizeof(SOPC_NodeId*));
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    for (uint32_t i = 0; i < CONF_NB_NODE_TO_WRITE; i++)
    {
        assert(strlen(CONF_NODE_ID_ARRAY[i]) <= INT32_MAX);

        g_nodeIdArray[i] = SOPC_NodeId_FromCString(CONF_NODE_ID_ARRAY[i], (int32_t) strlen(CONF_NODE_ID_ARRAY[i]));
        if (NULL == g_nodeIdArray[i])
        {
            printf("# Error: nodeid %d not recognized: \"%s\"\n", i, CONF_NODE_ID_ARRAY[i]);
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    struct argparse_option options[] = {OPT_HELP(),       OPT_GROUP("Write options"),
                                        CONN_OPTIONS[0],  CONN_OPTIONS[1],
                                        CONN_OPTIONS[2],  CONN_OPTIONS[3],
                                        CONN_OPTIONS[4],  CONN_OPTIONS[5],
                                        CONN_OPTIONS[6],  CONN_OPTIONS[7],
                                        CONN_OPTIONS[8],  CONN_OPTIONS[9],
                                        CONN_OPTIONS[10], CONN_OPTIONS[11],
                                        CONN_OPTIONS[12], OPT_END()};

    struct argparse argparse;

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse,
                      "\nS2OPC periodic write demo: write configured integer variables periodically (increment)",
                      "\nSee the s2opc_write_cyclic.c file to configure the Variable nodes to write."
                      "\n E.g.: ./s2opc_write_cyclic -u user1 -p password");
    int restArgc = argparse_parse(&argparse, argc, argv);

    if (0 != restArgc)
    {
        argparse_usage(&argparse);
    }

    printf("S2OPC periodic write demo\n");

    if (SOPC_STATUS_OK == status)
    {
        status = conf_initialize_nodeid_array();
    }

    if (SOPC_STATUS_OK == status)
    {
        for (uint32_t i = 0; i < CONF_NB_NODE_TO_WRITE; i++)
        {
            SOPC_DataValue_Initialize(&g_dvArray[i]);
            g_dvArray[i].Value.BuiltInTypeId = CONF_DV_TYPE_ARRAY[i];
            g_dvArray[i].Value.ArrayType = SOPC_VariantArrayType_SingleValue;
            g_dvArray[i].Value.Value.Uint64 = 0;
        }
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

    /* Active wait */
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
            SendWriteRequest(g_pSM);
            tempo = CONF_SEND_MS_PERIOD;
        }

        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Clear */
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

static void EventDispatcher_Write(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
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
            PrintWriteResponse((OpcUa_WriteResponse*) pParam);
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

static SOPC_ReturnStatus DataValue_Increase(SOPC_Variant* variant)
{
    switch (variant->BuiltInTypeId)
    {
    case SOPC_SByte_Id:
        if (127 == variant->Value.Sbyte)
        {
            variant->Value.Sbyte = -128;
        }
        else
        {
            variant->Value.Sbyte = (SOPC_SByte)(variant->Value.Sbyte + 1);
        }
        break;
    case SOPC_Byte_Id:
        if (UINT8_MAX == variant->Value.Byte)
        {
            variant->Value.Byte = 0;
        }
        else
        {
            variant->Value.Byte = (SOPC_Byte)(variant->Value.Byte + 1);
        }
        break;

    case SOPC_Int16_Id:
        if (INT16_MAX == variant->Value.Int16)
        {
            variant->Value.Int16 = INT16_MIN;
        }
        else
        {
            variant->Value.Int16 = (int16_t)(variant->Value.Int16 + 1);
        }
        break;
    case SOPC_UInt16_Id:
        if (UINT16_MAX == variant->Value.Uint16)
        {
            variant->Value.Uint16 = 0;
        }
        else
        {
            variant->Value.Uint16 = (uint16_t)(variant->Value.Uint16 + 1);
        }
        break;
    case SOPC_Int32_Id:
        if (INT32_MAX == variant->Value.Int32)
        {
            variant->Value.Int32 = INT32_MIN;
        }
        else
        {
            variant->Value.Int32 = (int32_t)(variant->Value.Int32 + 1);
        }
        break;
    case SOPC_UInt32_Id:
        if (UINT32_MAX == variant->Value.Uint32)
        {
            variant->Value.Uint32 = 0;
        }
        else
        {
            variant->Value.Uint32 = (uint32_t)(variant->Value.Uint32 + 1);
        }
        break;
    case SOPC_Int64_Id:
        if (INT64_MAX == variant->Value.Int64)
        {
            variant->Value.Int64 = INT64_MIN;
        }
        else
        {
            variant->Value.Int64 = (int64_t)(variant->Value.Int64 + 1);
        }
        break;
    case SOPC_UInt64_Id:
        if (UINT64_MAX == variant->Value.Uint64)
        {
            variant->Value.Uint64 = 0;
        }
        else
        {
            variant->Value.Uint64 = (uint64_t)(variant->Value.Uint64 + 1);
        }
        break;
    default:
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
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
    lwv = SOPC_Calloc(CONF_NB_NODE_TO_WRITE, sizeof(OpcUa_WriteValue));
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

        OpcUa_WriteRequest_Initialize(pReq);
        pReq->NoOfNodesToWrite = CONF_NB_NODE_TO_WRITE;
        pReq->NodesToWrite = lwv;

        for (uint32_t i = 0; i < CONF_NB_NODE_TO_WRITE; i++)
        {
            DataValue_Increase(&g_dvArray[i].Value);

            OpcUa_WriteValue_Initialize(&lwv[i]);

            status = SOPC_NodeId_Copy(&lwv[i].NodeId, g_nodeIdArray[i]);
            lwv[i].AttributeId = g_iAttr;

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_DataValue_Copy(&lwv[i].Value, &g_dvArray[i]);
            }
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
            OpcUa_WriteRequest_Clear(pReq);
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
    else
    {
        printf("Receives Write Response for attribute %i:\n", g_iAttr);
        for (i = 0; i < pResp->NoOfResults; ++i)
        {
            sNid = SOPC_NodeId_ToCString(g_nodeIdArray[i]);
            printf("Write Response StatusCode for \"%s\" is 0x%08X\n", sNid, pResp->Results[i]);
            SOPC_Free(sNid);
            sNid = NULL;
        }
    }
}
