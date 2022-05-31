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

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_pki_stack.h"

#include "test_results.h"

#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"

#define UNEXPECTED_ERROR 1
#define UNEXPECTED_STATUS_CODE 2
#define INVALID_ACTIVATION_SERVICE_FAULT 2
#define INVALID_SESSION_SERVICE_FAULT 3
#define INVALID_DISCOVERY_SERVICE_FAULT 4
#define INVALID_SESSION_CLOSING 5
#define TIMEOUT 6

int32_t sessionActivationFault = 0;
int32_t sessionClosedNominal = 0;
int32_t sessionActivated = 0;
int32_t sessionServiceFault = 0;
int32_t discoveryServiceFault = 0;
int32_t testStatus = 0;

static void Test_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    assert(appContext == 1); // Shall always 1 in this test
    if (event == SE_RCV_SESSION_RESPONSE)
    {
        printf(">>Test_Client_Toolkit: SE_RCV_SESSION_RESPONSE verification of service fault\n");

        if (NULL == param)
        {
            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_ERROR);
            return;
        }

        SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;

        if (encType != &OpcUa_ServiceFault_EncodeableType)
        {
            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_ERROR);
            return;
        }

        OpcUa_ServiceFault* serviceFault = (OpcUa_ServiceFault*) param;
        if (OpcUa_BadNothingToDo != serviceFault->ResponseHeader.ServiceResult)
        {
            printf(">>Test_Client_Toolkit: invalid reason %X !!!! \n", serviceFault->ResponseHeader.ServiceResult);

            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_STATUS_CODE);
            return;
        }
        SOPC_Atomic_Int_Set(&sessionServiceFault, true);
    }
    else if (event == SE_RCV_DISCOVERY_RESPONSE)
    {
        printf(">>Test_Client_Toolkit: SE_RCV_DISCOVERY_RESPONSE verification of service fault\n");

        if (NULL == param)
        {
            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_ERROR);
            return;
        }

        SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;

        if (encType != &OpcUa_ServiceFault_EncodeableType)
        {
            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_ERROR);
            return;
        }

        OpcUa_ServiceFault* serviceFault = (OpcUa_ServiceFault*) param;
        if (OpcUa_BadServiceUnsupported != serviceFault->ResponseHeader.ServiceResult)
        {
            printf(">>Test_Client_Toolkit: invalid reason %X !!!! \n", serviceFault->ResponseHeader.ServiceResult);

            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_STATUS_CODE);
            return;
        }
        SOPC_Atomic_Int_Set(&discoveryServiceFault, true);
    }
    else if (event == SE_ACTIVATED_SESSION)
    {
        assert(idOrStatus <= INT32_MAX);
        SOPC_Atomic_Int_Set(&sessionActivated, (int32_t) idOrStatus);
    }
    else if (event == SE_SESSION_ACTIVATION_FAILURE)
    {
        printf(">>Test_Client_Toolkit: SE_SESSION_ACTIVATION_FAILURE for service fault\n");

        SOPC_StatusCode scReason = (SOPC_StatusCode)(uintptr_t) param;

        if (SOPC_Atomic_Int_Get(&sessionActivationFault) == false)
        {
            if (scReason != OpcUa_BadIdentityTokenInvalid && scReason != OpcUa_BadIdentityTokenRejected)
            {
                printf(">>Test_Client_Toolkit: invalid reason %X !!!! \n", scReason);
                SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_STATUS_CODE);
                return;
            }

            SOPC_Atomic_Int_Set(&sessionActivationFault, true);
        }
        else
        {
            printf(">>Test_Client_Toolkit: unexpected session activation failure with reason %X !!!! \n", scReason);
            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_ERROR);
        }
    }
    else if (event == SE_CLOSED_SESSION)
    {
        printf(">>Test_Client_Toolkit: SE_CLOSED_SESSION for terminating test\n");

        SOPC_StatusCode scReason = (SOPC_StatusCode)(uintptr_t) param;
        if ((scReason & SOPC_GoodStatusOppositeMask) != 0)
        {
            // Closed with Bad status code reason
            printf(">>Test_Client_Toolkit: invalid reason %X !!!! \n", scReason);
            SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_STATUS_CODE);
            return;
        }

        SOPC_Atomic_Int_Set(&sessionClosedNominal, true);
    }
    else if (event == SE_SND_REQUEST_FAILED)
    {
        printf(">>Test_Client_Toolkit: SE_SND_REQUEST_FAILED\n");

        SOPC_Atomic_Int_Set(&testStatus, UNEXPECTED_ERROR);
        return;
    }
    else
    {
        assert(false);
    }
}

// A Secure channel connection configuration
SOPC_SecureChannel_Config scConfig = {.isClientSc = true,
                                      .clientConfigPtr = NULL,
                                      .expectedEndpoints = NULL,
                                      .serverUri = NULL,
                                      .url = DEFAULT_ENDPOINT_URL,
                                      .crt_cli = NULL,
                                      .key_priv_cli = NULL,
                                      .crt_srv = NULL,
                                      .pki = NULL,
                                      .reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI,
                                      .requestedLifetime = 20000,
                                      .msgSecurityMode = OpcUa_MessageSecurityMode_None};

int main(void)
{
    int mainResult = UNEXPECTED_ERROR;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    uint32_t channel_config_idx = 0;
    SOPC_PKIProvider* pki = NULL;

    /* Initialize SOPC_Common */

    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_service_faults_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);
    if (SOPC_STATUS_OK != status)
    {
        printf(">>Test_Client_Toolkit: Common initialization Failed\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit: Common initialized\n");
    }

    /* Init stack configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Test_ComEvent_FctClient);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client_Toolkit: Failed initializing\n");
        }
        else
        {
            printf(">>Test_Client_Toolkit: Stack initialized\n");
        }
    }

    // Init PKI provider with certificate authority
    if (SOPC_STATUS_OK == status)
    {
        char* lPathsTrustedRoots[] = {"./trusted/cacert.der", NULL};
        char* lPathsTrustedLinks[] = {NULL};
        char* lPathsUntrustedRoots[] = {NULL};
        char* lPathsUntrustedLinks[] = {NULL};
        char* lPathsIssuedCerts[] = {NULL};
        char* lPathsCRL[] = {"./revoked/cacrl.der", NULL};
        status = SOPC_PKIProviderStack_CreateFromPaths(lPathsTrustedRoots, lPathsTrustedLinks, lPathsUntrustedRoots,
                                                       lPathsUntrustedLinks, lPathsIssuedCerts, lPathsCRL, &pki);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to create PKI\n");
        }
        else
        {
            printf(">>Stub_Client: PKI created\n");
            scConfig.pki = pki;
        }
    }

    // Configure the secure channel connection and retrieve associated channel configuration index
    if (SOPC_STATUS_OK == status)
    {
        channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);

        if (channel_config_idx != 0)
        {
            printf(">>Test_Client_Toolkit: Client configured\n");
        }
        else
        {
            status = SOPC_STATUS_NOK;
            printf(">>Test_Client_Toolkit: Failed to configure the secure channel connections\n");
        }
    }

    /*
     * Testing service fault response on activate session service request
     */

    /* Asynchronous request to connect with invalid user identity */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
            channel_config_idx, NULL, 1, SOPC_UserTokenPolicy_UserNameBasic256Sha256_ID, "wrongUser",
            (const uint8_t*) "noPassword", (int32_t) strlen("noPassword"));
        printf(">>Test_Client_Toolkit: Creating/Activating 1 session with invalid user identity\n");
    }

    /* Wait until session is activated or timeout */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionActivationFault) == false &&
           SOPC_Atomic_Int_Get(&testStatus) == 0 && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        mainResult = TIMEOUT;
        status = SOPC_STATUS_TIMEOUT;
    }

    if (SOPC_Atomic_Int_Get(&testStatus) != 0 || SOPC_Atomic_Int_Get(&sessionActivationFault) == false)
    {
        status = SOPC_STATUS_NOK;
        if (SOPC_Atomic_Int_Get(&testStatus) == 0)
        {
            mainResult = INVALID_ACTIVATION_SERVICE_FAULT;
        }
        else
        {
            mainResult = SOPC_Atomic_Int_Get(&testStatus);
        }
    }

    /*
     * Activating a session
     */

    /* Asynchronous request to connect with valid user identity */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Atomic_Int_Set(&sessionActivationFault, 0);
        // Use 1, 2, 3 as session contexts
        SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
            channel_config_idx, NULL, 1, SOPC_UserTokenPolicy_UserNameBasic256Sha256_ID, "user1",
            (const uint8_t*) "password", (int32_t) strlen("password"));
        printf(">>Test_Client_Toolkit: Creating/Activating 1 session with valid user identity\n");
    }

    /* Wait until session is activated or timeout */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionActivated) == 0 &&
           SOPC_Atomic_Int_Get(&sessionActivationFault) == false && SOPC_Atomic_Int_Get(&testStatus) == 0 &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
        mainResult = TIMEOUT;
    }

    if (SOPC_Atomic_Int_Get(&sessionActivated) == 0 || SOPC_Atomic_Int_Get(&sessionActivationFault) != false ||
        SOPC_Atomic_Int_Get(&testStatus) != 0)
    {
        status = SOPC_STATUS_NOK;

        if (SOPC_Atomic_Int_Get(&testStatus) == 0)
        {
            mainResult = INVALID_SESSION_SERVICE_FAULT;
        }
        else
        {
            mainResult = SOPC_Atomic_Int_Get(&testStatus);
        }
    }

    /*
     * Testing service fault response on a service request on session
     */

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message with nothing to do and send it through session (read service)*/
        OpcUa_ReadRequest* emptyReadRequest = NULL;
        SOPC_Encodeable_Create(&OpcUa_ReadRequest_EncodeableType, (void**) &emptyReadRequest);

        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get(&sessionActivated),
                                                     emptyReadRequest, 1);
        printf(">>Test_Client_Toolkit: empty read request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionServiceFault) == false &&
           SOPC_Atomic_Int_Get(&testStatus) == 0 && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
        mainResult = TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&sessionServiceFault) == false || SOPC_Atomic_Int_Get(&testStatus) != 0)
    {
        status = SOPC_STATUS_NOK;

        if (SOPC_Atomic_Int_Get(&testStatus) == 0)
        {
            mainResult = INVALID_SESSION_SERVICE_FAULT;
        }
        else
        {
            mainResult = SOPC_Atomic_Int_Get(&testStatus);
        }
    }

    /*
     * Testing service fault response on discovery request sending (using session interface)
     */

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message with nothing to do and send it through session (read service)*/
        OpcUa_RegisterServerRequest* notSupportedServiceReq = NULL;
        SOPC_Encodeable_Create(&OpcUa_RegisterServerRequest_EncodeableType, (void**) &notSupportedServiceReq);

        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get(&sessionActivated),
                                                     notSupportedServiceReq, 1);
        printf(">>Test_Client_Toolkit: unsupported discovery request sending (using session API)\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&discoveryServiceFault) == false &&
           SOPC_Atomic_Int_Get(&testStatus) == 0 && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
        mainResult = TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&discoveryServiceFault) == false || SOPC_Atomic_Int_Get(&testStatus) != 0)
    {
        status = SOPC_STATUS_NOK;

        if (SOPC_Atomic_Int_Get(&testStatus) == 0)
        {
            mainResult = INVALID_DISCOVERY_SERVICE_FAULT;
        }
        else
        {
            mainResult = SOPC_Atomic_Int_Get(&testStatus);
        }
    }

    /*
     * Testing service fault response on discovery request sending (using discovery interface)
     */

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Atomic_Int_Set(&discoveryServiceFault, false); // Reset the flag

        /* Create a service request message with nothing to do and send it through session (read service)*/
        OpcUa_RegisterServerRequest* notSupportedServiceReq = NULL;
        SOPC_Encodeable_Create(&OpcUa_RegisterServerRequest_EncodeableType, (void**) &notSupportedServiceReq);

        SOPC_ToolkitClient_AsyncSendDiscoveryRequest(channel_config_idx, notSupportedServiceReq, 1);
        printf(">>Test_Client_Toolkit: unsupported discovery request sending (using discovery API)\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&discoveryServiceFault) == false &&
           SOPC_Atomic_Int_Get(&testStatus) == 0 && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
        mainResult = TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&discoveryServiceFault) == false || SOPC_Atomic_Int_Get(&testStatus) != 0)
    {
        status = SOPC_STATUS_NOK;

        if (SOPC_Atomic_Int_Get(&testStatus) == 0)
        {
            mainResult = INVALID_DISCOVERY_SERVICE_FAULT;
        }
        else
        {
            mainResult = SOPC_Atomic_Int_Get(&testStatus);
        }
    }

    /*
     * Closing session
     */
    if (0 != SOPC_Atomic_Int_Get(&sessionActivated))
    {
        SOPC_ToolkitClient_AsyncCloseSession((uint32_t) SOPC_Atomic_Int_Get(&sessionActivated));
    }

    /* Wait until session is closed or timeout */
    loopCpt = 0;
    do
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    } while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionClosedNominal) == false &&
             SOPC_Atomic_Int_Get(&testStatus) == 0 && loopCpt * sleepTimeout <= loopTimeout);

    if (SOPC_STATUS_OK == status)
    {
        if (loopCpt * sleepTimeout > loopTimeout)
        {
            status = SOPC_STATUS_TIMEOUT;
            mainResult = TIMEOUT;
        }
        else if (SOPC_Atomic_Int_Get(&sessionClosedNominal) == false || SOPC_Atomic_Int_Get(&testStatus) != false)
        {
            status = SOPC_STATUS_NOK;

            if (SOPC_Atomic_Int_Get(&testStatus) == 0)
            {
                mainResult = INVALID_SESSION_CLOSING;
            }
            else
            {
                mainResult = SOPC_Atomic_Int_Get(&testStatus);
            }
        }
    }

    SOPC_Toolkit_Clear();
    SOPC_PKIProvider_Free(&pki);

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit service faults final result: OK\n");
        return 0;
    }
    else
    {
        printf(">>Test_Client_Toolkit service faults final result: NOK\n");
        return mainResult;
    }
}
