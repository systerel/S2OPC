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

#include "assert.h"
#include "string.h"
#include "sopc_mutexes.h"
#include "sopc_mem_alloc.h"
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

static Mutex check_counter_mutex;
static Condition check_counter_condition;
static int32_t check_counter_connection_id = 0;
static int32_t check_counter_node_id_comparison_result = 1;
static SOPC_DataValue check_counter_data_value;

static void datachange_callback_check_counter(const int32_t c_id,
                                              const char* node_id,
                                              const SOPC_DataValue* value)
{
    assert(SOPC_STATUS_OK == Mutex_Lock(&check_counter_mutex));

    check_counter_connection_id = c_id;

    const char* correct_node_id = "ns=0;s=Counter";
    check_counter_node_id_comparison_result = strncmp(correct_node_id, node_id, strlen(correct_node_id));

    SOPC_DataValue_Copy(&check_counter_data_value, value);

    assert(SOPC_STATUS_OK == Mutex_Unlock(&check_counter_mutex));
    assert(SOPC_STATUS_OK == Condition_SignalAll(&check_counter_condition));
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

    /* delete subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_Unsubscribe(valid_con_id));
    /* create a subscription a second time */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

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

START_TEST(test_wrapper_add_monitored_items)
{
    /* create an array of valid node ids */
    char* nodeIds1[1] = { "ns=0;s=Counter" }; // value increments itself
    char* nodeIds2[1] = { "ns=0;i=1013" };
    char* nodeIds3[3] = { "ns=0;i=1009", "ns=0;i=1011", "ns=0;i=1001" };
    char* invalidNodeIds[1] = { NULL };
    char* invalidNodeIds2[2] = { "ns=0;s=Counter", NULL };

    /* add monitored items before toolkit being initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(1, nodeIds1, 1));

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* add monitored items before being connected */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(1, nodeIds1, 1));

    /* create a connection */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* add monitored items before subscription being created */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1));

    /* create a subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* invalid argument: connection id*/
    ck_assert_int_eq(-1, SOPC_ClientHelper_AddMonitoredItems(-1, nodeIds1, 1));
    /* invalid argument: nodeIds */
    ck_assert_int_eq(-2, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, NULL, 1));
    /* invalid argument: nodeIds content */
    ck_assert_int_eq(-3, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, invalidNodeIds, 1));
    ck_assert_int_eq(-4, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, invalidNodeIds2, 2));
    /* invalid argument: nbNodeIds */
    ck_assert_int_eq(-2, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 0));

    /* add one monitored item */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1));
    /* add one more monitored item */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds2, 1));
    /* add multiple monitored items */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds3, 3));

    /* delete subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_Unsubscribe(valid_con_id));

    /* add monitored items after subscription being deleted */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1));

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* add monitored items after being disconnected */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1));

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* add monitored items after toolkit being closed */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1));
}
END_TEST

START_TEST(test_wrapper_add_monitored_items_callback_called)
{
    char* nodeIds[1] = { "ns=0;s=Counter" };

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* create a connection */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* create a subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_check_counter));

    /* initialize mutex and condition */
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Initialization(&check_counter_mutex));
    ck_assert_int_eq(SOPC_STATUS_OK, Condition_Init(&check_counter_condition));

    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Lock(&check_counter_mutex));

    /* add one monitored item */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds, 1));

    /* verify that callback is called correctly */
    /* use a mutex and a condition to wait until datachange has been received (use a 1.2 sec timeout)*/
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_UnlockAndTimedWaitCond(&check_counter_condition, &check_counter_mutex, 1200));

    /* check connection id */
    ck_assert_int_eq(valid_con_id, check_counter_connection_id);
    /* check node id */
    ck_assert_int_eq(0, check_counter_node_id_comparison_result);
    /* check datavalue */
    ck_assert_int_eq(SOPC_UInt64_Id, check_counter_data_value.Value.BuiltInTypeId);
    uint64_t first_value = check_counter_data_value.Value.Value.Uint64;

    /* reset global values */
    check_counter_connection_id = 0;
    check_counter_node_id_comparison_result = 1;
    check_counter_data_value.Value.BuiltInTypeId = SOPC_Int16_Id;

    /* verify that callback is called correctly once again*/
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_UnlockAndTimedWaitCond(&check_counter_condition, &check_counter_mutex, 1200));
    /* verify datachange callback arguments again */
    /* check connection id */
    ck_assert_int_eq(valid_con_id, check_counter_connection_id);
    /* check node id */
    ck_assert_int_eq(0, check_counter_node_id_comparison_result);
    /* check datavalue */
    ck_assert_int_eq(SOPC_UInt64_Id, check_counter_data_value.Value.BuiltInTypeId);
    ck_assert_uint_ne(first_value, check_counter_data_value.Value.Value.Uint64);

    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Unlock(&check_counter_mutex));

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* clear mutex and condition */
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Clear(&check_counter_mutex));
    ck_assert_int_eq(SOPC_STATUS_OK, Condition_Clear(&check_counter_condition));
}
END_TEST

START_TEST(test_wrapper_unsubscribe)
{
    /* delete subscription before toolkit is initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(1));

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* delete subscription before connection */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(1));

    /* create a connection */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* delete subscription before subscription is created */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(valid_con_id));

    /* create a subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* invalid argument: connection id */
    ck_assert_int_eq(-1, SOPC_ClientHelper_Unsubscribe(-1));

    /* delete subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_Unsubscribe(valid_con_id));
    /* delete subscription once again */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(valid_con_id));

    /* delete subscription non existing connection id */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(valid_con_id + 1));

    /* create a subscription again */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* delete subscription after disconnection */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(valid_con_id));

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* delete subscription after toolkit is closed*/
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(valid_con_id));
}
END_TEST

START_TEST(test_wrapper_read)
{
    /* read before toolkit is initialized */
    {
        SOPC_ClientHelper_ReadValue readValue[1] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL }};
        SOPC_DataValue* readResults[1];
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(1, readValue, 1, readResults));
    }

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize("./check_wrapper_logs/", 0));

    /* read before connection is created */
    {
        SOPC_ClientHelper_ReadValue readValue[1] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL }};
        SOPC_DataValue* readResults[1];
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(1, readValue, 1, readResults));
    }

    /* create a connection */
    int32_t valid_con_id = SOPC_ClientHelper_Connect(valid_url, valid_security_none);
    ck_assert_int_gt(valid_con_id, 0);

    /* invalid arguments */
    {
        SOPC_ClientHelper_ReadValue readValue[1] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL }};
        SOPC_ClientHelper_ReadValue readValue2[2] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL },
                                                     { .nodeId = NULL, .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue* readResults[1];
        SOPC_DataValue* readResults2[1];

        /* invalid connection id */
        ck_assert_int_eq(-1, SOPC_ClientHelper_Read(-1, readValue, 1, readResults));
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(valid_con_id + 1, readValue, 1, readResults));
        /* invalid readValue */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Read(valid_con_id, NULL, 1, readResults));
        /* invalid nbElements */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Read(valid_con_id, readValue, 0, readResults));
        /* invalid values */
        ck_assert_int_eq(-3, SOPC_ClientHelper_Read(valid_con_id, readValue, 1, NULL));
        /* invalid readValue content (nodeId) */
        readValue[0].nodeId = NULL;
        ck_assert_int_eq(-4, SOPC_ClientHelper_Read(valid_con_id, readValue, 1, readResults));
        ck_assert_int_eq(-5, SOPC_ClientHelper_Read(valid_con_id, readValue2, 2, readResults2));
    }
    /* read one node */
    {
        SOPC_ClientHelper_ReadValue readValue1[1] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL }};
        SOPC_DataValue* readResults1[1];
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue1, 1, readResults1));
        /* check datavalue */
        ck_assert_ptr_ne(NULL, readResults1[0]);
        ck_assert_int_eq(SOPC_STATUS_OK, readResults1[0]->Status);
        ck_assert_int_eq(SOPC_UInt64_Id, readResults1[0]->Value.BuiltInTypeId);
        ck_assert_uint_ne(0, readResults1[0]->Value.Value.Uint64);
        /* free datavalue */
        SOPC_Free(readResults1[0]);
    }
    /* read multiple nodes */
    {
        SOPC_ClientHelper_ReadValue readValue2[2] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL },
                                                     { .nodeId = "ns=0;i=1001", .attributeId = 13, .indexRange = NULL }};
        SOPC_DataValue* readResults2[2];
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue2, 2, readResults2));
        /* check first datavalue */
        ck_assert_ptr_ne(NULL, readResults2[0]);
        ck_assert_int_eq(SOPC_STATUS_OK, readResults2[0]->Status);
        ck_assert_int_eq(SOPC_UInt64_Id, readResults2[0]->Value.BuiltInTypeId);
        ck_assert_uint_ne(0, readResults2[0]->Value.Value.Uint64);
        /* free first datavalue */
        SOPC_Free(readResults2[0]);

        /* check second datavalue */
        ck_assert_ptr_ne(NULL, readResults2[1]);
        ck_assert_int_eq(SOPC_STATUS_OK, readResults2[1]->Status);
        ck_assert_int_eq(SOPC_Int64_Id, readResults2[1]->Value.BuiltInTypeId);
        ck_assert_int_ne(0, readResults2[1]->Value.Value.Int64);
        /* free second datavalue */
        SOPC_Free(readResults2[1]);
    }
    /* read invalid node */
    {
        SOPC_ClientHelper_ReadValue readValue3[1] = {{ .nodeId = "ns=0;s=CounterThatShouldNotExist",
                                                       .attributeId = 13,
                                                        .indexRange = NULL }};
        SOPC_DataValue* readResults3[1];
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue3, 1, readResults3));
        /* check datavalue */
        ck_assert_ptr_ne(NULL, readResults3[0]);
        ck_assert_int_ne(SOPC_STATUS_OK, readResults3[0]->Status);
        /* free datavalue */
        SOPC_Free(readResults3[0]);
    }
    /* read mix of invalid nodes and valid nodes */
    {
        SOPC_ClientHelper_ReadValue readValue4[2] = {{ .nodeId = "ns=0;s=CounterThatShouldNotExist",
                                                       .attributeId = 13,
                                                       .indexRange = NULL },
                                                     { .nodeId = "ns=0;i=1001",
                                                        .attributeId = 13,
                                                        .indexRange = NULL }};
        SOPC_DataValue* readResults4[2];
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue4, 2, readResults4));
        /* check first datavalue */
        ck_assert_ptr_ne(NULL, readResults4[0]);
        ck_assert_int_ne(SOPC_STATUS_OK, readResults4[0]->Status);
        /* free first datavalue */
        SOPC_Free(readResults4[0]);

        /* check second datavalue */
        ck_assert_ptr_ne(NULL, readResults4[1]);
        ck_assert_int_eq(SOPC_STATUS_OK, readResults4[1]->Status);
        ck_assert_int_eq(SOPC_Int64_Id, readResults4[1]->Value.BuiltInTypeId);
        ck_assert_int_ne(0, readResults4[1]->Value.Value.Int64);
        /* free second datavalue */
        SOPC_Free(readResults4[1]);
    }

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* read after connection is closed */
    {
        SOPC_ClientHelper_ReadValue readValue5[1] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL }};
        SOPC_DataValue* readResults5[1];
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(valid_con_id, readValue5, 1, readResults5));
    }

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* read after toolkit is closed */
    {
        SOPC_ClientHelper_ReadValue readValue6[1] = {{ .nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL }};
        SOPC_DataValue* readResults6[1];
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(valid_con_id, readValue6, 1, readResults6));
    }
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
    tcase_add_test(tc_wrapper, test_wrapper_add_monitored_items);
    tcase_add_test(tc_wrapper, test_wrapper_add_monitored_items_callback_called);
    tcase_add_test(tc_wrapper, test_wrapper_unsubscribe);
    tcase_add_test(tc_wrapper, test_wrapper_read);
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
