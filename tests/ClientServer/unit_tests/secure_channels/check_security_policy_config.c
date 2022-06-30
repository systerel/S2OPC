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
 * \brief Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_common.h"
#include "sopc_macros.h"
#include "sopc_toolkit_config.h"

#define NB_SECU_POLICY_CONFIGS 1

// The function pointer pAppFct should point the following function to initialize the Toolkit
static void SOPC_ComEvent_Callback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext)
{
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(IdOrStatus);
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(appContext);
}

SOPC_ComEvent_Fct* pAppFct = &SOPC_ComEvent_Callback;

static SOPC_ReturnStatus check_security_combination(char* securityPolicy,
                                                    uint16_t securityModes,
                                                    char* userTokenPolicies,
                                                    OpcUa_UserTokenType TokenType)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Initialize Server configuration variable
    SOPC_Server_Config sConfig;
    sConfig.nbEndpoints = 1;
    SOPC_Endpoint_Config epConfig;
    epConfig.serverConfigPtr = &sConfig;
    sConfig.endpoints = &epConfig;

    SOPC_SecurityPolicy* pSecurityPolicy = &epConfig.secuConfigurations[0];
    OpcUa_UserTokenPolicy* pUserTokenPolicies = &epConfig.secuConfigurations[0].userTokenPolicies[0];
    uint32_t epConfigIdx = 0;

    // Initialize SOPC_Common
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_security_policy_config_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_Common_Initialize(logConfiguration);

    // Endpoint URL
    char* endpointUrl = "opc.tcp://localhost:4841/myEndPoint";

    // Create the Address Space
    SOPC_AddressSpace* securityPolicyAddressSpace = SOPC_AddressSpace_Create(true);

    // Initialize toolkit configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(pAppFct);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Server security check: Failed to initialize toolkit\n");
        }
    }

    // Set the Address space
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(securityPolicyAddressSpace);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Assign input parameters :
        pSecurityPolicy->securityModes = securityModes;
        pSecurityPolicy->nbOfUserTokenPolicies = 1;
        pUserTokenPolicies->TokenType = TokenType;
        SOPC_String_Initialize(&pSecurityPolicy->securityPolicy);
        status = SOPC_String_AttachFromCstring(&pSecurityPolicy->securityPolicy, securityPolicy);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String_Initialize(&pUserTokenPolicies->SecurityPolicyUri);
            status = SOPC_String_AttachFromCstring(&pUserTokenPolicies->SecurityPolicyUri, userTokenPolicies);
        }
    }

    // Start the server code
    if (SOPC_STATUS_OK == status)
    {
        epConfig.endpointURL = endpointUrl;
        epConfig.hasDiscoveryEndpoint = true;
        epConfig.nbSecuConfigs = NB_SECU_POLICY_CONFIGS;

        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(&epConfig);
        ck_assert_int_ne(epConfigIdx, 0);

        // Security check function is included in the following function :
        status = SOPC_ToolkitServer_Configured();
    }

    SOPC_String_Clear(&pSecurityPolicy->securityPolicy);
    SOPC_String_Clear(&pUserTokenPolicies->SecurityPolicyUri);
    SOPC_AddressSpace_Delete(securityPolicyAddressSpace);
    SOPC_Toolkit_Clear();
    return status;
}

START_TEST(SecurityMode_None_AND_SecurityUri_different_from_None)
{
    OpcUa_UserTokenType TokenType = OpcUa_UserTokenType_UserName;

    // CASE NOT ALLOWED :
    SOPC_ReturnStatus status =
        check_security_combination("Hello world", SOPC_SECURITY_MODE_NONE_MASK, "None", TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    status =
        check_security_combination(SOPC_SecurityPolicy_Basic256_URI, SOPC_SECURITY_MODE_NONE_MASK, "None", TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    status = check_security_combination(SOPC_SecurityPolicy_Basic256Sha256_URI, SOPC_SECURITY_MODE_NONE_MASK, "None",
                                        TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    // CASE ALLOWED :
    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK, "None", TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
}
END_TEST

START_TEST(SecurityMode_None_AND_UserTokenUri_Empty)
{
    OpcUa_UserTokenType TokenType = OpcUa_UserTokenType_UserName;

    // CASE NOT ALLOWED :
    SOPC_ReturnStatus status =
        check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK, "", TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    // CASE ALLOWED :
    status =
        check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK, "No empty", TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK,
                                        SOPC_SecurityPolicy_Basic256_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK,
                                        SOPC_SecurityPolicy_Basic256Sha256_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
}
END_TEST

START_TEST(SecurityMode_None_or_Sign_AND_UserTokenUri_None)
{
    OpcUa_UserTokenType TokenType = OpcUa_UserTokenType_UserName;

    // CASE NOT ALLOWED :
    SOPC_ReturnStatus status = check_security_combination(
        SOPC_SecurityPolicy_Basic256_URI, SOPC_SECURITY_MODE_SIGN_MASK, SOPC_SecurityPolicy_None_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    status = check_security_combination(SOPC_SecurityPolicy_Basic256Sha256_URI, SOPC_SECURITY_MODE_SIGN_MASK,
                                        SOPC_SecurityPolicy_None_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_SIGN_MASK,
                                        SOPC_SecurityPolicy_None_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK,
                                        SOPC_SecurityPolicy_None_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_INVALID_PARAMETERS);

    // CASE ALLOWED :
    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_SIGN_MASK, "Hello world",
                                        TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_Basic256_URI, SOPC_SECURITY_MODE_SIGN_MASK, "Hello world",
                                        TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_Basic256Sha256_URI, SOPC_SECURITY_MODE_SIGN_MASK,
                                        "Hello world", TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK, "Hello world",
                                        TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_Basic256_URI, SOPC_SECURITY_MODE_SIGN_MASK,
                                        SOPC_SecurityPolicy_Basic256_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_Basic256Sha256_URI, SOPC_SECURITY_MODE_SIGN_MASK,
                                        SOPC_SecurityPolicy_Basic256_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);

    status = check_security_combination(SOPC_SecurityPolicy_None_URI, SOPC_SECURITY_MODE_NONE_MASK,
                                        SOPC_SecurityPolicy_Basic256_URI, TokenType);
    ck_assert_int_eq(status, SOPC_STATUS_OK);
}
END_TEST

static Suite* tests_make_suite_security_policy_config(void)
{
    Suite* s;
    TCase* tc_security_policy_config;

    s = suite_create("Security policy configuration test");
    tc_security_policy_config = tcase_create("check invalid_security_policy_combination:");
    tcase_add_test(tc_security_policy_config, SecurityMode_None_AND_SecurityUri_different_from_None);
    tcase_add_test(tc_security_policy_config, SecurityMode_None_AND_UserTokenUri_Empty);
    tcase_add_test(tc_security_policy_config, SecurityMode_None_or_Sign_AND_UserTokenUri_None);
    suite_add_tcase(s, tc_security_policy_config);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    sr = srunner_create(tests_make_suite_security_policy_config());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
