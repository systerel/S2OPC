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
 * \brief Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <assert.h>
#include <check.h>
#include <stdio.h>

#include "check_helpers.h"
#include "hexlify.h"

#include "opcua_statuscodes.h"

#include "opcua_identifiers.h"
#include "sopc_async_queue.h"
#include "sopc_atomic.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encoder.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_mem_alloc.h"
#include "sopc_singly_linked_list.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_constants.h"

#define NODEID_NS 58942
#define NODEID_NSS "58942"
#define NODEID_I 126042
#define NODEID_IS "126042"
#define NODEID_S "s2opc.foo/bar"
#define NODEID_G                                                                       \
    {                                                                                  \
        0xC496578A, 0x0DFE, 0x4B8F, { 0x87, 0x0A, 0x74, 0x52, 0x38, 0xC6, 0xAE, 0xAE } \
    }
#define NODEID_GS "C496578A-0DFE-4b8f-870A-745238C6AEAE"

START_TEST(test_hexlify)
{
    unsigned char buf[33], c, d = 0;
    int i;

    // Init
    memset(buf, 0, 33);

    // Test single chars
    for (i = 0; i < 256; ++i)
    {
        c = (unsigned char) i;
        ck_assert(hexlify(&c, (char*) buf, 1) == 1);
        ck_assert(unhexlify((char*) buf, &d, 1) == 1);
        ck_assert(c == d);
    }

    // Test vector
    ck_assert(hexlify((unsigned char*) "\x00 Test \xFF", (char*) buf, 8) == 8);
    ck_assert(strncmp((char*) buf, "00205465737420ff", 16) == 0);
    ck_assert(unhexlify((char*) buf, buf + 16, 8) == 8);
    ck_assert(strncmp((char*) (buf + 16), "\x00 Test \xFF", 8) == 0);

    // Test overflow
    buf[32] = 0xDD;
    ck_assert(hexlify((unsigned char*) "\x00 Test \xFF\x00 Test \xFF", (char*) buf, 16) == 16);
    ck_assert(buf[32] == 0xDD);
    ck_assert(unhexlify("00205465737420ff00205465737420ff", buf, 16) == 16);
    ck_assert(buf[32] == 0xDD);
}
END_TEST

START_TEST(test_buffer_create)
{
    // Test creation
    //// Test nominal case
    SOPC_Buffer* buf = SOPC_Buffer_Create(10);
    ck_assert(buf != NULL);
    ck_assert(buf->data != NULL);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 0);
    ck_assert(buf->initial_size == 10);
    ck_assert(buf->current_size == 10);
    ck_assert(buf->maximum_size == 10);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test nominal case
    buf = SOPC_Buffer_CreateResizable(10, 100);
    ck_assert(buf != NULL);
    ck_assert(buf->data != NULL);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 0);
    ck_assert(buf->initial_size == 10);
    ck_assert(buf->current_size == 10);
    ck_assert(buf->maximum_size == 100);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test buffer creation degraded cases
    buf = SOPC_Buffer_Create(0);
    ck_assert(buf == NULL);

    buf = SOPC_Buffer_CreateResizable(0, 10);
    ck_assert(buf == NULL);

    buf = SOPC_Buffer_CreateResizable(100, 10);
    ck_assert(buf == NULL);
}
END_TEST

START_TEST(test_buffer_read_write)
{
    uint8_t data[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t readData[4] = {0x00, 0x00, 0x00, 0x00};
    SOPC_ReturnStatus status = 0;
    SOPC_Buffer* buf = NULL;

    // Test write
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 3);
    ck_assert(buf->position == 3);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    status = SOPC_Buffer_Write(buf, data, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x00);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = SOPC_Malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status != SOPC_STATUS_OK);
    SOPC_Free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status != SOPC_STATUS_OK);

    buf = SOPC_Buffer_Create(1);
    ////// NULL data pointer
    status = SOPC_Buffer_Write(buf, NULL, 3);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);

    ////// Full buffer
    status = SOPC_Buffer_Write(buf, data, 2);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    // Test read
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Reset position for reading content written
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 4);
    status = SOPC_Buffer_Read(readData, buf, 2);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->position == 2);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x00);
    ck_assert(readData[3] == 0x00);
    status = SOPC_Buffer_Read(&(readData[2]), buf, 2);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->position == 4);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x02);
    ck_assert(readData[3] == 0x03);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = SOPC_Malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Read(readData, buf, 3);
    ck_assert_int_ne(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    SOPC_Free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = SOPC_Buffer_Read(readData, buf, 3);
    ck_assert(status != SOPC_STATUS_OK);

    buf = SOPC_Buffer_Create(4);
    ////// NULL data pointer
    status = SOPC_Buffer_Read(NULL, buf, 3);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);

    ////// Empty buffer
    readData[0] = 0x00;
    readData[1] = 0x00;
    readData[2] = 0x00;
    readData[3] = 0x00;
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    //// DO NOT reset position for reading content written
    status = SOPC_Buffer_Read(readData, buf, 4);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x00);
    ck_assert(readData[2] == 0x00);
    ck_assert(readData[3] == 0x00);
    SOPC_Buffer_Delete(buf);
    buf = NULL;
}
END_TEST

START_TEST(test_buffer_copy)
{
    uint8_t data[4] = {0x00, 0x01, 0x02, 0x03};
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* buf = NULL;
    SOPC_Buffer* buf2 = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    buf2 = SOPC_Buffer_Create(5);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf2->length == 4);
    ck_assert(buf2->position == 4);
    ck_assert(buf2->data[0] == 0x00);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    ck_assert(buf2->data[3] == 0x03);
    ck_assert(buf->initial_size == 10);
    ck_assert(buf->current_size == 10);
    ck_assert(buf->maximum_size == 10);
    ck_assert(buf2->initial_size == 5);
    ck_assert(buf2->current_size == 5);
    ck_assert(buf2->maximum_size == 5);

    buf->data[0] = 0x01;
    buf->data[3] = 0x0F;
    status = SOPC_Buffer_SetPosition(buf, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 3);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    ck_assert(buf2->data[3] == 0x00);

    //// Test degraded cases
    /////// NULL pointers
    status = SOPC_Buffer_Copy(buf2, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    status = SOPC_Buffer_CopyWithLength(buf2, NULL, 4);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    status = SOPC_Buffer_Copy(NULL, buf);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(NULL, buf, 3);
    ck_assert(status != SOPC_STATUS_OK);

    /////// Destination buffer size insufficient
    SOPC_Buffer_Reset(buf);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    status = SOPC_Buffer_CopyWithLength(buf2, buf, 6);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    SOPC_Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = SOPC_Malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != SOPC_STATUS_OK);
    SOPC_Free(buf);
    buf = NULL;

    buf = SOPC_Buffer_Create(1);
    SOPC_Buffer_Delete(buf2);
    buf2 = NULL;

    /////// Non allocated buffer data
    buf2 = SOPC_Malloc(sizeof(SOPC_Buffer));
    memset(buf2, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != SOPC_STATUS_OK);
    SOPC_Free(buf2);
    buf2 = NULL;
    SOPC_Buffer_Delete(buf);
}
END_TEST

START_TEST(test_buffer_reset)
{
    uint8_t data[4] = {0x00, 0x01, 0x02, 0x03};
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Buffer_Reset(buf);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 0 <=> Reset
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_ResetAfterPosition(buf, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 2 in length of 4 buffer
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_ResetAfterPosition(buf, 2);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 4 in buffer with length 4
    SOPC_Buffer_Reset(buf);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_ResetAfterPosition(buf, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    //// Test degraded cases
    /////// NULL pointers
    status = SOPC_Buffer_ResetAfterPosition(NULL, 2);
    ck_assert(status != SOPC_STATUS_OK);

    /////// Invalid position: position > length
    status = SOPC_Buffer_ResetAfterPosition(buf, 5);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    SOPC_Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = SOPC_Malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status != SOPC_STATUS_OK);
    SOPC_Free(buf);
    buf = NULL;
}
END_TEST

START_TEST(test_buffer_set_properties)
{
    uint8_t data[4] = {0x00, 0x01, 0x02, 0x03};
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_SetPosition(buf, 2);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    status = SOPC_Buffer_SetDataLength(buf, 2);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    //// Test degraded cases
    /////// NULL pointers
    status = SOPC_Buffer_SetPosition(NULL, 1);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_SetDataLength(NULL, 1);
    ck_assert(status != SOPC_STATUS_OK);

    /////// Invalid position: position > length
    status = SOPC_Buffer_SetPosition(buf, 3);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);

    /////// Invalid length: length < position
    status = SOPC_Buffer_SetDataLength(buf, 1);
    ck_assert(status != SOPC_STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);

    SOPC_Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = SOPC_Malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_SetDataLength(buf, 0);
    ck_assert(status != SOPC_STATUS_OK);
    SOPC_Free(buf);
    buf = NULL;
}
END_TEST

START_TEST(test_buffer_append)
{
    uint8_t data[4] = {0x00, 0x01, 0x02, 0x03};
    SOPC_Buffer* bufA = NULL;
    SOPC_Buffer* bufB = NULL;

    /* After a write, position should change, but after a ReadFrom, it should not */
    bufA = SOPC_Buffer_Create(4);
    bufB = SOPC_Buffer_Create(12);
    ck_assert(SOPC_STATUS_OK == SOPC_Buffer_Write(bufA, data, 4));
    ck_assert(bufA->position == 4);
    ck_assert(bufA->length == 4);
    ck_assert(SOPC_STATUS_OK == SOPC_Buffer_Write(bufB, data, 4));
    ck_assert(bufB->position == 4);
    ck_assert(bufB->length == 4);
    ck_assert(SOPC_STATUS_OK == SOPC_Buffer_SetPosition(bufA, 0));
    /* bufA is appended 2 times in B */
    ck_assert(4 == SOPC_Buffer_ReadFrom(bufB, bufA, 8));
    ck_assert(0 == SOPC_Buffer_ReadFrom(bufB, bufA, 4));
    ck_assert(bufB->length == 8);
    ck_assert(SOPC_STATUS_OK == SOPC_Buffer_SetPosition(bufA, 0));
    ck_assert(4 == SOPC_Buffer_ReadFrom(bufB, bufA, 4));
    ck_assert(0 == SOPC_Buffer_ReadFrom(bufB, bufA, 0));
    ck_assert(bufB->position == 4);
    ck_assert(bufB->length == 12);

    SOPC_Buffer_Delete(bufA);
    SOPC_Buffer_Delete(bufB);
    bufA = NULL;
    bufB = NULL;
}
END_TEST

START_TEST(test_buffer_resizable)
{
    uint8_t data[20] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13};
    SOPC_ReturnStatus status = 0;
    SOPC_Buffer* buf = NULL;

    // Test write with resizable buffer
    //// Test nominal cases
    buf = SOPC_Buffer_CreateResizable(5, 15);
    status = SOPC_Buffer_Write(buf, data, 9);
    ck_assert_int_eq(status, 0);
    ck_assert_int_eq(buf->initial_size, 5);
    ck_assert_int_eq(buf->current_size, 5 * 2);
    ck_assert_int_eq(buf->maximum_size, 15);
    ck_assert_int_eq(buf->length, 9);
    ck_assert_int_eq(buf->position, 9);
    ck_assert_int_eq(buf->data[0], 0x00);
    ck_assert_int_eq(buf->data[1], 0x01);
    ck_assert_int_eq(buf->data[2], 0x02);
    ck_assert_int_eq(buf->data[3], 0x03);
    ck_assert_int_eq(buf->data[4], 0x04);
    ck_assert_int_eq(buf->data[5], 0x05);
    ck_assert_int_eq(buf->data[6], 0x06);
    ck_assert_int_eq(buf->data[7], 0x07);
    ck_assert_int_eq(buf->data[8], 0x08);
    status = SOPC_Buffer_Write(buf, &data[9], 2);
    ck_assert_int_eq(status, 0);
    ck_assert_int_eq(buf->initial_size, 5);
    ck_assert_int_eq(buf->current_size, 5 * 3);
    ck_assert_int_eq(buf->length, 11);
    ck_assert_int_eq(buf->position, 11);
    ck_assert_int_eq(buf->data[0], 0x00);
    ck_assert_int_eq(buf->data[1], 0x01);
    ck_assert_int_eq(buf->data[2], 0x02);
    ck_assert_int_eq(buf->data[3], 0x03);
    ck_assert_int_eq(buf->data[4], 0x04);
    ck_assert_int_eq(buf->data[5], 0x05);
    ck_assert_int_eq(buf->data[6], 0x06);
    ck_assert_int_eq(buf->data[7], 0x07);
    ck_assert_int_eq(buf->data[8], 0x08);
    ck_assert_int_eq(buf->data[9], 0x09);
    ck_assert_int_eq(buf->data[10], 0x0a);

    status = SOPC_Buffer_Write(buf, &data[11], 4);
    ck_assert_int_eq(status, 0);
    ck_assert_int_eq(buf->initial_size, 5);
    ck_assert_int_eq(buf->current_size, 5 * 3);
    ck_assert_int_eq(buf->length, 15);
    ck_assert_int_eq(buf->position, 15);
    ck_assert_int_eq(buf->data[0], 0x00);
    ck_assert_int_eq(buf->data[1], 0x01);
    ck_assert_int_eq(buf->data[2], 0x02);
    ck_assert_int_eq(buf->data[3], 0x03);
    ck_assert_int_eq(buf->data[4], 0x04);
    ck_assert_int_eq(buf->data[5], 0x05);
    ck_assert_int_eq(buf->data[6], 0x06);
    ck_assert_int_eq(buf->data[7], 0x07);
    ck_assert_int_eq(buf->data[8], 0x08);
    ck_assert_int_eq(buf->data[9], 0x09);
    ck_assert_int_eq(buf->data[10], 0x0a);
    ck_assert_int_eq(buf->data[11], 0x0b);
    ck_assert_int_eq(buf->data[12], 0x0c);
    ck_assert_int_eq(buf->data[13], 0x0d);
    ck_assert_int_eq(buf->data[14], 0x0e);

    //// Test degraded cases
    status = SOPC_Buffer_Write(buf, &data[15], 5);
    ck_assert_int_ne(status, 0);
    ck_assert_int_eq(buf->initial_size, 5);
    ck_assert_int_eq(buf->current_size, 5 * 3);
    ck_assert_int_eq(buf->length, 15);
    ck_assert_int_eq(buf->position, 15);
    ck_assert_int_eq(buf->data[0], 0x00);
    ck_assert_int_eq(buf->data[1], 0x01);
    ck_assert_int_eq(buf->data[2], 0x02);
    ck_assert_int_eq(buf->data[3], 0x03);
    ck_assert_int_eq(buf->data[4], 0x04);
    ck_assert_int_eq(buf->data[5], 0x05);
    ck_assert_int_eq(buf->data[6], 0x06);
    ck_assert_int_eq(buf->data[7], 0x07);
    ck_assert_int_eq(buf->data[8], 0x08);
    ck_assert_int_eq(buf->data[9], 0x09);
    ck_assert_int_eq(buf->data[10], 0x0a);
    ck_assert_int_eq(buf->data[11], 0x0b);
    ck_assert_int_eq(buf->data[12], 0x0c);
    ck_assert_int_eq(buf->data[13], 0x0d);
    ck_assert_int_eq(buf->data[14], 0x0e);

    SOPC_Buffer_Delete(buf);
    buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    SOPC_Buffer* buf2 = SOPC_Buffer_CreateResizable(1, 5);
    status = SOPC_Buffer_Write(buf, data, 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    ck_assert_int_eq(buf2->length, 4);
    ck_assert_int_eq(buf2->position, 4);
    ck_assert_int_eq(buf2->data[0], 0x00);
    ck_assert_int_eq(buf2->data[1], 0x01);
    ck_assert_int_eq(buf2->data[2], 0x02);
    ck_assert_int_eq(buf2->data[3], 0x03);
    ck_assert_int_eq(buf->initial_size, 10);
    ck_assert_int_eq(buf->current_size, 10);
    ck_assert_int_eq(buf->maximum_size, 10);
    ck_assert_int_eq(buf2->initial_size, 1);
    ck_assert_int_eq(buf2->current_size, 4);
    ck_assert_int_eq(buf2->maximum_size, 5);

    buf->data[0] = 0x01;
    buf->data[3] = 0x0F;
    status = SOPC_Buffer_SetPosition(buf, 1);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 3);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
    ck_assert_int_eq(buf2->length, 3);
    ck_assert_int_eq(buf2->position, 1);
    ck_assert_int_eq(buf2->data[0], 0x01);
    ck_assert_int_eq(buf2->data[1], 0x01);
    ck_assert_int_eq(buf2->data[2], 0x02);
    ck_assert_int_eq(buf2->data[3], 0x00);
    ck_assert_int_eq(buf2->initial_size, 1);
    ck_assert_int_eq(buf2->current_size, 4);
    ck_assert_int_eq(buf2->maximum_size, 5);

    //// Test degraded cases

    /////// Destination buffer size insufficient
    SOPC_Buffer_Reset(buf);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert_int_eq(status, SOPC_STATUS_OUT_OF_MEMORY);
    ck_assert_int_eq(buf2->length, 3);
    ck_assert_int_eq(buf2->position, 1);
    ck_assert_int_eq(buf2->data[0], 0x01);
    ck_assert_int_eq(buf2->data[1], 0x01);
    ck_assert_int_eq(buf2->data[2], 0x02);
    ck_assert_int_eq(buf2->initial_size, 1);
    ck_assert_int_eq(buf2->current_size, 4);
    ck_assert_int_eq(buf2->maximum_size, 5);

    status = SOPC_Buffer_SetPosition(buf, 1);
    ck_assert_int_eq(status, 0);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 6);
    ck_assert_int_eq(status, SOPC_STATUS_OUT_OF_MEMORY);
    ck_assert_int_eq(buf2->length, 3);
    ck_assert_int_eq(buf2->position, 1);
    ck_assert_int_eq(buf2->data[0], 0x01);
    ck_assert_int_eq(buf2->data[1], 0x01);
    ck_assert_int_eq(buf2->data[2], 0x02);

    SOPC_Buffer_Delete(buf2);
    buf2 = NULL;

    SOPC_Buffer_Delete(buf);
    buf = NULL;
}
END_TEST

START_TEST(test_linked_list)
{
    float value1 = 0.0;
    float value2 = 1.0;
    float value3 = 1.0;
    void* value = NULL;
    uint32_t id = 0;

    SOPC_SLinkedList* list = NULL;
    SOPC_SLinkedListIterator it = NULL;

    // Test creation
    //// Test nominal case
    list = SOPC_SLinkedList_Create(2);
    ck_assert(list != NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));
    SOPC_SLinkedList_Delete(list);
    list = NULL;

    // Test creation and addition
    //// Test nominal case
    list = SOPC_SLinkedList_Create(3);
    ck_assert(list != NULL);

    value = SOPC_SLinkedList_Prepend(list, 1, &value1);
    ck_assert(value == &value1);
    ck_assert(1 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_Prepend(list, 2, &value2);
    ck_assert(value == &value2);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_Append(list, 3, &value3);
    ck_assert(value == &value3);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    /// Test iterator nominal case
    it = SOPC_SLinkedList_GetIterator(list);
    ck_assert(it != NULL);
    bool hasNext = SOPC_SLinkedList_HasNext(&it);
    ck_assert(hasNext);
    value = SOPC_SLinkedList_NextWithId(&it, &id);
    ck_assert(value == &value2);
    ck_assert(id == 2);
    hasNext = SOPC_SLinkedList_HasNext(&it);
    ck_assert(hasNext);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == &value1);
    hasNext = SOPC_SLinkedList_HasNext(&it);
    ck_assert(hasNext);
    value = SOPC_SLinkedList_NextWithId(&it, &id);
    ck_assert(value == &value3);
    ck_assert(id == 3);
    hasNext = SOPC_SLinkedList_HasNext(&it);
    ck_assert(false == hasNext);
    value = SOPC_SLinkedList_NextWithId(&it, &id);
    ck_assert(value == NULL);

    //// Test degraded case: add in full linked list
    value = SOPC_SLinkedList_Prepend(list, 4, &value3);
    ck_assert(value == NULL);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    // Test pop nominal case
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value2);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_PopLast(list);
    ck_assert(value == &value3);
    ck_assert(1 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value1);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    // Test pop degraded case
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    SOPC_SLinkedList_Delete(list);
    list = NULL;

    // Test find and remove
    list = SOPC_SLinkedList_Create(4);
    ck_assert(list != NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    /// (Test iterator degraded case)
    it = SOPC_SLinkedList_GetIterator(list);
    ck_assert(it == NULL);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == NULL);

    /// (Continue initial test)
    value = SOPC_SLinkedList_Prepend(list, 0, &value1);
    ck_assert(value == &value1);
    ck_assert(1 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_Prepend(list, 2, &value2);
    ck_assert(value == &value2);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_Prepend(list, UINT32_MAX, &value3);
    ck_assert(value == &value3);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_Prepend(list, UINT32_MAX, &value1);
    ck_assert(value == &value1);
    ck_assert(4 == SOPC_SLinkedList_GetLength(list));

    //// Verify nominal find behavior
    value = SOPC_SLinkedList_FindFromId(list, 0);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_FindFromId(list, 2);
    ck_assert(value == &value2);
    //// Check LIFO behavior in case id not unique
    value = SOPC_SLinkedList_FindFromId(list, UINT32_MAX);
    ck_assert(value == &value1);
    //// Verify not found behavior
    value = SOPC_SLinkedList_FindFromId(list, 1);
    ck_assert(value == NULL);

    //// Verify nominal remove behavior
    value = SOPC_SLinkedList_RemoveFromId(list, 0);
    ck_assert(value == &value1);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_FindFromId(list, 0);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(list, 0);
    ck_assert(value == NULL);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_RemoveFromValuePtr(list, &value2);
    ck_assert(value == &value2);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_FindFromId(list, 2);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(list, 2);
    ck_assert(value == NULL);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    //// Check LIFO behavior in case id not unique
    value = SOPC_SLinkedList_RemoveFromId(list, UINT32_MAX);
    ck_assert(value == &value1);
    ck_assert(1 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_FindFromId(list, UINT32_MAX);
    ck_assert(value == &value3);

    value = SOPC_SLinkedList_RemoveFromId(list, UINT32_MAX);
    ck_assert(value == &value3);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_FindFromId(list, UINT32_MAX);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(list, UINT32_MAX);
    ck_assert(value == NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    //// Check apply to free elements
    void* p = NULL;
    SOPC_SLinkedList_Clear(list);
    p = SOPC_Malloc(sizeof(int));
    ck_assert(NULL != p);
    *(int*) p = 2;
    ck_assert(SOPC_SLinkedList_Prepend(list, 0, p) != NULL);
    p = SOPC_Malloc(sizeof(double));
    ck_assert(NULL != p);
    *(double*) p = 2.;
    ck_assert(SOPC_SLinkedList_Prepend(list, 1, p) != NULL);
    p = SOPC_Malloc(sizeof(char) * 5);
    ck_assert(NULL != p);
    memcpy(p, "toto", 5);
    ck_assert(SOPC_SLinkedList_Prepend(list, 2, p) != NULL);
    SOPC_SLinkedList_Apply(list, SOPC_SLinkedList_EltGenericFree);

    SOPC_SLinkedList_Delete(list);
    list = NULL;
}
END_TEST

static int8_t CompareUint8_T(void* left, void* right)
{
    assert(left != NULL && right != NULL);
    uint8_t leftUint = *(uint8_t*) left;
    uint8_t rightUint = *(uint8_t*) right;
    if (leftUint == rightUint)
    {
        return 0;
    }
    else if (leftUint < rightUint)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

START_TEST(test_sorted_linked_list)
{
    uint8_t values[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    void* value = NULL;
    uint32_t id = 0;

    SOPC_SLinkedList* list = NULL;
    SOPC_SLinkedListIterator it = NULL;

    // Test creation and insertion
    //// Test nominal case
    list = SOPC_SLinkedList_Create(9);
    ck_assert(list != NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    //// Test inserting a value as unique value
    value = SOPC_SLinkedList_SortedInsert(list, 6, &values[6], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(1 == SOPC_SLinkedList_GetLength(list));

    //// Test inserting a value as first value
    value = SOPC_SLinkedList_SortedInsert(list, 1, &values[1], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    //// Test inserting a value as last value
    value = SOPC_SLinkedList_SortedInsert(list, 9, &values[9], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    //// Test inserting the rest
    value = SOPC_SLinkedList_SortedInsert(list, 7, &values[7], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(4 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_SortedInsert(list, 5, &values[5], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(5 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_SortedInsert(list, 4, &values[4], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(6 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_SortedInsert(list, 3, &values[3], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(7 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_SortedInsert(list, 8, &values[8], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(8 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_SortedInsert(list, 2, &values[2], CompareUint8_T);
    ck_assert(value != NULL);
    ck_assert(9 == SOPC_SLinkedList_GetLength(list));

    //// Test degraded: try to insert when list is full
    value = SOPC_SLinkedList_SortedInsert(list, 10, &values[10], CompareUint8_T);
    ck_assert(value == NULL);
    ck_assert(9 == SOPC_SLinkedList_GetLength(list));

    // Test iteration (sorted values expected) and removal
    it = SOPC_SLinkedList_GetIterator(list);
    for (uint8_t i = 1; i < 10; i++)
    {
        value = SOPC_SLinkedList_NextWithId(&it, &id);
        ck_assert(value != NULL);
        ck_assert(*(uint8_t*) value == i);
        ck_assert(id == i);

        // Remove iterated value (possible during iteration since we remove already iterated item)
        value = SOPC_SLinkedList_RemoveFromId(list, i);
        ck_assert(value != NULL);
        ck_assert((uint8_t)(9 - i) == SOPC_SLinkedList_GetLength(list));
    }
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    SOPC_SLinkedList_Delete(list);
}
END_TEST

START_TEST(test_strncmp_ignore_case)
{
    char test1[] = "te";
    char test2[] = "Te";
    char test3[] = "tE";
    char test4[] = "TE";
    char ntest1[] = "de";
    char ntest2[] = "De";
    char ntest3[] = "ta";
    char ntest4[] = "tA";
    char long_test1[] = "tes";

    // Test nominal case (equality result)
    //// Each possible size class
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test2, 0));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test2, 1));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test2, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test2, 3));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test2, 4));
    //// Each possible combination of case
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test1, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test2, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test3, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, test4, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test2, test1, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test2, test2, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test2, test3, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test2, test4, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test3, test1, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test3, test2, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test3, test3, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test3, test4, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test4, test1, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test4, test2, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test4, test3, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test4, test4, 2));

    // Test nominal case (non equality result)
    //// Each possible size class
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, ntest1, 0));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest1, 1));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test1, 1));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest1, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test1, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest1, 3));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test1, 3));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest1, 4));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test1, 4));

    //// Each possible combination of case
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest1, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest2, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest3, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test1, ntest4, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test2, ntest1, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test2, ntest2, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test2, ntest3, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test2, ntest4, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test3, ntest1, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test3, ntest2, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test3, ntest3, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test3, ntest4, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test4, ntest1, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test4, ntest2, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test4, ntest3, 2));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(test4, ntest4, 2));

    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test1, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test2, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test3, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest1, test4, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest2, test1, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest2, test2, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest2, test3, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest2, test4, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest3, test1, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest3, test2, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest3, test3, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest3, test4, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest4, test1, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest4, test2, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest4, test3, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(ntest4, test4, 2));

    // Test nominal case (equality on 2 characters only then inequality)
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, long_test1, 0));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, long_test1, 1));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(long_test1, test1, 1));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(test1, long_test1, 2));
    ck_assert_int_eq(0, SOPC_strncmp_ignore_case(long_test1, test1, 2));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(test1, long_test1, 3));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(long_test1, test1, 3));
    ck_assert_int_eq(-1, SOPC_strncmp_ignore_case(test1, long_test1, 4));
    ck_assert_int_eq(+1, SOPC_strncmp_ignore_case(long_test1, test1, 4));
}
END_TEST

START_TEST(test_strcmp_ignore_case)
{
    char test1[] = "te";
    char test2[] = "Te";
    char test3[] = "tE";
    char test4[] = "TE";
    char ntest1[] = "de";
    char ntest2[] = "De";
    char ntest3[] = "ta";
    char ntest4[] = "tA";
    char long_test1[] = "tes";

    // Test nominal case (equality result)
    //// Each possible combination of case
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test1, test1));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test1, test2));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test1, test3));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test1, test4));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test2, test1));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test2, test2));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test2, test3));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test2, test4));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test3, test1));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test3, test2));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test3, test3));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test3, test4));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test4, test1));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test4, test2));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test4, test3));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case(test4, test4));

    // Test nominal case (non equality result)
    //// Each possible combination of case
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test1, ntest1));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test1, ntest2));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test1, ntest3));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test1, ntest4));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test2, ntest1));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test2, ntest2));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test2, ntest3));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test2, ntest4));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test3, ntest1));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test3, ntest2));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test3, ntest3));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test3, ntest4));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test4, ntest1));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test4, ntest2));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test4, ntest3));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(test4, ntest4));

    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest1, test1));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest1, test2));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest1, test3));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest1, test4));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest2, test1));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest2, test2));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest2, test3));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest2, test4));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest3, test1));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest3, test2));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest3, test3));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest3, test4));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest4, test1));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest4, test2));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest4, test3));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(ntest4, test4));

    // Test nominal case (inequality with different sizes)

    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case(test1, long_test1));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case(long_test1, test1));
}
END_TEST

START_TEST(test_strcmp_ignore_case_alt_end)
{
    char test1[] = "te";
    char test2[] = "Te-az";
    char test3[] = "tE/ee";
    char test4[] = "TE?r";
    char ntest1[] = "de";
    char ntest2[] = "De-uz";
    char ntest3[] = "ta";
    char ntest4[] = "tA-Zz";
    char long_test1[] = "tes";

    // Test nominal case (equality result)
    //// Each possible combination of case
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test1, test1, '-'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test1, test2, '-'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test1, test3, '/'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test1, test4, '?'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test2, test1, '-'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test2, test2, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(test2, test3, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(test2, test4, '-'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test3, test1, '/'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test3, test2, '-'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test3, test3, '/'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test3, test4, '?'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test4, test1, '?'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test4, test2, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test4, test3, '/'));
    ck_assert_int_eq(0, SOPC_strcmp_ignore_case_alt_end(test4, test4, '?'));

    // Test nominal case (non equality result)
    //// Each possible combination of case
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test1, ntest1, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test1, ntest2, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test1, ntest3, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test1, ntest4, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test2, ntest1, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test2, ntest2, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test2, ntest3, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test2, ntest4, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test3, ntest1, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test3, ntest2, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test3, ntest3, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test3, ntest4, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test4, ntest1, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test4, ntest2, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test4, ntest3, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(test4, ntest4, '-'));

    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest1, test1, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest1, test2, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest1, test3, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest1, test4, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest2, test1, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest2, test2, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest2, test3, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest2, test4, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest3, test1, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest3, test2, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest3, test3, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest3, test4, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest4, test1, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest4, test2, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest4, test3, '-'));
    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(ntest4, test4, '-'));

    // Test nominal case (inequality with different sizes)

    ck_assert_int_eq(-1, SOPC_strcmp_ignore_case_alt_end(test1, long_test1, '-'));
    ck_assert_int_eq(+1, SOPC_strcmp_ignore_case_alt_end(long_test1, test1, '-'));
}
END_TEST

static void uri_free(SOPC_UriType* type, char** hostname, char** port)
{
    SOPC_Free(*hostname);
    SOPC_Free(*port);
    if (*type != SOPC_URI_UNDETERMINED)
    {
        *type = SOPC_URI_UNDETERMINED;
    }
    *port = NULL;
    *hostname = NULL;
}

START_TEST(test_helper_uri)
{
    SOPC_UriType type = SOPC_URI_UNDETERMINED;
    char* hostname = NULL;
    char* port = NULL;

    // Not a TCP UA address
    ck_assert(SOPC_STATUS_OK != SOPC_Helper_URI_SplitUri("http://localhost:9999/test", &type, &hostname, &port));
    uri_free(&type, &hostname, &port);

    ck_assert(SOPC_STATUS_OK == SOPC_Helper_URI_SplitUri("opc.tcp://localhost:9999/test", &type, &hostname, &port));
    ck_assert(SOPC_URI_TCPUA == type);
    ck_assert(strncmp(hostname, "localhost", strlen("localhost") + 1) == 0);
    ck_assert(strncmp(port, "9999", strlen("9999") + 1) == 0);
    uri_free(&type, &hostname, &port);

    ck_assert(SOPC_STATUS_OK == SOPC_Helper_URI_SplitUri("opc.tcp://localhost:9999", &type, &hostname, &port));
    ck_assert(SOPC_URI_TCPUA == type);
    ck_assert(strncmp(hostname, "localhost", strlen("localhost") + 1) == 0);
    ck_assert(strncmp(port, "9999", strlen("9999") + 1) == 0);
    uri_free(&type, &hostname, &port);

    ck_assert(SOPC_STATUS_OK ==
              SOPC_Helper_URI_SplitUri("opc.tcp://localhost:9999/test/moreThingsInPath/plus", &type, &hostname, &port));
    ck_assert(SOPC_URI_TCPUA == type);
    ck_assert(strncmp(hostname, "localhost", strlen("localhost") + 1) == 0);
    ck_assert(strncmp(port, "9999", strlen("9999") + 1) == 0);
    uri_free(&type, &hostname, &port);

    // No port defined
    ck_assert(SOPC_STATUS_OK != SOPC_Helper_URI_SplitUri("opc.tcp://localhost:/test", &type, &hostname, &port));
    ck_assert_ptr_null(port);
    uri_free(&type, &hostname, &port);

    ck_assert(SOPC_STATUS_OK != SOPC_Helper_URI_SplitUri("opc.tcp://localhost/test", &type, &hostname, &port));
    ck_assert_ptr_null(port);
    uri_free(&type, &hostname, &port);

    ck_assert(SOPC_STATUS_OK == SOPC_Helper_URI_SplitUri("opc.tcp://192.168.4.1:9/test", &type, &hostname, &port));
    ck_assert(SOPC_URI_TCPUA == type);
    ck_assert(strncmp(hostname, "192.168.4.1", strlen("192.168.4.1") + 1) == 0);
    ck_assert(strncmp(port, "9", strlen("9") + 1) == 0);
    uri_free(&type, &hostname, &port);

    ck_assert(SOPC_STATUS_OK ==
              SOPC_Helper_URI_SplitUri("opc.tcp://[fe80::250:b6ff:fe17:9f3a]:99999/test", &type, &hostname, &port));
    ck_assert(SOPC_URI_TCPUA == type);
    ck_assert(strncmp(hostname, "[fe80::250:b6ff:fe17:9f3a]", strlen("[fe80::250:b6ff:fe17:9f3a]") + 1) == 0);
    ck_assert(strncmp(port, "99999", strlen("99999") + 1) == 0);
    uri_free(&type, &hostname, &port);
}
END_TEST

START_TEST(test_strtouint)
{
    uint8_t a = 0;
    uint16_t b = 0;
    uint32_t c = 0;

    /* 8 */
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint8_t("0", &a, 10, '\0'));
    ck_assert(0 == a);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint8_t("42", &a, 10, '\0'));
    ck_assert(42 == a);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint8_t("42", &a, 16, '\0'));
    ck_assert(0x42 == a);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint8_t("42;3", &a, 10, ';'));
    ck_assert(42 == a);

    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t("", &a, 10, '\0'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t(NULL, &a, 10, '\0'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t("42", NULL, 10, '\0'));

    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t("42", &a, 10, ';'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t("42zz-24", &a, 10, '-'));

    ck_assert(SOPC_STATUS_OK == SOPC_strtouint8_t("255", &a, 10, '\0'));
    ck_assert(0xFF == a);
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t("256", &a, 10, '\0'));

    /* 16 */
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("0", &b, 10, '\0'));
    ck_assert(0 == b);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("42", &b, 10, '\0'));
    ck_assert(42 == b);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("42", &b, 16, '\0'));
    ck_assert(0x42 == b);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("42;3", &b, 10, ';'));
    ck_assert(42 == b);

    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t("", &b, 10, '\0'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t(NULL, &b, 10, '\0'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t("42", NULL, 10, '\0'));

    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t("42", &b, 10, ';'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t("42zz-24", &b, 10, '-'));

    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("255", &b, 10, '\0'));
    ck_assert(0xFF == b);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("256", &b, 10, '\0'));
    ck_assert(0x100 == b);

    ck_assert(SOPC_STATUS_OK == SOPC_strtouint16_t("65535", &b, 10, '\0'));
    ck_assert(0xFFFF == b);
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t("65536", &b, 10, '\0'));

    /* 32 */
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("0", &c, 10, '\0'));
    ck_assert(0 == c);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("42", &c, 10, '\0'));
    ck_assert(42 == c);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("42", &c, 16, '\0'));
    ck_assert(0x42 == c);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("42;3", &c, 10, ';'));
    ck_assert(42 == c);

    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t("", &c, 10, '\0'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t(NULL, &c, 10, '\0'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t("42", NULL, 10, '\0'));

    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t("42", &c, 10, ';'));
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t("42zz-24", &c, 10, '-'));

    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("255", &c, 10, '\0'));
    ck_assert(0xFF == c);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("256", &c, 10, '\0'));
    ck_assert(0x100 == c);

    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("65535", &c, 10, '\0'));
    ck_assert(0xFFFF == c);
    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("65536", &c, 10, '\0'));
    ck_assert(0x10000 == c);

    ck_assert(SOPC_STATUS_OK == SOPC_strtouint32_t("4294967295", &c, 10, '\0'));
    ck_assert(0xFFFFFFFF == c);
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t("4294967296", &c, 10, '\0'));
}
END_TEST

START_TEST(test_strtouint2)
{
    uint8_t a = 0;
    uint16_t b = 0;
    uint32_t c = 0;
    uint64_t d = 0;

    /* 8 */
    ck_assert(SOPC_strtouint("0", strlen("0"), 8, &a));
    ck_assert(0 == a);
    ck_assert(SOPC_strtouint("42", strlen("42"), 8, &a));
    ck_assert(42 == a);
    ck_assert(!SOPC_strtouint("42;3", strlen("42;3"), 8, &a));
    ck_assert(SOPC_strtouint("42;3", 2, 8, &a));
    ck_assert(42 == a);
    ck_assert(!SOPC_strtouint("42zz-24", strlen("42zz-24"), 8, &a));
    ck_assert(SOPC_strtouint("42zz-24", 2, 8, &a));
    ck_assert(42 == a);

    ck_assert(!SOPC_strtouint("", strlen(""), 8, &a));
    ck_assert(!SOPC_strtouint(NULL, 0, 8, &a));
    ck_assert(!SOPC_strtouint("42", strlen("42"), 8, NULL));

    ck_assert(SOPC_strtouint("255", strlen("255"), 8, &a));
    ck_assert(0xFF == a);
    ck_assert(!SOPC_strtouint("256", strlen("256"), 8, &a));

    /* 16 */
    ck_assert(SOPC_strtouint("0", strlen("0"), 16, &b));
    ck_assert(0 == b);
    ck_assert(SOPC_strtouint("42", strlen("42"), 16, &b));
    ck_assert(42 == b);

    ck_assert(!SOPC_strtouint("", strlen(""), 16, &b));
    ck_assert(!SOPC_strtouint(NULL, 0, 16, &b));
    ck_assert(!SOPC_strtouint("42", strlen("42"), 16, NULL));

    ck_assert(SOPC_strtouint("255", strlen("255"), 16, &b));
    ck_assert(0xFF == b);
    ck_assert(SOPC_strtouint("256", strlen("256"), 16, &b));
    ck_assert(0x100 == b);

    ck_assert(SOPC_strtouint("65535", strlen("65535"), 16, &b));
    ck_assert(0xFFFF == b);
    ck_assert(!SOPC_strtouint("65536", strlen("65536"), 16, &b));

    /* 32 */
    ck_assert(SOPC_strtouint("0", strlen("0"), 32, &c));
    ck_assert(0 == c);
    ck_assert(SOPC_strtouint("42", strlen("42"), 32, &c));
    ck_assert(42 == c);

    ck_assert(!SOPC_strtouint("", strlen(""), 32, &c));
    ck_assert(!SOPC_strtouint(NULL, 0, 32, &c));
    ck_assert(!SOPC_strtouint("42", strlen("42"), 32, NULL));

    ck_assert(SOPC_strtouint("255", strlen("255"), 32, &c));
    ck_assert(0xFF == c);
    ck_assert(SOPC_strtouint("256", strlen("256"), 32, &c));
    ck_assert(0x100 == c);

    ck_assert(SOPC_strtouint("65535", strlen("65535"), 32, &c));
    ck_assert(0xFFFF == c);
    ck_assert(SOPC_strtouint("65536", strlen("65536"), 32, &c));
    ck_assert(0x10000 == c);

    ck_assert(SOPC_strtouint("4294967295", strlen("4294967295"), 32, &c));
    ck_assert(0xFFFFFFFF == c);
    ck_assert(!SOPC_strtouint("4294967296", strlen("4294967296"), 32, &c));

    /* 64 */
    ck_assert(SOPC_strtouint("0", strlen("0"), 64, &d));
    ck_assert(0 == d);
    ck_assert(SOPC_strtouint("42", strlen("42"), 64, &d));
    ck_assert(42 == d);

    ck_assert(!SOPC_strtouint("", strlen(""), 64, &d));
    ck_assert(!SOPC_strtouint(NULL, 0, 64, &d));
    ck_assert(!SOPC_strtouint("42", strlen("42"), 64, NULL));

    ck_assert(SOPC_strtouint("255", strlen("255"), 64, &d));
    ck_assert(0xFF == d);
    ck_assert(SOPC_strtouint("256", strlen("256"), 64, &d));
    ck_assert(0x100 == d);

    ck_assert(SOPC_strtouint("65535", strlen("65535"), 64, &d));
    ck_assert(0xFFFF == d);
    ck_assert(SOPC_strtouint("65536", strlen("65536"), 64, &d));
    ck_assert(0x10000 == d);

    ck_assert(SOPC_strtouint("4294967295", strlen("4294967295"), 64, &d));
    ck_assert(0xFFFFFFFF == d);
    ck_assert(SOPC_strtouint("4294967296", strlen("4294967296"), 64, &d));
    ck_assert(0x100000000 == d);

    ck_assert(SOPC_strtouint("18446744073709551615", strlen("18446744073709551615"), 64, &d));
    ck_assert(0xFFFFFFFFFFFFFFFF == d);
    ck_assert(!SOPC_strtouint("18446744073709551616", strlen("18446744073709551616"), 64, &d));
}
END_TEST

START_TEST(test_strtoint)
{
    int8_t a = 0;
    int16_t b = 0;
    int32_t c = 0;
    int64_t d = 0;

    /* 8 */
    ck_assert(SOPC_strtoint("0", strlen("0"), 8, &a));
    ck_assert(0 == a);
    ck_assert(SOPC_strtoint("42", strlen("42"), 8, &a));
    ck_assert(42 == a);

    ck_assert(!SOPC_strtoint("", strlen(""), 8, &a));
    ck_assert(!SOPC_strtoint(NULL, 0, 8, &a));
    ck_assert(!SOPC_strtoint("42", strlen("42"), 8, NULL));

    ck_assert(!SOPC_strtouint("42;3", strlen("42;3"), 8, &a));
    ck_assert(SOPC_strtouint("42;3", 2, 8, &a));
    ck_assert(42 == a);
    ck_assert(!SOPC_strtouint("42zz-24", strlen("42zz-24"), 8, &a));
    ck_assert(SOPC_strtouint("42zz-24", 2, 8, &a));
    ck_assert(42 == a);

    ck_assert(SOPC_strtoint("-128", strlen("-128"), 8, &a));
    ck_assert(INT8_MIN == a);
    ck_assert(!SOPC_strtoint("-129", strlen("-129"), 8, &a));

    ck_assert(SOPC_strtoint("127", strlen("127"), 8, &a));
    ck_assert(INT8_MAX == a);
    ck_assert(!SOPC_strtoint("128", strlen("128"), 8, &a));

    /* 16 */
    ck_assert(SOPC_strtoint("0", strlen("0"), 16, &b));
    ck_assert(0 == b);
    ck_assert(SOPC_strtoint("42", strlen("42"), 16, &b));
    ck_assert(42 == b);

    ck_assert(!SOPC_strtoint("", strlen(""), 16, &b));
    ck_assert(!SOPC_strtoint(NULL, 0, 16, &b));
    ck_assert(!SOPC_strtoint("42", strlen("42"), 16, NULL));

    ck_assert(SOPC_strtoint("-128", strlen("-128"), 16, &b));
    ck_assert(INT8_MIN == b);
    ck_assert(SOPC_strtoint("-129", strlen("-129"), 16, &b));
    ck_assert(INT8_MIN - 1 == b);

    ck_assert(SOPC_strtoint("127", strlen("127"), 16, &b));
    ck_assert(INT8_MAX == b);
    ck_assert(SOPC_strtoint("128", strlen("128"), 16, &b));
    ck_assert(INT8_MAX + 1 == b);

    ck_assert(SOPC_strtoint("-32768", strlen("-32768"), 16, &b));
    ck_assert(INT16_MIN == b);
    ck_assert(!SOPC_strtoint("-32769", strlen("-32769"), 16, &b));

    ck_assert(SOPC_strtoint("32767", strlen("32767"), 16, &b));
    ck_assert(INT16_MAX == b);
    ck_assert(!SOPC_strtoint("32768", strlen("32768"), 16, &b));

    /* 32 */
    ck_assert(SOPC_strtoint("0", strlen("0"), 32, &c));
    ck_assert(0 == c);
    ck_assert(SOPC_strtoint("42", strlen("42"), 32, &c));
    ck_assert(42 == c);

    ck_assert(!SOPC_strtoint("", strlen(""), 32, &c));
    ck_assert(!SOPC_strtoint(NULL, 0, 32, &c));
    ck_assert(!SOPC_strtoint("42", strlen("42"), 32, NULL));

    ck_assert(SOPC_strtoint("-128", strlen("-128"), 32, &c));
    ck_assert(INT8_MIN == c);
    ck_assert(SOPC_strtoint("-129", strlen("-129"), 32, &c));
    ck_assert(INT8_MIN - 1 == c);

    ck_assert(SOPC_strtoint("127", strlen("127"), 32, &c));
    ck_assert(INT8_MAX == c);
    ck_assert(SOPC_strtoint("128", strlen("128"), 32, &c));
    ck_assert(INT8_MAX + 1 == c);

    ck_assert(SOPC_strtoint("-32768", strlen("-32768"), 32, &c));
    ck_assert(INT16_MIN == c);
    ck_assert(SOPC_strtoint("-32769", strlen("-32769"), 32, &c));
    ck_assert(INT16_MIN - 1 == c);

    ck_assert(SOPC_strtoint("32767", strlen("32767"), 32, &c));
    ck_assert(INT16_MAX == c);
    ck_assert(SOPC_strtoint("32768", strlen("32768"), 32, &c));
    ck_assert(INT16_MAX + 1 == c);

    ck_assert(SOPC_strtoint("-2147483648", strlen("-2147483648"), 32, &c));
    ck_assert(INT32_MIN == c);
    ck_assert(!SOPC_strtoint("-2147483649", strlen("-2147483649"), 32, &c));

    ck_assert(SOPC_strtoint("2147483647", strlen("2147483647"), 32, &c));
    ck_assert(INT32_MAX == c);
    ck_assert(!SOPC_strtoint("2147483648", strlen("2147483648"), 32, &c));

    /* 64 */
    ck_assert(SOPC_strtoint("0", strlen("0"), 64, &d));
    ck_assert(0 == d);
    ck_assert(SOPC_strtoint("42", strlen("42"), 64, &d));
    ck_assert(42 == d);

    ck_assert(!SOPC_strtoint("", strlen(""), 64, &d));
    ck_assert(!SOPC_strtoint(NULL, 0, 64, &d));
    ck_assert(!SOPC_strtoint("42", strlen("42"), 64, NULL));

    ck_assert(SOPC_strtoint("-128", strlen("-128"), 64, &d));
    ck_assert(INT8_MIN == d);
    ck_assert(SOPC_strtoint("-129", strlen("-129"), 64, &d));
    ck_assert(INT8_MIN - 1 == d);

    ck_assert(SOPC_strtoint("127", strlen("127"), 64, &d));
    ck_assert(INT8_MAX == d);
    ck_assert(SOPC_strtoint("128", strlen("128"), 64, &d));
    ck_assert(INT8_MAX + 1 == d);

    ck_assert(SOPC_strtoint("-32768", strlen("-32768"), 64, &d));
    ck_assert(INT16_MIN == d);
    ck_assert(SOPC_strtoint("-32769", strlen("-32769"), 64, &d));
    ck_assert(INT16_MIN - 1 == d);

    ck_assert(SOPC_strtoint("32767", strlen("32767"), 64, &d));
    ck_assert(INT16_MAX == d);
    ck_assert(SOPC_strtoint("32768", strlen("32768"), 64, &d));
    ck_assert(INT16_MAX + 1 == d);

    ck_assert(SOPC_strtoint("-2147483648", strlen("-2147483648"), 64, &d));
    ck_assert(INT32_MIN == d);
    ck_assert(SOPC_strtoint("-2147483649", strlen("-2147483649"), 64, &d));
    ck_assert(-2147483649LL == d);

    ck_assert(SOPC_strtoint("2147483647", strlen("2147483647"), 64, &d));
    ck_assert(INT32_MAX == d);
    ck_assert(SOPC_strtoint("2147483648", strlen("2147483648"), 64, &d));
    ck_assert(2147483648LL == d);

    ck_assert(SOPC_strtoint("-9223372036854775808", strlen("-9223372036854775808"), 64, &d));
    ck_assert(INT64_MIN == d);
    ck_assert(!SOPC_strtoint("-9223372036854775809", strlen("-9223372036854775809"), 64, &d));

    ck_assert(SOPC_strtoint("9223372036854775807", strlen("9223372036854775807"), 64, &d));
    ck_assert(INT64_MAX == d);
    ck_assert(!SOPC_strtoint("9223372036854775808", strlen("9223372036854775808"), 64, &d));
}
END_TEST

START_TEST(test_string_nodeid)
{
    SOPC_NodeId nid;
    SOPC_Guid guid = NODEID_G;
    SOPC_NodeId* pNid;
    char* sNid = NULL;
    int32_t cmp = 0;

    /* Integer nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_Numeric;
    nid.Namespace = 0;
    nid.Data.Numeric = NODEID_I;

    pNid = SOPC_NodeId_FromCString("i=" NODEID_IS, (int32_t) strlen("i=" NODEID_IS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(strncmp(sNid, "i=" NODEID_IS, strlen("i=" NODEID_IS)) == 0);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    nid.Namespace = NODEID_NS;
    pNid =
        SOPC_NodeId_FromCString("ns=" NODEID_NSS ";i=" NODEID_IS, (int32_t) strlen("ns=" NODEID_NSS ";i=" NODEID_IS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(strncmp(sNid, "ns=" NODEID_NSS ";i=" NODEID_IS, strlen("ns=" NODEID_NSS ";i=" NODEID_IS)) == 0);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    /* String nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_String;
    nid.Namespace = 0;
    ck_assert(SOPC_STATUS_OK == SOPC_String_CopyFromCString(&nid.Data.String, NODEID_S));

    pNid = SOPC_NodeId_FromCString("s=" NODEID_S, (int32_t) strlen("s=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(strncmp(sNid, "s=" NODEID_S, strlen("s=" NODEID_S)) == 0);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    nid.Namespace = NODEID_NS;
    pNid = SOPC_NodeId_FromCString("ns=" NODEID_NSS ";s=" NODEID_S, (int32_t) strlen("ns=" NODEID_NSS ";s=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(strncmp(sNid, "ns=" NODEID_NSS ";s=" NODEID_S, strlen("ns=" NODEID_NSS ";s=" NODEID_S)) == 0);
    SOPC_NodeId_Clear(&nid);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    /* GUID nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_Guid;
    nid.Namespace = 0;
    nid.Data.Guid = &guid;

    pNid = SOPC_NodeId_FromCString("g=" NODEID_GS, (int32_t) strlen("g=" NODEID_GS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(SOPC_strncmp_ignore_case(sNid, "g=" NODEID_GS, strlen("g=" NODEID_GS)) == 0);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    nid.Namespace = NODEID_NS;
    pNid =
        SOPC_NodeId_FromCString("ns=" NODEID_NSS ";g=" NODEID_GS, (int32_t) strlen("ns=" NODEID_NSS ";g=" NODEID_GS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(SOPC_strncmp_ignore_case(sNid, "ns=" NODEID_NSS ";g=" NODEID_GS,
                                       strlen("ns=" NODEID_NSS ";g=" NODEID_GS)) == 0);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    /* ByteString nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_ByteString;
    nid.Namespace = 0;
    ck_assert(SOPC_STATUS_OK ==
              SOPC_ByteString_CopyFromBytes(&nid.Data.Bstring, (SOPC_Byte*) NODEID_S, (int32_t) strlen(NODEID_S)));

    pNid = SOPC_NodeId_FromCString("b=" NODEID_S, (int32_t) strlen("b=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(strncmp(sNid, "b=" NODEID_S, strlen("b=" NODEID_S)) == 0);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);

    nid.Namespace = NODEID_NS;
    pNid = SOPC_NodeId_FromCString("ns=" NODEID_NSS ";b=" NODEID_S, (int32_t) strlen("ns=" NODEID_NSS ";b=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
    ck_assert(strncmp(sNid, "ns=" NODEID_NSS ";b=" NODEID_S, strlen("ns=" NODEID_NSS ";b=" NODEID_S)) == 0);
    SOPC_NodeId_Clear(&nid);
    SOPC_Free(sNid);
    SOPC_NodeId_Clear(pNid);
    SOPC_Free(pNid);
}
END_TEST

// Note: use macros to keep line number of caller

#define check_string_datetime(datetime, success, expYear, expMonth, expDay, expHour, expMinute, expSecond,   \
                              expSecondAndFraction, expUtc, expUtc_neg_off, expUtc_hour_off, expUtc_min_off) \
    {                                                                                                        \
        SOPC_tm res;                                                                                         \
        memset(&res, 0, sizeof(res));                                                                        \
                                                                                                             \
        size_t len = strlen(datetime);                                                                       \
        bool parseRes = SOPC_tm_FromXsdDateTime(datetime, len, &res);                                        \
        ck_assert(parseRes == success);                                                                      \
        ck_assert_int_eq(expYear, res.year);                                                                 \
        ck_assert_uint_eq(expMonth, res.month);                                                              \
        ck_assert_uint_eq(expDay, res.day);                                                                  \
        ck_assert_uint_eq(expHour, res.hour);                                                                \
        ck_assert_uint_eq(expMinute, res.minute);                                                            \
        ck_assert_uint_eq(expSecond, res.second);                                                            \
        ck_assert_double_eq_tol(expSecondAndFraction, res.secondAndFrac, (double) 0.000000001);              \
        ck_assert(res.UTC == expUtc);                                                                        \
        ck_assert_uint_eq(expUtc_neg_off, res.UTC_neg_off);                                                  \
        ck_assert_uint_eq(expUtc_hour_off, res.UTC_hour_off);                                                \
        ck_assert_uint_eq(expUtc_min_off, res.UTC_min_off);                                                  \
    }

#define check_ok_string_datetime_no_timezeone(datetime, expYear, expMonth, expDay, expHour, expMinute, expSecond, \
                                              expSecondAndFraction)                                               \
    check_string_datetime(datetime, true, expYear, expMonth, expDay, expHour, expMinute, expSecond,               \
                          expSecondAndFraction, true, false, 0, 0)

#define check_nok_string_datetime(datetime)                           \
    {                                                                 \
        SOPC_tm res;                                                  \
        size_t len = strlen(datetime);                                \
        bool parseRes = SOPC_tm_FromXsdDateTime(datetime, len, &res); \
        ck_assert(parseRes == false);                                 \
    }

START_TEST(test_string_datetime_no_timezone)
{
    // Nominal
    check_ok_string_datetime_no_timezeone("2021-02-16T18:13:01.168764999", 2021, 02, 16, 18, 13, 01,
                                          (double) 1.168764999);

    // Invalid separators / format
    check_nok_string_datetime("2021+02-16T18:13:01.168764999");
    check_nok_string_datetime("2021");

    check_nok_string_datetime("2021-02.16T18:13:01.168764999");
    check_nok_string_datetime("2021-0216T18:13:01.168764999");
    check_nok_string_datetime("2021-02");

    check_nok_string_datetime("2021-02-16X18:13:01.168764999");
    check_nok_string_datetime("2021-02-16");

    check_nok_string_datetime("2021-02-16T18-13:01.168764999");
    check_nok_string_datetime("2021-02-16T18:1301.168764999");
    check_nok_string_datetime("2021-02-16T18");

    check_nok_string_datetime("2021-02-16T18:13.01.168764999");
    check_nok_string_datetime("2021-02-16T18:13");
    check_nok_string_datetime("2021-02-16T18:13:01-168764999");
    // Complete date without second fraction
    check_ok_string_datetime_no_timezeone("2021-02-16T18:13:01", 2021, 02, 16, 18, 13, 01, (double) 1);

    // Negative year ok
    check_ok_string_datetime_no_timezeone("-2021-02-16T18:13:01.168764999", -2021, 02, 16, 18, 13, 01,
                                          (double) 1.168764999);

    // Valid min and max years
    check_ok_string_datetime_no_timezeone("-32768-02-16T18:13:01.168764999", INT16_MIN, 02, 16, 18, 13, 01,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("32767-02-16T18:13:01.168764999", INT16_MAX, 02, 16, 18, 13, 01,
                                          (double) 1.168764999);

    // Invalid out of [min, max] years range
    check_nok_string_datetime("-32769-02-16T18:13:01.168764999");
    check_nok_string_datetime("32768-02-16T18:13:01.168764999");

    // First month
    check_ok_string_datetime_no_timezeone("2021-01-16T18:13:01.168764999", 2021, 01, 16, 18, 13, 01,
                                          (double) 1.168764999);
    // Last month
    check_ok_string_datetime_no_timezeone("2021-12-16T18:13:01.168764999", 2021, 12, 16, 18, 13, 01,
                                          (double) 1.168764999);

    // Invalid out of [min, max] months range
    check_nok_string_datetime("2021-00-16T18:13:01.168764999");

    check_nok_string_datetime("2021-13-16T18:13:01.168764999");

    // First day
    check_ok_string_datetime_no_timezeone("2021-02-01T18:13:01.168764999", 2021, 2, 1, 18, 13, 1, (double) 1.168764999);
    // Last day of each month
    check_ok_string_datetime_no_timezeone("2021-01-31T18:13:01.168764999", 2021, 1, 31, 18, 13, 1,
                                          (double) 1.168764999);

    check_ok_string_datetime_no_timezeone("2021-02-28T18:13:01.168764999", 2021, 2, 28, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("1700-02-28T18:13:01.168764999", 1700, 2, 28, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2000-02-29T18:13:01.168764999", 2000, 2, 29, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2004-02-29T18:13:01.168764999", 2004, 2, 29, 18, 13, 1,
                                          (double) 1.168764999);

    check_ok_string_datetime_no_timezeone("2021-03-31T18:13:01.168764999", 2021, 3, 31, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-04-30T18:13:01.168764999", 2021, 4, 30, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-05-31T18:13:01.168764999", 2021, 5, 31, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-06-30T18:13:01.168764999", 2021, 6, 30, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-07-31T18:13:01.168764999", 2021, 7, 31, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-08-31T18:13:01.168764999", 2021, 8, 31, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-09-30T18:13:01.168764999", 2021, 9, 30, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-10-31T18:13:01.168764999", 2021, 10, 31, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-11-30T18:13:01.168764999", 2021, 11, 30, 18, 13, 1,
                                          (double) 1.168764999);
    check_ok_string_datetime_no_timezeone("2021-12-31T18:13:01.168764999", 2021, 12, 31, 18, 13, 1,
                                          (double) 1.168764999);
    // Invalid out of [min, max] days range
    check_nok_string_datetime("2021-01-00T18:13:01.168764999");
    // Max 31
    check_nok_string_datetime("2021-01-32T18:13:01.168764999");
    check_nok_string_datetime("2021-03-32T18:13:01.168764999");
    check_nok_string_datetime("2021-05-32T18:13:01.168764999");
    check_nok_string_datetime("2021-07-32T18:13:01.168764999");
    check_nok_string_datetime("2021-08-32T18:13:01.168764999");
    check_nok_string_datetime("2021-10-32T18:13:01.168764999");
    check_nok_string_datetime("2021-12-32T18:13:01.168764999");
    // Max 30
    check_nok_string_datetime("2021-04-31T18:13:01.168764999");
    check_nok_string_datetime("2021-06-31T18:13:01.168764999");
    check_nok_string_datetime("2021-09-31T18:13:01.168764999");
    check_nok_string_datetime("2021-11-31T18:13:01.168764999");
    // Max 29
    check_nok_string_datetime("2000-02-30T18:13:01.168764999");
    check_nok_string_datetime("2004-02-30T18:13:01.168764999");
    // Max 28
    check_nok_string_datetime("2021-02-29T18:13:01.168764999");
    check_nok_string_datetime("1700-02-29T18:13:01.168764999");

    // First hour
    check_ok_string_datetime_no_timezeone("2021-02-16T00:13:01.168764999", 2021, 02, 16, 00, 13, 01,
                                          (double) 1.168764999);
    // Last hour
    check_ok_string_datetime_no_timezeone("2021-02-16T23:13:01.168764999", 2021, 02, 16, 23, 13, 01,
                                          (double) 1.168764999);
    // Hour == 24 only allowed if everything else is 0
    check_ok_string_datetime_no_timezeone("2021-02-16T24:00:00.0000000", 2021, 02, 16, 24, 00, 00, (double) 0);
    check_ok_string_datetime_no_timezeone("2021-02-16T24:00:00", 2021, 02, 16, 24, 00, 00, (double) 0);

    // Hour == 24 not allowed if anything else != 0
    check_nok_string_datetime("2021-02-16T24:13:00");
    check_nok_string_datetime("2021-02-16T24:00:02");
    check_nok_string_datetime("2021-02-16T24:00:00.0000000000001");
    // First minute
    check_ok_string_datetime_no_timezeone("2021-02-16T18:00:01.168764999", 2021, 02, 16, 18, 00, 01,
                                          (double) 1.168764999);
    // Last minute
    check_ok_string_datetime_no_timezeone("2021-02-16T18:59:01.168764999", 2021, 02, 16, 18, 59, 01,
                                          (double) 1.168764999);

    // Invalid minute (> max)
    check_nok_string_datetime("2021-02-16T18:60:01.168764999");

    // First second
    check_ok_string_datetime_no_timezeone("2021-02-16T18:13:00.000000000", 2021, 02, 16, 18, 13, 00, (double) 0);
    check_ok_string_datetime_no_timezeone("2021-02-16T18:13:00", 2021, 02, 16, 18, 13, 00, (double) 0);

    // Last second
    check_ok_string_datetime_no_timezeone("2021-02-16T18:13:59", 2021, 02, 16, 18, 13, 59, (double) 59);
    check_ok_string_datetime_no_timezeone("2021-02-16T18:13:59.9999999", 2021, 02, 16, 18, 13, 59, (double) 59.9999999);

    // Invalid second (> max)
    check_nok_string_datetime("2021-02-16T18:13:60.168764999");
}
END_TEST

#define check_ok_string_datetime(datetime, expYear, expMonth, expDay, expHour, expMinute, expSecond,            \
                                 expSecondAndFraction, expUtc, expUtc_neg_off, expUtc_hour_off, expUtc_min_off) \
    check_string_datetime(datetime, true, expYear, expMonth, expDay, expHour, expMinute, expSecond,             \
                          expSecondAndFraction, expUtc, expUtc_neg_off, expUtc_hour_off, expUtc_min_off)

START_TEST(test_string_datetime_with_timezone)
{
    // Nominal UTC+/-0
    check_ok_string_datetime("2021-02-16T18:13:01.168764999Z", 2021, 02, 16, 18, 13, 01, (double) 1.168764999, true,
                             false, 0, 0);
    check_ok_string_datetime("2021-02-16T18:13:01.168764999+00:00", 2021, 02, 16, 18, 13, 01, (double) 1.168764999,
                             true, false, 0, 0);
    check_ok_string_datetime("2021-02-16T18:13:01.168764999-00:00", 2021, 02, 16, 18, 13, 01, (double) 1.168764999,
                             true, true, 0, 0);
    // Nominal UTC+/-01:30
    check_ok_string_datetime("2021-02-16T18:13:01.168764999+01:30", 2021, 02, 16, 18, 13, 01, (double) 1.168764999,
                             false, false, 01, 30);
    check_ok_string_datetime("2021-02-16T18:13:01.168764999-01:30", 2021, 02, 16, 18, 13, 01, (double) 1.168764999,
                             false, true, 01, 30);

    // Invalid separators / format
    check_nok_string_datetime("2021-02-16T18:13:01.168764999X");
    check_nok_string_datetime("2021-02-16T18:13:01.168764999:03:45");
    check_nok_string_datetime("2021-02-16T18:13:01.168764999-");
    check_nok_string_datetime("2021-02-16T18:13:01.168764999+");
    check_nok_string_datetime("2021-02-16T18:13:01.168764999+01:30+");

    // First hour offset
    check_ok_string_datetime("2021-02-16T00:13:01.168764999-14:00", 2021, 02, 16, 00, 13, 01, (double) 1.168764999,
                             false, true, 14, 00);
    check_ok_string_datetime("2021-02-16T18:13:01.168764999-13:59", 2021, 02, 16, 18, 13, 01, (double) 1.168764999,
                             false, true, 13, 59);

    // Last hour offset
    check_ok_string_datetime("2021-02-16T00:13:01.168764999+14:00", 2021, 02, 16, 00, 13, 01, (double) 1.168764999,
                             false, false, 14, 00);
    check_ok_string_datetime("2021-02-16T18:13:01.168764999+13:59", 2021, 02, 16, 18, 13, 01, (double) 1.168764999,
                             false, false, 13, 59);

    // Hour == 14 not allowed if minute != 0
    check_nok_string_datetime("2021-02-16T00:13:01.168764999-14:01");
    check_nok_string_datetime("2021-02-16T00:13:01.168764999+14:01");
    // Hour > 14 not allowed
    check_nok_string_datetime("2021-02-16T00:13:01.168764999-15:00");
    check_nok_string_datetime("2021-02-16T00:13:01.168764999+15:00");

    // First minute
    check_ok_string_datetime("2021-02-16T00:13:01.168764999+12:00", 2021, 02, 16, 00, 13, 01, (double) 1.168764999,
                             false, false, 12, 00);
    // Last minute
    check_ok_string_datetime("2021-02-16T00:13:01.168764999+12:59", 2021, 02, 16, 00, 13, 01, (double) 1.168764999,
                             false, false, 12, 59);
    // Invalid minute (> max)
    check_nok_string_datetime("2021-02-16T00:13:01.168764999+12:60");
}

// Async queue

START_TEST(test_async_queue)
{
    void* arg = NULL;
    SOPC_AsyncQueue* queue = NULL;
    int paramAndRes = -10;
    int paramAndRes2 = 0;
    SOPC_ReturnStatus status = SOPC_AsyncQueue_Init(&queue, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &paramAndRes);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &paramAndRes2);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_AsyncQueue_BlockingDequeue(queue, &arg);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(arg == &paramAndRes);
    status = SOPC_AsyncQueue_NonBlockingDequeue(queue, &arg);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(arg == &paramAndRes2);
    status = SOPC_AsyncQueue_NonBlockingDequeue(queue, &arg);
    ck_assert(status == SOPC_STATUS_WOULD_BLOCK);
    SOPC_AsyncQueue_Free(&queue);
}
END_TEST

typedef struct AsyncQueue_Element
{
    SOPC_AsyncQueue* queue;
    uint8_t nbMsgs;
    int32_t dequeue_pending;
    int32_t success;
} AsyncQueue_Element;

static void* test_async_queue_blocking_dequeue_fct(void* args)
{
    void* arg = NULL;
    SOPC_ReturnStatus status;
    AsyncQueue_Element* param = (AsyncQueue_Element*) args;
    uint8_t nbDequeued = 0;
    uint8_t lastValueDeq = 0;
    bool success = true;
    // Dequeue expected number of messages with increased value sequence as action parameter
    while (nbDequeued < param->nbMsgs && success != false)
    {
        SOPC_Atomic_Int_Add(&param->dequeue_pending, 1);
        status = SOPC_AsyncQueue_BlockingDequeue(param->queue, &arg);
        SOPC_Atomic_Int_Add(&param->dequeue_pending, -1);
        nbDequeued++;
        assert(status == SOPC_STATUS_OK);
        if (*((uint8_t*) arg) == lastValueDeq + 1)
        {
            lastValueDeq++;
        }
        else
        {
            success = false;
        }
    }

    SOPC_Atomic_Int_Add(&param->success, success ? 1 : 0);
    return NULL;
}

static void* test_async_queue_nonblocking_dequeue_fct(void* args)
{
    void* arg = NULL;
    SOPC_ReturnStatus status;
    AsyncQueue_Element* param = (AsyncQueue_Element*) args;
    uint8_t nbDequeued = 0;
    uint8_t lastValueDeq = 0;
    bool success = true;
    // Dequeue expected number of messages with increased value sequence as action parameter
    while (nbDequeued < param->nbMsgs && success != false)
    {
        status = SOPC_AsyncQueue_NonBlockingDequeue(param->queue, &arg);
        if (status == SOPC_STATUS_OK)
        {
            nbDequeued++;
            assert(status == SOPC_STATUS_OK);
            if (*((uint8_t*) arg) == lastValueDeq + 1)
            {
                lastValueDeq++;
            }
            else
            {
                success = false;
            }
        }
        else
        {
            if (status != SOPC_STATUS_WOULD_BLOCK)
            {
                success = false;
            }
        }
        SOPC_Sleep(10);
    }
    param->success = success;
    return NULL;
}

START_TEST(test_async_queue_threads)
{
    Thread thread;
    AsyncQueue_Element params;
    SOPC_AsyncQueue* queue;
    uint8_t one = 1;
    uint8_t two = 2;
    uint8_t three = 3;
    uint8_t four = 4;
    uint8_t five = 5;
    SOPC_ReturnStatus status = SOPC_AsyncQueue_Init(&queue, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    params.success = 0;
    params.dequeue_pending = 0;
    params.nbMsgs = 5;
    params.queue = queue;
    // Nominal behavior of async queue FIFO (blocking dequeue)
    status = SOPC_Thread_Create(&thread, test_async_queue_blocking_dequeue_fct, &params, "B dequeue");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(wait_value(&params.dequeue_pending, 1));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &one));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &two));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &three));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &four));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &five));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Join(thread));
    ck_assert_int_eq(1, SOPC_Atomic_Int_Get(&params.success));

    // Nominal behavior of async queue FIFO (non blocking dequeue)
    SOPC_Atomic_Int_Set(&params.success, 0);
    SOPC_Atomic_Int_Set(&params.dequeue_pending, 0);
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_Thread_Create(&thread, test_async_queue_nonblocking_dequeue_fct, &params, "NB dequeue"));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &one));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &two));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &three));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &four));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &five));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Join(thread));
    ck_assert_int_eq(1, SOPC_Atomic_Int_Get(&params.success));

    SOPC_AsyncQueue_Free(&queue);
}
END_TEST

START_TEST(test_ua_encoder_endianness_mgt)
{
    int16_t v16 = 0;
    uint16_t vu16 = 0;
    int32_t v32 = 0;
    uint32_t vu32 = 0;
    int64_t v64 = 0;
    uint64_t vu64 = 0;
    float vfloat = 0.0;
    double vdouble = 0.0;

    uint8_t* bytes = NULL;

    // Test encoding with same endianness in machine and UA binary
    SOPC_Helper_Endianness_SetInteger(SOPC_Endianness_LittleEndian);
    bytes = (uint8_t*) &v16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_Int16(&v16);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC);

    bytes = (uint8_t*) &vu16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_UInt16(&vu16);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC);

    bytes = (uint8_t*) &v32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_Int32(&v32);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC && bytes[2] == 0xCD && bytes[3] == 0xDE);

    bytes = (uint8_t*) &vu32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_UInt32(&vu32);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC && bytes[2] == 0xCD && bytes[3] == 0xDE);

    bytes = (uint8_t*) &v64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_Int64(&v64);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x11 && bytes[2] == 0x22 && bytes[3] == 0x33 && bytes[4] == 0xAB &&
              bytes[5] == 0xBC && bytes[6] == 0xCD && bytes[7] == 0xDE);

    bytes = (uint8_t*) &vu64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_UInt64(&vu64);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x11 && bytes[2] == 0x22 && bytes[3] == 0x33 && bytes[4] == 0xAB &&
              bytes[5] == 0xBC && bytes[6] == 0xCD && bytes[7] == 0xDE);

    SOPC_Helper_Endianness_SetFloat(SOPC_Endianness_LittleEndian);
    bytes = (uint8_t*) &vfloat;
    bytes[0] = 0x00;
    bytes[1] = 0x00;
    bytes[2] = 0xD0;
    bytes[3] = 0xC0;
    SOPC_EncodeDecode_Float(&vfloat);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xD0 && bytes[3] == 0xC0);

    bytes = (uint8_t*) &vdouble;
    bytes[0] = 0x00;
    bytes[1] = 0x00;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    bytes[4] = 0x00;
    bytes[5] = 0x00;
    bytes[6] = 0x1A;
    bytes[7] = 0xC0;
    SOPC_EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0x00 && bytes[3] == 0x00 && bytes[4] == 0x00 &&
              bytes[5] == 0x00 && bytes[6] == 0x1A && bytes[7] == 0xC0);

    // Test encoding with different endianness in machine and UA binary
    SOPC_Helper_Endianness_SetInteger(SOPC_Endianness_BigEndian);
    bytes = (uint8_t*) &v16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_Int16(&v16);
    ck_assert(bytes[1] == 0xAB && bytes[0] == 0xBC);

    bytes = (uint8_t*) &vu16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_UInt16(&vu16);
    ck_assert(bytes[1] == 0xAB && bytes[0] == 0xBC);

    bytes = (uint8_t*) &v32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_Int32(&v32);
    ck_assert(bytes[3] == 0xAB && bytes[2] == 0xBC && bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &vu32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_UInt32(&vu32);
    ck_assert(bytes[3] == 0xAB && bytes[2] == 0xBC && bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &v64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_Int64(&v64);
    ck_assert(bytes[7] == 0x00 && bytes[6] == 0x11 && bytes[5] == 0x22 && bytes[4] == 0x33 && bytes[3] == 0xAB &&
              bytes[2] == 0xBC && bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &vu64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_UInt64(&vu64);
    ck_assert(bytes[7] == 0x00 && bytes[6] == 0x11 && bytes[5] == 0x22 && bytes[4] == 0x33 && bytes[3] == 0xAB &&
              bytes[2] == 0xBC && bytes[1] == 0xCD && bytes[0] == 0xDE);

    SOPC_Helper_Endianness_SetFloat(SOPC_Endianness_BigEndian);
    bytes = (uint8_t*) &vfloat;
    bytes[0] = 0xC0;
    bytes[1] = 0xD0;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    SOPC_EncodeDecode_Float(&vfloat);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0xD0 && bytes[3] == 0xC0);

    bytes = (uint8_t*) &vdouble;
    bytes[0] = 0xC0;
    bytes[1] = 0x1A;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    bytes[4] = 0x00;
    bytes[5] = 0x00;
    bytes[6] = 0x00;
    bytes[7] = 0x00;
    SOPC_EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0x00 && bytes[3] == 0x00 && bytes[4] == 0x00 &&
              bytes[5] == 0x00 && bytes[6] == 0x1A && bytes[7] == 0xC0);

    // Test encoding of ARM's middle endian doubles
    SOPC_Helper_Endianness_SetFloat(SOPC_Endianness_FloatARMMiddleEndian);
    bytes = (uint8_t*) &vfloat;
    bytes[0] = 0xC0;
    bytes[1] = 0xD0;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    SOPC_EncodeDecode_Float(&vfloat);
    ck_assert(bytes[0] == 0xC0 && bytes[1] == 0xD0 && bytes[2] == 0x00 && bytes[3] == 0x00);

    bytes = (uint8_t*) &vdouble;
    bytes[0] = 0xC0;
    bytes[1] = 0x1A;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    bytes[4] = 0x00;
    bytes[5] = 0x00;
    bytes[6] = 0x00;
    bytes[7] = 0x00;
    SOPC_EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 && bytes[2] == 0x00 && bytes[3] == 0x00 && bytes[4] == 0xC0 &&
              bytes[5] == 0x1A && bytes[6] == 0x00 && bytes[7] == 0x00);
}
END_TEST

START_TEST(test_ua_encoder_basic_types)
{
    SOPC_Helper_EndiannessCfg_Initialize(); // Necessary to initialize endianness configuration
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* buffer = SOPC_Buffer_Create(100);

    SOPC_Buffer* bufferFull = SOPC_Buffer_Create(8);

    // Test Byte nominal and degraded cases
    //// Nominal write
    SOPC_Byte byte = 0xAE;
    status = SOPC_Byte_Write(&byte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == 0xAE);
    //// Degraded write
    status = SOPC_Byte_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Byte_Write(&byte, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 8; // Set buffer full
    status = SOPC_Byte_Write(&byte, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    byte = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Byte_Read(&byte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(byte == 0xAE);
    //// Degraded read
    status = SOPC_Byte_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Byte_Read(&byte, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Byte_Read(&byte, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Boolean nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    SOPC_Boolean boolv = false;
    status = SOPC_Boolean_Write(&boolv, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == false);
    boolv = 1; // not FALSE
    status = SOPC_Boolean_Write(&boolv, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[1] == 1);
    boolv = 2; // not FALSE
    status = SOPC_Boolean_Write(&boolv, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[2] == 1); // True value always encoded as 1

    //// Degraded write
    status = SOPC_Boolean_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Write(&boolv, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Write(&boolv, bufferFull, 0); // Test with full buffer
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    boolv = 4;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    buffer->data[2] = 2;                // Simulates a true value received as 2
    status = SOPC_Boolean_Read(&boolv, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(boolv == false);
    status = SOPC_Boolean_Read(&boolv, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(boolv == 1);
    status = SOPC_Boolean_Read(&boolv, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(boolv == 1); // True value always decoded as 1
    //// Degraded read
    status = SOPC_Boolean_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Read(&boolv, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Read(&boolv, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test SByteuv16nal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    SOPC_SByte sbyte = -1;
    status = SOPC_SByte_Write(&sbyte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == 0xFF);
    //// Degraded write
    status = SOPC_SByte_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Write(&sbyte, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Write(&sbyte, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    sbyte = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_SByte_Read(&sbyte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(sbyte == -1);
    //// Degraded read
    status = SOPC_SByte_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Read(&sbyte, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Read(&sbyte, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Int16 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    int16_t v16 = -2;

    status = SOPC_Int16_Write(&v16, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF);
    //// Degraded write
    status = SOPC_Int16_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int16_Write(&v16, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 7; // Set buffer almost full (1 byte left)
    status = SOPC_Int16_Write(&v16, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    v16 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Int16_Read(&v16, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(v16 == -2);
    //// Degraded read
    status = SOPC_Int16_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int16_Read(&v16, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int16_Read(&v16, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test UInt16 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    uint16_t vu16 = 2;

    status = SOPC_UInt16_Write(&vu16, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x02 && buffer->data[1] == 0x00);
    //// Degraded write
    status = SOPC_UInt16_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt16_Write(&vu16, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 7; // Set buffer almost full (1 byte left)
    status = SOPC_UInt16_Write(&vu16, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vu16 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_UInt16_Read(&vu16, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(vu16 == 2);
    //// Degraded read
    status = SOPC_UInt16_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt16_Read(&vu16, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt16_Read(&vu16, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Int32 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    int32_t v32 = -2;

    status = SOPC_Int32_Write(&v32, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF && buffer->data[2] == 0xFF && buffer->data[3] == 0xFF);
    //// Degraded write
    status = SOPC_Int32_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int32_Write(&v32, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_Int32_Write(&v32, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    v32 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Int32_Read(&v32, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(v32 == -2);
    //// Degraded read
    status = SOPC_Int32_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int32_Read(&v32, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int32_Read(&v32, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test UInt32 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    uint32_t vu32 = 1048578;

    status = SOPC_UInt32_Write(&vu32, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x02 && buffer->data[1] == 0x00 && buffer->data[2] == 0x10 && buffer->data[3] == 0x00);
    //// Degraded write
    status = SOPC_UInt32_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt32_Write(&vu32, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_UInt32_Write(&vu32, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vu32 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_UInt32_Read(&vu32, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(vu32 == 1048578);
    //// Degraded read
    status = SOPC_UInt32_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt32_Read(&vu32, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt32_Read(&vu32, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Int64 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    int64_t v64 = -2;

    status = SOPC_Int64_Write(&v64, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF && buffer->data[2] == 0xFF &&
              buffer->data[3] == 0xFF && buffer->data[4] == 0xFF && buffer->data[5] == 0xFF &&
              buffer->data[6] == 0xFF && buffer->data[7] == 0xFF);
    //// Degraded write
    status = SOPC_Int64_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int64_Write(&v64, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_Int64_Write(&v64, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    v64 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Int64_Read(&v64, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(v64 == -2);
    //// Degraded read
    status = SOPC_Int64_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int64_Read(&v64, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int64_Read(&v64, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test UInt64 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    uint64_t vu64 = 0x100000000000002;

    status = SOPC_UInt64_Write(&vu64, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x02 && buffer->data[1] == 0x00 && buffer->data[2] == 0x00 &&
              buffer->data[3] == 0x00 && buffer->data[4] == 0x00 && buffer->data[5] == 0x00 &&
              buffer->data[6] == 0x00 && buffer->data[7] == 0x01);
    //// Degraded write
    status = SOPC_UInt64_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt64_Write(&vu64, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_UInt64_Write(&vu64, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vu64 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_UInt64_Read(&vu64, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(vu64 == 0x100000000000002);
    //// Degraded read
    status = SOPC_UInt64_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt64_Read(&vu64, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt64_Read(&vu64, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Float nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    float vfloat = -6.5;

    status = SOPC_Float_Write(&vfloat, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x00 && buffer->data[1] == 0x00 && buffer->data[2] == 0xD0 && buffer->data[3] == 0xC0);
    //// Degraded write
    status = SOPC_Float_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Float_Write(&vfloat, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_Float_Write(&vfloat, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vfloat = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Float_Read(&vfloat, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(fabsf(vfloat + (float) 6.5) <= 0);
    //// Degraded read
    status = SOPC_Float_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Float_Read(&vfloat, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Float_Read(&vfloat, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Double nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    double vdouble = -6.5;

    status = SOPC_Double_Write(&vdouble, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x00 && buffer->data[1] == 0x00 && buffer->data[2] == 0x00 &&
              buffer->data[3] == 0x00 && buffer->data[4] == 0x00 && buffer->data[5] == 0x00 &&
              buffer->data[6] == 0x1A && buffer->data[7] == 0xC0);
    //// Degraded write
    status = SOPC_Double_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Double_Write(&vdouble, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_Double_Write(&vdouble, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vdouble = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Double_Read(&vdouble, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(fabs(vdouble + (double) 6.5) <= 0);

    //// Degraded read
    status = SOPC_Double_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Double_Read(&vdouble, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Double_Read(&vdouble, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test DateTime nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    SOPC_DateTime vDate;
    SOPC_DateTime_Initialize(&vDate);
    vDate = -2;

    status = SOPC_DateTime_Write(&vDate, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF && buffer->data[2] == 0xFF &&
              buffer->data[3] == 0xFF && buffer->data[4] == 0xFF && buffer->data[5] == 0xFF &&
              buffer->data[6] == 0xFF && buffer->data[7] == 0xFF);
    //// Degraded write
    status = SOPC_DateTime_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_DateTime_Write(&vDate, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_DateTime_Write(&vDate, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_DateTime_Clear(&vDate);
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_DateTime_Read(&vDate, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(vDate == -2);
    //// Degraded read
    status = SOPC_DateTime_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_DateTime_Read(&vDate, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_DateTime_Read(&vDate, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    SOPC_Buffer_Delete(buffer);
    SOPC_Buffer_Delete(bufferFull);
}
END_TEST

START_TEST(test_ua_encoder_other_types)
{
    SOPC_Helper_EndiannessCfg_Initialize(); // Necessary to initialize endianness configuration
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Buffer* buffer = SOPC_Buffer_Create(100);

    SOPC_Buffer* bufferFull = SOPC_Buffer_Create(8);

    //////////////////////////////////////////////
    // Test ByteString nominal and degraded cases
    //// Nominal write
    SOPC_ByteString* bs = SOPC_ByteString_Create();
    SOPC_ByteString* bs2 = SOPC_ByteString_Create();
    uint8_t boyString[3] = {0x42, 0x6F, 0x79}; // Boy

    ////// Empty string
    bs->Length = 0;
    status = SOPC_ByteString_Write(bs, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    bs->Length = -1;
    status = SOPC_ByteString_Write(bs, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    bs->Length = -10;
    status = SOPC_ByteString_Write(bs, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    /////// Non empty bytestring
    SOPC_Buffer_Reset(buffer);
    status = SOPC_ByteString_CopyFromBytes(bs, boyString, 3);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ByteString_Write(bs, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == 0x03);
    ck_assert(buffer->data[1] == 0x00);
    ck_assert(buffer->data[2] == 0x00);
    ck_assert(buffer->data[3] == 0x00);
    ck_assert(buffer->data[4] == 0x42);
    ck_assert(buffer->data[5] == 0x6F);
    ck_assert(buffer->data[6] == 0x79);

    //// Degraded write
    status = SOPC_ByteString_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_ByteString_Write(bs, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_ByteString_Write(bs, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(SOPC_ByteString_Equal(bs, bs2) != false);
    ck_assert(bs2->Length == 3);
    ck_assert(bs2->Data[0] == 0x42);
    ck_assert(bs2->Data[1] == 0x6F);
    ck_assert(bs2->Data[2] == 0x79);

    ////// Read 0 length bytestring
    buffer->data[0] = 0x00;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0xFF;
    buffer->data[2] = 0xFF;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_ByteString_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_ByteString_Read(bs, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_ByteString_Read(bs, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    SOPC_ByteString_Delete(bs);
    SOPC_ByteString_Delete(bs2);
    bs = NULL;
    bs2 = NULL;

    /////////////////////////////////////////
    // Test String nominal and degraded cases
    //// Nominal write
    SOPC_Buffer_Reset(buffer);
    SOPC_String str;
    SOPC_String_Initialize(&str);
    SOPC_String str2;
    SOPC_String_Initialize(&str2);

    ////// Empty string
    str.Length = 0;
    status = SOPC_String_Write(&str, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    str.Length = -1;
    status = SOPC_String_Write(&str, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    str.Length = -10;
    status = SOPC_String_Write(&str, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    /////// Non empty bytestring
    SOPC_Buffer_Reset(buffer);
    status = SOPC_String_AttachFromCstring(&str, "Boy");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_Write(&str, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == 0x03);
    ck_assert(buffer->data[1] == 0x00);
    ck_assert(buffer->data[2] == 0x00);
    ck_assert(buffer->data[3] == 0x00);
    ck_assert(buffer->data[4] == 0x42);
    ck_assert(buffer->data[5] == 0x6F);
    ck_assert(buffer->data[6] == 0x79);

    //// Degraded write
    status = SOPC_String_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_String_Write(&str, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_String_Write(&str, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(SOPC_String_Equal(&str, &str2) != false);
    ck_assert(memcmp(SOPC_String_GetRawCString(&str2), "Boy", 3) == 0);

    ////// Read 0 length bytestring
    buffer->data[0] = 0x00;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0xFF;
    buffer->data[2] = 0xFF;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_String_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_String_Read(&str, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_String_Read(&str, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    SOPC_String_Clear(&str);
    SOPC_String_Clear(&str2);

    //////////////////////////////////////////
    // Test XmlElement nominal and degraded cases
    //// Nominal write
    SOPC_Buffer_Reset(buffer);
    SOPC_XmlElement xmlElt;
    SOPC_XmlElement_Initialize(&xmlElt);
    SOPC_XmlElement xmlElt2;
    SOPC_XmlElement_Initialize(&xmlElt2);
    uint8_t balA[3] = {0x3C, 0x41, 0x3E}; // <A>

    ////// Empty string
    xmlElt.Length = 0;
    status = SOPC_XmlElement_Write(&xmlElt, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    xmlElt.Length = -1;
    status = SOPC_XmlElement_Write(&xmlElt, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    xmlElt.Length = -10;
    status = SOPC_XmlElement_Write(&xmlElt, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    /////// Non empty bytestring
    SOPC_Buffer_Reset(buffer);
    xmlElt.Data = SOPC_Malloc(sizeof(SOPC_Byte) * 3);
    ck_assert(xmlElt.Data != NULL);
    ck_assert(memcpy(xmlElt.Data, balA, 3) == xmlElt.Data);
    xmlElt.Length = 3;
    status = SOPC_XmlElement_Write(&xmlElt, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == 0x03);
    ck_assert(buffer->data[1] == 0x00);
    ck_assert(buffer->data[2] == 0x00);
    ck_assert(buffer->data[3] == 0x00);
    ck_assert(buffer->data[4] == 0x3C);
    ck_assert(buffer->data[5] == 0x41);
    ck_assert(buffer->data[6] == 0x3E);

    //// Degraded write
    status = SOPC_XmlElement_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_XmlElement_Write(&xmlElt, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_XmlElement_Write(&xmlElt, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(memcmp(xmlElt.Data, xmlElt2.Data, 3) == 00);
    ck_assert(xmlElt2.Length == 3);
    ck_assert(xmlElt2.Data[0] == 0x3C);
    ck_assert(xmlElt2.Data[1] == 0x41);
    ck_assert(xmlElt2.Data[2] == 0x3E);

    ////// Read 0 length bytestring
    buffer->data[0] = 0x00;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0xFF;
    buffer->data[2] = 0xFF;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_XmlElement_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_XmlElement_Read(&xmlElt, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_XmlElement_Read(&xmlElt, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    SOPC_XmlElement_Clear(&xmlElt);
    SOPC_XmlElement_Clear(&xmlElt2);

    // Resize "full" msg buffer
    /////////////////////////////////////////
    SOPC_Buffer_Delete(bufferFull);
    bufferFull = SOPC_Buffer_Create(32);
    /////////////////////////////////////////
    // Test GUID nominal and degraded cases
    //// Nominal write
    SOPC_Buffer_Reset(buffer);
    SOPC_Guid guid;
    SOPC_Guid_Initialize(&guid);
    SOPC_Guid guid2;
    SOPC_Guid_Initialize(&guid2);
    guid.Data1 = 0x72962B91;
    guid.Data2 = 0xFA75;
    guid.Data3 = 0x4ae6;
    guid.Data4[0] = 0x8D;
    guid.Data4[1] = 0x28;
    guid.Data4[2] = 0xB4;
    guid.Data4[3] = 0x04;
    guid.Data4[4] = 0xDC;
    guid.Data4[5] = 0x7D;
    guid.Data4[6] = 0xAF;
    guid.Data4[7] = 0x63;

    status = SOPC_Guid_Write(&guid, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(buffer->data[0] == 0x91);
    ck_assert(buffer->data[1] == 0x2B);
    ck_assert(buffer->data[2] == 0x96);
    ck_assert(buffer->data[3] == 0x72);
    ck_assert(buffer->data[4] == 0x75);
    ck_assert(buffer->data[5] == 0xFA);
    ck_assert(buffer->data[6] == 0xE6);
    ck_assert(buffer->data[7] == 0x4A);
    ck_assert(buffer->data[8] == 0x8D);
    ck_assert(buffer->data[9] == 0x28);
    ck_assert(buffer->data[10] == 0xB4);
    ck_assert(buffer->data[11] == 0x04);
    ck_assert(buffer->data[12] == 0xDC);
    ck_assert(buffer->data[13] == 0x7D);
    ck_assert(buffer->data[14] == 0xAF);
    ck_assert(buffer->data[15] == 0x63);

    //// Degraded write
    status = SOPC_Guid_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Guid_Write(&guid, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 17; // Set buffer almost full (15 byte left)
    status = SOPC_Guid_Write(&guid, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Guid_Read(&guid2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(memcmp(&guid, &guid2, sizeof(SOPC_Guid)) == 0);

    //// Degraded read
    status = SOPC_Guid_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Guid_Read(&guid, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Guid_Read(&guid, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    SOPC_Guid_Clear(&guid);
    SOPC_Guid_Clear(&guid2);

    /////////////////////////////////////////
    // Test NodeId nominal and degraded cases
    //// Nominal write
    SOPC_Buffer_Reset(buffer);
    SOPC_NodeId nodeId;
    SOPC_NodeId_Initialize(&nodeId);
    SOPC_NodeId nodeId2;
    SOPC_NodeId_Initialize(&nodeId2);

    // Two bytes node id
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = OPCUA_NAMESPACE_INDEX;
    nodeId.Data.Numeric = 255;
    status = SOPC_NodeId_Write(&nodeId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Encoding byte
    ck_assert(buffer->data[0] == 0x00);
    // Id byte
    ck_assert(buffer->data[1] == 0xFF);

    // Four bytes node id (limit of two bytes representation on numeric value)
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = OPCUA_NAMESPACE_INDEX;
    nodeId.Data.Numeric = 256;
    status = SOPC_NodeId_Write(&nodeId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Encoding byte
    ck_assert(buffer->data[2] == 0x01);
    // Namespace byte
    ck_assert(buffer->data[3] == 0x00);
    // Id bytes
    ck_assert(buffer->data[4] == 0x00);
    ck_assert(buffer->data[5] == 0x01);

    // Four bytes node id (limit of two bytes representation on namespace value)
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = 1;
    nodeId.Data.Numeric = 255;
    status = SOPC_NodeId_Write(&nodeId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Encoding byte
    ck_assert(buffer->data[6] == 0x01);
    // Namespace byte
    ck_assert(buffer->data[7] == 0x01);
    // Id bytes
    ck_assert(buffer->data[8] == 0xFF);
    ck_assert(buffer->data[9] == 0x00);

    // Four bytes node id
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = 5;
    nodeId.Data.Numeric = 1025;
    status = SOPC_NodeId_Write(&nodeId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Encoding byte
    ck_assert(buffer->data[10] == 0x01);
    // Namespace byte
    ck_assert(buffer->data[11] == 0x05);
    // Id bytes
    ck_assert(buffer->data[12] == 0x01);
    ck_assert(buffer->data[13] == 0x04);

    // Numeric node id (limit of four bytes representation on numeric value)
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = 5;
    nodeId.Data.Numeric = 0x010000;
    status = SOPC_NodeId_Write(&nodeId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Encoding byte
    ck_assert(buffer->data[14] == 0x02);
    // Namespace bytes
    ck_assert(buffer->data[15] == 0x05);
    ck_assert(buffer->data[16] == 0x00);
    // Id bytes
    ck_assert(buffer->data[17] == 0x00);
    ck_assert(buffer->data[18] == 0x00);
    ck_assert(buffer->data[19] == 0x01);
    ck_assert(buffer->data[20] == 0x00);

    // Numeric node id (limit of four bytes representation on namespace value)
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = 256;
    nodeId.Data.Numeric = 1025;
    status = SOPC_NodeId_Write(&nodeId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Encoding byte
    ck_assert(buffer->data[21] == 0x02);
    // Namespace bytes
    ck_assert(buffer->data[22] == 0x00);
    ck_assert(buffer->data[23] == 0x01);
    // Id bytes
    ck_assert(buffer->data[24] == 0x01);
    ck_assert(buffer->data[25] == 0x04);
    ck_assert(buffer->data[26] == 0x00);
    ck_assert(buffer->data[27] == 0x00);

    // TODO: write all other types possibles !

    //// Degraded write
    status = SOPC_NodeId_Write(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_NodeId_Write(&nodeId, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 26; // Set buffer almost full (6 byte left)
    status = SOPC_NodeId_Write(&nodeId, bufferFull, 0);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    ////// Two bytes NodeId
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_NodeId_Read(&nodeId2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(nodeId2.Namespace == 0);
    ck_assert(nodeId2.Data.Numeric == 255);

    ////// Four bytes NodeId
    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(nodeId2.Namespace == OPCUA_NAMESPACE_INDEX);
    ck_assert(nodeId2.Data.Numeric == 256);

    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(nodeId2.Namespace == 1);
    ck_assert(nodeId2.Data.Numeric == 255);

    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(nodeId2.Namespace == 5);
    ck_assert(nodeId2.Data.Numeric == 1025);

    ////// Numeric NodeId
    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(nodeId2.Namespace == 5);
    ck_assert(nodeId2.Data.Numeric == 0x010000);

    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert(nodeId2.Namespace == 256);
    ck_assert(nodeId2.Data.Numeric == 1025);

    // TODO: read all other types possibles !

    //// Degraded read
    status = SOPC_NodeId_Read(NULL, buffer, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_NodeId_Read(&nodeId, NULL, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_NodeId_Read(&nodeId, buffer, 0); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    SOPC_NodeId_Clear(&nodeId);
    SOPC_NodeId_Clear(&nodeId2);

    SOPC_Buffer_Delete(buffer);
    SOPC_Buffer_Delete(bufferFull);
}
END_TEST

START_TEST(test_ua_decoder_allocation_limit)
{
    SOPC_Helper_EndiannessCfg_Initialize(); // Necessary to initialize endianness configuration
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_String s;
    SOPC_String_Initialize(&s);
    SOPC_ByteString bs;
    SOPC_ByteString_Initialize(&bs);
    SOPC_XmlElement xml;
    SOPC_XmlElement_Initialize(&xml);
    int32_t length = 0;
    SOPC_Variant v;
    SOPC_Variant_Initialize(&v);
    SOPC_Variant* pvar = NULL;
    uint32_t idx = 0;
    SOPC_Byte encodingByte = 0;
    SOPC_Boolean boolVal = false;
    SOPC_DiagnosticInfo diag;
    SOPC_DiagnosticInfo_Initialize(&diag);
    SOPC_DiagnosticInfo* pDiag = NULL;
    int32_t symbolicId = 0;

    SOPC_Buffer* buffer = SOPC_Buffer_Create(1024);

    status = SOPC_String_AttachFromCstring(&s, "An example of string !!! ");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Write in buffer the string and clear string
    status = SOPC_String_Write(&s, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_String_Clear(&s);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Nominal read of the string and clear string
    status = SOPC_String_Read(&s, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_String_Clear(&s);

    /* LIMIT SIZE OF A STRING */

    // Reset position to write to buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Change string length artificially in buffer
    length = SOPC_DEFAULT_MAX_STRING_LENGTH;
    SOPC_Int32_Write(&length, buffer, 0);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Degraded read of the string (buffer too short for the string length)
    status = SOPC_String_Read(&s, buffer, 0);
    // Expected buffer invalid state: not enough bytes
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Degraded read of the bytestring and clear bytestring
    status = SOPC_ByteString_Read(&bs, buffer, 0);
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);
    SOPC_ByteString_Clear(&bs);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Degraded read of the bytestring and clear bytestring
    status = SOPC_XmlElement_Read(&xml, buffer, 0);
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);
    SOPC_XmlElement_Clear(&xml);

    /* LIMIT SIZE + 1 OF A STRING */

    // Reset position to write to buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Change string length artificially in buffer
    length = SOPC_DEFAULT_MAX_STRING_LENGTH + 1;
    SOPC_Int32_Write(&length, buffer, 0);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Degraded read of the string (string too long)
    status = SOPC_String_Read(&s, buffer, 0);
    // Expected decoding error: string too long
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Nominal read of the bytestring and clear bytestring
    status = SOPC_ByteString_Read(&bs, buffer, 0);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Nominal read of the bytestring and clear bytestring
    status = SOPC_XmlElement_Read(&xml, buffer, 0);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* NOMINAL VARIANT ARRAY ELEMENTS */

    v.BuiltInTypeId = SOPC_Boolean_Id;
    v.ArrayType = SOPC_VariantArrayType_Array;
    v.Value.Array.Length = 10;
    v.Value.Array.Content.BooleanArr = SOPC_Calloc((size_t) v.Value.Array.Length, sizeof(SOPC_Boolean));
    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    /* LIMIT SIZE OF A VARIANT ARRAY ELEMENTS */

    // Set position to write to buffer: position 1 to write array length
    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Overwrite with limit length value
    length = SOPC_DEFAULT_MAX_ARRAY_LENGTH;
    status = SOPC_Int32_Write(&length, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);

    /* LIMIT SIZE + 1 OF A VARIANT ARRAY ELEMENTS */
    // Set position to write to buffer: position 1 to write array length
    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Overwrite with limit+1 length value
    length = SOPC_DEFAULT_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT SIZE + 1 OF A VARIANT MATRIX ELEMENTS */
    v.BuiltInTypeId = SOPC_Boolean_Id;
    v.ArrayType = SOPC_VariantArrayType_Matrix;
    v.Value.Matrix.Dimensions = 3;
    v.Value.Matrix.ArrayDimensions = SOPC_Malloc(sizeof(int32_t) * (size_t) v.Value.Matrix.Dimensions);
    ck_assert(v.Value.Matrix.ArrayDimensions != NULL);
    v.Value.Matrix.ArrayDimensions[0] = 3;
    v.Value.Matrix.ArrayDimensions[1] = 3;
    v.Value.Matrix.ArrayDimensions[2] = 1;
    v.Value.Matrix.Content.BooleanArr =
        SOPC_Calloc((size_t) v.Value.Matrix.ArrayDimensions[0] * (size_t) v.Value.Matrix.ArrayDimensions[1] *
                        (size_t) v.Value.Matrix.ArrayDimensions[2],
                    sizeof(SOPC_Boolean));
    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // Set position to write to buffer: position 1 to write array length
    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Overwrite with limit+1 length value
    length = SOPC_DEFAULT_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT SIZE + 1 OF A VARIANT MATRIX DIMENSIONS */
    v.BuiltInTypeId = SOPC_Boolean_Id;
    v.ArrayType = SOPC_VariantArrayType_Matrix;
    v.Value.Matrix.Dimensions = 3;
    v.Value.Matrix.ArrayDimensions = SOPC_Malloc(sizeof(int32_t) * (size_t) v.Value.Matrix.Dimensions);
    ck_assert(v.Value.Matrix.ArrayDimensions != NULL);
    v.Value.Matrix.ArrayDimensions[0] = 3;
    v.Value.Matrix.ArrayDimensions[1] = 3;
    v.Value.Matrix.ArrayDimensions[2] = 1;
    v.Value.Matrix.Content.BooleanArr =
        SOPC_Calloc((size_t) v.Value.Matrix.ArrayDimensions[0] * (size_t) v.Value.Matrix.ArrayDimensions[1] *
                        (size_t) v.Value.Matrix.ArrayDimensions[2],
                    sizeof(SOPC_Boolean));
    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // Set position to write to buffer: position = position - 4 * 4 bytes to write array dimensions length
    status = SOPC_Buffer_SetPosition(buffer, buffer->position - 4 * 4);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Overwrite with limit+1 length value
    length = SOPC_DEFAULT_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);

    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT SIZE + 1 OF A VARIANT ARRAY ELEMENTS (TEST NESTED VERSION OF CODE) */
    v.BuiltInTypeId = SOPC_Variant_Id;
    v.ArrayType = SOPC_VariantArrayType_Array;
    v.Value.Array.Length = 1;
    v.Value.Array.Content.VariantArr = SOPC_Malloc(sizeof(SOPC_Variant));
    SOPC_Variant_Initialize(&v.Value.Array.Content.VariantArr[0]);
    v.Value.Array.Content.VariantArr[0].BuiltInTypeId = SOPC_Boolean_Id;
    v.Value.Array.Content.VariantArr[0].ArrayType = SOPC_VariantArrayType_SingleValue;
    v.Value.Array.Content.VariantArr[0].Value.Boolean = true;
    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Overwrite with limit+1 length value
    length = SOPC_DEFAULT_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OUT_OF_MEMORY, status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT OF NESTED VARIANT ELEMENTS */

    // ENCODING A VARIANT WITH SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL nested levels
    // Variant/Array nested so limit has to be divided by 2, minus 1 for fields
    pvar = &v;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL / 2 - 2; idx++)
    {
        pvar->BuiltInTypeId = SOPC_Variant_Id;
        pvar->ArrayType = SOPC_VariantArrayType_Array;
        pvar->Value.Array.Length = 1;
        pvar->Value.Array.Content.VariantArr = SOPC_Malloc(sizeof(SOPC_Variant));
        ck_assert(pvar->Value.Array.Content.VariantArr != NULL);
        SOPC_Variant_Initialize(&pvar->Value.Array.Content.VariantArr[0]);
        pvar = pvar->Value.Array.Content.VariantArr;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // MANUAL ENCODING FOR THE SAME BELOW
    /*encodingByte = SOPC_Variant_Id | SOPC_VariantArrayValueFlag;
    length = 1;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // ArrayLength
        status = SOPC_Int32_Write(&length, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // Then next variant !
    }
     encodingByte = SOPC_Boolean_Id; // Boolean Id
    // EncodingMask byte
     status = SOPC_Byte_Write(&encodingByte, buffer, 0);
     ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Boolean value
     boolVal = true;
     status = SOPC_Boolean_Write(&boolVal, buffer, 0);
     ck_assert_int_eq(SOPC_STATUS_OK, status);*/
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // ENCODING A DATAVALUE WITH SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL Variant nested levels
    // Variant/DataValue nested so limit has to be divided by 2, minus one for fields
    pvar = &v;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL / 2 - 2; idx++)
    {
        pvar->BuiltInTypeId = SOPC_DataValue_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.DataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
        ck_assert(pvar->Value.DataValue != NULL);
        SOPC_DataValue_Initialize(pvar->Value.DataValue);
        SOPC_Variant_Initialize(&pvar->Value.DataValue->Value);
        pvar = &pvar->Value.DataValue->Value;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT + 1 OF NESTED VARIANT ELEMENTS */

    // TEST ENCODER FAILURE OF A VARIANT WITH SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL /2 nested levels
    // Nested Variant/Datavalue, so limit has to be divided by 2 minus 1 for fields
    pvar = &v;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL / 2 - 1; idx++)
    {
        pvar->BuiltInTypeId = SOPC_DataValue_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.DataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
        ck_assert(pvar->Value.DataValue != NULL);
        SOPC_DataValue_Initialize(pvar->Value.DataValue);
        SOPC_Variant_Initialize(&pvar->Value.DataValue->Value);
        pvar = &pvar->Value.DataValue->Value;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert(SOPC_STATUS_INVALID_STATE == status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // MANUAL ENCODING A VARIANT WITH SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL/2 nested levels
    // Nested Variant/Array so limit has to be divided by 2
    encodingByte = 152; // Variant Id + 2^7 bit to indicate an array
    length = 1;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL / 2 - 1; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // ArrayLength
        status = SOPC_Int32_Write(&length, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // Then next variant !
    }
    encodingByte = 1; // Boolean Id
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Boolean value
    boolVal = true;
    status = SOPC_Boolean_Write(&boolVal, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // TEST ENCODER FAILURE OF A DATAVALUE WITH SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL + 1 Variant nested levels
    pvar = &v;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL + 1; idx++)
    {
        pvar->BuiltInTypeId = SOPC_DataValue_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.DataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
        ck_assert(pvar->Value.DataValue != NULL);
        SOPC_DataValue_Initialize(pvar->Value.DataValue);
        SOPC_Variant_Initialize(&pvar->Value.DataValue->Value);
        pvar = &pvar->Value.DataValue->Value;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer, 0);
    ck_assert(SOPC_STATUS_INVALID_STATE == status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // MANUAL ENCODING OF A DATAVALUE WITH SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL + 1 Variant nested levels
    length = 1;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL + 1; idx++)
    {
        encodingByte = SOPC_DataValue_Id; // DataValue Id
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // DataValue
        encodingByte = SOPC_DataValue_NotNullValue; // It indicates there is a value and nothing else in DataValue
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);

        // Then next data value !
    }
    encodingByte = 1; // Boolean Id
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Boolean value
    boolVal = true;
    status = SOPC_Boolean_Write(&boolVal, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer, 0);
    ck_assert(SOPC_STATUS_INVALID_STATE == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT OF NESTED DIAG INFO */

    // ENCODING A DIAG INFO WITH SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL nested levels
    pDiag = &diag;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL; idx++)
    {
        pDiag->InnerDiagnosticInfo = SOPC_Malloc(sizeof(SOPC_DiagnosticInfo));
        ck_assert(pDiag->InnerDiagnosticInfo != NULL);
        SOPC_DiagnosticInfo_Initialize(pDiag->InnerDiagnosticInfo);
        pDiag = pDiag->InnerDiagnosticInfo;
    }
    pDiag->SymbolicId = 1000;

    status = SOPC_DiagnosticInfo_Write(&diag, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_DiagnosticInfo_Clear(&diag);

    // MANUAL ENCODING FOR THE SAME BELOW (~ avoiding not necessary fields)
    /*
    encodingByte = SOPC_DiagInfoEncoding_InnerDianosticInfo;
    length = 1;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // Then next diag info !
    }
    encodingByte = SOPC_DiagInfoEncoding_SymbolicId;
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Int32 value
    symbolicId = 1000;
    status = SOPC_Int32_Write(&symbolicId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    */
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read diag info
    status = SOPC_DiagnosticInfo_Read(&diag, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_DiagnosticInfo_Clear(&diag);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* LIMIT+1 OF NESTED DIAG INFO */

    // ENCODING A DIAG INFO WITH SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL+1 nested levels
    encodingByte = SOPC_DiagInfoEncoding_InnerDianosticInfo;
    length = 1;
    for (idx = 1; idx <= SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL + 1; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer, 0);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
        // Then next diag info !
    }
    encodingByte = SOPC_DiagInfoEncoding_SymbolicId;
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Int32 value
    symbolicId = 1000;
    status = SOPC_Int32_Write(&symbolicId, buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Read diag info
    status = SOPC_DiagnosticInfo_Read(&diag, buffer, 0);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    //
    SOPC_Buffer_Delete(buffer);
}
END_TEST

START_TEST(test_ua_string_type)
{
    // Non regression test on string copy
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool result = false;
    int32_t comp_res = -1;
    SOPC_String s1, s2;
    SOPC_String_Initialize(&s1);
    SOPC_String_Initialize(&s2);

    // Test a NULL string with length=-1 copy
    s1.Length = -1;
    status = SOPC_String_Copy(&s2, &s1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    result = SOPC_String_Equal(&s1, &s2);
    ck_assert(result != false);

    // Test a NULL string with length=0 copy
    s1.Length = 0;
    status = SOPC_String_Copy(&s2, &s1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    result = SOPC_String_Equal(&s1, &s2);
    ck_assert(result != false);

    // Test 2 possible forms of NULL strings equality
    s1.Length = -1;
    s2.Length = 0;
    result = SOPC_String_Equal(&s1, &s2);
    ck_assert(result != false);
    result = SOPC_String_Equal(&s2, &s1);
    ck_assert(result != false);

    // Test a non NULL string copy
    SOPC_String_AttachFromCstring(&s1, "Test of string copy !");
    result = SOPC_String_Equal(&s1, &s2);
    ck_assert(result == false);
    status = SOPC_String_Copy(&s2, &s1);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    result = SOPC_String_Equal(&s1, &s2);
    ck_assert(result != false);
    result = SOPC_String_Compare(&s1, &s2, true, &comp_res);
    ck_assert(comp_res == 0);
    result = SOPC_String_Compare(&s1, &s2, false, &comp_res);
    ck_assert(comp_res == 0);

    SOPC_String_Clear(&s1);
    SOPC_String_Clear(&s2);
}
END_TEST

START_TEST(test_ua_localized_text_type)
{
    // Non regression test on string copy
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool result = false;
    int32_t comp_res = -1;

    SOPC_LocalizedText singleLt, single2Lt, single3Lt;
    SOPC_LocalizedText setOfLt;
    SOPC_LocalizedText* arrayOfLt;
    int32_t nbElts = 0;

    SOPC_LocalizedText_Initialize(&singleLt);
    SOPC_LocalizedText_Initialize(&single2Lt);
    SOPC_LocalizedText_Initialize(&single3Lt);

    SOPC_LocalizedText_Initialize(&setOfLt);

    /* Test copy and comparison */

    /** Degraded **/

    status = SOPC_LocalizedText_Copy(&singleLt, NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_Copy(NULL, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    status = SOPC_LocalizedText_Compare(NULL, &singleLt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_Compare(&singleLt, NULL, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_Compare(&singleLt, &singleLt, NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    /** Nominal **/

    // Test a NULL LocalizedText with lengths=-1 copy
    singleLt.defaultLocale.Length = -1;
    singleLt.defaultText.Length = -1;
    status = SOPC_LocalizedText_Copy(&single2Lt, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // Test 2 possible forms of NULL LocalizedText equality
    singleLt.defaultLocale.Length = -1;
    singleLt.defaultText.Length = 0;
    single2Lt.defaultLocale.Length = 0;
    single2Lt.defaultText.Length = -1;
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);
    status = SOPC_LocalizedText_Compare(&single2Lt, &singleLt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // Test LocalizedText single value copy
    singleLt.defaultLocale.Length = -1;
    singleLt.defaultText.Length = 0;
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-US");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "safeguard");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_ne(0, comp_res);

    status = SOPC_LocalizedText_Copy(&single2Lt, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_Compare(&single2Lt, &singleLt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    /* Test set of localized text */

    /** Nominal **/

    // Test LocalizedText with a set of localized texts
    char* supportedLocales[] = {"en-US", "en-UK", "fr-FR", NULL};
    // Define a invariant default localized text
    status = SOPC_String_AttachFromCstring(&setOfLt.defaultText, "*");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Check invariant locale can still be modified
    SOPC_String_Clear(&singleLt.defaultLocale); // remove locale Id
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    result = SOPC_String_Equal(&setOfLt.defaultLocale, &singleLt.defaultLocale);
    ck_assert_int_eq(true, result);
    result = SOPC_String_Equal(&setOfLt.defaultText, &singleLt.defaultText);
    ck_assert_int_eq(true, result);

    // Check supported locales can be set
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-UK");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "shield");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-US");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "bumper");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "fr-FR");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "pare-choc");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Check supported locales can be overwritten
    SOPC_String_Clear(&singleLt.defaultLocale); // remove locale Id
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "fr-FR");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_String_Clear(&singleLt.defaultText); // remove text
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "pare-chocs");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Check unsupported locales cannot be set
    SOPC_String_Clear(&singleLt.defaultLocale); // remove locale Id
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "fr-BE");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_String_Clear(&singleLt.defaultText); // remove text
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "pare-chocs");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_NOT_SUPPORTED, status);

    // Check preferred locale can be retrieved
    char* preferredLocales[] = {"de-DE", "en-UK", "en-US", "fr-FR", NULL};

    // Check at same time resolution on a LocalizedText array instead of set:
    status = SOPC_LocalizedText_CopyToArray(&arrayOfLt, &nbElts, &setOfLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Check Set -> Array -> Set is equivalent to first Set
    SOPC_LocalizedText setOfLt2;
    SOPC_LocalizedText_Initialize(&setOfLt2);
    status = SOPC_LocalizedText_CopyFromArray(&setOfLt2, nbElts, arrayOfLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_Compare(&setOfLt, &setOfLt2, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);
    SOPC_LocalizedText_Clear(&setOfLt2);

    // en-UK shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales, &setOfLt);

    SOPC_LocalizedText_Clear(&single3Lt); // same with an LT array
    SOPC_LocalizedTextArray_GetPreferredLocale(&single3Lt, preferredLocales, nbElts, arrayOfLt);
    // expected:
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-UK");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "shield");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // compare:
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    status = SOPC_LocalizedText_Compare(&singleLt, &single3Lt, &comp_res); // same with an LT array
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    char* preferredLocales2[] = {"de-DE", "fr-FR", "en-UK", "en-US", NULL};

    // fr-FR shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales2, &setOfLt);

    SOPC_LocalizedText_Clear(&single3Lt); // same with an LT array
    SOPC_LocalizedTextArray_GetPreferredLocale(&single3Lt, preferredLocales2, nbElts, arrayOfLt);
    // expected:
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "fr-FR");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "pare-chocs");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // compare:
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    status = SOPC_LocalizedText_Compare(&singleLt, &single3Lt, &comp_res); // same with an LT array
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // End of LT array tests
    SOPC_LocalizedText_Clear(&single3Lt);
    SOPC_Clear_Array(&nbElts, (void**) &arrayOfLt, sizeof(SOPC_LocalizedText), SOPC_LocalizedText_ClearAux);

    // Delete the "en-UK" locale
    //   Create empty string for locale en-UK:
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-UK");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Keep only en-UK as english preferred locale and do not keep another exactly valid neither
    char* preferredLocales3[] = {"de-DE", "en-UK", "fr-BE", NULL};

    // en-US shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales3, &setOfLt);
    // expected:
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-US");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "bumper");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // compare:
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // Keep only 'en' as english preferred locale and do not keep another exactly valid neither
    char* preferredLocales4[] = {"de-DE", "en", "fr-BE", NULL};
    // en-US shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales4, &setOfLt);
    // expected: keep last 'en-US' localized text expected
    // compare:
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // Keep only en-UK as english preferred locale but also keep fr-FR exact and valid locale
    char* preferredLocales5[] = {"de-DE", "en-UK", "fr-FR", NULL};

    // fr-FR shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales5, &setOfLt);
    // expected:
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "fr-FR");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "pare-chocs");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // compare:
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // Keep only language compatible preferred locale but set fr-BE first
    char* preferredLocales6[] = {"de-DE", "fr-BE", "en-UK", NULL};

    // fr-FR shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales6, &setOfLt);
    // expected: keep fr-FR localized text
    // compare:
    status = SOPC_LocalizedText_Compare(&singleLt, &single2Lt, &comp_res);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(0, comp_res);

    // Keep only unsupported languages
    char* preferredLocales7[] = {"de-DE", "es-ES", "et-EE", NULL};

    // default localized text shall be preferred
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_GetPreferredLocale(&single2Lt, preferredLocales7, &setOfLt);
    // expected: default of setOfLt
    // compare:
    result = SOPC_String_Equal(&setOfLt.defaultLocale, &single2Lt.defaultLocale);
    ck_assert_int_eq(true, result);
    result = SOPC_String_Equal(&setOfLt.defaultText, &single2Lt.defaultText);
    ck_assert_int_eq(true, result);

    // Set the default locale to specialized locale: test specific case of deletion of the default one
    status = SOPC_String_AttachFromCstring(&setOfLt.defaultLocale, "en-UK");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Delete the "en-UK" locale
    //   Create empty string for locale en-UK:
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-UK");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    //   Set empty value
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Check a new default value is present
    // expected: en-US localized text
    SOPC_LocalizedText_Clear(&singleLt);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultLocale, "en-US");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_String_AttachFromCstring(&singleLt.defaultText, "bumper");
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // compare:
    result = SOPC_String_Equal(&setOfLt.defaultLocale, &singleLt.defaultLocale);
    ck_assert_int_eq(true, result);
    result = SOPC_String_Equal(&setOfLt.defaultText, &singleLt.defaultText);
    ck_assert_int_eq(true, result);

    // Delete the invariant locale => delete all localized texts
    //   Create empty string for invariant locale:
    SOPC_LocalizedText_Clear(&singleLt);
    //   Set empty value
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // All localized text shall be removed
    ck_assert_int_ge(0, setOfLt.defaultLocale.Length);
    ck_assert_int_ge(0, setOfLt.defaultText.Length);
    if (setOfLt.localizedTextList != NULL)
    {
        ck_assert_int_eq(0, SOPC_SLinkedList_GetLength(setOfLt.localizedTextList));
    }

    /** Degraded **/
    SOPC_LocalizedText_Clear(&singleLt);

    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, supportedLocales, NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_AddOrSetLocale(NULL, supportedLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_AddOrSetLocale(&setOfLt, NULL, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    status = SOPC_LocalizedText_GetPreferredLocale(&setOfLt, preferredLocales, NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_GetPreferredLocale(&setOfLt, NULL, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    status = SOPC_LocalizedText_GetPreferredLocale(NULL, preferredLocales, &singleLt);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    SOPC_LocalizedText_Clear(&singleLt);
    SOPC_LocalizedText_Clear(&single2Lt);
    SOPC_LocalizedText_Clear(&setOfLt);
}
END_TEST

START_TEST(test_ua_qname_parse)
{
    static const struct
    {
        const char* input;
        const char* name;
        uint16_t ns;
    } test_data[] = {{
                         "Hello",
                         "Hello",
                         0,
                     },
                     {
                         "42:Hello",
                         "Hello",
                         42,
                     },
                     {
                         ":Hello",
                         ":Hello",
                         0,
                     },
                     {
                         "Hello:World",
                         "Hello:World",
                         0,
                     },
                     {NULL, NULL, 0}};

    for (size_t i = 0; test_data[i].input != NULL; ++i)
    {
        SOPC_QualifiedName qname;
        SOPC_QualifiedName_Initialize(&qname);

        SOPC_ReturnStatus status = SOPC_QualifiedName_ParseCString(&qname, test_data[i].input);
        ck_assert_uint_eq(SOPC_STATUS_OK, status);
        ck_assert_str_eq(test_data[i].name, SOPC_String_GetRawCString(&qname.Name));
        ck_assert_uint_eq(test_data[i].ns, qname.NamespaceIndex);

        SOPC_QualifiedName_Clear(&qname);
    }
}
END_TEST

#define NULL_GUID                                                            \
    {                                                                        \
        0x00, 0x00, 0x00, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } \
    }

START_TEST(test_ua_guid_parse)
{
    SOPC_Helper_EndiannessCfg_Initialize();

    static const struct
    {
        const char* input;
        SOPC_Guid guid;
    } test_data[] = {{
                         "61ab45e4-0687-4944-b998-abf8166ac127",
                         {0x61ab45e4, 0x0687, 0x4944, {0xb9, 0x98, 0xab, 0xf8, 0x16, 0x6a, 0xc1, 0x27}},
                     },
                     {
                         "61AB45E4-0687-4944-B998-ABF8166AC127",
                         {0x61ab45e4, 0x0687, 0x4944, {0xb9, 0x98, 0xab, 0xf8, 0x16, 0x6a, 0xc1, 0x27}},
                     },
                     {
                         "This is not a UUID",
                         NULL_GUID,
                     },
                     {NULL, NULL_GUID}};

    for (size_t i = 0; test_data[i].input != NULL; ++i)
    {
        SOPC_Guid guid;
        SOPC_Guid_Initialize(&guid);

        SOPC_ReturnStatus status = SOPC_Guid_FromCString(&guid, test_data[i].input, strlen(test_data[i].input));

        if (test_data[i].guid.Data1 != 0)
        {
            ck_assert_uint_eq(SOPC_STATUS_OK, status);
            ck_assert_int_eq(0, memcmp(&guid, &test_data[i].guid, sizeof(SOPC_Guid)));
        }
        else
        {
            ck_assert_uint_ne(SOPC_STATUS_OK, status);
        }

        SOPC_Guid_Clear(&guid);
    }
}
END_TEST

START_TEST(test_ua_variant_get_range_scalar)
{
    SOPC_NumericRange *array_single = NULL, *array_range = NULL;
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("2", &array_single));
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("3:5", &array_range));

    // Except String and ByteString, we shouldn't be able to dereference scalar types
    for (SOPC_BuiltinId type_id = SOPC_Null_Id; type_id < SOPC_DiagnosticInfo_Id; ++type_id)
    {
        if (type_id == SOPC_String_Id || type_id == SOPC_ByteString_Id)
        {
            continue;
        }

        SOPC_Variant v;
        SOPC_Variant_Initialize(&v);
        v.ArrayType = SOPC_VariantArrayType_SingleValue;
        v.BuiltInTypeId = type_id;
        v.DoNotClear = false;

        SOPC_Variant deref;
        SOPC_Variant_Initialize(&deref);

        ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_Variant_GetRange(&deref, &v, array_range));

        SOPC_Variant_Clear(&deref);
        SOPC_Variant_Clear(&v);
    }

    SOPC_String short_string, long_string;
    SOPC_ByteString short_bstring, long_bstring;
    SOPC_Variant source, deref;

    SOPC_String_Initialize(&short_string);
    SOPC_String_Initialize(&long_string);
    SOPC_ByteString_Initialize(&short_bstring);
    SOPC_ByteString_Initialize(&long_bstring);
    SOPC_Variant_Initialize(&source);
    SOPC_Variant_Initialize(&deref);

    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&short_string, "a"));
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&long_string, "alpha"));
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_ByteString_CopyFromBytes(&short_bstring, (const SOPC_Byte*) "a", 1));
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_ByteString_CopyFromBytes(&long_bstring, (const SOPC_Byte*) "alpha", 5));

    source.ArrayType = SOPC_VariantArrayType_SingleValue;
    source.DoNotClear = true;
    source.BuiltInTypeId = SOPC_String_Id;

#define STR_COALESCE(str) ((str != NULL) ? str : "")
#define TEST_STR(source_str, range, expected)                                                               \
    source.Value.String = source_str;                                                                       \
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_Variant_GetRange(&deref, &source, range));                       \
    ck_assert_uint_eq(SOPC_VariantArrayType_SingleValue, deref.ArrayType);                                  \
    ck_assert_uint_eq(false, deref.DoNotClear);                                                             \
    ck_assert_uint_eq(SOPC_String_Id, deref.BuiltInTypeId);                                                 \
    ck_assert_str_eq(STR_COALESCE(expected), STR_COALESCE(SOPC_String_GetRawCString(&deref.Value.String))); \
    SOPC_Variant_Clear(&deref);                                                                             \
    SOPC_Variant_Initialize(&deref);

    TEST_STR(short_string, array_single, "");
    TEST_STR(long_string, array_single, "p");
    TEST_STR(short_string, array_range, "");
    TEST_STR(long_string, array_range, "ha");

#undef TEST_STR

    source.BuiltInTypeId = SOPC_ByteString_Id;

#define TEST_BSTR(source_bstr, range, expected)                                        \
    source.Value.Bstring = source_bstr;                                                \
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_Variant_GetRange(&deref, &source, range));  \
    ck_assert_uint_eq(SOPC_VariantArrayType_SingleValue, deref.ArrayType);             \
    ck_assert_uint_eq(false, deref.DoNotClear);                                        \
    ck_assert_uint_eq(SOPC_ByteString_Id, deref.BuiltInTypeId);                        \
    ck_assert_uint_eq(strlen(expected), (size_t) deref.Value.Bstring.Length);          \
    ck_assert_int_eq(0, memcmp(expected, deref.Value.Bstring.Data, strlen(expected))); \
    SOPC_Variant_Clear(&deref);                                                        \
    SOPC_Variant_Initialize(&deref);

    TEST_BSTR(short_bstring, array_single, "");
    TEST_BSTR(long_bstring, array_single, "p");
    TEST_BSTR(short_bstring, array_range, "");
    TEST_BSTR(long_bstring, array_range, "ha");

#undef TEST_BSTR

    SOPC_String_Clear(&short_string);
    SOPC_String_Clear(&long_string);
    SOPC_ByteString_Clear(&short_bstring);
    SOPC_ByteString_Clear(&long_bstring);
    SOPC_Variant_Clear(&deref);
    SOPC_Variant_Clear(&source);
    SOPC_NumericRange_Delete(array_range);
    SOPC_NumericRange_Delete(array_single);
}
END_TEST

START_TEST(test_ua_variant_get_range_array)
{
    SOPC_NumericRange *array_single = NULL, *array_range = NULL;
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("2", &array_single));
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("3:5", &array_range));

    SOPC_Variant source;
    SOPC_Variant_Initialize(&source);

    source.ArrayType = SOPC_VariantArrayType_Array;
    source.BuiltInTypeId = SOPC_UInt16_Id;
    source.DoNotClear = true;
    source.Value.Array.Length = 5;
    source.Value.Array.Content.Uint16Arr = (uint16_t[]){1, 2, 3, 4, 5};

    SOPC_Variant deref;
    SOPC_Variant_Initialize(&deref);
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_Variant_GetRange(&deref, &source, array_single));
    ck_assert_uint_eq(SOPC_VariantArrayType_Array, deref.ArrayType);
    ck_assert_uint_eq(source.BuiltInTypeId, deref.BuiltInTypeId);
    ck_assert_uint_eq(false, deref.DoNotClear);
    ck_assert_int_eq(1, deref.Value.Array.Length);
    ck_assert_uint_eq(3, deref.Value.Array.Content.Uint16Arr[0]);
    SOPC_Variant_Clear(&deref);

    SOPC_Variant_Initialize(&deref);
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_Variant_GetRange(&deref, &source, array_range));
    ck_assert_uint_eq(SOPC_VariantArrayType_Array, deref.ArrayType);
    ck_assert_uint_eq(source.BuiltInTypeId, deref.BuiltInTypeId);
    ck_assert_uint_eq(false, deref.DoNotClear);
    ck_assert_int_eq(2, deref.Value.Array.Length);
    ck_assert_uint_eq(4, deref.Value.Array.Content.Uint16Arr[0]);
    ck_assert_uint_eq(5, deref.Value.Array.Content.Uint16Arr[1]);
    SOPC_Variant_Clear(&deref);

    SOPC_Variant_Clear(&source);
    SOPC_NumericRange_Delete(array_range);
    SOPC_NumericRange_Delete(array_single);
}
END_TEST

static void copy_string_data(SOPC_Variant* variant, const char* data)
{
    if (strlen(data) == 0)
    {
        return;
    }

    if (variant->BuiltInTypeId == SOPC_String_Id)
    {
        ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_String_CopyFromCString(&variant->Value.String, data));
    }
    else if (variant->BuiltInTypeId == SOPC_ByteString_Id)
    {
        ck_assert_uint_eq(
            SOPC_STATUS_OK,
            SOPC_ByteString_CopyFromBytes(&variant->Value.Bstring, (const SOPC_Byte*) data, (int32_t) strlen(data)));
    }
    else
    {
        ck_assert(false);
    }
}

static void compare_string_data(const SOPC_Variant* variant, const char* data)
{
    if (variant->BuiltInTypeId == SOPC_String_Id)
    {
        ck_assert_str_eq(STR_COALESCE(data), STR_COALESCE(SOPC_String_GetRawCString(&variant->Value.String)));
    }
    else if (variant->BuiltInTypeId == SOPC_ByteString_Id)
    {
        ck_assert(variant->Value.Bstring.Length >= 0);
        ck_assert_uint_eq(strlen(data), (size_t) variant->Value.Bstring.Length);

        if (strlen(data) == 0)
        {
            ck_assert_ptr_null(variant->Value.Bstring.Data);
        }
        else
        {
            ck_assert_int_eq(0, memcmp(data, variant->Value.Bstring.Data, strlen(data)));
        }
    }
    else
    {
        ck_assert(false);
    }
}

static void test_ua_variant_set_range_scalar_string(SOPC_BuiltinId type_id,
                                                    const char* initial_str,
                                                    const char* patch_str,
                                                    const char* range_str,
                                                    const char* patched_str)
{
    SOPC_NumericRange* range = NULL;
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse(range_str, &range));

    SOPC_Variant variant, patch;

    SOPC_Variant_Initialize(&variant);
    variant.ArrayType = SOPC_VariantArrayType_SingleValue;
    variant.BuiltInTypeId = type_id;
    variant.DoNotClear = false;
    copy_string_data(&variant, initial_str);

    SOPC_Variant_Initialize(&patch);
    patch.ArrayType = SOPC_VariantArrayType_SingleValue;
    patch.BuiltInTypeId = type_id;
    patch.DoNotClear = false;
    copy_string_data(&patch, patch_str);

    SOPC_ReturnStatus status = SOPC_Variant_SetRange(&variant, &patch, range);
    SOPC_ReturnStatus expected_status = (patched_str != NULL) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    ck_assert_uint_eq(expected_status, status);
    ck_assert_uint_eq(SOPC_VariantArrayType_SingleValue, variant.ArrayType);
    ck_assert_uint_eq(type_id, variant.BuiltInTypeId);
    ck_assert(variant.DoNotClear == false);

    const char* expected_str = (patched_str != NULL) ? patched_str : initial_str;
    compare_string_data(&variant, expected_str);

    SOPC_NumericRange_Delete(range);
    SOPC_Variant_Clear(&patch);
    SOPC_Variant_Clear(&variant);
}

START_TEST(test_ua_variant_set_range_scalar)
{
    SOPC_NumericRange *array_single = NULL, *array_range = NULL;
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("2", &array_single));
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse("3:5", &array_range));

    // Except String and ByteString, we shouldn't be able to dereference scalar types
    for (SOPC_BuiltinId type_id = SOPC_Null_Id; type_id < SOPC_DiagnosticInfo_Id; ++type_id)
    {
        if (type_id == SOPC_String_Id || type_id == SOPC_ByteString_Id)
        {
            continue;
        }

        SOPC_Variant variant, src;
        SOPC_Variant_Initialize(&variant);
        variant.ArrayType = SOPC_VariantArrayType_SingleValue;
        variant.BuiltInTypeId = type_id;
        variant.DoNotClear = false;

        SOPC_Variant_Initialize(&src);
        variant.ArrayType = SOPC_VariantArrayType_SingleValue;
        variant.BuiltInTypeId = type_id;
        variant.DoNotClear = false;

        ck_assert_uint_eq(SOPC_STATUS_NOK, SOPC_Variant_SetRange(&variant, &src, array_range));

        SOPC_Variant_Clear(&src);
        SOPC_Variant_Clear(&variant);
    }

    SOPC_NumericRange_Delete(array_range);
    SOPC_NumericRange_Delete(array_single);

    static const SOPC_BuiltinId STR_TYPES[2] = {SOPC_String_Id, SOPC_ByteString_Id};

    for (size_t i = 0; i < sizeof(STR_TYPES) / sizeof(SOPC_BuiltinId); ++i)
    {
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "hello", "a", "1", "hallo");
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "hello", "xxx", "2:4", "hexxx");
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "hello", "xxxxx", "2:6", "hexxx");
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "hello", "xxx", "8:10", "hello");
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "hello", "xxx", "8:11", NULL);
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "hello", "", "4", NULL);
        test_ua_variant_set_range_scalar_string(STR_TYPES[i], "", "x", "1", "");
    }
}
END_TEST

static SOPC_Variant* create_string_array_variant(const char** strs, size_t n_strs)
{
    SOPC_Variant* variant = SOPC_Variant_Create();
    ck_assert_ptr_nonnull(variant);

    variant->ArrayType = SOPC_VariantArrayType_Array;
    variant->BuiltInTypeId = SOPC_String_Id;
    variant->DoNotClear = false;
    variant->Value.Array.Length = (int32_t) n_strs;

    if (n_strs == 0)
    {
        return variant;
    }

    variant->Value.Array.Content.StringArr = SOPC_Calloc(n_strs, sizeof(SOPC_String));
    ck_assert_ptr_nonnull(variant->Value.Array.Content.StringArr);

    for (size_t i = 0; i < n_strs; ++i)
    {
        ck_assert_uint_eq(SOPC_STATUS_OK,
                          SOPC_String_CopyFromCString(&variant->Value.Array.Content.StringArr[i], strs[i]));
    }

    return variant;
}

static void test_ua_variant_set_range_array_helper(const char** initial_strs,
                                                   size_t n_initial_strs,
                                                   const char** patch_strs,
                                                   size_t n_patch_strs,
                                                   const char* range_str,
                                                   const char** patched_strs,
                                                   size_t n_patched_strs)
{
    SOPC_NumericRange* range = NULL;
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_NumericRange_Parse(range_str, &range));

    SOPC_Variant* variant = create_string_array_variant(initial_strs, n_initial_strs);
    SOPC_Variant* patch = create_string_array_variant(patch_strs, n_patch_strs);
    SOPC_ReturnStatus status = SOPC_Variant_SetRange(variant, patch, range);

    SOPC_ReturnStatus expected_status =
        (initial_strs == NULL || patched_strs != NULL) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    ck_assert_uint_eq(expected_status, status);

    const char** expected_strs = (patched_strs != NULL) ? patched_strs : initial_strs;
    size_t n_expected_strs = (patched_strs != NULL) ? n_patched_strs : n_initial_strs;
    ck_assert(variant->Value.Array.Length >= 0);
    ck_assert_uint_eq(n_expected_strs, (size_t) variant->Value.Array.Length);

    ck_assert(NULL != expected_strs || (n_expected_strs <= 0 && NULL == expected_strs));
    for (size_t i = 0; i < n_expected_strs; ++i)
    {
        ck_assert_str_eq(STR_COALESCE(expected_strs[i]),
                         STR_COALESCE(SOPC_String_GetRawCString(&variant->Value.Array.Content.StringArr[i])));
    }

    SOPC_Variant_Delete(patch);
    SOPC_Variant_Delete(variant);
    SOPC_NumericRange_Delete(range);
}

START_TEST(test_ua_variant_set_range_array)
{
    test_ua_variant_set_range_array_helper((const char*[]){"hello", "world"}, 2, (const char*[]){"you"}, 1, "1",
                                           (const char*[]){"hello", "you"}, 2);
    test_ua_variant_set_range_array_helper((const char*[]){"this", "is", "a", "long", "sentence"}, 5,
                                           (const char*[]){"my", "first", "kiwi"}, 3, "2:4",
                                           (const char*[]){"this", "is", "my", "first", "kiwi"}, 5);
    test_ua_variant_set_range_array_helper((const char*[]){"this", "is", "a", "long", "sentence"}, 5,
                                           (const char*[]){"my", "first", "kiwi", "in", "Alaska"}, 5, "2:6",
                                           (const char*[]){"this", "is", "my", "first", "kiwi"}, 5);
    test_ua_variant_set_range_array_helper((const char*[]){"nothing", "more"}, 2, (const char*[]){"nothing", "less"}, 2,
                                           "2:3", (const char*[]){"nothing", "more"}, 2);
    test_ua_variant_set_range_array_helper((const char*[]){"nothing", "more"}, 2, (const char*[]){"nothing", "less"}, 2,
                                           "2:4", NULL, 0);
    test_ua_variant_set_range_array_helper((const char*[]){"nothing", "more"}, 2, NULL, 0, "1", NULL, 0);
    test_ua_variant_set_range_array_helper(NULL, 0, (const char*[]){"nothing", "less"}, 2, "2:3", NULL, 0);
}
END_TEST

Suite* tests_make_suite_tools(void)
{
    Suite* s;
    TCase *tc_hexlify, *tc_basetools, *tc_encoder, *tc_ua_types, *tc_buffer, *tc_linkedlist, *tc_async_queue;

    s = suite_create("Tools");
    tc_basetools = tcase_create("String tools");
    tcase_add_test(tc_basetools, test_strncmp_ignore_case);
    tcase_add_test(tc_basetools, test_strcmp_ignore_case);
    tcase_add_test(tc_basetools, test_strcmp_ignore_case_alt_end);
    tcase_add_test(tc_basetools, test_helper_uri);
    tcase_add_test(tc_basetools, test_strtouint);
    tcase_add_test(tc_basetools, test_strtouint2);
    tcase_add_test(tc_basetools, test_strtoint);
    tcase_add_test(tc_basetools, test_string_nodeid);
    tcase_add_test(tc_basetools, test_string_datetime_no_timezone);
    tcase_add_test(tc_basetools, test_string_datetime_with_timezone);
    suite_add_tcase(s, tc_basetools);
    tc_buffer = tcase_create("Buffer");
    tcase_add_test(tc_buffer, test_buffer_create);
    tcase_add_test(tc_buffer, test_buffer_read_write);
    tcase_add_test(tc_buffer, test_buffer_copy);
    tcase_add_test(tc_buffer, test_buffer_reset);
    tcase_add_test(tc_buffer, test_buffer_set_properties);
    tcase_add_test(tc_buffer, test_buffer_append);
    tcase_add_test(tc_buffer, test_buffer_resizable);
    suite_add_tcase(s, tc_buffer);
    tc_linkedlist = tcase_create("Linked List");
    tcase_add_test(tc_linkedlist, test_linked_list);
    tcase_add_test(tc_linkedlist, test_sorted_linked_list);
    suite_add_tcase(s, tc_linkedlist);

    tc_hexlify = tcase_create("Hexlify (tests only)");
    tcase_add_test(tc_hexlify, test_hexlify);
    suite_add_tcase(s, tc_hexlify);

    tc_async_queue = tcase_create("Async queue");
    tcase_add_test(tc_async_queue, test_async_queue);
    tcase_add_test(tc_async_queue, test_async_queue_threads);
    suite_add_tcase(s, tc_async_queue);

    tc_encoder = tcase_create("UA Encoder");
    tcase_add_test(tc_encoder, test_ua_encoder_endianness_mgt);
    tcase_add_test(tc_encoder, test_ua_encoder_basic_types);
    tcase_add_test(tc_encoder, test_ua_encoder_other_types);
    tcase_add_test(tc_encoder, test_ua_decoder_allocation_limit);
    suite_add_tcase(s, tc_encoder);

    tc_ua_types = tcase_create("UA Types");
    tcase_add_test(tc_ua_types, test_ua_string_type);
    tcase_add_test(tc_ua_types, test_ua_localized_text_type);
    tcase_add_test(tc_ua_types, test_ua_qname_parse);
    tcase_add_test(tc_ua_types, test_ua_guid_parse);
    tcase_add_test(tc_ua_types, test_ua_variant_get_range_scalar);
    tcase_add_test(tc_ua_types, test_ua_variant_get_range_array);
    tcase_add_test(tc_ua_types, test_ua_variant_set_range_scalar);
    tcase_add_test(tc_ua_types, test_ua_variant_set_range_array);
    suite_add_tcase(s, tc_ua_types);

    return s;
}
