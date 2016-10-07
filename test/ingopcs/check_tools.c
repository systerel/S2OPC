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
#include <ua_base_types.h>
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


Suite *tests_make_suite_tools(void)
{
    Suite *s;
    TCase *tc_buffer;

    s = suite_create("Tools");
    tc_buffer = tcase_create("Buffer");
    tcase_add_test(tc_buffer, test_buffer_create);
    suite_add_tcase(s, tc_buffer);

    return s;
}
