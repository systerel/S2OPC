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
#include <stdlib.h>

#include "sopc_dataset_layer.h"
#include "sopc_dataset_ll_layer.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pub_source_variable.h"
#include "sopc_reader_layer.h"
#include "sopc_sub_target_variable.h"
#include "sopc_time.h"

/* COMMON DATA */

#define NB_VARS 5

SOPC_Variant varArr[NB_VARS] = {{true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 12071982}},
                                {true, SOPC_Byte_Id, SOPC_VariantArrayType_SingleValue, {.Byte = 239}},
                                {true, SOPC_UInt16_Id, SOPC_VariantArrayType_SingleValue, {.Uint16 = 64852}},
                                {true, SOPC_Float_Id, SOPC_VariantArrayType_SingleValue, {.Floatv = (float) 0.12}},
                                {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 369852}}};

/* Test network message layer */

#define ENCODED_DATA_SIZE 35
uint8_t encoded_network_msg_data[ENCODED_DATA_SIZE] = {
    0x7C, 0x2E, 0x03, 0x2A, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x01, 0x05, 0x00, 0x07, 0x2E, 0x34,
    0xB8, 0x00, 0x03, 0xEF, 0x05, 0x54, 0xFD, 0x0A, 0x8F, 0xC2, 0xF5, 0x3D, 0x07, 0xBC, 0xA4, 0x05, 0x00};

SOPC_Buffer encoded_network_msg = {ENCODED_DATA_SIZE, ENCODED_DATA_SIZE,       ENCODED_DATA_SIZE, 0,
                                   ENCODED_DATA_SIZE, encoded_network_msg_data};

uint8_t encoded_network_msg_multi_dsm_data[ENCODED_DATA_SIZE] = {
    0x7C, 0x2E, 0x03, 0x2A, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x05, // Modified number of DSM to 5
    0xFF, 0x00, 0x01, 0x05, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, 0x03, 0xEF, 0x05,
    0x54, 0xFD, 0x0A, 0x8F, 0xC2, 0xF5, 0x3D, 0x07, 0xBC, 0xA4, 0x05, 0x00};

SOPC_Buffer encoded_network_msg_multi_dsm = {
    ENCODED_DATA_SIZE, ENCODED_DATA_SIZE, ENCODED_DATA_SIZE, 0, ENCODED_DATA_SIZE, encoded_network_msg_multi_dsm_data};

#define NETWORK_MSG_PUBLISHER_ID 46
#define NETWORK_MSG_VERSION 12
#define NETWORK_MSG_GROUP_ID 42
#define NETWORK_MSG_GROUP_VERSION 1000

#define DATASET_MSG_WRITER_ID_BASE 255

#define NB_DATASET_MSG NB_VARS

#if NB_DATASET_MSG > NB_VARS
#error "A data set message should contain at least one variable in the test"
#endif

#include <stdio.h>

static void check_network_msg_content_uni_dsm(SOPC_Dataset_LL_NetworkMessage* nm)
{
    ck_assert_uint_eq(1, SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm));

    ck_assert_uint_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm)->type);
    ck_assert_uint_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm)->data.byte);

    ck_assert_uint_eq(NETWORK_MSG_VERSION, SOPC_Dataset_LL_NetworkMessage_Get_Version(nm));

    ck_assert_uint_eq(NETWORK_MSG_GROUP_ID, SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
    ck_assert_uint_eq(NETWORK_MSG_GROUP_VERSION, SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));

    SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);

    ck_assert_uint_eq((uint16_t)(DATASET_MSG_WRITER_ID_BASE), SOPC_Dataset_LL_DataSetMsg_Get_WriterId(msg_dsm));

    ck_assert_uint_eq(NB_VARS, SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(msg_dsm));

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(msg_dsm, i);
        int32_t comp = 0;
        SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], var, &comp);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, comp);
    }
}

static void check_network_msg_content_multi_dsm(SOPC_Dataset_LL_NetworkMessage* nm)
{
    ck_assert_uint_eq(NB_DATASET_MSG, SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm));

    ck_assert_uint_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm)->type);
    ck_assert_uint_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(nm)->data.byte);

    ck_assert_uint_eq(NETWORK_MSG_VERSION, SOPC_Dataset_LL_NetworkMessage_Get_Version(nm));

    ck_assert_uint_eq(NETWORK_MSG_GROUP_ID, SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
    ck_assert_uint_eq(NETWORK_MSG_GROUP_VERSION, SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));

    for (uint16_t imsg = 0; imsg < NB_DATASET_MSG; imsg++)
    {
        SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, imsg);

        uint16_t nb_vars = (uint16_t)(NB_VARS - imsg);

        ck_assert_uint_eq((uint16_t)(DATASET_MSG_WRITER_ID_BASE + imsg),
                          SOPC_Dataset_LL_DataSetMsg_Get_WriterId(msg_dsm));

        ck_assert_uint_eq(nb_vars, SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(msg_dsm));

        for (uint16_t i = 0; i < nb_vars; i++)
        {
            const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(msg_dsm, i);
            int32_t comp = 0;
            SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], var, &comp);
            ck_assert_int_eq(SOPC_STATUS_OK, status);
            ck_assert_int_eq(0, comp);
        }
    }
}

START_TEST(test_hl_network_msg_encode)
{
    // Initialize endianess for encoders
    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();

    bool res = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, 1);
    ck_assert_int_eq(true, res);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(nm, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_SetVersion(nm, NETWORK_MSG_VERSION);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);

    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm, (uint16_t)(DATASET_MSG_WRITER_ID_BASE));
    res = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm, NB_VARS);
    ck_assert_int_eq(true, res);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        SOPC_Variant* var = SOPC_Variant_Create();
        SOPC_ReturnStatus status = SOPC_Variant_Copy(var, &varArr[i]);
        ck_assert_int_eq(SOPC_STATUS_OK, status);

        res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(msg_dsm, var, i);
        ck_assert_int_eq(true, res);
    }

    // Check network message content
    check_network_msg_content_uni_dsm(nm);

    SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(nm, NULL);

    ck_assert_uint_eq(ENCODED_DATA_SIZE, buffer->length);

    for (uint32_t i = 0; i < buffer->length; i++)
    {
        ck_assert_uint_eq(encoded_network_msg_data[i], buffer->data[i]);
    }

    SOPC_Buffer_Delete(buffer);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

START_TEST(test_hl_network_msg_decode)
{
    // Initialize endianess for encoders
    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_UADP_NetworkMessage* uadp_nm = SOPC_UADP_NetworkMessage_Decode(&encoded_network_msg, NULL);
    ck_assert_ptr_nonnull(uadp_nm);

    // Check network message content
    check_network_msg_content_uni_dsm(uadp_nm->nm);

    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
}
END_TEST

START_TEST(test_hl_network_msg_encode_multi_dsm)
{
    // Initialize endianess for encoders
    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_Create(NB_DATASET_MSG);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(nm, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_SetVersion(nm, NETWORK_MSG_VERSION);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    for (uint16_t imsg = 0; imsg < NB_DATASET_MSG; imsg++)
    {
        SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, imsg);

        uint16_t nb_vars = (uint16_t)(NB_VARS - imsg);

        SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm, (uint16_t)(DATASET_MSG_WRITER_ID_BASE + imsg));
        bool res = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm, nb_vars);
        ck_assert_int_eq(true, res);

        for (uint16_t i = 0; i < nb_vars; i++)
        {
            SOPC_Variant* var = SOPC_Variant_Create();
            SOPC_ReturnStatus status = SOPC_Variant_Copy(var, &varArr[i]);
            ck_assert_int_eq(SOPC_STATUS_OK, status);

            res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(msg_dsm, var, i);
            ck_assert_int_eq(true, res);
        }
    }

    // Check network message content
    check_network_msg_content_multi_dsm(nm);

    SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(nm, false);

    // NOT SUPPORTED: otherwise use encoded_network_msg_multi_dsm_data
    ck_assert_ptr_null(buffer);

    SOPC_Buffer_Delete(buffer);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

START_TEST(test_hl_network_msg_decode_multi_dsm)
{
    // Initialize endianess for encoders
    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_UADP_NetworkMessage* uadp_nm = SOPC_UADP_NetworkMessage_Decode(&encoded_network_msg_multi_dsm, NULL);

    // NOT SUPPORTED
    ck_assert_ptr_null(uadp_nm);
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
}
END_TEST

/* Test target variable layer */

static SOPC_PubSubConfiguration* build_Sub_Config(SOPC_DataSetReader** out_dsr)
{
    ck_assert_ptr_nonnull(out_dsr);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    ck_assert_ptr_nonnull(config);

    bool allocSuccess = SOPC_PubSubConfiguration_Allocate_SubConnection_Array(config, 1);
    ck_assert_int_eq(true, allocSuccess);

    allocSuccess = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 1);
    ck_assert_int_eq(true, allocSuccess);

    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    // Subscriber connection

    allocSuccess = SOPC_PubSubConnection_Allocate_ReaderGroup_Array(connection, 1);
    ck_assert_int_eq(true, allocSuccess);

    // Create reader group
    SOPC_ReaderGroup* readerGroup = SOPC_PubSubConnection_Get_ReaderGroup_At(connection, 0);
    ck_assert_ptr_nonnull(readerGroup);

    allocSuccess = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, 1);
    ck_assert_int_eq(true, allocSuccess);

    ck_assert_uint_eq(1, SOPC_ReaderGroup_Nb_DataSetReader(readerGroup));

    SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, 0);
    ck_assert_ptr_nonnull(dataSetReader);
    *out_dsr = dataSetReader;

    SOPC_DataSetReader_Set_WriterGroupId(dataSetReader, NETWORK_MSG_GROUP_ID);
    SOPC_DataSetReader_Set_WriterGroupVersion(dataSetReader, NETWORK_MSG_GROUP_VERSION);
    SOPC_DataSetReader_Set_DataSetWriterId(dataSetReader, DATASET_MSG_WRITER_ID_BASE);

    allocSuccess =
        SOPC_DataSetReader_Allocate_FieldMetaData_Array(dataSetReader, SOPC_TargetVariablesDataType, NB_VARS);
    ck_assert_int_eq(true, allocSuccess);
    ck_assert_uint_eq(NB_VARS, SOPC_DataSetReader_Nb_FieldMetaData(dataSetReader));

    SOPC_FieldMetaData* fieldMetaData = NULL;

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, i);
        ck_assert_ptr_nonnull(fieldMetaData);

        /* FieldMetaData: type the field */
        SOPC_FieldMetaData_Set_ValueRank(fieldMetaData, -1);
        SOPC_FieldMetaData_Set_BuiltinType(fieldMetaData, varArr[i].BuiltInTypeId);

        /* FieldTarget: link to the source/target data */
        SOPC_FieldTarget* fieldTarget = SOPC_FieldMetaData_Get_TargetVariable(fieldMetaData);
        ck_assert_ptr_nonnull(fieldTarget);
        SOPC_NodeId* nodeId = SOPC_Malloc(sizeof(*nodeId));
        ck_assert_ptr_nonnull(nodeId);
        SOPC_NodeId_Initialize(nodeId);
        nodeId->IdentifierType = SOPC_IdentifierType_Numeric;
        nodeId->Namespace = 1;
        nodeId->Data.Numeric = i;
        SOPC_FieldTarget_Set_NodeId(fieldTarget, nodeId);
        SOPC_FieldTarget_Set_AttributeId(fieldTarget, 13); // Value => AttributeId=13
    }

    return config;
}

static SOPC_Dataset_LL_NetworkMessage* build_NetworkMessage_From_VarArr(void)
{
    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_Create(1);
    ck_assert_ptr_nonnull(nm);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(nm, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
    ck_assert_ptr_nonnull(dsm);

    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(dsm, DATASET_MSG_WRITER_ID_BASE);

    bool alloc = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(dsm, NB_VARS);
    ck_assert_int_eq(true, alloc);

    SOPC_Variant* variant;

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        variant = SOPC_Variant_Create();
        ck_assert_ptr_nonnull(variant);
        SOPC_ReturnStatus status = SOPC_Variant_Copy(variant, &varArr[i]);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, variant, i);
        ck_assert_int_eq(true, res);
    }

    return nm;
}

static bool setTargetVariablesCb(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    ck_assert_int_eq(NB_VARS, nbValues);
    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        OpcUa_WriteValue* wv = &nodesToWrite[i];
        ck_assert_uint_eq(13, wv->AttributeId);     // Value => AttributeId=13
        ck_assert_int_ge(0, wv->IndexRange.Length); // No index range
        ck_assert_int_eq(SOPC_IdentifierType_Numeric, wv->NodeId.IdentifierType);
        ck_assert_uint_eq(1, wv->NodeId.Namespace);
        ck_assert_uint_eq(i, wv->NodeId.Data.Numeric);
        int32_t comp = -1;
        SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], &wv->Value.Value, &comp);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, comp);
        ck_assert_uint_eq(0, wv->Value.Status); // Status OK
        ck_assert_uint_eq(0, wv->Value.ServerPicoSeconds);
        ck_assert_int_eq(0, wv->Value.ServerTimestamp);
        ck_assert_uint_eq(0, wv->Value.SourcePicoSeconds);
        ck_assert_int_eq(0, wv->Value.SourceTimestamp);
        OpcUa_WriteValue_Clear(wv);
    }
    SOPC_Free(nodesToWrite);

    return true;
}

static bool setTargetVariablesCb_TargetTest_called = false;

static bool setTargetVariablesCb_TargetTest(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    setTargetVariablesCb_TargetTest_called = true;
    return setTargetVariablesCb(nodesToWrite, nbValues);
}

START_TEST(test_target_variable_layer)
{
    SOPC_Dataset_LL_NetworkMessage* nm = build_NetworkMessage_From_VarArr();
    SOPC_DataSetReader* dsr = NULL;
    SOPC_PubSubConfiguration* config = build_Sub_Config(&dsr);
    ck_assert_ptr_nonnull(config);

    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(setTargetVariablesCb_TargetTest);
    ck_assert_ptr_nonnull(targetConfig);

    bool setVariables =
        SOPC_SubTargetVariable_SetVariables(targetConfig, dsr, SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0));
    ck_assert_int_eq(true, setVariables);
    ck_assert_int_eq(true, setTargetVariablesCb_TargetTest_called);

    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

/* Test Subscriber reader layer */

static bool setTargetVariablesCb_ReaderTest_called = false;

static bool setTargetVariablesCb_ReaderTest(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    setTargetVariablesCb_ReaderTest_called = true;
    return setTargetVariablesCb(nodesToWrite, nbValues);
}

START_TEST(test_subscriber_reader_layer)
{
    // Initialize endianess for encoders
    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_DataSetReader* dsr = NULL;
    SOPC_PubSubConfiguration* config = build_Sub_Config(&dsr);
    ck_assert_ptr_nonnull(config);

    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);

    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(setTargetVariablesCb_ReaderTest);

    // NOMINAL
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_ReturnStatus status = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(true, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;

    // PUBLISHER ID
    SOPC_DataSetReader_Set_PublisherId_UInteger(dsr, NETWORK_MSG_PUBLISHER_ID + 1);

    status = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_DataSetReader_Set_PublisherId_UInteger(dsr, NETWORK_MSG_PUBLISHER_ID);

    // WRONG GROUP ID
    SOPC_DataSetReader_Set_WriterGroupId(dsr, NETWORK_MSG_GROUP_ID + 1);

    status = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_DataSetReader_Set_WriterGroupId(dsr, NETWORK_MSG_GROUP_ID);

    // WRONG GROUP VERSION
    SOPC_DataSetReader_Set_WriterGroupVersion(dsr, NETWORK_MSG_GROUP_VERSION + 1);

    status = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_DataSetReader_Set_WriterGroupVersion(dsr, NETWORK_MSG_GROUP_VERSION);

    // WRONG DATA SET WRITER ID
    SOPC_DataSetReader_Set_DataSetWriterId(dsr, DATASET_MSG_WRITER_ID_BASE + 1);

    status = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_DataSetReader_Set_DataSetWriterId(dsr, DATASET_MSG_WRITER_ID_BASE);

    // WRONG DATA SET DATA COMBINATION
    SOPC_DataSetReader_Set_WriterGroupId(dsr, NETWORK_MSG_GROUP_ID + 1);
    SOPC_DataSetReader_Set_WriterGroupVersion(dsr, NETWORK_MSG_GROUP_VERSION + 1);
    SOPC_DataSetReader_Set_DataSetWriterId(dsr, DATASET_MSG_WRITER_ID_BASE + 1);

    status = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_DataSetReader_Set_WriterGroupId(dsr, NETWORK_MSG_GROUP_ID);
    SOPC_DataSetReader_Set_DataSetWriterId(dsr, DATASET_MSG_WRITER_ID_BASE);
    SOPC_DataSetReader_Set_WriterGroupVersion(dsr, NETWORK_MSG_GROUP_VERSION);

    // UNINIT
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

/* Test source variable layer */
static SOPC_PubSubConfiguration* build_Pub_Config(SOPC_PublishedDataSet** out_pds)
{
    ck_assert_ptr_nonnull(out_pds);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    ck_assert_ptr_nonnull(config);

    bool allocSuccess = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 1);
    ck_assert_int_eq(true, allocSuccess);

    // Publisher data set
    *out_pds = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 0);
    ck_assert_ptr_nonnull(*out_pds);

    SOPC_PublishedDataSet_Init(*out_pds, SOPC_PublishedDataItemsDataType, NB_VARS);

    SOPC_FieldMetaData* fieldMetaData = NULL;

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        fieldMetaData = SOPC_PublishedDataSet_Get_FieldMetaData_At(*out_pds, i);
        ck_assert_ptr_nonnull(fieldMetaData);

        /* FieldMetaData: type the field */
        SOPC_FieldMetaData_Set_ValueRank(fieldMetaData, -1);
        SOPC_FieldMetaData_Set_BuiltinType(fieldMetaData, varArr[i].BuiltInTypeId);

        /* FieldTarget: link to the source/target data */
        SOPC_PublishedVariable* sourceVar = SOPC_FieldMetaData_Get_PublishedVariable(fieldMetaData);
        ck_assert_ptr_nonnull(sourceVar);
        SOPC_NodeId* nodeId = SOPC_Malloc(sizeof(*nodeId));
        ck_assert_ptr_nonnull(nodeId);
        SOPC_NodeId_Initialize(nodeId);
        nodeId->IdentifierType = SOPC_IdentifierType_Numeric;
        nodeId->Namespace = 1;
        nodeId->Data.Numeric = i;
        SOPC_PublishedVariable_Set_NodeId(sourceVar, nodeId);
        SOPC_PublishedVariable_Set_AttributeId(sourceVar, 13); // Value => AttributeId=13
    }

    return config;
}

static void check_returned_DataValues(SOPC_DataValue* dataValues)
{
    ck_assert_ptr_nonnull(dataValues);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        SOPC_DataValue* dataValue = &dataValues[i];

        ck_assert_int_eq(varArr[i].ArrayType, dataValue->Value.ArrayType);
        ck_assert_int_eq(varArr[i].BuiltInTypeId, dataValue->Value.BuiltInTypeId);
        int32_t comp = -1;
        SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], &dataValue->Value, &comp);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, comp);
        SOPC_DataValue_Clear(dataValue);
    }
    SOPC_Free(dataValues);
}

static SOPC_DataValue* GetSourceVariablesResponse(SOPC_PubSheduler_GetVariableRequestContext* requestResponse)
{
    SOPC_DataValue* ldv = NULL;

    if (NULL == requestResponse)
    {
        return NULL;
    }

    SOPC_PubSheduler_GetVariableRequestContext* requestContext =
        (SOPC_PubSheduler_GetVariableRequestContext*) requestResponse;

    if (NULL == requestResponse->ldv)
    {
        SOPC_Free(requestContext);
        return NULL;
    }

    check_returned_DataValues(requestResponse->ldv);

    ldv = requestContext->ldv;

    SOPC_Free(requestContext);

    ldv = NULL;

    return ldv;
}

static SOPC_ReturnStatus GetSourceVariablesRequest(
    SOPC_EventHandler* eventHandler, // message queue where response must be sent
    uintptr_t msgCtx,                // messageCtx, used by scheduler when it received response
    OpcUa_ReadValueId* lrv,
    int32_t nbValues)
{
    // Note: Return result is mandatory. If SOPC_STATUS_OK is not returned, then
    // READY status is set to messageCtx. So, request can be performed.
    // Else, BUSY is set to messageCtx. Next request will be ignored.
    // This is important to avoid memory issue in case of
    // treatment of request by services thread takes a long time.

    if (NULL == lrv || 0 >= nbValues)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_PubSheduler_GetVariableRequestContext* requestContext =
        SOPC_Calloc(1, sizeof(SOPC_PubSheduler_GetVariableRequestContext));

    if (NULL == requestContext)
    {
        return SOPC_STATUS_NOK;
    }
    requestContext->msgCtxt = msgCtx;            // Message context, forward by "0" timer event
    requestContext->eventHandler = eventHandler; // Message queue
    requestContext->ldv = NULL;                  // Datavalue request result
    requestContext->NoOfNodesToRead = nbValues;  // Use to alloc SOPC_DataValue by GetResponse

    /* Simulate response */
    ck_assert(nbValues <= NB_VARS);
    ck_assert(0 < nbValues);
    requestContext->ldv = SOPC_Calloc((size_t) nbValues, sizeof(*requestContext->ldv));
    ck_assert(NULL != requestContext->ldv);
    for (uint32_t i = 0; i < (uint32_t) nbValues; i++)
    {
        SOPC_DataValue* dataValue = &requestContext->ldv[i];
        SOPC_DataValue_Initialize(dataValue);

        OpcUa_ReadValueId* readValue = &lrv[i];
        ck_assert_uint_eq(13, readValue->AttributeId);     // Value => AttributeId=13
        ck_assert_int_ge(0, readValue->IndexRange.Length); // No index range
        ck_assert_int_eq(SOPC_IdentifierType_Numeric, readValue->NodeId.IdentifierType);
        ck_assert_uint_eq(1, readValue->NodeId.Namespace);
        ck_assert_uint_eq(i, readValue->NodeId.Data.Numeric);

        dataValue->Value.ArrayType = varArr[i].ArrayType;
        dataValue->Value.BuiltInTypeId = varArr[i].BuiltInTypeId;
        dataValue->Value.Value = varArr[i].Value;

        OpcUa_ReadValueId_Clear(lrv);
    }
    SOPC_Free(lrv);

    // Call directly response callback
    GetSourceVariablesResponse(requestContext);

    return SOPC_STATUS_OK;
}

START_TEST(test_source_variable_layer)
{
    SOPC_PublishedDataSet* pds = NULL;
    SOPC_PubSubConfiguration* config = build_Pub_Config(&pds);

    SOPC_PubSourceVariableConfig* sourceConfig =
        SOPC_PubSourceVariableConfig_Create(GetSourceVariablesRequest, GetSourceVariablesResponse);
    SOPC_ReturnStatus result = SOPC_PubSourceVariable_GetVariables(NULL, 0, sourceConfig, pds);

    ck_assert(result == SOPC_STATUS_OK);

    SOPC_PubSourceVariableConfig_Delete(sourceConfig);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

/* Test Publisher data set layer */

static SOPC_PubSubConfiguration* build_PubConfig_From_VarArr(SOPC_WriterGroup** group)
{
    ck_assert_ptr_nonnull(group);

    SOPC_PubSubConfiguration* config = SOPC_PubSubConfiguration_Create();
    ck_assert_ptr_nonnull(config);

    bool allocSucceeded = SOPC_PubSubConfiguration_Allocate_PubConnection_Array(config, 1);
    ck_assert_int_eq(true, allocSucceeded);
    ck_assert_uint_eq(1, SOPC_PubSubConfiguration_Nb_PubConnection(config));

    allocSucceeded = SOPC_PubSubConfiguration_Allocate_PublishedDataSet_Array(config, 1);
    ck_assert_int_eq(true, allocSucceeded);
    ck_assert_uint_eq(1, SOPC_PubSubConfiguration_Nb_PublishedDataSet(config));

    // Create the PublishedDataSet
    SOPC_PublishedDataSet* pubDataSet = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 0);
    ck_assert_ptr_nonnull(pubDataSet);
    allocSucceeded = SOPC_PublishedDataSet_Init(pubDataSet, SOPC_PublishedDataItemsDataType, NB_VARS);
    ck_assert_int_eq(true, allocSucceeded);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        SOPC_FieldMetaData* field = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet, i);
        ck_assert_ptr_nonnull(field);
        SOPC_FieldMetaData_Set_BuiltinType(field, varArr[i].BuiltInTypeId);
        SOPC_FieldMetaData_Set_ValueRank(field, -1); // SingleValue
    }

    // Create the Connection with 1 WriterGroup / DataSetWriter with associated PublishedDataSet
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    SOPC_PubSubConnection_Set_PublisherId_UInteger(connection, NETWORK_MSG_PUBLISHER_ID);

    allocSucceeded = SOPC_PubSubConnection_Allocate_WriterGroup_Array(connection, 1);
    ck_assert_int_eq(true, allocSucceeded);

    *group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, 0);
    ck_assert_ptr_nonnull(*group);
    SOPC_WriterGroup_Set_Id(*group, NETWORK_MSG_GROUP_ID);
    SOPC_WriterGroup_Set_Version(*group, NETWORK_MSG_GROUP_VERSION);

    allocSucceeded = SOPC_WriterGroup_Allocate_DataSetWriter_Array(*group, 1);
    ck_assert_int_eq(true, allocSucceeded);

    SOPC_DataSetWriter* dataSet = SOPC_WriterGroup_Get_DataSetWriter_At(*group, 0);
    ck_assert_ptr_nonnull(dataSet);

    SOPC_DataSetWriter_Set_Id(dataSet, DATASET_MSG_WRITER_ID_BASE);

    SOPC_DataSetWriter_Set_DataSet(dataSet, pubDataSet);

    return config;
}

static void equal_NetworkMessage(SOPC_Dataset_LL_NetworkMessage* left, SOPC_Dataset_LL_NetworkMessage* right)
{
    ck_assert_ptr_nonnull(left);
    ck_assert_ptr_nonnull(right);

    ck_assert_int_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(left));
    ck_assert_int_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(right));

    ck_assert_int_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(left)->data.byte);
    ck_assert_int_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(right)->data.byte);

    ck_assert_uint_eq(SOPC_Dataset_LL_NetworkMessage_Get_Version(left),
                      SOPC_Dataset_LL_NetworkMessage_Get_Version(right));

    ck_assert_uint_eq(SOPC_Dataset_LL_NetworkMessage_Get_GroupId(left),
                      SOPC_Dataset_LL_NetworkMessage_Get_GroupId(right));

    ck_assert_uint_eq(SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(left),
                      SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(right));

    // Other properties not verified since not set from the WriterGroup ?

    uint8_t leftNbMsgs = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(left);
    uint8_t rightNbMsgs = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(right);
    ck_assert_uint_eq(leftNbMsgs, rightNbMsgs);
    ck_assert_uint_eq(1, leftNbMsgs);

    SOPC_Dataset_LL_DataSetMessage* leftDsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(left, 0);
    SOPC_Dataset_LL_DataSetMessage* rightDsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(right, 0);

    ck_assert_uint_eq(SOPC_Dataset_LL_DataSetMsg_Get_WriterId(leftDsm),
                      SOPC_Dataset_LL_DataSetMsg_Get_WriterId(rightDsm));

    uint16_t leftNbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(leftDsm);
    uint16_t rightNbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(rightDsm);
    ck_assert_uint_eq(leftNbFields, rightNbFields);
    ck_assert_uint_eq(NB_VARS, leftNbFields);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        const SOPC_Variant* leftVar = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(leftDsm, i);
        const SOPC_Variant* rightVar = SOPC_Dataset_LL_DataSetMsg_Get_Variant_At(rightDsm, i);
        int32_t comp = 0;

        SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], leftVar, &comp);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, comp);
        status = SOPC_Variant_Compare(&varArr[i], rightVar, &comp);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, comp);
    }
}

START_TEST(test_dataset_layer)
{
    // Initialize endianess for encoders
    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_WriterGroup* group = NULL;
    SOPC_PubSubConfiguration* config = build_PubConfig_From_VarArr(&group);
    ck_assert_ptr_nonnull(config);
    ck_assert_ptr_nonnull(group);

    SOPC_Dataset_NetworkMessage* nm = SOPC_Create_NetworkMessage_From_WriterGroup(group);
    ck_assert_ptr_nonnull(nm);

    SOPC_PublishedDataSet* pubDataSet = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 0);
    ck_assert_ptr_nonnull(pubDataSet);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        SOPC_FieldMetaData* field = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet, i);
        SOPC_Variant* var = SOPC_Variant_Create();
        SOPC_ReturnStatus status = SOPC_Variant_Copy(var, &varArr[i]);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        SOPC_NetworkMessage_Set_Variant_At(nm, 0, i, var, field);
    }

    SOPC_Dataset_LL_NetworkMessage* expectedNm = build_NetworkMessage_From_VarArr();
    equal_NetworkMessage(nm, expectedNm);

    SOPC_Dataset_LL_NetworkMessage_Delete(expectedNm);
    SOPC_Delete_NetworkMessage(nm);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

int main(void)
{
    int number_failed;
    SRunner* sr;

    Suite* suite = suite_create("PubSub modules test suite");

    TCase* tc_hl_network_msg = tcase_create("Network message layer");
    suite_add_tcase(suite, tc_hl_network_msg);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_encode);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_encode_multi_dsm);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode_multi_dsm);

    TCase* tc_sub_target_variable_layer = tcase_create("Subscriber target variable layer");
    suite_add_tcase(suite, tc_sub_target_variable_layer);
    tcase_add_test(tc_sub_target_variable_layer, test_target_variable_layer);

    TCase* tc_sub_reader_layer = tcase_create("Subscriber reader layer");
    suite_add_tcase(suite, tc_sub_reader_layer);
    tcase_add_test(tc_sub_reader_layer, test_subscriber_reader_layer);

    TCase* tc_pub_source_variable_layer = tcase_create("Publisher source variable layer");
    suite_add_tcase(suite, tc_pub_source_variable_layer);
    tcase_add_test(tc_pub_source_variable_layer, test_source_variable_layer);

    TCase* tc_dataset_layer = tcase_create("Publisher Dataset layer");
    suite_add_tcase(suite, tc_dataset_layer);
    tcase_add_test(tc_dataset_layer, test_dataset_layer);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
