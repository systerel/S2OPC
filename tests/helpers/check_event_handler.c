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

#include <assert.h>
#include <check.h>

#include "check_helpers.h"
#include "sopc_async_queue.h"
#include "sopc_atomic.h"
#include "sopc_event_handler.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"

struct Event
{
    SOPC_EventHandler* handler;
    int32_t event;
    uint32_t id;
    uintptr_t params;
    uintptr_t auxParam;
};

static SOPC_AsyncQueue* queue = NULL;

static int32_t block_queue = 0;

static void test_callback(SOPC_EventHandler* handler,
                          int32_t event,
                          uint32_t eltId,
                          uintptr_t params,
                          uintptr_t auxParam)
{
    struct Event* ev = SOPC_Calloc(1, sizeof(struct Event));
    assert(ev != NULL);

    ev->handler = handler;
    ev->event = event;
    ev->id = eltId;
    ev->params = params;
    ev->auxParam = auxParam;

    ck_assert(SOPC_AsyncQueue_BlockingEnqueue(queue, ev) == SOPC_STATUS_OK);

    while (SOPC_Atomic_Int_Get(&block_queue) == 1)
    {
        SOPC_Sleep(50);
    }
}

static void expect_event(SOPC_EventHandler* handler,
                         int32_t event,
                         uint32_t eltId,
                         uintptr_t params,
                         uintptr_t auxParam)
{
    struct Event* ev;
    ck_assert(SOPC_AsyncQueue_BlockingDequeue(queue, (void**) &ev) == SOPC_STATUS_OK);

    ck_assert_ptr_eq(handler, ev->handler);
    ck_assert_int_eq(event, ev->event);
    ck_assert_uint_eq(eltId, ev->id);
    ck_assert_uint_eq(params, ev->params);
    ck_assert_uint_eq(auxParam, ev->auxParam);

    SOPC_Free(ev);
}

START_TEST(test_event_handler_empty_looper)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test_events");
    ck_assert_ptr_nonnull(looper);
    SOPC_Looper_Delete(looper);
}
END_TEST

START_TEST(test_event_handler_post)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test_events");
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* h1 = SOPC_EventHandler_Create(looper, test_callback);
    SOPC_EventHandler* h2 = SOPC_EventHandler_Create(looper, test_callback);

    SOPC_EventHandler_Post(h1, 0, 0, 0, 0);
    SOPC_EventHandler_Post(h2, 1, 1, 0x01, 1);
    SOPC_EventHandler_Post(h1, 2, 2, 0x02, 2);

    expect_event(h1, 0, 0, 0, 0);
    expect_event(h2, 1, 1, 0x01, 1);
    expect_event(h1, 2, 2, 0x02, 2);

    SOPC_Looper_Delete(looper);
}
END_TEST

START_TEST(test_event_handler_post_as_next)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test events");
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* h1 = SOPC_EventHandler_Create(looper, test_callback);

    SOPC_Atomic_Int_Set(&block_queue, 1);
    SOPC_EventHandler_Post(h1, 0, 0, 0, 0);
    expect_event(h1, 0, 0, 0, 0);

    SOPC_EventHandler_Post(h1, 1, 1, 0x01, 1);
    SOPC_EventHandler_PostAsNext(h1, 2, 2, 0x02, 2);
    SOPC_Atomic_Int_Set(&block_queue, 0);

    expect_event(h1, 2, 2, 0x02, 2);
    expect_event(h1, 1, 1, 0x01, 1);

    SOPC_Looper_Delete(looper);
}
END_TEST

static void test_setup(void)
{
    ck_assert(SOPC_AsyncQueue_Init(&queue, "") == SOPC_STATUS_OK);
}

static void test_teardown(void)
{
    SOPC_AsyncQueue_Free(&queue);
}

Suite* tests_make_suite_event_handler(void)
{
    Suite* s = suite_create("Event handler");
    TCase* c;

    c = tcase_create("Event handler");
    tcase_add_checked_fixture(c, test_setup, test_teardown);
    tcase_add_test(c, test_event_handler_empty_looper);
    tcase_add_test(c, test_event_handler_post);
    tcase_add_test(c, test_event_handler_post_as_next);
    suite_add_tcase(s, c);

    return s;
}
