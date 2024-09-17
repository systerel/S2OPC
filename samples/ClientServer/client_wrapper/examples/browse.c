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
 * \brief A browse example using the high-level client API
 *
 * Requires the toolkit_demo_server to be running.
 * Connect to the server and browse the given node.
 * Then disconnect and closes the toolkit.
 *
 */

#include <stdio.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"
#define DEFAULT_CONFIG_ID "read"

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);

    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    printf("ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event, status);
}

int main(int argc, char* const argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <nodeId> (e.g. %s \"ns=0;i=85\").\nThe '" DEFAULT_CONFIG_ID
               "' connection configuration "
               "from " DEFAULT_CLIENT_CONFIG_XML " is used.\n",
               argv[0], argv[0]);
        return -2;
    }
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_browse_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
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
            printf("<Example_wrapper_browse: failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* readConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        readConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == readConnCfg)
        {
            printf("<Example_wrapper_get_endpoints: failed to load configuration id '" DEFAULT_CONFIG_ID
                   "' from XML config file %s\n",
                   DEFAULT_CLIENT_CONFIG_XML);

            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_AskPass_FromTerminal);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(readConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_browse: Failed to connect\n");
        }
    }

    OpcUa_BrowseRequest* browseRequest = NULL;
    OpcUa_BrowseResponse* browseResponse = NULL;
    if (SOPC_STATUS_OK == status)
    {
        browseRequest = SOPC_BrowseRequest_Create(1, 0, NULL);
        if (NULL != browseRequest)
        {
            status = SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
                browseRequest, 0, argv[1], OpcUa_BrowseDirection_Forward, NULL, true, 0,
                OpcUa_BrowseResultMask_ReferenceTypeId | OpcUa_BrowseResultMask_DisplayName);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, browseRequest, (void**) &browseResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(browseResponse->ResponseHeader.ServiceResult) && 1 == browseResponse->NoOfResults)
        {
            OpcUa_BrowseResult* result = &browseResponse->Results[0];

            printf("status: 0x%08" PRIX32 ", nbOfReferences: %" PRIi32 "\n", result->StatusCode,
                   result->NoOfReferences);
            if (SOPC_IsGoodStatus(result->StatusCode))
            {
                printf("Ref from %s:\n", argv[1]);
                for (int32_t i = 0; i < result->NoOfReferences; i++)
                {
                    OpcUa_ReferenceDescription* ref = &result->References[i];
                    char* strNodeId = SOPC_NodeId_ToCString(&ref->NodeId.NodeId);
                    char* strRefTypeId = SOPC_NodeId_ToCString(&ref->ReferenceTypeId);
                    printf("#%" PRIi32 "\n", i);
                    printf("- nodeId: %s\n", strNodeId);
                    printf("- displayName: %s\n", SOPC_String_GetRawCString(&ref->DisplayName.defaultText));
                    printf("- refTypeId: %s\n", strRefTypeId);

                    SOPC_Free(strNodeId);
                    SOPC_Free(strRefTypeId);
                }
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != browseResponse)
    {
        SOPC_EncodeableObject_Delete(browseResponse->encodeableType, (void**) &browseResponse);
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_browse: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
