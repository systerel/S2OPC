/*
 * Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
 *  Created on: Oct 7, 2016
 *      Author: VMO
 */


#include <stdlib.h>
#include <check.h>
#include <buffer.h>
#include <singly_linked_list.h>
#include <sopc_base_types.h>
#include "check_stack.h"


START_TEST(test_buffer_create)
{
    SOPC_StatusCode status = 0;

    // Test creation
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    ck_assert(buf != NULL);
    ck_assert(buf->data != NULL);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 0);
    ck_assert(buf->max_size == 10);
    Buffer_Delete(buf);
    buf = NULL;

    //// Test buffer creation degraded cases
    buf = Buffer_Create(0);
    ck_assert(buf == NULL);

    // Test initialization
    //// Test nominal case
    buf = malloc(sizeof(Buffer));
    status = Buffer_Init(buf, 100);
    ck_assert(status == 0);
    ck_assert(buf->data != NULL);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 0);
    ck_assert(buf->max_size == 100);
    Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded case: NULL buffer
    buf = NULL;
    status = Buffer_Init(buf, 100);
    ck_assert(status != 0);
    ck_assert(buf == NULL);

    //// Test degraded case: invalid size
    buf = NULL;
    status = Buffer_Init(buf, 0);
    ck_assert(status != 0);
    ck_assert(buf == NULL);

}
END_TEST

START_TEST(test_buffer_read_write)
{
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    uint8_t readData[4] = {0x00, 0x00, 0x00, 0x00};
    SOPC_StatusCode status = 0;
    Buffer* buf = NULL;

    // Test write
    //// Test nominal cases
    buf = Buffer_Create(10);
    status = Buffer_Write(buf, data, 3);
    ck_assert(status == 0);
    ck_assert(buf->length == 3);
    ck_assert(buf->position == 3);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    status = Buffer_Write(buf, data, 1);
    ck_assert(status == 0);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x00);
    Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = malloc(sizeof(Buffer));
    memset(buf, 0, sizeof(Buffer));
    ////// Non allocated buffer data
    status = Buffer_Write(buf, data, 3);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = Buffer_Write(buf, data, 3);
    ck_assert(status != STATUS_OK);

    buf = Buffer_Create(1);
    ////// NULL data pointer
    status = Buffer_Write(buf, NULL, 3);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);

    ////// Full buffer
    status = Buffer_Write(buf, data, 2);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    Buffer_Delete(buf);
    buf = NULL;


    // Test read
    //// Test nominal cases
    buf = Buffer_Create(10);
    status = Buffer_Write(buf, data, 4);
    // Reset position for reading content written
    status = Buffer_SetPosition(buf, 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->length == 4);
    status = Buffer_Read(readData, buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->position == 2);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x00);
    ck_assert(readData[3] == 0x00);
    status = Buffer_Read(&(readData[2]), buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->position == 4);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x01);
    ck_assert(readData[2] == 0x02);
    ck_assert(readData[3] == 0x03);
    Buffer_Delete(buf);
    buf = NULL;

    //// Test degraded cases
    buf = malloc(sizeof(Buffer));
    memset(buf, 0, sizeof(Buffer));
    ////// Non allocated buffer data
    status = Buffer_Read(readData, buf, 3);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;

    ////// NULL buf pointer
    status = Buffer_Read(readData, buf, 3);
    ck_assert(status != STATUS_OK);

    buf = Buffer_Create(4);
    ////// NULL data pointer
    status = Buffer_Read(NULL, buf, 3);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);

    ////// Empty buffer
    readData[0] = 0x00;
    readData[1] = 0x00;
    readData[2] = 0x00;
    readData[3] = 0x00;
    status = Buffer_Write(buf, data, 4);
    //// DO NOT reset position for reading content written
    status = Buffer_Read(readData, buf, 4);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(readData[0] == 0x00);
    ck_assert(readData[1] == 0x00);
    ck_assert(readData[2] == 0x00);
    ck_assert(readData[3] == 0x00);
    Buffer_Delete(buf);
    buf = NULL;

}
END_TEST

START_TEST(test_buffer_copy)
{
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    SOPC_StatusCode status = STATUS_OK;
    Buffer* buf = NULL;
    Buffer* buf2 = NULL;

    // Test copy
    //// Test nominal cases
    buf = Buffer_Create(10);
    buf2 = Buffer_Create(5);
    status = Buffer_Write(buf, data, 4);
    status = Buffer_Copy(buf2, buf);
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
    status = Buffer_SetPosition(buf, 1);
    status = Buffer_CopyWithLength(buf2, buf, 3);
    ck_assert(status == STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    ck_assert(buf2->data[3] == 0x00);

    //// Test degraded cases
    /////// NULL pointers
    status = Buffer_Copy(buf2, NULL);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);
    status = Buffer_CopyWithLength(buf2, NULL, 4);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    status = Buffer_Copy(NULL, buf);
    ck_assert(status != STATUS_OK);
    status = Buffer_CopyWithLength(NULL, buf, 3);
    ck_assert(status != STATUS_OK);


    /////// Destination buffer size insufficient
    Buffer_Reset(buf);
    status = Buffer_Write(buf, data, 4);
    status = Buffer_Write(buf, data, 4);
    status = Buffer_Copy(buf2, buf);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    status = Buffer_CopyWithLength(buf2, buf, 6);
    ck_assert(status != STATUS_OK);
    ck_assert(buf2->length == 3);
    ck_assert(buf2->position == 1);
    ck_assert(buf2->data[0] == 0x01);
    ck_assert(buf2->data[1] == 0x01);
    ck_assert(buf2->data[2] == 0x02);

    Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = malloc(sizeof(Buffer));
    memset(buf, 0, sizeof(Buffer));
    ////// Non allocated buffer data
    status = Buffer_Copy(buf2, buf);
    ck_assert(status != STATUS_OK);
    status = Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;

    buf = Buffer_Create(1);
    Buffer_Delete(buf2);
    buf2 = NULL;

    /////// Non allocated buffer data
    buf2 = malloc(sizeof(Buffer));
    memset(buf2, 0, sizeof(Buffer));
    ////// Non allocated buffer data
    status = Buffer_Copy(buf2, buf);
    ck_assert(status != STATUS_OK);
    status = Buffer_CopyWithLength(buf2, buf, 4);
    ck_assert(status != STATUS_OK);
    free(buf2);
    buf2 = NULL;
    Buffer_Delete(buf);

}
END_TEST


START_TEST(test_buffer_reset)
{
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    SOPC_StatusCode status = STATUS_OK;
    Buffer* buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = Buffer_Create(10);
    status = Buffer_Write(buf, data, 4);
    Buffer_Reset(buf);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 0 <=> Reset
    status = Buffer_Write(buf, data, 4);
    status = Buffer_ResetAfterPosition(buf, 0);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 0);
    ck_assert(buf->position == 0);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x00);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 2 in length of 4 buffer
    status = Buffer_Write(buf, data, 4);
    status = Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    ////// Reset with position = 4 in buffer with length 4
    Buffer_Reset(buf);
    status = Buffer_Write(buf, data, 4);
    status = Buffer_ResetAfterPosition(buf, 4);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    //// Test degraded cases
    /////// NULL pointers
    status = Buffer_ResetAfterPosition(NULL, 2);
    ck_assert(status != STATUS_OK);

    /////// Invalid position: position > length
    status = Buffer_ResetAfterPosition(buf, 5);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 4);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = malloc(sizeof(Buffer));
    memset(buf, 0, sizeof(Buffer));
    ////// Non allocated buffer data
        status = Buffer_ResetAfterPosition(buf, 2);
    ck_assert(status != STATUS_OK);
    free(buf);
    buf = NULL;
}
END_TEST

START_TEST(test_buffer_set_properties)
{
    uint8_t data[4] = {0x00, 0x01, 0x02 , 0x03};
    SOPC_StatusCode status = STATUS_OK;
    Buffer* buf = NULL;

    // Test copy
    //// Test nominal cases
    buf = Buffer_Create(10);
    status = Buffer_Write(buf, data, 4);
    status = Buffer_SetPosition(buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 4);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x02);
    ck_assert(buf->data[3] == 0x03);

    status = Buffer_SetDataLength(buf, 2);
    ck_assert(status == STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);
    ck_assert(buf->data[2] == 0x00);
    ck_assert(buf->data[3] == 0x00);

    //// Test degraded cases
    /////// NULL pointers
    status = Buffer_SetPosition(NULL, 1);
    ck_assert(status != STATUS_OK);
    status = Buffer_SetDataLength(NULL, 1);
    ck_assert(status != STATUS_OK);

    /////// Invalid position: position > length
    status = Buffer_SetPosition(buf, 3);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);

    /////// Invalid length: length < position
    status = Buffer_SetDataLength(buf, 1);
    ck_assert(status != STATUS_OK);
    ck_assert(buf->length == 2);
    ck_assert(buf->position == 2);
    ck_assert(buf->data[0] == 0x00);
    ck_assert(buf->data[1] == 0x01);

    Buffer_Delete(buf);
    buf = NULL;

    /////// Non allocated buffer data
    buf = malloc(sizeof(Buffer));
    memset(buf, 0, sizeof(Buffer));
    ////// Non allocated buffer data
    status = Buffer_SetPosition(buf, 0);
    ck_assert(status != STATUS_OK);
    status = Buffer_SetDataLength(buf, 0);
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

    SLinkedList* list = NULL;

    // Test creation
    //// Test nominal case
    list = SLinkedList_Create(2);
    ck_assert(list != NULL);
    SLinkedList_Delete(list);
    list = NULL;

    // Test creation and addition
    //// Test nominal case
    list = SLinkedList_Create(2);
    ck_assert(list != NULL);
    value = SLinkedList_Add(list, 1, &value1);
    ck_assert(value == &value1);
    value = SLinkedList_Add(list, 2, &value2);
    ck_assert(value == &value2);
    //// Test degraded case: add in full linked list
    value = SLinkedList_Add(list, 3, &value3);
    ck_assert(value == NULL);
    //// Test degraded case: add to NULL pointer linked list
    value = SLinkedList_Add(NULL, 3, &value3);
    ck_assert(value == NULL);
    SLinkedList_Delete(list);
    list = NULL;

    // Test find and remove
    list = SLinkedList_Create(4);
    ck_assert(list != NULL);
    value = SLinkedList_Add(list, 0, &value1);
    ck_assert(value == &value1);
    value = SLinkedList_Add(list, 2, &value2);
    ck_assert(value == &value2);
    value = SLinkedList_Add(list, UINT32_MAX, &value3);
    ck_assert(value == &value3);
    value = SLinkedList_Add(list, UINT32_MAX, &value1);
    ck_assert(value == &value1);
    //// Verify nominal find behavior
    value = SLinkedList_Find(list, 0);
    ck_assert(value == &value1);
    value = SLinkedList_Find(list, 2);
    ck_assert(value == &value2);
    //// Check LIFO behavior in case id not unique
    value = SLinkedList_Find(list, UINT32_MAX);
    ck_assert(value == &value1);
    //// Verify degraded find behavior
    value = SLinkedList_Find(NULL, 2);
    ck_assert(value == NULL);
    value = SLinkedList_Find(list, 1);
    ck_assert(value == NULL);

    //// Verify nominal remove behavior
    value = SLinkedList_Remove(list, 0);
    ck_assert(value == &value1);
    value = SLinkedList_Find(list, 0);
    ck_assert(value == NULL);
    value = SLinkedList_Remove(list, 0);
    ck_assert(value == NULL);

    value = SLinkedList_Remove(list, 2);
    ck_assert(value == &value2);
    value = SLinkedList_Find(list, 2);
    ck_assert(value == NULL);
    value = SLinkedList_Remove(list, 2);
    ck_assert(value == NULL);

    //// Check LIFO behavior in case id not unique
    value = SLinkedList_Remove(list, UINT32_MAX);
    ck_assert(value == &value1);
    value = SLinkedList_Find(list, UINT32_MAX);
    ck_assert(value == &value3);
    value = SLinkedList_Remove(list, UINT32_MAX);
    ck_assert(value == &value3);
    value = SLinkedList_Find(list, UINT32_MAX);
    ck_assert(value == NULL);
    value = SLinkedList_Remove(list, UINT32_MAX);
    ck_assert(value == NULL);

    //// Verify degraded remove behavior
    value = SLinkedList_Remove(NULL, 2);
    ck_assert(value == NULL);
    value = SLinkedList_Remove(NULL, 1);
    ck_assert(value == NULL);

    SLinkedList_Delete(list);
    list = NULL;
}
END_TEST

Suite *tests_make_suite_tools(void)
{
    Suite *s;
    TCase *tc_buffer, *tc_linkedlist;

    s = suite_create("Tools");
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

    return s;
}
