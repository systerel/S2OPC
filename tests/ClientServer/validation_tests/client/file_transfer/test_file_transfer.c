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

#include "libs2opc_common_config.h"
#include "libs2opc_file_transfer.h"
#include "libs2opc_request_builder.h"
#include "without_wrapper_test/check_file_transfer_method.h"

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"

#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_helper_askpass.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"

#define ITEM1_SELECTED true

static int32_t connectionClosed = false;
SOPC_Boolean booleanNotification = false;
SOPC_Boolean booleanUserCloseCallback = false;

static SOPC_ClientConnection* gConnection = NULL;

/*---------------------------------------------------------------------------
 *                          Client initialization
 *---------------------------------------------------------------------------*/

// Callback for unexpected connection events
static void client_ConnectionEventCallback(SOPC_ClientConnection* config,
                                           SOPC_ClientConnectionEvent event,
                                           SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&connectionClosed, true);
    printf("Unexpected connection event");
    ck_assert(false);
}

static SOPC_SecureConnection_Config* client_create_configuration(void)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientConfigHelper_SetPreferredLocaleIds(2, (const char*[]){"fr-FR", "en-US"});

    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientConfigHelper_SetApplicationDescription(
        DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI, DEFAULT_APPLICATION_NAME, "fr-FR", OpcUa_ApplicationType_Client);

    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_CRLList* cacrl = NULL;
    SOPC_CertificateList* ca = NULL;

    status = SOPC_KeyManager_Certificate_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/certs/cacert.der", &ca);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_KeyManager_CRL_CreateOrAddFromFile("./S2OPC_Demo_PKI/trusted/crl/cacrl.der", &cacrl);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath("./client_public/client_4k_cert.der",
                                                            "./client_private/encrypted_client_4k_key.pem", true);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_PKIProvider* pkiProvider = NULL;
    status = SOPC_PKIProvider_CreateFromStore("./S2OPC_Demo_PKI", &pkiProvider);

    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);

    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_SecureConnection_Config* scConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Client_check_ft", DEFAULT_ENDPOINT_URL, OpcUa_MessageSecurityMode_Sign, SOPC_SecurityPolicy_Basic256Sha256);
    ck_assert_ptr_nonnull(scConfig);

    status = SOPC_SecureConnectionConfig_SetUserName(scConfig, "username_Basic256Sha256", "me", "1234");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(scConfig, "./server_public/server_4k_cert.der");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_String endpointUrl;
    SOPC_String_Initialize(&endpointUrl);
    status = SOPC_String_AttachFromCstring(&endpointUrl, DEFAULT_ENDPOINT_URL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_KeyManager_Certificate_Free(ca);
    SOPC_KeyManager_CRL_Free(cacrl);

    return scConfig;
}

static void get_read_response(const char* nodeId, SOPC_DataValue* values)
{
    OpcUa_ReadResponse* readResponse = NULL;
    OpcUa_ReadRequest* readRequest = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Neither);
    ck_assert_ptr_nonnull(readRequest);

    SOPC_ReturnStatus status =
        SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 0, nodeId, SOPC_AttributeId_Value, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientHelper_ServiceSync(gConnection, readRequest, (void**) &readResponse);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(readResponse);
    ck_assert_int_eq(readResponse->NoOfResults, 1);

    SOPC_DataValue_Copy(values, readResponse->Results);

    OpcUa_ReadResponse_Clear(readResponse);
    ck_assert_ptr_nonnull(values);
    SOPC_EncodeableObject_Delete(readResponse->encodeableType, (void**) &readResponse);
}

/*---------------------------------------------------------------------------
 *                   File Transfer Test
 *---------------------------------------------------------------------------*/

START_TEST(test_file_transfer_method)
{
    // Data which will contain value read
    SOPC_DataValue readValSizeItem1;
    SOPC_DataValue readValOpenCountItem1;
    SOPC_DataValue readValUserWritableItem1;
    SOPC_DataValue readValWritableItem1;
    SOPC_DataValue_Initialize(&readValSizeItem1);
    SOPC_DataValue_Initialize(&readValOpenCountItem1);
    SOPC_DataValue_Initialize(&readValUserWritableItem1);
    SOPC_DataValue_Initialize(&readValWritableItem1);

    // Pointer to these values:
    SOPC_Boolean* pUserWritableItem1 = &readValUserWritableItem1.Value.Value.Boolean;
    SOPC_Boolean* pWritableItem1 = &readValWritableItem1.Value.Value.Boolean;
    uint64_t* pSizeItem1 = &readValSizeItem1.Value.Value.Uint64;
    uint16_t* pOpenCountItem1 = &readValOpenCountItem1.Value.Value.Uint16;

    // Method write input parameters:
    SOPC_ByteString dataToWrite_ABCDString;
    SOPC_ByteString_Initialize(&dataToWrite_ABCDString);
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite_ABCDString, ABCDString, 4);
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

    // Method Open, Read, GetPosition, SetPosition input parameters:
    SOPC_Byte mode = 0;
    int32_t nbOfBytesToRead = 0;
    uint64_t setPositionItem1 = 0;
    uint64_t getPositionItem1 = 0;
    uint32_t fileHandleItem1 = 0;

    // S2OPC CallRequest and CallResult structure for item 1:
    OpcUa_CallResponse* callResponseItem1 = NULL;

    // Functional test: PHASE 3: declaration variables:
    SOPC_DataValue readValClient;
    SOPC_DataValue writeValClient;
    SOPC_DataValue_Initialize(&readValClient);
    SOPC_String* pHelloWorldString = &writeValClient.Value.Value.String;
    SOPC_String_Initialize(pHelloWorldString);
    SOPC_String_AttachFromCstring(pHelloWorldString, "Hello World");

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
    status = SOPC_CommonHelper_Initialize(&log_config, NULL);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_SecureConnection_Config* clientCfg = client_create_configuration();

    // Connect client to server
    status = SOPC_ClientHelper_Connect(clientCfg, client_ConnectionEventCallback, &gConnection);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: PHASE 0:\n");

    // TC_SOPC_FileTransfer_001:
    // Verify default values of item1's PreloadFile type after FileTransfer API initialization.

    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    get_read_response(NODEID_VAR_WRITABLE_ITEM1, &readValWritableItem1);
    get_read_response(NODEID_VAR_USER_WRITABLE_ITEM1, &readValUserWritableItem1);

    ck_assert("TC_SOPC_FileTransfer_001" && 0 == *pSizeItem1 && 0 == *pOpenCountItem1);
    ck_assert("TC_SOPC_FileTransfer_001" && false == *pUserWritableItem1 && false == *pWritableItem1);

    // TC_SOPC_FileTransfer_002:
    // Test case where the Read method is called before opening the file.
    nbOfBytesToRead = 100;
    fileHandleItem1 = 1500;
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    ck_assert_ptr_null(callResponseItem1->Results[0].OutputArguments);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_003:
    // Test case where the Write method is called before opening the file.
    SOPC_TEST_FileTransfer_WriteMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                       &dataToWrite_ABCDString);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_004:
    // Test case where the GetPosition method is called before opening the file.
    uint64_t pos =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    ck_assert_ptr_null(callResponseItem1->Results[0].OutputArguments);
    ck_assert_uint_eq(INVALID_POSITION, pos);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_005:
    // Test case where the SetPosition method is called before opening the file.
    setPositionItem1 = 650;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_006:
    // Test case where the Close method is called before opening the file.
    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_007:
    // Use the Open method with an invalid mode (bits 4 to 7 are reserved for future use).
    mode = 16;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    ck_assert_int_eq(INVALID_FILE_HANDLE, fileHandleItem1);
    ck_assert_uint_eq(0, *pOpenCountItem1);
    ck_assert_int_eq(0, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_008:
    // Use the Open method with an invalid mode:
    // bit number 2 of mode parameter (EraseExisting) can only be activated if the file opened in write mode.
    mode = 4;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    ck_assert_int_eq(INVALID_FILE_HANDLE, fileHandleItem1);
    ck_assert_uint_eq(0, *pOpenCountItem1);
    ck_assert_int_eq(0, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_009:
    // Test to open a file in write mode when it is already open in read mode.
    mode = 1;
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    uint32_t tmpFileHandle = fileHandleItem1;
    mode = 2; // try to open a second time with write mode
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    ck_assert_int_eq(OpcUa_BadNotWritable, callResponseItem1->Results[0].StatusCode);
    ck_assert_int_eq(INVALID_FILE_HANDLE, fileHandleItem1);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_uint_eq(1, *pOpenCountItem1);
    ck_assert_int_eq(0, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, tmpFileHandle);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_010:
    // Test to open a file in read mode when it is already open in write mode.
    mode = 2;
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    tmpFileHandle = fileHandleItem1;
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_int_eq(OpcUa_BadNotReadable, callResponseItem1->Results[0].StatusCode);
    ck_assert_int_eq(1, *pOpenCountItem1);
    ck_assert_int_eq(0, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);
    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, tmpFileHandle);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_011:
    // Test to open the file in read mode to then try to write in it.
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_WriteMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                       &dataToWrite_ABCDString);
    ck_assert_int_eq(OpcUa_BadInvalidState, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_012:
    // Test to open the file in write mode to then try to read in it.
    mode = 2;
    nbOfBytesToRead = 100;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    ck_assert_int_eq(OpcUa_BadInvalidState, callResponseItem1->Results[0].StatusCode);
    ck_assert_ptr_null(callResponseItem1->Results[0].OutputArguments);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "<Test_File_Transfer: PHASE 1:\n");

    // TC_SOPC_FileTransfer_013:
    // Test the limitation of only one simultaneous opening per file (see libs2opc_file_transfer.h)

    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert_uint_ne(INVALID_FILE_HANDLE, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    uint32_t tmpFileHandleItem1 = 0;

    mode = 1; // try to open a second time
    tmpFileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_int_eq(OpcUa_BadNotSupported, callResponseItem1->Results[0].StatusCode);
    ck_assert_int_eq(1, *pOpenCountItem1);
    ck_assert_uint_eq(INVALID_FILE_HANDLE, tmpFileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert_uint_eq(0, getPositionItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_014:
    // Test where the file is opened in read/write mode and the position is at the beginning of a file.
    mode = 3;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert_uint_eq(0, getPositionItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_015:
    // Verification of the Size variable after writing ABCD inside the file.
    mode = 3;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    ck_assert_uint_eq(0, *pSizeItem1);

    SOPC_TEST_FileTransfer_WriteMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                       &dataToWrite_ABCDString);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    ck_assert_uint_eq(4, *pSizeItem1);

    // TC_SOPC_FileTransfer_016:
    // Verification that the position is at the end of the file.
    getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert_uint_eq(4, getPositionItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_017:
    // Verification of the Size variable by reading the file.
    SOPC_ByteString dataToRead;
    SOPC_ByteString_Initialize(&dataToRead);
    uint8_t ABString[2] = {0x41, 0x42};
    status = SOPC_ByteString_CopyFromBytes(&dataToRead, ABString, 2);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    setPositionItem1 = 0;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    nbOfBytesToRead = 2;
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_VariantValue* pVariantOutput = &callResponseItem1->Results[0].OutputArguments->Value;
    ck_assert_ptr_nonnull(pVariantOutput->Bstring.Data);
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToRead));
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);
    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    ck_assert_uint_eq(4, *pSizeItem1);

    SOPC_ByteString_Clear(&dataToRead);

    // TC_SOPC_FileTransfer_018:
    // Test of SetPosition method.
    setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert_uint_eq(2, getPositionItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_019:
    // Test case where the Read method is called with an invalid number of bytes to read.
    nbOfBytesToRead = -1;
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    ck_assert_int_eq(OpcUa_BadInvalidArgument, callResponseItem1->Results[0].StatusCode);
    ck_assert_ptr_null(callResponseItem1->Results[0].OutputArguments);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_020:
    // Verify that Read mode by reading the last to bytes of the ABCD string.
    // Also test the SetPosition method from TC_SOPC_FileTransfer_018.
    nbOfBytesToRead = 2; // "CD"
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    pVariantOutput = &callResponseItem1->Results[0].OutputArguments->Value;
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_021:
    // Replace the C in the ABCD string by E.
    setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    uint8_t EString[1] = {0x45};
    SOPC_ByteString toWrite;
    SOPC_ByteString_Initialize(&toWrite);
    status = SOPC_ByteString_CopyFromBytes(&toWrite, EString, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_TEST_FileTransfer_WriteMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1, &toWrite);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_ByteString_Clear(&toWrite);

    // TC_SOPC_FileTransfer_022:
    // Verify that the character C was correctly replaced by E.
    setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_ByteString_Clear(&dataToCompare);
    status = SOPC_ByteString_CopyFromBytes(&dataToCompare, EString, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    nbOfBytesToRead = 1; // "E"
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    pVariantOutput = &callResponseItem1->Results[0].OutputArguments->Value;
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_023:
    // Verify the value of Size and OpenCount variables.
    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);
    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_uint_eq(4, *pSizeItem1);
    ck_assert_int_eq(0, *pOpenCountItem1);

    // TC_SOPC_FileTransfer_024:
    // Verify that the Size variable is not updated after Open method.
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);
    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_int_eq(1, *pOpenCountItem1);
    ck_assert_uint_eq(4, *pSizeItem1);
    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_025:
    // Check that Size variable has been updated after Write method.
    mode = 3;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    setPositionItem1 = 0;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_uint_eq(0, getPositionItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    SOPC_TEST_FileTransfer_WriteMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                       &dataWriteItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    ck_assert_uint_ne(0, fileHandleItem1);
    ck_assert_uint_eq(8, *pSizeItem1); // "preload1"
    ck_assert_uint_eq(1, *pOpenCountItem1);

    // TC_SOPC_FileTransfer_026:
    // Check that cursor is at the end of the file after Write method.
    getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_uint_eq(8, getPositionItem1);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    // TC_SOPC_FileTransfer_027:
    // Read the "load" string inside the file.
    setPositionItem1 = 3;
    SOPC_TEST_FileTransfer_SetPositionMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                             setPositionItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    uint8_t loadString[4] = {0x6C, 0x6F, 0x61, 0x64}; // "load"
    SOPC_ByteString_Clear(&dataToCompare);
    SOPC_ByteString_CopyFromBytes(&dataToCompare, loadString, 4);

    nbOfBytesToRead = 4;
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    pVariantOutput = &callResponseItem1->Results[0].OutputArguments->Value;
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataToCompare));
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    get_read_response(NODEID_VAR_SIZE_ITEM1, &readValSizeItem1);
    ck_assert_uint_eq(8, *pSizeItem1);

    SOPC_TEST_FileTransfer_CloseMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    get_read_response(NODEID_VAR_OPEN_COUNT_ITEM1, &readValOpenCountItem1);
    ck_assert_uint_eq(0, *pOpenCountItem1);

    // TC_SOPC_FileTransfer_028:
    // Test that the content of previous file is locally saved.
    // Note: According to the OPC UA norm, whether the file is locally saved or not is server specific.
    //       For this demo server, the file is automatically saved.
    mode = 1;
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, mode);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    nbOfBytesToRead = 8;
    SOPC_TEST_FileTransfer_ReadMethod(gConnection, ITEM1_SELECTED, &callResponseItem1, fileHandleItem1,
                                      nbOfBytesToRead);
    ck_assert_int_eq(SOPC_GoodGenericStatus, callResponseItem1->Results[0].StatusCode);
    pVariantOutput = &callResponseItem1->Results[0].OutputArguments->Value;
    ck_assert(
        SOPC_ByteString_Equal(&pVariantOutput->Bstring, &dataWriteItem1)); // "preload1" content of last opened file
    SOPC_EncodeableObject_Delete(callResponseItem1->encodeableType, (void**) &callResponseItem1);

    /*---------------------------------------------------------------------------
     *        Clear the client toolkit library & FileTransfer API
     *---------------------------------------------------------------------------*/
    SOPC_ByteString_Clear(&dataWriteItem1);
    SOPC_ByteString_Clear(&dataToWrite_ABCDString);
    SOPC_ByteString_Clear(&dataToCompare);

    SOPC_ClientHelper_Disconnect(&gConnection);
    SOPC_FileTransfer_Clear();
    SOPC_ClientConfigHelper_Clear();
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
    tcase_set_timeout(tc_file_transfer_method, 10); // 10 seconds

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
