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

#include "check_helpers.h"

#include <check.h>
#include <inttypes.h>

#include "sopc_array.h"
#include "sopc_mem_alloc.h"

START_TEST(test_array_create)
{
    // Create with no initial capacity
    SOPC_Array* a = SOPC_Array_Create(sizeof(char), 0, NULL);
    ck_assert_ptr_nonnull(a);
    char val = 'a';
    ck_assert(SOPC_Array_Append(a, val));
    SOPC_Array_Delete(a);

    // Create with initial capacity
    a = SOPC_Array_Create(sizeof(char), 10, NULL);
    ck_assert_ptr_nonnull(a);
    ck_assert(SOPC_Array_Append(a, val));
    SOPC_Array_Delete(a);
}
END_TEST

static void set_char_to_0(void* data)
{
    **((char**) data) = '\0';
}

START_TEST(test_array_free_func)
{
    SOPC_Array* a = SOPC_Array_Create(sizeof(char*), 0, set_char_to_0);
    ck_assert_ptr_nonnull(a);
    ck_assert(set_char_to_0 == SOPC_Array_Get_Free_Func(a));

    char val = 'a';
    char* pVal = &val;
    ck_assert(SOPC_Array_Append(a, pVal));
    SOPC_Array_Delete(a);
    ck_assert_int_eq('\0', val);

    val = 'a';
    a = SOPC_Array_Create(sizeof(char*), 0, set_char_to_0);
    ck_assert_ptr_nonnull(a);
    ck_assert(set_char_to_0 == SOPC_Array_Get_Free_Func(a));
    SOPC_Array_Set_Free_Func(a, NULL);
    ck_assert(NULL == SOPC_Array_Get_Free_Func(a));
    SOPC_Array_Delete(a);
    ck_assert_int_eq('a', val);
}
END_TEST

START_TEST(test_array_append_get)
{
    SOPC_Array* a = SOPC_Array_Create(sizeof(int32_t), 0, NULL);
    ck_assert_ptr_nonnull(a);
    ck_assert_uint_eq(0, SOPC_Array_Size(a));

    int32_t i1 = 30;
    int32_t ix[] = {7, 8};

    ck_assert(SOPC_Array_Append(a, i1));
    ck_assert_uint_eq(1, SOPC_Array_Size(a));

    ck_assert(SOPC_Array_Append_Values(a, NULL, 0));
    ck_assert_uint_eq(1, SOPC_Array_Size(a));

    ck_assert(SOPC_Array_Append_Values(a, ix, 2));
    ck_assert_uint_eq(3, SOPC_Array_Size(a));

    ck_assert_int_eq(i1, SOPC_Array_Get(a, int32_t, 0));
    ck_assert_int_eq(ix[0], SOPC_Array_Get(a, int32_t, 1));
    ck_assert_int_eq(ix[1], SOPC_Array_Get(a, int32_t, 2));
    ck_assert_int_eq(ix[0], *((int32_t*) SOPC_Array_Get_Ptr(a, 1)));

    SOPC_Array_Delete(a);
}
END_TEST

static int char_compare_func(const void* a, const void* b)
{
    return *((const char*) a) - *((const char*) b);
}

START_TEST(test_array_sort)
{
    SOPC_Array* a = SOPC_Array_Create(sizeof(char), 4, NULL);
    ck_assert_ptr_nonnull(a);

    char values[] = {'h', 'e', 'l', 'l', 'o'};
    ck_assert(SOPC_Array_Append_Values(a, values, 5));
    SOPC_Array_Sort(a, char_compare_func);

    ck_assert_int_eq('e', SOPC_Array_Get(a, char, 0));
    ck_assert_int_eq('h', SOPC_Array_Get(a, char, 1));
    ck_assert_int_eq('l', SOPC_Array_Get(a, char, 2));
    ck_assert_int_eq('l', SOPC_Array_Get(a, char, 3));
    ck_assert_int_eq('o', SOPC_Array_Get(a, char, 4));

    SOPC_Array_Delete(a);
}
END_TEST

START_TEST(test_array_into_raw)
{
    SOPC_Array* a = SOPC_Array_Create(sizeof(char), 4, NULL);
    ck_assert_ptr_nonnull(a);

    char values[] = {'a', 'b', 'c', '\0'};
    ck_assert(SOPC_Array_Append_Values(a, values, 4));
    char* data = SOPC_Array_Into_Raw(a);

    ck_assert_str_eq("abc", data);
    SOPC_Free(data);

    a = SOPC_Array_Create(sizeof(char), 4, NULL);
    ck_assert_ptr_nonnull(a);
    data = SOPC_Array_Into_Raw(a);

    ck_assert_ptr_null(data);
}
END_TEST

Suite* tests_make_suite_array(void)
{
    Suite* s;
    TCase* tc_array;

    s = suite_create("Array tests");
    tc_array = tcase_create("Array");

    tcase_add_test(tc_array, test_array_create);
    tcase_add_test(tc_array, test_array_free_func);
    tcase_add_test(tc_array, test_array_append_get);
    tcase_add_test(tc_array, test_array_sort);
    tcase_add_test(tc_array, test_array_into_raw);
    suite_add_tcase(s, tc_array);

    return s;
}
