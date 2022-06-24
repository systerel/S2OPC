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
 * \brief A subscribe example using the high-level client API
 *
 * Requires the toolkit_test_server to be running.
 * Connect to the server and subscribes to a UInt64 value to a predefined node.
 * Waits 30 seconds and then disconnect and closes the toolkit.
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"
#include "sopc_macros.h"

#include "sopc_time.h"

static void datachange_callback(const int32_t c_id, const char* node_id, const SOPC_DataValue* value)
{
    char sz[1024];
    size_t n;

    n = (size_t) snprintf(sz, sizeof(sz) / sizeof(sz[0]),
                          "Client %" PRIu32 " data change:\n  value id %s\n  new value ", c_id, node_id);

    if (NULL == value)
    {
        snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "NULL");
    }
    else
    {
        SOPC_Variant variant = value->Value;
        if (SOPC_UInt64_Id == variant.BuiltInTypeId)
        {
            snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "%" PRIu64, (int64_t) variant.Value.Uint64);
        }
    }

    printf("%s\n", sz);
}

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
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_subscribe_logs/";
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
    char* node_id = "ns=1;i=1012";

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
        res = SOPC_ClientHelper_CreateSubscription(connectionId, datachange_callback);
    }

    if (0 == res)
    {
        SOPC_StatusCode result = SOPC_UncertainStatusMask;
        res = SOPC_ClientHelper_AddMonitoredItems(connectionId, &node_id, 1, &result);
        if (res > 0)
        {
            printf("Error in add operation for the monitored item: 0x%08" PRIX32 "\n", result);
        }
    }

    if (res == 0)
    {
        SOPC_Sleep(30);
        SOPC_ClientHelper_Unsubscribe(connectionId);
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
