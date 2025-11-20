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

/** \file
 *
 * \brief A DeleteNodes example using the high-level client API
 *
 * Requires the toolkit_demo_server to be running compiled with S2OPC_NODE_MANAGEMENT.
 * Connects to the server and realizes an DeleteNodes request on a node.
 * Then disconnects and closes the toolkit.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "opcua_identifiers.h"

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"
#define DEFAULT_CONFIG_ID "deleteNode"

static bool AskKeyPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Password for client key:", outPassword);
}

static bool AskUserNamePass_FromTerminal(const SOPC_SecureConnection_Config* secConnConfig,
                                         char** userName,
                                         char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    const char* prompt1 = "UserName of user (e.g.: 'user1') : ";
    const char* prompt2 = "Password for user : ";

    bool res = SOPC_AskPass_CustomPromptFromTerminal(prompt1, userName);
    if (!res)
    {
        return false;
    }
    res = SOPC_AskPass_CustomPromptFromTerminal(prompt2, outPassword);
    if (!res)
    {
        SOPC_Free(*userName);
        return false;
    }
    return res;
}

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    printf("ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event, status);
}

static void usage(void)
{
    printf(
        "Usage: ./s2opc_wrapper_delete_nodes <nodeId> \n"
        "       (e.g. ./s2opc_wrapper_delete_nodes \"ns=1;i=1012\" true).\n\n"
        "The '" DEFAULT_CONFIG_ID
        "' connection configuration "
        "from " DEFAULT_CLIENT_CONFIG_XML
        " is used.\nThe third parameter is for the parameter DeleteTargetReferences.\n");
}

int main(int argc, char* const argv[])
{
    if (argc != 3)
    {
        usage();
        return -2;
    }
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_delete_nodes_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    size_t nbConfigs = 0;
    SOPC_SecureConnection_Config** scConfigArray = NULL;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_ConfigureFromXML(DEFAULT_CLIENT_CONFIG_XML, NULL, &nbConfigs, &scConfigArray);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_delete_node failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* deleteNodeConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        deleteNodeConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == deleteNodeConnCfg)
        {
            printf("<Example_wrapper_delete_node: failed to load configuration id '" DEFAULT_CONFIG_ID
                   "' from XML config file %s\n",
                   DEFAULT_CLIENT_CONFIG_XML);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&AskKeyPass_FromTerminal);
    }

    /* Define callback to retrieve the client's user password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&AskUserNamePass_FromTerminal);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(deleteNodeConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_delete_node: Failed to connect\n");
        }
    }

    OpcUa_DeleteNodesResponse* deleteNodesResp = NULL;
    OpcUa_DeleteNodesRequest* deleteNodesReq = NULL;
    SOPC_NodeId* nodeIdToDelete = SOPC_NodeId_FromCString(argv[1]);
    if (NULL == nodeIdToDelete)
    {
        printf("Fail to recognize input NodeId %s. Please verify that it is a good format NodeId.\n", argv[1]);
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        deleteNodesReq = SOPC_DeleteNodesRequest_Create(1);
        status = SOPC_DeleteNodesRequest_SetParameters(deleteNodesReq, 0, nodeIdToDelete,
                                                       0 == strcmp(argv[2], "true") ? true : false);
        SOPC_NodeId_Clear(nodeIdToDelete);
        SOPC_Free(nodeIdToDelete);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) deleteNodesReq, (void**) &deleteNodesResp);
        }
        else
        {
            SOPC_ReturnStatus delStatus =
                SOPC_EncodeableObject_Delete(&OpcUa_DeleteNodesRequest_EncodeableType, (void**) &deleteNodesReq);
            SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != deleteNodesResp);
        if (!SOPC_IsGoodStatus(deleteNodesResp->ResponseHeader.ServiceResult))
        {
            printf("Bad status return code. Status: 0x%08" PRIX32 "\n", deleteNodesResp->ResponseHeader.ServiceResult);
            status = SOPC_STATUS_NOK;
        }
        else
        {
            if (deleteNodesResp->NoOfResults != 1)
            {
                printf("Bad number of result in response. Expected one, got:%d\n", deleteNodesResp->NoOfResults);
                status = SOPC_STATUS_NOK;
            }
            else if (!SOPC_IsGoodStatus(deleteNodesResp->Results[0]))
            {
                printf("Bad operation result: 0x%08" PRIX32 "\n", deleteNodesResp->Results[0]);
                status = SOPC_STATUS_NOK;
            }
        }

        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(deleteNodesResp->encodeableType, (void**) &deleteNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Client: DeleteNodes Success.\n");
    }
    else
    {
        printf(">>Client: DeleteNodes Fail.\n");
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_delete_node: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
