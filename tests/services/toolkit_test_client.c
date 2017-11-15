/*
 *  Copyright (C) 2017 Systerel and others.
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "b2c.h"
#include "sopc_crypto_profiles.h"
#include "sopc_pki_stack.h"

#include "constants_bs.h"
#include "io_dispatch_mgr.h"
#include "toolkit_header_init.h"

#include "address_space_impl.h"
#include "test_results.h"
#include "util_b2c.h"

#include "opcua_statuscodes.h"
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

static bool sessionActivated = false;
static bool sessionClosed = false;
static constants__t_session_i session = constants__c_session_indet;

static uint32_t cptReadResps = 0;

void Test_ComEvent_FctClient(SOPC_App_Com_Event event, void* param, SOPC_StatusCode status)
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
                if (cptReadResps <= 1)
                {
                    test_results_set_service_result(test_read_request_response(readResp, status, 0) ? true : false);
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
                printf(">>Test_Client_Toolkit: received WriteResponse \n");
                OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) param;
                test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
            }
        }
    }
    else if (event == SE_ACTIVATED_SESSION)
    {
        sessionActivated = true;
        session = *(constants__t_session_i*) param;
    }
    else if (event == SE_SESSION_ACTIVATION_FAILURE || event == SE_CLOSED_SESSION)
    {
        sessionClosed = true;
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

SOPC_SecureChannel_Config scConfig = {.isClientSc = true,
                                      .url = ENDPOINT_URL,
                                      .crt_cli = NULL,
                                      .key_priv_cli = NULL,
                                      .crt_srv = NULL,
                                      .pki = NULL,
                                      .reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI,
                                      .requestedLifetime = 5,
                                      .msgSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt};

int main(void)
{
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 2000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_PKIProvider* pki = NULL;
    SOPC_Certificate *crt_cli = NULL, *crt_srv = NULL;
    SOPC_Certificate* crt_ca = NULL;
    SOPC_AsymmetricKey* priv_cli = NULL;

    constants__t_channel_config_idx_i channel_config_idx = constants__c_channel_config_idx_indet;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    OpcUa_WriteRequest* pWriteReq = NULL;

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Client private key
    char* keyLocation = "./client_private/client.key";

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

    // Configure the secure channel connection to use and retrieve channel configuration index
    if (SOPC_STATUS_OK == status)
    {
        channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        if (channel_config_idx != constants__c_channel_config_idx_indet)
        {
            status = SOPC_Toolkit_Configured();
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client_Toolkit: Failed to configure the secure channel\n");
        }
        else
        {
            printf(">>Test_Client_Toolkit: Client configured\n");
        }
    }

    /* Asynchronous request to create a session (and underlying secure channel if necessary). */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncActivateSession(channel_config_idx);
        printf(">>Test_Client_Toolkit: Creating/Activating session: OK\n");
    }

    /* Wait until session is activated or timeout */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && sessionActivated == false && sessionClosed == false &&
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

    if (sessionClosed == true)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status && sessionActivated != false)
    {
        printf(">>Test_Client_Toolkit: Session activated: OK'\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit: Session activated: NOK'\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session, getReadRequest_message());
        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(100);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends a WriteRequest */
        pWriteReq = tlibw_new_WriteRequest();
        test_results_set_WriteRequest(pWriteReq);
        // msg freed when sent
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session, pWriteReq);

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
        SOPC_Sleep(100);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends another ReadRequest, to verify that the AddS has changed */
        /* The callback will call the verification */
        // msg freed when sent
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session, getReadRequest_verif_message());

        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(100);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReq);

    /* Close the session */
    if (constants__c_session_indet != session)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session);
    }

    /* Wait until session is closed or timeout */
    loopCpt = 0;
    do
    {
        loopCpt++;
        SOPC_Sleep(100);
    } while (SOPC_STATUS_OK == status && sessionClosed == false && loopCpt * sleepTimeout <= loopTimeout);

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
        printf(">>Test_Client_Toolkit: read request not received or BAD status (%X) ! \n", status);
        printf(">>Test_Client_Toolkit final result: NOK\n");
        return status;
    }
}
