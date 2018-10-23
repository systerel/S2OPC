/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <stdlib.h>

#include "check_helpers.h"
#include "hexlify.h"

#include "opcua_statuscodes.h"

#include "sopc_async_queue.h"
#include "sopc_buffer.h"
#include "sopc_builtintypes.h"
#include "sopc_encoder.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_namespace_table.h"
#include "sopc_singly_linked_list.h"
#include "sopc_threads.h"
#include "sopc_time.h"

#define NODEID_NS 58942
#define NODEID_NSS "58942"
#define NODEID_I 126042
#define NODEID_IS "126042"
#define NODEID_S "ingopcs.foo/bar"
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
    SOPC_ReturnStatus status = 0;

    // Test creation
    //// Test nominal case
    SOPC_Buffer* buf = SOPC_Buffer_Create(10);
    ck_assert(buf != NULL);
    ck_assert(buf->data != NULL);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 0);
    ck_assert(buf->max_size == 10);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test buffer creation degraded cases
    buf = SOPC_Buffer_Create(0);
    ck_assert(buf == NULL);

    // Test initialization
    //// Test nominal case
    buf = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    status = SOPC_Buffer_Init(buf, 100);
    ck_assert(status == 0);
    ck_assert(buf->data != NULL);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 0);
    ck_assert(buf->max_size == 100);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded case: NULL buffer
    buf = NULL;
    status = SOPC_Buffer_Init(buf, 100);
    ck_assert(status != 0);
    ck_assert(buf == NULL);

    //// Test degraded case: invalid size
    buf = NULL;
    status = SOPC_Buffer_Init(buf, 0);
    ck_assert(status != 0);
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
    ck_assert(status == 0);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    ck_assert(buf->length == 3);
    ck_assert(buf->position == 3);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    status = SOPC_Buffer_Write(buf, data, 1);
    ck_assert(status == 0);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x00);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status != SOPC_STATUS_OK);
    free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status != SOPC_STATUS_OK);

    buf = SOPC_Buffer_Create(1);
    ////// NULL data pointer
    status = SOPC_Buffer_Write(buf, NULL, 3);
    ck_assert(status != SOPC_STATUS_OK);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
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
    // Reset position for reading content written
    status = SOPC_Buffer_SetPosition(buf, 0);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 4);
    status = SOPC_Buffer_Read(readData, buf, 2);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buf->position == 2);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x00);
    ck_assert(readData[3] == 0x00);
    status = SOPC_Buffer_Read(&(readData[2]), buf, 2);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buf->position == 4);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x02);
    ck_assert(readData[3] == 0x03);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Read(readData, buf, 3);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(status != SOPC_STATUS_OK);
    free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = SOPC_Buffer_Read(readData, buf, 3);
    ck_assert(status != SOPC_STATUS_OK);

    buf = SOPC_Buffer_Create(4);
    ////// NULL data pointer
    status = SOPC_Buffer_Read(NULL, buf, 3);
    ck_assert(status != SOPC_STATUS_OK);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);

    ////// Empty buffer
    readData[0] = 0x00;
    readData[1] = 0x00;
    readData[2] = 0x00;
    readData[3] = 0x00;
    status = SOPC_Buffer_Write(buf, data, 4);
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
    status = SOPC_Buffer_Copy(buf2, buf);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
    ck_assert(NULL != buf2);
#endif
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buf2->length == 4);
    ck_assert(buf2->position == 4);
    ck_assert(buf2->data[0] == 0x00);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    ck_assert(buf2->data[3] == 0x03);
    ck_assert(buf->max_size == 10);
    ck_assert(buf2->max_size == 5);

    buf->data[0] = 0x01;
    buf->data[3] = 0x0F;
    status = SOPC_Buffer_SetPosition(buf, 1);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 3);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_Buffer_Write(buf, data, 4);
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
    buf = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != SOPC_STATUS_OK);
    free(buf);
    buf = NULL;

    buf = SOPC_Buffer_Create(1);
    SOPC_Buffer_Delete(buf2);
    buf2 = NULL;

    /////// Non allocated buffer data
    buf2 = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
    ck_assert(NULL != buf2);
#endif
    memset(buf2, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != SOPC_STATUS_OK);
    free(buf2);
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
    SOPC_Buffer_Reset(buf);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 0 <=> Reset
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_ResetAfterPosition(buf, 0);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 2 in length of 4 buffer
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 4 in buffer with length 4
    SOPC_Buffer_Reset(buf);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_ResetAfterPosition(buf, 4);
    ck_assert(status == SOPC_STATUS_OK);
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
    buf = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status != SOPC_STATUS_OK);
    free(buf);
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
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_SetPosition(buf, 2);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    status = SOPC_Buffer_SetDataLength(buf, 2);
    ck_assert(status == SOPC_STATUS_OK);
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
    buf = malloc(sizeof(SOPC_Buffer));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buf);
#endif
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Buffer_SetDataLength(buf, 0);
    ck_assert(status != SOPC_STATUS_OK);
    free(buf);
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
    // Insert NULL value should result in returning a NULL value and do not add it
    value = SOPC_SLinkedList_Prepend(list, 1, NULL);
    ck_assert(NULL == value);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

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
    value = SOPC_SLinkedList_NextWithId(&it, &id);
    ck_assert(value == &value2);
    ck_assert(id == 2);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_NextWithId(&it, &id);
    ck_assert(value == &value3);
    ck_assert(id == 3);
    value = SOPC_SLinkedList_NextWithId(&it, &id);
    ck_assert(value == NULL);

    //// Test degraded case: add in full linked list
    value = SOPC_SLinkedList_Prepend(list, 4, &value3);
    ck_assert(value == NULL);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    //// Test degraded case: add to NULL pointer linked list
    value = SOPC_SLinkedList_Prepend(NULL, 4, &value3);
    ck_assert(value == NULL);
    ck_assert(3 == SOPC_SLinkedList_GetLength(list));

    // Test pop nominal case
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value2);
    ck_assert(2 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value1);
    ck_assert(1 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value3);
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
    //// Verify degraded find behavior
    value = SOPC_SLinkedList_FindFromId(NULL, 2);
    ck_assert(value == NULL);
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

    value = SOPC_SLinkedList_RemoveFromId(list, 2);
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

    //// Verify degraded remove behavior
    value = SOPC_SLinkedList_RemoveFromId(NULL, 2);
    ck_assert(value == NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    value = SOPC_SLinkedList_RemoveFromId(NULL, 1);
    ck_assert(value == NULL);
    ck_assert(0 == SOPC_SLinkedList_GetLength(list));

    //// Check apply to free elements
    void* p = NULL;
    SOPC_SLinkedList_Clear(list);
    p = malloc(sizeof(int));
    ck_assert(NULL != p);
    *(int*) p = 2;
    ck_assert(SOPC_SLinkedList_Prepend(list, 0, p) != NULL);
    p = malloc(sizeof(double));
    ck_assert(NULL != p);
    *(double*) p = 2.;
    ck_assert(SOPC_SLinkedList_Prepend(list, 1, p) != NULL);
    p = malloc(sizeof(char) * 5);
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

START_TEST(test_base_tools)
{
    char test1[] = "te";
    char test2[] = "Te";
    char test3[] = "tE";
    char test4[] = "TE";
    char ntest1[] = "de";
    char ntest2[] = "De";
    char ntest3[] = "ta";
    char ntest4[] = "tA";

    // Test nominal case (equality result)
    //// Each possible size class
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test2, 0));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test2, 1));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test2, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test2, 3));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test2, 4));
    //// Each possible combination of case
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test1, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test2, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test3, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, test4, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test2, test1, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test2, test2, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test2, test3, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test2, test4, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test3, test1, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test3, test2, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test3, test3, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test3, test4, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test4, test1, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test4, test2, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test4, test3, 2));
    ck_assert(0 == SOPC_strncmp_ignore_case(test4, test4, 2));

    // Test nominal case (non equality result)
    //// Each possible size class
    ck_assert(0 == SOPC_strncmp_ignore_case(test1, ntest1, 0));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest1, 1));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test1, 1));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest1, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test1, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest1, 3));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test1, 3));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest1, 4));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test1, 4));

    //// Each possible combination of case
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest1, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest2, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest3, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test1, ntest4, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test2, ntest1, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test2, ntest2, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test2, ntest3, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test2, ntest4, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test3, ntest1, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test3, ntest2, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test3, ntest3, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test3, ntest4, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test4, ntest1, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test4, ntest2, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test4, ntest3, 2));
    ck_assert(+1 == SOPC_strncmp_ignore_case(test4, ntest4, 2));

    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test1, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test2, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test3, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest1, test4, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest2, test1, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest2, test2, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest2, test3, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest2, test4, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest3, test1, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest3, test2, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest3, test3, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest3, test4, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest4, test1, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest4, test2, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest4, test3, 2));
    ck_assert(-1 == SOPC_strncmp_ignore_case(ntest4, test4, 2));
}
END_TEST

START_TEST(test_helper_uri)
{
    size_t hostLength = 0;
    size_t portIdx = 0;
    size_t portLength = 0;

    // Not a TCP UA address
    ck_assert(false == SOPC_Helper_URI_ParseTcpUaUri("http://localhost:9999/test", &hostLength, &portIdx, &portLength));

    ck_assert(false !=
              SOPC_Helper_URI_ParseTcpUaUri("opc.tcp://localhost:9999/test", &hostLength, &portIdx, &portLength));
    ck_assert(9 == hostLength);
    ck_assert(10 + hostLength + 1 == portIdx);
    ck_assert(4 == portLength);

    ck_assert(false != SOPC_Helper_URI_ParseTcpUaUri("opc.tcp://localhost:9999/test/moreThingsInPath/plus", &hostLength,
                                                     &portIdx, &portLength));
    ck_assert(9 == hostLength);
    ck_assert(10 + hostLength + 1 == portIdx);
    ck_assert(4 == portLength);

    // No port defined
    ck_assert(false == SOPC_Helper_URI_ParseTcpUaUri("opc.tcp://localhost:/test", &hostLength, &portIdx, &portLength));
    ck_assert(false == SOPC_Helper_URI_ParseTcpUaUri("opc.tcp://localhost/test", &hostLength, &portIdx, &portLength));

    ck_assert(false !=
              SOPC_Helper_URI_ParseTcpUaUri("opc.tcp://192.168.4.1:9/test", &hostLength, &portIdx, &portLength));
    ck_assert(11 == hostLength);
    ck_assert(10 + hostLength + 1 == portIdx);
    ck_assert(1 == portLength);

    ck_assert(false != SOPC_Helper_URI_ParseTcpUaUri("opc.tcp://[fe80::250:b6ff:fe17:9f3a]:99999/test", &hostLength,
                                                     &portIdx, &portLength));
    ck_assert(26 == hostLength);
    ck_assert(10 + hostLength + 1 == portIdx);
    ck_assert(5 == portLength);
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
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint8_t("42", &a, 1, '\0'));
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
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint16_t("42", &b, 1, '\0'));
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
    ck_assert(SOPC_STATUS_OK != SOPC_strtouint32_t("42", &c, 1, '\0'));
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

    pNid = SOPC_NodeId_FromCString("i=" NODEID_IS, strlen("i=" NODEID_IS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(strncmp(sNid, "i=" NODEID_IS, strlen("i=" NODEID_IS)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    nid.Namespace = NODEID_NS;
    pNid = SOPC_NodeId_FromCString("ns=" NODEID_NSS ";i=" NODEID_IS, strlen("ns=" NODEID_NSS ";i=" NODEID_IS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(strncmp(sNid, "ns=" NODEID_NSS ";i=" NODEID_IS, strlen("ns=" NODEID_NSS ";i=" NODEID_IS)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    /* String nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_String;
    nid.Namespace = 0;
    ck_assert(SOPC_STATUS_OK == SOPC_String_CopyFromCString(&nid.Data.String, NODEID_S));

    pNid = SOPC_NodeId_FromCString("s=" NODEID_S, strlen("s=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(strncmp(sNid, "s=" NODEID_S, strlen("s=" NODEID_S)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    nid.Namespace = NODEID_NS;
    pNid = SOPC_NodeId_FromCString("ns=" NODEID_NSS ";s=" NODEID_S, strlen("ns=" NODEID_NSS ";s=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(strncmp(sNid, "ns=" NODEID_NSS ";s=" NODEID_S, strlen("ns=" NODEID_NSS ";s=" NODEID_S)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    /* GUID nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_Guid;
    nid.Namespace = 0;
    nid.Data.Guid = &guid;

    pNid = SOPC_NodeId_FromCString("g=" NODEID_GS, strlen("g=" NODEID_GS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(SOPC_strncmp_ignore_case(sNid, "g=" NODEID_GS, strlen("g=" NODEID_GS)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    nid.Namespace = NODEID_NS;
    pNid = SOPC_NodeId_FromCString("ns=" NODEID_NSS ";g=" NODEID_GS, strlen("ns=" NODEID_NSS ";g=" NODEID_GS));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(SOPC_strncmp_ignore_case(sNid, "ns=" NODEID_NSS ";g=" NODEID_GS,
                                       strlen("ns=" NODEID_NSS ";g=" NODEID_GS)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    /* ByteString nid */
    SOPC_NodeId_Initialize(&nid);
    nid.IdentifierType = SOPC_IdentifierType_ByteString;
    nid.Namespace = 0;
    ck_assert(SOPC_STATUS_OK ==
              SOPC_ByteString_CopyFromBytes(&nid.Data.Bstring, (SOPC_Byte*) NODEID_S, strlen(NODEID_S)));

    pNid = SOPC_NodeId_FromCString("b=" NODEID_S, strlen("b=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(strncmp(sNid, "b=" NODEID_S, strlen("b=" NODEID_S)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);

    nid.Namespace = NODEID_NS;
    pNid = SOPC_NodeId_FromCString("ns=" NODEID_NSS ";b=" NODEID_S, strlen("ns=" NODEID_NSS ";b=" NODEID_S));
    ck_assert(SOPC_STATUS_OK == SOPC_NodeId_Compare(pNid, &nid, &cmp));
    ck_assert(0 == cmp);
    sNid = SOPC_NodeId_ToCString(pNid);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != sNid);
#endif
    ck_assert(strncmp(sNid, "ns=" NODEID_NSS ";b=" NODEID_S, strlen("ns=" NODEID_NSS ";b=" NODEID_S)) == 0);
    free(sNid);
    SOPC_NodeId_Clear(pNid);
}
END_TEST

// Async queue

START_TEST(test_async_queue)
{
    void* arg = NULL;
    SOPC_AsyncQueue* queue = NULL;
    int paramAndRes = -10;
    int paramAndRes2 = 0;
    SOPC_ReturnStatus status = SOPC_AsyncQueue_Init(&queue, NULL);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &paramAndRes);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &paramAndRes2);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingDequeue(queue, &arg);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(arg == &paramAndRes);
    status = SOPC_AsyncQueue_NonBlockingDequeue(queue, &arg);
    ck_assert(status == SOPC_STATUS_OK);
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
    bool blockingDequeue;
    bool success;
} AsyncQueue_Element;

void* test_async_queue_blocking_dequeue_fct(void* args)
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
        param->blockingDequeue = true;
        status = SOPC_AsyncQueue_BlockingDequeue(param->queue, &arg);
        param->blockingDequeue = false;
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
    param->success = success;
    return NULL;
}

void* test_async_queue_nonblocking_dequeue_fct(void* args)
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
    params.success = false;
    params.blockingDequeue = false;
    params.nbMsgs = 5;
    params.queue = queue;
    // Nominal behavior of async queue FIFO (blocking dequeue)
    status = SOPC_Thread_Create(&thread, test_async_queue_blocking_dequeue_fct, &params);
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(params.blockingDequeue == !false);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &one);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &two);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &three);
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(100);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &four);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &five);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(params.success == !false);

    // Nominal behavior of async queue FIFO (non blocking dequeue)
    params.success = false;
    params.blockingDequeue = false;
    params.nbMsgs = 5;
    params.queue = queue;
    status = SOPC_Thread_Create(&thread, test_async_queue_nonblocking_dequeue_fct, &params);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &one);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &two);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &three);
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(100);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &four);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &five);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(params.success == !false);

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

#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buffer);
    ck_assert(NULL != bufferFull);
#endif
    // Test Byte nominal and degraded cases
    //// Nominal write
    SOPC_Byte byte = 0xAE;
    status = SOPC_Byte_Write(&byte, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[0] == 0xAE);
    //// Degraded write
    status = SOPC_Byte_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Byte_Write(&byte, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 8; // Set buffer full
    status = SOPC_Byte_Write(&byte, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    byte = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Byte_Read(&byte, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(byte == 0xAE);
    //// Degraded read
    status = SOPC_Byte_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Byte_Read(&byte, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Byte_Read(&byte, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Boolean nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    SOPC_Boolean boolv = false;
    status = SOPC_Boolean_Write(&boolv, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[0] == false);
    boolv = 1; // not FALSE
    status = SOPC_Boolean_Write(&boolv, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[1] == 1);
    boolv = 2; // not FALSE
    status = SOPC_Boolean_Write(&boolv, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[2] == 1); // True value always encoded as 1

    //// Degraded write
    status = SOPC_Boolean_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Write(&boolv, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Write(&boolv, bufferFull); // Test with full buffer
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    boolv = 4;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    buffer->data[2] = 2;                // Simulates a true value received as 2
    status = SOPC_Boolean_Read(&boolv, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(boolv == false);
    status = SOPC_Boolean_Read(&boolv, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(boolv == 1);
    status = SOPC_Boolean_Read(&boolv, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(boolv == 1); // True value always decoded as 1
    //// Degraded read
    status = SOPC_Boolean_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Read(&boolv, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Boolean_Read(&boolv, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test SByteuv16nal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    SOPC_SByte sbyte = -1;
    status = SOPC_SByte_Write(&sbyte, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[0] == 0xFF);
    //// Degraded write
    status = SOPC_SByte_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Write(&sbyte, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Write(&sbyte, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    sbyte = 0x00;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_SByte_Read(&sbyte, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(sbyte == -1);
    //// Degraded read
    status = SOPC_SByte_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Read(&sbyte, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_SByte_Read(&sbyte, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Int16 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    int16_t v16 = -2;

    status = SOPC_Int16_Write(&v16, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF);
    //// Degraded write
    status = SOPC_Int16_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int16_Write(&v16, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 7; // Set buffer almost full (1 byte left)
    status = SOPC_Int16_Write(&v16, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    v16 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Int16_Read(&v16, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(v16 == -2);
    //// Degraded read
    status = SOPC_Int16_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int16_Read(&v16, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int16_Read(&v16, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test UInt16 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    uint16_t vu16 = 2;

    status = SOPC_UInt16_Write(&vu16, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x02 && buffer->data[1] == 0x00);
    //// Degraded write
    status = SOPC_UInt16_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt16_Write(&vu16, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 7; // Set buffer almost full (1 byte left)
    status = SOPC_UInt16_Write(&vu16, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vu16 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_UInt16_Read(&vu16, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(vu16 == 2);
    //// Degraded read
    status = SOPC_UInt16_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt16_Read(&vu16, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt16_Read(&vu16, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Int32 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    int32_t v32 = -2;

    status = SOPC_Int32_Write(&v32, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF && buffer->data[2] == 0xFF && buffer->data[3] == 0xFF);
    //// Degraded write
    status = SOPC_Int32_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int32_Write(&v32, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_Int32_Write(&v32, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    v32 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Int32_Read(&v32, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(v32 == -2);
    //// Degraded read
    status = SOPC_Int32_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int32_Read(&v32, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int32_Read(&v32, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test UInt32 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    uint32_t vu32 = 1048578;

    status = SOPC_UInt32_Write(&vu32, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x02 && buffer->data[1] == 0x00 && buffer->data[2] == 0x10 && buffer->data[3] == 0x00);
    //// Degraded write
    status = SOPC_UInt32_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt32_Write(&vu32, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_UInt32_Write(&vu32, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vu32 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_UInt32_Read(&vu32, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(vu32 == 1048578);
    //// Degraded read
    status = SOPC_UInt32_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt32_Read(&vu32, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt32_Read(&vu32, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Int64 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    int64_t v64 = -2;

    status = SOPC_Int64_Write(&v64, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF && buffer->data[2] == 0xFF &&
              buffer->data[3] == 0xFF && buffer->data[4] == 0xFF && buffer->data[5] == 0xFF &&
              buffer->data[6] == 0xFF && buffer->data[7] == 0xFF);
    //// Degraded write
    status = SOPC_Int64_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int64_Write(&v64, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_Int64_Write(&v64, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    v64 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Int64_Read(&v64, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(v64 == -2);
    //// Degraded read
    status = SOPC_Int64_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int64_Read(&v64, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Int64_Read(&v64, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test UInt64 nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    uint64_t vu64 = 0x100000000000002;

    status = SOPC_UInt64_Write(&vu64, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x02 && buffer->data[1] == 0x00 && buffer->data[2] == 0x00 &&
              buffer->data[3] == 0x00 && buffer->data[4] == 0x00 && buffer->data[5] == 0x00 &&
              buffer->data[6] == 0x00 && buffer->data[7] == 0x01);
    //// Degraded write
    status = SOPC_UInt64_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt64_Write(&vu64, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_UInt64_Write(&vu64, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vu64 = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_UInt64_Read(&vu64, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(vu64 == 0x100000000000002);
    //// Degraded read
    status = SOPC_UInt64_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt64_Read(&vu64, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_UInt64_Read(&vu64, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Float nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    float vfloat = -6.5;

    status = SOPC_Float_Write(&vfloat, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x00 && buffer->data[1] == 0x00 && buffer->data[2] == 0xD0 && buffer->data[3] == 0xC0);
    //// Degraded write
    status = SOPC_Float_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Float_Write(&vfloat, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_Float_Write(&vfloat, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vfloat = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Float_Read(&vfloat, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(vfloat == -6.5);
    //// Degraded read
    status = SOPC_Float_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Float_Read(&vfloat, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Float_Read(&vfloat, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test Double nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    double vdouble = -6.5;

    status = SOPC_Double_Write(&vdouble, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0x00 && buffer->data[1] == 0x00 && buffer->data[2] == 0x00 &&
              buffer->data[3] == 0x00 && buffer->data[4] == 0x00 && buffer->data[5] == 0x00 &&
              buffer->data[6] == 0x1A && buffer->data[7] == 0xC0);
    //// Degraded write
    status = SOPC_Double_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Double_Write(&vdouble, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_Double_Write(&vdouble, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    vdouble = 0;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Double_Read(&vdouble, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(vdouble == -6.5);
    //// Degraded read
    status = SOPC_Double_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Double_Read(&vdouble, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Double_Read(&vdouble, buffer); // Nothing to read anymore
    ck_assert(status != SOPC_STATUS_OK);

    // Test DateTime nominal and degraded cases
    SOPC_Buffer_Reset(buffer);
    //// Nominal write
    SOPC_DateTime vDate;
    SOPC_DateTime_Initialize(&vDate);
    vDate = -2;

    status = SOPC_DateTime_Write(&vDate, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// Little endian encoded
    ck_assert(buffer->data[0] == 0xFE && buffer->data[1] == 0xFF && buffer->data[2] == 0xFF &&
              buffer->data[3] == 0xFF && buffer->data[4] == 0xFF && buffer->data[5] == 0xFF &&
              buffer->data[6] == 0xFF && buffer->data[7] == 0xFF);
    //// Degraded write
    status = SOPC_DateTime_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_DateTime_Write(&vDate, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = bufferFull->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_DateTime_Write(&vDate, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_DateTime_Clear(&vDate);
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_DateTime_Read(&vDate, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(vDate == -2);
    //// Degraded read
    status = SOPC_DateTime_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_DateTime_Read(&vDate, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_DateTime_Read(&vDate, buffer); // Nothing to read anymore
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

#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != buffer);
    ck_assert(NULL != bufferFull);
    ck_assert(NULL != bs);
    ck_assert(NULL != bs2);
#endif
    ////// Empty string
    bs->Length = 0;
    status = SOPC_ByteString_Write(bs, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    bs->Length = -1;
    status = SOPC_ByteString_Write(bs, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    bs->Length = -10;
    status = SOPC_ByteString_Write(bs, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    /////// Non empty bytestring
    SOPC_Buffer_Reset(buffer);
    status = SOPC_ByteString_CopyFromBytes(bs, boyString, 3);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_ByteString_Write(bs, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[0] == 0x03);
    ck_assert(buffer->data[1] == 0x00);
    ck_assert(buffer->data[2] == 0x00);
    ck_assert(buffer->data[3] == 0x00);
    ck_assert(buffer->data[4] == 0x42);
    ck_assert(buffer->data[5] == 0x6F);
    ck_assert(buffer->data[6] == 0x79);

    //// Degraded write
    status = SOPC_ByteString_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_ByteString_Write(bs, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_ByteString_Write(bs, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != bs2->Data);
#endif
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_ByteString_Read(bs2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0xFF;
    buffer->data[2] = 0xFF;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_ByteString_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_ByteString_Read(bs, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_ByteString_Read(bs, buffer); // Nothing to read anymore
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
    status = SOPC_String_Write(&str, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    str.Length = -1;
    status = SOPC_String_Write(&str, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    str.Length = -10;
    status = SOPC_String_Write(&str, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    /////// Non empty bytestring
    SOPC_Buffer_Reset(buffer);
    status = SOPC_String_AttachFromCstring(&str, "Boy");
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_String_Write(&str, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[0] == 0x03);
    ck_assert(buffer->data[1] == 0x00);
    ck_assert(buffer->data[2] == 0x00);
    ck_assert(buffer->data[3] == 0x00);
    ck_assert(buffer->data[4] == 0x42);
    ck_assert(buffer->data[5] == 0x6F);
    ck_assert(buffer->data[6] == 0x79);

    //// Degraded write
    status = SOPC_String_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_String_Write(&str, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_String_Write(&str, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != str2.Data);
#endif
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_String_Read(&str2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0xFF;
    buffer->data[2] = 0xFF;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_String_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_String_Read(&str, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_String_Read(&str, buffer); // Nothing to read anymore
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
    status = SOPC_XmlElement_Write(&xmlElt, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    xmlElt.Length = -1;
    status = SOPC_XmlElement_Write(&xmlElt, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    SOPC_Buffer_Reset(buffer);
    xmlElt.Length = -10;
    status = SOPC_XmlElement_Write(&xmlElt, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(buffer->data[0] == 0xFF);
    ck_assert(buffer->data[1] == 0xFF);
    ck_assert(buffer->data[2] == 0xFF);
    ck_assert(buffer->data[3] == 0xFF);

    /////// Non empty bytestring
    SOPC_Buffer_Reset(buffer);
    xmlElt.Data = malloc(sizeof(SOPC_Byte) * 3);
    ck_assert(xmlElt.Data != NULL);
    ck_assert(memcpy(xmlElt.Data, balA, 3) == xmlElt.Data);
    xmlElt.Length = 3;
    status = SOPC_XmlElement_Write(&xmlElt, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(buffer->data[0] == 0x03);
    ck_assert(buffer->data[1] == 0x00);
    ck_assert(buffer->data[2] == 0x00);
    ck_assert(buffer->data[3] == 0x00);
    ck_assert(buffer->data[4] == 0x3C);
    ck_assert(buffer->data[5] == 0x41);
    ck_assert(buffer->data[6] == 0x3E);

    //// Degraded write
    status = SOPC_XmlElement_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_XmlElement_Write(&xmlElt, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_XmlElement_Write(&xmlElt, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != xmlElt2.Data);
#endif
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
    status = SOPC_XmlElement_Read(&xmlElt2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0x00;
    buffer->data[2] = 0x00;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    buffer->data[0] = 0xFF;
    buffer->data[1] = 0xFF;
    buffer->data[2] = 0xFF;
    buffer->data[3] = 0xFF;
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_XmlElement_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_XmlElement_Read(&xmlElt, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_XmlElement_Read(&xmlElt, buffer); // Nothing to read anymore
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

    status = SOPC_Guid_Write(&guid, buffer);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_Guid_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Guid_Write(&guid, NULL);
    ck_assert(status != SOPC_STATUS_OK);
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != bufferFull);
#endif
    bufferFull->position = 17; // Set buffer almost full (15 byte left)
    status = SOPC_Guid_Write(&guid, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_Guid_Read(&guid2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(memcmp(&guid, &guid2, sizeof(SOPC_Guid)) == 0);

    //// Degraded read
    status = SOPC_Guid_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Guid_Read(&guid, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_Guid_Read(&guid, buffer); // Nothing to read anymore
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
    status = SOPC_NodeId_Write(&nodeId, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    // Encoding byte
    ck_assert(buffer->data[0] == 0x00);
    // Id byte
    ck_assert(buffer->data[1] == 0xFF);

    // Four bytes node id (limit of two bytes representation on numeric value)
    nodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    nodeId.Namespace = OPCUA_NAMESPACE_INDEX;
    nodeId.Data.Numeric = 256;
    status = SOPC_NodeId_Write(&nodeId, buffer);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_NodeId_Write(&nodeId, buffer);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_NodeId_Write(&nodeId, buffer);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_NodeId_Write(&nodeId, buffer);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_NodeId_Write(&nodeId, buffer);
    ck_assert(status == SOPC_STATUS_OK);
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
    status = SOPC_NodeId_Write(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_NodeId_Write(&nodeId, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    bufferFull->position = 26; // Set buffer almost full (6 byte left)
    status = SOPC_NodeId_Write(&nodeId, bufferFull);
    ck_assert(status != SOPC_STATUS_OK);

    //// Nominal read
    ////// Two bytes NodeId
    SOPC_Buffer_SetPosition(buffer, 0); // Reset position for reading
    status = SOPC_NodeId_Read(&nodeId2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(nodeId2.Namespace == 0);
    ck_assert(nodeId2.Data.Numeric == 255);

    ////// Four bytes NodeId
    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(nodeId2.Namespace == OPCUA_NAMESPACE_INDEX);
    ck_assert(nodeId2.Data.Numeric == 256);

    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(nodeId2.Namespace == 1);
    ck_assert(nodeId2.Data.Numeric == 255);

    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(nodeId2.Namespace == 5);
    ck_assert(nodeId2.Data.Numeric == 1025);

    ////// Numeric NodeId
    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(nodeId2.Namespace == 5);
    ck_assert(nodeId2.Data.Numeric == 0x010000);

    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, buffer);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(nodeId2.Namespace == 256);
    ck_assert(nodeId2.Data.Numeric == 1025);

    // TODO: read all other types possibles !

    //// Degraded read
    status = SOPC_NodeId_Read(NULL, buffer);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_NodeId_Read(&nodeId, NULL);
    ck_assert(status != SOPC_STATUS_OK);
    status = SOPC_NodeId_Read(&nodeId, buffer); // Nothing to read anymore
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
    ck_assert(SOPC_STATUS_OK == status);

    // Write in buffer the string and clear string
    status = SOPC_String_Write(&s, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_String_Clear(&s);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Nominal read of the string and clear string
    status = SOPC_String_Read(&s, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_String_Clear(&s);

    /* LIMIT SIZE OF A STRING */

    // Reset position to write to buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    // Change string length artificially in buffer
    length = SOPC_MAX_STRING_LENGTH;
    SOPC_Int32_Write(&length, buffer);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Degraded read of the string (buffer too short for the string length)
    status = SOPC_String_Read(&s, buffer);
    // Expected buffer invalid state: not enough bytes
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Degraded read of the bytestring and clear bytestring
    status = SOPC_ByteString_Read(&bs, buffer);
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);
    SOPC_ByteString_Clear(&bs);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Degraded read of the bytestring and clear bytestring
    status = SOPC_XmlElement_Read(&xml, buffer);
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);
    SOPC_XmlElement_Clear(&xml);

    /* LIMIT SIZE + 1 OF A STRING */

    // Reset position to write to buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Change string length artificially in buffer
    length = SOPC_MAX_STRING_LENGTH + 1;
    SOPC_Int32_Write(&length, buffer);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Degraded read of the string (string too long)
    status = SOPC_String_Read(&s, buffer);
    // Expected decoding error: string too long
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Nominal read of the bytestring and clear bytestring
    status = SOPC_ByteString_Read(&bs, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Nominal read of the bytestring and clear bytestring
    status = SOPC_XmlElement_Read(&xml, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* NOMINAL VARIANT ARRAY ELEMENTS */

    v.BuiltInTypeId = SOPC_Boolean_Id;
    v.ArrayType = SOPC_VariantArrayType_Array;
    v.Value.Array.Length = 10;
    v.Value.Array.Content.BooleanArr = calloc(v.Value.Array.Length, sizeof(SOPC_Boolean));
    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    /* LIMIT SIZE OF A VARIANT ARRAY ELEMENTS */

    // Set position to write to buffer: position 1 to write array length
    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert(SOPC_STATUS_OK == status);
    // Overwrite with limit length value
    length = SOPC_MAX_ARRAY_LENGTH;
    status = SOPC_Int32_Write(&length, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_ENCODING_ERROR == status);

    /* LIMIT SIZE + 1 OF A VARIANT ARRAY ELEMENTS */
    // Set position to write to buffer: position 1 to write array length
    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert(SOPC_STATUS_OK == status);
    // Overwrite with limit+1 length value
    length = SOPC_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT SIZE + 1 OF A VARIANT MATRIX ELEMENTS */
    v.BuiltInTypeId = SOPC_Boolean_Id;
    v.ArrayType = SOPC_VariantArrayType_Matrix;
    v.Value.Matrix.Dimensions = 3;
    v.Value.Matrix.ArrayDimensions = malloc(sizeof(int32_t) * v.Value.Matrix.Dimensions);
    ck_assert(v.Value.Matrix.ArrayDimensions != NULL);
    v.Value.Matrix.ArrayDimensions[0] = 3;
    v.Value.Matrix.ArrayDimensions[1] = 3;
    v.Value.Matrix.ArrayDimensions[2] = 1;
    v.Value.Matrix.Content.BooleanArr = calloc(
        v.Value.Matrix.ArrayDimensions[0] * v.Value.Matrix.ArrayDimensions[1] * v.Value.Matrix.ArrayDimensions[2],
        sizeof(SOPC_Boolean));
    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // Set position to write to buffer: position 1 to write array length
    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert(SOPC_STATUS_OK == status);
    // Overwrite with limit+1 length value
    length = SOPC_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer);
    ck_assert(SOPC_STATUS_OK == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT SIZE + 1 OF A VARIANT MATRIX DIMENSIONS */
    v.BuiltInTypeId = SOPC_Boolean_Id;
    v.ArrayType = SOPC_VariantArrayType_Matrix;
    v.Value.Matrix.Dimensions = 3;
    v.Value.Matrix.ArrayDimensions = malloc(sizeof(int32_t) * v.Value.Matrix.Dimensions);
    ck_assert(v.Value.Matrix.ArrayDimensions != NULL);
    v.Value.Matrix.ArrayDimensions[0] = 3;
    v.Value.Matrix.ArrayDimensions[1] = 3;
    v.Value.Matrix.ArrayDimensions[2] = 1;
    v.Value.Matrix.Content.BooleanArr = calloc(
        v.Value.Matrix.ArrayDimensions[0] * v.Value.Matrix.ArrayDimensions[1] * v.Value.Matrix.ArrayDimensions[2],
        sizeof(SOPC_Boolean));
    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // Set position to write to buffer: position = position - 4 * 4 bytes to write array dimensions length
    status = SOPC_Buffer_SetPosition(buffer, buffer->position - 4 * 4);
    ck_assert(SOPC_STATUS_OK == status);
    // Overwrite with limit+1 length value
    length = SOPC_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer);
    ck_assert(SOPC_STATUS_OK == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);

    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT SIZE + 1 OF A VARIANT ARRAY ELEMENTS (TEST NESTED VERSION OF CODE) */
    v.BuiltInTypeId = SOPC_Variant_Id;
    v.ArrayType = SOPC_VariantArrayType_Array;
    v.Value.Array.Length = 1;
    v.Value.Array.Content.VariantArr = malloc(sizeof(SOPC_Variant));
#ifdef __TRUSTINSOFT_BUGFIX__
    // check malloc != NULL
    ck_assert(NULL != v.Value.Array.Content.VariantArr);
#endif
    SOPC_Variant_Initialize(&v.Value.Array.Content.VariantArr[0]);
    v.Value.Array.Content.VariantArr[0].BuiltInTypeId = SOPC_Boolean_Id;
    v.Value.Array.Content.VariantArr[0].ArrayType = SOPC_VariantArrayType_SingleValue;
    v.Value.Array.Content.VariantArr[0].Value.Boolean = true;
    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    status = SOPC_Buffer_SetPosition(buffer, 1);
    ck_assert(SOPC_STATUS_OK == status);
    // Overwrite with limit+1 length value
    length = SOPC_MAX_ARRAY_LENGTH + 1;
    status = SOPC_Int32_Write(&length, buffer);
    ck_assert(SOPC_STATUS_OK == status);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT OF NESTED VARIANT ELEMENTS */

    // ENCODING A VARIANT WITH SOPC_MAX_VARIANT_NESTED_LEVEL nested levels
    pvar = &v;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL; idx++)
    {
        pvar->BuiltInTypeId = SOPC_Variant_Id;
        pvar->ArrayType = SOPC_VariantArrayType_Array;
        pvar->Value.Array.Length = 1;
        pvar->Value.Array.Content.VariantArr = malloc(sizeof(SOPC_Variant));
#ifdef __TRUSTINSOFT_BUGFIX__
        // check malloc != NULL
        ck_assert(NULL != pvar->Value.Array.Content.VariantArr);
#endif
        ck_assert(pvar->Value.Array.Content.VariantArr != NULL);
        SOPC_Variant_Initialize(&pvar->Value.Array.Content.VariantArr[0]);
        pvar = pvar->Value.Array.Content.VariantArr;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // MANUAL ENCODING FOR THE SAME BELOW
    /*encodingByte = SOPC_Variant_Id | SOPC_VariantArrayValueFlag;
    length = 1;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // ArrayLength
        status = SOPC_Int32_Write(&length, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // Then next variant !
    }
     encodingByte = SOPC_Boolean_Id; // Boolean Id
    // EncodingMask byte
     status = SOPC_Byte_Write(&encodingByte, buffer);
     ck_assert(SOPC_STATUS_OK == status);
    // Boolean value
     boolVal = true;
     status = SOPC_Boolean_Write(&boolVal, buffer);
     ck_assert(SOPC_STATUS_OK == status);*/
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // ENCODING A DATAVALUE WITH SOPC_MAX_VARIANT_NESTED_LEVEL Variant nested levels
    pvar = &v;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL; idx++)
    {
        pvar->BuiltInTypeId = SOPC_DataValue_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.DataValue = malloc(sizeof(SOPC_DataValue));
        ck_assert(pvar->Value.DataValue != NULL);
        SOPC_DataValue_Initialize(pvar->Value.DataValue);
        SOPC_Variant_Initialize(&pvar->Value.DataValue->Value);
        pvar = &pvar->Value.DataValue->Value;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT + 1 OF NESTED VARIANT ELEMENTS */

    // TEST ENCODER FAILURE OF A VARIANT WITH SOPC_MAX_VARIANT_NESTED_LEVEL + 1 nested levels
    pvar = &v;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL + 1; idx++)
    {
        pvar->BuiltInTypeId = SOPC_DataValue_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.DataValue = malloc(sizeof(SOPC_DataValue));
        ck_assert(pvar->Value.DataValue != NULL);
        SOPC_DataValue_Initialize(pvar->Value.DataValue);
        SOPC_Variant_Initialize(&pvar->Value.DataValue->Value);
        pvar = &pvar->Value.DataValue->Value;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // MANUAL ENCODING A VARIANT WITH SOPC_MAX_VARIANT_NESTED_LEVEL + 1 nested levels
    encodingByte = 152; // Variant Id + 2^7 bit to indicate an array
    length = 1;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL + 1; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // ArrayLength
        status = SOPC_Int32_Write(&length, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // Then next variant !
    }
    encodingByte = 1; // Boolean Id
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // Boolean value
    boolVal = true;
    status = SOPC_Boolean_Write(&boolVal, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // TEST ENCODER FAILURE OF A DATAVALUE WITH SOPC_MAX_VARIANT_NESTED_LEVEL + 1 Variant nested levels
    pvar = &v;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL + 1; idx++)
    {
        pvar->BuiltInTypeId = SOPC_DataValue_Id;
        pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
        pvar->Value.DataValue = malloc(sizeof(SOPC_DataValue));
        ck_assert(pvar->Value.DataValue != NULL);
        SOPC_DataValue_Initialize(pvar->Value.DataValue);
        SOPC_Variant_Initialize(&pvar->Value.DataValue->Value);
        pvar = &pvar->Value.DataValue->Value;
    }
    pvar->BuiltInTypeId = SOPC_Boolean_Id;
    pvar->ArrayType = SOPC_VariantArrayType_SingleValue;
    pvar->Value.Boolean = true;

    status = SOPC_Variant_Write(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);
    SOPC_Variant_Clear(&v);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    // MANUAL ENCODING OF A DATAVALUE WITH SOPC_MAX_VARIANT_NESTED_LEVEL + 1 Variant nested levels
    length = 1;
    for (idx = 1; idx <= SOPC_MAX_VARIANT_NESTED_LEVEL + 1; idx++)
    {
        encodingByte = SOPC_DataValue_Id; // DataValue Id
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // DataValue
        encodingByte = SOPC_DataValue_NotNullValue; // It indicates there is a value and nothing else in DataValue
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer);
        ck_assert(SOPC_STATUS_OK == status);

        // Then next data value !
    }
    encodingByte = 1; // Boolean Id
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // Boolean value
    boolVal = true;
    status = SOPC_Boolean_Write(&boolVal, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read variant
    status = SOPC_Variant_Read(&v, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT OF NESTED DIAG INFO */

    // ENCODING A DIAG INFO WITH SOPC_MAX_DIAG_INFO_NESTED_LEVEL nested levels
    pDiag = &diag;
    for (idx = 1; idx <= SOPC_MAX_DIAG_INFO_NESTED_LEVEL; idx++)
    {
        pDiag->InnerDiagnosticInfo = malloc(sizeof(SOPC_DiagnosticInfo));
        ck_assert(pDiag->InnerDiagnosticInfo != NULL);
        SOPC_DiagnosticInfo_Initialize(pDiag->InnerDiagnosticInfo);
        pDiag = pDiag->InnerDiagnosticInfo;
    }
    pDiag->SymbolicId = 1000;

    status = SOPC_DiagnosticInfo_Write(&diag, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_DiagnosticInfo_Clear(&diag);

    // MANUAL ENCODING FOR THE SAME BELOW (~ avoiding not necessary fields)
    /*
    encodingByte = SOPC_DiagInfoEncoding_InnerDianosticInfo;
    length = 1;
    for (idx = 1; idx <= SOPC_MAX_DIAG_INFO_NESTED_LEVEL; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // Then next diag info !
    }
    encodingByte = SOPC_DiagInfoEncoding_SymbolicId;
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // Int32 value
    symbolicId = 1000;
    status = SOPC_Int32_Write(&symbolicId, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    */
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read diag info
    status = SOPC_DiagnosticInfo_Read(&diag, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    SOPC_DiagnosticInfo_Clear(&diag);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

    /* LIMIT+1 OF NESTED DIAG INFO */

    // ENCODING A DIAG INFO WITH SOPC_MAX_DIAG_INFO_NESTED_LEVEL+1 nested levels
    encodingByte = SOPC_DiagInfoEncoding_InnerDianosticInfo;
    length = 1;
    for (idx = 1; idx <= SOPC_MAX_DIAG_INFO_NESTED_LEVEL + 1; idx++)
    {
        // EncodingMask byte
        status = SOPC_Byte_Write(&encodingByte, buffer);
        ck_assert(SOPC_STATUS_OK == status);
        // Then next diag info !
    }
    encodingByte = SOPC_DiagInfoEncoding_SymbolicId;
    // EncodingMask byte
    status = SOPC_Byte_Write(&encodingByte, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // Int32 value
    symbolicId = 1000;
    status = SOPC_Int32_Write(&symbolicId, buffer);
    ck_assert(SOPC_STATUS_OK == status);
    // END OF ENCODING

    // Reset position to read from buffer
    status = SOPC_Buffer_SetPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    // Read diag info
    status = SOPC_DiagnosticInfo_Read(&diag, buffer);
    ck_assert(SOPC_STATUS_OUT_OF_MEMORY == status);

    // Reset position of buffer
    status = SOPC_Buffer_ResetAfterPosition(buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);

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
    ck_assert(SOPC_STATUS_OK == status);
    result = SOPC_String_Equal(&s1, &s2);
    ck_assert(result != false);

    // Test a NULL string with length=0 copy
    s1.Length = 0;
    status = SOPC_String_Copy(&s2, &s1);
    ck_assert(SOPC_STATUS_OK == status);
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
#ifdef __TRUSTINSOFT_BUGFIX__
    // check status ok (ie. malloc != NULL)
    ck_assert(SOPC_STATUS_OK == status);
#endif
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

Suite* tests_make_suite_tools(void)
{
    Suite* s;
    TCase *tc_hexlify, *tc_basetools, *tc_encoder, *tc_ua_types, *tc_buffer, *tc_linkedlist, *tc_async_queue;

    s = suite_create("Tools");
    tc_basetools = tcase_create("String tools");
    tcase_add_test(tc_basetools, test_base_tools);
    tcase_add_test(tc_basetools, test_helper_uri);
    tcase_add_test(tc_basetools, test_strtouint);
    tcase_add_test(tc_basetools, test_string_nodeid);
    suite_add_tcase(s, tc_basetools);
    tc_buffer = tcase_create("Buffer");
    tcase_add_test(tc_buffer, test_buffer_create);
    tcase_add_test(tc_buffer, test_buffer_read_write);
    tcase_add_test(tc_buffer, test_buffer_copy);
    tcase_add_test(tc_buffer, test_buffer_reset);
    tcase_add_test(tc_buffer, test_buffer_set_properties);
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
#ifdef __TRUSTINSOFT_HELPER__
    // skip threads tests
#else
    tcase_add_test(tc_async_queue, test_async_queue_threads);
#endif
    suite_add_tcase(s, tc_async_queue);

    tc_encoder = tcase_create("UA Encoder");
    tcase_add_test(tc_encoder, test_ua_encoder_endianness_mgt);
    tcase_add_test(tc_encoder, test_ua_encoder_basic_types);
    tcase_add_test(tc_encoder, test_ua_encoder_other_types);
    tcase_add_test(tc_encoder, test_ua_decoder_allocation_limit);
    suite_add_tcase(s, tc_encoder);

    tc_ua_types = tcase_create("UA Types");
    tcase_add_test(tc_ua_types, test_ua_string_type);
    suite_add_tcase(s, tc_ua_types);

    return s;
}
