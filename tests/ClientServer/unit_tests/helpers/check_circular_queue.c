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
 * \file check_circular_queue.c
 * \brief Unit tests for SOPC_CircularQueue.
 */

#include <check.h>
#include <stdint.h>

#include "check_helpers.h"

#include "sopc_circular_queue.h"

START_TEST(test_cq_create_rejects_zero_capacity)
{
    ck_assert_ptr_null(SOPC_CircularQueue_Create(0));
}
END_TEST

START_TEST(test_cq_append_fill_and_reject)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(3);
    ck_assert_ptr_nonnull(queue);
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetCapacity(queue));
    ck_assert_uint_eq(0, SOPC_CircularQueue_GetLength(queue));

    ck_assert_uint_eq(1, SOPC_CircularQueue_Append(queue, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueue_Append(queue, 2));
    ck_assert_uint_eq(3, SOPC_CircularQueue_Append(queue, 3));
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetLength(queue));

    /* Full: Append must fail; value 0 is also invalid */
    ck_assert_uint_eq(0, SOPC_CircularQueue_Append(queue, 4));
    ck_assert_uint_eq(0, SOPC_CircularQueue_Append(queue, 0));
    ck_assert_uint_eq(1, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetLast(queue));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

START_TEST(test_cq_fifo_pop_head)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(4);
    ck_assert_ptr_nonnull(queue);

    ck_assert_uint_eq(10, SOPC_CircularQueue_Append(queue, 10));
    ck_assert_uint_eq(20, SOPC_CircularQueue_Append(queue, 20));
    ck_assert_uint_eq(30, SOPC_CircularQueue_Append(queue, 30));

    ck_assert_uint_eq(10, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(20, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(30, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(30, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(0, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(0, SOPC_CircularQueue_GetLength(queue));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

START_TEST(test_cq_prepend_and_pop_last)
{
    /* Discard-newest pattern: Append then PopLast; Prepend overflow at head */
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(3);
    ck_assert_ptr_nonnull(queue);

    ck_assert_uint_eq(1, SOPC_CircularQueue_Append(queue, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueue_Append(queue, 2));
    ck_assert_uint_eq(3, SOPC_CircularQueue_Append(queue, 3));

    ck_assert_uint_eq(3, SOPC_CircularQueue_PopLast(queue));
    ck_assert_uint_eq(2, SOPC_CircularQueue_GetLast(queue));
    ck_assert_uint_eq(1, SOPC_CircularQueue_GetHead(queue));

    ck_assert_uint_eq(99, SOPC_CircularQueue_Prepend(queue, 99));
    ck_assert_uint_eq(99, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(2, SOPC_CircularQueue_GetLast(queue));
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetLength(queue));

    /* Full after prepend */
    ck_assert_uint_eq(0, SOPC_CircularQueue_Prepend(queue, 100));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

static uint32_t g_apply_count;
static uintptr_t g_apply_seen[8];

static void test_apply_collect(uintptr_t val)
{
    if (g_apply_count < 8)
    {
        g_apply_seen[g_apply_count] = val;
    }
    g_apply_count++;
}

START_TEST(test_cq_apply_order_head_to_tail)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(4);
    ck_assert_ptr_nonnull(queue);

    /* Shift head away from 0 so Apply walks a wrapped ring */
    ck_assert_uint_eq(1, SOPC_CircularQueue_Append(queue, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueue_Append(queue, 2));
    ck_assert_uint_eq(1, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(3, SOPC_CircularQueue_Append(queue, 3));
    ck_assert_uint_eq(4, SOPC_CircularQueue_Append(queue, 4));
    ck_assert_uint_eq(5, SOPC_CircularQueue_Prepend(queue, 5));

    g_apply_count = 0;
    SOPC_CircularQueue_Apply(queue, test_apply_collect);
    ck_assert_uint_eq(4, g_apply_count);
    ck_assert_uint_eq(5, g_apply_seen[0]);
    ck_assert_uint_eq(2, g_apply_seen[1]);
    ck_assert_uint_eq(3, g_apply_seen[2]);
    ck_assert_uint_eq(4, g_apply_seen[3]);

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

START_TEST(test_cq_set_capacity_shrink_grow)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(4);
    ck_assert_ptr_nonnull(queue);

    ck_assert_uint_eq(1, SOPC_CircularQueue_Append(queue, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueue_Append(queue, 2));
    ck_assert_uint_eq(3, SOPC_CircularQueue_Append(queue, 3));

    /* Cannot shrink below length */
    ck_assert(!SOPC_CircularQueue_SetCapacity(queue, 2));
    ck_assert(!SOPC_CircularQueue_SetCapacity(queue, 0));

    ck_assert(SOPC_CircularQueue_SetCapacity(queue, 3));
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetCapacity(queue));
    ck_assert_uint_eq(0, SOPC_CircularQueue_Append(queue, 4));

    /* Move the head away from 0 and wrap the ring before growing: in
     * production, growth typically happens after Pop/Append churn rather
     * than right after Create, so the head is rarely still at index 0. */
    ck_assert_uint_eq(1, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(2, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(3, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(10, SOPC_CircularQueue_Append(queue, 10));
    ck_assert_uint_eq(20, SOPC_CircularQueue_Append(queue, 20));
    ck_assert_uint_eq(30, SOPC_CircularQueue_Append(queue, 30));
    /* allocatedCap is still 4 (from Create); head is now index 3,
     * so this fill wraps: idx3=10, idx0=20, idx1=30 */
    ck_assert_uint_eq(0, SOPC_CircularQueue_Append(queue, 40));
    ck_assert_uint_eq(10, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(30, SOPC_CircularQueue_GetLast(queue));

    /* Grow beyond original allocation while head is wrapped and non-zero */
    ck_assert(SOPC_CircularQueue_SetCapacity(queue, 6));
    ck_assert_uint_eq(6, SOPC_CircularQueue_GetCapacity(queue));
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetLength(queue));
    ck_assert_uint_eq(10, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(30, SOPC_CircularQueue_GetLast(queue));

    ck_assert_uint_eq(40, SOPC_CircularQueue_Append(queue, 40));
    ck_assert_uint_eq(50, SOPC_CircularQueue_Append(queue, 50));
    ck_assert_uint_eq(60, SOPC_CircularQueue_Append(queue, 60));
    ck_assert_uint_eq(6, SOPC_CircularQueue_GetLength(queue));
    ck_assert_uint_eq(10, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(60, SOPC_CircularQueue_GetLast(queue));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

START_TEST(test_cq_clear_then_reuse)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(2);
    ck_assert_ptr_nonnull(queue);

    ck_assert_uint_eq(7, SOPC_CircularQueue_Append(queue, 7));
    ck_assert_uint_eq(8, SOPC_CircularQueue_Append(queue, 8));
    SOPC_CircularQueue_Clear(queue);
    ck_assert_uint_eq(0, SOPC_CircularQueue_GetLength(queue));
    ck_assert_uint_eq(0, SOPC_CircularQueue_GetHead(queue));

    ck_assert_uint_eq(9, SOPC_CircularQueue_Append(queue, 9));
    ck_assert_uint_eq(9, SOPC_CircularQueue_GetHead(queue));
    ck_assert_uint_eq(1, SOPC_CircularQueue_GetLength(queue));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

START_TEST(test_cq_iterator_head_to_tail)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(4);
    ck_assert_ptr_nonnull(queue);

    /* Wrap the ring so iteration is not a contiguous linear scan. */
    ck_assert_uint_eq(1, SOPC_CircularQueue_Append(queue, 1));
    ck_assert_uint_eq(2, SOPC_CircularQueue_Append(queue, 2));
    ck_assert_uint_eq(1, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(3, SOPC_CircularQueue_Append(queue, 3));
    ck_assert_uint_eq(4, SOPC_CircularQueue_Append(queue, 4));
    ck_assert_uint_eq(5, SOPC_CircularQueue_Prepend(queue, 5));

    SOPC_CircularQueueIterator it = SOPC_CircularQueue_GetIterator(queue);
    uint32_t index = UINT32_MAX;
    ck_assert(SOPC_CircularQueue_HasNext(&it));
    ck_assert_uint_eq(5, SOPC_CircularQueue_NextWithIndex(&it, &index));
    ck_assert_uint_eq(0, index);
    ck_assert_uint_eq(2, SOPC_CircularQueue_NextWithIndex(&it, &index));
    ck_assert_uint_eq(1, index);
    ck_assert_uint_eq(3, SOPC_CircularQueue_Next(&it));
    ck_assert_uint_eq(4, SOPC_CircularQueue_Next(&it));
    ck_assert(!SOPC_CircularQueue_HasNext(&it));
    ck_assert_uint_eq(0, SOPC_CircularQueue_Next(&it));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

START_TEST(test_cq_remove_at)
{
    SOPC_CircularQueue* queue = SOPC_CircularQueue_Create(5);
    ck_assert_ptr_nonnull(queue);

    ck_assert_uint_eq(10, SOPC_CircularQueue_Append(queue, 10));
    ck_assert_uint_eq(20, SOPC_CircularQueue_Append(queue, 20));
    ck_assert_uint_eq(30, SOPC_CircularQueue_Append(queue, 30));
    ck_assert_uint_eq(40, SOPC_CircularQueue_Append(queue, 40));

    /* Remove middle element by logical index; order of remaining must be preserved. */
    ck_assert_uint_eq(20, SOPC_CircularQueue_RemoveAt(queue, 1));
    ck_assert_uint_eq(3, SOPC_CircularQueue_GetLength(queue));
    ck_assert_uint_eq(10, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(30, SOPC_CircularQueue_PopHead(queue));
    ck_assert_uint_eq(40, SOPC_CircularQueue_PopHead(queue));

    /* Out of range */
    ck_assert_uint_eq(0, SOPC_CircularQueue_RemoveAt(queue, 0));
    ck_assert_uint_eq(0, SOPC_CircularQueue_RemoveAt(NULL, 0));

    SOPC_CircularQueue_Delete(queue);
}
END_TEST

Suite* tests_make_suite_circular_queue(void)
{
    Suite* s = suite_create("Circular queue");
    TCase* tc = tcase_create("CircularQueue");

    tcase_add_test(tc, test_cq_create_rejects_zero_capacity);
    tcase_add_test(tc, test_cq_append_fill_and_reject);
    tcase_add_test(tc, test_cq_fifo_pop_head);
    tcase_add_test(tc, test_cq_prepend_and_pop_last);
    tcase_add_test(tc, test_cq_apply_order_head_to_tail);
    tcase_add_test(tc, test_cq_set_capacity_shrink_grow);
    tcase_add_test(tc, test_cq_clear_then_reuse);
    tcase_add_test(tc, test_cq_iterator_head_to_tail);
    tcase_add_test(tc, test_cq_remove_at);

    suite_add_tcase(s, tc);
    return s;
}
