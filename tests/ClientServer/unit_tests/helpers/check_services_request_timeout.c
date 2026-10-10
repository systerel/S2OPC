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

/** \file
 *
 * \brief Tests of the server side request TimeoutHint evaluation (Services layer).
 */

#include <check.h>
#include <stdbool.h>

#include "check_helpers.h"

#include "message_in_bs.h"
#include "msg_subscription_publish_ack_bs.h"
#include "sopc_services_request_timeout.h"
#include "sopc_time_reference.h"
#include "sopc_types.h"

START_TEST(test_timeout_hint_rule)
{
    const SOPC_TimeReference rx = 100000;

    /* TimeoutHint 0 means no timeout */
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx, 0, 0));
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 1000000, 0, 0));
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 1000000, 0, 500));

    /* No margin: expired strictly after TimeoutHint elapsed */
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx, 1000, 0));
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 999, 1000, 0));
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 1000, 1000, 0));
    ck_assert(SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 1001, 1000, 0));

    /* Margin anticipates the expiration */
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 800, 1000, 200));
    ck_assert(SOPC_Services_IsRequestTimeoutHintExpired(rx, rx + 801, 1000, 200));
    /* Margin greater than TimeoutHint: always expired */
    ck_assert(SOPC_Services_IsRequestTimeoutHintExpired(rx, rx, 100, 200));

    /* Current time before reception time (unexpected): not expired */
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(rx, rx - 1, 1000, 0));

    /* Extreme values: no overflow */
    ck_assert(SOPC_Services_IsRequestTimeoutHintExpired(0, UINT64_MAX, UINT32_MAX, UINT32_MAX));
    ck_assert(!SOPC_Services_IsRequestTimeoutHintExpired(UINT64_MAX, UINT64_MAX, UINT32_MAX, 0));
    ck_assert(SOPC_Services_IsRequestTimeoutHintExpired(UINT64_MAX - 1, UINT64_MAX, 1, UINT32_MAX));
}
END_TEST

START_TEST(test_current_msg_reception_time)
{
    SOPC_TimeReference rx = 0;

    SOPC_Services_ResetCurrentMsgReceptionTime();
    ck_assert(!SOPC_Services_GetCurrentMsgReceptionTime(&rx));
    ck_assert(!SOPC_Services_GetCurrentMsgReceptionTime(NULL));

    SOPC_Services_SetCurrentMsgReceptionTime(42);
    ck_assert(!SOPC_Services_GetCurrentMsgReceptionTime(NULL));
    ck_assert(SOPC_Services_GetCurrentMsgReceptionTime(&rx));
    ck_assert_uint_eq(42, rx);

    SOPC_Services_ResetCurrentMsgReceptionTime();
    ck_assert(!SOPC_Services_GetCurrentMsgReceptionTime(&rx));
}
END_TEST

START_TEST(test_message_in_bs_timeout_expired)
{
    OpcUa_RequestHeader header;
    OpcUa_RequestHeader_Initialize(&header);
    t_bool expired = true;

    /* No message currently treated: never expired */
    SOPC_Services_ResetCurrentMsgReceptionTime();
    header.TimeoutHint = 1;
    message_in_bs__server_is_msg_req_header_timeout_expired(&header, &expired);
    ck_assert(!expired);

    /* Received a long time ago with a non-zero TimeoutHint: expired */
    SOPC_TimeReference now = SOPC_TimeReference_GetCurrent();
    SOPC_Services_SetCurrentMsgReceptionTime(now - 10000);
    header.TimeoutHint = 5000;
    message_in_bs__server_is_msg_req_header_timeout_expired(&header, &expired);
    ck_assert(expired);

    /* Same reception time but TimeoutHint = 0 (no timeout): not expired */
    header.TimeoutHint = 0;
    message_in_bs__server_is_msg_req_header_timeout_expired(&header, &expired);
    ck_assert(!expired);

    /* Just received with a large TimeoutHint: not expired */
    SOPC_Services_SetCurrentMsgReceptionTime(SOPC_TimeReference_GetCurrent());
    header.TimeoutHint = 60000;
    message_in_bs__server_is_msg_req_header_timeout_expired(&header, &expired);
    ck_assert(!expired);

    SOPC_Services_ResetCurrentMsgReceptionTime();
    OpcUa_RequestHeader_Clear(&header);
}
END_TEST

START_TEST(test_publish_expiration_time)
{
    OpcUa_RequestHeader header;
    OpcUa_RequestHeader_Initialize(&header);
    constants__t_timeref_i expiration = 0;

    /* Reception time available: expiration = reception time + TimeoutHint, whatever the time spent in queue */
    const SOPC_TimeReference rx = SOPC_TimeReference_GetCurrent() - 3000; // received 3s ago
    SOPC_Services_SetCurrentMsgReceptionTime(rx);
    header.TimeoutHint = 5000;
    msg_subscription_publish_ack_bs__get_msg_header_expiration_time(&header, &expiration);
    ck_assert_uint_eq(SOPC_TimeReference_AddMilliseconds(rx, 5000), expiration);

    /* Waited in queue longer than the TimeoutHint: expiration time is already in the past */
    header.TimeoutHint = 1000;
    msg_subscription_publish_ack_bs__get_msg_header_expiration_time(&header, &expiration);
    ck_assert_uint_eq(SOPC_TimeReference_AddMilliseconds(rx, 1000), expiration);
    ck_assert(expiration < SOPC_TimeReference_GetCurrent());

    /* TimeoutHint = 0 (no timeout): 1 year validity from reception time */
    header.TimeoutHint = 0;
    msg_subscription_publish_ack_bs__get_msg_header_expiration_time(&header, &expiration);
    ck_assert_uint_eq(SOPC_TimeReference_AddMilliseconds(rx, 31536000000), expiration);

    /* No reception time (unexpected, a warning is traced): fallback on current time */
    SOPC_Services_ResetCurrentMsgReceptionTime();
    header.TimeoutHint = 5000;
    const SOPC_TimeReference before = SOPC_TimeReference_GetCurrent();
    msg_subscription_publish_ack_bs__get_msg_header_expiration_time(&header, &expiration);
    const SOPC_TimeReference after = SOPC_TimeReference_GetCurrent();
    ck_assert(expiration >= SOPC_TimeReference_AddMilliseconds(before, 5000));
    ck_assert(expiration <= SOPC_TimeReference_AddMilliseconds(after, 5000));

    OpcUa_RequestHeader_Clear(&header);
}
END_TEST

Suite* tests_make_suite_services_request_timeout(void)
{
    Suite* s = suite_create("Services request TimeoutHint");
    TCase* tc = tcase_create("Request TimeoutHint");
    tcase_add_test(tc, test_timeout_hint_rule);
    tcase_add_test(tc, test_current_msg_reception_time);
    tcase_add_test(tc, test_message_in_bs_timeout_expired);
    tcase_add_test(tc, test_publish_expiration_time);
    suite_add_tcase(s, tc);
    return s;
}
