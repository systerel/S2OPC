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
 * \brief A read example using the high-level client API
 *
 * Requires the toolkit_test_server to be running.
 * Connect to the server and reads a UInt64 value in a predefined node.
 * Then disconnect and closes the toolkit.
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include "libs2opc_client_cmds.h"
#include "libs2opc_client_toolkit_config.h"
#include "libs2opc_common_config.h"
#include "sopc_macros.h"

static void disconnect_callback(const uint32_t c_id)
{
    printf("===> connection #%d has been terminated!\n", c_id);
}

/**
 * \brief Type of callback to receive user password for decryption of the client private key.
 *
 * \param ppPassword      out parameter, the newly allocated password.
 * \param writtenStatus   out parameter, the status code of the callback process.
 *
 * \warning The callback function shall not do anything blocking or long treatment.
 *          The implementation of the user callback must free the \p ppPassword in case of failure.
 *          The implementation of the user need to update the \p writtenStatus (error or not)
 */
static void Client_PrivateKey_LoadUserPassword(SOPC_String** ppPassword, SOPC_StatusCode* writtenStatus)
{
    /* Retrieve the user password to decrypt the client private key from environment variable
     * TEST_CLIENT_PRIVATE_KEY_PWD.
     */

    *writtenStatus = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == ppPassword)
    {
        return;
    }
    const char* password_ref = getenv("TEST_CLIENT_PRIVATE_KEY_PWD");
    if (NULL == password_ref)
    {
        printf("<Example_wrapper_read: The following environment variables is missing: TEST_CLIENT_PRIVATE_KEY_PWD\n");
        return;
    }
    /* Allocation */
    *ppPassword = SOPC_String_Create();
    if (NULL == *ppPassword)
    {
        *writtenStatus = SOPC_STATUS_OUT_OF_MEMORY;
        return;
    }

    *writtenStatus = SOPC_String_CopyFromCString(*ppPassword, password_ref);
    if (SOPC_STATUS_OK != *writtenStatus)
    {
        printf(
            "<Example_wrapper_read: Failed to copy user password from the environment variable "
            "TEST_CLIENT_PRIVATE_KEY_PWD\n");
        SOPC_String_Delete(*ppPassword);
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
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_read_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK != status)
    {
        res = -1;
    }
    int32_t init = SOPC_ClientHelper_Initialize(disconnect_callback);
    if (init < 0)
    {
        res = -1;
    }

    SOPC_ClientHelper_Security security = {
        .security_policy = SOPC_SecurityPolicy_None_URI,
        .security_mode = OpcUa_MessageSecurityMode_None,
        .path_cert_auth = "./trusted/cacert.der",
        .path_crl = "./revoked/cacrl.der",
        .path_cert_srv = "./server_public/server_2k_cert.der",
        .path_cert_cli = "./client_public/client_2k_cert.der",
        .path_key_cli = "./client_private/encrypted_client_2k_key.pem",
        .policyId = "anonymous",
        .username = NULL,
        .password = NULL,
    };

    SOPC_ClientHelper_EndpointConnection endpoint = {
        .endpointUrl = "opc.tcp://localhost:4841",
        .serverUri = NULL,
        .reverseConnectionConfigId = 0,
    };

    char* node_id = "ns=1;i=1012";

    /* callback to retrieve the client's private key password */
    status = SOPC_HelperConfigClient_SetClientKeyUsrPwdCallback(&Client_PrivateKey_LoadUserPassword);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Example_wrapper_read: Failed to configure the client key user paswword callback\n");
        res = -1;
    }

    /* connect to the endpoint */
    int32_t configurationId = 0;
    if (0 == res)
    {
        configurationId = SOPC_ClientHelper_CreateConfiguration(&endpoint, &security, NULL);
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
        SOPC_ClientHelper_ReadValue readValue;

        /* initalize read value structure */
        readValue.nodeId = node_id;
        readValue.attributeId = 13; // value
        readValue.indexRange = NULL;

        SOPC_DataValue readDataValue;

        /* read the node id value */
        res = SOPC_ClientHelper_Read(connectionId, &readValue, 1, &readDataValue);

        if (0 == res)
        {
            printf("NodeId:\"%s\" - Value:\n", node_id);
            SOPC_Variant_Print(&readDataValue.Value);
        }
        SOPC_DataValue_Clear(&readDataValue);
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
