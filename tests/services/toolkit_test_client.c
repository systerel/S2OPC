/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_crypto_profiles.h"
#include "sopc_pki_stack.h"

#include "test_results.h"

#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"
#include "testlib_write.h"
#include "wrap_read.h"

#include "test_results.h"
#include "testlib_read_response.h"

#include "sopc_addspace.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"

static uint8_t sessionsActivated = 0;
static uint8_t sessionsClosed = 0;
static uint8_t sendFailures = 0;
static uint32_t session = 0;
static uint32_t session2 = 0;
static uint32_t session3 = 0;
static bool getEndpointsReceived = false;

#define NB_SESSIONS 3

static uint32_t cptReadResps = 0;

void Test_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
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
                SOPC_String endpointUrl;
                SOPC_String_Initialize(&endpointUrl);
                SOPC_ReturnStatus testStatus = SOPC_String_AttachFromCstring(&endpointUrl, ENDPOINT_URL);
                bool validEndpoints = true;
                OpcUa_GetEndpointsResponse* getEndpointsResp = (OpcUa_GetEndpointsResponse*) param;

                if (testStatus != SOPC_STATUS_OK || getEndpointsResp->NoOfEndpoints <= 0)
                {
                    // At least one endpoint shall be described with the correct endpoint URL
                    validEndpoints = false;
                }

                for (int32_t idx = 0; idx < getEndpointsResp->NoOfEndpoints && validEndpoints != false; idx++)
                {
                    validEndpoints = SOPC_String_Equal(&getEndpointsResp->Endpoints[idx].EndpointUrl, &endpointUrl);
                }

                getEndpointsReceived = validEndpoints;
            }
        }
    }
    else if (event == SE_ACTIVATED_SESSION)
    {
        sessionsActivated++;
        // Check context value is same as one provided with activation request
        if (sessionsActivated == 1)
        {
            session = idOrStatus;
        }
        else if (sessionsActivated == 2)
        {
            session2 = idOrStatus;
        }
        else if (sessionsActivated == 3)
        {
            session3 = idOrStatus;
        }
        else
        {
            assert(false);
        }
    }
    else if (event == SE_SESSION_ACTIVATION_FAILURE || event == SE_CLOSED_SESSION)
    {
        sessionsClosed++;
    }
    else if (event == SE_SND_REQUEST_FAILED)
    {
        sendFailures++;
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

#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // declare function to simulate threads activation
    bool SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(uint32_t msecTimeout);
    void TIS_Sockets_Dispatch (void);
    void TIS_AppEventDispatch (void);
    void TIS_ServicesEventDispatch (void);
    void TIS_SecureChannelsEventDispatch (void);
#endif
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
                                      .reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI,
                                      .requestedLifetime = 20000,
                                      .msgSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt};

int main(void)
{
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_PKIProvider* pki = NULL;
    SOPC_Certificate *crt_cli = NULL, *crt_srv = NULL;
    SOPC_Certificate* crt_ca = NULL;
    SOPC_AsymmetricKey* priv_cli = NULL;

    uint32_t channel_config_idx = 0;
    uint32_t channel_config_idx2 = 0;
    uint32_t channel_config_idx3 = 0;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    OpcUa_WriteRequest* pWriteReq = NULL;

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client_2k.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server_2k.der";
    // Client private key
    char* keyLocation = "./client_private/client_2k.key";

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
        status = SOPC_KeyManager_Certificate_CreateFromFile(certificateLocation, &crt_cli);
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
        status = SOPC_KeyManager_Certificate_CreateFromFile(certificateSrvLocation, &crt_srv);
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
        status = SOPC_KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_cli, NULL, 0);
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
        if (SOPC_STATUS_OK != SOPC_KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca))
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

    // Set an address space (to could check test result valid)
    if (SOPC_STATUS_OK == status)
    {
        // NECESSARY ONLY FOR TEST PURPOSES: a client should not define an @ space in a nominal case
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(&addressSpace);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client_Toolkit: Failed to configure the @ space\n");
        }
        else
        {
            printf(">>Test_Client_Toolkit: @ space configured\n");
        }
    }

    // Configure the 2 secure channel connections to use and retrieve channel configuration index
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
#ifdef __TRUSTINSOFT_DEBUG__
    printf("TIS:Test_Client_Toolkit: start loop 1\n");
#endif
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && getEndpointsReceived == false && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
#ifdef __TRUSTINSOFT_NO_MTHREAD__
      TIS_ServicesEventDispatch (); // APP_TO_SE_ACTIVATE_SESSION
      TIS_SecureChannelsEventDispatch (); // SC_CONNECT
      TIS_Sockets_Dispatch (); // SOCKET_CREATE_CLIENT
      SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10); // SOCKET_STATE_CONNECTING
      TIS_Sockets_Dispatch (); // INT_SOCKET_CONNECTED
      TIS_SecureChannelsEventDispatch (); // SOCKET_CONNECTION
      TIS_SecureChannelsEventDispatch (); // INT_SC_SND_HEL
      TIS_Sockets_Dispatch (); // SOCKET_WRITE
      SOPC_SocketsNetworkEventMgr_TreatSocketsEvents (10);
      TIS_Sockets_Dispatch (); // INT_SOCKET_READY_TO_READ
      TIS_SecureChannelsEventDispatch (); // SOCKET_RCV_BYTES
      TIS_SecureChannelsEventDispatch (); // INT_SC_RCV_ACK
      TIS_SecureChannelsEventDispatch(); // INT_SC_SND_OPN
      TIS_Sockets_Dispatch(); // SOCKET_WRITE
      SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(10); // -> INT_SOCKET_READY_TO_READ
      TIS_Sockets_Dispatch(); // INT_SOCKET_READY_TO_READ + recv
      TIS_SecureChannelsEventDispatch(); // SOCKET_RCV_BYTES
      TIS_SecureChannelsEventDispatch(); // INT_SC_RCV_OPN
      TIS_ServicesEventDispatch(); // SC_TO_SE_SC_CONNECTED
      TIS_ServicesEventDispatch(); // SE_TO_SE_CREATE_SESSION
      // pData_2
      TIS_SecureChannelsEventDispatch(); // SC_SERVICE_SND_MSG
      TIS_SecureChannelsEventDispatch(); // INT_SC_SND_MSG_CHUNKS
      // force_SOPC_Chunks_EncodePadding_realPaddingLength_2 invalid...
      TIS_Sockets_Dispatch(); // SOCKET_WRITE
      SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(10);
      TIS_Sockets_Dispatch(); // INT_SOCKET_READY_TO_READ
      // msg_3
      TIS_SecureChannelsEventDispatch(); // SOCKET_RCV_BYTES
      TIS_SecureChannelsEventDispatch(); // INT_SC_RCV_MSG_CHUNKS
      // <=== ok
      TIS_ServicesEventDispatch(); // SC_TO_SE_SC_SERVICE_RCV_MSG
#endif
        SOPC_Sleep(sleepTimeout);
    }
#ifdef __TRUSTINSOFT_DEBUG__
    printf("TIS:Test_Client_Toolkit: end loop 1\n");
#endif

    if (getEndpointsReceived == false || sendFailures > 0)
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
        SOPC_ToolkitClient_AsyncActivateSession(channel_config_idx, 1);
        SOPC_ToolkitClient_AsyncActivateSession(channel_config_idx, 2);
        SOPC_ToolkitClient_AsyncActivateSession(channel_config_idx2, 3);
        printf(">>Test_Client_Toolkit: Creating/Activating 3 sessions on 2 SC: OK\n");
    }

    /* Wait until session is activated or timeout */
#ifdef __TRUSTINSOFT_DEBUG__
    printf("TIS:Test_Client_Toolkit: start loop 2\n");
#endif
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && (sessionsActivated + sessionsClosed) < NB_SESSIONS &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
#ifdef __TRUSTINSOFT_NO_MTHREAD__
      TIS_ServicesEventDispatch(); // SE_TO_SE_ACTIVATE_SESSION
      TIS_AppEventDispatch();
      TIS_SecureChannelsEventDispatch();
      TIS_SecureChannelsEventDispatch();
      TIS_Sockets_Dispatch();
      SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(10);
      TIS_Sockets_Dispatch();
      TIS_SecureChannelsEventDispatch();
      TIS_SecureChannelsEventDispatch();
      TIS_ServicesEventDispatch();
      TIS_AppEventDispatch();

//       sessionActivated = true;
//     }
#endif
        SOPC_Sleep(sleepTimeout);
    }
#ifdef __TRUSTINSOFT_DEBUG__
    printf("TIS:Test_Client_Toolkit: end loop 2\n");
#endif

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    if (sessionsClosed != 0 || sendFailures > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status && sessionsActivated == NB_SESSIONS)
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
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session, getReadRequest_message(), 1);
        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
#ifdef __TRUSTINSOFT_DEBUG__
    printf("TIS:Test_Client_Toolkit: start loop 3\n");
#endif
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // dummy response
    TIS_ServicesEventDispatch();
    TIS_SecureChannelsEventDispatch();
    TIS_SecureChannelsEventDispatch();
    TIS_Sockets_Dispatch();
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(10);
    TIS_Sockets_Dispatch();
    TIS_SecureChannelsEventDispatch();
    TIS_SecureChannelsEventDispatch();
    TIS_AppEventDispatch();
    SOPC_SocketsNetworkEventMgr_TreatSocketsEvents(10);
//     int tis_nondet(int a, int b);
//     if (tis_nondet (0, 1))
//       test_results_set_service_result (true);
#endif
        SOPC_Sleep(sleepTimeout);
    }
#ifdef __TRUSTINSOFT_DEBUG__
    printf("TIS:Test_Client_Toolkit: end loop 3\n");
#endif

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (sendFailures > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends a WriteRequest */
        pWriteReq = tlibw_new_WriteRequest();
        test_results_set_WriteRequest(pWriteReq);
        // msg freed when sent
        // Use 1 as write request context
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session, pWriteReq, 1);

        /* Same data must be provided to verify result, since request will be freed on sending allocate a new (same
         * content) */
        pWriteReq = tlibw_new_WriteRequest();
        test_results_set_WriteRequest(pWriteReq);

        printf(">>Test_Client_Toolkit: write request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // dummy response
    int tis_nondet(int a, int b);
    if (tis_nondet (0, 1))
      test_results_set_service_result (true);
#endif
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (sendFailures > 0)
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
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session, getReadRequest_verif_message(), 2);

        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // dummy response
    int tis_nondet(int a, int b);
    if (tis_nondet (0, 1))
      test_results_set_service_result (true);
#endif
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (sendFailures > 0)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReq);

    /* Close the session */
    if (0 != session)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session);
    }

    if (0 != session2)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session2);
    }

    if (0 != session3)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session3);
    }

    /* Wait until session is closed or timeout */
    loopCpt = 0;
    do
    {
        loopCpt++;
#ifdef __TRUSTINSOFT_NO_MTHREAD__
    // dummy 'sessionsClosed'
    int tis_nondet(int a, int b);
    if (tis_nondet (0, 1))
      sessionsClosed++;
#endif
        SOPC_Sleep(sleepTimeout);
    } while (SOPC_STATUS_OK == status && sessionsClosed < NB_SESSIONS && loopCpt * sleepTimeout <= loopTimeout);

    SOPC_Toolkit_Clear();

    // Clear locally allocated memory
    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None)
    {
        SOPC_KeyManager_Certificate_Free(crt_cli);
        SOPC_KeyManager_Certificate_Free(crt_srv);
        SOPC_KeyManager_AsymmetricKey_Free(priv_cli);
        SOPC_KeyManager_Certificate_Free(crt_ca);
        SOPC_PKIProviderStack_Free(pki);
    }

    if (SOPC_STATUS_OK == status && test_results_get_service_result() != false)
    {
        printf(">>Test_Client_Toolkit: read request received ! \n");
        printf(">>Test_Client_Toolkit final result: OK\n");
        return 0;
    }
    else
    {
        printf(">>Test_Client_Toolkit: read request not received or BAD status (%" PRIu32 ") ! \n", status);
        printf(">>Test_Client_Toolkit final result: NOK\n");
        return status;
    }
}
