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

/* FreeRTOS include */
#include "FreeRTOS.h"
#include "task.h" // Used for disable task entering in user assert
#include "sys_evt.h" // Used to signal when MxNet has finished configuring Network interface

/* S2OPC includes */
#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"

#include "p_threads.h"
#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_udp_sockets.h"

/* MIMXRT1064 includes */
//#include "fsl_debug_console.h"
//#include "p_ethernet_if.h"

#include "logging.h"

#define ENDPOINT_URL "opc.tcp://192.168.1.64:4841"
//#define ENDPOINT_URL "opc.tcp://192.168.1.108:4841"
//#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define SERVER_URI "urn:S2OPC:localhost"

static void cbDisconnect(const uint32_t connectionId);
static SOPC_ReturnStatus clientInitialize(void);
/* return ConfigurationId used by connection API
 ConfigurationId should be 0< otherwise configuration failed */
static int32_t clientConfigure(void);
/* Do a browse request to nodeId and populate result of the request in parameter browseResult
    If nodeId is NULL then browse node "ns=0;i=85" which is root node Id
*/
static SOPC_ReturnStatus clientBrowse(int32_t connectionId,
                                      const char* nodeId,
                                      SOPC_ClientHelper_BrowseResult* browseResult);
/* Do a write request to nodeId "ns=1;s=Int32_001" which store a counter incrementing each time the function is called
 */
static SOPC_ReturnStatus clientWrite(int32_t connectionId);
/* Do a read request of nodeId "ns=1;s=Int32_001" */
static SOPC_ReturnStatus clientRead(int32_t connectionId);

static void log_UserCallback(const char* context, const char* text)
{
    SOPC_UNUSED_ARG(context);
    if (NULL != text)
    {
    	//LogDebug("log_UserCallback function");
        LogInfo("%s\r\n", text);
    }
}

static void assert_userCallback(const char* context)
{
    LogInfo("ASSERT FAILED : <%p>\r\n", (void*) context);
    LogInfo("Context: <%s>", context);
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

void cbToolkit_test_client(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t configurationId = 0;
    int32_t connectionId = 0;
    //eEthernetIfResult ethResult = ETHERNET_IF_RESULT_OK;
    SOPC_ClientHelper_BrowseResult* browseResult = SOPC_Calloc(1, sizeof(SOPC_ClientHelper_BrowseResult));
    uint32_t timeoutIf = UINT32_MAX;

    /* Block until the network interface is connected */
    ( void ) xEventGroupWaitBits( xSystemEvents,
                                  EVT_MASK_NET_CONNECTED,
                                  0x00,
                                  pdTRUE,
                                  portMAX_DELAY );

    // Initialize Client Toolkit
    status = clientInitialize();

    LogDebug("client init"); //n'y arrive pas encore 14/03 -> Check

    if (SOPC_STATUS_OK == status)
    {
        configurationId = clientConfigure();
        if (configurationId <= 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        //ethResult = P_ETHERNET_IF_IsReady(timeoutIf);
        if (1)//ETHERNET_IF_RESULT_OK == ethResult)
        {
            connectionId = SOPC_ClientHelper_CreateConnection(configurationId); //ICI
            if (connectionId <= 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }

    }

    if (SOPC_STATUS_OK == status)
    {
        status = clientBrowse(connectionId, NULL, browseResult);
        if (SOPC_STATUS_OK != status)
        {
            LogInfo("Failed to Browse specified node\r\n");
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = clientRead(connectionId);
        if (SOPC_STATUS_OK != status)
        {
            LogInfo("Failed to read specified node \n\r");
        }
    }

    while (SOPC_STATUS_OK == status)
    {
        status = clientWrite(connectionId);
        SOPC_Sleep(100);
    }
    // Clear browseResult
    SOPC_Free(browseResult);

    // Clear S2OPC Client
    SOPC_ClientHelper_Finalize();
    SOPC_Common_Clear();
}

void cbDisconnect(const uint32_t connectionId)
{
    SOPC_UNUSED_ARG(connectionId);
}

SOPC_ReturnStatus clientInitialize(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t res = 0;

    // Set user assert
    SOPC_Assert_Set_UserCallback(&assert_userCallback);

    // Initialize toolkit and configure logs
    SOPC_Log_Configuration logConfig = {.logLevel = SOPC_LOG_LEVEL_INFO,   //change loglevel
                                        .logSystem = SOPC_LOG_SYSTEM_USER,
                                        .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};

    status = SOPC_CommonHelper_Initialize(&logConfig);

    if (SOPC_STATUS_OK == status)
    {
        // Initialize client and set callback when client disconnect
        res = SOPC_ClientHelper_Initialize(cbDisconnect);
        if (res < 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

int32_t clientConfigure(void)
{
    int32_t configurationId = 0;
    SOPC_ClientHelper_EndpointConnection endpoint = {
        .endpointUrl = ENDPOINT_URL,
        .serverUri = SERVER_URI,
        .reverseConnectionConfigId = 0,
    };
    SOPC_ClientHelper_Security security = {
		.path_cert_auth = NULL,
		.path_cert_cli = NULL,
		.path_cert_srv = NULL,
		.path_crl = NULL,
		.path_key_cli = NULL,
        .policyId = "anonymous",
        .security_mode = OpcUa_MessageSecurityMode_None,
        .security_policy = SOPC_SecurityPolicy_None_URI,
        .username = NULL,//"user1",//NULL
        .password = NULL,//"password",//NULL
    };
    //        .path_cert_auth = "./Certs/trusted/cacert.der",
    //        .path_cert_cli = "./Certs/client_public/client_4k_cert.der",
    //        .path_cert_srv = "./Certs/server_public/server_4k_cert.der",
    //        .path_crl = "./Certs/revoked/cacrl.der",
    //        .path_key_cli = "./Certs/client_private/encrypted_client_4k_key.pem",
    //        .security_mode = OpcUa_MessageSecurityMode_Sign,
    //        .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,

    // Configure client
    configurationId = SOPC_ClientHelper_CreateConfiguration(&endpoint, &security, NULL);

    return configurationId;
}

SOPC_ReturnStatus clientBrowse(int32_t connectionId, const char* nodeId, SOPC_ClientHelper_BrowseResult* browseResult)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t res = 0;
    if (NULL == nodeId)
    {
        nodeId = "ns=0;i=85";
    }
    SOPC_ClientHelper_BrowseRequest browseRequest = {
        .direction = OpcUa_BrowseDirection_Forward, // forward
        .nodeId = nodeId,
        .referenceTypeId = NULL, // all reference types
        .includeSubtypes = true,
    };

    res = SOPC_ClientHelper_Browse(connectionId, &browseRequest, 1, browseResult);
    if (0 == res)
    {
        for (int32_t i = 0; i < browseResult->nbOfReferences; i++)
        {
            const SOPC_ClientHelper_BrowseResultReference* ref = &browseResult->references[i];
            LogInfo("Item #%i \r\n", i);
            LogInfo("- nodeId: %s\r\n", ref->nodeId);
            LogInfo("- displayName: %s\r\n", ref->displayName);

            SOPC_Free(ref->nodeId);
            SOPC_Free(ref->displayName);
            SOPC_Free(ref->browseName);
            SOPC_Free(ref->referenceTypeId);
        }
        SOPC_Free(browseResult->references);
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }
    return status;
}

SOPC_ReturnStatus clientWrite(int32_t connectionId)
{
    static int32_t counter = 0;
    counter++;
    int32_t saveCounter = counter;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int32_t res = 0;

    SOPC_ClientHelper_WriteValue* writeValue = SOPC_Calloc(1, sizeof(SOPC_ClientHelper_WriteValue));
    SOPC_StatusCode statusCode = 0;
    SOPC_DataValue* dv = SOPC_Calloc(1, sizeof(SOPC_DataValue));

    // Set WriteValue parameters
    writeValue->nodeId = "ns=1;s=Int32_001";
    writeValue->indexRange = NULL;

    // Initialize dataValue and fill it
    SOPC_DataValue_Initialize(dv);
    dv->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv->Value.BuiltInTypeId = SOPC_Int32_Id;
    dv->Value.DoNotClear = true;
    dv->Value.Value.Int32 = saveCounter;

    // fill WriteValue with dataValue
    writeValue->value = dv;

    // Send write Request Helper
    res = SOPC_ClientHelper_Write(connectionId, writeValue, 1, &statusCode);
    if (res < 0)
    {
        status = SOPC_STATUS_NOK;
    }

    LogDebug("Write done");
    // Clear
    SOPC_Free(dv);
    SOPC_Free(writeValue);
    return status;
}

SOPC_ReturnStatus clientRead(int32_t connectionId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ClientHelper_ReadValue* readValue = SOPC_Calloc(1, sizeof(SOPC_ClientHelper_ReadValue));
    SOPC_DataValue* dv = SOPC_Calloc(1, sizeof(SOPC_DataValue));
    SOPC_DataValue_Initialize(dv);

    readValue->nodeId = "ns=1;s=Int32_001";
    readValue->attributeId = SOPC_AttributeId_Value;
    readValue->indexRange = NULL;

    status = SOPC_ClientHelper_Read(connectionId, readValue, 1, dv);

    LogInfo("Read node %s \r\n", readValue->nodeId);
    LogInfo("- Value %u \r\n", dv->Value.Value.Uint32);

    SOPC_Free(dv);
    SOPC_Free(readValue);
    return status;
}
