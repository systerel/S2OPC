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

#include <stdio.h>
#include <string.h>

#include "libs2opc_common_config.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_file_transfer.h"
#include "sopc_helper_askpass.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define ITEM_1_FILE_NAME "item1File.log"
#define ITEM_2_FILE_NAME "item2File.log"

static char* gLogDirPath = NULL;

/*-----------------------
 * Logger configuration
 *-----------------------*/

/* Set the log path and set directory path built on executable name prefix */
static char* Server_ConfigLogPath(const char* logPrefix)
{
    char* logDirPath = NULL;

    size_t logDirPathSize = strlen(logPrefix) + 7; // <logPrefix> + "_logs/" + '\0'

    logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));

    int res = snprintf(logDirPath, logDirPathSize, "%s_logs/", logPrefix);
    if (NULL != logDirPath && (int) (logDirPathSize - 1) != res)
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
}

static SOPC_ReturnStatus Server_LoadServerConfigurationFromPaths(void)
{
    // Server endpoints and PKI configuration
    const char* xml_server_cfg_path = "./S2OPC_Server_UACTT_Config.xml";
    // Server address space configuration
    const char* xml_address_space_path = "./ft_data/address_space.xml";
    // User credentials and authorizations
    const char* xml_users_cfg_path = "./S2OPC_UACTT_Users.xml";

    return SOPC_HelperConfigServer_ConfigureFromXML(xml_server_cfg_path, xml_address_space_path, xml_users_cfg_path,
                                                    NULL);
}

/* Close file CallBack */
static void(UserCloseCallback)(const char* tmp_file_path)
{
    char* fileName = strstr(tmp_file_path, ITEM_1_FILE_NAME);
    if (NULL != fileName)
    {
        fileName = ITEM_1_FILE_NAME;
    }
    else
    {
        fileName = strstr(tmp_file_path, ITEM_2_FILE_NAME);
        if (NULL != fileName)
        {
            fileName = ITEM_2_FILE_NAME;
        }
    }
    SOPC_ASSERT(NULL != fileName);

    size_t sizeFilePath = strlen(gLogDirPath) + strlen(fileName) + 1; // +1 \0
    char* newName = SOPC_Calloc(sizeFilePath, sizeof(char));
    SOPC_ASSERT("<test_server_file_transfer: Closing callback: failed to create a new file name." && NULL != newName);
    int res = snprintf(newName, sizeFilePath, "%s%s", gLogDirPath, fileName);
    SOPC_ASSERT("<test_server_file_transfer: Closing callback: failed to create a new file name." &&
                (int) (sizeFilePath - 1) == res);
    res = rename(tmp_file_path, newName);
    SOPC_ASSERT("<test_server_file_transfer: Closing callback: failed to rename the file name." && 0 == res);
    SOPC_Free(newName);
}

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    printf("<test_server_file_transfer: Server started\n");

    // Configure the server to support file transfer message size of 100 Ko
    SOPC_Common_EncodingConstants encConf = SOPC_Common_GetDefaultEncodingConstants();
    encConf.max_string_length = 102400; // 100 Ko

    bool res = SOPC_Common_SetEncodingConstants(encConf);
    SOPC_ASSERT("<test_server_file_transfer: Failed to configure message size of S2OPC" && false != res);

    // Get default log config and set the custom path
    SOPC_Log_Configuration log_config = SOPC_Common_GetDefaultLogConfiguration();
    log_config.logLevel = SOPC_LOG_LEVEL_DEBUG;
    gLogDirPath = Server_ConfigLogPath(argv[0]);
    log_config.logSysConfig.fileSystemLogConfig.logDirPath = gLogDirPath;
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&log_config);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    SOPC_ASSERT("<test_server_file_transfer: Server initialization failed" && SOPC_STATUS_OK == status);

    // Define the callback to retrieve the password of server private key
    status = SOPC_HelperConfigServer_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    SOPC_ASSERT("<test_server_file_transfer: SetKeyPasswordCallback failed" && SOPC_STATUS_OK == status);

    // load config from XML file :
    status = Server_LoadServerConfigurationFromPaths();
    SOPC_ASSERT("<test_server_file_transfer: Load config from XML failed" && SOPC_STATUS_OK == status);

    // INITIALIZE FILE TRANSFER API :
    SOPC_FileType_Config item1PreloadFile = {.file_path = "",
                                             .fileType_nodeId = "ns=1;i=15478",
                                             .met_openId = "ns=1;i=15484",
                                             .met_closeId = "ns=1;i=15487",
                                             .met_readId = "ns=1;i=15489",
                                             .met_writeId = "ns=1;i=15492",
                                             .met_getposId = "ns=1;i=15494",
                                             .met_setposId = "ns=1;i=15497",
                                             .var_sizeId = "ns=1;i=15479",
                                             .var_openCountId = "ns=1;i=15482",
                                             .var_userWritableId = "ns=1;i=15481",
                                             .var_writableId = "ns=1;i=15480",
                                             .pFunc_UserCloseCallback = &UserCloseCallback};

    SOPC_FileType_Config item2PreloadFile = {.file_path = "",
                                             .fileType_nodeId = "ns=1;i=15529",
                                             .met_openId = "ns=1;i=15535",
                                             .met_closeId = "ns=1;i=15538",
                                             .met_readId = "ns=1;i=15540",
                                             .met_writeId = "ns=1;i=15543",
                                             .met_getposId = "ns=1;i=15545",
                                             .met_setposId = "ns=1;i=15548",
                                             .var_sizeId = "ns=1;i=15530",
                                             .var_openCountId = "ns=1;i=15533",
                                             .var_userWritableId = "ns=1;i=15532",
                                             .var_writableId = "ns=1;i=15531",
                                             .pFunc_UserCloseCallback = &UserCloseCallback};

    // Store the FileTransfer temporary files.
    size_t size_tmp_path = strlen(gLogDirPath) + strlen(ITEM_1_FILE_NAME) + 1; // +1 \0
    char* tmp_file_path_item1 = SOPC_Calloc(size_tmp_path, sizeof(char));
    char* tmp_file_path_item2 = SOPC_Calloc(size_tmp_path, sizeof(char));
    SOPC_ASSERT("<test_server_file_transfer: calloc function failed for item1" && NULL != tmp_file_path_item1);
    SOPC_ASSERT("<test_server_file_transfer: calloc function failed for item2" && NULL != tmp_file_path_item2);
    int res_1 = snprintf(tmp_file_path_item1, size_tmp_path, "%s%s", gLogDirPath, ITEM_1_FILE_NAME);
    int res_2 = snprintf(tmp_file_path_item2, size_tmp_path, "%s%s", gLogDirPath, ITEM_2_FILE_NAME);
    SOPC_ASSERT("<test_server_file_transfer: snprintf function has failed for item1" && 0 <= res_1);
    SOPC_ASSERT("<test_server_file_transfer: snprintf function has failed for item2" && 0 <= res_2);
    item1PreloadFile.file_path = tmp_file_path_item1;
    item2PreloadFile.file_path = tmp_file_path_item2;

    status = SOPC_FileTransfer_Initialize();
    SOPC_ASSERT("<test_server_file_transfer: Intialization failed" && status == SOPC_STATUS_OK);
    status = SOPC_FileTransfer_Add_File(&item1PreloadFile);
    SOPC_ASSERT("<test_server_file_transfer: Add g_item1PreloadFile failed" && status == SOPC_STATUS_OK);
    status = SOPC_FileTransfer_Add_File(&item2PreloadFile);
    SOPC_ASSERT("<test_server_file_transfer: Add g_item2PreloadFile failed" && status == SOPC_STATUS_OK);

    SOPC_Free(tmp_file_path_item1);
    SOPC_Free(tmp_file_path_item2);

    // START SERVER :
    status = SOPC_FileTransfer_StartServer();
    SOPC_ASSERT("<test_server_file_transfer: Server startup failed" && status == SOPC_STATUS_OK);

    while (SOPC_FileTransfer_IsServerRunning())
    {
        SOPC_Sleep(1);
    }

    SOPC_Free(gLogDirPath);

    SOPC_FileTransfer_Clear();
    printf("<test_server_file_transfer: Server ended to serve\n");
    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
