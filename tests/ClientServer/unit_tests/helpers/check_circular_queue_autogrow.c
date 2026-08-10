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

/**
 * \file check_circular_queue_autogrow.c
 * \brief Unit tests for SOPC_CircularQueueAutoGrow.
 */

#include <check.h>
#include <stdint.h>

#include "check_helpers.h"

#include "sopc_circular_queue_autogrow.h"

START_TEST(test_cqag_create_rejects_zero_capacity)
{
    ck_assert_ptr_null(SOPC_CircularQueueAutoGrow_Create(0));
}
END_TEST

START_TEST(test_cqag_append_within_capacity_no_grow)
{
    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(4);
    ck_assert_ptr_nonnull(handle);

    ck_assert_uint_eq(1, SOPC_CircularQueueAutoGrow_Append(handle, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueueAutoGrow_Append(handle, 2));
    ck_assert_uint_eq(2, SOPC_CircularQueueAutoGrow_GetLength(handle));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

START_TEST(test_cqag_append_grows_transparently_never_fails_full)
{
    /* Start at capacity 1 and push far beyond it: never rejected for being full. */
    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(1);
    ck_assert_ptr_nonnull(handle);

    for (uintptr_t i = 1; i <= 50; i++)
    {
        ck_assert_uint_eq(i, SOPC_CircularQueueAutoGrow_Append(handle, i));
    }
    ck_assert_uint_eq(50, SOPC_CircularQueueAutoGrow_GetLength(handle));

    /* Content and order must be intact across every grow step. */
    SOPC_CircularQueueIterator it = SOPC_CircularQueueAutoGrow_GetIterator(handle);
    for (uintptr_t i = 1; i <= 50; i++)
    {
        ck_assert(SOPC_CircularQueue_HasNext(&it));
        ck_assert_uint_eq(i, SOPC_CircularQueue_Next(&it));
    }
    ck_assert(!SOPC_CircularQueue_HasNext(&it));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

START_TEST(test_cqag_append_doubles_capacity)
{
    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(1);
    ck_assert_ptr_nonnull(handle);

    /* Capacity only changes when an Append finds the queue full, and then doubles. */
    uint32_t expectedCapacity = 1;
    for (uintptr_t i = 1; i <= 9; i++)
    {
        if (i > expectedCapacity)
        {
            expectedCapacity *= 2;
        }
        ck_assert_uint_eq(i, SOPC_CircularQueueAutoGrow_Append(handle, i));
        ck_assert_uint_eq(expectedCapacity, SOPC_CircularQueue_GetCapacity(handle));
    }
    /* 1 -> 2 -> 4 -> 8 -> 16 */
    ck_assert_uint_eq(16, SOPC_CircularQueue_GetCapacity(handle));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

START_TEST(test_cqag_append_grows_after_remove)
{
    /* RemoveValue (RemoveAt-based) shifts the ring down but never moves head
     * the way PopHead would; the wrapped-ring grow path itself is already
     * covered at the SOPC_CircularQueue level (test_cq_set_capacity_shrink_grow).
     * This test only checks that a grow triggered right after a removal still
     * preserves content and order. */
    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(2);
    ck_assert_ptr_nonnull(handle);

    ck_assert_uint_eq(1, SOPC_CircularQueueAutoGrow_Append(handle, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueueAutoGrow_Append(handle, 2));
    ck_assert_uint_eq(1, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 1));
    ck_assert_uint_eq(3, SOPC_CircularQueueAutoGrow_Append(handle, 3)); /* still within capacity 2 */
    ck_assert_uint_eq(4, SOPC_CircularQueueAutoGrow_Append(handle, 4)); /* triggers grow (2 -> 4) */

    ck_assert_uint_eq(3, SOPC_CircularQueueAutoGrow_GetLength(handle));
    SOPC_CircularQueueIterator it = SOPC_CircularQueueAutoGrow_GetIterator(handle);
    ck_assert_uint_eq(2, SOPC_CircularQueue_Next(&it));
    ck_assert_uint_eq(3, SOPC_CircularQueue_Next(&it));
    ck_assert_uint_eq(4, SOPC_CircularQueue_Next(&it));
    ck_assert(!SOPC_CircularQueue_HasNext(&it));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

START_TEST(test_cqag_remove_value_middle_preserves_order)
{
    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(4);
    ck_assert_ptr_nonnull(handle);

    ck_assert_uint_eq(10, SOPC_CircularQueueAutoGrow_Append(handle, 10));
    ck_assert_uint_eq(20, SOPC_CircularQueueAutoGrow_Append(handle, 20));
    ck_assert_uint_eq(30, SOPC_CircularQueueAutoGrow_Append(handle, 30));

    ck_assert_uint_eq(20, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 20));
    ck_assert_uint_eq(2, SOPC_CircularQueueAutoGrow_GetLength(handle));

    /* Not found: neither an absent value nor a NULL handle crash or remove anything else. */
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 999));
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_RemoveValue(NULL, 10));
    ck_assert_uint_eq(2, SOPC_CircularQueueAutoGrow_GetLength(handle));

    SOPC_CircularQueueIterator it = SOPC_CircularQueueAutoGrow_GetIterator(handle);
    ck_assert_uint_eq(10, SOPC_CircularQueue_Next(&it));
    ck_assert_uint_eq(30, SOPC_CircularQueue_Next(&it));
    ck_assert(!SOPC_CircularQueue_HasNext(&it));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

START_TEST(test_cqag_remove_value_first_occurrence_only)
{
    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(4);
    ck_assert_ptr_nonnull(handle);

    ck_assert_uint_eq(10, SOPC_CircularQueueAutoGrow_Append(handle, 10));
    ck_assert_uint_eq(20, SOPC_CircularQueueAutoGrow_Append(handle, 20));
    ck_assert_uint_eq(10, SOPC_CircularQueueAutoGrow_Append(handle, 10));

    /* Only the oldest occurrence is removed */
    ck_assert_uint_eq(10, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 10));
    ck_assert_uint_eq(2, SOPC_CircularQueueAutoGrow_GetLength(handle));
    SOPC_CircularQueueIterator it = SOPC_CircularQueueAutoGrow_GetIterator(handle);
    ck_assert_uint_eq(20, SOPC_CircularQueue_Next(&it));
    ck_assert_uint_eq(10, SOPC_CircularQueue_Next(&it));
    ck_assert(!SOPC_CircularQueue_HasNext(&it));

    /* Second call removes the remaining occurrence, third finds nothing */
    ck_assert_uint_eq(10, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 10));
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 10));
    ck_assert_uint_eq(1, SOPC_CircularQueueAutoGrow_GetLength(handle));
    it = SOPC_CircularQueueAutoGrow_GetIterator(handle);
    ck_assert_uint_eq(20, SOPC_CircularQueue_Next(&it));
    ck_assert(!SOPC_CircularQueue_HasNext(&it));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

START_TEST(test_cqag_invalid_args)
{
    /* NULL queue */
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_Append(NULL, 1));
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_GetLength(NULL));
    SOPC_CircularQueueIterator it = SOPC_CircularQueueAutoGrow_GetIterator(NULL);
    ck_assert(!SOPC_CircularQueue_HasNext(&it));
    SOPC_CircularQueueAutoGrow_Delete(NULL); /* no crash */

    SOPC_CircularQueueAutoGrow* handle = SOPC_CircularQueueAutoGrow_Create(2);
    ck_assert_ptr_nonnull(handle);

    /* Remove from an empty queue */
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 1));

    /* Value 0 is rejected and never stored or removed */
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_Append(handle, 0));
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_GetLength(handle));
    ck_assert_uint_eq(1, SOPC_CircularQueueAutoGrow_Append(handle, 1));
    ck_assert_uint_eq(0, SOPC_CircularQueueAutoGrow_RemoveValue(handle, 0));
    ck_assert_uint_eq(1, SOPC_CircularQueueAutoGrow_GetLength(handle));

    SOPC_CircularQueueAutoGrow_Delete(handle);
}
END_TEST

Suite* tests_make_suite_circular_queue_autogrow(void)
{
    Suite* s = suite_create("Circular queue autogrow");
    TCase* tc = tcase_create("CircularQueueAutoGrow");

    tcase_add_test(tc, test_cqag_create_rejects_zero_capacity);
    tcase_add_test(tc, test_cqag_append_within_capacity_no_grow);
    tcase_add_test(tc, test_cqag_append_grows_transparently_never_fails_full);
    tcase_add_test(tc, test_cqag_append_doubles_capacity);
    tcase_add_test(tc, test_cqag_append_grows_after_remove);
    tcase_add_test(tc, test_cqag_remove_value_middle_preserves_order);
    tcase_add_test(tc, test_cqag_remove_value_first_occurrence_only);
    tcase_add_test(tc, test_cqag_invalid_args);

    suite_add_tcase(s, tc);
    return s;
}
