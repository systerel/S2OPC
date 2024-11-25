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
 * \brief A connect example using the high-level client API
 *
 * This is a connect example that just tries to connect to the server. Used for testing purpose.
 * - Requires the toolkit_demo_server to be running.
 * - Connects to the server.
 * - Disconnects and closes the toolkit.
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
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#define APP_NAME "s2opc_wrapper_connect"
#define LOG_DIR "./" APP_NAME "_logs/"
#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Audit_Config.xml"

#define TEST_PASSWORD_PRIVATE_KEY "TEST_PASSWORD_PRIVATE_KEY"
#define TEST_USERNAME "TEST_USERNAME"
#define TEST_PASSWORD_USER "TEST_PASSWORD_USER"

/******************************************************************************
 *  CALLBACKS
 ******************************************************************************/
static bool askUserPass_FromEnv(const SOPC_SecureConnection_Config* secConnConfig,
                                char** outUserName,
                                char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    SOPC_ASSERT(NULL != outUserName);
    SOPC_ASSERT(NULL != outPassword);

    char* _outUser = getenv(TEST_USERNAME);
    if (NULL == _outUser)
    {
        printf("<" APP_NAME ": The following environment variable is missing: %s\n", TEST_USERNAME);
        return false;
    }

    char* _outPassword = getenv(TEST_PASSWORD_USER);
    if (NULL == _outUser)
    {
        printf("<" APP_NAME ": The following environment variable is missing: %s\n", TEST_PASSWORD_USER);
        return false;
    }

    printf("Used user from environment variable %s/%s\n", TEST_USERNAME, TEST_PASSWORD_USER);
    *outUserName = SOPC_strdup(_outUser);     // Do a copy
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    return true;
}

static bool askPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        We have to make a copy here because in any case, we will free the password and not distinguish if it come
        from environement or terminal after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
    */
    char* _outPassword = getenv(TEST_PASSWORD_PRIVATE_KEY);
    if (NULL == _outPassword)
    {
        printf("<" APP_NAME ": The following environment variable is missing: %s\n", TEST_PASSWORD_PRIVATE_KEY);
        return false;
    }
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    printf("Key password read from environment variable %s\n", TEST_PASSWORD_PRIVATE_KEY);

    return NULL != *outPassword;
}

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
        printf(
            "Usage: %s <configId> (e.g. %s '1').\n"
            "Makes a simple connection using DEFAULT_CLIENT_CONFIG_XML='" DEFAULT_CLIENT_CONFIG_XML
            "' file and the provided connection configuration Id.\n"
            "Note : In this test-purpose application, the passwords must be provided using environment variables.\n",
            argv[0], argv[0]);
        return -2;
    }
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = LOG_DIR;
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
            printf("<" APP_NAME ": failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* readConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        readConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(argv[1]);

        if (NULL == readConnCfg)
        {
            printf("<" APP_NAME
                   ": failed to load configuration id '%s"
                   "' from XML config file " DEFAULT_CLIENT_CONFIG_XML "\n",
                   argv[1]);

            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&askPass_FromEnv);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&askUserPass_FromEnv);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(readConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<" APP_NAME ": Failed to connect\n");
        }
    }

    SOPC_Sleep(200);

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<" APP_NAME ": Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
