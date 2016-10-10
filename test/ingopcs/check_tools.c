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
#include <ua_base_types.h>
#include <buffer.h>
#include <singly_linked_list.h>
#include "check_stack.h"


START_TEST(test_buffer_create)
{
    StatusCode status = 0;

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
    suite_add_tcase(s, tc_buffer);
    tc_linkedlist = tcase_create("Linked List");
    tcase_add_test(tc_linkedlist, test_linked_list);
    suite_add_tcase(s, tc_linkedlist);

    return s;
}
