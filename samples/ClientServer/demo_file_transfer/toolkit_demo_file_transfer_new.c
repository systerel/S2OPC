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
 * \brief Typical use of the API to manage the OPC UA FileTransfer.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_file_transfer.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "opcua_statuscodes.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

// Server endpoints and PKI configuration
#define XML_SERVER_CFG_PATH "./S2OPC_Server_Demo_Config.xml"
// Server address space configuration
#define XML_ADDRESS_SPACE_PATH "./s2opc_file_transfer.xml"
// User credentials and authorizations
#define XML_USERS_CFG_PATH "./S2OPC_Users_Demo_Config.xml"

/*-----------------------
 * Logger configuration :
 *-----------------------*/

/* Set the log path and set directory path built on executable name prefix */
static char* Server_ConfigLogPath(const char* logPrefix)
{
    char* logDirPath = NULL;

    size_t logDirPathSize = strlen(logPrefix) + 7; // <logPrefix> + "_logs/" + '\0'

    logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));

    if (NULL != logDirPath && (int) (logDirPathSize - 1) != snprintf(logDirPath, logDirPathSize, "%s_logs/", logPrefix))
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
}

/*
 * Server callback definition to ask for private key password during configuration phase
 */
static bool SOPC_PrivateKeyAskPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Private key password:\n", outPassword);
}

static SOPC_ReturnStatus Server_LoadServerConfigurationFromPaths(void)
{
    SOPC_ReturnStatus status =
        SOPC_ServerConfigHelper_ConfigureFromXML(XML_SERVER_CFG_PATH, XML_ADDRESS_SPACE_PATH, XML_USERS_CFG_PATH, NULL);
    if (SOPC_STATUS_OK != status)
    {
        printf("******* Failed to load configuration from paths:\n");
        printf("******* \t--> need file (relative path where the server is running):\t%s\n", XML_USERS_CFG_PATH);
        printf("******* \t--> need file (relative path where the server is running):\t%s\n", XML_SERVER_CFG_PATH);
        printf("******* \t--> need file (relative path where the server is running):\t%s\n", XML_ADDRESS_SPACE_PATH);
    }
    return status;
}

/*
 * User Server callback definition used for address space modification by client.
 * The callback function shall not do anything blocking or long treatment
 */
static void UserWriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                          OpcUa_WriteValue* writeValue,
                                          SOPC_StatusCode writeStatus)
{
    /********************/
    /* USER CODE BEGING */
    /********************/
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);

    printf("Write notification (%s) on node '%s' by user '%s'\n", writeSuccess, sNodeId, SOPC_User_ToCString(user));
    SOPC_Free(sNodeId);
    /********************/
    /* END USER CODE   */
    /********************/
}

static void UserOpenCb(SOPC_OpenMode openMode, bool* authorized, SOPC_ByteString* fileByteString)
{
    SOPC_UNUSED_ARG(openMode);
    SOPC_UNUSED_ARG(authorized);
    SOPC_UNUSED_ARG(fileByteString);
    printf(" > UserOpenCb\n");
}

static void UserCloseCb(SOPC_ByteString* fileByteString, bool* localSave)
{
    SOPC_UNUSED_ARG(localSave);
    printf(" > UserCloseCb. File Closed : ");
    for (int i = 0; i < fileByteString->Length; i++)
    {
        printf("%c", fileByteString->Data[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[])
{
    printf("******* Starting demo FileTransfer server\n");

    // Note: avoid unused parameter warning from compiler
    (void) argc;

    /* Configure the server logger:
     * DEBUG traces generated in ./<argv[0]_logs/ */
    SOPC_Log_Configuration logConfig = SOPC_Common_GetDefaultLogConfiguration();
    logConfig.logLevel = SOPC_LOG_LEVEL_DEBUG;
    char* logDirPath = Server_ConfigLogPath(argv[0]);
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;

    SOPC_ReturnStatus status;

    /* Configure the server to support message size of 128 Mo */
    SOPC_Common_EncodingConstants encConf = SOPC_Common_GetDefaultEncodingConstants();
    encConf.buffer_size = 2097152;
    encConf.receive_max_nb_chunks = 100;
    /* receive_max_msg_size = buffer_size * receive_max_nb_chunks */
    encConf.receive_max_msg_size = 209715200; // 209 Mo
    encConf.send_max_nb_chunks = 100;
    /* send_max_msg_size = buffer_size  * send_max_nb_chunks */
    encConf.send_max_msg_size = 209715200; // 209 Mo
    encConf.max_string_length = 209715200; // 209 Mo

    bool res = SOPC_Common_SetEncodingConstants(encConf);
    if (false == res)
    {
        printf("******* Failed to configure message size of S2OPC\n");
        return 0;
    }
    status = SOPC_CommonHelper_Initialize(&logConfig, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Unable to initialize the S2OPC Server frontend configuration.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* This function must be called after the initialization functions of the server library and
           before starting the server and its configuration. */
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_PrivateKeyAskPass_FromTerminal);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* status = Server_LoadServerConfigurationFromFiles(); */
        status = Server_LoadServerConfigurationFromPaths();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetWriteNotifCallback(&UserWriteNotificationCallback);
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to configure the @ space modification notification callback\n");
        }
    }

    /* Start File Transfer Initialization */

    // Add basic file method (of a FileType object).
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        // Note: in case of Nano compliant server this manager will never be used
        // since CallMethod service is not available
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileTransfer_Initialize(mcm);
    }

    SOPC_FileType* file = NULL;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_FileType_Config* fileConfig = SOPC_FileType_Config_CreateFromStringNodeId(
            "ns=1;i=15478", "ns=1;i=15484", "ns=1;i=15487", "ns=1;i=15489", "ns=1;i=15492", "ns=1;i=15494",
            "ns=1;i=15497", "ns=1;i=15479", "ns=1;i=15482", "ns=1;i=15481", "ns=1;i=15480");

        if (fileConfig != NULL)
        {
            file = SOPC_FileType_Create(fileConfig);
        }
        if (NULL == file)
        {
            status = SOPC_STATUS_NOK;
            printf("Fail to create File object !");
        }
    }

    /* Adds user cb */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileType_SetUserOpenCb(file, &UserOpenCb);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileType_SetUserCloseCb(file, &UserCloseCb);
    }
    if (SOPC_STATUS_OK == status)
    {
        printf("<Demo_Server: Server started\n");

        /* Run the server until error  or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_Serve(true);

        /**
         * Alternative is using SOPC_ServerHelper_StartServer to make server start asynchronously
         * (and then SOPC_ServerHelper_StopServer to stop it).
         * It is then possible to use local services to access / write data:
         * - Synchronously with SOPC_ServerHelper_LocalServiceSync(...)
         * - Asynchronously with SOPC_ServerHelper_LocalServiceAsync(...): need
         * SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(...) to be configured prior to start server
         */

        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Server: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<Demo_Server: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<Demo_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_FileTransfer_Clear();
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Demo_Server: Terminating with error status, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
