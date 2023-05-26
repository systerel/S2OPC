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

#include "string.h"

#include "libs2opc_client_cmds.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "sopc_helper_askpass.h"

#define SLEEP_TIME 10
#define CONNECTION_TIMEOUT 10000

#define VALID_URL "opc.tcp://localhost:4841"
#define INVALID_URL "opc.tcp://localhost:5841"

#define REVERSE_ENDPOINT_URL "opc.tcp://localhost:4844"

static SOPC_ClientHelper_Security valid_security_signAndEncrypt_b256sha256 = {
    .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
    .security_mode = OpcUa_MessageSecurityMode_SignAndEncrypt,
    .path_cert_auth = "./trusted/cacert.der",
    .path_crl = "./revoked/cacrl.der",
    .path_cert_srv = "./server_public/server_2k_cert.der",
    .path_cert_cli = "./client_public/client_4k_cert.der",
    .path_key_cli = "./client_private/encrypted_client_4k_key.pem",
    .policyId = "anonymous",
    .username = NULL,
    .password = NULL,
    .path_cert_x509_token = NULL,
    .path_key_x509_token = NULL,
    .key_x509_token_encrypted = false,
};

START_TEST(test_wrapper_reverse_connections)
{
    /* initialize wrapper */
    ck_assert_int_eq(0, SOPC_ClientHelper_Initialize(NULL));

    /* create reverse endpoint */

    int32_t reverseEpId = SOPC_ClientHelper_CreateReverseEndpoint(REVERSE_ENDPOINT_URL);
    ck_assert_int_gt(reverseEpId, 0);

    SOPC_ClientHelper_EndpointConnection reverse_connection_endpoint = {
        .endpointUrl = INVALID_URL,
        .serverUri = NULL,
        .reverseConnectionConfigId = (uint32_t) reverseEpId,
    };

    /* invalid server URL */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        ck_assert_int_eq(-100, SOPC_ClientHelper_GetEndpoints(&reverse_connection_endpoint, &result));
    }

    reverse_connection_endpoint.endpointUrl = VALID_URL;
    reverse_connection_endpoint.serverUri = "invalid_uri";
    /* invalid server URI */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        ck_assert_int_eq(-100, SOPC_ClientHelper_GetEndpoints(&reverse_connection_endpoint, &result));
    }

    reverse_connection_endpoint.serverUri = "urn:S2OPC:localhost";
    /* get endpoints  valid */
    {
        SOPC_ClientHelper_GetEndpointsResult* result;
        ck_assert_int_eq(0, SOPC_ClientHelper_GetEndpoints(&reverse_connection_endpoint, &result));
        /* check result content */
        ck_assert_int_ge(result->nbOfEndpoints, 1);
        /* free result */
        SOPC_ClientHelper_GetEndpointsResult_Free(&result);
    }

    /* callback to retrieve the client's private key password */
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* create a connection */
    int32_t valid_conf_id = SOPC_ClientHelper_CreateConfiguration(&reverse_connection_endpoint,
                                                                  &valid_security_signAndEncrypt_b256sha256, NULL);
    ck_assert_int_gt(valid_conf_id, 0);
    int32_t valid_con_id = SOPC_ClientHelper_CreateConnection(valid_conf_id);
    ck_assert_int_gt(valid_con_id, 0);

    /* read one node */
    {
        SOPC_ClientHelper_ReadValue readValue1[1] = {
            {.nodeId = "ns=1;s=UInt64_001", .attributeId = 13, .indexRange = NULL}};
        SOPC_DataValue readResults1;
        ck_assert_int_eq(0, SOPC_ClientHelper_Read(valid_con_id, readValue1, 1, &readResults1));
        /* check datavalue */
        ck_assert_int_eq(SOPC_STATUS_OK, readResults1.Status);
        ck_assert_int_eq(SOPC_UInt64_Id, readResults1.Value.BuiltInTypeId);
        ck_assert_uint_ne(0, readResults1.Value.Value.Uint64);
        /* free results */
        SOPC_ClientHelper_ReadResults_Free(1, &readResults1);
    }
}
END_TEST

static void setup(void)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./check_wrapper_reverse_connection_logs/";
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

    s = suite_create("Client wrapper library (RHE)");

    tc_wrapper = tcase_create("Wrapper C reverse connection");
    tcase_add_checked_fixture(tc_wrapper, setup, teardown);
    tcase_add_test(tc_wrapper, test_wrapper_reverse_connections);
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
