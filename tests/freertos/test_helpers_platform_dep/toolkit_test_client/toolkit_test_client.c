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

#include "sopc_crypto_profiles.h"
#include "sopc_pki_stack.h"

#include "test_results.h"

#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "testlib_write.h"
#include "wrap_read.h"

#include "test_results.h"
#include "testlib_read_response.h"

#include "embedded/loader.h"

#include "p_ethernet_if.h"
#include "p_sopc_log_srv.h"

#define ENDPOINT_URL "opc.tcp://192.168.1.102:4841"

static int32_t sessionsActivated = 0;
static int32_t sessionsClosed = 0;
static int32_t sendFailures = 0;
static uint32_t session = 0;
static uint32_t session2 = 0;
static uint32_t session3 = 0;
static uintptr_t sessionContext[3] = {1, 2, 3};
static uint32_t context2session[4] = {0, 0, 0, 0};

static int32_t getEndpointsReceived = 0;

#define NB_SESSIONS 3

static uint32_t cptReadResps = 0;

static void Test_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    uintptr_t sessionContext0 = (uintptr_t) SOPC_Atomic_Ptr_Get((void**) &sessionContext[0]);
    uintptr_t sessionContext1 = (uintptr_t) SOPC_Atomic_Ptr_Get((void**) &sessionContext[1]);
    uintptr_t sessionContext2 = (uintptr_t) SOPC_Atomic_Ptr_Get((void**) &sessionContext[2]);

    if (event == SE_RCV_SESSION_RESPONSE)
    {
        if (NULL != param)
        {
            SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;
            if (encType == &OpcUa_ReadResponse_EncodeableType)
            {
                printf(">>Test_Client_Toolkit: received ReadResponse \n");
                OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) param;
                cptReadResps++;
                // Check context value is same as those provided with request
                assert(cptReadResps == appContext);
                if (cptReadResps <= 1)
                {
                    test_results_set_service_result(
                        test_read_request_response(readResp, readResp->ResponseHeader.ServiceResult, 0) ? true : false);
                }
                else
                {
                    // Second read response is to test write effect (through read result)
                    test_results_set_service_result(
                        tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
                }
            }
            else if (encType == &OpcUa_WriteResponse_EncodeableType)
            {
                // Check context value is same as one provided with request
                assert(1 == appContext);
                printf(">>Test_Client_Toolkit: received WriteResponse \n");
                OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) param;
                test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
            }
        }
    }
    else if (event == SE_RCV_DISCOVERY_RESPONSE)
    {
        if (NULL != param)
        {
            // Check context value is same as one provided with request
            assert(1 == appContext);

            SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;
            if (encType == &OpcUa_GetEndpointsResponse_EncodeableType)
            {
                printf(">>Test_Client_Toolkit: received GetEndpointsResponse \n");
                bool validEndpoints = true;
                OpcUa_GetEndpointsResponse* getEndpointsResp = (OpcUa_GetEndpointsResponse*) param;

                if (getEndpointsResp->NoOfEndpoints <= 0)
                {
                    // At least one endpoint shall be described with the correct endpoint URL
                    validEndpoints = false;
                }

                SOPC_Atomic_Int_Add(&getEndpointsReceived, validEndpoints ? 1 : 0);
            }
        }
    }
    else if (event == SE_ACTIVATED_SESSION)
    {
        int n_sessions_activated = SOPC_Atomic_Int_Add(&sessionsActivated, 1);
        n_sessions_activated++; // SOPC_Atomic_Int_Add returns the old value

        // Check context value is same as one provided with activation request
        if (n_sessions_activated == 1)
        {
            SOPC_Atomic_Int_Set((int32_t*) &session, (int32_t) idOrStatus);
        }
        else if (n_sessions_activated == 2)
        {
            SOPC_Atomic_Int_Set((int32_t*) &session2, (int32_t) idOrStatus);
        }
        else if (n_sessions_activated == 3)
        {
            SOPC_Atomic_Int_Set((int32_t*) &session3, (int32_t) idOrStatus);
        }
        else
        {
            assert(false);
        }

        uint32_t session_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &context2session[appContext]);

        if (appContext != 0 &&
            (appContext == sessionContext0 || appContext == sessionContext1 || appContext == sessionContext2) &&
            session_idx == 0)
        {
            SOPC_Atomic_Int_Set((int32_t*) &context2session[appContext], (int32_t) idOrStatus);
        }
        else
        {
            // Invalid context
            assert(false);
        }
    }
    else if (event == SE_SESSION_ACTIVATION_FAILURE || event == SE_CLOSED_SESSION)
    {
        if (appContext != 0 &&
            (appContext == sessionContext0 || appContext == sessionContext1 || appContext == sessionContext2))
        {
            // Context valid but not yet associated to a session Id (never activated before failure)
            // OR context is the one associated to the session Id (activated once before failure)
            uint32_t session_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &context2session[appContext]);
            assert(session_idx == 0 || session_idx == idOrStatus);
        }
        else
        {
            // Invalid context
            assert(false);
        }
        SOPC_Atomic_Int_Add(&sessionsClosed, 1);
    }
    else if (event == SE_SND_REQUEST_FAILED)
    {
        SOPC_Atomic_Int_Add(&sendFailures, 1);
    }
    else
    {
        assert(false);
    }
}

/* Function to build the read service request message */
static void* getReadRequest_message(void)
{
    return read_new_read_request();
}

/* Function to build the verification read request */
static void* getReadRequest_verif_message(void)
{
    return tlibw_new_ReadRequest_check();
}

/* Function to build the getEndpoints service request message */
static void* getGetEndpoints_message(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_GetEndpointsRequest* getEndpointReq = NULL;
    status = SOPC_Encodeable_Create(&OpcUa_GetEndpointsRequest_EncodeableType, (void**) &getEndpointReq);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&getEndpointReq->EndpointUrl, ENDPOINT_URL);
    }
    return getEndpointReq;
}

// A Secure channel connection configuration
SOPC_SecureChannel_Config scConfig = {.isClientSc = true,
                                      .url = ENDPOINT_URL,
                                      .crt_cli = NULL,
                                      .key_priv_cli = NULL,
                                      .crt_srv = NULL,
                                      .pki = NULL,
                                      .reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI,
                                      .requestedLifetime = 20000,
                                      .msgSecurityMode = OpcUa_MessageSecurityMode_None};

int cbToolkit_test_client(void* arg)
{
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    (void) arg;

    SOPC_PKIProvider* pki = NULL;
    SOPC_SerializedCertificate *crt_cli = NULL, *crt_srv = NULL;
    SOPC_SerializedCertificate* crt_ca = NULL;
    SOPC_SerializedAsymmetricKey* priv_cli = NULL;

    uint32_t channel_config_idx = 0;
    uint32_t channel_config_idx2 = 0;
    uint32_t channel_config_idx3 = 0;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client_2k_cert.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server_2k_cert.der";
    // Client private key
    char* keyLocation = "./client_private/client_2k_key.pem";

    SOPC_LogSrv_Start(60, 4023);
    SOPC_LogSrv_WaitClient(UINT32_MAX);

    P_ETHERNET_IF_IsReady(UINT32_MAX);

    SOPC_AddressSpace* address_space = SOPC_Embedded_AddressSpace_Load();

    // Get Toolkit Configuration
    SOPC_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("toolkitVersion: %s\n", build_info.toolkitVersion);
    printf("toolkitSrcCommit: %s\n", build_info.toolkitSrcCommit);
    printf("toolkitDockerId: %s\n", build_info.toolkitDockerId);
    printf("toolkitBuildDate: %s\n", build_info.toolkitBuildDate);

    // If security mode is set, load certificates and key

    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None)
    {
        // The certificates: load
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certificateLocation, &crt_cli);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load client certificate\n");
        }
        else
        {
            printf(">>Stub_Client: Client certificate loaded\n");
            scConfig.crt_cli = crt_cli;
        }
    }

    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certificateSrvLocation, &crt_srv);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load server certificate\n");
        }
        else
        {
            printf(">>Stub_Client: Server certificate loaded\n");
            scConfig.crt_srv = crt_srv;
        }
    }

    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        // Private key: load
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(keyLocation, &priv_cli);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load private key\n");
        }
        else
        {
            printf(">>Stub_Client: Client private key loaded\n");
            scConfig.key_priv_cli = priv_cli;
        }
    }

    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        // Certificate Authority: load
        if (SOPC_STATUS_OK != SOPC_KeyManager_SerializedCertificate_CreateFromFile("./trusted/cacert.der", &crt_ca))
        {
            printf(">>Stub_Client: Failed to load CA\n");
        }
        else
        {
            printf(">>Stub_Client: CA certificate loaded\n");
        }
    }

    // Init PKI provider with certificate authority
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_STATUS_OK != SOPC_PKIProviderStack_Create(crt_ca, NULL, &pki))
        {
            printf(">>Stub_Client: Failed to create PKI\n");
        }
        else
        {
            printf(">>Stub_Client: PKI created\n");
            scConfig.pki = pki;
        }
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

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitConfig_SetCircularLogPath("./toolkit_test_client_logs/", true);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG);
    }

    // Set an address space (to check test result valid)
    if (SOPC_STATUS_OK == status)
    {
        // NECESSARY ONLY FOR TEST PURPOSES: a client should not define an @ space in a nominal case
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(address_space);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client_Toolkit: Failed to configure the @ space\n");
        }
        else
        {
            printf(">>Test_Client_Toolkit: @ space configured\n");
        }
    }

    // Configure the 3 secure channel connections to use and retrieve channel configuration index
    if (SOPC_STATUS_OK == status)
    {
        channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        channel_config_idx2 = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        channel_config_idx3 = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        if (channel_config_idx != 0 && channel_config_idx2 != 0 && channel_config_idx3 != 0)
        {
            status = SOPC_Toolkit_Configured();
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client_Toolkit: Failed to configure the secure channel connections\n");
        }
        else
        {
            printf(">>Test_Client_Toolkit: Client configured\n");
        }
    }

    /* Asynchronous request to get endpoints */
    if (SOPC_STATUS_OK == status)
    {
        // Use 1 as getEndpoints request context
        SOPC_ToolkitClient_AsyncSendDiscoveryRequest(channel_config_idx3, getGetEndpoints_message(), 1);
        printf(">>Test_Client_Toolkit: Get endpoints on 1 SC without session: OK\n");
    }

    /* Wait until get endpoints response or timeout */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&getEndpointsReceived) == 0 &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (SOPC_Atomic_Int_Get(&getEndpointsReceived) == 0 || SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        printf(">>Test_Client_Toolkit: GetEndpoints Response received: NOK\n");
        status = SOPC_STATUS_NOK;
    }
    else
    {
        printf(">>Test_Client_Toolkit: GetEndpoints Response received: OK\n");
    }

    /* Asynchronous request to create 3 sessions on 2 secure channels
     * (and underlying secure channel connections if necessary). */
    if (SOPC_STATUS_OK == status)
    {
        // Use 1, 2, 3 as session contexts
        SOPC_ToolkitClient_AsyncActivateSession_Anonymous(channel_config_idx, sessionContext[0], "anonymous");
        SOPC_ToolkitClient_AsyncActivateSession_Anonymous(channel_config_idx, sessionContext[1], "anonymous");
        SOPC_ToolkitClient_AsyncActivateSession_Anonymous(channel_config_idx2, sessionContext[2], "anonymous");
        printf(">>Test_Client_Toolkit: Creating/Activating 3 sessions on 2 SC: OK\n");
    }

    /* Wait until session is activated or timeout */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status &&
           (SOPC_Atomic_Int_Get(&sessionsActivated) + SOPC_Atomic_Int_Get(&sessionsClosed)) < NB_SESSIONS &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    if (SOPC_Atomic_Int_Get(&sessionsClosed) != 0 || SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionsActivated) == NB_SESSIONS)
    {
        printf(">>Test_Client_Toolkit: Sessions activated: OK'\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit: Sessions activated: NOK'\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session),
                                                     getReadRequest_message(), 1);
        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);

        // Create WriteRequest to be sent (deallocated by toolkit)
        pWriteReqSent = tlibw_new_WriteRequest(address_space);

        // Create same WriteRequest to check results on response reception
        pWriteReqCopy = tlibw_new_WriteRequest(address_space);

        test_results_set_WriteRequest(pWriteReqCopy);

        // Use 1 as write request context
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session), pWriteReqSent,
                                                     1);

        printf(">>Test_Client_Toolkit: write request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends another ReadRequest, to verify that the AddS has changed */
        /* The callback will call the verification */
        // msg freed when sent
        // Use 2 as read request context
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session),
                                                     getReadRequest_verif_message(), 2);

        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReqCopy);

    uint32_t session1_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session);
    uint32_t session2_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session2);
    uint32_t session3_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session3);

    /* Close the session */
    if (0 != session1_idx)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session1_idx);
    }

    if (0 != session2_idx)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session2_idx);
    }

    if (0 != session3_idx)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session3_idx);
    }

    /* Wait until session is closed or timeout */
    loopCpt = 0;
    do
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    } while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionsClosed) < NB_SESSIONS &&
             loopCpt * sleepTimeout <= loopTimeout);

    SOPC_Toolkit_Clear();

    // Clear locally allocated memory
    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(crt_cli);
        SOPC_KeyManager_SerializedCertificate_Delete(crt_srv);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(priv_cli);
        SOPC_KeyManager_SerializedCertificate_Delete(crt_ca);
        SOPC_PKIProvider_Free(&pki);
    }

    SOPC_AddressSpace_Delete(address_space);

    if (SOPC_STATUS_OK == status && test_results_get_service_result() != false)
    {
        printf(">>Test_Client_Toolkit: read request received ! \n");
        printf(">>Test_Client_Toolkit final result: OK\n");
        return 0;
    }
    else
    {
        printf(">>Test_Client_Toolkit: read request not received or BAD status (%d) ! \n", status);
        printf(">>Test_Client_Toolkit final result: NOK\n");
        return 1;
    }
}
