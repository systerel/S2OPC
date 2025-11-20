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
 * \brief A AddNode example using the high-level client API
 *
 * Requires the toolkit_demo_server to be running compiled with S2OPC_NODE_MANAGEMENT.
 * Connects to the server and realizes an AddNodes request for adding a Variable or Object node
 * with default reference to parent set to "Organizes", and default TypeDefinition set
 * to "ObjectType" if the node is an Object.
 * Then disconnects and closes the toolkit.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
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
#define DEFAULT_CONFIG_ID "addNode"

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
        "Usage: ./s2opc_wrapper_add_nodes <BrowseName> <nodeClass> <parentNodeId> \n"
        "       (e.g. ./s2opc_wrapper_add_nodes \"MyNewVariable\" \"Variable\" \"ns=0;i=85\").\n\n"
        "The '" DEFAULT_CONFIG_ID
        "' connection configuration "
        "from " DEFAULT_CLIENT_CONFIG_XML " is used.\n");
}

static SOPC_ReturnStatus client_send_add_nodes_req_test(SOPC_ClientConnection* secureConnection,
                                                        const char* VariableOrObject,
                                                        SOPC_ExpandedNodeId* nodeToAdd,
                                                        SOPC_QualifiedName* nodeToAddBrowseName,
                                                        SOPC_ExpandedNodeId* parentNodeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;
    SOPC_ExpandedNodeId typeDefinition;
    SOPC_ExpandedNodeId_Initialize(&typeDefinition);
    SOPC_NodeId referenceTypeId;
    SOPC_NodeId_Initialize(&referenceTypeId);
    // Reference type "Organizes" node by default
    referenceTypeId.Data.Numeric = OpcUaId_Organizes;

    addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (0 == strcmp(VariableOrObject, "Object"))
    {
        // Type definition can be any NodeId which NodeClass is ObjectType (ServerType, FolderType, ...)
        // Use NodeId BaseObjectType for this example
        typeDefinition.NodeId.Data.Numeric = OpcUaId_BaseObjectType;
        static const SOPC_Byte byte_one = 1;
        status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 0, parentNodeId, &referenceTypeId, nodeToAdd,
                                                         nodeToAddBrowseName, &typeDefinition, NULL, NULL, NULL, NULL,
                                                         &byte_one);
    }
    else if (0 == strcmp(VariableOrObject, "Variable"))
    {
        typeDefinition.NodeId.Data.Numeric = OpcUaId_BaseDataVariableType;
        status = SOPC_AddNodeRequest_SetVariableAttributes(addNodesReq, 0, parentNodeId, &referenceTypeId, nodeToAdd,
                                                           nodeToAddBrowseName, &typeDefinition, NULL, NULL, NULL, NULL,
                                                           NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
    }
    else // NodeClass not supported yet.
    {
        printf(">> Invalid NodeClass for new node. NodeClass supported for the moment: Variable, Object.\n\n");
        usage();
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != addNodesResp);
        if (!SOPC_IsGoodStatus(addNodesResp->ResponseHeader.ServiceResult))
        {
            printf("Bad Service status code returned: 0x%08" PRIX32 "\n", addNodesResp->ResponseHeader.ServiceResult);
            status = SOPC_STATUS_NOK;
        }
        else
        {
            printf("Service result returned good.\n");
            if (addNodesResp->NoOfResults != 1)
            {
                printf("Bad number of result in response. Expected one, got:%d\n", addNodesResp->NoOfResults);
                status = SOPC_STATUS_NOK;
            }
            if (!SOPC_IsGoodStatus(addNodesResp->Results[0].StatusCode))
            {
                printf("Bad Operation status code returned: 0x%08" PRIX32 "\n", addNodesResp->Results[0].StatusCode);
                status = SOPC_STATUS_NOK;
            }
            else if (!SOPC_NodeId_Equal(&nodeToAdd->NodeId, &addNodesResp->Results[0].AddedNodeId))
            {
                printf("NodeId added is not equal to the one requested to add!\nData string:%s\n",
                       SOPC_String_GetRawCString(&addNodesResp->Results[0].AddedNodeId.Data.String));
                status = SOPC_STATUS_NOK;
            }
        }

        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    return status;
}

int main(int argc, char* const argv[])
{
    if (argc != 4)
    {
        usage();
        return -2;
    }
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_add_nodes_logs/";
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
            printf("<Example_wrapper_add_node failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* addNodeConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        addNodeConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == addNodeConnCfg)
        {
            printf("<Example_wrapper_add_node: failed to load configuration id '" DEFAULT_CONFIG_ID
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

    /* Update UserPolicyId */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_UpdateUserPolicyId(addNodeConnCfg);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(addNodeConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_add_node: Failed to connect\n");
        }
    }

    /* Set parameters of the add nodes service */
    SOPC_ExpandedNodeId nodeToAdd;
    SOPC_QualifiedName nodeToAddBrowseName;
    SOPC_ExpandedNodeId parentNode;
    SOPC_NodeId* parentNodeId = NULL;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ExpandedNodeId_Initialize(&nodeToAdd);
        SOPC_ExpandedNodeId_Initialize(&parentNode);
        // Make node to add
        nodeToAdd.NodeId.Namespace = 1;
        nodeToAdd.NodeId.IdentifierType = SOPC_IdentifierType_String;
        status = SOPC_String_AttachFromCstring(&nodeToAdd.NodeId.Data.String, argv[1]);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        // Make its BrowseName
        SOPC_QualifiedName_Initialize(&nodeToAddBrowseName);
        nodeToAddBrowseName.NamespaceIndex = 1;
        status = SOPC_String_AttachFromCstring(&nodeToAddBrowseName.Name, argv[1]);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        // Parent node is given by NodeId
        parentNodeId = SOPC_NodeId_FromCString(argv[3]);
        if (NULL == parentNodeId)
        {
            printf(">> Invalid format for parent NodeId.\n\n");
            usage();
            status = SOPC_STATUS_NOK;
        }
        else
        {
            parentNode.NodeId = *parentNodeId;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status =
            client_send_add_nodes_req_test(secureConnection, argv[2], &nodeToAdd, &nodeToAddBrowseName, &parentNode);
        // Free parent node
        SOPC_NodeId_Clear(parentNodeId);
        SOPC_Free(parentNodeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Client: Test AddNodes Success.\n");
    }
    else
    {
        printf(">>Client: Test AddNodes Fail.\n");
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_add_node: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
