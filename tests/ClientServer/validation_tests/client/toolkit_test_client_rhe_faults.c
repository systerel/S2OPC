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

#include <stdbool.h>
#include <stdio.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define WRONG_ENDPOINT_URL "opc.tcp://localhost:6666"
#define REVERSE_ENDPOINT_URL "opc.tcp://localhost:4844"
#define CLIENT_APPLICATION_URI "urn:S2OPC:client"
#define SERVER_APPLICATION_URI "urn:S2OPC:localhost"
#define WRONG_APPLICATION_URI "urn:S2OPC:1000"
#define APPLICATION_NAME "S2OPC_TestClient"

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 200;
// Loop timeout in milliseconds
static const uint32_t loopTimeout4secs = 4000;
static const uint32_t loopTimeout2secs = 2000;

static char* preferred_locale_ids[] = {"en-US", "fr-FR", NULL};

static int32_t reverseEpClosedRequested = 0;
static int32_t reverseEpClosed = 0;
static int32_t sessionsActivatedContextVal = 0;
static int32_t sessionsClosed = 0;
static int32_t sessionFailureContextVal = 0;
static int32_t session = 0;
static uintptr_t sessionContext[3] = {1, 2, 3};

static void Test_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    SOPC_UNUSED_ARG(param);
    if (event == SE_ACTIVATED_SESSION)
    {
        SOPC_Atomic_Int_Set(&session, (int32_t) idOrStatus);
        SOPC_Atomic_Int_Set(&sessionsActivatedContextVal, (int32_t) appContext);
    }
    else if (event == SE_SESSION_ACTIVATION_FAILURE)
    {
        SOPC_Atomic_Int_Set(&sessionFailureContextVal, (int32_t) appContext);
    }
    else if (event == SE_CLOSED_SESSION)
    {
        SOPC_Atomic_Int_Add(&sessionsClosed, 1);
    }
    else if (event == SE_REVERSE_ENDPOINT_CLOSED)
    {
        if (0 == SOPC_Atomic_Int_Get(&reverseEpClosedRequested))
        {
            SOPC_ASSERT(false && "Unexpected reverse endpoint closure");
        }
        else
        {
            SOPC_Atomic_Int_Add(&reverseEpClosed, 1);
        }
    }
    else
    {
        SOPC_ASSERT(false);
    }
}

// A Secure channel connection configuration
SOPC_SecureChannel_Config scConfig = {.isClientSc = true,
                                      .clientConfigPtr = NULL,
                                      .expectedEndpoints = NULL,
                                      .serverUri = SERVER_APPLICATION_URI,
                                      .url = DEFAULT_ENDPOINT_URL,
                                      .peerAppCert = NULL,
                                      .reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI,
                                      .requestedLifetime = 20000,
                                      .msgSecurityMode = OpcUa_MessageSecurityMode_None};

static bool test_activate_session_without_reverse_ep(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx,
                                                     SOPC_SecureChannelConfigIdx secureChannelConfigIdx)
{
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_EndpointConnectionCfg endpointConnectionCfg =
        SOPC_EndpointConnectionCfg_CreateReverse(reverseEndpointConfigIdx, secureChannelConfigIdx);
    SOPC_ToolkitClient_AsyncActivateSession_Anonymous(endpointConnectionCfg, NULL, sessionContext[0], "anonymous");

    // An activation failure is expected without the expected reverse endpoint opened
    while ((SOPC_Atomic_Int_Get(&sessionsActivatedContextVal) + SOPC_Atomic_Int_Get(&sessionFailureContextVal)) == 0 &&
           loopCpt * sleepTimeout <= loopTimeout4secs)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout4secs)
    {
        return false;
    }
    else if (0 == SOPC_Atomic_Int_Get(&sessionsActivatedContextVal) &&
             SOPC_Atomic_Int_Get(&sessionFailureContextVal) == (int32_t) sessionContext[0])
    {
        // Retrieved context for failure is the one expected
        SOPC_Atomic_Int_Set(&sessionFailureContextVal, 0);
        return true;
    }

    return false;
}

// Note: secure channel configuration shall avoid possible connection with server (wrong URL, wrong URI)
static bool test_waiting_activate_session_and_close_ep(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx,
                                                       SOPC_SecureChannelConfigIdx secureChannelConfigIdx)
{
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_EndpointConnectionCfg endpointConnectionCfg =
        SOPC_EndpointConnectionCfg_CreateReverse(reverseEndpointConfigIdx, secureChannelConfigIdx);
    SOPC_ToolkitClient_AsyncActivateSession_Anonymous(endpointConnectionCfg, NULL, sessionContext[1], "anonymous");

    // No activation is expected nor failure since expected URL is not correct
    while ((SOPC_Atomic_Int_Get(&sessionsActivatedContextVal) + SOPC_Atomic_Int_Get(&sessionFailureContextVal)) == 0 &&
           loopCpt * sleepTimeout <= loopTimeout2secs)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout2secs)
    {
        // Timeout expected, now close the reverse endpoint
        SOPC_Atomic_Int_Set(&reverseEpClosedRequested, 1);
        SOPC_ToolkitClient_AsyncCloseReverseEndpoint(reverseEndpointConfigIdx);
    }
    else
    {
        return false;
    }

    // Wait the reverse endpoint to close and activation failure since secure connection was not yet established
    while ((SOPC_Atomic_Int_Get(&sessionsActivatedContextVal) + SOPC_Atomic_Int_Get(&sessionFailureContextVal)) == 0 &&
           SOPC_Atomic_Int_Get(&reverseEpClosed) == 0 && loopCpt * sleepTimeout <= loopTimeout4secs)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }
    if (loopCpt * sleepTimeout > loopTimeout4secs)
    {
        return false;
    }
    if (SOPC_Atomic_Int_Get(&sessionFailureContextVal) != (int32_t) sessionContext[1] ||
        SOPC_Atomic_Int_Get(&reverseEpClosed) == 0)
    {
        return false;
    }

    SOPC_Atomic_Int_Set(&reverseEpClosedRequested, 0);
    SOPC_Atomic_Int_Set(&reverseEpClosed, 0);
    SOPC_Atomic_Int_Set(&sessionFailureContextVal, 0);
    return true;
}

static bool test_activate_session_and_close_session(SOPC_ReverseEndpointConfigIdx reverseEndpointConfigIdx,
                                                    SOPC_SecureChannelConfigIdx secureChannelConfigIdx)
{
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_EndpointConnectionCfg endpointConnectionCfg =
        SOPC_EndpointConnectionCfg_CreateReverse(reverseEndpointConfigIdx, secureChannelConfigIdx);
    SOPC_ToolkitClient_AsyncActivateSession_Anonymous(endpointConnectionCfg, NULL, sessionContext[2], "anonymous");

    // No activation is expected nor failure since expected URL is not correct
    while ((SOPC_Atomic_Int_Get(&sessionsActivatedContextVal) + SOPC_Atomic_Int_Get(&sessionFailureContextVal)) == 0 &&
           loopCpt * sleepTimeout <= loopTimeout4secs)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout4secs)
    {
        // Timeout
        return false;
    }
    else if (SOPC_Atomic_Int_Get(&sessionsActivatedContextVal) != (int32_t) sessionContext[2])
    {
        return false;
    }

    // Close activated session
    SOPC_ToolkitClient_AsyncCloseSession((SOPC_SessionId) SOPC_Atomic_Int_Get(&session));
    while (SOPC_Atomic_Int_Get(&sessionsClosed) == 0 && loopCpt * sleepTimeout <= loopTimeout4secs)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout4secs)
    {
        // Timeout
        return false;
    }

    SOPC_Atomic_Int_Set(&sessionsActivatedContextVal, 0);
    SOPC_Atomic_Int_Set(&session, 0);
    SOPC_Atomic_Int_Set(&sessionsClosed, 0);

    return true;
}

int main(void)
{
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;
    // Test result
    bool result = false;

    // Define client application configuration
    SOPC_S2OPC_Config appConfig;
    SOPC_S2OPC_Config_Initialize(&appConfig);
    SOPC_Client_Config* clientAppConfig = &appConfig.clientConfig;
    clientAppConfig->clientDescription.ApplicationType = OpcUa_ApplicationType_Client;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    status = SOPC_String_AttachFromCstring(&clientAppConfig->clientDescription.ApplicationUri, CLIENT_APPLICATION_URI);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_String_AttachFromCstring(&clientAppConfig->clientDescription.ApplicationName.defaultText,
                                           APPLICATION_NAME);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    clientAppConfig->clientLocaleIds = preferred_locale_ids;
    scConfig.clientConfigPtr = clientAppConfig;

    SOPC_SecureChannel_Config channel_config_wrong_URL = scConfig;
    channel_config_wrong_URL.url = WRONG_ENDPOINT_URL;
    SOPC_SecureChannel_Config channel_config_wrong_URI = scConfig;
    channel_config_wrong_URI.serverUri = WRONG_APPLICATION_URI;

    SOPC_SecureChannelConfigIdx channel_config_URL_and_URI_idx = 0;
    SOPC_SecureChannelConfigIdx channel_config_wrong_URL_idx = 0;
    SOPC_SecureChannelConfigIdx channel_config_wrong_URI_idx = 0;

    SOPC_ReverseEndpointConfigIdx reverse_ep_config_idx = 0;

    /* Initialize SOPC_Common */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_rhe_faults_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_Common_Initialize(logConfiguration);
    }

    /* Init stack configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Test_ComEvent_FctClient);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client_RHE_faults: Failed initializing\n");
        }
        else
        {
            printf(">>Test_Client_RHE_faults: initialized\n");
        }
    }

    // Configure the secure channel connections to use and retrieve channel configuration indexes
    if (SOPC_STATUS_OK == status)
    {
        channel_config_URL_and_URI_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        channel_config_wrong_URL_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&channel_config_wrong_URL);
        channel_config_wrong_URI_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&channel_config_wrong_URI);
        if (channel_config_URL_and_URI_idx != 0 && channel_config_wrong_URL_idx != 0 &&
            channel_config_wrong_URI_idx != 0)
        {
            printf(">>Test_Client_RHE_faults: Client configured\n");
        }
        else
        {
            status = SOPC_STATUS_NOK;
            printf(">>Test_Client_RHE_faults: Client NOT configured\n");
        }
    }

    /* Create a Reverse Endpoint to create a Secure Channel through a connection initiated by server */
    if (SOPC_STATUS_OK == status)
    {
        reverse_ep_config_idx = SOPC_ToolkitClient_AddReverseEndpointConfig(REVERSE_ENDPOINT_URL);
        if (0 != reverse_ep_config_idx)
        {
            result = true;
        }
        else
        {
            printf(">>Test_Client_RHE_faults ERROR: failed to configure reverse endpoint\n");
        }
    }

    /* Test to activate a session without reverse endpoint opened */
    if (result)
    {
        result = test_activate_session_without_reverse_ep(reverse_ep_config_idx, channel_config_URL_and_URI_idx);

        if (!result)
        {
            printf(
                ">>Test_Client_RHE_faults ERROR: unexpected behavior when activating session on closed reverse EP\n");
        }
    }

    /* Open the reverse endpoint */
    if (result)
    {
        SOPC_ToolkitClient_AsyncOpenReverseEndpoint(reverse_ep_config_idx);
    }

    /* Test to activate a session with wrong server endpoint URL and to close the reverse endpoint */
    if (result)
    {
        result = test_waiting_activate_session_and_close_ep(reverse_ep_config_idx, channel_config_wrong_URL_idx);
        if (!result)
        {
            printf(
                ">>Test_Client_RHE_faults ERROR: unexpected behavior when closing reverse EP with secure connection "
                "waiting for wrong URL in RHE\n");
        }
    }

    /* Open the reverse endpoint */
    if (result)
    {
        SOPC_ToolkitClient_AsyncOpenReverseEndpoint(reverse_ep_config_idx);
    }

    /* Test to activate a session with wrong server URI and to close the reverse endpoint */
    if (result)
    {
        result = test_waiting_activate_session_and_close_ep(reverse_ep_config_idx, channel_config_wrong_URI_idx);
        if (!result)
        {
            printf(
                ">>Test_Client_RHE_faults ERROR: unexpected behavior when closing reverse EP with secure connection "
                "waiting for wrong URI in RHE\n");
        }
    }

    /* Open the reverse endpoint */
    if (result)
    {
        SOPC_ToolkitClient_AsyncOpenReverseEndpoint(reverse_ep_config_idx);
    }

    /* Test to activate a session with configured correct server URL and URI */
    if (result)
    {
        result = test_activate_session_and_close_session(reverse_ep_config_idx, channel_config_URL_and_URI_idx);
        if (!result)
        {
            printf(
                ">>Test_Client_RHE_faults ERROR: unexpected behavior when setting correct URL/URI in configuration\n");
        }
    }

    SOPC_Atomic_Int_Set(&reverseEpClosedRequested, 1);
    SOPC_ToolkitClient_AsyncCloseReverseEndpoint(reverse_ep_config_idx);

    // Wait the reverse endpoint to close and activation failure since secure connection was not yet established
    while (SOPC_Atomic_Int_Get(&reverseEpClosed) == 0 && loopCpt * sleepTimeout <= loopTimeout4secs)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    SOPC_Toolkit_Clear();

    // Clear locally allocated memory
    SOPC_S2OPC_Config_Clear(&appConfig);

    if (SOPC_STATUS_OK == status && result)
    {
        printf(">>Test_Client_RHE_faults final result: OK\n");
        return 0;
    }
    else
    {
        printf(">>Test_Client_RHE_faults final result: NOK\n");
        return 1;
    }
}
