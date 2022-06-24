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
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_time.h" /* SOPC_Sleep */
#include "string.h"

#include "libs2opc_client_cmds.h"
#include "libs2opc_client_cmds_internal_api.h"
#include "libs2opc_common_config.h"

#define SLEEP_TIME 10
#define CONNECTION_TIMEOUT 10000

static const char* valid_url = "opc.tcp://localhost:4841";
static const char* invalid_url = "opc.tcp://localhost:5841";

// Define number of read values in read request to force multi chunk use in request and response:
// use max buffer size for 1 chunk and encoded size of a ReadValueId / DataValue which is 18 bytes in this test
#define NB_READ_VALUES ((SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE / 18) + 1)

static SOPC_ClientHelper_Security valid_security_none = {.security_policy = SOPC_SecurityPolicy_None_URI,
                                                         .security_mode = OpcUa_MessageSecurityMode_None,
                                                         .path_cert_auth = "./trusted/cacert.der",
                                                         .path_crl = "./revoked/cacrl.der",
                                                         .path_cert_srv = NULL,
                                                         .path_cert_cli = NULL,
                                                         .path_key_cli = NULL,
                                                         .policyId = "anonymous",
                                                         .username = NULL,
                                                         .password = NULL};

static SOPC_ClientHelper_Security valid_security_signAndEncrypt_b256sha256 = {
    .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
    .security_mode = OpcUa_MessageSecurityMode_SignAndEncrypt,
    .path_cert_auth = "./trusted/cacert.der",
    .path_crl = "./revoked/cacrl.der",
    .path_cert_srv = "./server_public/server_4k_cert.der",
    .path_cert_cli = "./client_public/client_4k_cert.der",
    .path_key_cli = "./client_private/client_4k_key.pem",
    .policyId = "username",
    .username = "username",
    .password = "password"};

static void datachange_callback_none(const int32_t c_id, const char* node_id, const SOPC_DataValue* value)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_UNUSED_ARG(node_id);
    SOPC_UNUSED_ARG(value);
}

static Mutex check_counter_mutex;
static Condition check_counter_condition;
static int32_t check_counter_connection_id = 0;
static int32_t check_counter_node_id_comparison_result = 1;
static SOPC_DataValue check_counter_data_value;
static int32_t disconnected = 0;

static void disconnect_callback(const uint32_t c_id)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_Atomic_Int_Set(&disconnected, 1);
}

static void datachange_callback_check_counter(const int32_t c_id, const char* node_id, const SOPC_DataValue* value)
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
    /* simple initialization */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));
    /* double finalize shall not fail*/
    SOPC_ClientHelper_Finalize();
    SOPC_ClientHelper_Finalize();

    /* double initialization shall fail */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));
    ck_assert_int_eq(-100, SOPC_ClientHelper_Initialize(NULL));

    SOPC_ClientHelper_Finalize();
}
END_TEST

START_TEST(test_wrapper_create_configuration)
{
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* configuration of a valid endpoint */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);

    /* check multiple configurations */
    int32_t conf_ids;
    conf_ids = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(conf_ids, 0);
    conf_ids = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(conf_ids, 0);
    conf_ids = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(conf_ids, 0);
    conf_ids = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(conf_ids, 0);
    conf_ids = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(conf_ids, 0);

    SOPC_ClientHelper_Finalize();

    /* configure a connection without wrapper being initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL));
}
END_TEST

START_TEST(test_wrapper_create_connection)
{
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* configuration of a valid endpoint */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);

    /* configuration of an invalid url */
    int32_t invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(invalid_url, &valid_security_none, NULL);
    ck_assert_int_gt(invalid_conf_id, 0);

    /* connect using a valid configuration */
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* connect multiple times using a valid configuration */
    int32_t con_ids[5];
    con_ids[0] = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(con_ids[0], 0);
    con_ids[1] = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(con_ids[1], 0);
    con_ids[2] = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(con_ids[2], 0);
    con_ids[3] = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(con_ids[3], 0);
    con_ids[4] = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(con_ids[4], 0);

    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[0]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[1]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[2]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[3]));
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(con_ids[4]));

    /* connect using an invalid configuration */
    int32_t invalid_con_id = SOPC_ClientHelper_CreateConnection(invalid_conf_id);
    ck_assert_int_eq(invalid_con_id, -100);

    /* connect using a non-existing configuration */
    int32_t invalid_con_id_2 = SOPC_ClientHelper_CreateConnection(valid_conf_id + invalid_conf_id);
    ck_assert_int_eq(invalid_con_id_2, -100);

    /* connect using a non-existing configuration */
    int32_t invalid_con_id_3 = SOPC_ClientHelper_CreateConnection(-1);
    ck_assert_int_eq(invalid_con_id_3, -1);

    SOPC_ClientHelper_Finalize();

    /* connect without wrapper being initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateConnection(valid_conf_id));
}
END_TEST

START_TEST(test_wrapper_config_invalid_arguments)
{
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    int32_t invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(NULL, &valid_security_none, NULL);
    ck_assert_int_eq(-1, invalid_conf_id);

    {
        SOPC_ClientHelper_Security invalid_security = valid_security_none;
        invalid_security.security_policy = NULL;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-11, invalid_conf_id);
        invalid_security.security_policy = "InvalidSecurityPolicy";
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-11, invalid_conf_id);
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_none;
        invalid_security.security_mode = 0;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-12, invalid_conf_id);
        invalid_security.security_mode = 4;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-12, invalid_conf_id);
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256sha256;
        invalid_security.path_cert_srv = NULL;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-15, invalid_conf_id);
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256sha256;
        invalid_security.path_cert_cli = NULL;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-16, invalid_conf_id);
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256sha256;
        invalid_security.path_key_cli = NULL;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-17, invalid_conf_id);
    }
    {
        SOPC_ClientHelper_Security invalid_security = valid_security_signAndEncrypt_b256sha256;
        invalid_security.policyId = NULL;
        invalid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &invalid_security, NULL);
        ck_assert_int_eq(-18, invalid_conf_id);
    }
    /* cannot test username and password */

    SOPC_ClientHelper_Finalize();
}
END_TEST

START_TEST(test_wrapper_disconnect)
{
    /* disconnect before wrapper has been initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Disconnect(1));

    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* connection to a valid endpoint */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
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
    ck_assert_int_eq(-100, SOPC_ClientHelper_Disconnect(1));
}
END_TEST

START_TEST(test_wrapper_create_subscription)
{
    /* create subscription before wrapper has been initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_CreateSubscription(1, datachange_callback_none));

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* connection to a valid endpoint */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
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
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
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
    char* nodeIds1[1] = {"ns=0;s=Counter"}; // value increments itself
    char* nodeIds2[1] = {"ns=0;i=1013"};
    char* nodeIds3[3] = {"ns=0;i=1009", "ns=0;i=1011", "ns=0;i=1001"};
    char* nodeIds3_plus_unknown[4] = {"ns=0;i=1009", "ns=0;i=1011", "ns=0;i=1001", "ns=1;s=Invalid_NodeId"};
    char* nodeIds2_plus_2_unknown[4] = {"ns=1;s=Invalid_NodeId", "ns=0;i=1009", "ns=0;i=1011", "ns=1;s=Invalid_NodeId"};

    char* invalidNodeIds[1] = {NULL};
    char* invalidNodeIds2[2] = {"ns=0;s=Counter", NULL};

    SOPC_StatusCode results[4] = {SOPC_UncertainStatusMask, SOPC_UncertainStatusMask, SOPC_UncertainStatusMask,
                                  SOPC_UncertainStatusMask};

    /* add monitored items before toolkit being initialized */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(1, nodeIds1, 1, NULL));

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* add monitored items before being connected */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(1, nodeIds1, 1, NULL));

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* add monitored items before subscription being created */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1, NULL));

    /* create a subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_none));

    /* invalid argument: connection id*/
    ck_assert_int_eq(-1, SOPC_ClientHelper_AddMonitoredItems(-1, nodeIds1, 1, NULL));
    /* invalid argument: nodeIds */
    ck_assert_int_eq(-2, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, NULL, 1, NULL));
    /* invalid argument: nodeIds content */
    ck_assert_int_eq(-3, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, invalidNodeIds, 1, NULL));
    ck_assert_int_eq(-4, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, invalidNodeIds2, 2, NULL));
    /* invalid argument: nbNodeIds */
    ck_assert_int_eq(-2, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 0, NULL));

    /* add one monitored item */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1, results));
    ck_assert(SOPC_IsGoodStatus(results[0]));
    /* add one more monitored item */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds2, 1, results));
    ck_assert(SOPC_IsGoodStatus(results[0]));
    /* add multiple monitored items */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds3, 3, results));
    ck_assert(SOPC_IsGoodStatus(results[0]));
    ck_assert(SOPC_IsGoodStatus(results[1]));
    ck_assert(SOPC_IsGoodStatus(results[2]));

    /* add multiple monitored items with 1 unkown node id*/
    ck_assert_int_eq(1, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds3_plus_unknown, 4, results));
    ck_assert(SOPC_IsGoodStatus(results[0]));
    ck_assert(SOPC_IsGoodStatus(results[1]));
    ck_assert(SOPC_IsGoodStatus(results[2]));
    ck_assert(!SOPC_IsGoodStatus(results[3]));

    ck_assert_int_eq(2, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds2_plus_2_unknown, 4, results));
    ck_assert(!SOPC_IsGoodStatus(results[0]));
    ck_assert(SOPC_IsGoodStatus(results[1]));
    ck_assert(SOPC_IsGoodStatus(results[2]));
    ck_assert(!SOPC_IsGoodStatus(results[3]));

    /* delete subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_Unsubscribe(valid_con_id));

    /* add monitored items after subscription being deleted */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1, NULL));

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* add monitored items after being disconnected */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1, NULL));

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* add monitored items after toolkit being closed */
    ck_assert_int_eq(-100, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds1, 1, NULL));
}
END_TEST

START_TEST(test_wrapper_add_monitored_items_callback_called)
{
    char* nodeIds[1] = {"ns=0;s=Counter"};

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* create a subscription */
    ck_assert_int_eq(0, SOPC_ClientHelper_CreateSubscription(valid_con_id, datachange_callback_check_counter));

    /* initialize mutex and condition */
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Initialization(&check_counter_mutex));
    ck_assert_int_eq(SOPC_STATUS_OK, Condition_Init(&check_counter_condition));

    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Lock(&check_counter_mutex));

    /* add one monitored item */
    ck_assert_int_eq(0, SOPC_ClientHelper_AddMonitoredItems(valid_con_id, nodeIds, 1, NULL));

    /* verify that callback is called correctly */
    /* use a mutex and a condition to wait until datachange has been received (use a 1.2 sec timeout)*/
    ck_assert_int_eq(SOPC_STATUS_OK,
                     Mutex_UnlockAndTimedWaitCond(&check_counter_condition, &check_counter_mutex, 1200));

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
    ck_assert_int_eq(SOPC_STATUS_OK,
                     Mutex_UnlockAndTimedWaitCond(&check_counter_condition, &check_counter_mutex, 1200));
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
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* delete subscription before connection */
    ck_assert_int_eq(-100, SOPC_ClientHelper_Unsubscribe(1));

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
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
        SOPC_ClientHelper_ReadValue readValue[1] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults;
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(1, readValue, 1, &readResults));
    }

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* read before connection is created */
    {
        SOPC_ClientHelper_ReadValue readValue[1] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults;
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(1, readValue, 1, &readResults));
    }

    /* create a connection */
    int32_t valid_conf_id =
        SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_signAndEncrypt_b256sha256, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* invalid arguments */
    {
        SOPC_ClientHelper_ReadValue readValue[1] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL}};
        SOPC_ClientHelper_ReadValue readValue2[2] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL},
            {.nodeId = NULL, .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults;
        SOPC_DataValue readResults2[2];

        /* invalid connection id */
        ck_assert_int_eq(-1, SOPC_ClientHelper_Read(-1, readValue, 1, &readResults));
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(valid_con_id + 1, readValue, 1, &readResults));
        /* invalid readValue */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Read(valid_con_id, NULL, 1, &readResults));
        /* invalid nbElements */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Read(valid_con_id, readValue, 0, &readResults));
        /* invalid values */
        ck_assert_int_eq(-3, SOPC_ClientHelper_Read(valid_con_id, readValue, 1, NULL));
        /* invalid readValue content (nodeId) */
        readValue[0].nodeId = NULL;
        ck_assert_int_eq(-4, SOPC_ClientHelper_Read(valid_con_id, readValue, 1, &readResults));
        ck_assert_int_eq(-5, SOPC_ClientHelper_Read(valid_con_id, readValue2, 2, readResults2));
    }
    /* read one node */
    {
        SOPC_ClientHelper_ReadValue readValue1[1] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults1;
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue1, 1, &readResults1));
        /* check datavalue */
        ck_assert_int_eq(SOPC_STATUS_OK, readResults1.Status);
        ck_assert_int_eq(SOPC_UInt64_Id, readResults1.Value.BuiltInTypeId);
        ck_assert_uint_ne(0, readResults1.Value.Value.Uint64);
        /* free results */
        SOPC_ClientHelper_ReadResults_Free(1, &readResults1);
    }
    /* read multiple nodes */
    {
        SOPC_ClientHelper_ReadValue readValueMultiple[NB_READ_VALUES];
        /* = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL},
            {.nodeId = "ns=0;i=1001", .attributeId = 13, .indexRange = NULL}};
            */
        for (size_t i = 0; i < NB_READ_VALUES; i++)
        {
            readValueMultiple[i].nodeId = "ns=0;s=Counter";
            readValueMultiple[i].attributeId = 13;
            readValueMultiple[i].indexRange = NULL;
        }
        SOPC_DataValue readResultsMultiple[NB_READ_VALUES];
        ck_assert_int_eq(0,
                         SOPC_ClientHelper_Read(valid_con_id, readValueMultiple, NB_READ_VALUES, readResultsMultiple));

        for (size_t i = 0; i < NB_READ_VALUES; i++)
        {
            /* check datavalue */
            ck_assert_ptr_ne(NULL, readResultsMultiple);
            ck_assert_int_eq(SOPC_STATUS_OK, readResultsMultiple[i].Status);
            ck_assert_int_eq(SOPC_UInt64_Id, readResultsMultiple[i].Value.BuiltInTypeId);
            ck_assert_uint_ne(0, readResultsMultiple[i].Value.Value.Uint64);
        }

        /* free results */
        SOPC_ClientHelper_ReadResults_Free(NB_READ_VALUES, readResultsMultiple);
    }
    /* read invalid node */
    {
        SOPC_ClientHelper_ReadValue readValue3[1] = {
            {.nodeId = "ns=0;s=CounterThatShouldNotExist", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults3;
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue3, 1, &readResults3));
        /* check datavalue */
        ck_assert_int_ne(SOPC_STATUS_OK, readResults3.Status);
        /* free results */
        SOPC_ClientHelper_ReadResults_Free(1, &readResults3);
    }
    /* read mix of invalid nodes and valid nodes */
    {
        SOPC_ClientHelper_ReadValue readValue4[2] = {
            {.nodeId = "ns=0;s=CounterThatShouldNotExist", .attributeId = 13, .indexRange = NULL},
            {.nodeId = "ns=0;i=1001", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults4[2];
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue4, 2, readResults4));
        /* check first datavalue */
        ck_assert_ptr_ne(NULL, readResults4);
        ck_assert_int_ne(SOPC_STATUS_OK, readResults4[0].Status);

        /* check second datavalue */
        ck_assert_ptr_ne(NULL, readResults4);
        ck_assert_int_eq(SOPC_STATUS_OK, readResults4[1].Status);
        ck_assert_int_eq(SOPC_Int64_Id, readResults4[1].Value.BuiltInTypeId);
        ck_assert_int_ne(0, readResults4[1].Value.Value.Int64);

        /* free results */
        SOPC_ClientHelper_ReadResults_Free(2, readResults4);
    }

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* read after connection is closed */
    {
        SOPC_ClientHelper_ReadValue readValue5[1] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults5;
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(valid_con_id, readValue5, 1, &readResults5));
    }

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* read after toolkit is closed */
    {
        SOPC_ClientHelper_ReadValue readValue6[1] = {
            {.nodeId = "ns=0;s=Counter", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults6;
        ck_assert_int_eq(-100, SOPC_ClientHelper_Read(valid_con_id, readValue6, 1, &readResults6));
    }
}
END_TEST

START_TEST(test_wrapper_write)
{
    /* write a node before toolkit is initialized */
    {
        SOPC_DataValue value;
        SOPC_ClientHelper_WriteValue writeValues[1] = {{.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value}};
        SOPC_StatusCode writeResults[1] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -500;

        ck_assert_int_eq(-100, SOPC_ClientHelper_Write(1, writeValues, 1, writeResults));
    }

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* write a node before connection */
    {
        SOPC_DataValue value;
        SOPC_ClientHelper_WriteValue writeValues[1] = {{.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value}};
        SOPC_StatusCode writeResults[1] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -500;

        ck_assert_int_eq(-100, SOPC_ClientHelper_Write(1, writeValues, 1, writeResults));
    }

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* invalid arguments */
    {
        SOPC_DataValue value;
        SOPC_ClientHelper_WriteValue writeValues[1] = {{.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value}};
        SOPC_StatusCode writeResults[1] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -500;

        /* invalid connection id */
        ck_assert_int_eq(-1, SOPC_ClientHelper_Write(-1, writeValues, 1, writeResults));
        /* invalid write values */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Write(valid_con_id, NULL, 1, writeResults));
        /* invalid nbElements */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Write(valid_con_id, writeValues, 0, writeResults));
        /* invalid writeResults */
        ck_assert_int_eq(-3, SOPC_ClientHelper_Write(valid_con_id, writeValues, 1, NULL));
    }

    /* write a node */
    {
        SOPC_DataValue value;
        SOPC_ClientHelper_WriteValue writeValues[1] = {{.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value}};
        SOPC_StatusCode writeResults[1] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -500;

        ck_assert_int_eq(0, SOPC_ClientHelper_Write(valid_con_id, writeValues, 1, writeResults));
        ck_assert_int_eq(SOPC_STATUS_OK, writeResults[0]);
    }
    /* write multiple nodes */
    {
        SOPC_DataValue value[2];
        SOPC_ClientHelper_WriteValue writeValues[2] = {
            {.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value[0]},
            {.nodeId = "ns=0;i=1009", .indexRange = NULL, .value = &value[1]},
        };
        SOPC_StatusCode writeResults[2] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -200;

        SOPC_DataValue_Initialize(writeValues[1].value);
        writeValues[1].value->Value.DoNotClear = false;
        writeValues[1].value->Value.BuiltInTypeId = SOPC_Int16_Id;
        writeValues[1].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[1].value->Value.Value.Int64 = -20;

        ck_assert_int_eq(0, SOPC_ClientHelper_Write(valid_con_id, writeValues, 2, writeResults));
        ck_assert_int_eq(SOPC_STATUS_OK, writeResults[0]);
        ck_assert_int_eq(SOPC_STATUS_OK, writeResults[1]);
    }

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* write a node after disconnection */
    {
        SOPC_DataValue value;
        SOPC_ClientHelper_WriteValue writeValues[1] = {{.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value}};
        SOPC_StatusCode writeResults[1] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -500;

        ck_assert_int_eq(-100, SOPC_ClientHelper_Write(valid_con_id, writeValues, 1, writeResults));
    }

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* write a node after toolkit is closed */
    {
        SOPC_DataValue value;
        SOPC_ClientHelper_WriteValue writeValues[1] = {{.nodeId = "ns=0;i=1001", .indexRange = NULL, .value = &value}};
        SOPC_StatusCode writeResults[1] = {SOPC_STATUS_NOK};

        SOPC_DataValue_Initialize(writeValues[0].value);
        writeValues[0].value->Value.DoNotClear = false;
        writeValues[0].value->Value.BuiltInTypeId = SOPC_Int64_Id;
        writeValues[0].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
        writeValues[0].value->Value.Value.Int64 = -500;

        ck_assert_int_eq(-100, SOPC_ClientHelper_Write(valid_con_id, writeValues, 1, writeResults));
    }
}
END_TEST

START_TEST(test_wrapper_browse)
{
    /* browse before initialization */
    {
        // Root/ - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[1] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[1];

        ck_assert_int_eq(-100, SOPC_ClientHelper_Browse(1, browseRequest, 1, browseResult));
    }

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* browse before connection */
    {
        // Root/ - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[1] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[1];

        ck_assert_int_eq(-100, SOPC_ClientHelper_Browse(1, browseRequest, 1, browseResult));
    }

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* invalid arguments */
    {
        // Root/ - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[1] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[1];

        /* invalid connection id */
        ck_assert_int_eq(-1, SOPC_ClientHelper_Browse(-1, browseRequest, 1, browseResult));
        /* invalid browseRequests */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Browse(valid_con_id, NULL, 1, browseResult));
        /*  invalid nbElements */
        ck_assert_int_eq(-2, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 0, browseResult));
        /*  invalid browseResults */
        ck_assert_int_eq(-3, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 1, NULL));
    }
    /* browse */
    {
        // Root/ - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[1] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[1];

        ck_assert_int_eq(0, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 1, browseResult));
        ck_assert_int_eq(SOPC_STATUS_OK, browseResult[0].statusCode);
        ck_assert_int_eq(3, browseResult[0].nbOfReferences);

        ck_assert_ptr_ne(NULL, &browseResult[0].references[0]);
        ck_assert_ptr_ne(NULL, browseResult[0].references[0].referenceTypeId);
        ck_assert(true == browseResult[0].references[0].isForward);
        ck_assert_ptr_ne(NULL, browseResult[0].references[0].nodeId);
        ck_assert_ptr_ne(NULL, browseResult[0].references[0].browseName);
        ck_assert_ptr_ne(NULL, browseResult[0].references[0].displayName);
        ck_assert_int_eq(1, browseResult[0].references[0].nodeClass);
        /* free */
        SOPC_Free(browseResult[0].references[0].referenceTypeId);
        SOPC_Free(browseResult[0].references[0].nodeId);
        SOPC_Free(browseResult[0].references[0].browseName);
        SOPC_Free(browseResult[0].references[0].displayName);

        ck_assert_ptr_ne(NULL, &browseResult[0].references[1]);
        ck_assert_ptr_ne(NULL, browseResult[0].references[1].referenceTypeId);
        ck_assert(true == browseResult[0].references[1].isForward);
        ck_assert_ptr_ne(NULL, browseResult[0].references[1].nodeId);
        ck_assert_ptr_ne(NULL, browseResult[0].references[1].browseName);
        ck_assert_ptr_ne(NULL, browseResult[0].references[1].displayName);
        ck_assert_int_eq(1, browseResult[0].references[1].nodeClass);
        /* free */
        SOPC_Free(browseResult[0].references[1].referenceTypeId);
        SOPC_Free(browseResult[0].references[1].nodeId);
        SOPC_Free(browseResult[0].references[1].browseName);
        SOPC_Free(browseResult[0].references[1].displayName);

        ck_assert_ptr_ne(NULL, &browseResult[0].references[2]);
        ck_assert_ptr_ne(NULL, browseResult[0].references[2].referenceTypeId);
        ck_assert(true == browseResult[0].references[2].isForward);
        ck_assert_ptr_ne(NULL, browseResult[0].references[2].nodeId);
        ck_assert_ptr_ne(NULL, browseResult[0].references[2].browseName);
        ck_assert_ptr_ne(NULL, browseResult[0].references[2].displayName);
        ck_assert_int_eq(1, browseResult[0].references[2].nodeClass);
        /* free */
        SOPC_Free(browseResult[0].references[2].referenceTypeId);
        SOPC_Free(browseResult[0].references[2].nodeId);
        SOPC_Free(browseResult[0].references[2].browseName);
        SOPC_Free(browseResult[0].references[2].displayName);

        SOPC_Free(browseResult[0].references);
    }
    /* browse too many browse requests */
    {
        // FreeOPCUA is ignoring the maxReferencesPerNode and sends only one response,
        // no BrowseNextRequest. I need at least one or two to trigger the "too many"
        // browse next requests.
        // This test should work with a server that is not ignoring maxReferencesPerNode.
        // TODO see if upgrading FreeOPCUA solves the problem, else use another one
        // SOPC_ClientHelper_BrowseRequest browseRequest[1] = {
        //        { .nodeId = "ns=0;i=7617", .direction = OpcUa_BrowseDirection_Forward, .referenceTypeId = "ns=0;i=33",
        //        .includeSubtypes = true }
        //};
        // SOPC_ClientHelper_BrowseResult browseResult[1];
        // CfgMaxReferencesPerNode = 1;
        // CfgMaxBrowseNextRequests = 10;

        // ck_assert_int_eq(-4, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 1, browseResult));
    } /* browse multiple nodes */
    {
        // Root/ and Root/Objects - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[2] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true},
                                                            {.nodeId = "ns=0;i=85",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[2];

        ck_assert_int_eq(0, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 2, browseResult));
        ck_assert_int_eq(SOPC_STATUS_OK, browseResult[0].statusCode);
        ck_assert_int_eq(3, browseResult[0].nbOfReferences);
        ck_assert_int_eq(16, browseResult[1].nbOfReferences);

        /* free */
        for (int32_t i = 0; i < browseResult[0].nbOfReferences; i++)
        {
            ck_assert_ptr_ne(NULL, &browseResult[0].references[i]);
            ck_assert_ptr_ne(NULL, browseResult[0].references[i].referenceTypeId);
            ck_assert(true == browseResult[0].references[i].isForward);
            ck_assert_ptr_ne(NULL, browseResult[0].references[i].nodeId);
            ck_assert_ptr_ne(NULL, browseResult[0].references[i].browseName);
            ck_assert_ptr_ne(NULL, browseResult[0].references[i].displayName);

            SOPC_Free(browseResult[0].references[i].referenceTypeId);
            SOPC_Free(browseResult[0].references[i].nodeId);
            SOPC_Free(browseResult[0].references[i].browseName);
            SOPC_Free(browseResult[0].references[i].displayName);
        }
        SOPC_Free(browseResult[0].references);

        for (int32_t i = 0; i < browseResult[1].nbOfReferences; i++)
        {
            ck_assert_ptr_ne(NULL, &browseResult[1].references[i]);
            ck_assert_ptr_ne(NULL, browseResult[1].references[i].referenceTypeId);
            ck_assert(true == browseResult[1].references[i].isForward);
            ck_assert_ptr_ne(NULL, browseResult[1].references[i].nodeId);
            ck_assert_ptr_ne(NULL, browseResult[1].references[i].browseName);
            ck_assert_ptr_ne(NULL, browseResult[1].references[i].displayName);

            SOPC_Free(browseResult[1].references[i].referenceTypeId);
            SOPC_Free(browseResult[1].references[i].nodeId);
            SOPC_Free(browseResult[1].references[i].browseName);
            SOPC_Free(browseResult[1].references[i].displayName);
        }
        SOPC_Free(browseResult[1].references);
    }

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* browse after disconnection */
    {
        // Root/ - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[1] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[1];

        ck_assert_int_eq(-100, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 1, browseResult));
    }

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* browse after toolkit is closed */
    {
        // Root/ - Hierarchical references
        SOPC_ClientHelper_BrowseRequest browseRequest[1] = {{.nodeId = "ns=0;i=84",
                                                             .direction = OpcUa_BrowseDirection_Forward,
                                                             .referenceTypeId = "ns=0;i=33",
                                                             .includeSubtypes = true}};
        SOPC_ClientHelper_BrowseResult browseResult[1];

        ck_assert_int_eq(-100, SOPC_ClientHelper_Browse(valid_con_id, browseRequest, 1, browseResult));
    }
}
END_TEST

START_TEST(test_wrapper_get_endpoints)
{
    /* get endpoints before initialization */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        ck_assert_int_eq(-100, SOPC_ClientHelper_GetEndpoints(valid_url, &result));
    }

    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* get endpoints  valid request */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        ck_assert_int_eq(0, SOPC_ClientHelper_GetEndpoints(valid_url, &result));
        /* check result content */
        ck_assert_int_ge(result->nbOfEndpoints, 1);
        /* free result */
        SOPC_ClientHelper_GetEndpointsResult_Free(&result);
    }

    /* invalid arguments */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        /* bad url NULL */
        ck_assert_int_eq(-1, SOPC_ClientHelper_GetEndpoints(NULL, &result));
        /* bad result NULL */
        ck_assert_int_eq(-2, SOPC_ClientHelper_GetEndpoints(valid_url, NULL));
        /* bad invalid_url */
        ck_assert_int_eq(-100, SOPC_ClientHelper_GetEndpoints(invalid_url, &result));
    }

    /* close wrapper */
    SOPC_ClientHelper_Finalize();

    /* get endpoints after toolkit is closed */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        ck_assert_int_eq(-100, SOPC_ClientHelper_GetEndpoints(valid_url, &result));
    }
}
END_TEST

START_TEST(test_wrapper_disconnect_callback)
{
    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(disconnect_callback));

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(valid_url, &valid_security_none, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* disconnect */
    ck_assert_int_eq(0, SOPC_ClientHelper_Disconnect(valid_con_id));

    /* wait until timeout or until callback is called */
    int iCnt = 0;
    while (iCnt * SLEEP_TIME <= CONNECTION_TIMEOUT && SOPC_Atomic_Int_Get(&disconnected) == 0)
    {
        SOPC_Sleep(SLEEP_TIME);
        iCnt++;
    }

    /* verify that the disconnect callback has been called */
    ck_assert(SOPC_Atomic_Int_Get(&disconnected) == 1);

    /* Close wrapper */
    SOPC_ClientHelper_Finalize();
}
END_TEST

static void setup(void)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./check_wrapper_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
}

static void teardown(void)
{
    SOPC_ClientHelper_Finalize();
    SOPC_CommonHelper_Clear();
}

static Suite* tests_make_suite_wrapper(void)
{
    Suite* s = NULL;
    TCase* tc_wrapper;

    s = suite_create("Client wrapper library");

    tc_wrapper = tcase_create("WrapperC");
    // Add a teardown to guarantee the call to SOPC_ClientHelper_Finalize after each test
    tcase_add_checked_fixture(tc_wrapper, setup, teardown);
    tcase_add_test(tc_wrapper, test_wrapper_initialize_finalize);
    tcase_add_test(tc_wrapper, test_wrapper_create_configuration);
    tcase_add_test(tc_wrapper, test_wrapper_create_connection);
    tcase_add_test(tc_wrapper, test_wrapper_config_invalid_arguments);
    tcase_add_test(tc_wrapper, test_wrapper_disconnect);
    tcase_add_test(tc_wrapper, test_wrapper_create_subscription);
    tcase_add_test(tc_wrapper, test_wrapper_create_subscription_after_disconnect);
    tcase_add_test(tc_wrapper, test_wrapper_add_monitored_items);
    tcase_add_test(tc_wrapper, test_wrapper_add_monitored_items_callback_called);
    tcase_add_test(tc_wrapper, test_wrapper_unsubscribe);
    tcase_add_test(tc_wrapper, test_wrapper_read);
    tcase_add_test(tc_wrapper, test_wrapper_write);
    tcase_add_test(tc_wrapper, test_wrapper_browse);
    tcase_add_test(tc_wrapper, test_wrapper_get_endpoints);
    tcase_add_test(tc_wrapper, test_wrapper_disconnect_callback);
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
