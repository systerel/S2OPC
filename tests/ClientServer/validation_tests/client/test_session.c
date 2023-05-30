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

/** \fil
 * \brief Test the Toolkit API
 */

#include <check.h>
#include <stdbool.h>

#include "sopc_helper_askpass.h"
#include "test_suite_client.h"

#include "libs2opc_client_config_custom.h"
#include "libs2opc_new_client.h"
#include "libs2opc_request_builder.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_Sign
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256

// Client certificate path
#define CLI_CERT_PATH "./client_public/client_4k_cert.der"
// Server certificate path
#define SRV_CERT_PATH "./server_public/server_4k_cert.der"
// Client private key path
#define CLI_KEY_PATH "./client_private/encrypted_client_4k_key.pem"

// PKI trusted CA
static char* default_trusted_root_issuers[] = {"trusted/cacert.der", /* Demo CA */
                                               NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der", NULL};
static char* default_empty_cert_paths[] = {NULL};

static void SOPC_ClientConnectionEventCb(SOPC_ClientConnection* config,
                                         SOPC_ClientConnectionEvent event,
                                         SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    ck_assert(false);
}

static bool SOPC_GetClientUser1Password(char** outUserName, char** outPassword)
{
    const char* user1 = "user";
    char* userName = SOPC_Calloc(strlen(user1) + 1, sizeof(*userName));
    if (NULL == userName)
    {
        return false;
    }
    memcpy(userName, user1, strlen(user1) + 1);
    bool res = SOPC_TestHelper_AskPassWithContext_FromEnv(user1, outPassword);
    if (!res)
    {
        SOPC_Free(userName);
        return false;
    }
    *outUserName = userName;
    return true;
}

START_TEST(test_anonymous)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_session_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Set callback to retrieve password (from environment variable)
    status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Load client certificate and key from files */
    status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(CLI_CERT_PATH, CLI_KEY_PATH, true);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Create the PKI (Public Key Infrastructure) provider */
    SOPC_PKIProvider* pkiProvider = NULL;
    status = SOPC_PKIProviderStack_CreateFromPaths(default_trusted_root_issuers, default_empty_cert_paths,
                                                   default_empty_cert_paths, default_empty_cert_paths,
                                                   default_empty_cert_paths, default_revoked_certs, &pkiProvider);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pkiProvider);
    /* Set PKI provider as client PKI*/
    status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "user_session", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);

    status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(secureConnConfig, SRV_CERT_PATH);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_SecureConnectionConfig_SetAnonymous(secureConnConfig, "anonymous");
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_ClientConnection* secureConnection = NULL;
    status = SOPC_ClientHelperNew_Connect(secureConnConfig, &SOPC_ClientConnectionEventCb, &secureConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(secureConnection);

    status = SOPC_ClientHelperNew_Disconnect(&secureConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(secureConnection);

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
}
END_TEST

START_TEST(test_username_password)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_session_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Set callback to retrieve password (from environment variable)
    status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    // Set callback necessary to retrieve user password (from environment variable)
    status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&SOPC_GetClientUser1Password);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Load client certificate and key from files */
    status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(CLI_CERT_PATH, CLI_KEY_PATH, true);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Create the PKI (Public Key Infrastructure) provider */
    SOPC_PKIProvider* pkiProvider = NULL;
    status = SOPC_PKIProviderStack_CreateFromPaths(default_trusted_root_issuers, default_empty_cert_paths,
                                                   default_empty_cert_paths, default_empty_cert_paths,
                                                   default_empty_cert_paths, default_revoked_certs, &pkiProvider);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(pkiProvider);
    /* Set PKI provider as client PKI*/
    status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "user_session", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);

    status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(secureConnConfig, SRV_CERT_PATH);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_SecureConnectionConfig_SetUserName(secureConnConfig, "username", NULL, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_ClientConnection* secureConnection = NULL;
    status = SOPC_ClientHelperNew_Connect(secureConnConfig, &SOPC_ClientConnectionEventCb, &secureConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_nonnull(secureConnection);

    status = SOPC_ClientHelperNew_Disconnect(&secureConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_ptr_null(secureConnection);

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
}
END_TEST

Suite* client_suite_make_session(void)
{
    Suite* s = NULL;
    TCase* tc_user = NULL;

    s = suite_create("Client session");
    /* Anonymous is tested in toolkit_test_client for now */
    tc_user = tcase_create("Username password");

    suite_add_tcase(s, tc_user);
    tcase_add_test(tc_user, test_anonymous);
    tcase_add_test(tc_user, test_username_password);

    return s;
}
