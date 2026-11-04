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
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

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

START_TEST(test_event_handler_create_prioritized)
{
    SOPC_Looper* looper = SOPC_Looper_CreatePrioritized("test_prio", 0, -1);
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* handler = SOPC_EventHandler_Create(looper, test_callback);
    ck_assert_ptr_nonnull(handler);

    SOPC_EventHandler_Post(handler, 3, 3, 0x03, 3);
    expect_event(handler, 3, 3, 0x03, 3);

    SOPC_Looper_Delete(looper);
}
END_TEST

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

static SOPC_TimeReference postTimeInCallback = 0;
static SOPC_TimeReference processingTimeInCallback = 0;
static int32_t postTimeCallbackCalled = 0;

static void post_time_callback(SOPC_EventHandler* handler,
                               int32_t event,
                               uint32_t eltId,
                               uintptr_t params,
                               uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(eltId);
    SOPC_UNUSED_ARG(params);
    SOPC_UNUSED_ARG(auxParam);
    postTimeInCallback = SOPC_EventHandler_GetCurrentEventPostTime(handler);
    processingTimeInCallback = SOPC_TimeReference_GetCurrent();
    SOPC_Atomic_Int_Set(&postTimeCallbackCalled, 1);
}

START_TEST(test_event_handler_post_time)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test_post_time");
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* h1 = SOPC_EventHandler_Create(looper, post_time_callback);
    ck_assert_ptr_nonnull(h1);
    /* No event processed */
    ck_assert_uint_eq(0, SOPC_EventHandler_GetCurrentEventPostTime(h1));
    ck_assert_uint_eq(0, SOPC_EventHandler_GetCurrentEventPostTime(NULL));

    SOPC_TimeReference beforePost = SOPC_TimeReference_GetCurrent();
    ck_assert(SOPC_STATUS_OK == SOPC_EventHandler_Post(h1, 0, 0, 0, 0));
    SOPC_TimeReference afterPost = SOPC_TimeReference_GetCurrent();
    ck_assert(wait_value(&postTimeCallbackCalled, 1));

    /* Post time recorded at post and available during callback */
    ck_assert(postTimeInCallback >= beforePost);
    ck_assert(postTimeInCallback <= afterPost);
    ck_assert(postTimeInCallback <= processingTimeInCallback);

    SOPC_Looper_Delete(looper);
}
END_TEST

/* Events 10 to 19 are of category 0, 20 to 29 of category 1, the other ones are not counted */
static int32_t category_by_event(int32_t event, uint32_t eltId, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(eltId);
    SOPC_UNUSED_ARG(params);
    SOPC_UNUSED_ARG(auxParam);
    if (event >= 10 && event < 20)
    {
        return 0;
    }
    if (event >= 20 && event < 30)
    {
        return 1;
    }
    return SOPC_EVENT_HANDLER_NO_CATEGORY;
}

/* Pending events of category 0 at the treatment of the event (index = event code) */
#define NB_RECORDED_EVENTS 32
static uint32_t pendingCat0AtTreatment[NB_RECORDED_EVENTS];

static void counting_callback(SOPC_EventHandler* handler,
                              int32_t event,
                              uint32_t eltId,
                              uintptr_t params,
                              uintptr_t auxParam)
{
    if (event >= 0 && event < NB_RECORDED_EVENTS)
    {
        pendingCat0AtTreatment[event] = SOPC_EventHandler_Get_PendingCountByCategory(handler, 0);
    }
    test_callback(handler, event, eltId, params, auxParam);
}

START_TEST(test_event_handler_pending_count_by_category)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test events");
    ck_assert_ptr_nonnull(looper);

    ck_assert_ptr_null(SOPC_EventHandler_CreateWithCategory(looper, test_callback, NULL));
    SOPC_EventHandler* hCounted = SOPC_EventHandler_CreateWithCategory(looper, counting_callback, category_by_event);
    SOPC_EventHandler* hOther = SOPC_EventHandler_Create(looper, test_callback);
    ck_assert_ptr_nonnull(hCounted);
    ck_assert_ptr_nonnull(hOther);

    // Block the looper thread on a first event so that the next posted events stay pending
    SOPC_Atomic_Int_Set(&block_queue, 1);
    SOPC_EventHandler_Post(hCounted, 0, 0, 0, 0);
    expect_event(hCounted, 0, 0, 0, 0);

    SOPC_EventHandler_Post(hCounted, 10, 0, 0, 0);
    SOPC_EventHandler_Post(hCounted, 11, 0, 0, 0);
    SOPC_EventHandler_PostAsNext(hCounted, 12, 0, 0, 0);
    SOPC_EventHandler_Post(hCounted, 20, 0, 0, 0);
    SOPC_EventHandler_Post(hCounted, 1, 0, 0, 0);
    // Not counted: handler without category function
    SOPC_EventHandler_Post(hOther, 13, 0, 0, 0);

    // Counters are shared by the handlers of the looper
    ck_assert_uint_eq(3, SOPC_EventHandler_Get_PendingCountByCategory(hCounted, 0));
    ck_assert_uint_eq(1, SOPC_EventHandler_Get_PendingCountByCategory(hOther, 1));
    ck_assert_uint_eq(0, SOPC_EventHandler_Get_PendingCountByCategory(hCounted, 2));
    ck_assert_uint_eq(0, SOPC_EventHandler_Get_PendingCountByCategory(hCounted, -1));
    ck_assert_uint_eq(0, SOPC_EventHandler_Get_PendingCountByCategory(hCounted, SOPC_EVENT_HANDLER_NB_CATEGORIES));
    ck_assert_uint_eq(0, SOPC_EventHandler_Get_PendingCountByCategory(NULL, 0));
    SOPC_Atomic_Int_Set(&block_queue, 0);

    expect_event(hCounted, 12, 0, 0, 0);
    expect_event(hCounted, 10, 0, 0, 0);
    expect_event(hCounted, 11, 0, 0, 0);
    expect_event(hCounted, 20, 0, 0, 0);
    expect_event(hCounted, 1, 0, 0, 0);
    expect_event(hOther, 13, 0, 0, 0);

    // An event is no longer pending at its treatment
    ck_assert_uint_eq(2, pendingCat0AtTreatment[12]);
    ck_assert_uint_eq(1, pendingCat0AtTreatment[10]);
    ck_assert_uint_eq(0, pendingCat0AtTreatment[11]);
    ck_assert_uint_eq(0, SOPC_EventHandler_Get_PendingCountByCategory(hCounted, 0));
    ck_assert_uint_eq(0, SOPC_EventHandler_Get_PendingCountByCategory(hCounted, 1));

    SOPC_Looper_Delete(looper);
}
END_TEST

START_TEST(test_looper_stop)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test_stop");
    ck_assert_ptr_nonnull(looper);
    SOPC_EventHandler* handler = SOPC_EventHandler_Create(looper, test_callback);
    ck_assert_ptr_nonnull(handler);

    // Events posted before the stop are treated
    SOPC_EventHandler_Post(handler, 1, 1, 0x01, 1);
    SOPC_EventHandler_Post(handler, 2, 2, 0x02, 2);
    SOPC_Looper_Stop(looper);
    ck_assert_uint_eq(2, SOPC_AsyncQueue_GetSize(queue));
    expect_event(handler, 1, 1, 0x01, 1);
    expect_event(handler, 2, 2, 0x02, 2);

    // An event posted after the stop is accepted but never treated, handlers remain valid
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_EventHandler_Post(handler, 3, 3, 0x03, 3));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_EventHandler_PostAsNext(handler, 4, 4, 0x04, 4));
    SOPC_Looper_Stop(looper);
    ck_assert_uint_eq(2, SOPC_EventHandler_Get_QueueSize(handler));
    ck_assert_uint_eq(0, SOPC_AsyncQueue_GetSize(queue));

    // Untreated events are freed
    SOPC_Looper_Delete(looper);
}
END_TEST

static SOPC_EventHandler* chainHandler = NULL;

// Posts event 2 once the treatment of event 1 is unblocked
static void chain_callback(SOPC_EventHandler* handler,
                           int32_t event,
                           uint32_t eltId,
                           uintptr_t params,
                           uintptr_t auxParam)
{
    test_callback(handler, event, eltId, params, auxParam);
    if (1 == event)
    {
        ck_assert_int_eq(SOPC_STATUS_OK, SOPC_EventHandler_Post(chainHandler, 2, 2, 0x02, 2));
    }
}

static void* stop_looper(void* looper)
{
    SOPC_Looper_Stop((SOPC_Looper*) looper);
    return NULL;
}

START_TEST(test_looper_stop_post_from_callback)
{
    SOPC_Looper* looper = SOPC_Looper_Create("test_stop_chain");
    ck_assert_ptr_nonnull(looper);
    chainHandler = SOPC_EventHandler_Create(looper, chain_callback);
    ck_assert_ptr_nonnull(chainHandler);

    // Block the treatment of event 1
    SOPC_Atomic_Int_Set(&block_queue, 1);
    SOPC_EventHandler_Post(chainHandler, 1, 1, 0x01, 1);
    expect_event(chainHandler, 1, 1, 0x01, 1);

    // Stop the looper during the treatment: wait for the stop request to be queued
    SOPC_Thread stopThread;
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Create(&stopThread, stop_looper, looper, "test_stop"));
    // Event 1 already dequeued: the only possible queued element is the poison pill enqueued by SOPC_Looper_Stop
    while (0 == SOPC_EventHandler_Get_QueueSize(chainHandler))
    {
        SOPC_Sleep(10);
    }
    SOPC_Atomic_Int_Set(&block_queue, 0);
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Join(&stopThread));

    // Event 2 posted after the stop request is never treated
    ck_assert_uint_eq(1, SOPC_EventHandler_Get_QueueSize(chainHandler));
    ck_assert_uint_eq(0, SOPC_AsyncQueue_GetSize(queue));

    SOPC_Looper_Delete(looper);
    chainHandler = NULL;
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
    tcase_add_test(c, test_event_handler_create_prioritized);
    tcase_add_test(c, test_event_handler_post);
    tcase_add_test(c, test_event_handler_post_as_next);
    tcase_add_test(c, test_event_handler_post_time);
    tcase_add_test(c, test_event_handler_pending_count_by_category);
    tcase_add_test(c, test_looper_stop);
    tcase_add_test(c, test_looper_stop_post_from_callback);
    suite_add_tcase(s, c);

    return s;
}
