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
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"
#define BUFF_SIZE 20u

static int32_t connectionClosed = false;
static int32_t endpointClosed = false;
SOPC_Boolean booleanNotification = false;
SOPC_Boolean booleanUserCloseCallback = false;

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

// Server stop callback
static void ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_FileTransfer_Clear();
#if TEST_DEBUG_FT
    printf("<Test_File_Transfer: Server stopped\n");
#else
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: Server stopped\n");
#endif
    SOPC_Atomic_Int_Set(&endpointClosed, true);
}

// Client disconnect callback
static void disconnect_callback(const uint32_t c_id)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_Atomic_Int_Set(&connectionClosed, true);
}

// Close file CallBack
static void(UserCloseCallback)(const char* tmp_file_path)
{
    booleanUserCloseCallback = true;
#if TEST_DEBUG_FT
    printf("<Test_File_Transfer: tmp file path name: '%s'\n", tmp_file_path);
#else
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: tmp file path name: '%s'\n",
                           tmp_file_path);
#endif
}

/*
 * User Server callback definition used for address space modification by client.
 */
static void UserWriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                          OpcUa_WriteValue* writeValue,
                                          SOPC_StatusCode writeStatus)
{
    booleanNotification = true;
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);

#if TEST_DEBUG_FT
    printf("Write notification (%s) on node '%s' by user '%s'\n", writeSuccess, sNodeId, SOPC_User_ToCString(user));
#else
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Write notification (%s) on node '%s' by user '%s'\n",
                           writeSuccess, sNodeId, SOPC_User_ToCString(user));
#endif
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
    booleanNotification = true;
    return status;
}

/*---------------------------------------------------------------------------
 *                          Client initialization
 *---------------------------------------------------------------------------*/

static OpcUa_GetEndpointsResponse* expectedEndpoints = NULL;

static int32_t client_create_configuration(void)
{
    int32_t res = SOPC_ClientHelper_Initialize(disconnect_callback);
    if (res < 0)
    {
        return res;
    }

    SOPC_ReturnStatus status = SOPC_ClientHelper_SetLocaleIds(2, (char*[]){"fr-FR", "en-US"});

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
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &expectedEndpoints);
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
        .security_policy = SOPC_SecurityPolicy_None_URI, // None, Basic256
        .security_mode = OpcUa_MessageSecurityMode_None, // Sign and Encrypt, None or Sign only
        .path_cert_auth = "./data/trusted/cacert.der",
        .path_crl = "./data/revoked/cacrl.der",
        .path_cert_srv = "./data/server_public/server_2k_cert.der",
        .path_cert_cli = "./data/client_public/client_2k_cert.der",
        .path_key_cli = "./data/client_private/client_2k_key.pem",
        .policyId = "anonymous",
        .username = NULL,
        .password = NULL,
    };

    // connect to the endpoint
    return SOPC_ClientHelper_CreateConfiguration(DEFAULT_ENDPOINT_URL, &security, expectedEndpoints);
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

/*---------------------------------------------------------------------------
 *                   File Transfer Test
 *---------------------------------------------------------------------------*/

START_TEST(test_file_transfer_method)
{
    SOPC_FileType_Config item1PreloadFile = {.file_path = "/tmp/item1",
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

    SOPC_FileType_Config item2PreloadFile = {.file_path = "/tmp/item2",
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

    // Client read 4 variables in PreloadFile (item1 or 2):
    SOPC_ClientHelper_ReadValue readValueItem1 = {.attributeId = SOPC_AttributeId_Value, .indexRange = NULL};
    SOPC_ClientHelper_ReadValue readValueItem2 = {.attributeId = SOPC_AttributeId_Value, .indexRange = NULL};

    // Data which will contain value read
    SOPC_DataValue readValSizeItem1;
    SOPC_DataValue readValOpenCountItem1;
    SOPC_DataValue readValUserWritableItem1;
    SOPC_DataValue readValWritableItem1;
    SOPC_DataValue readValSizeItem2;
    SOPC_DataValue readValOpenCountItem2;
    SOPC_DataValue_Initialize(&readValSizeItem1);
    SOPC_DataValue_Initialize(&readValOpenCountItem1);
    SOPC_DataValue_Initialize(&readValUserWritableItem1);
    SOPC_DataValue_Initialize(&readValWritableItem1);
    SOPC_DataValue_Initialize(&readValSizeItem2);
    SOPC_DataValue_Initialize(&readValOpenCountItem2);

    // Pointer to these values:
    SOPC_Boolean* pUserWritableItem1 = &readValUserWritableItem1.Value.Value.Boolean;
    SOPC_Boolean* pWritableItem1 = &readValWritableItem1.Value.Value.Boolean;
    uint64_t* pSizeItem1 = &readValSizeItem1.Value.Value.Uint64;
    uint16_t* pOpenCountItem1 = &readValOpenCountItem1.Value.Value.Uint16;
    uint64_t* pSizeItem2 = &readValSizeItem2.Value.Value.Uint64;
    uint16_t* pOpenCountItem2 = &readValOpenCountItem2.Value.Value.Uint16;
    size_t nbElements = 1;

    // Method write input parameters:
    SOPC_ByteString dataToWrite;
    SOPC_ByteString_Initialize(&dataToWrite);
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    SOPC_ByteString dataToCompare;
    SOPC_ByteString_Initialize(&dataToCompare);
    uint8_t CDString[2] = {0x43, 0x44};
    status = SOPC_ByteString_CopyFromBytes(&dataToCompare, CDString, 2);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_ByteString dataWriteItem1;
    SOPC_ByteString_Initialize(&dataWriteItem1);
    uint8_t stringItem1[8] = {0x70, 0x72, 0x65, 0x6C, 0x6F, 0x61, 0x64, 0x31};
    status = SOPC_ByteString_CopyFromBytes(&dataWriteItem1, stringItem1, 8);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_ByteString dataWriteItem2;
    SOPC_ByteString_Initialize(&dataWriteItem2);
    uint8_t stringItem2[5] = {0x49, 0x74, 0x65, 0x6D, 0x32};
    status = SOPC_ByteString_CopyFromBytes(&dataWriteItem2, stringItem2, 5);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    // Method Open, Read, GetPosition, SetPosition input parameters:
    SOPC_Byte mode = 0;
    int32_t nbOfBytesToRead = 0;
    uint64_t setPositionItem1 = 0;
    uint64_t setPositionItem2 = 0;
    uint64_t getPositionItem1 = 0;
    uint64_t getPositionItem2 = 0;
    uint32_t fileHandleItem1 = 0;
    uint32_t fileHandleItem2 = 0;
    SOPC_StatusCode statusMethodItem1 = 0;
    SOPC_StatusCode statusMethodItem2 = 0;

    // S2OPC CallRequest and CallResult structure for item 1 and 2:
    SOPC_ClientHelper_CallMethodRequest callRequestsItem1;
    SOPC_ClientHelper_CallMethodResult callResultsItem1;
    SOPC_ClientHelper_CallMethodRequest callRequestsItem2;
    SOPC_ClientHelper_CallMethodResult callResultsItem2;
    memset(&callRequestsItem1, 0, sizeof(SOPC_ClientHelper_CallMethodRequest));
    memset(&callResultsItem1, 0, sizeof(SOPC_ClientHelper_CallMethodResult));
    memset(&callRequestsItem2, 0, sizeof(SOPC_ClientHelper_CallMethodRequest));
    memset(&callResultsItem2, 0, sizeof(SOPC_ClientHelper_CallMethodResult));
    // Node ID of the object on which method are called
    callRequestsItem1.objectNodeId = item1PreloadFile.fileType_nodeId;
    callRequestsItem2.objectNodeId = item2PreloadFile.fileType_nodeId;

    // Fonctional test: PHASE 1: declaration variables (TC_033 and TC_034):
    const char* bin_file_path = "./data/bin_file_TC_33_34.a";
    FILE* bin_file_fp = NULL;
    int filedes = -1;
    int ret = -1;
    size_t read_count = 0;
    int32_t comparison = -1;
    SOPC_ByteString bin_buffer_write;
    SOPC_ByteString bin_buffer_write_tmp;
    SOPC_ByteString bin_buffer_read;
    SOPC_ByteString_Initialize(&bin_buffer_write);
    SOPC_ByteString_Initialize(&bin_buffer_write_tmp);
    SOPC_ByteString_Initialize(&bin_buffer_read);

    // Fonctional test: PHASE 3: declaration variables:
    char* remoteResetId = "ns=1;i=15789";
    char* met_remoteResetId = "ns=1;i=15790";
    SOPC_ClientHelper_WriteValue writeValueClient = {.indexRange = NULL};
    SOPC_DataValue readValClient;
    SOPC_DataValue writeValClient;
    SOPC_DataValue_Initialize(&readValClient);
    SOPC_DataValue_Initialize(writeValueClient.value);
    SOPC_String* pHelloWorldString = &writeValClient.Value.Value.String;
    SOPC_String_Initialize(pHelloWorldString);
    SOPC_String_AttachFromCstring(pHelloWorldString, "Hello World");
    int32_t clientResultCode = 0;

    SOPC_ClientHelper_CallMethodRequest callRequestsClient;
    SOPC_ClientHelper_CallMethodResult callResultsClient;
    memset(&callRequestsClient, 0, sizeof(SOPC_ClientHelper_CallMethodRequest));
    memset(&callResultsClient, 0, sizeof(SOPC_ClientHelper_CallMethodResult));

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
    log_config.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_file_transfer_logs/";
    status = SOPC_CommonHelper_Initialize(&log_config);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_Server_Client: Failed initializing\n");
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: Server initialized\n");
    }

    // load config from XML file :
    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadServerConfigurationFromPaths();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetWriteNotifCallback(&UserWriteNotificationCallback);
    }

    // INITIALIZE FILE TRANSFER API :
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileTransfer_Initialize();
        ck_assert_int_eq(status, SOPC_STATUS_OK);
        status = SOPC_FileTransfer_Add_File(&item1PreloadFile);
        ck_assert_int_eq(status, SOPC_STATUS_OK);
        status = SOPC_FileTransfer_Add_File(&item2PreloadFile);
        ck_assert_int_eq(status, SOPC_STATUS_OK);
        status = SOPC_FileTransfer_Add_MethodItems(&RemoteExecution_Method_Test, "RemoteExecution_Method_Test",
                                                   met_remoteResetId);
        ck_assert_int_eq(status, SOPC_STATUS_OK);
    }

    // START SERVER :
    status = SOPC_FileTransfer_StartServer(ServerStoppedCallback);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: Failed to start server ...\n");
        SOPC_FileTransfer_Clear();
    }

    // Create client configuration
    int32_t clientCfgId = -1;
    if (SOPC_STATUS_OK == status)
    {
        clientCfgId = client_create_configuration();
        if (clientCfgId > 0)
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: created configuration\n");
        }
        else
        {
            status = SOPC_STATUS_NOK;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<Test_File_Transfer: Failed to create configuration\n");
        }
    }

    // Connect client to server
    int32_t coId = 0;
    if (SOPC_STATUS_OK == status)
    {
        coId = SOPC_ClientHelper_CreateConnection(clientCfgId);

        if (coId <= 0)
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "<Test_File_Transfer: failed to establish connection\n");
        }
        else
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: connection established\n");
        }
    }

    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: PHASE 0:\n");

    // TC_SOPC_FileTransfer_001:
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    readValueItem1.nodeId = item1PreloadFile.var_writableId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValWritableItem1);
    readValueItem1.nodeId = item1PreloadFile.var_userWritableId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValUserWritableItem1);
    ck_assert("TC_SOPC_FileTransfer_001" && 0 == *pSizeItem1 && 0 == *pOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_001" && true == *pUserWritableItem1 && true == *pWritableItem1);

    // TC_SOPC_FileTransfer_002:
    nbOfBytesToRead = 100;
    fileHandleItem1 = 1500;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    ck_assert("TC_SOPC_FileTransfer_002" && OpcUa_BadInvalidArgument == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_002" && NULL == callResultsItem1.outputParams);

    // TC_SOPC_FileTransfer_003:
    statusMethodItem1 = SOPC_TEST_FileTransfer_WriteMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    ck_assert("TC_SOPC_FileTransfer_003" && OpcUa_BadInvalidArgument == statusMethodItem1);

    // TC_SOPC_FileTransfer_004:
    SOPC_TEST_FileTransfer_GetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                             item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_004" && OpcUa_BadInvalidArgument == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_004" && NULL == callResultsItem1.outputParams);

    // TC_SOPC_FileTransfer_005:
    setPositionItem1 = 650;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_SetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                 item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    ck_assert("TC_SOPC_FileTransfer_005" && OpcUa_BadInvalidArgument == statusMethodItem1);

    // TC_SOPC_FileTransfer_006:
    statusMethodItem1 = SOPC_TEST_FileTransfer_CloseMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_006" && OpcUa_BadInvalidArgument == statusMethodItem1);

    // TC_SOPC_FileTransfer_007:
    mode = 16;
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_007" && OpcUa_BadInvalidArgument == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_007" && 0 == *pOpenCountItem1 && 0 == fileHandleItem1);

    // TC_SOPC_FileTransfer_008:
    mode = 4;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_008" && OpcUa_BadInvalidArgument == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_008" && 0 == *pOpenCountItem1 && 0 == fileHandleItem1);

    // TC_SOPC_FileTransfer_009:
    mode = 1;
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    uint32_t tmpFileHandle = fileHandleItem1;
    mode = 8; // try to open a second time with a different mode
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_009" && OpcUa_BadNotWritable == callResultsItem1.status);
    ck_assert("TC_SOPC_FileTransfer_009" && 1 == *pOpenCountItem1 && 0 == fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       tmpFileHandle);

    // TC_SOPC_FileTransfer_010:
    mode = 10;
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    tmpFileHandle = fileHandleItem1;
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_010" && OpcUa_BadNotReadable == callResultsItem1.status);
    ck_assert("TC_SOPC_FileTransfer_010" && 1 == *pOpenCountItem1 && 0 == fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       tmpFileHandle);

    // TC_SOPC_FileTransfer_011:
    // need to send again ABCD String into data to avoid double free
    SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    statusMethodItem1 = SOPC_TEST_FileTransfer_WriteMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_011" && OpcUa_BadInvalidState == statusMethodItem1);

    // TC_SOPC_FileTransfer_012:
    mode = 2;
    nbOfBytesToRead = 100;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_012" && OpcUa_BadInvalidState == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_012" && NULL == callResultsItem1.outputParams);

    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: PHASE 1:\n");

    // TC_SOPC_FileTransfer_013:
    /* Manual check to do with UA Expert:
     *      - The old temporary file has been deleted (deviation Alstom)
     *      - A new temporary file was created (deviation Alstom)
     */
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    mode = 1; // try to open a second time with a different mode
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_013" && SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert("TC_SOPC_FileTransfer_013" && 0 != fileHandleItem1 && 1 == *pOpenCountItem1);

    // TC_SOPC_FileTransfer_014:
    mode = 3;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                                item1PreloadFile.met_getposId, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_014" && SOPC_GoodGenericStatus == callResultsItem1.status && 0 == getPositionItem1);

    // TC_SOPC_FileTransfer_015:
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    mode = 3;
    SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    statusMethodItem1 = SOPC_TEST_FileTransfer_WriteMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    ck_assert("TC_SOPC_FileTransfer_015" && SOPC_GoodGenericStatus == statusMethodItem1 && 0 == *pSizeItem1);

    // TC_SOPC_FileTransfer_016:
    getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                                item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_016" && SOPC_GoodGenericStatus == callResultsItem1.status && 4 == getPositionItem1);

    // TC_SOPC_FileTransfer_017:
    nbOfBytesToRead = 4;
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    SOPC_VariantValue* pVariantOutput = &callResultsItem1.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_017" && SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert("TC_SOPC_FileTransfer_017" && 4 == *pSizeItem1 && NULL == pVariantOutput->Bstring.Data);

    // TC_SOPC_FileTransfer_018:
    setPositionItem1 = 2;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_SetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                 item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    ck_assert("TC_SOPC_FileTransfer_018" && SOPC_GoodGenericStatus == statusMethodItem1);

    // TC_SOPC_FileTransfer_019:
    nbOfBytesToRead = -1;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    ck_assert("TC_SOPC_FileTransfer_019" && OpcUa_BadInvalidArgument == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_019" && NULL == callResultsItem1.outputParams);

    // TC_SOPC_FileTransfer_020:
    nbOfBytesToRead = 2; // "CD"
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    pVariantOutput = &callResultsItem1.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_020" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_020" && true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));

    // TC_SOPC_FileTransfer_021:
    uint8_t EString[1] = {0x45};
    SOPC_ByteString_CopyFromBytes(&dataToWrite, EString, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                             item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    statusMethodItem1 = SOPC_TEST_FileTransfer_WriteMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_writeId, fileHandleItem1, &dataToWrite);
    ck_assert("TC_SOPC_FileTransfer_021" && SOPC_GoodGenericStatus == statusMethodItem1);

    // TC_SOPC_FileTransfer_022:
    setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                             item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    nbOfBytesToRead = 1; // "E"
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    status = SOPC_ByteString_CopyFromBytes(&dataToCompare, EString, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    pVariantOutput = &callResultsItem1.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_022" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_022" && true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));

    // TC_SOPC_FileTransfer_023:
    SOPC_TEST_FileTransfer_CloseMethod(coId, false, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       fileHandleItem1);
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_023" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_023" && 4 == *pSizeItem1 && 0 == *pOpenCountItem1);

    // TC_SOPC_FileTransfer_024:
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_024" && 0 == *pSizeItem1 && 1 == *pOpenCountItem1);
    SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1, item1PreloadFile.met_closeId,
                                       fileHandleItem1);

    // TC_SOPC_FileTransfer_033:
    /* Manual check to do with UA Expert
        - Call the close method and manually open the tmp_path that was created.
        - Check that the data written are the expected ones.
    */
    SOPC_TEST_FileTransfer_SetTestCaseNumber(33);

    bin_file_fp = fopen(bin_file_path, "rb+");
    ck_assert("TC_SOPC_FileTransfer_033: the fopen function has failed (./data/bin_file_TC_33_34.a may missing)" &&
              NULL != bin_file_fp);
    filedes = fileno(bin_file_fp);
    ck_assert("TC_SOPC_FileTransfer_033: the fileno function has failed (./data/bin_file_TC_33_34.a)" && -1 != filedes);
    struct stat sb;
    ret = fstat(filedes, &sb);
    ck_assert("TC_SOPC_FileTransfer_033: the fstat function has failed (./data/bin_file_TC_33_34.a)" && -1 != ret);
    bin_buffer_write.Length = (int32_t) sb.st_size;
    bin_buffer_write.Data = SOPC_Malloc((size_t) bin_buffer_write.Length);
    read_count = fread(bin_buffer_write.Data, 1, (size_t) bin_buffer_write.Length, bin_file_fp);
    ck_assert("TC_SOPC_FileTransfer_033: the fread function has failed (./data/bin_file_TC_33_34.a)" &&
              read_count == (size_t) bin_buffer_write.Length);
    // bin_buffer_write is clear by the after the callRequests
    SOPC_ByteString_Copy(&bin_buffer_write_tmp, &bin_buffer_write);
    ret = fclose(bin_file_fp);
    ck_assert("TC_SOPC_FileTransfer_033: the fclose function has failed (./data/bin_file_TC_33_34.a)" && 0 == ret);
    mode = 3;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                                item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_033" && 0 == *pSizeItem1);
    ck_assert("TC_SOPC_FileTransfer_033" && 1 == *pOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_033" && 0 == getPositionItem1);
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_WriteMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                           item1PreloadFile.met_writeId, fileHandleItem1, &bin_buffer_write);
    ck_assert("TC_SOPC_FileTransfer_033" && SOPC_GoodGenericStatus == statusMethodItem1);

    // TC_SOPC_FileTransfer_034:
    /* Manual check to do with UA Expert
        - Check that the data read are the expected ones.
    */
    setPositionItem1 = 0;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_SetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                 item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);
    ck_assert("TC_SOPC_FileTransfer_034" && SOPC_GoodGenericStatus == statusMethodItem1);
    getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                                item1PreloadFile.met_getposId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_034" && 0 == getPositionItem1);
    nbOfBytesToRead = bin_buffer_write.Length;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);
    pVariantOutput = &callResultsItem1.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_034" && SOPC_GoodGenericStatus == callResultsItem1.status);
    bin_buffer_read = pVariantOutput->Bstring;
    SOPC_ByteString_Compare(&bin_buffer_write_tmp, &bin_buffer_read, &comparison);
    ck_assert("TC_SOPC_FileTransfer_034" && 0 == comparison);
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_034" && bin_buffer_write_tmp.Length == (int32_t) *pSizeItem1);
    ck_assert("TC_SOPC_FileTransfer_034" && 1 == *pOpenCountItem1);
    statusMethodItem1 = SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_034" && SOPC_GoodGenericStatus == callResultsItem1.status);

    SOPC_Free(bin_buffer_write_tmp.Data);
    SOPC_ByteString_Initialize(&bin_buffer_write_tmp);

    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: PHASE 2:\n");

    // TC_SOPC_FileTransfer_025:
    SOPC_TEST_FileTransfer_SetTestCaseNumber(25);
    mode = 3;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    fileHandleItem2 = SOPC_TEST_FileTransfer_OpenMethod(coId, true, &callRequestsItem2, &callResultsItem2,
                                                        item2PreloadFile.met_openId, mode);

    statusMethodItem1 =
        SOPC_TEST_FileTransfer_WriteMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                           item1PreloadFile.met_writeId, fileHandleItem1, &dataWriteItem1);

    statusMethodItem2 =
        SOPC_TEST_FileTransfer_WriteMethod(coId, true, &callRequestsItem2, &callResultsItem2,
                                           item2PreloadFile.met_writeId, fileHandleItem2, &dataWriteItem2);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    readValueItem2.nodeId = item2PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    SOPC_ClientHelper_Read(coId, &readValueItem2, nbElements, &readValOpenCountItem2);
    readValueItem1.nodeId = item1PreloadFile.var_sizeId;
    readValueItem2.nodeId = item2PreloadFile.var_sizeId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    SOPC_ClientHelper_Read(coId, &readValueItem2, nbElements, &readValSizeItem2);
    ck_assert("TC_SOPC_FileTransfer_025" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_025" && SOPC_GoodGenericStatus == statusMethodItem2);
    ck_assert("TC_SOPC_FileTransfer_025" && 0 != fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_025" && 0 != fileHandleItem2);
    ck_assert("TC_SOPC_FileTransfer_025" && 0 == *pSizeItem1 && 1 == *pOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_025" && 0 == *pSizeItem2 && 1 == *pOpenCountItem2);

    // TC_SOPC_FileTransfer_026:
    getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                                item1PreloadFile.met_getposId, fileHandleItem1);

    getPositionItem2 = SOPC_TEST_FileTransfer_GetPositionMethod(coId, true, &callRequestsItem2, &callResultsItem2,
                                                                item2PreloadFile.met_getposId, fileHandleItem2);
    ck_assert("TC_SOPC_FileTransfer_026" && 8 == getPositionItem1);
    ck_assert("TC_SOPC_FileTransfer_026" && 5 == getPositionItem2);

    // TC_SOPC_FileTransfer_027:
    setPositionItem1 = 3;
    setPositionItem2 = 2;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_SetPositionMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                 item1PreloadFile.met_setposId, fileHandleItem1, setPositionItem1);

    statusMethodItem2 =
        SOPC_TEST_FileTransfer_SetPositionMethod(coId, true, &callRequestsItem2, &callResultsItem2,
                                                 item2PreloadFile.met_setposId, fileHandleItem2, setPositionItem2);
    ck_assert("TC_SOPC_FileTransfer_027" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_027" && SOPC_GoodGenericStatus == statusMethodItem2);

    // TC_SOPC_FileTransfer_028:
    nbOfBytesToRead = 4;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                          item1PreloadFile.met_readId, fileHandleItem1, nbOfBytesToRead);

    nbOfBytesToRead = 3;
    statusMethodItem1 =
        SOPC_TEST_FileTransfer_ReadMethod(coId, true, &callRequestsItem2, &callResultsItem2,
                                          item2PreloadFile.met_readId, fileHandleItem2, nbOfBytesToRead);

    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValSizeItem1);
    SOPC_ClientHelper_Read(coId, &readValueItem2, nbElements, &readValSizeItem2);

    uint8_t loadString[4] = {0x6C, 0x6F, 0x61, 0x64}; // "load"
    uint8_t em2String[3] = {0x65, 0x6D, 0x32};        // "em2"
    SOPC_ByteString_CopyFromBytes(&dataToCompare, loadString, 4);
    pVariantOutput = &callResultsItem1.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_028" && true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));
    SOPC_ByteString_CopyFromBytes(&dataToCompare, em2String, 3);
    pVariantOutput = &callResultsItem2.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_028" && true == SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));
    ck_assert("TC_SOPC_FileTransfer_028" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_028" && SOPC_GoodGenericStatus == statusMethodItem2);
    ck_assert("TC_SOPC_FileTransfer_028" && 8 == *pSizeItem1 && 5 == *pSizeItem2);

    // TC_SOPC_FileTransfer_029:
    statusMethodItem1 = SOPC_TEST_FileTransfer_CloseMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_closeId, fileHandleItem1);

    statusMethodItem2 = SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem2, &callResultsItem2,
                                                           item2PreloadFile.met_closeId, fileHandleItem2);
    readValueItem1.nodeId = item1PreloadFile.var_openCountId;
    readValueItem2.nodeId = item2PreloadFile.var_openCountId;
    SOPC_ClientHelper_Read(coId, &readValueItem1, nbElements, &readValOpenCountItem1);
    SOPC_ClientHelper_Read(coId, &readValueItem2, nbElements, &readValOpenCountItem2);
    ck_assert("TC_SOPC_FileTransfer_029" && 0 == *pOpenCountItem1 && 0 == *pOpenCountItem2);
    ck_assert("TC_SOPC_FileTransfer_029" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_029" && SOPC_GoodGenericStatus == statusMethodItem2);

    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: PHASE 3:\n");
    SOPC_TEST_FileTransfer_SetTestCaseNumber(30);

    // TC_SOPC_FileTransfer_030:
    booleanNotification = false;
    callRequestsClient.objectNodeId = remoteResetId;
    callRequestsClient.methodNodeId = met_remoteResetId;
    clientResultCode = SOPC_ClientHelper_CallMethod(coId, &callRequestsClient, nbElements, &callResultsClient);
    pVariantOutput = &callResultsClient.outputParams->Value;
    ck_assert("TC_SOPC_FileTransfer_030" && 0 == clientResultCode);
    ck_assert("TC_SOPC_FileTransfer_030" && 0 == callResultsClient.status);
    ck_assert("TC_SOPC_FileTransfer_030" && true == pVariantOutput->Boolean);
    ck_assert("TC_SOPC_FileTransfer_030" && true == booleanNotification); // Check if callback notification is called

    // TC_SOPC_FileTransfer_031 (automatic test of the notification to the user during the closing process):
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(coId, false, &callRequestsItem1, &callResultsItem1,
                                                        item1PreloadFile.met_openId, mode);
    ck_assert("TC_SOPC_FileTransfer_031" && SOPC_GoodGenericStatus == statusMethodItem1);
    booleanUserCloseCallback = false;
    statusMethodItem1 = SOPC_TEST_FileTransfer_CloseMethod(coId, true, &callRequestsItem1, &callResultsItem1,
                                                           item1PreloadFile.met_closeId, fileHandleItem1);
    ck_assert("TC_SOPC_FileTransfer_031" && SOPC_GoodGenericStatus == statusMethodItem1);
    ck_assert("TC_SOPC_FileTransfer_031" && true == booleanUserCloseCallback);

    /*---------------------------------------------------------------------------
     *        Clear the client/server toolkit library & FileTransfer API
     *---------------------------------------------------------------------------*/
    SOPC_ClientHelper_Disconnect(coId);
    SOPC_FileTransfer_Clear();
    SOPC_ClientHelper_Finalize();
    SOPC_ServerHelper_StopServer();
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();
}
END_TEST

static Suite* tests_file_transfer(void)
{
    Suite* s;
    TCase* tc_file_transfer_method;

    s = suite_create("Test file transfer");
    tc_file_transfer_method = tcase_create("test file transfer method:");
    tcase_add_test(tc_file_transfer_method, test_file_transfer_method);
    suite_add_tcase(s, tc_file_transfer_method);
    /* All tests are run with a timeout, the default being 4 seconds. If the test is not finished within that time, it
     * is killed and logged as an error. */
    // tcase_set_timeout(tc_file_transfer_method, 10); // 10 seconds

    return s;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    sr = srunner_create(tests_file_transfer());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
