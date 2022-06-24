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
 * Requires the toolkit_test_server to be running.
 * Connect to the server and browse the "/Root/Objects" node.
 * Then disconnect and closes the toolkit.
 *
 */

#include <assert.h>
#include <stdio.h>

#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"
#include "sopc_macros.h"

static void disconnect_callback(const uint32_t c_id)
{
    printf("===> connection #%d has been terminated!\n", c_id);
}

int main(int argc, char* const argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_browse_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK != status)
    {
        res = -1;
    }

    if (0 == res)
    {
        int32_t init = SOPC_ClientHelper_Initialize(disconnect_callback);
        if (init < 0)
        {
            res = -1;
        }
    }

    SOPC_ClientHelper_Security security = {
        .security_policy = SOPC_SecurityPolicy_None_URI,
        .security_mode = OpcUa_MessageSecurityMode_None,
        .path_cert_auth = "./trusted/cacert.der",
        .path_crl = "./revoked/cacrl.der",
        .path_cert_srv = "./server_public/server_2k_cert.der",
        .path_cert_cli = "./client_public/client_2k_cert.der",
        .path_key_cli = "./client_private/client_2k_key.pem",
        .policyId = "anonymous",
        .username = NULL,
        .password = NULL,
    };

    const char* endpoint_url = "opc.tcp://localhost:4841";

    /* connect to the endpoint */
    int32_t configurationId = 0;
    if (0 == res)
    {
        configurationId = SOPC_ClientHelper_CreateConfiguration(endpoint_url, &security, NULL);
        if (configurationId <= 0)
        {
            res = -1;
        }
    }

    int32_t connectionId = 0;
    if (0 == res)
    {
        connectionId = SOPC_ClientHelper_CreateConnection(configurationId);

        if (connectionId <= 0)
        {
            /* connectionId is invalid */
            res = -1;
        }
    }

    if (0 == res)
    {
        SOPC_ClientHelper_BrowseRequest browseRequest;
        SOPC_ClientHelper_BrowseResult browseResult;

        browseRequest.nodeId = "ns=0;i=85";                      // Root/Objects/
        browseRequest.direction = OpcUa_BrowseDirection_Forward; // forward
        browseRequest.referenceTypeId = "";                      // all reference types
        browseRequest.includeSubtypes = true;

        /* Browse specified node */
        res = SOPC_ClientHelper_Browse(connectionId, &browseRequest, 1, &browseResult);

        if (0 == res)
        {
            printf("status: %d, nbOfResults: %d\n", browseResult.statusCode, browseResult.nbOfReferences);
            for (int32_t i = 0; i < browseResult.nbOfReferences; i++)
            {
                const SOPC_ClientHelper_BrowseResultReference* ref = &browseResult.references[i];
                printf("Item #%d\n", i);
                printf("- nodeId: %s\n", ref->nodeId);
                printf("- displayName: %s\n", ref->displayName);

                free(ref->nodeId);
                free(ref->displayName);
                free(ref->browseName);
                free(ref->referenceTypeId);
            }
            free(browseResult.references);
        }
        else
        {
            printf("Call to Browse service through client wrapper failed with return code: %d\n", res);
        }
    }

    if (connectionId > 0)
    {
        int32_t discoRes = SOPC_ClientHelper_Disconnect(connectionId);
        res = res != 0 ? res : discoRes;
    }

    /* Close the toolkit */
    SOPC_ClientHelper_Finalize();
    SOPC_CommonHelper_Clear();

    return res;
}
