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

#include "fuzz_mgr__receive_msg_buffer.h"
#include "fuzz_mgr__receive_msg_buffer_client.h"
#include "fuzz_mgr__receive_msg_buffer_server.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

#define NB_SESSIONS 1

static t_CerKey ck;

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

// Configure the 2 secure channel connections to use and retrieve channel configuration index

SOPC_ReturnStatus CerAndKeyLoader_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config epConfig;

    if (secuActive)
    {
        //#ifdef WITH_STATIC_SECURITY_DATA
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(client_2k_cert, sizeof(client_2k_cert),
                                                                     &(ck).Certificate);
        epConfig.serverCertificate = (ck).Certificate;

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(client_2k_key),
                                                                            &&(ck).Key);
            epConfig.serverKey = (ck).Key;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &(ck).authCertificate);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(&(ck).authCertificate, NULL, &(ck).pkiProvider);
            epConfig.pki = (ck).pkiProvider;
        }
        //#endif
        if (SOPC_STATUS_OK != status)
        {
            if (true == debug)
            {
                printf("<Test_Server_Toolkit: Failed loading certificates and key (check paths are valid)\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf("<Test_Server_Toolkit: Certificates and key loaded\n");
            }
        }
    }
    else
    {
        epConfig.serverCertificate = NULL;
        epConfig.serverKey = NULL;
        epConfig.pki = NULL;
    }

    if (secuActive)
    {
        epConfig.nbSecuConfigs = 2;
    }
    else
    {
        epConfig.nbSecuConfigs = 1;
    }
    //#endif
    return (status);
}

static void setScConfig_client(bool onSecu)
{
    if (true == onSecu)
    {
        scConfig->isClientSc = true;
        scConfig->url = ENDPOINT_URL;
        scConfig->crt_cli = NULL;
        scConfig->key_priv_cli = NULL;
        scConfig->crt_srv = NULL;
        scConfig->pki = NULL;
        scConfig->reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI;
        scConfig->requestedLifetime = 20000;
        scConfig->msgSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
    }
    else
    {
        scConfig->isClientSc = true;
        scConfig->url = ENDPOINT_URL;
        scConfig->crt_cli = NULL;
        scConfig->key_priv_cli = NULL;
        scConfig->crt_srv = NULL;
        scConfig->pki = NULL;
        scConfig->reqSecuPolicyUri = SOPC_SecurityPolicy_None_URI;
        scConfig->requestedLifetime = 20000;
        scConfig->msgSecurityMode = OpcUa_MessageSecurityMode_None;
    }
}

static SOPC_ReturnStatus ActivateSessionWait_client(uint32_t* channel_config_idx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    /* Wait until session is activated or timeout */
    while ((SOPC_Atomic_Int_Get(&session) != 0) && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    if (SOPC_Atomic_Int_Get(&scState) != SESSION_CONN_CONNECTED || SOPC_Atomic_Int_Get(&sendFailures) > 0)
    {
        if (true == debug)
        {
            printf(">>Test_Client_Toolkit: Sessions activated: OK'\n");
        }
    }
    else
    {
        if (true == debug)
        {
            printf(">>Test_Client_Toolkit: Sessions activated: NOK'\n");
        }
        status = SOPC_STATUS_NOK;
    }
    return (status);
}

SOPC_ReturnStatus Setup_client()
{
    uint32_t channel_config_idx;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Certificate and key serialization
    status = CerAndKeyLoader_client();
    if (SOPC_STATUS_OK == status)
    {
        setScConfig_client(false);
        if (true == debug)
        {
            printf(">>FUZZ_Client: Certificate, key and Sc configured successfully\n");
        }
    }

    if (true == secuActive)
    {
        // sc config a généré
        channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        if (channel_config_idx != 0)
        {
            if (true == debug)
            {
                printf(">>FUZZ_Client:  Client configured\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf(">>FUZZ_Client:  Failed to configure the secure channel connections\n");
            }
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitClient_AsyncActivateSession_Anonymous(channel_config_idx, 1, "anonymous");
        if (SOPC_STATUS_OK == status)
        {
            if (true == debug)
            {
                printf(">>FUZZ_Client: Creating/Activating 1 sessions on 1 SC: OK\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf(">>FUZZ_Client: Failed to create/Activate session\n");
            }
        }
    }
    ActivateSessionWait_client(&channel_config_idx);
    return (status);
}

SOPC_ReturnStatus Wait_response_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

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
    return (status);
}

SOPC_ReturnStatus Send_client() // replace write request for msgbuffer ?
{
    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

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

        if (true == debug)
        {
            printf(">>FUZZ_Client: write request sending\n");
        }
    }
    status = Wait_response_client();
    return (status);
}

SOPC_ReturnStatus Run_client()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        SOPC_ToolkitClient_AsyncSendRequestOnSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session),
                                                     getReadRequest_message(), 1);
        if (true == debug)
        {
            printf(">>FUZZ_Client: read request sending\n");
        }
    }

    status = Wait_response_client();
    status = Send_client();
    status = Wait_response_client();

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

        if (true == debug)
        {
            printf(">>FUZZ_Client: read request sending\n");
        }
    }

    status = Wait_response_client();
    return (status);
}

SOPC_ReturnStatus Teardown_client()
{
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReqCopy); // replace write request for msgbuffer ?

    uint32_t session1_idx = (uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session);

    /* Close the session */
    if (0 != session1_idx)
    {
        SOPC_ToolkitClient_AsyncCloseSession(session1_idx);
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
    if (scConfig.msgSecurityMode != OpcUa_MessageSecurityMode_None) // changer le test
    {
        SOPC_KeyManager_SerializedCertificate_Delete(ck.Certificate);
        //        SOPC_KeyManager_SerializedCertificate_Delete(crt_srv);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(ck.Key);
        SOPC_KeyManager_SerializedCertificate_Delete(ck.authCertificate);
        SOPC_PKIProvider_Free(&(ck).pkiProvider);
    }

    SOPC_AddressSpace_Delete(address_space);

    if (SOPC_STATUS_OK == status && test_results_get_service_result() != false)
    {
        if (true == debug)
        {
            printf(">>FUZZ_Client: read request received ! \n");
            printf(">>FUZZ_Client: final result: OK\n");
        }
        return 0;
    }
    else
    {
        if (true == debug)
        {
            printf(">>FUZZ_Client: read request not received or BAD status (%" PRIu32 ") ! \n", status);
            printf(">>FUZZ_Client: final result: NOK\n");
        }
        return 1;
    }
    return (status);
}
