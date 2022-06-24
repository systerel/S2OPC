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
 * \brief A getEnpoints example using the high-level client API
 *
 * Requires the toolkit_test_server to be running.
 * Retrieve GetEndpoints information.
 * Then closes the toolkit.
 *
 */

#include <assert.h>
#include <stdio.h>

#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"
#include "sopc_macros.h"

static void print_endpoints(SOPC_ClientHelper_GetEndpointsResult* result)
{
    if (NULL == result)
    {
        return;
    }
    printf("Server has %d endpoints.\n", result->nbOfEndpoints);

    for (int32_t i = 0; i < result->nbOfEndpoints; i++)
    {
        printf("Endpoint #%d:\n", i);
        printf(" - url: %s\n", result->endpoints[i].endpointUrl);
        printf(" - security level: %d\n", result->endpoints[i].securityLevel);
        printf(" - security mode: %d\n", result->endpoints[i].security_mode);
        printf(" - security policy Uri: %s\n", result->endpoints[i].security_policyUri);
        printf(" - transport profile Uri: %s\n", result->endpoints[i].transportProfileUri);

        SOPC_ClientHelper_UserIdentityToken* userIds = result->endpoints[i].userIdentityTokens;
        for (int32_t j = 0; j < result->endpoints[i].nbOfUserIdentityTokens; j++)
        {
            printf("  - User Identity #%d\n", j);
            printf("    - policy Id: %s\n", userIds[j].policyId);
            printf("    - token type: %d\n", userIds[j].tokenType);
            printf("    - issued token type: %s\n", userIds[j].issuedTokenType);
            printf("    - issuer endpoint Url: %s\n", userIds[j].issuerEndpointUrl);
            printf("    - security policy Uri: %s\n", userIds[j].securityPolicyUri);
        }
    }
}

int main(int argc, char* const argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_get_endpoints_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK != status)
    {
        res = -1;
    }

    if (0 == res)
    {
        int32_t init = SOPC_ClientHelper_Initialize(NULL);
        if (init < 0)
        {
            res = -1;
        }
    }

    const char* endpoint_url = "opc.tcp://localhost:4841";

    /* GetEndpoints */
    SOPC_ClientHelper_GetEndpointsResult* getEndpointResult = NULL;
    if (0 == res)
    {
        res = SOPC_ClientHelper_GetEndpoints(endpoint_url, &getEndpointResult);
    }

    if (0 == res)
    {
        print_endpoints(getEndpointResult);
        SOPC_ClientHelper_GetEndpointsResult_Free(&getEndpointResult);
    }
    else
    {
        printf("GetEndpoints FAILED, error code: %d\n", res);
    }

    /* Close the toolkit */
    SOPC_ClientHelper_Finalize();
    SOPC_CommonHelper_Clear();

    return res;
}
