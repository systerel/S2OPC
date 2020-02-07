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

#include "request_handle_bs.h"

START_TEST(test_request_handle_bs)
{
    constants__t_client_request_handle_i req_handle = 0;

    // Use all request handles available && check values are correctly set
    for (uint32_t i = 1; i <= SOPC_MAX_PENDING_REQUESTS; i++)
    {
        request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
                                                   constants__e_msg_session_create_resp, false, (uintptr_t) i,
                                                   &req_handle);
        ck_assert_int_eq(i, req_handle); // request handle matches index (implem dependent)

        // check associated context
        constants__t_msg_type_i msg_type = constants__c_msg_type_indet;
        request_handle_bs__get_req_handle_req_typ(req_handle, &msg_type);
        ck_assert_int_eq(constants__e_msg_session_create_req, msg_type);
        request_handle_bs__get_req_handle_resp_typ(req_handle, &msg_type);
        ck_assert_int_eq(constants__e_msg_session_create_resp, msg_type);
        bool is_app = true;
        constants__t_application_context_i app_context = 0;
        request_handle_bs__get_req_handle_app_context(req_handle, &is_app, &app_context);
        ck_assert_int_eq(false, is_app);
        ck_assert_int_eq(i, (uint32_t) app_context);

        // check channel association
        request_handle_bs__set_req_handle_channel(req_handle, i);
        constants__t_channel_i channel = constants_bs__c_channel_indet;
        request_handle_bs__get_req_handle_channel(req_handle, &channel);
        ck_assert_int_eq(i, channel);

        // check validate function
        bool is_valid = false;
        request_handle_bs__is_valid_req_handle(req_handle, &is_valid);
        ck_assert_int_eq(true, is_valid);
        is_valid = false;
        request_handle_bs__client_validate_response_request_handle(i, i, constants__e_msg_session_create_resp,
                                                                   &is_valid);
        ck_assert_int_eq(true, is_valid);

        /* Test an invalid channel with valid request handle */
        request_handle_bs__client_validate_response_request_handle(i + 1, i, constants__e_msg_session_create_resp,
                                                                   &is_valid);
        ck_assert_int_eq(false, is_valid);

        /* Test an invalid request handle with valid channel */
        is_valid = true;
        request_handle_bs__client_validate_response_request_handle(i, i + 1, constants__e_msg_session_create_resp,
                                                                   &is_valid);
        ck_assert_int_eq(false, is_valid);

        /* Test an valid request handle with valid channel but invalid response message type */
        is_valid = true;
        request_handle_bs__client_validate_response_request_handle(i, i, constants__e_msg_session_activate_resp,
                                                                   &is_valid);
        ck_assert_int_eq(false, is_valid);
    }

    // Check there is no more available
    req_handle = 1; // init non indet value
    ck_assert_int_ne(constants__c_client_request_handle_indet, req_handle);
    request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
                                               constants__e_msg_session_create_resp, false, (uintptr_t) NULL,
                                               &req_handle);
    ck_assert_int_eq(constants__c_client_request_handle_indet, req_handle);

    // Remove non contiguous request handles
    uint32_t remove_req_handles[] = {10, 2, 50, 1};

    // Remove the first 2
    request_handle_bs__client_remove_req_handle(remove_req_handles[0]);
    request_handle_bs__client_remove_req_handle(remove_req_handles[1]);

    request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
                                               constants__e_msg_session_create_resp, false, 0, &req_handle);
    // request handle matches expected (implem dependent):
    //  due to circular behavior 2 is first after SOPC_MAX_PENDING_REQUESTS
    ck_assert_int_eq(2, req_handle);

    request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
                                               constants__e_msg_session_create_resp, false, 0, &req_handle);
    ck_assert_int_eq(10, req_handle); // request handle matches expected (implem dependent)

    // Remove the last 2
    request_handle_bs__client_remove_req_handle(remove_req_handles[2]);
    request_handle_bs__client_remove_req_handle(remove_req_handles[3]);

    request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
                                               constants__e_msg_session_create_resp, false, 0, &req_handle);
    // request handle matches expected (implem dependent):
    //  due to circular behavior 50 is first after 10
    ck_assert_int_eq(50, req_handle);
    request_handle_bs__client_fresh_req_handle(constants__e_msg_session_create_req,
                                               constants__e_msg_session_create_resp, false, 0, &req_handle);
    ck_assert_int_eq(1, req_handle); // request handle matches expected (implem dependent)
}
END_TEST

Suite* tests_make_suite_B_base_machines(void)
{
    Suite* s;
    TCase* tc_request_handle;

    s = suite_create("B base machine tests");
    tc_request_handle = tcase_create("Request handle");

    tcase_add_test(tc_request_handle, test_request_handle_bs);
    suite_add_tcase(s, tc_request_handle);

    return s;
}
