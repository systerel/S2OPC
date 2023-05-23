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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_new_client.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encodeable.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "test_results.h"
#include "testlib_read_response.h"
#include "testlib_write.h"
#include "wrap_read.h"

#include "embedded/sopc_addspace_loader.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define REVERSE_ENDPOINT_URL "opc.tcp://localhost:4844"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define APPLICATION_NAME "S2OPC_TestClient"

#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256

// Client certificate path
#define CLI_CERT_PATH "./client_public/client_2k_cert.der"
// Server certificate path
#define SRV_CERT_PATH "./server_public/server_2k_cert.der"
// Client private key path
#define CLI_KEY_PATH "./client_private/encrypted_client_2k_key.pem"

// User certificate path
#define USER_CERT_PATH "./user_public/user_2k_cert.der"
// User key path
#define USER_KEY_PATH "./user_private/encrypted_user_2k_key.pem"

static const char* preferred_locale_ids[] = {"en-US", "fr-FR", NULL};

static int32_t getEndpointsReceived = 0;

#define NB_SESSIONS 3

static uint32_t cptReadResps = 0;

#if S2OPC_NANO_PROFILE
#define TEST_SUB_SERVICE_UNSUPPORTED true
#else
#define TEST_SUB_SERVICE_UNSUPPORTED false
#endif

static void SOPC_Client_AsyncRespCb(SOPC_EncodeableType* encType, const void* response, uintptr_t appContext)
{
    if (encType == &OpcUa_ReadResponse_EncodeableType)
    {
        printf(">>Test_Client_Toolkit: received ReadResponse \n");
        const OpcUa_ReadResponse* readResp = (const OpcUa_ReadResponse*) response;
        cptReadResps++;
        // Check context value is same as those provided with request
        SOPC_ASSERT(cptReadResps == appContext);
        if (cptReadResps <= 1)
        {
            test_results_set_service_result(
                test_read_request_response(readResp, readResp->ResponseHeader.ServiceResult, 0) ? true : false);
        }
        else
        {
            // Second read response is to test write effect (through read result)
            test_results_set_service_result(tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
        }
    }
    else if (encType == &OpcUa_WriteResponse_EncodeableType)
    {
        // Check context value is same as one provided with request
        SOPC_ASSERT(1 == appContext);
        printf(">>Test_Client_Toolkit: received WriteResponse \n");
        const OpcUa_WriteResponse* writeResp = (const OpcUa_WriteResponse*) response;
        test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
    }
    else if (encType == &OpcUa_GetEndpointsResponse_EncodeableType)
    {
        // Check context value is same as one provided with request
        SOPC_ASSERT(1 == appContext);

        printf(">>Test_Client_Toolkit: received GetEndpointsResponse \n");
        SOPC_String endpointUrl;
        SOPC_String_Initialize(&endpointUrl);
        SOPC_ReturnStatus testStatus = SOPC_String_AttachFromCstring(&endpointUrl, DEFAULT_ENDPOINT_URL);
        bool validEndpoints = true;
        const OpcUa_GetEndpointsResponse* getEndpointsResp = (const OpcUa_GetEndpointsResponse*) response;

        if (testStatus != SOPC_STATUS_OK || getEndpointsResp->NoOfEndpoints <= 0)
        {
            // At least one endpoint shall be described with the correct endpoint URL
            validEndpoints = false;
        }

        for (int32_t idx = 0; idx < getEndpointsResp->NoOfEndpoints && validEndpoints != false; idx++)
        {
            validEndpoints = SOPC_String_Equal(&getEndpointsResp->Endpoints[idx].EndpointUrl, &endpointUrl);
        }

        SOPC_Atomic_Int_Add(&getEndpointsReceived, validEndpoints ? 1 : 0);
    }
    else if (encType == &OpcUa_ServiceFault_EncodeableType)
    {
        printf(">>Test_Client_Toolkit: received ServiceFault \n");
        const OpcUa_ServiceFault* serviceFaultResp = (const OpcUa_ServiceFault*) response;
        test_results_set_service_result(OpcUa_BadServiceUnsupported == appContext &&
                                        OpcUa_BadServiceUnsupported == serviceFaultResp->ResponseHeader.ServiceResult);
    }
}

static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    SOPC_ASSERT(false && "Unexpected connection event");
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
        status = SOPC_String_AttachFromCstring(&getEndpointReq->EndpointUrl, DEFAULT_ENDPOINT_URL);
    }
    return getEndpointReq;
}

static bool lastConnectionTypeIsClassic = false;
static SOPC_ReturnStatus SOPC_SetReverseConnection(SOPC_SecureConnection_Config* config, unsigned int connectionType)
{
    switch (connectionType)
    {
    case 0: // Classic
        return SOPC_STATUS_OK;
    case 1: // Reverse
        return SOPC_SecureConnectionConfig_SetReverseConnection(config, REVERSE_ENDPOINT_URL);
    default: // Alternate classic and reverse
        if (lastConnectionTypeIsClassic)
        {
            lastConnectionTypeIsClassic = false;
            return SOPC_SecureConnectionConfig_SetReverseConnection(config, REVERSE_ENDPOINT_URL);
        }
        else
        {
            lastConnectionTypeIsClassic = true;
            return SOPC_STATUS_OK;
        }
    }
}

int main(void)
{
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 3000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_Initialize();
    }

    SOPC_PKIProvider* pki = NULL;
    SOPC_SerializedCertificate *crt_cli = NULL, *crt_srv = NULL;
    SOPC_SerializedAsymmetricKey* priv_cli = NULL;

    // Define client application configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetPreferredLocaleIds(sizeof(preferred_locale_ids) - 1, preferred_locale_ids);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetApplicationDescription(APPLICATION_URI, APPLICATION_URI, APPLICATION_NAME,
                                                                   NULL, OpcUa_ApplicationType_Client);
    }

    SOPC_SecureConnection_Config** secureConnConfigArray = NULL;
    size_t previousNbSecConnCfgs = 0;
    size_t nbSecConnCfgs = 0;

    SOPC_ClientConnection* secureConnection1 = NULL;
    SOPC_ClientConnection* secureConnection2 = NULL;
    SOPC_ClientConnection* secureConnection3 = NULL;

    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;

    SOPC_AddressSpace* address_space = SOPC_Embedded_AddressSpace_Load();

    // Get Toolkit Configuration
    SOPC_Toolkit_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    // If security mode is set, load certificates and key

    if (MSG_SECURITY_MODE != OpcUa_MessageSecurityMode_None)
    {
        // Load client certificate
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(CLI_CERT_PATH, &crt_cli);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load client certificate\n");
        }
        else
        {
            printf(">>Stub_Client: Client certificate loaded\n");
        }
        // Load client private key
        if (SOPC_STATUS_OK == status)
        {
            // Private key: Retrieve the password
            char* password = NULL;
            size_t lenPassword = 0;

            bool res = SOPC_TestHelper_AskPass_FromEnv(&password);
            status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

            if (SOPC_STATUS_OK == status)
            {
                lenPassword = strlen(password);
                if (UINT32_MAX < lenPassword)
                {
                    status = SOPC_STATUS_NOK;
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                // Private key: Decrypt and Load
                status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd(
                    CLI_KEY_PATH, &priv_cli, password, (uint32_t) lenPassword);
                if (SOPC_STATUS_OK != status)
                {
                    printf(
                        ">>Stub_Client: Failed to decrypt client private key, please check the password or key format "
                        "(PEM)\n");
                }
            }

            if (NULL != password)
            {
                SOPC_Free(password);
            }

            if (SOPC_STATUS_OK == status)
            {
                printf(">>Stub_Client: Client private key loaded\n");
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            const SOPC_ExposedBuffer* privKeyData = SOPC_SecretBuffer_Expose(priv_cli);
            status = SOPC_HelperConfigClient_SetKeyCertPairFromBytes(
                (size_t) crt_cli->length, crt_cli->data, (size_t) SOPC_SecretBuffer_GetLength(priv_cli), privKeyData);
            SOPC_SecretBuffer_Unexpose(privKeyData, priv_cli);
        }
        SOPC_KeyManager_SerializedCertificate_Delete(crt_cli);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(priv_cli);
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
            status = SOPC_HelperConfigClient_SetPKIprovider(pki);
        }
    }

    // Set asynchronous response callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetServiceAsyncResponse(SOPC_Client_AsyncRespCb);
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

    // Test will run 3 times: 1 with classic endpoint connection, 1 with reverse endpoint connection and 1 with both
    for (unsigned int connectionType = 0; SOPC_STATUS_OK == status && connectionType <= 2; connectionType++)
    {
        // Configure the 3 secure channel connections to use and retrieve channel configuration index
        if (SOPC_STATUS_OK == status)
        {
            SOPC_SecureConnection_Config* secureConnConfig1 = SOPC_HelperConfigClient_CreateSecureConnection(
                "1", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
            SOPC_SecureConnection_Config* secureConnConfig2 = SOPC_HelperConfigClient_CreateSecureConnection(
                "2", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
            SOPC_SecureConnection_Config* secureConnConfig3 = SOPC_HelperConfigClient_CreateSecureConnection(
                "3", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
            if (secureConnConfig1 != NULL && secureConnConfig2 != NULL && secureConnConfig3 != NULL)
            {
                printf(">>Test_Client_Toolkit: Client configured\n");
            }
            else
            {
                status = SOPC_STATUS_NOK;
                printf(">>Test_Client_Toolkit: Client configured\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            previousNbSecConnCfgs = nbSecConnCfgs;
            status = SOPC_HelperConfigClient_GetSecureConnectionConfigs(&nbSecConnCfgs, &secureConnConfigArray);
        }

        // Load server certificate
        if (MSG_SECURITY_MODE != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(SRV_CERT_PATH, &crt_srv);
            if (SOPC_STATUS_OK != status)
            {
                printf(">>Stub_Client: Failed to load server certificate\n");
            }
            else
            {
                printf(">>Stub_Client: Server certificate loaded\n");
            }
            for (size_t i = previousNbSecConnCfgs; SOPC_STATUS_OK == status && i < nbSecConnCfgs; i++)
            {
                status = SOPC_SecureConnectionConfig_AddServerCertificateFromBytes(secureConnConfigArray[i],
                                                                                   crt_srv->length, crt_srv->data);
                // Set username  as authentication mode for second connection
                if (i == 1)
                {
                    // Username: retrieve the password
                    char* password = NULL;
                    size_t lenPassword = 0;

                    bool res = SOPC_TestHelper_AskPass_FromEnv(&password);
                    status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

                    if (SOPC_STATUS_OK == status)
                    {
                        lenPassword = strlen(password);
                        if (UINT32_MAX < lenPassword)
                        {
                            status = SOPC_STATUS_NOK;
                        }
                    }

                    if (SOPC_STATUS_OK == status)
                    {
                    }

                    status = SOPC_SecureConnectionConfig_AddUserName(secureConnConfigArray[i],
                                                                     "username_Basic256Sha256", "user1", password);

                    if (NULL != password)
                    {
                        SOPC_Free(password);
                    }
                }
                // Set X509 as authentication mode for third connection
                else if (i == 2)
                {
                    // TODO: replace by SOPC_SecureConnectionConfig_AddUserX509FromBytes when available
                    /*
                    status = SOPC_SecureConnectionConfig_AddUserX509FromPaths(secureConnConfigArray[i], "X509",
                                                                              USER_CERT_PATH, USER_KEY_PATH, true);
                    */
                    status = SOPC_SecureConnectionConfig_AddAnonymous(secureConnConfigArray[i], "anon");
                }
            }
            SOPC_KeyManager_SerializedCertificate_Delete(crt_srv);
        }

        /* Asynchronous request to get endpoints */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_SecureConnection_Config* currentSecureConfig = SOPC_HelperConfigClient_GetConfigFromId("3");
            if (NULL != currentSecureConfig)
            {
                status = SOPC_SetReverseConnection(currentSecureConfig, connectionType);
            }
            else
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ClientHelper_DiscoveryServiceAsync(currentSecureConfig, getGetEndpoints_message(), 1);
            }
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

        if (SOPC_Atomic_Int_Get(&getEndpointsReceived) == 0)
        {
            printf(">>Test_Client_Toolkit: GetEndpoints Response received: NOK\n");
            status = SOPC_STATUS_NOK;
        }
        else
        {
            printf(">>Test_Client_Toolkit: GetEndpoints Response received: OK\n");
        }

        /* Create the 3 connections */
        if (SOPC_STATUS_OK == status)
        {
            for (size_t i = previousNbSecConnCfgs; SOPC_STATUS_OK == status && i < nbSecConnCfgs; i++)
            {
                status =
                    SOPC_ClientHelper_Connect(secureConnConfigArray[i], SOPC_Client_ConnEventCb, &secureConnection1);
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            printf(">>Test_Client_Toolkit: Activated 3 sessions on 3 SCs: OK\n");
            /* Create a service request message and send it through session (read service)*/
            // msg freed when sent
            // Use 1 as read request context
            status = SOPC_ClientHelper_ServiceAsync(secureConnection1, getReadRequest_message(), 1);
            printf(">>Test_Client_Toolkit: read request sending\n");
        }

        /* Wait until service response is received */
        loopCpt = 0;
        while (SOPC_STATUS_OK == status && !test_results_get_service_result() && loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }

        if (loopCpt * sleepTimeout > loopTimeout)
        {
            status = SOPC_STATUS_TIMEOUT;
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
            status = SOPC_ClientHelper_ServiceAsync(secureConnection1, pWriteReqSent, 1);
            printf(">>Test_Client_Toolkit: write request sending\n");
        }

        /* Wait until service response is received */
        loopCpt = 0;
        while (SOPC_STATUS_OK == status && !test_results_get_service_result() && loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
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
            // Use 2 as read request context
            status = SOPC_ClientHelper_ServiceAsync(secureConnection1, getReadRequest_verif_message(), 2);
            printf(">>Test_Client_Toolkit: read request sending\n");
        }

        /* Wait until service response is received */
        loopCpt = 0;
        while (SOPC_STATUS_OK == status && !test_results_get_service_result() && loopCpt * sleepTimeout <= loopTimeout)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }

        if (loopCpt * sleepTimeout > loopTimeout)
        {
            status = SOPC_STATUS_TIMEOUT;
        }

        /* Now the request can be freed */
        test_results_set_WriteRequest(NULL);
        tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReqCopy);

        /* In case the subscription service shall not be supported, check service response is unsupported service*/
        if (TEST_SUB_SERVICE_UNSUPPORTED)
        {
            if (SOPC_STATUS_OK == status)
            {
                OpcUa_CreateSubscriptionRequest* createSubReq = NULL;
                status =
                    SOPC_Encodeable_Create(&OpcUa_CreateSubscriptionRequest_EncodeableType, (void**) &createSubReq);
                SOPC_ASSERT(SOPC_STATUS_OK == status);

                createSubReq->MaxNotificationsPerPublish = 0;
                createSubReq->Priority = 0;
                createSubReq->PublishingEnabled = true;
                createSubReq->RequestedLifetimeCount = 3;
                createSubReq->RequestedMaxKeepAliveCount = 1;
                createSubReq->RequestedPublishingInterval = 1000;

                // Reset expected result
                test_results_set_service_result(false);

                status = SOPC_ClientHelper_ServiceAsync(secureConnection1, createSubReq, OpcUa_BadServiceUnsupported);

                printf(">>Test_Client_Toolkit: create subscription sending\n");
            }

            /* Wait until service response is received */
            loopCpt = 0;
            while (SOPC_STATUS_OK == status && !test_results_get_service_result() &&
                   loopCpt * sleepTimeout <= loopTimeout)
            {
                loopCpt++;
                SOPC_Sleep(sleepTimeout);
            }

            if (loopCpt * sleepTimeout > loopTimeout)
            {
                status = SOPC_STATUS_TIMEOUT;
            }
        }

        /* Close the connections */
        SOPC_ReturnStatus discoStatus = SOPC_STATUS_NOK;
        if (NULL != secureConnection1)
        {
            discoStatus = SOPC_ClientHelper_Disconnect(&secureConnection1);
            SOPC_ASSERT(SOPC_STATUS_OK == discoStatus);
        }

        if (NULL != secureConnection2)
        {
            discoStatus = SOPC_ClientHelper_Disconnect(&secureConnection2);
            SOPC_ASSERT(SOPC_STATUS_OK == discoStatus);
        }

        if (NULL != secureConnection3)
        {
            discoStatus = SOPC_ClientHelper_Disconnect(&secureConnection3);
            SOPC_ASSERT(SOPC_STATUS_OK == discoStatus);
        }

        cptReadResps = 0;
    }
    /* Close the toolkit */
    SOPC_HelperConfigClient_Clear();
    SOPC_CommonHelper_Clear();

    SOPC_AddressSpace_Delete(address_space);

    if (SOPC_STATUS_OK == status && test_results_get_service_result() != false)
    {
        printf(">>Test_Client_Toolkit final result: OK\n");
        return 0;
    }
    else
    {
        printf(">>Test_Client_Toolkit final result: NOK (BAD status: %" PRIu32 ")\n", status);
        return 1;
    }
}
