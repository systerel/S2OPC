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

#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_file_transfer.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_platform_time.h"

static int32_t gB_file_is_close = false;
static char* gCstr_tmp_path = NULL;
static void set_file_closing_status(SOPC_Boolean res);
static SOPC_Boolean get_file_closing_status(void);
static SOPC_Boolean bEnd = false;
void sigint(int arg);

static void set_file_closing_status(SOPC_Boolean res)
{
    SOPC_Atomic_Int_Set(&gB_file_is_close, res ? true : false);
}

static SOPC_Boolean get_file_closing_status(void)
{
    return SOPC_Atomic_Int_Get(&gB_file_is_close) == 1;
}

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

static SOPC_ReturnStatus Server_LoadServerConfigurationFromPaths(void)
{
    // Server endpoints and PKI configuration
    const char* xml_server_cfg_path = "./data/server_config.xml";
    // Server address space configuration
    const char* xml_address_space_path = "./data/address_space.xml";
    // User credentials and authorizations
    const char* xml_users_cfg_path = "./data/users_config.xml";

    return SOPC_HelperConfigServer_ConfigureFromXML(xml_server_cfg_path, xml_address_space_path, xml_users_cfg_path,
                                                    NULL);
}

static SOPC_StatusCode RemoteExecution_Method_Test(const SOPC_CallContext* callContextPtr,
                                                   const SOPC_NodeId* objectId,
                                                   uint32_t nbInputArgs,
                                                   const SOPC_Variant* inputArgs,
                                                   uint32_t* nbOutputArgs,
                                                   SOPC_Variant** outputArgs,
                                                   void* param)
{
    /********************/
    /* USER CODE BEGING */
    /********************/

    /* avoid unused parameter compiler warning */
    (void) callContextPtr;
    (void) objectId;
    (void) param;
    (void) nbInputArgs;
    (void) inputArgs;
    (void) param;
    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode status = SOPC_GoodGenericStatus;

    SOPC_Variant* v = SOPC_Variant_Create(); // Free by the Method Call Manager
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_Boolean_Id;
        v->Value.Boolean = true;

        *nbOutputArgs = 1;
        *outputArgs = v;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:UserMethod_Test: unable to create a variant");
        status = OpcUa_BadUnexpectedError;
    }

    printf("<toolkit_demo_file_transfer> Test of the user method: successful!\n");

    return status;

    /********************/
    /* END USER CODE   */
    /********************/
}

static void ServerStoppedCallback(SOPC_ReturnStatus status)
{
    (void) status;
    if (NULL != gCstr_tmp_path)
    {
        SOPC_Free(gCstr_tmp_path);
        gCstr_tmp_path = NULL;
    }
    SOPC_FileTransfer_Clear();
    printf("******* Server stopped\n");
}

void sigint(int arg)
{
    (void) arg;
    if (NULL != gCstr_tmp_path)
    {
        SOPC_Free(gCstr_tmp_path);
        gCstr_tmp_path = NULL;
    }
    SOPC_FileTransfer_Clear();
    printf("******* Server stopped\n");
    bEnd = true;
}

/*
 * User Close method callback definition.
 * The callback function shall not do anything blocking or long treatment since it will block any other
 * callback call.
 * After this callback, the path of the tmp_file will be deallocated and the user should copy it.
 */
static void UserCloseCallback(const char* tmp_file_path)
{
    /********************/
    /* USER CODE BEGING */
    /********************/
    if (NULL != gCstr_tmp_path)
    {
        SOPC_Free(gCstr_tmp_path);
        gCstr_tmp_path = NULL;
    }
    gCstr_tmp_path = SOPC_Calloc(strlen(tmp_file_path) + 1, sizeof(char));
    memcpy(gCstr_tmp_path, tmp_file_path, (size_t) strlen(tmp_file_path));
    set_file_closing_status(true);
    /********************/
    /* END USER CODE   */
    /********************/
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

int main(int argc, char* argv[])
{
    printf("******* API test\n");
    signal(SIGINT, sigint);

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
    status = SOPC_CommonHelper_Initialize(&logConfig);
    SOPC_Free(logDirPath);
    logDirPath = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Unable to initialize the S2OPC Server frontend configuration.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* status = Server_LoadServerConfigurationFromFiles(); */
        status = Server_LoadServerConfigurationFromPaths();
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to load configuration from paths:\n");
            printf("******* \t--> need file (relative path where the server is running):\t./data/users_config.xml\n");
            printf("******* \t--> need file (relative path where the server is running):\t./data/server_config.xml\n");
            printf("******* \t--> need file (relative path where the server is running):\t./data/address_space.xml\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetWriteNotifCallback(&UserWriteNotificationCallback);
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to configure the @ space modification notification callback\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileTransfer_Initialize();
        if (SOPC_STATUS_OK != status)
        {
            printf("******* API FileTransfer intialization failed\n");
            return 1; // SOPC_FileTransfer_Initialize free the memory in case of error
        }
    }
    else
    {
        SOPC_FileTransfer_Clear();
        return 1;
    }

    printf("******* Code begin ...\n");

    const SOPC_FileType_Config config_Item1_PreloadFile = {.file_path = "/tmp/Item1_preloadFile",
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

    status = SOPC_FileTransfer_Add_File(&config_Item1_PreloadFile);
    if (SOPC_STATUS_OK != status)
    {
        printf("******* Failed to add file into server\n");
        return 1; // SOPC_FileTransfer_Add_File free the memory in case of error
    }
    printf("******* File added ...\n");
    status =
        SOPC_FileTransfer_Add_MethodItems(&RemoteExecution_Method_Test, "RemoteExecution_Method_Test", "ns=1;i=15790");
    if (SOPC_STATUS_OK != status)
    {
        printf("******* Failed to add UserMethod_Test to the server ...\n");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileTransfer_StartServer(ServerStoppedCallback);
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to start server ...\n");
            SOPC_FileTransfer_Clear();
            return 1;
        }
    }

    /********************/
    /* USER CODE BEGING */
    /********************/
    bool file_is_close = false;

    while (!bEnd)
    {
        file_is_close = get_file_closing_status();
        if (file_is_close)
        {
            printf("<toolkit_demo_file_transfer> Tmp file path name = '%s'\n", gCstr_tmp_path);
            if (NULL != gCstr_tmp_path)
            {
                SOPC_Free(gCstr_tmp_path);
                gCstr_tmp_path = NULL;
            }
            set_file_closing_status(false);
        }
        SOPC_Sleep(500);
    }
    /********************/
    /* END USER CODE   */
    /********************/
    return 0;
}
