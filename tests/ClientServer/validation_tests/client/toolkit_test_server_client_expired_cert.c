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
#include <stdlib.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "sopc_assert.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#include "embedded/sopc_addspace_loader.h"

#define SOPC_SERVER_PKI_PATH "./S2OPC_Tests_Expired_Certs_PKI"
#define SOPC_CLIENT_PKI_PATH "./S2OPC_Demo_PKI_Client"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost:product"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"

#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256

/* Server certificate */
#define SRV_CERT_PATH "./server_public/server_2k_cert.der"

static SOPC_PKIProvider* g_serverPkiProvider = NULL;
static SOPC_PKIProvider* g_clientPkiProvider = NULL;
static bool g_expected_conn_failure = false;

/*---------------------------------------------------------------------------
 *                          Callbacks
 *---------------------------------------------------------------------------*/

/*
 * Server stop callback
 */
static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
}

// Connection event callback (only for unexpected events)
static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);

    if (!g_expected_conn_failure)
    {
        SOPC_ASSERT(false && "Unexpected connection event");
    }
}

/*---------------------------------------------------------------------------
 *                          Client configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus client_create_configuration(const char* cert_path,
                                                     const char* key_path,
                                                     SOPC_SecureConnection_Config** outSecureConnConfig)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetApplicationDescription(
        DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI, DEFAULT_APPLICATION_NAME, NULL, OpcUa_ApplicationType_Client);

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed to configure the client key password callback\n");
        return status;
    }

    status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(cert_path, key_path, true);
    if (SOPC_STATUS_OK != status)
    {
        printf(">>Test_Client: Failed to load expired client certificate/key\n");
        return status;
    }

    status = SOPC_PKIProvider_CreateFromStore(SOPC_CLIENT_PKI_PATH, &g_clientPkiProvider);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(g_clientPkiProvider);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf(">>Test_Client: Failed to create client PKI\n");
        return status;
    }

    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);

    if (NULL == secureConnConfig)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(secureConnConfig, SRV_CERT_PATH);

    if (SOPC_STATUS_OK == status)
    {
        *outSecureConnConfig = secureConnConfig;
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                          Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_SetServerConfiguration(void)
{
    SOPC_Endpoint_Config* ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
    SOPC_SecurityPolicy* sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);

    if (NULL == ep || NULL == sp)
    {
        SOPC_Free(ep);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_SignAndEncrypt);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromPath("./server_public/server_2k_cert.der",
                                                                "./server_private/encrypted_server_2k_key.pem", true);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(SOPC_SERVER_PKI_PATH, &g_serverPkiProvider);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerConfigHelper_SetPKIprovider(g_serverPkiProvider);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed loading server certificates/key/PKI\n");
        return status;
    }

    status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                               "S2OPC toolkit server example", NULL,
                                                               OpcUa_ApplicationType_Server);

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed setting application description\n");
        return status;
    }

    SOPC_AddressSpace* address_space = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                          Tests configuration
 *---------------------------------------------------------------------------*/

static void setup_client_server(void)
{
    SOPC_Log_Configuration log_config = SOPC_Common_GetDefaultLogConfiguration();
    log_config.logLevel = SOPC_LOG_LEVEL_DEBUG;
    log_config.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_client_expired_cert_logs/";

    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&log_config, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ServerConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = Server_SetServerConfiguration();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
}

static void teardown_client_server(void)
{
    SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();
    ck_assert_int_eq(SOPC_STATUS_OK, stopStatus);
    SOPC_ClientConfigHelper_Clear();
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
}

static void run_expired_cert_case(const char* cert_path, const char* key_path, const char* case_name)
{
    printf("\n=== Running case: %s ===\n", case_name);

    SOPC_SecureConnection_Config* secConnConfig = NULL;
    SOPC_ReturnStatus status = client_create_configuration(cert_path, key_path, &secConnConfig);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(secConnConfig);

    SOPC_ClientConnection* connection = NULL;

    // First attempt: Expired cert or CA, the connection must fail.
    g_expected_conn_failure = true;
    status = SOPC_ClientHelper_Connect(secConnConfig, &SOPC_Client_ConnEventCb, &connection);

    ck_assert_int_eq(SOPC_STATUS_CLOSED, status);
    ck_assert_ptr_null(connection);

    printf(">>Client: Connection failed as expected\n");

    // Force the server PKI to ignore expired certificates.
    status = SOPC_PKIProvider_SuppressValidityPeriodCheck(g_serverPkiProvider, true);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    printf(">>Server: PKI configured to ignore expired certificates\n");

    // Second attempt: Same expired cert or CA, the connection should now be accepted.
    g_expected_conn_failure = false;
    status = SOPC_ClientHelper_Connect(secConnConfig, &SOPC_Client_ConnEventCb, &connection);

    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(connection);

    printf(">>Client: Connection succeeded after ignoring expired certificates\n");

    // Force the server PKI to reject expired certificates again.
    status = SOPC_ClientHelper_Disconnect(&connection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(connection);

    status = SOPC_PKIProvider_SuppressValidityPeriodCheck(g_serverPkiProvider, false);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    printf(">>Server: PKI configured to reject expired certificates again\n");

    // Third attempt: Same expired cert or CA, the connection must fail again.
    g_expected_conn_failure = true;
    status = SOPC_ClientHelper_Connect(secConnConfig, &SOPC_Client_ConnEventCb, &connection);

    ck_assert_int_eq(SOPC_STATUS_CLOSED, status);
    ck_assert_ptr_null(connection);

    printf(">>Client: Connection failed again as expected\n");

    SOPC_ClientConfigHelper_Clear();
}

/*---------------------------------------------------------------------------
 *                          Test
 *---------------------------------------------------------------------------*/

START_TEST(test_server_client_expired_certs1)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/client_4k_cert_expired.der",
                          "./S2OPC_expired_client_certs/client_private/encrypted_client_4k_key_expired.pem",
                          "Valid CA / Expired client certificate");
}
END_TEST

START_TEST(test_server_client_expired_certs2)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/client_cert_by_ca_cert_expired.der",
                          "./S2OPC_expired_client_certs/client_private/client_key_by_ca_cert_expired.pem",
                          "Expired CA / Valid client certificat");
}
END_TEST

START_TEST(test_server_client_expired_certs3)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/client_cert_by_int_cert_expired.der",
                          "./S2OPC_expired_client_certs/client_private/client_key_by_int_cert_expired.pem",
                          "Valid CA / Expired intermediate CA / Valid client certificate");
}
END_TEST

START_TEST(test_server_client_expired_certs4)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/ca_selfsigned_pathLen0_expired.der",
                          "./S2OPC_expired_client_certs/client_private/ca_selfsigned_pathLen0key_expired.pem",
                          "Expired self-signed certificate with CA=true");
}
END_TEST

START_TEST(test_server_client_expired_certs5)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/ca_selfsigned_pathLen0_expired_caFalse.der",
                          "./S2OPC_expired_client_certs/client_private/ca_selfsigned_pathLen0key_expired_caFalse.pem",
                          "Expired self-signed certificate with CA=false");
}
END_TEST

START_TEST(test_server_client_expired_certs6)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/client_4k_cert_for_crl_expired.der",
                          "./S2OPC_expired_client_certs/client_private/client_4k_key_for_crl_expired.pem",
                          "Valid CA / Expired CRL / Valid certificate");
}
END_TEST

START_TEST(test_server_client_expired_certs7)
{
    run_expired_cert_case("./S2OPC_expired_client_certs/client_public/client_4k_cert_expired_for_crl_expired.der",
                          "./S2OPC_expired_client_certs/client_private/client_4k_key_expired_for_crl_expired.pem",
                          "Valid CA / Expired CRL / Expired certificate");
}
END_TEST

/* The following test scenario "expired CA + expired CRL" is not necessary since the expired CA will be rejected first.
 * 'test_server_client_expired_certs2' is sufficient. */

static Suite* tests_make_suite_server_client(void)
{
    Suite* s = suite_create("Server/Client expired certs");
    TCase* tc_server_client = tcase_create("Expired certs test");

    tcase_add_checked_fixture(tc_server_client, setup_client_server, teardown_client_server);
    tcase_add_test(tc_server_client, test_server_client_expired_certs1);
    tcase_add_test(tc_server_client, test_server_client_expired_certs2);
    tcase_add_test(tc_server_client, test_server_client_expired_certs3);
    tcase_add_test(tc_server_client, test_server_client_expired_certs4);
    tcase_add_test(tc_server_client, test_server_client_expired_certs5);
    tcase_add_test(tc_server_client, test_server_client_expired_certs6);
    tcase_add_test(tc_server_client, test_server_client_expired_certs7);
    tcase_set_timeout(tc_server_client, 0);

    suite_add_tcase(s, tc_server_client);

    return s;
}

int main(void)
{
    int number_failed = 0;

    SRunner* sr = srunner_create(tests_make_suite_server_client());
    srunner_run_all(sr, CK_NORMAL);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
