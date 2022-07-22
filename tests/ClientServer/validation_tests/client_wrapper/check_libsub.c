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
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdint.h>
#include <stdlib.h> /* EXIT_* */

#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encodeable.h"
#include "sopc_log_manager.h"
#include "sopc_macros.h"
#include "sopc_time.h" /* SOPC_Sleep, SOPC_TimeReference */
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

#include "toolkit_helpers.h"

#define SLEEP_TIME 10
#define CONNECTION_TIMEOUT 10000
#define ROBUSTNESS_TIMEOUT 20000
#define ROBUSTNESS_RETRY_PERIOD 2000

START_TEST(test_time_conversion)
{
    /* Thu Sep 21 00:00:00 1905 UTC, unix timestamp is -2028499761.000000 */
    ck_assert(Helpers_OPCTimeToNTP(96159738390000000ULL) == 775194519791468544ULL);
    /* Tue Jan  3 19:44:21 1978 UTC, unix timestamp is 252701061.000000 */
    ck_assert(Helpers_OPCTimeToNTP(118971746610000000ULL) == 10572877445889785856ULL);
    /* Thu Nov 30 04:57:25 2034 UTC, unix timestamp is 2048471845.694287 */
    ck_assert(Helpers_OPCTimeToNTP(136929454456942870ULL) == 18285654237264005879ULL);
    /* Tue Nov 30 04:57:25 2055 UTC, unix timestamp is 2711159845.694287 */
    ck_assert(Helpers_OPCTimeToNTP(143556334456942870ULL) == 2685133451006102263ULL);
    /* Fri May  4 17:34:36 2018 UTC, unix timestamp is 1525448076.741346 */
    ck_assert(Helpers_OPCTimeToNTP(131699216767413460ULL) == 16039284254580464121ULL);
}
END_TEST

/* Subscription test */

static SOPC_LibSub_ConnectionId con_id = 0;
static SOPC_LibSub_DataId dat_id = 0;
static int32_t firstValueIsUninit = 1;
static int32_t iFirstValue = 0;
static int32_t valueChanged = 0;
static int32_t disconnected = 0;
static int32_t responseReceived = 0;

static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    /* This is not just assert(false), as the disconnection shall happen when closing the lib */
    ck_assert(c_id == con_id);
    SOPC_Atomic_Int_Set(&disconnected, 1);
}

static void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
                                const SOPC_LibSub_DataId d_id,
                                const SOPC_LibSub_Value* value)
{
    ck_assert(c_id == con_id);
    ck_assert(d_id == dat_id);
    ck_assert(value->type == SOPC_LibSub_DataType_integer);
    ck_assert(value->quality == 0);

    /* TODO: Atomic flip bit is not atomic */
    if (SOPC_Atomic_Int_Get(&firstValueIsUninit) == 1)
    {
        SOPC_Atomic_Int_Set(&firstValueIsUninit, 0);
        int64_t val = *(int64_t*) (value->value);
        val %= (INT32_MAX + 1LL);
        SOPC_Atomic_Int_Set(&iFirstValue, (int32_t) val);
    }
    /* TODO: Atomic flip bit is not atomic */
    else if (SOPC_Atomic_Int_Get(&valueChanged) == 0)
    {
        SOPC_Atomic_Int_Set(&valueChanged, 1);
        int64_t val = *(int64_t*) (value->value);
        /* Once every 2%32, this would fail, but the server always starts at 0 */
        ck_assert((val % (INT32_MAX + 1LL)) == SOPC_Atomic_Int_Get(&iFirstValue) + 1);
    }
}

static void generic_event_callback(SOPC_LibSub_ConnectionId c_id,
                                   SOPC_LibSub_ApplicativeEvent event,
                                   SOPC_StatusCode status,
                                   const void* response,
                                   uintptr_t responseContext)
{
    SOPC_UNUSED_ARG(event);

    ck_assert(c_id == con_id);
    ck_assert(SOPC_LibSub_ApplicativeEvent_Response);
    ck_assert(SOPC_STATUS_OK == status);
    ck_assert(42 == responseContext);

    SOPC_EncodeableType* pEncType = *(SOPC_EncodeableType* const*) response;
    ck_assert(&OpcUa_ReadResponse_EncodeableType == pEncType);

    ck_assert(SOPC_Atomic_Int_Get(&responseReceived) == 0);
    SOPC_Atomic_Int_Set(&responseReceived, 1);
}

START_TEST(test_subscription)
{
    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = Helpers_LoggerStdout,
                                     .disconnect_callback = disconnect_callback,
                                     .toolkit_logger = {.level = SOPC_LOG_LEVEL_DEBUG,
                                                        .log_path = "./check_libsub_subscription_logs/",
                                                        .maxBytes = 1048576,
                                                        .maxFiles = 50}};
    SOPC_LibSub_ConnectionCfg cfg_con = {.server_url = "opc.tcp://localhost:4841",
                                         .security_policy = SOPC_SecurityPolicy_None_URI,
                                         .security_mode = OpcUa_MessageSecurityMode_None,
                                         .disable_certificate_verification = false,
                                         .path_cert_auth = "./trusted/cacert.der",
                                         .path_cert_srv = NULL,
                                         .path_cert_cli = NULL,
                                         .path_key_cli = NULL,
                                         .path_crl = "./revoked/cacrl.der",
                                         .policyId = "anonymous",
                                         .username = NULL,
                                         .password = NULL,
                                         .publish_period_ms = 100,
                                         .n_max_keepalive = 3,
                                         .n_max_lifetime = 1000,
                                         .data_change_callback = datachange_callback,
                                         .timeout_ms = CONNECTION_TIMEOUT,
                                         .sc_lifetime = 60000,
                                         .token_target = 3,
                                         .generic_response_callback = generic_event_callback,
                                         .expected_endpoints = NULL};
    OpcUa_ReadValueId* lrv = NULL;
    OpcUa_ReadRequest* read_req = NULL;

    SOPC_ReturnStatus status = SOPC_Encodeable_Create(&OpcUa_ReadRequest_EncodeableType, (void**) &read_req);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_Encodeable_Create(&OpcUa_ReadValueId_EncodeableType, (void**) &lrv);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_LibSub_ConfigurationId cfg_id = 0;

    ck_assert_ptr_nonnull(lrv);
    *lrv = (OpcUa_ReadValueId){.encodeableType = &OpcUa_ReadValueId_EncodeableType,
                               .NodeId = {.IdentifierType = SOPC_IdentifierType_Numeric, .Data.Numeric = 84},
                               .AttributeId = SOPC_LibSub_AttributeId_BrowseName};

    ck_assert_ptr_nonnull(read_req);
    *read_req = (OpcUa_ReadRequest){.encodeableType = &OpcUa_ReadRequest_EncodeableType,
                                    .MaxAge = 0.,
                                    .TimestampsToReturn = OpcUa_TimestampsToReturn_Both,
                                    .NoOfNodesToRead = 1,
                                    .NodesToRead = lrv};

    ck_assert(SOPC_LibSub_Initialize(&cfg_cli) == SOPC_STATUS_OK);

    ck_assert(SOPC_LibSub_ConfigureConnection(&cfg_con, &cfg_id) == SOPC_STATUS_OK);
    ck_assert(SOPC_LibSub_Configured() == SOPC_STATUS_OK);
    ck_assert(SOPC_LibSub_Connect(cfg_id, &con_id) == SOPC_STATUS_OK);
    SOPC_LibSub_AttributeId attribute_id_value = SOPC_LibSub_AttributeId_Value;
    const char* nid = "s=Counter";
    ck_assert(SOPC_LibSub_AddToSubscription(con_id, &nid, &attribute_id_value, 1, &dat_id) == SOPC_STATUS_OK);

    ck_assert(SOPC_LibSub_AsyncSendRequestOnSession(con_id, read_req, 42) == SOPC_STATUS_OK);

    int iCnt = 0;
    /* Wait for deconnection, failed assert, or subscription success */
    while (iCnt * SLEEP_TIME <= CONNECTION_TIMEOUT && SOPC_Atomic_Int_Get(&valueChanged) == 0 &&
           SOPC_Atomic_Int_Get(&disconnected) == 0)
    {
        SOPC_Sleep(SLEEP_TIME);
    }

    ck_assert(SOPC_Atomic_Int_Get(&disconnected) == 0);
    ck_assert(SOPC_LibSub_Disconnect(con_id) == SOPC_STATUS_OK);
    SOPC_LibSub_Clear();
}
END_TEST

#define N_CONNECTIONS 3
SOPC_LibSub_ConfigurationId cfg_ids[N_CONNECTIONS] = {0, 0, 0};
SOPC_LibSub_DataId dat_ids[N_CONNECTIONS] = {0, 0, 0};
SOPC_LibSub_ConnectionId con_ids[N_CONNECTIONS] = {0, 0, 0};
bool connect_statuses[N_CONNECTIONS] = {false, false, false};
SOPC_TimeReference disconnect_times[N_CONNECTIONS] = {0, 0, 0};

static void datachange_callback_do_nothing(const SOPC_LibSub_ConnectionId c_id,
                                           const SOPC_LibSub_DataId d_id,
                                           const SOPC_LibSub_Value* value)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_UNUSED_ARG(d_id);
    SOPC_UNUSED_ARG(value);
}

static void disconnect_callback_multi(const SOPC_LibSub_ConnectionId c_id)
{
    /* Search index of connection */
    int idx = 0;
    bool found = false;
    for (; idx < N_CONNECTIONS; ++idx)
    {
        if (con_ids[idx] == c_id)
        {
            found = true;
            break;
        }
    }
    ck_assert(found);

    connect_statuses[idx] = false;
    disconnect_times[idx] = SOPC_TimeReference_GetCurrent();
}

/* This test reproduce a potentially erroneuous situation where connections are not always made */
START_TEST(test_half_broken_subscriptions)
{
    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = Helpers_LoggerStdout,
                                     .disconnect_callback = disconnect_callback_multi,
                                     .toolkit_logger = {.level = SOPC_LOG_LEVEL_DEBUG,
                                                        .log_path = "./check_libsub_subscription_logs/",
                                                        .maxBytes = 1048576,
                                                        .maxFiles = 50}};
    SOPC_LibSub_ConnectionCfg cfg_con[N_CONNECTIONS] = {
        {.server_url = "opc.tcp://localhost:4841",
         .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
         .security_mode = OpcUa_MessageSecurityMode_Sign,
         .disable_certificate_verification = false,
         .path_cert_auth = "./trusted/cacert.der",
         .path_crl = "./revoked/cacrl.der",
         .path_cert_srv = "./server_public/server_4k_cert.der",
         .path_cert_cli = "./client_public/client_4k_cert.der",
         .path_key_cli = "./client_private/client_4k_key.pem",
         .policyId = "username",
         .username = "user",
         .password = "",
         .publish_period_ms = 500,
         .n_max_keepalive = 3,
         .n_max_lifetime = 1000,
         .data_change_callback = datachange_callback_do_nothing,
         .timeout_ms = CONNECTION_TIMEOUT,
         .sc_lifetime = 3600000,
         .token_target = 3,
         .generic_response_callback = NULL,
         .expected_endpoints = NULL},
        {.server_url = "opc.tcp://localhost:4842", /* Do not connect this one */
         .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
         .security_mode = OpcUa_MessageSecurityMode_SignAndEncrypt,
         .disable_certificate_verification = false,
         .path_cert_auth = "./trusted/cacert.der",
         .path_crl = "./revoked/cacrl.der",
         .path_cert_srv = "./server_public/server_4k_cert.der",
         .path_cert_cli = "./client_public/client_4k_cert.der",
         .path_key_cli = "./client_private/client_4k_key.pem",
         .policyId = "username",
         .username = "user",
         .password = "",
         .publish_period_ms = 500,
         .n_max_keepalive = 3,
         .n_max_lifetime = 1000,
         .data_change_callback = datachange_callback_do_nothing,
         .timeout_ms = CONNECTION_TIMEOUT,
         .sc_lifetime = 3600000,
         .token_target = 3,
         .generic_response_callback = NULL,
         .expected_endpoints = NULL},
        {.server_url = "opc.tcp://localhost:4843", /* Do not connect this one */
         .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
         .security_mode = OpcUa_MessageSecurityMode_SignAndEncrypt,
         .disable_certificate_verification = false,
         .path_cert_auth = "./trusted/cacert.der",
         .path_crl = "./revoked/cacrl.der",
         .path_cert_srv = "./server_public/server_4k_cert.der",
         .path_cert_cli = "./client_public/client_4k_cert.der",
         .path_key_cli = "./client_private/client_4k_key.pem",
         .policyId = "username",
         .username = "user",
         .password = "",
         .publish_period_ms = 500,
         .n_max_keepalive = 3,
         .n_max_lifetime = 1000,
         .data_change_callback = datachange_callback_do_nothing,
         .timeout_ms = CONNECTION_TIMEOUT,
         .sc_lifetime = 3600000,
         .token_target = 3,
         .generic_response_callback = NULL,
         .expected_endpoints = NULL}};

    ck_assert(SOPC_LibSub_Initialize(&cfg_cli) == SOPC_STATUS_OK);
    for (int i = 0; i < N_CONNECTIONS; ++i)
    {
        ck_assert(SOPC_LibSub_ConfigureConnection(&cfg_con[i], &cfg_ids[i]) == SOPC_STATUS_OK);
    }
    ck_assert(SOPC_LibSub_Configured() == SOPC_STATUS_OK);

    /* Wait for deconnection, failed assert, or subscription success */
    SOPC_TimeReference timeout =
        SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference_GetCurrent(), ROBUSTNESS_TIMEOUT);
    SOPC_LibSub_AttributeId attribute_id_value = SOPC_LibSub_AttributeId_Value;
    const char* nid = "s=Counter";
    while (SOPC_TimeReference_Compare(SOPC_TimeReference_GetCurrent(), timeout) <= 0)
    {
        SOPC_TimeReference curTime = SOPC_TimeReference_GetCurrent();
        for (int i = 0; i < N_CONNECTIONS; ++i)
        {
            /* Disconnection time is used as a time reference.
             * As connect() is blocking, there cannot be two connection request for the same i in the same time */
            if (!connect_statuses[i] &&
                SOPC_TimeReference_Compare(
                    SOPC_TimeReference_AddMilliseconds(disconnect_times[i], ROBUSTNESS_RETRY_PERIOD), curTime) <= 0)
            {
                Helpers_Log(SOPC_LOG_LEVEL_INFO, "New connection with cfg_id %i.", cfg_ids[i]);
                if (SOPC_LibSub_Connect(cfg_ids[i], &con_ids[i]) == SOPC_STATUS_OK)
                {
                    ck_assert(SOPC_LibSub_AddToSubscription(con_ids[i], &nid, &attribute_id_value, 1, &dat_ids[i]) ==
                              SOPC_STATUS_OK);
                    connect_statuses[i] = true;
                }
                else
                {
                    connect_statuses[i] = false;
                    disconnect_times[i] = SOPC_TimeReference_GetCurrent();
                }
            }
        }

        SOPC_Sleep(SLEEP_TIME);
    }

    ck_assert(SOPC_LibSub_Disconnect(con_ids[0]) == SOPC_STATUS_OK);
    /* The following two assert that the connections are not in the client list anymore,
     * hence having been correctly removed when connection failed. */
    ck_assert(SOPC_LibSub_Disconnect(con_ids[1]) == SOPC_STATUS_INVALID_PARAMETERS);
    ck_assert(SOPC_LibSub_Disconnect(con_ids[2]) == SOPC_STATUS_INVALID_PARAMETERS);

    SOPC_LibSub_Clear();
}
END_TEST

static Suite* tests_make_suite_libsub(void)
{
    Suite* s = NULL;
    TCase *tc_time = NULL, *tc_libsub;

    s = suite_create("Client subscription library");

    tc_time = tcase_create("Time");
    tcase_add_test(tc_time, test_time_conversion);
    suite_add_tcase(s, tc_time);

    tc_libsub = tcase_create("LibSub");
    tcase_add_test(tc_libsub, test_subscription);
    tcase_add_test(tc_libsub, test_half_broken_subscriptions);
    tcase_set_timeout(tc_libsub, 0);
    suite_add_tcase(s, tc_libsub);

    return s;
}

int main(void)
{
    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_make_suite_libsub());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
