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

#include <check.h>

#include "check_helpers.h"

#include "sopc_static_pool.h"

/* The pointer member guarantees the sizeof(void*) alignment required by SOPC_StaticPool_Init
 * for static buffers declared as test_block_t arrays. */
typedef struct
{
    int a;
    int b;
    void* p;
} test_block_t;

START_TEST(test_static_pool_init_alloc_free)
{
    static test_block_t buffer[4];
    SOPC_StaticPool pool;
    SOPC_StaticPool_Init(&pool, buffer, sizeof(test_block_t), 4);

    void* b1 = SOPC_StaticPool_Alloc(&pool);
    void* b2 = SOPC_StaticPool_Alloc(&pool);
    ck_assert_ptr_nonnull(b1);
    ck_assert_ptr_nonnull(b2);
    ck_assert_ptr_ne(b1, b2);
    ck_assert_uint_eq(0, SOPC_StaticPool_GetNbFallbackAllocsStat(&pool));

    /* Keep b1 address as an integer: b1 shall not be used once freed */
    const uintptr_t b1Addr = (uintptr_t) b1;
    SOPC_StaticPool_Free(&pool, b1);
    void* b3 = SOPC_StaticPool_Alloc(&pool);
    /* LIFO: the block just freed (b1) must be the next one returned */
    ck_assert_uint_eq(b1Addr, (uintptr_t) b3);

    SOPC_StaticPool_Free(&pool, b2);
    SOPC_StaticPool_Free(&pool, b3);
}
END_TEST

START_TEST(test_static_pool_fallback_on_exhaustion)
{
    static test_block_t buffer[2];
    SOPC_StaticPool pool;
    SOPC_StaticPool_Init(&pool, buffer, sizeof(test_block_t), 2);

    void* b1 = SOPC_StaticPool_Alloc(&pool);
    void* b2 = SOPC_StaticPool_Alloc(&pool);
    ck_assert_ptr_nonnull(b1);
    ck_assert_ptr_nonnull(b2);
    ck_assert_uint_eq(0, SOPC_StaticPool_GetNbFallbackAllocsStat(&pool));

    /* Pool exhausted: this third block must come from the malloc fallback */
    void* b3 = SOPC_StaticPool_Alloc(&pool);
    ck_assert_ptr_nonnull(b3);
    ck_assert_uint_eq(1, SOPC_StaticPool_GetNbFallbackAllocsStat(&pool));
    /* b3 must not be inside [buffer, buffer + capacity) */
    ck_assert(((uint8_t*) b3 < (uint8_t*) buffer) || ((uint8_t*) b3 >= (uint8_t*) buffer + (2 * sizeof(test_block_t))));

    SOPC_StaticPool_Free(&pool, b1);
    SOPC_StaticPool_Free(&pool, b2);
    /* Freeing the fallback block must not corrupt the freelist (still only 2
     * pool slots available afterwards, not 3) */
    SOPC_StaticPool_Free(&pool, b3);

    void* b4 = SOPC_StaticPool_Alloc(&pool);
    void* b5 = SOPC_StaticPool_Alloc(&pool);
    ck_assert(b4 == b1 || b4 == b2);
    ck_assert(b5 == b1 || b5 == b2);
    ck_assert_ptr_ne(b4, b5);
    ck_assert_uint_eq(1, SOPC_StaticPool_GetNbFallbackAllocsStat(&pool));

    SOPC_StaticPool_Free(&pool, b4);
    SOPC_StaticPool_Free(&pool, b5);
}
END_TEST

START_TEST(test_static_pool_alloc_is_zeroed)
{
    static test_block_t buffer[2];
    SOPC_StaticPool pool;
    SOPC_StaticPool_Init(&pool, buffer, sizeof(test_block_t), 2);

    test_block_t* b1 = (test_block_t*) SOPC_StaticPool_Alloc(&pool);
    b1->a = 42;
    b1->b = 43;
    /* Keep b1 address as an integer: b1 shall not be used once freed */
    const uintptr_t b1Addr = (uintptr_t) b1;
    SOPC_StaticPool_Free(&pool, b1);

    /* Re-allocating the same physical block must yield zeroed content, not
     * the previous 42/43 values (this is the LIFO block, so it is b1 again) */
    test_block_t* b2 = (test_block_t*) SOPC_StaticPool_Alloc(&pool);
    ck_assert_uint_eq(b1Addr, (uintptr_t) b2);
    ck_assert_int_eq(0, b2->a);
    ck_assert_int_eq(0, b2->b);

    SOPC_StaticPool_Free(&pool, b2);
}
END_TEST

START_TEST(test_static_pool_zero_capacity_heap_only)
{
    /* No backing buffer: every block comes from the malloc fallback */
    SOPC_StaticPool pool;
    SOPC_StaticPool_Init(&pool, NULL, sizeof(test_block_t), 0);

    test_block_t* b1 = (test_block_t*) SOPC_StaticPool_Alloc(&pool);
    test_block_t* b2 = (test_block_t*) SOPC_StaticPool_Alloc(&pool);
    ck_assert_ptr_nonnull(b1);
    ck_assert_ptr_nonnull(b2);
    ck_assert_ptr_ne(b1, b2);
    ck_assert_int_eq(0, b1->a);
    ck_assert_int_eq(0, b1->b);
    ck_assert_uint_eq(2, SOPC_StaticPool_GetNbFallbackAllocsStat(&pool));

    SOPC_StaticPool_Free(&pool, b1);
    SOPC_StaticPool_Free(&pool, b2);
    SOPC_StaticPool_Free(&pool, NULL); /* no-op */
}
END_TEST

START_TEST(test_static_pool_init_null_buffer_with_capacity_aborts)
{
    /* A NULL buffer is only allowed with a 0 capacity: SOPC_ASSERT (raises SIGABRT). */
    SOPC_StaticPool pool;
    SOPC_StaticPool_Init(&pool, NULL, sizeof(test_block_t), 2);
}
END_TEST

START_TEST(test_static_pool_free_misaligned_pointer_aborts)
{
    /* Properly aligned buffer: Init shall succeed so that only Free can abort */
    static test_block_t buffer[2];
    SOPC_StaticPool pool;
    SOPC_StaticPool_Init(&pool, buffer, sizeof(test_block_t), 2);

    /* buffer + 1 byte is inside the pool's range but not on a block boundary:
     * SOPC_StaticPool_Free must SOPC_ASSERT (raises SIGABRT) before any free.
     * Intentional invalid pointer for this negative test. */
    /* coverity[bad_free] */
    SOPC_StaticPool_Free(&pool, (uint8_t*) buffer + 1);
}
END_TEST

Suite* tests_make_suite_static_pool(SRunner* sr)
{
    Suite* s = suite_create("Static pool allocator");
    TCase* tc = tcase_create("StaticPool");

    tcase_add_test(tc, test_static_pool_init_alloc_free);
    tcase_add_test(tc, test_static_pool_fallback_on_exhaustion);
    tcase_add_test(tc, test_static_pool_alloc_is_zeroed);
    tcase_add_test(tc, test_static_pool_zero_capacity_heap_only);

    if (srunner_fork_status(sr) != CK_NOFORK)
    {
        // 6 == SIGABRT
        tcase_add_test_raise_signal(tc, test_static_pool_free_misaligned_pointer_aborts, 6);
        tcase_add_test_raise_signal(tc, test_static_pool_init_null_buffer_with_capacity_aborts, 6);
    }

    suite_add_tcase(s, tc);
    return s;
}
