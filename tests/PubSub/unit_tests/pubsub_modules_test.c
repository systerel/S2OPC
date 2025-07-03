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
 * \brief Entry point for tests. Tests use libcheck.
 * https://libcheck.github.io/check/doc/check_html/check_3.html
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <math.h>
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
#include "sopc_time_reference.h"
#include "sopc_types.h"

/* COMMON DATA */

#define NB_VARS 5

static SOPC_Variant varArr[NB_VARS] = {SOPC_VARIANT_UINT32(12071982), SOPC_VARIANT_BYTE(239),
                                       SOPC_VARIANT_UINT16(64852), SOPC_VARIANT_FLOAT((float) 0.12),
                                       SOPC_VARIANT_UINT32(369852)};

/* Test network message layer JSON encoded */

#define ENCODED_DATA_SIZE_JSON 548
#define NB_VARS_JSON 9

static SOPC_Byte gSampleText[] = "This is a text !";
static SOPC_Variant varArrJSON[NB_VARS_JSON] = {SOPC_VARIANT_BOOL(true),
                                                SOPC_VARIANT_UINT32(64839),
                                                SOPC_VARIANT_INT32(-65133),
                                                SOPC_VARIANT_DOUBLE((double) 5462.16515561), // ~ 5462.165156
                                                SOPC_VARIANT_FLOAT((float) 546216515561),    // ~ 5.462165094e+11
                                                SOPC_VARIANT_FLOAT((float) 1.0 / 0.0),
                                                SOPC_VARIANT_FLOAT((float) -1.0 / 0.0),
                                                SOPC_VARIANT_DOUBLE(NAN),
                                                SOPC_VARIANT_STRING(gSampleText)};

static const uint8_t encoded_network_msg_json[ENCODED_DATA_SIZE_JSON] =
    "{"
    "\"MessageId\":\"42-0\","
    "\"MessageType\":\"ua-data\","
    "\"PublisherId\":\"46\","
    "\"Messages\":["
    "{"
    "\"DataSetWriterId\":255,"
    "\"MessageType\":\"ua-keyframe\","
    "\"Payload\":{"
    "\"0-0\":{"
    "\"Type\":1,"
    "\"Body\":true"
    "},"
    "\"0-1\":{"
    "\"Type\":7,"
    "\"Body\":64839"
    "},"
    "\"0-2\":{"
    "\"Type\":6,"
    "\"Body\":-65133"
    "},"
    "\"0-3\":{"
    "\"Type\":11,"
    "\"Body\":5462.165156"
    "},"
    "\"0-4\":{"
    "\"Type\":10,"
    "\"Body\":5.462165094e+11"
    "},"
    "\"0-5\":{"
    "\"Type\":10,"
    "\"Body\":\"Infinity\""
    "},"
    "\"0-6\":{"
    "\"Type\":10,"
    "\"Body\":\"-Infinity\""
    "},"
    "\"0-7\":{"
    "\"Type\":11,"
    "\"Body\":\"NaN\""
    "},"
    "\"0-8\":{"
    "\"Type\":12,"
    "\"Body\":\"This is a text !\""
    "}"
    "}"
    "},"
    "{"
    "\"DataSetWriterId\":10,"
    "\"MessageType\":\"ua-keyframe\","
    "\"Payload\":{"
    "\"1-0\":{"
    "\"Type\":1,"
    "\"Body\":true"
    "}"
    "}"
    "}"
    "]"
    "}";

/* Test network message layer UADP encoded*/

#define ENCODED_DATA_SIZE 37
uint8_t encoded_network_msg_data[ENCODED_DATA_SIZE] = {0x71, 0x2E, 0x03, 0x2A, 0x00, 0xE8, 0x03, 0x00, 0x00,
                                                       // Payload header/Message Count & DSM WriterIds
                                                       0x01, 0xFF, 0x00,
                                                       // DSM1 (Flags, nbFields)
                                                       0x09, 0x00, 0x00, 0x05, 0x00,
                                                       // DSM1 = vars 1..5
                                                       0x07, 0x2E, 0x34, 0xB8, 0x00, 0x03, 0xEF, 0x05, 0x54, 0xFD, 0x0A,
                                                       0x8F, 0xC2, 0xF5, 0x3D, 0x07, 0xBC, 0xA4, 0x05, 0x00};

SOPC_Buffer encoded_network_msg = {ENCODED_DATA_SIZE, ENCODED_DATA_SIZE,       ENCODED_DATA_SIZE, 0,
                                   ENCODED_DATA_SIZE, encoded_network_msg_data};

#define ENCODED_KEEP_ALIVE_DATA 16
uint8_t encoded_network_msg_keep_alive[ENCODED_KEEP_ALIVE_DATA] = {
    0x71,                   // Flags + Version (NETWORK_MSG_VERSION)
    0x2E,                   // PublisherId (NETWORK_MSG_PUBLISHER_ID)
    0x03,                   // GroupFlags
    0x2A, 0x00,             // WriterGroupId (NETWORK_MSG_GROUP_ID)
    0xE8, 0x03, 0x00, 0x00, // GroupVersion (NETWORK_MSG_GROUP_VERSION)
    0x01,                   // Payload header/Message Count
    0xFF, 0x00,             // Payload header/ DSM WriterIds
    0x89,                   // DSM Header/ dataSet Flags1 plus
    0x03,                   // DSM Header/ dataSet Flags2
    0x00, 0x00              // DSM Sequence number
};

SOPC_Buffer encoded_network_keep_alive_msg = {ENCODED_KEEP_ALIVE_DATA, ENCODED_KEEP_ALIVE_DATA,
                                              ENCODED_KEEP_ALIVE_DATA, 0,
                                              ENCODED_KEEP_ALIVE_DATA, encoded_network_msg_keep_alive};

#define ENCODED_DATA_SIZE2 63
uint8_t encoded_network_msg_data2[ENCODED_DATA_SIZE2] = {0x71, 0x2E, 0x03, 0x2A, 0x00, 0xE8, 0x03, 0x00, 0x00,
                                                         // DSM Count & DSM WriterIds
                                                         0x02, 0xFF, 0x00, 0x00, 0x01,
                                                         // DSM1 & 2 size
                                                         0x19, 0x00, 0x14, 0x00,
                                                         // DSM1 (Flags, nbFields)
                                                         0x09, 0x00, 0x00, 0x05, 0x00,
                                                         // DSM1 = vars 1..5
                                                         0x07, 0x2E, 0x34, 0xB8, 0x00, 0x03, 0xEF, 0x05, 0x54, 0xFD,
                                                         0x0A, 0x8F, 0xC2, 0xF5, 0x3D, 0x07, 0xBC, 0xA4, 0x05, 0x00,
                                                         // DSM2 (Flags, nbFields)
                                                         0x09, 0x00, 0x00, 0x04, 0x00,
                                                         // DSM2 = vars 1..4
                                                         0x07, 0x2E, 0x34, 0xB8, 0x00, 0x03, 0xEF, 0x05, 0x54, 0xFD,
                                                         0x0A, 0x8F, 0xC2, 0xF5, 0x3D};

SOPC_Buffer encoded_network_msg2 = {ENCODED_DATA_SIZE2, ENCODED_DATA_SIZE2,       ENCODED_DATA_SIZE2, 0,
                                    ENCODED_DATA_SIZE2, encoded_network_msg_data2};

#define ENCODED_DSM_PRE_FIELD_SIZE 5u // DataSet Flags1 + number of fields + Sequence Number

// Content of encoded variables (see "varArr") for Multi-DataSetMessage
#define ENCODED_VAR0_SIZE 5u
#define ENCODED_VAR1_SIZE 2u
#define ENCODED_VAR2_SIZE 3u
#define ENCODED_VAR3_SIZE 5u
#define ENCODED_VAR4_SIZE 5u
#define ENCODED_VARS0_1_SIZE (ENCODED_VAR0_SIZE + ENCODED_VAR1_SIZE)
#define ENCODED_VARS0_2_SIZE (ENCODED_VARS0_1_SIZE + ENCODED_VAR2_SIZE)
#define ENCODED_VARS0_3_SIZE (ENCODED_VARS0_2_SIZE + ENCODED_VAR3_SIZE)
#define ENCODED_VARS0_4_SIZE (ENCODED_VARS0_3_SIZE + ENCODED_VAR4_SIZE)

uint8_t encoded_vars_dsm[ENCODED_VARS0_4_SIZE] = {0x07, 0x2E, 0x34, 0xB8, 0x00, 0x03, 0xEF, 0x05, 0x54, 0xFD,
                                                  0x0A, 0x8F, 0xC2, 0xF5, 0x3D, 0x07, 0xBC, 0xA4, 0x05, 0x00};

uint8_t encoded_network_msg_multi_dsm_data[] = {
    0x71,                                                       // Flags + Version (NETWORK_MSG_VERSION)
    0x2E,                                                       // PublisherId (NETWORK_MSG_PUBLISHER_ID)
    0x03,                                                       // GroupFlags
    0x2A, 0x00,                                                 // WriterGroupId (NETWORK_MSG_GROUP_ID)
    0xE8, 0x03, 0x00, 0x00,                                     // GroupVersion (NETWORK_MSG_GROUP_VERSION)
    0x05,                                                       // Payload header/Message Count
    0xFF, 0x00,                                                 // DSM WriterId[0]
    0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01,             // DSM WriterId[1..4]
    0x17, 0x00, 0x12, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x08, 0x00, // DSM Sizes [0..4]
    // DSM[0] (Flags, nbFields)
    0x01, 0x05, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF, 0x05, 0x54, 0xFD,                   // VAR2 & 3
    0x0A, 0x8F, 0xC2, 0xF5, 0x3D,                   // VAR4
    0x07, 0xBC, 0xA4, 0x05, 0x00,                   // VAR5
    // DSM[1] (Flags, nbFields)
    0x01, 0x04, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF, 0x05, 0x54, 0xFD,                   // VAR2 & 3
    0x0A, 0x8F, 0xC2, 0xF5, 0x3D,                   // VAR4
    // DSM[2] (Flags, nbFields)
    0x01, 0x03, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF, 0x05, 0x54, 0xFD,                   // VAR2 & 3
    // DSM[3] (Flags, nbFields)
    0x01, 0x02, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF,                                     // VAR2
    // DSM[4] (Flags, nbFields)
    0x01, 0x01, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00 // VAR1
};

uint8_t encoded_network_msg_no_pubId[] = {
    0x61,                                                       // Flags + Version (NETWORK_MSG_VERSION)
                                                                // No PublisherId
    0x03,                                                       // GroupFlags
    0x2A, 0x00,                                                 // WriterGroupId (NETWORK_MSG_GROUP_ID)
    0xE8, 0x03, 0x00, 0x00,                                     // GroupVersion (NETWORK_MSG_GROUP_VERSION)
    0x05,                                                       // Payload header/Message Count
    0xFF, 0x00,                                                 // DSM WriterId[0]
    0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01,             // DSM WriterId[1..4]
    0x17, 0x00, 0x12, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x08, 0x00, // DSM Sizes [0..4]
    // DSM[0] (Flags, nbFields)
    0x01, 0x05, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF, 0x05, 0x54, 0xFD,                   // VAR2 & 3
    0x0A, 0x8F, 0xC2, 0xF5, 0x3D,                   // VAR4
    0x07, 0xBC, 0xA4, 0x05, 0x00,                   // VAR5
    // DSM[1] (Flags, nbFields)
    0x01, 0x04, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF, 0x05, 0x54, 0xFD,                   // VAR2 & 3
    0x0A, 0x8F, 0xC2, 0xF5, 0x3D,                   // VAR4
    // DSM[2] (Flags, nbFields)
    0x01, 0x03, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF, 0x05, 0x54, 0xFD,                   // VAR2 & 3
    // DSM[3] (Flags, nbFields)
    0x01, 0x02, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00, // VAR1
    0x03, 0xEF,                                     // VAR2
    // DSM[4] (Flags, nbFields)
    0x01, 0x01, 0x00, 0x07, 0x2E, 0x34, 0xB8, 0x00 // VAR1
};

static SOPC_TargetVariableCtx* gTargetVariable_dsm1 = NULL;
static SOPC_TargetVariableCtx* gTargetVariable_dsm2 = NULL;

#define NETWORK_MSG_PUBLISHER_ID 46
#define NETWORK_MSG_VERSION 1
#define NETWORK_MSG_GROUP_ID 42
#define NETWORK_MSG_GROUP_VERSION 1000

#define DATASET_MSG_WRITER_ID_BASE 255

#define NB_DATASET_MSG NB_VARS

#define BYTE1(x) ((uint8_t)(((x) &0xFF)))
#define BYTE2(x) ((uint8_t)(((x) &0xFF00) >> 8))
#define BYTE3(x) ((uint8_t)(((x) &0xFF0000) >> 16))
#define BYTE4(x) ((uint8_t)(((x) &0xFF000000) >> 24))

#if NB_DATASET_MSG > NB_VARS
#error "A data set message should contain at least one variable in the test"
#endif

#include <stdio.h>

static SOPC_PubSubConfiguration* build_Sub_Config(SOPC_DataSetReader** out_dsr, size_t nbDsr);

static SOPC_NetworkMessage_Error_Code Decode_NetworkMessage_NoSecu(SOPC_Buffer* buffer,
                                                                   SOPC_PubSubConnection* connection,
                                                                   SOPC_UADP_NetworkMessage** uadp_nm)
{
    const SOPC_UADP_NetworkMessage_Reader_Configuration readerConf = {
        .pGetSecurity_Func = NULL,
        .callbacks = SOPC_Reader_NetworkMessage_Default_Readers,
        .checkDataSetMessageSN_Func = NULL,
        .updateTimeout_Func = NULL,
        .targetConfig = NULL,
        .targetVariable_Func = NULL};

    SOPC_NetworkMessage_Error_Code errorCode =
        SOPC_UADP_NetworkMessage_Decode(buffer, &readerConf, connection, uadp_nm);
    return errorCode;
}

static void check_network_msg_content_uni_keep_alive_dsm(SOPC_Dataset_LL_NetworkMessage* nm)
{
    ck_assert_uint_eq(1, SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm));
    const SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader_Const(nm);
    ck_assert_ptr_nonnull(header);

    ck_assert_uint_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->type);
    ck_assert_uint_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->data.byte);

    ck_assert_uint_eq(NETWORK_MSG_VERSION, SOPC_Dataset_LL_NetworkMessage_GetVersion(header));

    ck_assert_uint_eq(NETWORK_MSG_GROUP_ID, SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
    ck_assert_uint_eq(NETWORK_MSG_GROUP_VERSION, SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));

    SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);

    ck_assert_uint_eq((uint16_t)(DATASET_MSG_WRITER_ID_BASE), SOPC_Dataset_LL_DataSetMsg_Get_WriterId(msg_dsm));
    const SOPC_DataSet_LL_UadpDataSetMessageContentMask* conf = SOPC_Dataset_LL_DataSetMsg_Get_ContentMask(msg_dsm);
    ck_assert_ptr_nonnull(conf);

    ck_assert_uint_eq(DataSet_LL_MessageType_KeepAlive, conf->dataSetMessageType);

    ck_assert_uint_eq(0, SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(msg_dsm));
}

static void check_network_msg_content_uni_dsm(SOPC_Dataset_LL_NetworkMessage* nm)
{
    ck_assert_uint_eq(1, SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm));
    const SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader_Const(nm);
    ck_assert_ptr_nonnull(header);

    ck_assert_uint_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->type);
    ck_assert_uint_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->data.byte);

    ck_assert_uint_eq(NETWORK_MSG_VERSION, SOPC_Dataset_LL_NetworkMessage_GetVersion(header));

    ck_assert_uint_eq(NETWORK_MSG_GROUP_ID, SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm));
    ck_assert_uint_eq(NETWORK_MSG_GROUP_VERSION, SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm));

    SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);

    ck_assert_uint_eq((uint16_t)(DATASET_MSG_WRITER_ID_BASE), SOPC_Dataset_LL_DataSetMsg_Get_WriterId(msg_dsm));

    ck_assert_uint_eq(NB_VARS, SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(msg_dsm));

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(msg_dsm, i);
        int32_t comp = 0;
        SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], var, &comp);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        ck_assert_int_eq(0, comp);
    }
}

static void check_network_msg_content_multi_dsm(SOPC_Dataset_LL_NetworkMessage* nm)
{
    ck_assert_uint_eq(NB_DATASET_MSG, SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(nm));
    const SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader_Const(nm);
    ck_assert_ptr_nonnull(header);

    ck_assert_uint_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->type);
    ck_assert_uint_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header)->data.byte);

    ck_assert_uint_eq(NETWORK_MSG_VERSION, SOPC_Dataset_LL_NetworkMessage_GetVersion(header));

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
            const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(msg_dsm, i);
            int32_t comp = 0;
            SOPC_ReturnStatus status = SOPC_Variant_Compare(&varArr[i], var, &comp);
            ck_assert_int_eq(SOPC_STATUS_OK, status);
            ck_assert_int_eq(0, comp);
        }
    }
}

START_TEST(test_tc_tools_test)
{
    SOPC_HighRes_TimeReference* t1 = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference* t2 = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_Copy(t2, t1);

    int64_t delta_us;

    ck_assert_ptr_nonnull(t1);
    ck_assert_ptr_nonnull(t2);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t2, 2500, -1);

    // Check that t2 is close to t1 + 2500 us
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t1, t2);
    ck_assert_int_lt(delta_us - 2500, 10);

    SOPC_HighRes_TimeReference_SleepUntil(t2);

    // Check that the minimum amount of time has elapsed
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t1, NULL);
    ck_assert_int_gt(delta_us, 2499);

    SOPC_HighRes_TimeReference_Delete(&t1);
    SOPC_HighRes_TimeReference_Delete(&t2);
}

START_TEST(test_hl_network_msg_encode_json)
{
    SOPC_Helper_Endianness_Check();

    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);

    bool res = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, 2);
    ck_assert_int_eq(true, res);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(header, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_SetVersion(header, NETWORK_MSG_VERSION);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    SOPC_Dataset_LL_DataSetMessage* msg_dsm0 = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
    SOPC_Dataset_LL_DataSetMessage* msg_dsm1 = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 1);

    /* 1st Dataset Message */
    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm0, (uint16_t)(DATASET_MSG_WRITER_ID_BASE));
    res = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm0, NB_VARS_JSON);
    ck_assert_int_eq(true, res);

    // Fill in variants of 1st Dataset Message
    for (uint16_t i = 0; i < NB_VARS_JSON; i++)
    {
        res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(msg_dsm0, &varArrJSON[i], i);
        ck_assert_int_eq(true, res);
    }

    /* 2nd Dataset Message */
    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm1, (uint16_t)(10));
    res = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm1, 1);

    // Fill in variant of 2nd Dataset Message
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(msg_dsm1, &varArrJSON[0], 0);
    ck_assert_int_eq(true, res);

    /* Encode */
    SOPC_Buffer* buffer = NULL;
    SOPC_NetworkMessage_Error_Code errorCode = SOPC_JSON_NetworkMessage_Encode(nm, NULL, &buffer);

    ck_assert_int_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);

    /* Control encoding result */
    // Check Size
    ck_assert_uint_eq(ENCODED_DATA_SIZE_JSON, buffer->length);

    // Check buffer values
    for (uint32_t i = 0; i < buffer->length; i++)
    {
        ck_assert_uint_eq(encoded_network_msg_json[i], buffer->data[i]);
    }

    SOPC_Buffer_Delete(buffer);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

START_TEST(test_hl_network_msg_encode)
{
    SOPC_Helper_Endianness_Check();

    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);

    bool res = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, 1);
    ck_assert_int_eq(true, res);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(header, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_SetVersion(header, NETWORK_MSG_VERSION);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);

    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm, (uint16_t)(DATASET_MSG_WRITER_ID_BASE));
    res = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm, NB_VARS);
    ck_assert_int_eq(true, res);

    SOPC_DataSet_LL_UadpDataSetMessageContentMask conf = {
        .validFlag = true,
        .fieldEncoding = DataSet_LL_FieldEncoding_Variant,
        .dataSetMessageSequenceNumberFlag = true,
        .statusFlag = false,
        .configurationVersionMajorVersionFlag = false,
        .configurationVersionMinorFlag = false,
        .dataSetMessageType = DataSet_LL_MessageType_KeyFrame,
        .timestampFlag = false,
        .picoSecondsFlag = false,
    };

    SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(msg_dsm, &conf);
    SOPC_Dataset_LL_DataSetMsg_Set_EnableEmission(msg_dsm, true);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(msg_dsm, &varArr[i], i);
        ck_assert_int_eq(true, res);
    }

    // Check network message content
    check_network_msg_content_uni_dsm(nm);

    SOPC_Buffer* buffer = NULL;
    SOPC_Buffer* buffer_payload = NULL;
    SOPC_NetworkMessage_Error_Code errorCode =
        SOPC_UADP_NetworkMessage_Encode_Buffers(nm, NULL, &buffer, &buffer_payload);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);
    ck_assert_ptr_nonnull(buffer_payload);

    errorCode = SOPC_UADP_NetworkMessage_BuildFinalMessage(NULL, buffer, &buffer_payload);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);
    ck_assert_ptr_null(buffer_payload);

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
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[1];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 1);
    ck_assert_ptr_nonnull(config);
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    SOPC_UADP_NetworkMessage* uadp_nm = NULL;
    SOPC_NetworkMessage_Error_Code code = Decode_NetworkMessage_NoSecu(&encoded_network_msg, connection, &uadp_nm);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, code);
    ck_assert_ptr_nonnull(uadp_nm);

    // Check network message content
    check_network_msg_content_uni_dsm(uadp_nm->nm);

    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

START_TEST(test_hl_network_msg_encode_uni_keep_alive_dsm)
{
    SOPC_Helper_Endianness_Check();

    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_CreateEmpty();
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);

    bool res = SOPC_Dataset_LL_NetworkMessage_Allocate_DataSetMsg_Array(nm, 1);
    ck_assert_int_eq(true, res);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(header, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_SetVersion(header, NETWORK_MSG_VERSION);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
    SOPC_DataSet_LL_UadpDataSetMessageContentMask conf = {
        .validFlag = true,
        .fieldEncoding = DataSet_LL_FieldEncoding_Variant,
        .dataSetMessageSequenceNumberFlag = true,
        .statusFlag = false,
        .configurationVersionMajorVersionFlag = false,
        .configurationVersionMinorFlag = false,
        .dataSetMessageType = DataSet_LL_MessageType_KeepAlive,
        .timestampFlag = false,
        .picoSecondsFlag = false,
    };

    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm, (uint16_t)(DATASET_MSG_WRITER_ID_BASE));
    SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(msg_dsm, &conf);
    SOPC_Dataset_LL_DataSetMsg_Set_EnableEmission(msg_dsm, true);

    // Check network message content
    check_network_msg_content_uni_keep_alive_dsm(nm);

    SOPC_Buffer* buffer = NULL;
    SOPC_Buffer* buffer_payload = NULL;
    SOPC_NetworkMessage_Error_Code errorCode =
        SOPC_UADP_NetworkMessage_Encode_Buffers(nm, NULL, &buffer, &buffer_payload);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);
    ck_assert_ptr_nonnull(buffer_payload);

    errorCode = SOPC_UADP_NetworkMessage_BuildFinalMessage(NULL, buffer, &buffer_payload);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);
    ck_assert_ptr_null(buffer_payload);

    ck_assert_uint_eq(ENCODED_KEEP_ALIVE_DATA, buffer->length);

    for (uint32_t i = 0; i < buffer->length; i++)
    {
        ck_assert_uint_eq(encoded_network_msg_keep_alive[i], buffer->data[i]);
    }

    SOPC_Buffer_Delete(buffer);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

START_TEST(test_hl_network_msg_decode_uni_keep_alive_dsm)
{
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[1];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 1);
    ck_assert_ptr_nonnull(config);
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    SOPC_UADP_NetworkMessage* uadp_nm = NULL;
    SOPC_NetworkMessage_Error_Code code =
        Decode_NetworkMessage_NoSecu(&encoded_network_keep_alive_msg, connection, &uadp_nm);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, code);
    ck_assert_ptr_nonnull(uadp_nm);

    check_network_msg_content_uni_keep_alive_dsm(uadp_nm->nm);

    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(&encoded_network_keep_alive_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

START_TEST(test_hl_network_msg_encode_multi_dsm)
{
    /*
     * In this test 5 DSM are created in the same message:
     * - DSM0 contains varArr[0..4]
     * - DSM1 contains varArr[0..3]
     * - DSM2 contains varArr[0..2]
     * - DSM3 contains varArr[0..1]
     * - DSM4 contains varArr[0..0]
     */

    SOPC_Helper_Endianness_Check();

    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_Create(NB_DATASET_MSG, 1);
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(header, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_SetVersion(header, NETWORK_MSG_VERSION);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    for (uint16_t imsg = 0; imsg < NB_DATASET_MSG; imsg++)
    {
        SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, imsg);

        uint16_t nb_vars = (uint16_t)(NB_VARS - imsg);

        SOPC_DataSet_LL_UadpDataSetMessageContentMask conf = {
            .validFlag = true,
            .fieldEncoding = DataSet_LL_FieldEncoding_Variant,
            .dataSetMessageSequenceNumberFlag = true,
            .statusFlag = false,
            .configurationVersionMajorVersionFlag = false,
            .configurationVersionMinorFlag = false,
            .dataSetMessageType = DataSet_LL_MessageType_KeyFrame,
            .timestampFlag = false,
            .picoSecondsFlag = false,
        };

        SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(msg_dsm, &conf);
        SOPC_Dataset_LL_DataSetMsg_Set_EnableEmission(msg_dsm, true);
        SOPC_Dataset_LL_DataSetMsg_Set_WriterId(msg_dsm, (uint16_t)(DATASET_MSG_WRITER_ID_BASE + imsg));
        bool res = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(msg_dsm, nb_vars);
        ck_assert_int_eq(true, res);

        for (uint16_t i = 0; i < nb_vars; i++)
        {
            res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(msg_dsm, &varArr[i], i);
            ck_assert_int_eq(true, res);
        }
    }

    // Check network message content
    check_network_msg_content_multi_dsm(nm);

    SOPC_Buffer* buffer = NULL;
    SOPC_Buffer* buffer_payload = NULL;
    SOPC_NetworkMessage_Error_Code errorCode =
        SOPC_UADP_NetworkMessage_Encode_Buffers(nm, NULL, &buffer, &buffer_payload);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);
    ck_assert_ptr_nonnull(buffer_payload);

    errorCode = SOPC_UADP_NetworkMessage_BuildFinalMessage(NULL, buffer, &buffer_payload);

    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, errorCode);
    ck_assert_ptr_nonnull(buffer);
    ck_assert_ptr_null(buffer_payload);

#if 0 // switch to debug
    {
        fprintf(stderr, "Buffer = (%d)[", buffer->length);
        for (uint32_t i = 0; i < buffer->length ; i++)
        {
            fprintf(stderr, "0x%02X, ", buffer->data[i]);
        }
        fprintf(stderr, "]\n");
    }
#endif

    uint8_t* data = buffer->data;
    ck_assert_uint_eq(data[0] & 0x0F, NETWORK_MSG_VERSION); // Version
    ck_assert_uint_eq(data[0] & 0xF0, 0x70);                // Flags1 (PayloadHdr + GroupHdr+ PublishId)
    // No ExtendedFlags1/ExtendedFlags2
    // In this case : PublisherId is of DataType Byte
    ck_assert_uint_eq(data[1], NETWORK_MSG_PUBLISHER_ID); // PublisherId
    // No DataSetClassId
    // GroupFlags
    ck_assert_uint_eq(data[2], 3);                                // WriterGroupId en + GroupVersion en
    ck_assert_uint_eq(data[3], BYTE1(NETWORK_MSG_GROUP_ID));      // WriterGroupId
    ck_assert_uint_eq(data[4], BYTE2(NETWORK_MSG_GROUP_ID));      // WriterGroupId
    ck_assert_uint_eq(data[5], BYTE1(NETWORK_MSG_GROUP_VERSION)); // GroupVersion
    ck_assert_uint_eq(data[6], BYTE2(NETWORK_MSG_GROUP_VERSION)); // GroupVersion
    ck_assert_uint_eq(data[7], BYTE3(NETWORK_MSG_GROUP_VERSION)); // GroupVersion
    ck_assert_uint_eq(data[8], BYTE4(NETWORK_MSG_GROUP_VERSION)); // GroupVersion
    // No NetworkMessageNumber
    // No SequenceNumber
    // PayloadHeader/Count
    ck_assert_uint_eq(data[9], BYTE1(NB_VARS)); // Count
    // PayloadHeader/DataSetWriterIds[0]
    ck_assert_uint_eq(data[10], BYTE1(DATASET_MSG_WRITER_ID_BASE + 0)); // DataSetWriterIds[0]
    ck_assert_uint_eq(data[11], BYTE2(DATASET_MSG_WRITER_ID_BASE + 0)); // DataSetWriterIds[0]
    ck_assert_uint_eq(data[12], BYTE1(DATASET_MSG_WRITER_ID_BASE + 1)); // DataSetWriterIds[1]
    ck_assert_uint_eq(data[13], BYTE2(DATASET_MSG_WRITER_ID_BASE + 1)); // DataSetWriterIds[1]
    ck_assert_uint_eq(data[14], BYTE1(DATASET_MSG_WRITER_ID_BASE + 2)); // DataSetWriterIds[2]
    ck_assert_uint_eq(data[15], BYTE2(DATASET_MSG_WRITER_ID_BASE + 2)); // DataSetWriterIds[2]
    ck_assert_uint_eq(data[16], BYTE1(DATASET_MSG_WRITER_ID_BASE + 3)); // DataSetWriterIds[3]
    ck_assert_uint_eq(data[17], BYTE2(DATASET_MSG_WRITER_ID_BASE + 3)); // DataSetWriterIds[3]
    ck_assert_uint_eq(data[18], BYTE1(DATASET_MSG_WRITER_ID_BASE + 4)); // DataSetWriterIds[4]
    ck_assert_uint_eq(data[19], BYTE2(DATASET_MSG_WRITER_ID_BASE + 4)); // DataSetWriterIds[4]
    // No NetworkMessage Header Extended
    // No security Header
    // Payload:
    //   Size[0..4]
    ck_assert_uint_eq(data[20], BYTE1(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_4_SIZE));
    ck_assert_uint_eq(data[21], BYTE2(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_4_SIZE));
    ck_assert_uint_eq(data[22], BYTE1(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_3_SIZE));
    ck_assert_uint_eq(data[23], BYTE2(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_3_SIZE));
    ck_assert_uint_eq(data[24], BYTE1(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_2_SIZE));
    ck_assert_uint_eq(data[25], BYTE2(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_2_SIZE));
    ck_assert_uint_eq(data[26], BYTE1(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_1_SIZE));
    ck_assert_uint_eq(data[27], BYTE2(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VARS0_1_SIZE));
    ck_assert_uint_eq(data[28], BYTE1(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VAR0_SIZE));
    ck_assert_uint_eq(data[29], BYTE2(ENCODED_DSM_PRE_FIELD_SIZE + ENCODED_VAR0_SIZE));
    static const uint8_t DS_Flags1 = 9;
    // Note: each DSM starts with DataSetFlags set to 1 "Valid / DataValue=Variant"
    // and is followed by 2 bytes giving the number of fields (different for each DSM)

    // Check DSM[0] containing var[0..4]
    unsigned int dsm_idx = 30;
    ck_assert_uint_eq(data[dsm_idx], BYTE1(DS_Flags1));
    // DSM[0]/ Sequence Number equal to 0
    ck_assert_uint_eq(data[dsm_idx + 1], BYTE1(0));
    ck_assert_uint_eq(data[dsm_idx + 2], BYTE2(0));
    // DSM[0]/nb DataSet (5 fields)
    ck_assert_uint_eq(data[dsm_idx + 3], BYTE1(5));
    ck_assert_uint_eq(data[dsm_idx + 4], BYTE2(5));
    dsm_idx = dsm_idx + ENCODED_DSM_PRE_FIELD_SIZE;
    ck_assert_mem_eq(encoded_vars_dsm, &data[dsm_idx], ENCODED_VARS0_4_SIZE);

    // Check DSM[1] containing var[0..3]
    dsm_idx = dsm_idx + ENCODED_VARS0_4_SIZE;
    ck_assert_uint_eq(data[dsm_idx], BYTE1(DS_Flags1));
    // DSM[1]/ Sequence Number equal to 0
    ck_assert_uint_eq(data[dsm_idx + 1], BYTE1(0));
    ck_assert_uint_eq(data[dsm_idx + 2], BYTE2(0));
    // DSM[1]/nb DataSet (4 fields)
    ck_assert_uint_eq(data[dsm_idx + 3], BYTE1(4));
    ck_assert_uint_eq(data[dsm_idx + 4], BYTE2(4));
    dsm_idx = dsm_idx + ENCODED_DSM_PRE_FIELD_SIZE;
    ck_assert_mem_eq(encoded_vars_dsm, &data[dsm_idx], ENCODED_VARS0_3_SIZE);

    // Check DSM[2] containing var[0..2]
    dsm_idx = dsm_idx + ENCODED_VARS0_3_SIZE;
    ck_assert_uint_eq(data[dsm_idx], BYTE1(DS_Flags1)); // Valid / DataValue, no status or config version
    // DSM[2]/ Sequence Number equal to 0
    ck_assert_uint_eq(data[dsm_idx + 1], BYTE1(0));
    ck_assert_uint_eq(data[dsm_idx + 2], BYTE2(0));
    // DSM[2]/nb DataSet (3 fields)
    ck_assert_uint_eq(data[dsm_idx + 3], BYTE1(3));
    ck_assert_uint_eq(data[dsm_idx + 4], BYTE2(3));
    dsm_idx = dsm_idx + ENCODED_DSM_PRE_FIELD_SIZE;
    ck_assert_mem_eq(encoded_vars_dsm, &data[dsm_idx], ENCODED_VARS0_2_SIZE);

    // Check DSM[3] containing var[0..1]
    dsm_idx = dsm_idx + ENCODED_VARS0_2_SIZE;
    ck_assert_uint_eq(data[dsm_idx], BYTE1(DS_Flags1)); // Valid / DataValue, no status or config version
    // DSM[3]/ Sequence Number equal to 0
    ck_assert_uint_eq(data[dsm_idx + 1], BYTE1(0));
    ck_assert_uint_eq(data[dsm_idx + 2], BYTE2(0));
    // DSM[3]/nb DataSet (2 fields)
    ck_assert_uint_eq(data[dsm_idx + 3], BYTE1(2));
    ck_assert_uint_eq(data[dsm_idx + 4], BYTE2(2));
    dsm_idx = dsm_idx + ENCODED_DSM_PRE_FIELD_SIZE;
    ck_assert_mem_eq(encoded_vars_dsm, &data[dsm_idx], ENCODED_VARS0_1_SIZE);

    // Check DSM[4] containing var[0..0]
    dsm_idx = dsm_idx + ENCODED_VARS0_1_SIZE;
    ck_assert_uint_eq(data[dsm_idx], BYTE1(DS_Flags1)); // Valid / DataValue, no status or config version
    // DSM[4]/ Sequence Number equal to 0
    ck_assert_uint_eq(data[dsm_idx + 1], BYTE1(0));
    ck_assert_uint_eq(data[dsm_idx + 2], BYTE2(0));
    // DSM[4]/nb DataSet (1 field)
    ck_assert_uint_eq(data[dsm_idx + 3], BYTE1(1));
    ck_assert_uint_eq(data[dsm_idx + 4], BYTE2(1));
    dsm_idx = dsm_idx + ENCODED_DSM_PRE_FIELD_SIZE;
    ck_assert_mem_eq(encoded_vars_dsm, &data[dsm_idx], ENCODED_VAR0_SIZE);

    SOPC_Buffer_Delete(buffer);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

START_TEST(test_hl_network_msg_decode_multi_dsm)
{
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[5];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 5);
    ck_assert_ptr_nonnull(config);
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    const uint32_t bufferLen = sizeof(encoded_network_msg_multi_dsm_data);
    SOPC_Buffer* buffer = SOPC_Buffer_Create(bufferLen);
    ck_assert_ptr_nonnull(buffer);

    SOPC_Buffer_Write(buffer, encoded_network_msg_multi_dsm_data, bufferLen);
    SOPC_Buffer_SetPosition(buffer, 0);

    SOPC_UADP_NetworkMessage* uadp_nm = NULL;
    SOPC_NetworkMessage_Error_Code code = Decode_NetworkMessage_NoSecu(buffer, connection, &uadp_nm);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, code);
    ck_assert_ptr_nonnull(uadp_nm);

    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    ck_assert_ptr_nonnull(nm);

    // Check that Network message is as expected
    const SOPC_Dataset_LL_PublisherId* pubId = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header);
    ck_assert_ptr_nonnull(pubId);
    ck_assert_uint_eq(pubId->type, DataSet_LL_PubId_Byte_Id);
    ck_assert_uint_eq(pubId->data.byte, NETWORK_MSG_PUBLISHER_ID);

    const uint8_t nm_version = SOPC_Dataset_LL_NetworkMessage_GetVersion(header);
    ck_assert_uint_eq(nm_version, NETWORK_MSG_VERSION);

    const uint16_t nm_groupId = SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm);
    ck_assert_uint_eq(nm_groupId, NETWORK_MSG_GROUP_ID);

    const uint32_t nm_groupVersion = SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm);
    ck_assert_uint_eq(nm_groupVersion, NETWORK_MSG_GROUP_VERSION);

    for (uint16_t imsg = 0; imsg < NB_DATASET_MSG; imsg++)
    {
        const SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, imsg);
        const uint16_t nb_vars = (uint16_t)(NB_VARS - imsg);
        const uint16_t writerId = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(msg_dsm);
        ck_assert_uint_eq(writerId, (uint16_t)(DATASET_MSG_WRITER_ID_BASE + imsg));

        for (uint16_t i = 0; i < nb_vars; i++)
        {
            int32_t comparison = -1;
            const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(msg_dsm, i);
            SOPC_ReturnStatus status = SOPC_Variant_Compare(var, &varArr[i], &comparison);
            ck_assert_int_eq(SOPC_STATUS_OK, status);
            ck_assert_int_eq(comparison, 0);
        }
    }
    uint32_t position;
    SOPC_ReturnStatus status = SOPC_Buffer_GetPosition(buffer, &position);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(position, bufferLen);
    SOPC_Buffer_Delete(buffer);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

START_TEST(test_hl_network_msg_decode_multi_dsm_nok)
{
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[5];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 5);
    ck_assert_ptr_nonnull(config);
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    const uint32_t bufferLen = sizeof(encoded_network_msg_multi_dsm_data);
    SOPC_Buffer* buffer = SOPC_Buffer_Create(bufferLen);
    ck_assert_ptr_nonnull(buffer);

    SOPC_ReturnStatus status = SOPC_Buffer_Write(buffer, encoded_network_msg_multi_dsm_data, bufferLen);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Modify size header of DSM2
    uint8_t oldValue;
    status = SOPC_Buffer_SetPosition(buffer, 22);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_Read(&oldValue, buffer, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    oldValue++; // corrupt size
    status = SOPC_Buffer_SetPosition(buffer, 22);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_Write(buffer, &oldValue, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_UADP_NetworkMessage* uadp_nm = NULL;
    SOPC_NetworkMessage_Error_Code code = Decode_NetworkMessage_NoSecu(buffer, connection, &uadp_nm);

    // Check that Network message is not decoded
    ck_assert_uint_eq(SOPC_UADP_NetworkMessage_Error_Read_DsmSizeCheck_Failed, code);
    ck_assert_ptr_null(uadp_nm);

    SOPC_Buffer_Delete(buffer);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

START_TEST(test_hl_network_msg_decode_null_pubid)
{
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[5];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 5);
    ck_assert_ptr_nonnull(config);
    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    ck_assert_ptr_nonnull(connection);

    const uint32_t bufferLen = sizeof(encoded_network_msg_no_pubId);
    SOPC_Buffer* buffer = SOPC_Buffer_Create(bufferLen);
    ck_assert_ptr_nonnull(buffer);

    SOPC_Buffer_Write(buffer, encoded_network_msg_no_pubId, bufferLen);
    SOPC_Buffer_SetPosition(buffer, 0);

    SOPC_UADP_NetworkMessage* uadp_nm = NULL;
    SOPC_NetworkMessage_Error_Code code = Decode_NetworkMessage_NoSecu(buffer, connection, &uadp_nm);
    ck_assert_ptr_nonnull(uadp_nm);
    ck_assert_uint_eq(SOPC_NetworkMessage_Error_Code_None, code);
    SOPC_Dataset_LL_NetworkMessage* nm = uadp_nm->nm;
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    ck_assert_ptr_nonnull(nm);

    // Check that Network message is as expected
    const SOPC_Dataset_LL_PublisherId* pubId = SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(header);
    ck_assert_ptr_null(pubId);

    const uint8_t nm_version = SOPC_Dataset_LL_NetworkMessage_GetVersion(header);
    ck_assert_uint_eq(nm_version, NETWORK_MSG_VERSION);

    const uint16_t nm_groupId = SOPC_Dataset_LL_NetworkMessage_Get_GroupId(nm);
    ck_assert_uint_eq(nm_groupId, NETWORK_MSG_GROUP_ID);

    const uint32_t nm_groupVersion = SOPC_Dataset_LL_NetworkMessage_Get_GroupVersion(nm);
    ck_assert_uint_eq(nm_groupVersion, NETWORK_MSG_GROUP_VERSION);

    for (uint16_t imsg = 0; imsg < NB_DATASET_MSG; imsg++)
    {
        const SOPC_Dataset_LL_DataSetMessage* msg_dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, imsg);
        const uint16_t nb_vars = (uint16_t)(NB_VARS - imsg);
        const uint16_t writerId = SOPC_Dataset_LL_DataSetMsg_Get_WriterId(msg_dsm);
        ck_assert_uint_eq(writerId, (uint16_t)(DATASET_MSG_WRITER_ID_BASE + imsg));

        for (uint16_t i = 0; i < nb_vars; i++)
        {
            int32_t comparison = -1;
            const SOPC_Variant* var = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(msg_dsm, i);
            SOPC_ReturnStatus status = SOPC_Variant_Compare(var, &varArr[i], &comparison);
            ck_assert_int_eq(SOPC_STATUS_OK, status);
            ck_assert_int_eq(comparison, 0);
        }
    }
    uint32_t position;
    SOPC_ReturnStatus status = SOPC_Buffer_GetPosition(buffer, &position);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_uint_eq(position, bufferLen);
    SOPC_Buffer_Delete(buffer);
    SOPC_UADP_NetworkMessage_Delete(uadp_nm);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

/* Test target variable layer */

static SOPC_PubSubConfiguration* build_Sub_Config(SOPC_DataSetReader** out_dsr, size_t nbDsr)
{
    ck_assert_ptr_nonnull(out_dsr);
    ck_assert(nbDsr > 0);
    ck_assert(nbDsr <= NB_VARS);

    SOPC_PubSub_ArrayDimension arrDimension = {.valueRank = -1, .arrayDimensions = NULL};
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

    allocSuccess = SOPC_ReaderGroup_Allocate_DataSetReader_Array(readerGroup, (uint8_t) nbDsr);
    ck_assert_int_eq(true, allocSuccess);

    ck_assert_uint_eq(nbDsr, SOPC_ReaderGroup_Nb_DataSetReader(readerGroup));

    SOPC_ReaderGroup_Set_GroupId(readerGroup, NETWORK_MSG_GROUP_ID);
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, NETWORK_MSG_GROUP_VERSION);

    // Setup DSR
    for (uint16_t iDsr = 0; iDsr < nbDsr; iDsr++)
    {
        ck_assert_int_gt(NB_DATASET_MSG, iDsr);
        SOPC_DataSetReader* dataSetReader = SOPC_ReaderGroup_Get_DataSetReader_At(readerGroup, (uint8_t) iDsr);
        ck_assert_ptr_nonnull(dataSetReader);
        const uint16_t writerId = (uint16_t)(DATASET_MSG_WRITER_ID_BASE + iDsr);
        SOPC_DataSetReader_Set_DataSetWriterId(dataSetReader, writerId);

        const uint8_t nbVars = (uint8_t)(iDsr < NB_VARS ? NB_VARS - iDsr : 1);
        allocSuccess =
            SOPC_DataSetReader_Allocate_FieldMetaData_Array(dataSetReader, SOPC_TargetVariablesDataType, nbVars);
        ck_assert_int_eq(true, allocSuccess);
        ck_assert_uint_eq(nbVars, SOPC_DataSetReader_Nb_FieldMetaData(dataSetReader));

        SOPC_FieldMetaData* fieldMetaData = NULL;

        for (uint16_t i = 0; i < nbVars; i++)
        {
            fieldMetaData = SOPC_DataSetReader_Get_FieldMetaData_At(dataSetReader, i);
            ck_assert_ptr_nonnull(fieldMetaData);

            /* FieldMetaData: type the field */
            SOPC_FieldMetaData_ArrayDimension_Move(fieldMetaData, &arrDimension);
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

        out_dsr[iDsr] = dataSetReader;
    }

    return config;
}

static SOPC_Dataset_LL_NetworkMessage* build_NetworkMessage_From_VarArr(void)
{
    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_Create(1, 1);
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    ck_assert_ptr_nonnull(nm);

    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_Byte(header, NETWORK_MSG_PUBLISHER_ID);

    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, NETWORK_MSG_GROUP_ID);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, NETWORK_MSG_GROUP_VERSION);

    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
    ck_assert_ptr_nonnull(dsm);

    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(dsm, DATASET_MSG_WRITER_ID_BASE);

    bool alloc = SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(dsm, NB_VARS);
    ck_assert_int_eq(true, alloc);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &varArr[i], i);
        ck_assert_int_eq(true, res);
    }
    return nm;
}

static bool setTargetVariablesCb(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues, int nbExpected)
{
    if (nbExpected > 0)
    {
        ck_assert_int_eq(nbExpected, nbValues);
    }
    for (uint16_t i = 0; i < nbExpected; i++)
    {
        const OpcUa_WriteValue* wv = &nodesToWrite[i];
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
    }
    return true;
}

static bool setTargetVariablesCb_TargetTest_called = false;

static bool setTargetVariablesCb_TargetTest(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues)
{
    setTargetVariablesCb_TargetTest_called = true;
    return setTargetVariablesCb(nodesToWrite, nbValues, NB_VARS);
}

START_TEST(test_target_variable_layer)
{
    SOPC_Dataset_LL_NetworkMessage* nm = build_NetworkMessage_From_VarArr();
    SOPC_DataSetReader* dsr[1];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 1);
    ck_assert_ptr_nonnull(config);

    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(&setTargetVariablesCb_TargetTest);
    ck_assert_ptr_nonnull(targetConfig);

    SOPC_TargetVariableCtx* targetVariable = SOPC_SubTargetVariable_TargetVariablesCtx_Create(dsr[0]);
    ck_assert_ptr_nonnull(targetVariable);

    bool setVariables = SOPC_SubTargetVariable_SetVariables(targetConfig, targetVariable, *dsr,
                                                            SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0));
    ck_assert_int_eq(true, setVariables);
    ck_assert_int_eq(true, setTargetVariablesCb_TargetTest_called);

    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_SubTargetVariable_TargetVariableCtx_Delete(&targetVariable);
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
}
END_TEST

/* Test Subscriber reader layer */

static bool setTargetVariablesCb_ReaderTest_called = false;
static int32_t setTargetVariablesCb_ReaderTest_nbVal = 0;

static bool setTargetVariablesCb_ReaderTest(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues)
{
    setTargetVariablesCb_ReaderTest_called = true;
    setTargetVariablesCb_ReaderTest_nbVal += nbValues;
    return setTargetVariablesCb(nodesToWrite, nbValues, NB_VARS);
}

static bool setTargetVariablesCb_ReaderTest_Multi(const OpcUa_WriteValue* nodesToWrite, const int32_t nbValues)
{
    setTargetVariablesCb_ReaderTest_called = true;
    setTargetVariablesCb_ReaderTest_nbVal += nbValues;
    return setTargetVariablesCb(nodesToWrite, nbValues, 0);
}

static SOPC_TargetVariableCtx* getGlobalTargetVariable(const SOPC_Conf_PublisherId* pubId,
                                                       const uint16_t groupId,
                                                       const uint16_t writerId)
{
    if (NETWORK_MSG_PUBLISHER_ID == pubId->data.uint && NETWORK_MSG_GROUP_ID == groupId &&
        writerId == DATASET_MSG_WRITER_ID_BASE)
    {
        return gTargetVariable_dsm1;
    }
    else if (NETWORK_MSG_PUBLISHER_ID == pubId->data.uint && NETWORK_MSG_GROUP_ID == groupId &&
             writerId == (DATASET_MSG_WRITER_ID_BASE + 1))
    {
        return gTargetVariable_dsm2;
    }
    else
    {
        // Should not go there
        ck_assert(false);
    }
}

START_TEST(test_subscriber_reader_layer_multi_dsm)
{
    // Same test as above, with 2 DSM
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[2];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 2);
    ck_assert_ptr_nonnull(config);

    gTargetVariable_dsm1 = SOPC_SubTargetVariable_TargetVariablesCtx_Create(dsr[0]);
    ck_assert_ptr_nonnull(gTargetVariable_dsm1);
    gTargetVariable_dsm2 = SOPC_SubTargetVariable_TargetVariablesCtx_Create(dsr[1]);
    ck_assert_ptr_nonnull(gTargetVariable_dsm2);

    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);

    SOPC_SubTargetVariableConfig* targetConfig =
        SOPC_SubTargetVariableConfig_Create(&setTargetVariablesCb_ReaderTest_Multi);

    // NOMINAL
    setTargetVariablesCb_ReaderTest_called = false;
    setTargetVariablesCb_ReaderTest_nbVal = 0;
    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg2, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_NetworkMessage_Error_Code_None, code);
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(&encoded_network_msg2, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(true, setTargetVariablesCb_ReaderTest_called);
    ck_assert_int_eq(9, setTargetVariablesCb_ReaderTest_nbVal);

    setTargetVariablesCb_ReaderTest_called = false;
    setTargetVariablesCb_ReaderTest_nbVal = 0;

    SOPC_ReaderGroup* readerGroup = SOPC_DataSetReader_Get_ReaderGroup(*dsr);

    // Change PUBLISHER ID and check that message has not been fully parsed.
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, NETWORK_MSG_PUBLISHER_ID + 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg2, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg2, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    ck_assert_int_eq(0, setTargetVariablesCb_ReaderTest_nbVal);
    setTargetVariablesCb_ReaderTest_called = false;
    setTargetVariablesCb_ReaderTest_nbVal = 0;
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, NETWORK_MSG_PUBLISHER_ID);

    // WRONG DATA SET WRITER ID on second DSM
    SOPC_DataSetReader_Set_DataSetWriterId(dsr[1], DATASET_MSG_WRITER_ID_BASE - 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg2, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_NetworkMessage_Error_Code_None, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg2, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(true, setTargetVariablesCb_ReaderTest_called);
    ck_assert_int_eq(5, setTargetVariablesCb_ReaderTest_nbVal);
    setTargetVariablesCb_ReaderTest_called = false;
    setTargetVariablesCb_ReaderTest_nbVal = 0;
    SOPC_DataSetReader_Set_DataSetWriterId(dsr[1], DATASET_MSG_WRITER_ID_BASE + 1);

    // WRONG DATA SET WRITER ID on first DSM
    SOPC_DataSetReader_Set_DataSetWriterId(dsr[0], DATASET_MSG_WRITER_ID_BASE - 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg2, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_NetworkMessage_Error_Code_None, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg2, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(true, setTargetVariablesCb_ReaderTest_called);
    ck_assert_int_eq(4, setTargetVariablesCb_ReaderTest_nbVal);
    setTargetVariablesCb_ReaderTest_called = false;
    setTargetVariablesCb_ReaderTest_nbVal = 0;
    SOPC_DataSetReader_Set_DataSetWriterId(dsr[0], DATASET_MSG_WRITER_ID_BASE);

    // UNINIT
    SOPC_SubTargetVariable_TargetVariableCtx_Delete(&gTargetVariable_dsm1);
    SOPC_SubTargetVariable_TargetVariableCtx_Delete(&gTargetVariable_dsm2);
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);
}
END_TEST

START_TEST(test_subscriber_reader_layer)
{
    SOPC_NetworkMessage_Error_Code code = SOPC_NetworkMessage_Error_Code_None;
    SOPC_Helper_Endianness_Check();

    SOPC_DataSetReader* dsr[1];
    SOPC_PubSubConfiguration* config = build_Sub_Config(dsr, 1);
    ck_assert_ptr_nonnull(config);

    gTargetVariable_dsm1 = SOPC_SubTargetVariable_TargetVariablesCtx_Create(dsr[0]);
    ck_assert_ptr_nonnull(gTargetVariable_dsm1);

    SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_SubConnection_At(config, 0);
    SOPC_SubTargetVariableConfig* targetConfig = SOPC_SubTargetVariableConfig_Create(&setTargetVariablesCb_ReaderTest);

    // NOMINAL
    setTargetVariablesCb_ReaderTest_called = false;
    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_NetworkMessage_Error_Code_None, code);
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(true, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;

    SOPC_ReaderGroup* readerGroup = SOPC_DataSetReader_Get_ReaderGroup(*dsr);

    // Change PUBLISHER ID and check that message has not been fully parsed.
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, NETWORK_MSG_PUBLISHER_ID + 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_ReaderGroup_Set_PublisherId_UInteger(readerGroup, NETWORK_MSG_PUBLISHER_ID);

    // WRONG GROUP ID
    SOPC_ReaderGroup_Set_GroupId(readerGroup, NETWORK_MSG_GROUP_ID + 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_ReaderGroup_Set_GroupId(readerGroup, NETWORK_MSG_GROUP_ID);

    // WRONG GROUP VERSION
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, NETWORK_MSG_GROUP_VERSION + 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, NETWORK_MSG_GROUP_VERSION);

    // WRONG DATA SET WRITER ID
    SOPC_DataSetReader_Set_DataSetWriterId(*dsr, DATASET_MSG_WRITER_ID_BASE + 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_UADP_NetworkMessage_Error_Read_NoMatchingReader, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_DataSetReader_Set_DataSetWriterId(*dsr, DATASET_MSG_WRITER_ID_BASE);

    // WRONG DATA SET DATA COMBINATION
    SOPC_ReaderGroup_Set_GroupId(readerGroup, NETWORK_MSG_GROUP_ID + 1);
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, NETWORK_MSG_GROUP_VERSION + 1);
    SOPC_DataSetReader_Set_DataSetWriterId(*dsr, DATASET_MSG_WRITER_ID_BASE + 1);

    code = SOPC_Reader_Read_UADP(connection, &encoded_network_msg, targetConfig, NULL, NULL, getGlobalTargetVariable,
                                 NULL);
    ck_assert_int_eq(SOPC_UADP_NetworkMessage_Error_Read_NoMatchingGroup, code);
    status = SOPC_Buffer_SetPosition(&encoded_network_msg, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(false, setTargetVariablesCb_ReaderTest_called);
    setTargetVariablesCb_ReaderTest_called = false;
    SOPC_ReaderGroup_Set_GroupId(readerGroup, NETWORK_MSG_GROUP_ID);
    SOPC_DataSetReader_Set_DataSetWriterId(*dsr, DATASET_MSG_WRITER_ID_BASE);
    SOPC_ReaderGroup_Set_GroupVersion(readerGroup, NETWORK_MSG_GROUP_VERSION);

    // UNINIT
    SOPC_SubTargetVariable_TargetVariableCtx_Delete(&gTargetVariable_dsm1);
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

    SOPC_PubSub_ArrayDimension arrDimension = {.valueRank = -1, .arrayDimensions = NULL};

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
        SOPC_FieldMetaData_ArrayDimension_Move(fieldMetaData, &arrDimension);
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

static SOPC_DataValue* getSourceVariablesCb(const OpcUa_ReadValueId* nodesToRead, const int32_t nbValues)
{
    SOPC_DataValue* dataValues = SOPC_Calloc(NB_VARS, sizeof(*dataValues));
    ck_assert_ptr_nonnull(dataValues);
    ck_assert_int_eq(NB_VARS, nbValues);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        SOPC_DataValue* dataValue = &dataValues[i];
        SOPC_DataValue_Initialize(dataValue);
        const OpcUa_ReadValueId* readValue = &nodesToRead[i];

        ck_assert_uint_eq(13, readValue->AttributeId);     // Value => AttributeId=13
        ck_assert_int_ge(0, readValue->IndexRange.Length); // No index range
        ck_assert_int_eq(SOPC_IdentifierType_Numeric, readValue->NodeId.IdentifierType);
        ck_assert_uint_eq(1, readValue->NodeId.Namespace);
        ck_assert_uint_eq(i, readValue->NodeId.Data.Numeric);

        dataValue->Value.ArrayType = varArr[i].ArrayType;
        dataValue->Value.BuiltInTypeId = varArr[i].BuiltInTypeId;
        dataValue->Value.Value = varArr[i].Value;
    }
    return dataValues;
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

START_TEST(test_source_variable_layer)
{
    SOPC_PublishedDataSet* pds = NULL;
    SOPC_PubSubConfiguration* config = build_Pub_Config(&pds);

    SOPC_SourceVariableCtx* sourceVariable = SOPC_PubSourceVariable_SourceVariablesCtx_Create(pds);
    ck_assert_ptr_nonnull(sourceVariable);

    SOPC_PubSourceVariableConfig* sourceConfig = SOPC_PubSourceVariableConfig_Create(&getSourceVariablesCb);
    SOPC_DataValue* dataValues = SOPC_PubSourceVariable_GetVariables(sourceConfig, sourceVariable);
    check_returned_DataValues(dataValues);

    SOPC_PubSourceVariable_SourceVariableCtx_Delete(&sourceVariable);
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

    SOPC_PubSub_ArrayDimension arrDimension = {.valueRank = -1, .arrayDimensions = NULL};

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
        SOPC_FieldMetaData_ArrayDimension_Move(field, &arrDimension); // SingleValue
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

    SOPC_Dataset_LL_NetworkMessage_Header* hLeft = SOPC_Dataset_LL_NetworkMessage_GetHeader(left);
    SOPC_Dataset_LL_NetworkMessage_Header* hRight = SOPC_Dataset_LL_NetworkMessage_GetHeader(right);
    ck_assert_int_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(hLeft));
    ck_assert_int_eq(DataSet_LL_PubId_Byte_Id, SOPC_Dataset_LL_NetworkMessage_Get_PublisherIdType(hRight));

    ck_assert_int_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(hLeft)->data.byte);
    ck_assert_int_eq(NETWORK_MSG_PUBLISHER_ID, SOPC_Dataset_LL_NetworkMessage_Get_PublisherId(hRight)->data.byte);

    ck_assert_uint_eq(SOPC_Dataset_LL_NetworkMessage_GetVersion(hLeft),
                      SOPC_Dataset_LL_NetworkMessage_GetVersion(hRight));

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
        const SOPC_Variant* leftVar = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(leftDsm, i);
        const SOPC_Variant* rightVar = SOPC_Dataset_LL_DataSetMsg_Get_ConstVariant_At(rightDsm, i);
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
    SOPC_Helper_Endianness_Check();

    SOPC_WriterGroup* group = NULL;
    SOPC_PubSubConfiguration* config = build_PubConfig_From_VarArr(&group);
    ck_assert_ptr_nonnull(config);
    ck_assert_ptr_nonnull(group);

    SOPC_Dataset_NetworkMessage* nm = SOPC_Create_NetworkMessage_From_WriterGroup(group, false);
    ck_assert_ptr_nonnull(nm);

    SOPC_PublishedDataSet* pubDataSet = SOPC_PubSubConfiguration_Get_PublishedDataSet_At(config, 0);
    ck_assert_ptr_nonnull(pubDataSet);

    for (uint16_t i = 0; i < NB_VARS; i++)
    {
        SOPC_FieldMetaData* field = SOPC_PublishedDataSet_Get_FieldMetaData_At(pubDataSet, i);
        SOPC_NetworkMessage_Set_Variant_At(nm, 0, i, &varArr[i], field);
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

    TCase* tc_tools_test = tcase_create("Utility-level tools checks");
    suite_add_tcase(suite, tc_tools_test);
    tcase_add_test(tc_tools_test, test_tc_tools_test);

    TCase* tc_hl_network_msg = tcase_create("Network message layer");
    suite_add_tcase(suite, tc_hl_network_msg);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_encode_json);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_encode);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_encode_multi_dsm);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode_multi_dsm);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode_multi_dsm_nok);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode_null_pubid);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_encode_uni_keep_alive_dsm);
    tcase_add_test(tc_hl_network_msg, test_hl_network_msg_decode_uni_keep_alive_dsm);

    TCase* tc_sub_target_variable_layer = tcase_create("Subscriber target variable layer");
    suite_add_tcase(suite, tc_sub_target_variable_layer);
    tcase_add_test(tc_sub_target_variable_layer, test_target_variable_layer);

    TCase* tc_sub_reader_layer = tcase_create("Subscriber reader layer");
    suite_add_tcase(suite, tc_sub_reader_layer);
    tcase_add_test(tc_sub_reader_layer, test_subscriber_reader_layer);
    tcase_add_test(tc_sub_reader_layer, test_subscriber_reader_layer_multi_dsm);

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
