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
 * \brief A get endpoints example using the high-level client API
 *
 * Requires an OPC UA server with a discovery endpoint at the given URL.
 * Connect to the server and get endpoints from the given discovery endpoint URL.
 * Then disconnect and closes the toolkit.
 *
 */

#include <stdio.h>

#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_client.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"

#define DEFAULT_CONFIG_ID "discovery"

static const char* token_type_string(OpcUa_UserTokenType tokenType)
{
    switch (tokenType)
    {
    case OpcUa_UserTokenType_Anonymous:
        return "Anonymous";
    case OpcUa_UserTokenType_UserName:
        return "UserName";
    case OpcUa_UserTokenType_Certificate:
        return "Certificate";
    case OpcUa_UserTokenType_IssuedToken:
        return "IssuedToken";
    default:
        return "INVALID TOKEN TYPE";
    }
}

static void print_endpoints(OpcUa_GetEndpointsResponse* resp)
{
    if (NULL == resp)
    {
        return;
    }
    printf("Server has %" PRIi32 " endpoints.\n", resp->NoOfEndpoints);

    for (int32_t i = 0; i < resp->NoOfEndpoints; i++)
    {
        printf("Endpoint #%" PRIi32 ":\n", i);
        printf(" - url: %s\n", SOPC_String_GetRawCString(&resp->Endpoints[i].EndpointUrl));
        printf(" - security level: %" PRIu8 "\n", resp->Endpoints[i].SecurityLevel);
        printf(" - security mode: %d\n", resp->Endpoints[i].SecurityMode);
        printf(" - security policy Uri: %s\n", SOPC_String_GetRawCString(&resp->Endpoints[i].SecurityPolicyUri));
        printf(" - transport profile Uri: %s\n", SOPC_String_GetRawCString(&resp->Endpoints[i].TransportProfileUri));

        OpcUa_UserTokenPolicy* userIds = resp->Endpoints[i].UserIdentityTokens;
        for (int32_t j = 0; j < resp->Endpoints[i].NoOfUserIdentityTokens; j++)
        {
            printf("  - User Identity #%" PRIi32 "\n", j);
            printf("    - policy Id: %s\n", SOPC_String_GetRawCString(&userIds[j].PolicyId));
            printf("    - token type: %s\n", token_type_string(userIds[j].TokenType));
            printf("    - issued token type: %s\n", SOPC_String_GetRawCString(&userIds[j].IssuedTokenType));
            printf("    - issuer endpoint Url: %s\n", SOPC_String_GetRawCString(&userIds[j].IssuerEndpointUrl));
            printf("    - security policy Uri: %s\n", SOPC_String_GetRawCString(&userIds[j].SecurityPolicyUri));
        }
    }
}

int main(int argc, char* const argv[])
{
    printf("Usage: %s <discovery endpoint URL> (e.g. %s \"opc.tcp://localhost:4841\").\nThe '" DEFAULT_CONFIG_ID
           "' connection configuration "
           "from " DEFAULT_CLIENT_CONFIG_XML " is used by default if no parameter provided.\n\n",
           argv[0], argv[0]);

    const char* endpointURL = NULL;
    if (argc > 2)
    {
        printf("ERROR: invalid number of arguments provided\n");
        return -2;
    }
    else if (2 == argc)
    {
        endpointURL = argv[1];
        printf("Calling GetEndpoints on endpoint %s (SecurityMode None)\n\n", endpointURL);
    }
    else
    {
        printf("Calling GetEndpoints using the '" DEFAULT_CONFIG_ID
               "' connection configuration "
               "from " DEFAULT_CLIENT_CONFIG_XML ".\n\n");
    }
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_get_endpoints_logs/";
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
        if (NULL == endpointURL)
        {
            // Use XML configuration
            status =
                SOPC_ClientConfigHelper_ConfigureFromXML(DEFAULT_CLIENT_CONFIG_XML, NULL, &nbConfigs, &scConfigArray);

            if (SOPC_STATUS_OK != status)
            {
                printf("<Example_wrapper_get_endpoints: failed to load XML config file %s\n",
                       DEFAULT_CLIENT_CONFIG_XML);
            }
        }
        else
        {
            // Create dedicated configuration
            SOPC_SecureConnection_Config* tmpConnCfg = SOPC_ClientConfigHelper_CreateSecureConnection(
                DEFAULT_CONFIG_ID, endpointURL, OpcUa_MessageSecurityMode_None, SOPC_SecurityPolicy_None);
            if (NULL == tmpConnCfg)
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }

    SOPC_SecureConnection_Config* discConnCfg = NULL;
    if (SOPC_STATUS_OK == status)
    {
        discConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == discConnCfg)
        {
            printf("<Example_wrapper_get_endpoints: failed to load configuration id '" DEFAULT_CONFIG_ID
                   "' from XML config file %s (or created from command line endpoint URL)\n",
                   DEFAULT_CLIENT_CONFIG_XML);

            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_AskPass_FromTerminal);
    }

    OpcUa_GetEndpointsRequest* getEndpointsRequest = NULL;
    OpcUa_GetEndpointsResponse* getEndpointsResponse = NULL;
    if (SOPC_STATUS_OK == status)
    {
        getEndpointsRequest = SOPC_GetEndpointsRequest_Create(argv[1]);
        status = (NULL == getEndpointsRequest ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);
    }

    /* Call Discovery Service without secure connection */
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_ClientHelperNew_DiscoveryServiceSync(discConnCfg, getEndpointsRequest, (void**) &getEndpointsResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(getEndpointsResponse->ResponseHeader.ServiceResult))
        {
            print_endpoints(getEndpointsResponse);
        }
        else
        {
            printf(
                "<Example_wrapper_get_endpoints: GetEndpoints service call on URL %s failed with status: 0x%08" PRIX32
                "\n",
                argv[1], getEndpointsResponse->ResponseHeader.ServiceResult);
        }
    }
    if (NULL != getEndpointsResponse)
    {
        SOPC_EncodeableObject_Delete(getEndpointsResponse->encodeableType, (void**) &getEndpointsResponse);
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
