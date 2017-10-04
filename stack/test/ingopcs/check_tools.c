/*
 * Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
 *
 *  Copyright (C) 2016 Systerel and others.
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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <check.h>

#include "sopc_helper_string.h"
#include "sopc_buffer.h"
#include "sopc_singly_linked_list.h"
#include "sopc_base_types.h"
#include "check_stack.h"
#include "hexlify.h"
#include "sopc_action_queue.h"
#include "sopc_async_queue.h"
#include "sopc_threads.h"
#include "sopc_time.h"

START_TEST(test_hexlify)
{
    unsigned char buf[33], c, d = 0;
    int i;

    // Init
    memset(buf, 0, 33);

    // Test single chars
    for(i=0; i<256; ++i)
    {
        c = (unsigned char)i;
        ck_assert(hexlify(&c, (char *)buf, 1) == 1);
        ck_assert(unhexlify((char *)buf, &d, 1) == 1);
        ck_assert(c == d);
    }

    // Test vector
    ck_assert(hexlify((unsigned char *)"\x00 Test \xFF", (char *)buf, 8) == 8);
    ck_assert(strncmp((char *)buf, "00205465737420ff", 16) == 0);
    ck_assert(unhexlify((char *)buf, buf+16, 8) == 8);
    ck_assert(strncmp((char *)(buf+16), "\x00 Test \xFF", 8) == 0);

    // Test overflow
    buf[32] = 0xDD;
    ck_assert(hexlify((unsigned char *)"\x00 Test \xFF\x00 Test \xFF", (char *)buf, 16) == 16);
    ck_assert(buf[32] == 0xDD);
    ck_assert(unhexlify("00205465737420ff00205465737420ff", buf, 16) == 16);
    ck_assert(buf[32] == 0xDD);
}
END_TEST


START_TEST(test_buffer_create)
{
    SOPC_StatusCode status = 0;

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
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    uint8_t readData[4] = {0x00, 0x00, 0x00, 0x00};
    SOPC_StatusCode status = 0;
    SOPC_Buffer* buf = NULL;

    // Test write
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status == 0);
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
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = SOPC_Buffer_Write(buf, data, 3);
    ck_assert(status != STATUS_OK);

    buf = SOPC_Buffer_Create(1);
    ////// NULL data pointer
    status = SOPC_Buffer_Write(buf, NULL, 3);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);

    ////// Full buffer
    status = SOPC_Buffer_Write(buf, data, 2);
    ck_assert(status != STATUS_OK);
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
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 4);
    status = SOPC_Buffer_Read(readData, buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->position == 2);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x00);
    ck_assert(readData[3] == 0x00);
    status = SOPC_Buffer_Read(&(readData[2]), buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->position == 4);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x02);
    ck_assert(readData[3] == 0x03);
    SOPC_Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Read(readData, buf, 3);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = SOPC_Buffer_Read(readData, buf, 3);
    ck_assert(status != STATUS_OK);

    buf = SOPC_Buffer_Create(4);
    ////// NULL data pointer
    status = SOPC_Buffer_Read(NULL, buf, 3);
    ck_assert(status != STATUS_OK);
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
    ck_assert(status != STATUS_OK);
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
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Buffer* buf = NULL;
    SOPC_Buffer* buf2 = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    buf2 = SOPC_Buffer_Create(5);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status == STATUS_OK);
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
    ck_assert(status == STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    ck_assert(buf2->data[3] == 0x00);

    //// Test degraded cases
    /////// NULL pointers
    status = SOPC_Buffer_Copy(buf2, NULL);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    status = SOPC_Buffer_CopyWithLength(buf2, NULL, 4);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    status = SOPC_Buffer_Copy(NULL, buf);
    ck_assert(status != STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(NULL, buf, 3);
    ck_assert(status != STATUS_OK);


    /////// Destination buffer size insufficient
    SOPC_Buffer_Reset(buf);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    status = SOPC_Buffer_CopyWithLength(buf2, buf, 6);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    SOPC_Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;

    buf = SOPC_Buffer_Create(1);
    SOPC_Buffer_Delete(buf2);
    buf2 = NULL;

    /////// Non allocated buffer data
    buf2 = malloc(sizeof(SOPC_Buffer));
    memset(buf2, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_Copy(buf2, buf);
    ck_assert(status != STATUS_OK);
    status = SOPC_Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != STATUS_OK);
    free(buf2);
    buf2 = NULL;
    SOPC_Buffer_Delete(buf);

}
END_TEST


START_TEST(test_buffer_reset)
{
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Buffer* buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 4);
    SOPC_Buffer_Reset(buf);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 0 <=> Reset
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_ResetAfterPosition(buf, 0);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 2 in length of 4 buffer
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status == STATUS_OK);
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
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    //// Test degraded cases
    /////// NULL pointers
    status = SOPC_Buffer_ResetAfterPosition(NULL, 2);
    ck_assert(status != STATUS_OK);

    /////// Invalid position: position > length
    status = SOPC_Buffer_ResetAfterPosition(buf, 5);
    ck_assert(status != STATUS_OK);
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
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
        status = SOPC_Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;
}
END_TEST

START_TEST(test_buffer_set_properties)
{
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Buffer* buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = SOPC_Buffer_Create(10);
    status = SOPC_Buffer_Write(buf, data, 4);
    status = SOPC_Buffer_SetPosition(buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    status = SOPC_Buffer_SetDataLength(buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    //// Test degraded cases
    /////// NULL pointers
    status = SOPC_Buffer_SetPosition(NULL, 1);
    ck_assert(status != STATUS_OK);
    status = SOPC_Buffer_SetDataLength(NULL, 1);
    ck_assert(status != STATUS_OK);

    /////// Invalid position: position > length
    status = SOPC_Buffer_SetPosition(buf, 3);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);

    /////// Invalid length: length < position
    status = SOPC_Buffer_SetDataLength(buf, 1);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);

    SOPC_Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = malloc(sizeof(SOPC_Buffer));
    memset(buf, 0, sizeof(SOPC_Buffer));
    ////// Non allocated buffer data
    status = SOPC_Buffer_SetPosition(buf, 0);
    ck_assert(status != STATUS_OK);
    status = SOPC_Buffer_SetDataLength(buf, 0);
    ck_assert(status != STATUS_OK);
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

    SOPC_SLinkedList* list = NULL;
    SOPC_SLinkedListIterator it = NULL;

    // Test creation
    //// Test nominal case
    list = SOPC_SLinkedList_Create(2);
    ck_assert(list != NULL);
    SOPC_SLinkedList_Delete(list);
    list = NULL;

    // Test creation and addition
    //// Test nominal case
    list = SOPC_SLinkedList_Create(3);
    ck_assert(list != NULL);
    value = SOPC_SLinkedList_Prepend(list, 1, &value1);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_Prepend(list, 2, &value2);
    ck_assert(value == &value2);
    value = SOPC_SLinkedList_Append(list, 3, &value3);
    ck_assert(value == &value3);

    /// Test iterator nominal case
    it = SOPC_SLinkedList_GetIterator(list);
    ck_assert(it != NULL);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == &value2);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == &value3);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == NULL);

    //// Test degraded case: add in full linked list
    value = SOPC_SLinkedList_Prepend(list, 4, &value3);
    ck_assert(value == NULL);
    //// Test degraded case: add to NULL pointer linked list
    value = SOPC_SLinkedList_Prepend(NULL, 4, &value3);
    ck_assert(value == NULL);

    // Test pop nominal case
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value2);
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == &value3);

    // Test pop degraded case
    value = SOPC_SLinkedList_PopHead(list);
    ck_assert(value == NULL);

    SOPC_SLinkedList_Delete(list);
    list = NULL;

    // Test find and remove
    list = SOPC_SLinkedList_Create(4);
    ck_assert(list != NULL);

    /// (Test iterator degraded case)
    it = SOPC_SLinkedList_GetIterator(list);
    ck_assert(it == NULL);
    value = SOPC_SLinkedList_Next(&it);
    ck_assert(value == NULL);

    /// (Continue initial test)
    value = SOPC_SLinkedList_Prepend(list, 0, &value1);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_Prepend(list, 2, &value2);
    ck_assert(value == &value2);
    value = SOPC_SLinkedList_Prepend(list, UINT32_MAX, &value3);
    ck_assert(value == &value3);
    value = SOPC_SLinkedList_Prepend(list, UINT32_MAX, &value1);
    ck_assert(value == &value1);
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
    value = SOPC_SLinkedList_FindFromId(list, 0);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(list, 0);
    ck_assert(value == NULL);

    value = SOPC_SLinkedList_RemoveFromId(list, 2);
    ck_assert(value == &value2);
    value = SOPC_SLinkedList_FindFromId(list, 2);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(list, 2);
    ck_assert(value == NULL);

    //// Check LIFO behavior in case id not unique
    value = SOPC_SLinkedList_RemoveFromId(list, UINT32_MAX);
    ck_assert(value == &value1);
    value = SOPC_SLinkedList_FindFromId(list, UINT32_MAX);
    ck_assert(value == &value3);
    value = SOPC_SLinkedList_RemoveFromId(list, UINT32_MAX);
    ck_assert(value == &value3);
    value = SOPC_SLinkedList_FindFromId(list, UINT32_MAX);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(list, UINT32_MAX);
    ck_assert(value == NULL);

    //// Verify degraded remove behavior
    value = SOPC_SLinkedList_RemoveFromId(NULL, 2);
    ck_assert(value == NULL);
    value = SOPC_SLinkedList_RemoveFromId(NULL, 1);
    ck_assert(value == NULL);

    //// Check apply to free elements
    void *p = NULL;
    SOPC_SLinkedList_Clear(list);
    p = malloc(sizeof(int));
    ck_assert(NULL != p);
    *(int *)p = 2;
    ck_assert(SOPC_SLinkedList_Prepend(list, 0, p) != NULL);
    p = malloc(sizeof(double));
    ck_assert(NULL != p);
    *(double *)p = 2.;
    ck_assert(SOPC_SLinkedList_Prepend(list, 1, p) != NULL);
    p = malloc(sizeof(char)*5);
    ck_assert(NULL != p);
    memcpy(p, "toto", 5);
    ck_assert(SOPC_SLinkedList_Prepend(list, 2, p) != NULL);
    SOPC_SLinkedList_Apply(list, SOPC_SLinkedList_EltGenericFree);

    SOPC_SLinkedList_Delete(list);
    list = NULL;
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

void FctPointer (void* arg){
    (void) arg;
}

START_TEST(test_msg_queue)
{
    SOPC_ActionFunction* af = NULL;
    void* arg = NULL;
    const char* txt = NULL;
    SOPC_ActionQueue* queue = NULL;
    int paramAndRes = -10;
    int paramAndRes2 = 0;
    SOPC_StatusCode status = SOPC_ActionQueue_Init(&queue, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &paramAndRes, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &paramAndRes2, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingDequeue(queue, &af, &arg, &txt);
    ck_assert(status == STATUS_OK);
    ck_assert(FctPointer == af && arg == &paramAndRes);
    af = NULL;
    status = SOPC_Action_NonBlockingDequeue(queue, &af, &arg, &txt);
    ck_assert(status == STATUS_OK);
    ck_assert(FctPointer == af && arg == &paramAndRes2);
    status = SOPC_Action_NonBlockingDequeue(queue, &af, &arg, &txt);
    ck_assert(status == OpcUa_BadWouldBlock);
    SOPC_ActionQueue_Free(&queue);
}
END_TEST

typedef struct Msg_Queue_Params {
    SOPC_ActionQueue* queue;
    uint8_t nbMsgs;
    uint8_t blockingDequeue;
    uint8_t success;
} Msg_Queue_Params;

void* test_msg_queue_blocking_dequeue_fct(void* args){
    SOPC_ActionFunction* af = NULL;
    void* arg = NULL;
    SOPC_StatusCode status;
    Msg_Queue_Params* param = (Msg_Queue_Params*) args;
    uint8_t nbDequeued = 0;
    uint8_t lastValueDeq = 0;
    uint8_t success = !FALSE;
    // Dequeue expected number of messages with increased value sequence as action parameter
    while(nbDequeued < param->nbMsgs && success != FALSE){
        param->blockingDequeue = !FALSE;
        status = SOPC_Action_BlockingDequeue(param->queue, &af, &arg, NULL);
        param->blockingDequeue = FALSE;
        nbDequeued++;
        assert(status == STATUS_OK);
        if(af == FctPointer && *((uint8_t*) arg) == lastValueDeq + 1){
            lastValueDeq++;
        }else{
            success = FALSE;
        }
    }
    param->success = success;
    return NULL;
}

void* test_msg_queue_nonblocking_dequeue_fct(void* args){
    SOPC_ActionFunction* af = NULL;
    void* arg = NULL;
    SOPC_StatusCode status;
    Msg_Queue_Params* param = (Msg_Queue_Params*) args;
    uint8_t nbDequeued = 0;
    uint8_t lastValueDeq = 0;
    uint8_t success = !FALSE;
    // Dequeue expected number of messages with increased value sequence as action parameter
    while(nbDequeued < param->nbMsgs && success != FALSE){
        status = SOPC_Action_NonBlockingDequeue(param->queue, &af, &arg, NULL);
        if(status == STATUS_OK){
            nbDequeued++;
            assert(status == STATUS_OK);
            if(af == FctPointer && *((uint8_t*) arg) == lastValueDeq + 1){
                lastValueDeq++;
            }else{
                success = FALSE;
            }
        }else{
            if(status != OpcUa_BadWouldBlock){
                success = FALSE;
            }
        }
        SOPC_Sleep(10);
    }
    param->success = success;
    return NULL;
}

START_TEST(test_msg_queue_threads)
{
    Thread thread;
    Msg_Queue_Params params;
    SOPC_ActionQueue* queue;
    const uint8_t one = 1;
    const uint8_t two = 2;
    const uint8_t three = 3;
    const uint8_t four = 4;
    const uint8_t five = 5;
    SOPC_StatusCode status = SOPC_ActionQueue_Init(&queue, NULL);
    params.success = FALSE;
    params.blockingDequeue = FALSE;
    params.nbMsgs = 5;
    params.queue = queue;
    // Nominal behavior of async queue FIFO (blocking dequeue)
    status = SOPC_Thread_Create(&thread, test_msg_queue_blocking_dequeue_fct, &params);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(params.blockingDequeue == !FALSE);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &one, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &two, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &three, NULL);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(100);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &four, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &five, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_OK);
    ck_assert(params.success == !FALSE);

    // Nominal behavior of async queue FIFO (non blocking dequeue)
    params.success = FALSE;
    params.blockingDequeue = FALSE;
    params.nbMsgs = 5;
    params.queue = queue;
    status = SOPC_Thread_Create(&thread, test_msg_queue_nonblocking_dequeue_fct, &params);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &one, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &two, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &three, NULL);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(100);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &four, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Action_BlockingEnqueue(queue, FctPointer, (void*) &five, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_OK);
    ck_assert(params.success == !FALSE);

    SOPC_ActionQueue_Free(&queue);

}
END_TEST

// Async queue

START_TEST(test_async_queue)
{
    void* arg = NULL;
    SOPC_AsyncQueue* queue = NULL;
    int paramAndRes = -10;
    int paramAndRes2 = 0;
    SOPC_StatusCode status = SOPC_AsyncQueue_Init(&queue, NULL);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &paramAndRes);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &paramAndRes2);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingDequeue(queue, &arg);
    ck_assert(status == STATUS_OK);
    ck_assert(arg == &paramAndRes);
    status = SOPC_AsyncQueue_NonBlockingDequeue(queue, &arg);
    ck_assert(status == STATUS_OK);
    ck_assert(arg == &paramAndRes2);
    status = SOPC_AsyncQueue_NonBlockingDequeue(queue, &arg);
    ck_assert(status == OpcUa_BadWouldBlock);
    SOPC_AsyncQueue_Free(&queue);
}
END_TEST

typedef struct AsyncQueue_Element {
    SOPC_AsyncQueue* queue;
    uint8_t nbMsgs;
    uint8_t blockingDequeue;
    uint8_t success;
} AsyncQueue_Element;

void* test_async_queue_blocking_dequeue_fct(void* args){
    void* arg = NULL;
    SOPC_StatusCode status;
    AsyncQueue_Element* param = (AsyncQueue_Element*) args;
    uint8_t nbDequeued = 0;
    uint8_t lastValueDeq = 0;
    uint8_t success = !FALSE;
    // Dequeue expected number of messages with increased value sequence as action parameter
    while(nbDequeued < param->nbMsgs && success != FALSE){
        param->blockingDequeue = !FALSE;
        status = SOPC_AsyncQueue_BlockingDequeue(param->queue, &arg);
        param->blockingDequeue = FALSE;
        nbDequeued++;
        assert(status == STATUS_OK);
        if(*((uint8_t*) arg) == lastValueDeq + 1){
            lastValueDeq++;
        }else{
            success = FALSE;
        }
    }
    param->success = success;
    return NULL;
}

void* test_async_queue_nonblocking_dequeue_fct(void* args){
    void* arg = NULL;
    SOPC_StatusCode status;
    AsyncQueue_Element* param = (AsyncQueue_Element*) args;
    uint8_t nbDequeued = 0;
    uint8_t lastValueDeq = 0;
    uint8_t success = !FALSE;
    // Dequeue expected number of messages with increased value sequence as action parameter
    while(nbDequeued < param->nbMsgs && success != FALSE){
        status = SOPC_AsyncQueue_NonBlockingDequeue(param->queue, &arg);
        if(status == STATUS_OK){
            nbDequeued++;
            assert(status == STATUS_OK);
            if(*((uint8_t*) arg) == lastValueDeq + 1){
                lastValueDeq++;
            }else{
                success = FALSE;
            }
        }else{
            if(status != OpcUa_BadWouldBlock){
                success = FALSE;
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
    const uint8_t one = 1;
    const uint8_t two = 2;
    const uint8_t three = 3;
    const uint8_t four = 4;
    const uint8_t five = 5;
    SOPC_StatusCode status = SOPC_AsyncQueue_Init(&queue, NULL);
    params.success = FALSE;
    params.blockingDequeue = FALSE;
    params.nbMsgs = 5;
    params.queue = queue;
    // Nominal behavior of async queue FIFO (blocking dequeue)
    status = SOPC_Thread_Create(&thread, test_async_queue_blocking_dequeue_fct, &params);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(params.blockingDequeue == !FALSE);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &one);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue,(void*) &two);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue,(void*) &three);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(100);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue,(void*) &four);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue,(void*) &five);
    ck_assert(status == STATUS_OK);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_OK);
    ck_assert(params.success == !FALSE);

    // Nominal behavior of async queue FIFO (non blocking dequeue)
    params.success = FALSE;
    params.blockingDequeue = FALSE;
    params.nbMsgs = 5;
    params.queue = queue;
    status = SOPC_Thread_Create(&thread, test_async_queue_nonblocking_dequeue_fct, &params);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &one);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &two);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &three);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(100);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &four);
    ck_assert(status == STATUS_OK);
    status = SOPC_AsyncQueue_BlockingEnqueue(queue, (void*) &five);
    ck_assert(status == STATUS_OK);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_OK);
    ck_assert(params.success == !FALSE);

    SOPC_AsyncQueue_Free(&queue);

}
END_TEST

Suite *tests_make_suite_tools(void)
{
    Suite *s;
    TCase *tc_hexlify, *tc_msg_queue, *tc_basetools, *tc_buffer, *tc_linkedlist, *tc_async_queue;

    s = suite_create("Tools");
    tc_basetools = tcase_create("Base tools");
    tcase_add_test(tc_basetools, test_base_tools);
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
    suite_add_tcase(s, tc_linkedlist);

    tc_hexlify = tcase_create("Hexlify (tests only)");
    tcase_add_test(tc_hexlify, test_hexlify);
    suite_add_tcase(s, tc_hexlify);

    tc_msg_queue = tcase_create("Action queue");
    tcase_add_test(tc_msg_queue, test_msg_queue);
    tcase_add_test(tc_msg_queue, test_msg_queue_threads);
    suite_add_tcase(s, tc_msg_queue);

    tc_async_queue = tcase_create("Async queue");
    tcase_add_test(tc_async_queue, test_async_queue);
    tcase_add_test(tc_async_queue, test_async_queue_threads);
    suite_add_tcase(s, tc_async_queue);

    return s;
}
