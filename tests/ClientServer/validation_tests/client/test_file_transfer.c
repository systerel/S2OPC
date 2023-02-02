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

#include <check.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "test_file_transfer_method.h"

#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_file_transfer.h"
#include "sopc_filesystem.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"

#define STR_MARGIN_SIZE 50u // To create a C string for temporary file paths
#define NB_NODE_ATTR_TO_READ 1u

// Bit masks value for opening mode
#define READ_MASK 0x01u
#define WRITE_MASK 0x02u
#define ERASE_EXISTING_MASK 0x04u
#define APPEND_MASK 0x08u
#define INVALID_MODE_BIT_4 0x10 // bit 4:7 are reserved for future use

// Initialization values of variables for FileType object
#define VAR_USER_WRITABLE_INIT true
#define VAR_WRITABLE_INIT true
#define VAR_OPENCOUNT_INIT 0u
#define VAR_SIZE_INIT 0u

#define START_OF_FILE_POS 0u

/*---------------------------------------------------------------------------
 *                          Global variables for callbacks
 *---------------------------------------------------------------------------*/

static int32_t g_connectionClosed = false;
static int32_t g_endpointClosed = false;
SOPC_Boolean g_booleanNotification = false;
SOPC_Boolean g_booleanUserCloseCallback = false;

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

// Server stop callback
static void ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_FileTransfer_Clear();
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: Server stopped\n");
    SOPC_Atomic_Int_Set(&g_endpointClosed, true);
}

// Client disconnect callback
static void disconnect_callback(const uint32_t c_id)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_Atomic_Int_Set(&g_connectionClosed, true);
}

// Close file CallBack
static void(UserCloseCallback)(const char* tmp_file_path)
{
    g_booleanUserCloseCallback = true;
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: tmp file path name: '%s'\n",
                           tmp_file_path);

    // Set .log extension to keep file in artifacts (CI)
    size_t size_tmp_path = strlen(tmp_file_path) + STR_MARGIN_SIZE;
    char* new_name = SOPC_Calloc(size_tmp_path, sizeof(char));
    ck_assert("In closing callback: failed to set .log ext." && NULL != new_name);
    ck_assert("In closing callback, failed to set .log ext." &&
              0 <= snprintf(new_name, size_tmp_path, "%s.log", tmp_file_path));
    ck_assert("In closing callback, failed to set .log ext." && 0 == rename(tmp_file_path, new_name));
    SOPC_Free(new_name);
}

/*
 * User Server callback definition used for address space modification by client.
 */
static void UserWriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                          OpcUa_WriteValue* writeValue,
                                          SOPC_StatusCode writeStatus)
{
    g_booleanNotification = true;
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);

    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Write notification (%s) on node '%s' by user '%s'\n",
                           writeSuccess, sNodeId, SOPC_User_ToCString(user));
    SOPC_Free(sNodeId);
}

/*
 * Method added by the server: SOPC_FileTransfer_Add_MethodItems
 */
static SOPC_StatusCode RemoteExecution_Method_Test(const SOPC_CallContext* callContextPtr,
                                                   const SOPC_NodeId* objectId,
                                                   uint32_t nbInputArgs,
                                                   const SOPC_Variant* inputArgs,
                                                   uint32_t* nbOutputArgs,
                                                   SOPC_Variant** outputArgs,
                                                   void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);

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
        status = OpcUa_BadUnexpectedError;
    }
    g_booleanNotification = true;
    return status;
}

/*---------------------------------------------------------------------------
 *                          Client configuration
 *---------------------------------------------------------------------------*/

static OpcUa_GetEndpointsResponse* g_expectedEndpoints = NULL;

static int32_t client_create_configuration(void)
{
    int32_t res = SOPC_ClientHelper_Initialize(disconnect_callback);
    if (res < 0)
    {
        return res;
    }

    SOPC_ReturnStatus status = SOPC_ClientHelper_SetLocaleIds(2, (const char*[]){"fr-FR", "en-US"});

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                             DEFAULT_APPLICATION_NAME, "fr-FR",
                                                             OpcUa_ApplicationType_Client);
    }

    // Retrieve endpoints from server
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_GetEndpointsRequest* request = SOPC_GetEndpointsRequest_Create(DEFAULT_ENDPOINT_URL);
        if (NULL != request)
        {
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &g_expectedEndpoints);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        return -100;
    }

    SOPC_ClientHelper_Security security = {
        .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
        .security_mode = OpcUa_MessageSecurityMode_Sign,
        .path_cert_auth = "./trusted/cacert.der",
        .path_crl = "./revoked/cacrl.der",
        .path_cert_srv = "./server_public/server_4k_cert.der",
        .path_cert_cli = "./client_public/client_4k_cert.der",
        .path_key_cli = "./client_private/client_4k_key.pem",
        .policyId = "user",
        .username = "me",
        .password = "1234",
    };

    SOPC_ClientHelper_EndpointConnection endpoint = {
        .endpointUrl = DEFAULT_ENDPOINT_URL,
        .serverUri = NULL,
        .reverseConnectionConfigId = 0,
    };

    // connect to the endpoint
    return SOPC_ClientHelper_CreateConfiguration(&endpoint, &security, g_expectedEndpoints);
}

/*---------------------------------------------------------------------------
 *                          Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_LoadServerConfigurationFromPaths(void)
{
    // Server endpoints and PKI configuration
    const char* xml_server_cfg_path = "./S2OPC_Server_Demo_Config.xml";
    // Server address space configuration
    const char* xml_address_space_path = "./ft_data/address_space.xml";
    // User credentials and authorizations
    const char* xml_users_cfg_path = "./S2OPC_Users_Demo_Config.xml";

    return SOPC_HelperConfigServer_ConfigureFromXML(xml_server_cfg_path, xml_address_space_path, xml_users_cfg_path,
                                                    NULL);
}

/*---------------------------------------------------------------------------
 *                          Global variables for test cases
 *---------------------------------------------------------------------------*/

// FileType configuation
SOPC_FileType_Config g_item1PreloadFile = {0};
SOPC_FileType_Config g_item2PreloadFile = {0};

// Client connection Id
int32_t g_coId = 0;

// Client read 4 variables in PreloadFile (item1 or 2):
SOPC_ClientHelper_ReadValue g_readValueItem1 = {.attributeId = SOPC_AttributeId_Value, .indexRange = NULL};
SOPC_ClientHelper_ReadValue g_readValueItem2 = {.attributeId = SOPC_AttributeId_Value, .indexRange = NULL};

// Data which will contain value to read
SOPC_DataValue g_readValSizeItem1 = {0};
SOPC_DataValue g_readValOpenCountItem1 = {0};
SOPC_DataValue g_readValUserWritableItem1 = {0};
SOPC_DataValue g_readValWritableItem1 = {0};
SOPC_DataValue g_readValSizeItem2 = {0};
SOPC_DataValue g_readValOpenCountItem2 = {0};

// Pointer to these values:
SOPC_Boolean* g_pUserWritableItem1 = &g_readValUserWritableItem1.Value.Value.Boolean;
SOPC_Boolean* g_pWritableItem1 = &g_readValWritableItem1.Value.Value.Boolean;
uint64_t* g_pSizeItem1 = &g_readValSizeItem1.Value.Value.Uint64;
uint16_t* g_pOpenCountItem1 = &g_readValOpenCountItem1.Value.Value.Uint16;
uint64_t* g_pSizeItem2 = &g_readValSizeItem2.Value.Value.Uint64;
uint16_t* g_pOpenCountItem2 = &g_readValOpenCountItem2.Value.Value.Uint16;

// S2OPC CallRequest and CallResult structure for item 1 and 2:
SOPC_ClientHelper_CallMethodRequest g_callRequestsItem1 = {0};
SOPC_ClientHelper_CallMethodResult g_callResultsItem1 = {0};
SOPC_ClientHelper_CallMethodRequest g_callRequestsItem2 = {0};
SOPC_ClientHelper_CallMethodResult g_callResultsItem2 = {0};

// API specific test:
const char* g_bin_file_path = "./ft_data/bin_file_TC_33_34.a"; // TC_SOPC_FileTransfer_033 and TC_SOPC_FileTransfer_034
char* g_remoteResetId = "ns=1;i=15789";
char* g_met_remoteResetId = "ns=1;i=15790";
SOPC_ClientHelper_CallMethodRequest g_callRequestsClient = {0};
SOPC_ClientHelper_CallMethodResult g_callResultsClient = {0};

static void configure_client_server(const char* log_path)
{
    // Configure the server to support message size of 128 Mo
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
    ck_assert("Failed to configure message size of S2OPC" && false != res);

    // Get default log config and set the custom path
    SOPC_Log_Configuration log_config = SOPC_Common_GetDefaultLogConfiguration();
    log_config.logLevel = SOPC_LOG_LEVEL_DEBUG;
    log_config.logSysConfig.fileSystemLogConfig.logDirPath = log_path;
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&log_config);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }

    ck_assert("Server intialization failed" && SOPC_STATUS_OK == status);

    // load config from XML file :
    status = Server_LoadServerConfigurationFromPaths();
    ck_assert("Load config from XML failed" && SOPC_STATUS_OK == status);
    status = SOPC_HelperConfigServer_SetWriteNotifCallback(&UserWriteNotificationCallback);
    ck_assert("Unable to configure write notification callback" && SOPC_STATUS_OK == status);

    // INITIALIZE FILE TRANSFER API :
    g_item1PreloadFile.fileType_nodeId = "ns=1;i=15478";
    g_item1PreloadFile.met_openId = "ns=1;i=15484";
    g_item1PreloadFile.met_closeId = "ns=1;i=15487";
    g_item1PreloadFile.met_readId = "ns=1;i=15489";
    g_item1PreloadFile.met_writeId = "ns=1;i=15492";
    g_item1PreloadFile.met_getposId = "ns=1;i=15494";
    g_item1PreloadFile.met_setposId = "ns=1;i=15497";
    g_item1PreloadFile.var_sizeId = "ns=1;i=15479";
    g_item1PreloadFile.var_openCountId = "ns=1;i=15482";
    g_item1PreloadFile.var_userWritableId = "ns=1;i=15481";
    g_item1PreloadFile.var_writableId = "ns=1;i=15480";
    g_item1PreloadFile.pFunc_UserCloseCallback = &UserCloseCallback;

    g_item2PreloadFile.fileType_nodeId = "ns=1;i=15529";
    g_item2PreloadFile.met_openId = "ns=1;i=15535";
    g_item2PreloadFile.met_closeId = "ns=1;i=15538";
    g_item2PreloadFile.met_readId = "ns=1;i=15540";
    g_item2PreloadFile.met_writeId = "ns=1;i=15543";
    g_item2PreloadFile.met_getposId = "ns=1;i=15545";
    g_item2PreloadFile.met_setposId = "ns=1;i=15548";
    g_item2PreloadFile.var_sizeId = "ns=1;i=15530";
    g_item2PreloadFile.var_openCountId = "ns=1;i=15533";
    g_item2PreloadFile.var_userWritableId = "ns=1;i=15532";
    g_item2PreloadFile.var_writableId = "ns=1;i=15531";
    g_item2PreloadFile.pFunc_UserCloseCallback = &UserCloseCallback;

    // Store the FileTransfer temporary files in a sub-folder for each test case.
    size_t size_tmp_path = strlen(log_path) + STR_MARGIN_SIZE;
    char* tmp_file_path_item1 = SOPC_Calloc(size_tmp_path, sizeof(char));
    char* tmp_file_path_item2 = SOPC_Calloc(size_tmp_path, sizeof(char));
    ck_assert("FileTransfer API: calloc function failed for item1" && NULL != tmp_file_path_item1);
    ck_assert("FileTransfer API: calloc function failed for item2" && NULL != tmp_file_path_item2);
    int res_1 = snprintf(tmp_file_path_item1, size_tmp_path, "%sitem1", log_path);
    int res_2 = snprintf(tmp_file_path_item2, size_tmp_path, "%sitem2", log_path);
    ck_assert("FileTransfer API: snprintf function has failed for item1" && 0 <= res_1);
    ck_assert("FileTransfer API: snprintf function has failed for item2" && 0 <= res_2);
    g_item1PreloadFile.file_path = tmp_file_path_item1;
    g_item2PreloadFile.file_path = tmp_file_path_item2;

    status = SOPC_FileTransfer_Initialize();
    ck_assert("FileTransfer API: intialization failed" && status == SOPC_STATUS_OK);
    status = SOPC_FileTransfer_Add_File(&g_item1PreloadFile);
    ck_assert("FileTransfer API: add g_item1PreloadFile failed" && status == SOPC_STATUS_OK);
    status = SOPC_FileTransfer_Add_File(&g_item2PreloadFile);
    ck_assert("FileTransfer API: add g_item2PreloadFile failed" && status == SOPC_STATUS_OK);
    status = SOPC_FileTransfer_Add_MethodItems(&RemoteExecution_Method_Test, "RemoteExecution_Method_Test",
                                               g_met_remoteResetId);
    ck_assert("FileTransfer API: add RemoteExecution_Method_Test failed" && status == SOPC_STATUS_OK);

    SOPC_Free(tmp_file_path_item1);
    SOPC_Free(tmp_file_path_item2);

    // START SERVER :
    status = SOPC_FileTransfer_StartServer(ServerStoppedCallback);
    ck_assert("FileTransfer API: server startup failed" && status == SOPC_STATUS_OK);

    // Create client configuration
    int32_t clientCfgId = -1;
    clientCfgId = client_create_configuration();
    ck_assert("Client configuration failed" && clientCfgId > 0);

    // Connect client to server
    g_coId = SOPC_ClientHelper_CreateConnection(clientCfgId);
    ck_assert("Client configuration failed" && g_coId > 0);

    SOPC_DataValue_Initialize(&g_readValSizeItem1);
    SOPC_DataValue_Initialize(&g_readValOpenCountItem1);
    SOPC_DataValue_Initialize(&g_readValUserWritableItem1);
    SOPC_DataValue_Initialize(&g_readValWritableItem1);
    SOPC_DataValue_Initialize(&g_readValSizeItem2);
    SOPC_DataValue_Initialize(&g_readValOpenCountItem2);

    memset(&g_callRequestsItem1, 0, sizeof(SOPC_ClientHelper_CallMethodRequest));
    memset(&g_callResultsItem1, 0, sizeof(SOPC_ClientHelper_CallMethodResult));
    memset(&g_callRequestsItem2, 0, sizeof(SOPC_ClientHelper_CallMethodRequest));
    memset(&g_callResultsItem2, 0, sizeof(SOPC_ClientHelper_CallMethodResult));

    memset(&g_callRequestsClient, 0, sizeof(SOPC_ClientHelper_CallMethodRequest));
    memset(&g_callResultsClient, 0, sizeof(SOPC_ClientHelper_CallMethodResult));
    g_callRequestsClient.objectNodeId = g_remoteResetId;
    g_callRequestsClient.methodNodeId = g_met_remoteResetId;

    // Node ID of the object on which method are called
    g_callRequestsItem1.objectNodeId = g_item1PreloadFile.fileType_nodeId;
    g_callRequestsItem2.objectNodeId = g_item2PreloadFile.fileType_nodeId;
}

static inline void teardown_client_server(void)
{
    SOPC_ClientHelper_Disconnect(g_coId);
    SOPC_ClientHelper_Finalize();
    SOPC_FileTransfer_Clear();
}

/*---------------------------------------------------------------------------
 *                   File Transfer Test
 *---------------------------------------------------------------------------*/
static inline void setup_001(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_001/");
}
START_TEST(TC_SOPC_FileTransfer_001)
{
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_writableId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValWritableItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_userWritableId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValUserWritableItem1);
    ck_assert(VAR_SIZE_INIT == *g_pSizeItem1 && VAR_OPENCOUNT_INIT == *g_pOpenCountItem1);
    ck_assert(VAR_USER_WRITABLE_INIT == *g_pUserWritableItem1 && VAR_WRITABLE_INIT == *g_pWritableItem1);
}
END_TEST

static inline void setup_002(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_002/");
}
START_TEST(TC_SOPC_FileTransfer_002)
{
    int32_t nbOfBytesToRead = 100;
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, nbOfBytesToRead);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
    ck_assert(NULL == g_callResultsItem1.outputParams);
}
END_TEST

static inline void setup_003(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_003/");
}
START_TEST(TC_SOPC_FileTransfer_003)
{
    SOPC_ByteString dataToWrite;
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ByteString_Initialize(&dataToWrite);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4); // Free by the methodCall
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
}
END_TEST

static inline void setup_004(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_004/");
}
START_TEST(TC_SOPC_FileTransfer_004)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                             g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
    ck_assert(NULL == g_callResultsItem1.outputParams);
}
END_TEST

static inline void setup_005(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_005/");
}
START_TEST(TC_SOPC_FileTransfer_005)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint64_t setPositionItem1 = 650;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                             g_item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
}
END_TEST

static inline void setup_006(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_006/");
}
START_TEST(TC_SOPC_FileTransfer_006)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
}
END_TEST

static inline void setup_007(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_007/");
}
START_TEST(TC_SOPC_FileTransfer_007)
{
    SOPC_Byte mode = INVALID_MODE_BIT_4; // bits 4:7 are reserved for future use
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(VAR_OPENCOUNT_INIT == *g_pOpenCountItem1 && INVALID_FILE_HANDLE == fileHandleItem1);
}
END_TEST

static inline void setup_008(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_008/");
}
START_TEST(TC_SOPC_FileTransfer_008)
{
    SOPC_Byte mode = ERASE_EXISTING_MASK; // This bit can only be set if the file is opened for writing
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(VAR_OPENCOUNT_INIT == *g_pOpenCountItem1 && INVALID_FILE_HANDLE == fileHandleItem1);
}
END_TEST

static inline void setup_009(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_009/");
}
START_TEST(TC_SOPC_FileTransfer_009)
{
    SOPC_Byte mode = READ_MASK; // Reading
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    uint32_t tmpFileHandle = fileHandleItem1;
    mode = APPEND_MASK; // Try to open a second time with a different mode (writing into appening)
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                        g_item1PreloadFile.met_openId, mode);
    ck_assert(OpcUa_BadNotWritable == g_callResultsItem1.status);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(1 == *g_pOpenCountItem1 && INVALID_FILE_HANDLE == fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, tmpFileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_010(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_010/");
}
START_TEST(TC_SOPC_FileTransfer_010)
{
    SOPC_Byte mode = APPEND_MASK | WRITE_MASK; // Writing into appening mode
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    uint32_t tmpFileHandle = fileHandleItem1;
    mode = READ_MASK; // Reading
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                        g_item1PreloadFile.met_openId, mode);
    ck_assert(OpcUa_BadNotReadable == g_callResultsItem1.status);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(1 == *g_pOpenCountItem1 && INVALID_FILE_HANDLE == fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, tmpFileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_011(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_011/");
}
START_TEST(TC_SOPC_FileTransfer_011)
{
    SOPC_ByteString dataToWrite;
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ByteString_Initialize(&dataToWrite); // Free by the methodCall
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_Byte mode = READ_MASK;
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    ck_assert(OpcUa_BadInvalidState == g_callResultsItem1.status);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_012(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_012/");
}
START_TEST(TC_SOPC_FileTransfer_012)
{
    SOPC_Byte mode = WRITE_MASK;
    int32_t nbOfBytesToRead = 100;
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, nbOfBytesToRead);
    ck_assert(OpcUa_BadInvalidState == g_callResultsItem1.status);
    ck_assert(NULL == g_callResultsItem1.outputParams);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}

static inline void setup_013(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_013/");
}
START_TEST(TC_SOPC_FileTransfer_013)
{
    /* This test has to be done manualy:
    The purpose is to verify that only one temporary file is created
    (the first tmp file is deleted during the second open but the two fileHandle's are still valid)*/

    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_Byte mode = READ_MASK; // Reading
    uint32_t firstFileHandle = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    mode = READ_MASK; // try to open a second time with the same mode
    uint32_t newFileHandle = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                               g_item1PreloadFile.met_openId, mode);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(INVALID_FILE_HANDLE != newFileHandle && 1 == *g_pOpenCountItem1);
    // first fileHandle is still valid:
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                            g_item1PreloadFile.met_getposId, firstFileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(START_OF_FILE_POS == pos);
    // close the file with the new fileHandle
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, newFileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}

static inline void setup_014(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_014/");
}
START_TEST(TC_SOPC_FileTransfer_014)
{
    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(
        g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && START_OF_FILE_POS == getPositionItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static void check_015(uint32_t* fileHandle)
{
    SOPC_ByteString dataToWrite;
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ByteString_Initialize(&dataToWrite);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4); // Free by the methodCall
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    // Size is updated only when the read method is called
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && VAR_SIZE_INIT == *g_pSizeItem1);

    *fileHandle = fileHandleItem1;
}

static inline void setup_015(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_015/");
}
START_TEST(TC_SOPC_FileTransfer_015)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_015(&fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_016(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_016/");
}
START_TEST(TC_SOPC_FileTransfer_016)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_015(&fileHandleItem1);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(
        g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && 4 == getPositionItem1);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    // Size is updated only when the read method is called
    ck_assert(VAR_SIZE_INIT == *g_pSizeItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_017(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_017/");
}
START_TEST(TC_SOPC_FileTransfer_017)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_015(&fileHandleItem1);

    // Current position = 4
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                            g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && 4 == pos);

    int32_t nbOfBytesToRead = 2;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, nbOfBytesToRead);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    SOPC_VariantValue* pVariantOutput = &g_callResultsItem1.outputParams->Value;
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    // Size is updated after the read methodl call
    ck_assert(4 == *g_pSizeItem1 && NULL == pVariantOutput->Bstring.Data);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static void check_018(uint32_t* fileHandle)
{
    check_015(fileHandle);

    // Current position = 4
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                            g_item1PreloadFile.met_getposId, *fileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && 4 == pos);

    // Set position = 2
    uint64_t setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                             g_item1PreloadFile.met_setposId, *fileHandle, setPositionItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
static inline void setup_018(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_018/");
}
START_TEST(TC_SOPC_FileTransfer_018)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_018(&fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_019(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_019/");
}
START_TEST(TC_SOPC_FileTransfer_019)
{
    int32_t nbOfBytesToRead = -1;
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_018(&fileHandleItem1);

    // Current position = 2
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                            g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && 2 == pos);

    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, nbOfBytesToRead);
    ck_assert(OpcUa_BadInvalidArgument == g_callResultsItem1.status);
    ck_assert(NULL == g_callResultsItem1.outputParams);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_020(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_020/");
}
START_TEST(TC_SOPC_FileTransfer_020)
{
    int32_t nbOfBytesToRead = 2; // "CD"
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_018(&fileHandleItem1);

    SOPC_ByteString* dataToCompare = SOPC_ByteString_Create();
    ck_assert(NULL != dataToCompare);
    uint8_t CDString[2] = {0x43, 0x44};
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(dataToCompare, CDString, 2);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    // Current position = 2
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                            g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && 2 == pos);

    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, nbOfBytesToRead);
    SOPC_VariantValue* pVariantOutput = &g_callResultsItem1.outputParams->Value;
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, dataToCompare));
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);

    SOPC_ByteString_Delete(dataToCompare);
}
END_TEST

static inline void check_021(uint32_t* fileHandle)
{
    check_018(fileHandle);
    SOPC_ByteString toWrite;
    uint8_t EString[1] = {0x45};
    SOPC_ByteString_Initialize(&toWrite);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&toWrite, EString, 1); // Free by the methodCall
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_writeId, *fileHandle, &toWrite);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}

static inline void setup_021(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_021/");
}
START_TEST(TC_SOPC_FileTransfer_021)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_021(&fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static void check_022(uint32_t* fileHandle)
{
    check_021(fileHandle);

    uint64_t setPositionItem1 = 2;
    uint8_t estring[1] = {0x45}; // E
    SOPC_ByteString* toCompare = SOPC_ByteString_Create();
    ck_assert(NULL != toCompare);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(toCompare, estring, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    // Current position = 3
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                            g_item1PreloadFile.met_getposId, *fileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status && 3 == pos);

    // Set position = 2
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                             g_item1PreloadFile.met_setposId, *fileHandle, setPositionItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      *fileHandle, 1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    SOPC_VariantValue* pVariantOutput = &g_callResultsItem1.outputParams->Value;
    ck_assert(true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, toCompare));

    SOPC_ByteString_Delete(toCompare);
}
static inline void setup_022(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_022/");
}
START_TEST(TC_SOPC_FileTransfer_022)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_022(&fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static inline void setup_023(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_023/");
}
START_TEST(TC_SOPC_FileTransfer_023)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_022(&fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    // Size variable is reset only if the file is open again
    ck_assert(4 == *g_pSizeItem1 && VAR_OPENCOUNT_INIT == *g_pOpenCountItem1);
}
END_TEST

static inline void setup_024(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_024/");
}
START_TEST(TC_SOPC_FileTransfer_024)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    check_022(&fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);

    // Size variable is reset only if the file is open again
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    ck_assert(4 == *g_pSizeItem1);

    SOPC_Byte mode = READ_MASK; // Reading
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                        g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    // The Size variable is reset
    ck_assert(VAR_SIZE_INIT == *g_pSizeItem1 && 1 == *g_pOpenCountItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
END_TEST

static void check_025(uint32_t* fileHandleItem1, uint32_t* fileHandleItem2)
{
    SOPC_ByteString dataWriteItem1;
    SOPC_ByteString_Initialize(&dataWriteItem1);
    uint8_t stringItem1[8] = {0x70, 0x72, 0x65, 0x6C, 0x6F, 0x61, 0x64, 0x31}; //  stringItem1 = "preload1"
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataWriteItem1, stringItem1, 8); // Free by the methodCall
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_ByteString dataWriteItem2;
    SOPC_ByteString_Initialize(&dataWriteItem2);
    uint8_t stringItem2[5] = {0x49, 0x74, 0x65, 0x6D, 0x32};                 // stringItem2 = "Item2"
    status = SOPC_ByteString_CopyFromBytes(&dataWriteItem2, stringItem2, 5); // Free by the methodCall
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    *fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                         g_item1PreloadFile.met_openId, mode);
    *fileHandleItem2 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                                         g_item2PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_writeId, *fileHandleItem1, &dataWriteItem1);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                       g_item2PreloadFile.met_writeId, *fileHandleItem2, &dataWriteItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);

    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    g_readValueItem2.nodeId = g_item2PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem2, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem2);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    g_readValueItem2.nodeId = g_item2PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem2, NB_NODE_ATTR_TO_READ, &g_readValSizeItem2);
    ck_assert(INVALID_FILE_HANDLE != *fileHandleItem1);
    ck_assert(INVALID_FILE_HANDLE != *fileHandleItem2);
    ck_assert(VAR_SIZE_INIT == *g_pSizeItem1 && 1 == *g_pOpenCountItem1);
    ck_assert(VAR_SIZE_INIT == *g_pSizeItem2 && 1 == *g_pOpenCountItem2);
}
static inline void setup_025(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_025/");
}
START_TEST(TC_SOPC_FileTransfer_025)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    check_025(&fileHandleItem1, &fileHandleItem2);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                       g_item2PreloadFile.met_closeId, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
}
END_TEST

static inline void setup_026(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_026/");
}
START_TEST(TC_SOPC_FileTransfer_026)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    check_025(&fileHandleItem1, &fileHandleItem2);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(
        g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_getposId, fileHandleItem1);
    uint64_t getPositionItem2 = SOPC_TEST_FileTransfer_GetPositionMethod(
        g_coId, &g_callRequestsItem2, &g_callResultsItem2, g_item2PreloadFile.met_getposId, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
    ck_assert(8 == getPositionItem1); // End of file for item1
    ck_assert(5 == getPositionItem2); // End of file for item2
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                       g_item2PreloadFile.met_closeId, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
}
END_TEST

static void check_027(uint32_t* fileHandleItem1, uint32_t* fileHandleItem2)
{
    check_025(fileHandleItem1, fileHandleItem2);
    uint64_t setPositionItem1 = 3;
    uint64_t setPositionItem2 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                             g_item1PreloadFile.met_setposId, *fileHandleItem1, setPositionItem1);
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                             g_item2PreloadFile.met_setposId, *fileHandleItem2, setPositionItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
}
static inline void setup_027(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_027/");
}
START_TEST(TC_SOPC_FileTransfer_027)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    check_027(&fileHandleItem1, &fileHandleItem2);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                       g_item2PreloadFile.met_closeId, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
}
END_TEST

static inline void setup_028(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_028/");
}
START_TEST(TC_SOPC_FileTransfer_028)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    check_027(&fileHandleItem1, &fileHandleItem2);

    int32_t nbOfBytesToRead = 4;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, nbOfBytesToRead);
    nbOfBytesToRead = 3;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2, g_item2PreloadFile.met_readId,
                                      fileHandleItem2, nbOfBytesToRead);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);

    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem2, NB_NODE_ATTR_TO_READ, &g_readValSizeItem2);

    uint8_t loadString[4] = {0x6C, 0x6F, 0x61, 0x64}; // "load"
    uint8_t em2String[3] = {0x65, 0x6D, 0x32};        // "em2"
    SOPC_ByteString* dataToCompareItem1 = SOPC_ByteString_Create();
    SOPC_ByteString* dataToCompareItem2 = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(dataToCompareItem1, loadString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    status = SOPC_ByteString_CopyFromBytes(dataToCompareItem2, em2String, 3);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    SOPC_VariantValue* pVariantOutputItem1 = &g_callResultsItem1.outputParams->Value;
    SOPC_VariantValue* pVariantOutputItem2 = &g_callResultsItem2.outputParams->Value;

    ck_assert(true == SOPC_ByteString_Equal(&pVariantOutputItem1->Bstring, dataToCompareItem1));
    ck_assert(true == SOPC_ByteString_Equal(&pVariantOutputItem2->Bstring, dataToCompareItem2));
    ck_assert(8 == *g_pSizeItem1 && 5 == *g_pSizeItem2);

    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                       g_item2PreloadFile.met_closeId, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);

    SOPC_ByteString_Delete(dataToCompareItem1);
    SOPC_ByteString_Delete(dataToCompareItem2);
}
END_TEST

static inline void setup_029(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_029/");
}
START_TEST(TC_SOPC_FileTransfer_029)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    check_025(&fileHandleItem1, &fileHandleItem2);
    ck_assert(false == g_booleanUserCloseCallback);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem2, &g_callResultsItem2,
                                       g_item2PreloadFile.met_closeId, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem2.status);
    ck_assert(true == g_booleanUserCloseCallback);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    g_readValueItem2.nodeId = g_item2PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem2, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem2);
    ck_assert(VAR_OPENCOUNT_INIT == *g_pOpenCountItem1 && VAR_OPENCOUNT_INIT == *g_pOpenCountItem2);
}
END_TEST

static inline void setup_030(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_030/");
}
START_TEST(TC_SOPC_FileTransfer_030)
{
    ck_assert(false == g_booleanNotification);
    int32_t clientResultCode = SOPC_ClientHelper_CallMethod(g_coId, &g_callRequestsClient, 1, &g_callResultsClient);
    SOPC_VariantValue* pVariantOutput = &g_callResultsClient.outputParams->Value;
    ck_assert(0 == clientResultCode);
    ck_assert(0 == g_callResultsClient.status);
    ck_assert(true == pVariantOutput->Boolean);
    ck_assert(true == g_booleanNotification); // Check if callback notification is called
}
END_TEST

static inline void setup_031(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_031/");
}
START_TEST(TC_SOPC_FileTransfer_031)
{
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                                 g_item1PreloadFile.met_openId, 3);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(false == g_booleanUserCloseCallback);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(true == g_booleanUserCloseCallback);
}
END_TEST

static void check_033(uint32_t* fileHandle, SOPC_ByteString** ppDataToCompare)
{
    FILE* bin_file_fp = fopen(g_bin_file_path, "rb+");
    ck_assert("The fopen function has failed (./ft_data/bin_file_TC_33_34.a may missing)" && NULL != bin_file_fp);
    int filedes = fileno(bin_file_fp);
    ck_assert("The fileno function has failed (./ft_data/bin_file_TC_33_34.a)" && -1 != filedes);
    struct stat sb;
    int ret = fstat(filedes, &sb);
    ck_assert("The fstat function has failed (./ft_data/bin_file_TC_33_34.a)" && -1 != ret);

    SOPC_ByteString bin_buffer_write;
    bin_buffer_write.Length = (int32_t) sb.st_size;
    bin_buffer_write.Data = SOPC_Malloc((size_t) bin_buffer_write.Length);
    ck_assert(NULL != bin_buffer_write.Data);
    size_t read_count = fread(bin_buffer_write.Data, 1, (size_t) bin_buffer_write.Length, bin_file_fp);

    ck_assert("The fread function has failed (./ft_data/bin_file_TC_33_34.a)" &&
              read_count == (size_t) bin_buffer_write.Length);
    // bin_buffer_write is clear by the after the callRequests
    SOPC_ByteString* buffer = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = SOPC_ByteString_Copy(buffer, &bin_buffer_write);
    ck_assert(NULL != buffer);
    ck_assert(SOPC_STATUS_OK == status);
    *ppDataToCompare = buffer;
    ret = fclose(bin_file_fp);
    ck_assert("The fclose function has failed (./ft_data/bin_file_TC_33_34.a)" && 0 == ret);
    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    *fileHandle = SOPC_TEST_FileTransfer_OpenMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                                    g_item1PreloadFile.met_openId, mode);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(
        g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_getposId, *fileHandle);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(VAR_SIZE_INIT == *g_pSizeItem1);
    ck_assert(1 == *g_pOpenCountItem1);
    ck_assert(START_OF_FILE_POS == getPositionItem1);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_writeId, *fileHandle, &bin_buffer_write);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
}
static inline void setup_033(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_033/");
}
START_TEST(TC_SOPC_FileTransfer_033)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_ByteString* buffer = NULL;
    check_033(&fileHandleItem1, &buffer);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    SOPC_ByteString_Delete(buffer);
}
END_TEST

static inline void setup_034(void)
{
    configure_client_server("./toolkit_test_file_transfer_logs/TC_SOPC_FileTransfer_034/");
}
START_TEST(TC_SOPC_FileTransfer_034)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_ByteString* ppDataToCompare = NULL;
    check_033(&fileHandleItem1, &ppDataToCompare);

    uint64_t setPositionItem1 = 0;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                             g_item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(
        g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    ck_assert(START_OF_FILE_POS == getPositionItem1);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1, g_item1PreloadFile.met_readId,
                                      fileHandleItem1, ppDataToCompare->Length);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);
    SOPC_VariantValue* pVariantOutput = &g_callResultsItem1.outputParams->Value;
    ck_assert(true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, ppDataToCompare));

    g_readValueItem1.nodeId = g_item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValSizeItem1);
    g_readValueItem1.nodeId = g_item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(g_coId, &g_readValueItem1, NB_NODE_ATTR_TO_READ, &g_readValOpenCountItem1);
    ck_assert(ppDataToCompare->Length == (int32_t) *g_pSizeItem1 && 1 == *g_pOpenCountItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, &g_callRequestsItem1, &g_callResultsItem1,
                                       g_item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == g_callResultsItem1.status);

    SOPC_ByteString_Delete(ppDataToCompare);
}
END_TEST

static Suite* tests_file_transfer(void)
{
    Suite* s;
    TCase *tc_001 = NULL, *tc_002 = NULL, *tc_003 = NULL, *tc_004 = NULL, *tc_005 = NULL, *tc_006 = NULL,
          *tc_007 = NULL, *tc_008 = NULL, *tc_009 = NULL, *tc_010 = NULL, *tc_011 = NULL, *tc_012 = NULL,
          *tc_013 = NULL, *tc_014 = NULL, *tc_015 = NULL, *tc_016 = NULL, *tc_017 = NULL, *tc_018 = NULL,
          *tc_019 = NULL, *tc_020 = NULL, *tc_021 = NULL, *tc_022 = NULL, *tc_023 = NULL, *tc_024 = NULL,
          *tc_025 = NULL, *tc_026 = NULL, *tc_027 = NULL, *tc_028 = NULL, *tc_029 = NULL, *tc_030 = NULL,
          *tc_031 = NULL, *tc_033 = NULL, *tc_034 = NULL;

    s = suite_create("Test file transfer");
    tc_001 = tcase_create("Test service");
    tc_002 = tcase_create("Test service");
    tc_003 = tcase_create("Test service");
    tc_004 = tcase_create("Test service");
    tc_005 = tcase_create("Test service");
    tc_006 = tcase_create("Test service");
    tc_007 = tcase_create("Test service");
    tc_008 = tcase_create("Test service");
    tc_009 = tcase_create("Test service");
    tc_010 = tcase_create("Test service");
    tc_011 = tcase_create("Test service");
    tc_012 = tcase_create("Test service");
    tc_013 = tcase_create("Test service");
    tc_014 = tcase_create("Test service");
    tc_015 = tcase_create("Test service");
    tc_016 = tcase_create("Test service");
    tc_017 = tcase_create("Test service");
    tc_018 = tcase_create("Test service");
    tc_019 = tcase_create("Test service");
    tc_020 = tcase_create("Test service");
    tc_021 = tcase_create("Test service");
    tc_022 = tcase_create("Test service");
    tc_023 = tcase_create("Test service");
    tc_024 = tcase_create("Test service");
    tc_025 = tcase_create("Test service");
    tc_026 = tcase_create("Test service");
    tc_027 = tcase_create("Test service");
    tc_028 = tcase_create("Test service");
    tc_029 = tcase_create("Test service");
    tc_030 = tcase_create("Test API");
    tc_031 = tcase_create("Test API");
    tc_033 = tcase_create("Test API");
    tc_034 = tcase_create("Test API");

    suite_add_tcase(s, tc_001);
    tcase_add_checked_fixture(tc_001, setup_001, teardown_client_server);
    tcase_add_test(tc_001, TC_SOPC_FileTransfer_001);

    suite_add_tcase(s, tc_002);
    tcase_add_checked_fixture(tc_002, setup_002, teardown_client_server);
    tcase_add_test(tc_002, TC_SOPC_FileTransfer_002);

    suite_add_tcase(s, tc_003);
    tcase_add_checked_fixture(tc_003, setup_003, teardown_client_server);
    tcase_add_test(tc_003, TC_SOPC_FileTransfer_003);

    suite_add_tcase(s, tc_004);
    tcase_add_checked_fixture(tc_004, setup_004, teardown_client_server);
    tcase_add_test(tc_004, TC_SOPC_FileTransfer_004);

    suite_add_tcase(s, tc_005);
    tcase_add_checked_fixture(tc_005, setup_005, teardown_client_server);
    tcase_add_test(tc_005, TC_SOPC_FileTransfer_005);

    suite_add_tcase(s, tc_006);
    tcase_add_checked_fixture(tc_006, setup_006, teardown_client_server);
    tcase_add_test(tc_006, TC_SOPC_FileTransfer_006);

    suite_add_tcase(s, tc_007);
    tcase_add_checked_fixture(tc_007, setup_007, teardown_client_server);
    tcase_add_test(tc_007, TC_SOPC_FileTransfer_007);

    suite_add_tcase(s, tc_008);
    tcase_add_checked_fixture(tc_008, setup_008, teardown_client_server);
    tcase_add_test(tc_008, TC_SOPC_FileTransfer_008);

    suite_add_tcase(s, tc_009);
    tcase_add_checked_fixture(tc_009, setup_009, teardown_client_server);
    tcase_add_test(tc_009, TC_SOPC_FileTransfer_009);

    suite_add_tcase(s, tc_010);
    tcase_add_checked_fixture(tc_010, setup_010, teardown_client_server);
    tcase_add_test(tc_010, TC_SOPC_FileTransfer_010);

    suite_add_tcase(s, tc_011);
    tcase_add_checked_fixture(tc_011, setup_011, teardown_client_server);
    tcase_add_test(tc_011, TC_SOPC_FileTransfer_011);

    suite_add_tcase(s, tc_012);
    tcase_add_checked_fixture(tc_012, setup_012, teardown_client_server);
    tcase_add_test(tc_012, TC_SOPC_FileTransfer_012);

    suite_add_tcase(s, tc_013);
    tcase_add_checked_fixture(tc_013, setup_013, teardown_client_server);
    tcase_add_test(tc_013, TC_SOPC_FileTransfer_013);

    suite_add_tcase(s, tc_014);
    tcase_add_checked_fixture(tc_014, setup_014, teardown_client_server);
    tcase_add_test(tc_014, TC_SOPC_FileTransfer_014);

    suite_add_tcase(s, tc_015);
    tcase_add_checked_fixture(tc_015, setup_015, teardown_client_server);
    tcase_add_test(tc_015, TC_SOPC_FileTransfer_015);

    suite_add_tcase(s, tc_016);
    tcase_add_checked_fixture(tc_016, setup_016, teardown_client_server);
    tcase_add_test(tc_016, TC_SOPC_FileTransfer_016);

    suite_add_tcase(s, tc_017);
    tcase_add_checked_fixture(tc_017, setup_017, teardown_client_server);
    tcase_add_test(tc_017, TC_SOPC_FileTransfer_017);

    suite_add_tcase(s, tc_018);
    tcase_add_checked_fixture(tc_018, setup_018, teardown_client_server);
    tcase_add_test(tc_018, TC_SOPC_FileTransfer_018);

    suite_add_tcase(s, tc_019);
    tcase_add_checked_fixture(tc_019, setup_019, teardown_client_server);
    tcase_add_test(tc_019, TC_SOPC_FileTransfer_019);

    suite_add_tcase(s, tc_020);
    tcase_add_checked_fixture(tc_020, setup_020, teardown_client_server);
    tcase_add_test(tc_020, TC_SOPC_FileTransfer_020);

    suite_add_tcase(s, tc_021);
    tcase_add_checked_fixture(tc_021, setup_021, teardown_client_server);
    tcase_add_test(tc_021, TC_SOPC_FileTransfer_021);

    suite_add_tcase(s, tc_022);
    tcase_add_checked_fixture(tc_022, setup_022, teardown_client_server);
    tcase_add_test(tc_022, TC_SOPC_FileTransfer_022);

    suite_add_tcase(s, tc_023);
    tcase_add_checked_fixture(tc_023, setup_023, teardown_client_server);
    tcase_add_test(tc_023, TC_SOPC_FileTransfer_023);

    suite_add_tcase(s, tc_024);
    tcase_add_checked_fixture(tc_024, setup_024, teardown_client_server);
    tcase_add_test(tc_024, TC_SOPC_FileTransfer_024);

    suite_add_tcase(s, tc_025);
    tcase_add_checked_fixture(tc_025, setup_025, teardown_client_server);
    tcase_add_test(tc_025, TC_SOPC_FileTransfer_025);

    suite_add_tcase(s, tc_026);
    tcase_add_checked_fixture(tc_026, setup_026, teardown_client_server);
    tcase_add_test(tc_026, TC_SOPC_FileTransfer_026);

    suite_add_tcase(s, tc_027);
    tcase_add_checked_fixture(tc_027, setup_027, teardown_client_server);
    tcase_add_test(tc_027, TC_SOPC_FileTransfer_027);

    suite_add_tcase(s, tc_028);
    tcase_add_checked_fixture(tc_028, setup_028, teardown_client_server);
    tcase_add_test(tc_028, TC_SOPC_FileTransfer_028);

    suite_add_tcase(s, tc_029);
    tcase_add_checked_fixture(tc_029, setup_029, teardown_client_server);
    tcase_add_test(tc_029, TC_SOPC_FileTransfer_029);

    suite_add_tcase(s, tc_030);
    tcase_add_checked_fixture(tc_030, setup_030, teardown_client_server);
    tcase_add_test(tc_030, TC_SOPC_FileTransfer_030);

    suite_add_tcase(s, tc_031);
    tcase_add_checked_fixture(tc_031, setup_031, teardown_client_server);
    tcase_add_test(tc_031, TC_SOPC_FileTransfer_031);

    suite_add_tcase(s, tc_033);
    tcase_add_checked_fixture(tc_033, setup_033, teardown_client_server);
    tcase_add_test(tc_033, TC_SOPC_FileTransfer_033);

    suite_add_tcase(s, tc_034);
    tcase_add_checked_fixture(tc_034, setup_034, teardown_client_server);
    tcase_add_test(tc_034, TC_SOPC_FileTransfer_034);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

#if SOPC_HAS_FILESYSTEM
    // Set common folder for log
    SOPC_FileSystem_CreationResult res = SOPC_FileSystem_mkdir("./toolkit_test_file_transfer_logs/");
    if (SOPC_FileSystem_Creation_Error_PathAlreadyExists == res)
    {
        fprintf(stderr, "WARNING: path <./toolkit_test_file_transfer_logs/> already exists\n");
    }
    if (SOPC_FileSystem_Creation_Error_PathAlreadyExists != res && SOPC_FileSystem_Creation_OK != res)
    {
        fprintf(stderr, "ERROR: Cannot create log directory <./toolkit_test_file_transfer_logs/>\n");
    }
#else /* SOPC_HAS_FILESYSTEM */
    fprintf(stderr, "ERROR: Cannot use SOPC_LOG_SYSTEM_FILE with SOPC_HAS_FILESYSTEM not set to true \n");
    return -1;
#endif

    sr = srunner_create(tests_file_transfer());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
