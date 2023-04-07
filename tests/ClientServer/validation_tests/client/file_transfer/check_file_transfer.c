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
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#include "check_file_transfer_method.h"
#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"

#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_APPLICATION_NAME "Client_Check_File_Transfer_S2OPC"

#define BIN_READ_FILE_PATH "./ft_data/bin_file.a"

// Bit masks value for opening mode
#define READ_MASK 0x01u
#define WRITE_MASK 0x02u
#define ERASE_EXISTING_MASK 0x04u
#define APPEND_MASK 0x08u
#define INVALID_MODE_BIT_4 0x10 // bit 4:7 are reserved for future use

#define ITEM_1_PATH "./toolkit_test_server_file_transfer_logs/item1File.log"
#define ITEM_2_PATH "./toolkit_test_server_file_transfer_logs/item2File.log"

/*---------------------------------------------------------------------------
 *                          Global variables for test cases
 *---------------------------------------------------------------------------*/

// Client configuration ID
int32_t g_coId = -1;

/* Read the value of the Variable nodeId, asserts that it is an uint64_t, and compares it to expected */
static void assert_read_uint64(int32_t coId, const char* nodeId, uint64_t expected)
{
    SOPC_ClientHelper_ReadValue rv = {.nodeId = nodeId, .attributeId = SOPC_AttributeId_Value};
    SOPC_DataValue dv = {0};
    SOPC_Variant* variant = &dv.Value;

    int32_t res = SOPC_ClientHelper_Read(coId, &rv, 1, &dv);

    ck_assert(0 == res);
    ck_assert(SOPC_VariantArrayType_SingleValue == variant->ArrayType);
    ck_assert(SOPC_UInt64_Id == variant->BuiltInTypeId);
    ck_assert(variant->Value.Uint64 == expected);
}

/* Read the value of the Variable nodeId, asserts that it is an uint16_t, and compares it to expected */
static void assert_read_uint16(int32_t coId, const char* nodeId, uint16_t expected)
{
    SOPC_ClientHelper_ReadValue rv = {.nodeId = nodeId, .attributeId = SOPC_AttributeId_Value};
    SOPC_DataValue dv = {0};
    SOPC_Variant* variant = &dv.Value;

    int32_t res = SOPC_ClientHelper_Read(coId, &rv, 1, &dv);

    ck_assert(0 == res);
    ck_assert(SOPC_VariantArrayType_SingleValue == variant->ArrayType);
    ck_assert(SOPC_UInt16_Id == variant->BuiltInTypeId);
    ck_assert(variant->Value.Uint16 == expected);
}

/* Read the value of the Variable nodeId, asserts that it is a boolean, and compares it to expected */
static void assert_read_bool(int32_t coId, const char* nodeId, bool expected)
{
    SOPC_ClientHelper_ReadValue rv = {.nodeId = nodeId, .attributeId = SOPC_AttributeId_Value};
    SOPC_DataValue dv = {0};
    SOPC_Variant* variant = &dv.Value;

    int32_t res = SOPC_ClientHelper_Read(coId, &rv, 1, &dv);

    ck_assert(0 == res);
    ck_assert(SOPC_VariantArrayType_SingleValue == variant->ArrayType);
    ck_assert(SOPC_Boolean_Id == variant->BuiltInTypeId);
    ck_assert(variant->Value.Boolean == expected);
}

/*---------------------------------------------------------------------------
 *                          Client configuration
 *---------------------------------------------------------------------------*/

static int32_t client_create_configuration(void)
{
    int32_t res = SOPC_ClientHelper_Initialize(NULL);
    ck_assert("Client configuration failed" && 0 == res);

    SOPC_ClientHelper_Security security = {.security_policy = SOPC_SecurityPolicy_None_URI,
                                           .security_mode = OpcUa_MessageSecurityMode_None,
                                           .path_cert_auth = "./trusted/cacert.der",
                                           .path_crl = "./revoked/cacrl.der",
                                           .path_cert_srv = NULL,
                                           .path_cert_cli = NULL,
                                           .path_key_cli = NULL,
                                           .policyId = "anonymous",
                                           .username = NULL,
                                           .password = NULL};

    SOPC_ClientHelper_EndpointConnection endpoint = {
        .endpointUrl = DEFAULT_ENDPOINT_URL,
        .serverUri = NULL,
        .reverseConnectionConfigId = 0,
    };

    int32_t clientCfgId = SOPC_ClientHelper_CreateConfiguration(&endpoint, &security, NULL);
    ck_assert("Client configuration failed" && clientCfgId > 0);
    return clientCfgId;
}

static int remove_file(const char* path)
{
    int res = remove(path);
    if (0 != res)
    {
        if (ENOENT != errno)
        {
            return 1;
        }
    }
    return 0;
}

static void client_check_no_such_file(const char* path)
{
    FILE* fd = fopen(path, "rb");
    ck_assert_ptr_null(fd);
    ck_assert(ENOENT == errno); // No such file or directory
}

static void client_check_file_exists(const char* path)
{
    FILE* fd = fopen(path, "rb");
    ck_assert_ptr_nonnull(fd);
    fclose(fd);
}

/*---------------------------------------------------------------------------
 *                   File Transfer Test
 *---------------------------------------------------------------------------*/

START_TEST(browse_file_type)
{
    /* Assert that Item1 and Item2 has component PreloadFile on test server using Browse */

    SOPC_ClientHelper_BrowseRequest browseRequest[2] = {{.nodeId = NODEID_IEM1, // From Item1 node
                                                         .direction = OpcUa_BrowseDirection_Forward,
                                                         .referenceTypeId = "ns=0;i=33", // All ref
                                                         .includeSubtypes = true},
                                                        {.nodeId = NODEID_ITEM2, // From Item2 node
                                                         .direction = OpcUa_BrowseDirection_Forward,
                                                         .referenceTypeId = "ns=0;i=33", // All ref
                                                         .includeSubtypes = true}};
    SOPC_ClientHelper_BrowseResult browseResult[2];

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Browse(g_coId, browseRequest, 2, browseResult));
    char* fileTypeNodeId[2] = {NODEID_FILE_TYPE_ITEM1, NODEID_FILE_TYPE_ITEM2};
    bool found_preload_file[2] = {false};
    bool browse_name_match[2] = {false};
    bool node_id_match[2] = {false};
    for (int32_t i = 0; i < 2; i++)
    {
        for (int32_t j = 0; j < browseResult[i].nbOfReferences; j++)
        {
            const SOPC_ClientHelper_BrowseResultReference* ref = &browseResult[i].references[j];

            node_id_match[i] = strcmp(ref->nodeId, fileTypeNodeId[i]) == 0;
            browse_name_match[i] = strcmp(ref->browseName, "PreloadFile") == 0;
            if (node_id_match[i] && browse_name_match[i])
            {
                found_preload_file[i] = true;
            }
        }
        ck_assert(found_preload_file[i]);
    }

    SOPC_ClientHelper_BrowseResults_Clear(2, browseResult);
}
END_TEST

START_TEST(variables_init)
{
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    assert_read_bool(g_coId, NODEID_VAR_USER_WRITABLE_ITEM1, true);
    assert_read_bool(g_coId, NODEID_VAR_WRITABLE_ITEM1, true);
}
END_TEST

START_TEST(invalid_file_handle_read)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    int32_t nbOfBytesToRead = 100;
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, fileHandleItem1, nbOfBytesToRead);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    ck_assert_ptr_null(callResults.outputParams);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
}
END_TEST

START_TEST(invalid_file_handle_write)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_ByteString dataToWrite;
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ByteString_Initialize(&dataToWrite);
    /* The content of dataToWrite is allocated here and the whole structure is copied to the CallRequest and freed by
       the library but still accessible after SOPC_TEST_FileTransfer_WriteMethod */
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, true, &callResults, fileHandleItem1, &dataToWrite);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    /* Reset the content of dataToWrite to make it unreachable */
    SOPC_ByteString_Initialize(&dataToWrite);
}
END_TEST

START_TEST(invalid_file_handle_get_position)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    ck_assert_ptr_null(callResults.outputParams);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
}
END_TEST

START_TEST(invalid_file_handle_set_position)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint64_t setPositionItem1 = 650;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, true, &callResults, fileHandleItem1, setPositionItem1);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
}
END_TEST

START_TEST(invalid_file_handle_close)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_no_such_file(ITEM_1_PATH);
}
END_TEST

START_TEST(invalid_open_mode_bit_4)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_Byte mode = INVALID_MODE_BIT_4; // bits 4:7 are reserved for future use
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    ck_assert(INVALID_FILE_HANDLE == fileHandleItem1);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
}
END_TEST

START_TEST(invalid_open_mode_erase_existing)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_Byte mode = ERASE_EXISTING_MASK; // This bit can only be set if the file is opened for writing
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    ck_assert(INVALID_FILE_HANDLE == fileHandleItem1);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
}
END_TEST

START_TEST(invalid_open_mode_not_writable)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_Byte mode = READ_MASK; // Reading
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    uint32_t tmpFileHandle = fileHandleItem1;
    mode = APPEND_MASK; // Try to open a second time with a different mode (writing into appending)
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(OpcUa_BadNotWritable == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    ck_assert(INVALID_FILE_HANDLE == fileHandleItem1);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, tmpFileHandle);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

START_TEST(invalid_open_mode_not_readable)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_Byte mode = APPEND_MASK | WRITE_MASK; // Writing into appending mode
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    uint32_t tmpFileHandle = fileHandleItem1;
    mode = READ_MASK; // Reading
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(OpcUa_BadNotReadable == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    ck_assert(INVALID_FILE_HANDLE == fileHandleItem1);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, tmpFileHandle);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

START_TEST(invalid_write)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_ByteString dataToWrite;
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    /* The content of dataToWrite is allocated here and the whole structure is copied to the CallRequest and free by the
       library but still accessible after SOPC_TEST_FileTransfer_WriteMethod */
    SOPC_ByteString_Initialize(&dataToWrite);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_Byte mode = READ_MASK;
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, true, &callResults, fileHandleItem1, &dataToWrite);
    ck_assert(OpcUa_BadInvalidState == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
    /* Reset the content of dataToWrite to make it unreachable */
    SOPC_ByteString_Initialize(&dataToWrite);
}
END_TEST

START_TEST(invalid_read)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_Byte mode = WRITE_MASK;
    int32_t nbOfBytesToRead = 100;
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, fileHandleItem1, nbOfBytesToRead);
    ck_assert(OpcUa_BadInvalidState == callResults.status);
    ck_assert_ptr_null(callResults.outputParams);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}

START_TEST(get_position_empty_file)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(0 == getPositionItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

static void write_abcd_string(uint32_t* fileHandle)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_ByteString dataToWrite;
    uint8_t ABCDString[4] = {0x41, 0x42, 0x43, 0x44};
    SOPC_ByteString_Initialize(&dataToWrite);
    /* The content of dataToWrite is allocated here and the whole structure is copied to the CallRequest and free by the
       library but still accessible after SOPC_TEST_FileTransfer_WriteMethod */
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataToWrite, ABCDString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    uint32_t fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, true, &callResults, fileHandleItem1, &dataToWrite);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    // Size is updated after the write method
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 4);
    *fileHandle = fileHandleItem1;
    /* Reset the content of dataToWrite to make it unreachable */
    SOPC_ByteString_Initialize(&dataToWrite);
}

START_TEST(abcd_file_read_at_the_end_of_file)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    write_abcd_string(&fileHandleItem1);
    int32_t nbOfBytesToRead = 2;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, fileHandleItem1, nbOfBytesToRead);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(1 == callResults.nbOfOutputParams);
    ck_assert_ptr_nonnull(callResults.outputParams);
    ck_assert(SOPC_ByteString_Id == callResults.outputParams->BuiltInTypeId);
    SOPC_VariantValue* pVariantOutput = &callResults.outputParams->Value;
    ck_assert(-1 == pVariantOutput->Bstring.Length);
    ck_assert_ptr_null(pVariantOutput->Bstring.Data);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

static void set_position_2_of_abcd_file(uint32_t* fileHandle)
{
    write_abcd_string(fileHandle);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    // Current position = 4
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, *fileHandle);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(4 == pos);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    // Set position = 2
    uint64_t setPositionItem1 = 2;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, true, &callResults, *fileHandle, setPositionItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
}

START_TEST(abcd_file_invalid_read_length)
{
    int32_t nbOfBytesToRead = -1;
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    set_position_2_of_abcd_file(&fileHandleItem1);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    // Current position = 2
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(2 == pos);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, fileHandleItem1, nbOfBytesToRead);
    ck_assert(OpcUa_BadInvalidArgument == callResults.status);
    ck_assert_ptr_null(callResults.outputParams);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

START_TEST(abcd_file_from_position_2_read_cd)
{
    int32_t nbOfBytesToRead = 2; // "CD"
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    set_position_2_of_abcd_file(&fileHandleItem1);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_ByteString* dataToCompare = SOPC_ByteString_Create();
    ck_assert_ptr_nonnull(dataToCompare);
    uint8_t CDString[2] = {0x43, 0x44};
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(dataToCompare, CDString, 2);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    // Current position = 2
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(2 == pos);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, fileHandleItem1, nbOfBytesToRead);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(1 == callResults.nbOfOutputParams);
    ck_assert_ptr_nonnull(callResults.outputParams);
    ck_assert(SOPC_ByteString_Id == callResults.outputParams->BuiltInTypeId);
    SOPC_VariantValue* pVariantOutput = &callResults.outputParams->Value;
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, dataToCompare));
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_ByteString_Delete(dataToCompare);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

static inline void replace_c_by_e_in_abcd_file_from_position_2(uint32_t* fileHandle)
{
    set_position_2_of_abcd_file(fileHandle);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_ByteString toWrite;
    uint8_t EString[1] = {0x45};
    SOPC_ByteString_Initialize(&toWrite);
    /* The content of toWrite is allocated here and the whole structure is copied to the CallRequest and free by the
       library but still accessible after SOPC_TEST_FileTransfer_WriteMethod */
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&toWrite, EString, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, true, &callResults, *fileHandle, &toWrite);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    /* Reset the content of dataToWrite to make it unreachable */
    SOPC_ByteString_Initialize(&toWrite);
}

static void read_back_e_replaced_in_abcd_file(uint32_t* fileHandle)
{
    replace_c_by_e_in_abcd_file_from_position_2(fileHandle);

    uint64_t setPositionItem1 = 2;
    uint8_t estring[1] = {0x45}; // E
    SOPC_ByteString* toCompare = SOPC_ByteString_Create();
    ck_assert_ptr_nonnull(toCompare);
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(toCompare, estring, 1);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    // Current position = 3
    uint64_t pos = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, *fileHandle);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(3 == pos);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    // Set position = 2
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, true, &callResults, *fileHandle, setPositionItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, *fileHandle, 1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(1 == callResults.nbOfOutputParams);
    ck_assert_ptr_nonnull(callResults.outputParams);
    ck_assert(SOPC_ByteString_Id == callResults.outputParams->BuiltInTypeId);
    SOPC_VariantValue* pVariantOutput = &callResults.outputParams->Value;
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, toCompare));
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_ByteString_Delete(toCompare);
}

START_TEST(abcd_file_replace_c_by_e_from_position_2_and_read_back)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    read_back_e_replaced_in_abcd_file(&fileHandleItem1);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

START_TEST(abed_file_close_and_size_variable_is_reset_after_a_new_open)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    read_back_e_replaced_in_abcd_file(&fileHandleItem1);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
    // Size variable is reset only if the file is open again
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 4);
    SOPC_Byte mode = READ_MASK; // Reading
    fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    // The Size variable is reset
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    // The OpenCount variable is reset after a close
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

static void write_preload1_in_item1_and_write_Item2_in_item2(uint32_t* fileHandleItem1, uint32_t* fileHandleItem2)
{
    SOPC_ByteString dataWriteItem1;
    SOPC_ByteString_Initialize(&dataWriteItem1);
    uint8_t stringItem1[8] = {0x70, 0x72, 0x65, 0x6C, 0x6F, 0x61, 0x64, 0x31}; //  stringItem1 = "preload1"
    /* The content of dataWriteItem1 is allocated here and the whole structure is copied to the CallRequest and free by
       the library but still accessible after SOPC_TEST_FileTransfer_WriteMethod */
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(&dataWriteItem1, stringItem1, 8);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_ByteString dataWriteItem2;
    SOPC_ByteString_Initialize(&dataWriteItem2);
    uint8_t stringItem2[5] = {0x49, 0x74, 0x65, 0x6D, 0x32}; // stringItem2 = "Item2"
    /* The content of dataWriteItem2 is allocated here and the whole structure is copied to the CallRequest and free by
       the library but still accessible after SOPC_TEST_FileTransfer_WriteMethod */
    status = SOPC_ByteString_CopyFromBytes(&dataWriteItem2, stringItem2, 5);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    SOPC_ClientHelper_CallMethodResult callResultsItem1 = {0};
    SOPC_ClientHelper_CallMethodResult callResultsItem2 = {0};

    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM2, 0);
    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    *fileHandleItem1 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResultsItem1, mode);
    *fileHandleItem2 = SOPC_TEST_FileTransfer_OpenMethod(g_coId, false, &callResultsItem2, mode);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM2, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, true, &callResultsItem1, *fileHandleItem1, &dataWriteItem1);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, false, &callResultsItem2, *fileHandleItem2, &dataWriteItem2);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);
    ck_assert(INVALID_FILE_HANDLE != *fileHandleItem1);
    ck_assert(INVALID_FILE_HANDLE != *fileHandleItem2);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 8);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM2, 5);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM2, 1);
    /* Reset the content of dataWriteItem1 to make it unreachable */
    SOPC_ByteString_Initialize(&dataWriteItem1);
    /* Reset the content of dataWriteItem1 to make it unreachable */
    SOPC_ByteString_Initialize(&dataWriteItem2);
}

START_TEST(write_in_two_files_and_get_position)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    write_preload1_in_item1_and_write_Item2_in_item2(&fileHandleItem1, &fileHandleItem2);
    SOPC_ClientHelper_CallMethodResult callResultsItem1 = {0};
    SOPC_ClientHelper_CallMethodResult callResultsItem2 = {0};
    uint64_t getPositionItem1 =
        SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResultsItem1, fileHandleItem1);
    uint64_t getPositionItem2 =
        SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, false, &callResultsItem2, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);
    ck_assert(8 == getPositionItem1); // End of file for item1
    ck_assert(5 == getPositionItem2); // End of file for item2
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResultsItem1, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, false, &callResultsItem2, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM2, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);
    client_check_file_exists(ITEM_1_PATH);
    client_check_file_exists(ITEM_2_PATH);
}
END_TEST

static void write_and_change_position_in_two_files(uint32_t* fileHandleItem1, uint32_t* fileHandleItem2)
{
    write_preload1_in_item1_and_write_Item2_in_item2(fileHandleItem1, fileHandleItem2);
    uint64_t setPositionItem1 = 3;
    uint64_t setPositionItem2 = 2;
    SOPC_ClientHelper_CallMethodResult callResultsItem1 = {0};
    SOPC_ClientHelper_CallMethodResult callResultsItem2 = {0};
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, true, &callResultsItem1, *fileHandleItem1, setPositionItem1);
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, false, &callResultsItem2, *fileHandleItem2, setPositionItem2);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);
}

START_TEST(write_in_two_files_and_read_from_the_new_position)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    uint32_t fileHandleItem2 = INVALID_FILE_HANDLE;
    write_and_change_position_in_two_files(&fileHandleItem1, &fileHandleItem2);
    SOPC_ClientHelper_CallMethodResult callResultsItem1 = {0};
    SOPC_ClientHelper_CallMethodResult callResultsItem2 = {0};
    int32_t nbOfBytesToRead = 4;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResultsItem1, fileHandleItem1, nbOfBytesToRead);
    nbOfBytesToRead = 3;
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, false, &callResultsItem2, fileHandleItem2, nbOfBytesToRead);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);

    uint8_t loadString[4] = {0x6C, 0x6F, 0x61, 0x64}; // "load"
    uint8_t em2String[3] = {0x65, 0x6D, 0x32};        // "em2"
    SOPC_ByteString* dataToCompareItem1 = SOPC_ByteString_Create();
    SOPC_ByteString* dataToCompareItem2 = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = SOPC_ByteString_CopyFromBytes(dataToCompareItem1, loadString, 4);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    status = SOPC_ByteString_CopyFromBytes(dataToCompareItem2, em2String, 3);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    ck_assert(1 == callResultsItem1.nbOfOutputParams);
    ck_assert(1 == callResultsItem2.nbOfOutputParams);
    ck_assert_ptr_nonnull(callResultsItem1.outputParams);
    ck_assert_ptr_nonnull(callResultsItem2.outputParams);
    ck_assert(SOPC_ByteString_Id == callResultsItem1.outputParams->BuiltInTypeId);
    ck_assert(SOPC_ByteString_Id == callResultsItem2.outputParams->BuiltInTypeId);
    SOPC_VariantValue* pVariantOutputItem1 = &callResultsItem1.outputParams->Value;
    SOPC_VariantValue* pVariantOutputItem2 = &callResultsItem2.outputParams->Value;

    ck_assert(SOPC_ByteString_Equal(&pVariantOutputItem1->Bstring, dataToCompareItem1));
    ck_assert(SOPC_ByteString_Equal(&pVariantOutputItem2->Bstring, dataToCompareItem2));
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 8);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM2, 5);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResultsItem1, fileHandleItem1);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, false, &callResultsItem2, fileHandleItem2);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem1.status);
    ck_assert(SOPC_GoodGenericStatus == callResultsItem2.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM2, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResultsItem2);

    SOPC_ByteString_Delete(dataToCompareItem1);
    SOPC_ByteString_Delete(dataToCompareItem2);

    client_check_file_exists(ITEM_1_PATH);
    client_check_file_exists(ITEM_2_PATH);
}
END_TEST

static void write_binary_file(uint32_t* fileHandle, SOPC_ByteString** ppDataToCompare)
{
    FILE* bin_file_fp = fopen(BIN_READ_FILE_PATH, "rb+");
    ck_assert_ptr_nonnull(bin_file_fp);
    int filedes = fileno(bin_file_fp);
    ck_assert(-1 != filedes);
    struct stat sb;
    int ret = fstat(filedes, &sb);
    ck_assert(-1 != ret);

    SOPC_ByteString bin_buffer_write = {0};
    bin_buffer_write.Length = (int32_t) sb.st_size;
    bin_buffer_write.Data = SOPC_Malloc((size_t) bin_buffer_write.Length);
    ck_assert_ptr_nonnull(bin_buffer_write.Data);
    size_t read_count = fread(bin_buffer_write.Data, 1, (size_t) bin_buffer_write.Length, bin_file_fp);

    ck_assert(read_count == (size_t) bin_buffer_write.Length);
    // bin_buffer_write is clear by the after the callRequests
    SOPC_ByteString* buffer = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = SOPC_ByteString_Copy(buffer, &bin_buffer_write);
    ck_assert_ptr_nonnull(buffer);
    ck_assert(SOPC_STATUS_OK == status);
    *ppDataToCompare = buffer;
    ret = fclose(bin_file_fp);
    ck_assert(0 == ret);

    SOPC_ClientHelper_CallMethodResult callResults = {0};
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_Byte mode = READ_MASK | WRITE_MASK; // Reading and writing
    *fileHandle = SOPC_TEST_FileTransfer_OpenMethod(g_coId, true, &callResults, mode);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, *fileHandle);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 1);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, 0);
    ck_assert(0 == getPositionItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_WriteMethod(g_coId, true, &callResults, *fileHandle, &bin_buffer_write);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, (uint64_t) bin_buffer_write.Length);
}

START_TEST(write_a_binary_file_and_read_back)
{
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_ByteString* ppDataToCompare = NULL;
    write_binary_file(&fileHandleItem1, &ppDataToCompare);
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    uint64_t setPositionItem1 = 0;
    SOPC_TEST_FileTransfer_SetPositionMethod(g_coId, true, &callResults, fileHandleItem1, setPositionItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    uint64_t getPositionItem1 = SOPC_TEST_FileTransfer_GetPositionMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(0 == getPositionItem1);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_ReadMethod(g_coId, true, &callResults, fileHandleItem1, ppDataToCompare->Length);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    ck_assert(1 == callResults.nbOfOutputParams);
    ck_assert_ptr_nonnull(callResults.outputParams);
    ck_assert(SOPC_ByteString_Id == callResults.outputParams->BuiltInTypeId);
    SOPC_VariantValue* pVariantOutput = &callResults.outputParams->Value;
    ck_assert(SOPC_ByteString_Equal(&pVariantOutput->Bstring, ppDataToCompare));
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, (uint64_t) ppDataToCompare->Length);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);

    SOPC_ByteString_Delete(ppDataToCompare);

    client_check_file_exists(ITEM_1_PATH);
}
END_TEST

START_TEST(write_a_binary_file_and_read_back_from_file_system)
{
    SOPC_ClientHelper_CallMethodResult callResults = {0};
    uint32_t fileHandleItem1 = INVALID_FILE_HANDLE;
    SOPC_ByteString* ppDataToCompare = NULL;
    write_binary_file(&fileHandleItem1, &ppDataToCompare);
    assert_read_uint64(g_coId, NODEID_VAR_SIZE_ITEM1, (uint64_t) ppDataToCompare->Length);
    SOPC_TEST_FileTransfer_CloseMethod(g_coId, true, &callResults, fileHandleItem1);
    ck_assert(SOPC_GoodGenericStatus == callResults.status);
    assert_read_uint16(g_coId, NODEID_VAR_OPEN_COUNT_ITEM1, 0);
    SOPC_ClientHelper_CallMethodResults_Clear(1, &callResults);

    FILE* bin_file_fp = fopen(ITEM_1_PATH, "rb+");
    ck_assert_ptr_nonnull(bin_file_fp);
    int filedes = fileno(bin_file_fp);
    ck_assert(-1 != filedes);
    struct stat sb;
    int ret = fstat(filedes, &sb);
    ck_assert(-1 != ret);

    SOPC_ByteString bin_buffer_read = {0};
    bin_buffer_read.Length = (int32_t) sb.st_size;
    bin_buffer_read.Data = SOPC_Malloc((size_t) bin_buffer_read.Length);
    ck_assert_ptr_nonnull(bin_buffer_read.Data);
    size_t read_count = fread(bin_buffer_read.Data, 1, (size_t) bin_buffer_read.Length, bin_file_fp);
    ck_assert(read_count == (size_t) bin_buffer_read.Length);
    ret = fclose(bin_file_fp);
    ck_assert(0 == ret);
    ck_assert(SOPC_ByteString_Equal(&bin_buffer_read, ppDataToCompare));
    SOPC_ByteString_Delete(ppDataToCompare);
    SOPC_Free(bin_buffer_read.Data);
}
END_TEST

static void setup(void)
{
    bool initialized = SOPC_Common_EncodingConstantsGetInitialized();
    if (!initialized)
    {
        /* Configure the server to support file transfer message size of 100 Ko */
        SOPC_Common_EncodingConstants encConf = SOPC_Common_GetDefaultEncodingConstants();
        encConf.max_string_length = 102400; // 100 Ko
        bool bRes = SOPC_Common_SetEncodingConstants(encConf);
        ck_assert(bRes);
    }
    initialized = SOPC_CommonHelper_GetInitialized();
    if (!initialized)
    {
        /* Initialize the toolkit library */
        SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(NULL);
        ck_assert(SOPC_STATUS_OK == status);
    }
    /* Create client configuration */
    int32_t clientCfgId = client_create_configuration();
    // Connect client to server
    g_coId = SOPC_ClientHelper_CreateConnection(clientCfgId);
    ck_assert(g_coId > 0);
    client_check_no_such_file(ITEM_1_PATH);
    client_check_no_such_file(ITEM_2_PATH);
}

static void teardown(void)
{
    int res = remove_file(ITEM_1_PATH);
    ck_assert(0 == res);
    res = remove_file(ITEM_2_PATH);
    ck_assert(0 == res);
    int32_t dis = SOPC_ClientHelper_Disconnect(g_coId);
    ck_assert(0 == dis);
    SOPC_ClientHelper_Finalize();
    /* Clear the toolkit library */
    SOPC_CommonHelper_Clear();
}

static Suite* tests_file_transfer(void)
{
    Suite* s = NULL;
    TCase* tc_file_transfer;

    s = suite_create("Client Check File Transfer");

    tc_file_transfer = tcase_create("file_transfer");
    tcase_add_checked_fixture(tc_file_transfer, setup, teardown);
    tcase_add_test(tc_file_transfer, browse_file_type);
    tcase_add_test(tc_file_transfer, variables_init);
    tcase_add_test(tc_file_transfer, invalid_file_handle_read);
    tcase_add_test(tc_file_transfer, invalid_file_handle_write);
    tcase_add_test(tc_file_transfer, invalid_file_handle_get_position);
    tcase_add_test(tc_file_transfer, invalid_file_handle_set_position);
    tcase_add_test(tc_file_transfer, invalid_file_handle_close);
    tcase_add_test(tc_file_transfer, invalid_open_mode_bit_4);
    tcase_add_test(tc_file_transfer, invalid_open_mode_erase_existing);
    tcase_add_test(tc_file_transfer, invalid_open_mode_not_writable);
    tcase_add_test(tc_file_transfer, invalid_open_mode_not_readable);
    tcase_add_test(tc_file_transfer, invalid_write);
    tcase_add_test(tc_file_transfer, invalid_read);
    tcase_add_test(tc_file_transfer, get_position_empty_file);
    tcase_add_test(tc_file_transfer, abcd_file_read_at_the_end_of_file);
    tcase_add_test(tc_file_transfer, abcd_file_invalid_read_length);
    tcase_add_test(tc_file_transfer, abcd_file_from_position_2_read_cd);
    tcase_add_test(tc_file_transfer, abcd_file_replace_c_by_e_from_position_2_and_read_back);
    tcase_add_test(tc_file_transfer, abed_file_close_and_size_variable_is_reset_after_a_new_open);
    tcase_add_test(tc_file_transfer, write_in_two_files_and_get_position);
    tcase_add_test(tc_file_transfer, write_in_two_files_and_read_from_the_new_position);
    tcase_add_test(tc_file_transfer, write_a_binary_file_and_read_back);
    tcase_add_test(tc_file_transfer, write_a_binary_file_and_read_back_from_file_system);
    suite_add_tcase(s, tc_file_transfer);

    return s;
}

int main(void)
{
    int res = remove_file(ITEM_1_PATH);
    if (0 != res)
    {
        return EXIT_FAILURE;
    }
    res = remove_file(ITEM_2_PATH);
    if (0 != res)
    {
        return EXIT_FAILURE;
    }

    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_file_transfer());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
