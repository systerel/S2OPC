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
 * \brief Entry point for client wrapper tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdint.h>
#include <stdlib.h> /* EXIT_* */

#include "libs2opc_client_cmds.h"

static const char* valid_url = "opc.tcp://localhost:4841";
static const char* invalid_url = "opc.tcp://localhost:5841";



static SOPC_ClientHelper_Security valid_security_none = {
                                         .security_policy = SOPC_SecurityPolicy_None_URI,
                                         .security_mode = OpcUa_MessageSecurityMode_None,
                                         .path_cert_auth = "./trusted/cacert.der",
                                         .path_cert_srv = NULL,
                                         .path_cert_cli = NULL,
                                         .path_key_cli = NULL,
                                         .policyId = "anonymous",
                                         .username = NULL,
                                         .password = NULL};

static SOPC_ClientHelper_Security valid_security_sign_b256 = {
                                         .security_policy = SOPC_SecurityPolicy_Basic256_URI,
                                         .security_mode = OpcUa_MessageSecurityMode_Sign,
                                         .path_cert_auth = "./trusted/cacert.der",
                                         .path_cert_srv = "TODO",
                                         .path_cert_cli = "TODO",
                                         .path_key_cli = "TODO",
                                         .policyId = "TODO",
                                         .username = "TODO",
                                         .password = "TODO"};

static SOPC_ClientHelper_Security valid_security_signAndEncrypt_b256 = {
                                         .security_policy = SOPC_SecurityPolicy_Basic256_URI,
                                         .security_mode = OpcUa_MessageSecurityMode_SignAndEncrypt,
                                         .path_cert_auth = "./trusted/cacert.der",
                                         .path_cert_srv = "TODO",
                                         .path_cert_cli = "TODO",
                                         .path_key_cli = "TODO",
                                         .policyId = "TODO",
                                         .username = "TODO",
                                         .password = "TODO"};

static void datachange_callback_none(const int32_t c_id,
                                     const char* node_id,
                                     const SOPC_DataValue* value)
{
    (void) c_id;
    (void) node_id;
    (void) value;
}

START_TEST(test_wrapper_initialize_finalize)
{
    //TODO complete tests with invalid arguments and errors
    /* simple initialization */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));
    /* double finalize shall not fail*/
    SOPC_ClientHelper_Finalize();
    SOPC_ClientHelper_Finalize();

    /* double initialization shall fail */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));
    ck_assert_int_eq(-2, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));
}
END_TEST

START_TEST(test_wrapper_connect)
{
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    //TODO check different security_mode/security_policy combination
    //TODO check invalid client/srv cert (not signed by CA)

    /* connection to a valid endpoint */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* disconnect a valid endpoint */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* connect to an invalid url */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Connect(invalid_url, valid_security_none));

    /* check multiple connections */
    int32_t con_ids[5];
    con_ids[0] = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(con_ids[0], 0);
    con_ids[1] = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(con_ids[1], 0);
    con_ids[2] = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(con_ids[2], 0);
    con_ids[3] = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(con_ids[3], 0);
    con_ids[4] = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(con_ids[4], 0);

    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[0]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[1]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[2]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[3]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[4]));

    SOPC_ClientHelper_Finalize();

    /* connect without wrapper being initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Connect(valid_url, valid_security_none));
}
END_TEST

START_TEST(test_wrapper_connect_invalid_arguments)
{
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* invalid arguments */
    ck_assert_int_eq(-1, SOPC_ClientHelper_Connect(NULL, valid_security_none));
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_none;
        invalid_security.security_policy = NULL;
        ck_assert_int_eq(-11, SOPC_ClientHelper_Connect(valid_url, invalid_security));
        invalid_security.security_policy = "InvalidSecurityPolicy";
        ck_assert_int_eq(-11, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_none;
        invalid_security.security_mode = 0;
        ck_assert_int_eq(-12, SOPC_ClientHelper_Connect(valid_url, invalid_security));
        invalid_security.security_mode = 4;
        ck_assert_int_eq(-12, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_sign_b256;
        invalid_security.path_cert_auth = NULL;
        ck_assert_int_eq(-13, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256;
        invalid_security.path_cert_srv = NULL;
        ck_assert_int_eq(-14, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256;
        invalid_security.path_cert_cli = NULL;
        ck_assert_int_eq(-15, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256;
        invalid_security.path_key_cli = NULL;
        ck_assert_int_eq(-16, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256;
        invalid_security.policyId = NULL;
        ck_assert_int_eq(-17, SOPC_ClientHelper_Connect(valid_url, invalid_security));
    }
    /* cannot test username and password */

    SOPC_ClientHelper_Finalize();
}
END_TEST

START_TEST(test_wrapper_disconnect)
{
    //TODO complete tests
    /* disconnect before wrapper has been initialized */
    ck_assert_int_eq(-2, SOPC_ClientHelper_Disconnect(1));

    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* connection to a valid endpoint */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* disconnect a valid endpoint */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* disconnect a non valid connection */
    ck_assert_int_eq(-1, SOPC_ClientHelper_Disconnect(-1));

    /* disconnect an already closed connection */
    ck_assert_int_eq(-3, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* disconnect a non existing connection */
    ck_assert_int_eq(-3, SOPC_ClientHelper_Disconnect(31));

    SOPC_ClientHelper_Finalize();

    /* disconnect after wrapper has been closed */
    ck_assert_int_eq(-2, SOPC_ClientHelper_Disconnect(1));
}
END_TEST

START_TEST(test_wrapper_create_subscription)
{
    /* create subscription before wrapper has been initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateSubscription(1, datachange_callback_none));

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* create a connection */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* create connection using invalid connection id */
    ck_assert_int_eq(-1, SOPC_ClientHelper_CreateSubscription(-1, datachange_callback_none));

    /* create a connection using an invalid callback */
    ck_assert_int_eq(-2, SOPC_ClientHelper_CreateSubscription(valid_con_id, NULL));

    /* create a connection using a non existing connection */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateSubscription(valid_con_id + 1, datachange_callback_none));

    /* create a valid subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* call the subscription creation a second time */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* create a subscription after wrapper has been closed */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));
}
END_TEST

START_TEST(test_wrapper_create_subscription_after_disconnect)
{
    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* create a connection */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* create a subscription after being disconnected */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* close wrapper */
    SOPC_ClientHelper_Finalize();
}
END_TEST

START_TEST(test_wrapper_browse)
{
    //ck_assert(SOPC_STATUS_INVALID_STATE == SOPC_STATUS_OK);
}
END_TEST

static Suite* tests_make_suite_wrapper(void)
{
    Suite* s = NULL;
    TCase *tc_wrapper;

    s = suite_create("Client subscription library");

    tc_wrapper = tcase_create("WrapperC");
    tcase_add_test(tc_wrapper, test_wrapper_initialize_finalize);
    tcase_add_test(tc_wrapper, test_wrapper_connect);
    tcase_add_test(tc_wrapper, test_wrapper_connect_invalid_arguments);
    tcase_add_test(tc_wrapper, test_wrapper_disconnect);
    tcase_add_test(tc_wrapper, test_wrapper_create_subscription);
    tcase_add_test(tc_wrapper, test_wrapper_create_subscription_after_disconnect);
    tcase_add_test(tc_wrapper, test_wrapper_browse);
    tcase_set_timeout(tc_wrapper, 0);
    suite_add_tcase(s, tc_wrapper);

    return s;
}

int main(void)
{
    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_make_suite_wrapper());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
