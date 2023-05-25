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

static const char* preferred_locale_ids[] = {"en-US", "fr-FR", NULL};

#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256

// Client certificate path
#define CLI_CERT_PATH "./client_public/client_2k_cert.der"
// Server certificate path
#define SRV_CERT_PATH "./server_public/server_2k_cert.der"
// Client private key path
#define CLI_KEY_PATH "./client_private/encrypted_client_2k_key.pem"

#ifdef WITH_STATIC_SECURITY_DATA
#include "client_static_security_data.h"
#include "server_static_security_data.h"
#else
// PKI trusted CA
static char* default_trusted_root_issuers[] = {"trusted/cacert.der", /* Demo CA */
                                               NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der", NULL};
static char* default_empty_cert_paths[] = {NULL};
#endif // WITH_STATIC_SECURITY_DATA

// User certificate path
#define USER_CERT_PATH "./user_public/user_2k_cert.der"
// User key path
#define USER_KEY_PATH "./user_private/encrypted_user_2k_key.pem"

static int32_t getEndpointsReceived = 0;

static uint32_t cptReadResps = 0;

#if S2OPC_NANO_PROFILE
#define TEST_SUB_SERVICE_UNSUPPORTED true
#else
#define TEST_SUB_SERVICE_UNSUPPORTED false
#endif

// Asynchronous service response callback
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

// Connection event callback (only for unexpected events)
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

static SOPC_ReturnStatus Client_Initialize(void)
{
    // Print Toolkit Configuration
    SOPC_Toolkit_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize client wrapper */

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

    return status;
}

/*
 * Configure the applications authentication parameters of the endpoint:
 * - Client certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Client_SetDefaultAppsAuthConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (MSG_SECURITY_MODE != OpcUa_MessageSecurityMode_None)
    {
        SOPC_PKIProvider* pkiProvider = NULL;

#ifdef WITH_STATIC_SECURITY_DATA
        SOPC_SerializedCertificate* serializedCAcert = NULL;
        SOPC_CRLList* serializedCAcrl = NULL;

        /* Load client certificates and key from C source files (no filesystem needed) */
        status = SOPC_HelperConfigClient_SetKeyCertPairFromBytes(sizeof(client_2k_cert), client_2k_cert,
                                                                 sizeof(client_2k_key), client_2k_key);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &serializedCAcert);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &serializedCAcrl);
        }

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(serializedCAcert, serializedCAcrl, &pkiProvider);
        }
        SOPC_KeyManager_SerializedCertificate_Delete(serializedCAcert);
#else // WITH_STATIC_SECURITY_DATA == false

        /* Load client certificate and key from files */
        status = SOPC_HelperConfigClient_SetKeyCertPairFromPath(CLI_CERT_PATH, CLI_KEY_PATH, true);

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_CreateFromPaths(
                default_trusted_root_issuers, default_empty_cert_paths, default_empty_cert_paths,
                default_empty_cert_paths, default_empty_cert_paths, default_revoked_certs, &pkiProvider);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to create PKI\n");
        }
        else
        {
            printf(">>Stub_Client: PKI created\n");
        }

#endif

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_HelperConfigClient_SetPKIprovider(pkiProvider);
        }

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Client_Toolkit: Failed loading certificates and key (check paths are valid)\n");
        }
        else
        {
            printf("<Test_Client_Toolkit: Certificates and key loaded\n");
        }
    }

    return status;
}

/*
 * Default client configuration loader (without XML configuration)
 */
static SOPC_ReturnStatus Client_SetDefaultConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    // Define client application configuration
    SOPC_ReturnStatus status = SOPC_HelperConfigClient_SetPreferredLocaleIds(
        (sizeof(preferred_locale_ids) / sizeof(preferred_locale_ids[0]) - 1), preferred_locale_ids);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetApplicationDescription(APPLICATION_URI, APPLICATION_URI, APPLICATION_NAME,
                                                                   NULL, OpcUa_ApplicationType_Client);
    }

    /**
     * Define client certificate and PKI provider
     */
    if (SOPC_STATUS_OK == status)
    {
        status = Client_SetDefaultAppsAuthConfig();
    }

    // Configure the 3 secure channel connections to use and retrieve channel configuration index
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SecureConnection_Config* secureConnConfig1 = SOPC_HelperConfigClient_CreateSecureConnection(
            "1", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
        SOPC_SecureConnection_Config* secureConnConfig2 = SOPC_HelperConfigClient_CreateSecureConnection(
            "2", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
        SOPC_SecureConnection_Config* secureConnConfig3 = SOPC_HelperConfigClient_CreateSecureConnection(
            "3", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
        status = SOPC_SecureConnectionConfig_SetReverseConnection(secureConnConfig3, REVERSE_ENDPOINT_URL);

        if (secureConnConfig1 == NULL || secureConnConfig2 == NULL || secureConnConfig3 == NULL ||
            SOPC_STATUS_OK != status)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_GetSecureConnectionConfigs(nbSecConnCfgs, secureConnConfigArray);
    }

    // Load server certificate
    if (MSG_SECURITY_MODE != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        for (size_t i = 0; SOPC_STATUS_OK == status && i < *nbSecConnCfgs; i++)
        {
#ifdef WITH_STATIC_SECURITY_DATA
            status = SOPC_SecureConnectionConfig_AddServerCertificateFromBytes((*secureConnConfigArray)[i],
                                                                               sizeof(server_2k_cert), server_2k_cert);
#else
            status =
                SOPC_SecureConnectionConfig_AddServerCertificateFromPath((*secureConnConfigArray)[i], SRV_CERT_PATH);
#endif

            // Set username  as authentication mode for second connection
            if (i == 1)
            {
                status = SOPC_SecureConnectionConfig_AddUserName((*secureConnConfigArray)[i], "username_Basic256Sha256",
                                                                 NULL, NULL);
            }
            // Set X509 as authentication mode for third connection
            else if (i == 2)
            {
#ifdef WITH_STATIC_SECURITY_DATA
                status = SOPC_SecureConnectionConfig_AddUserX509FromBytes((*secureConnConfigArray)[i], "X509",
                                                                          sizeof(user_2k_cert), user_2k_cert,
                                                                          sizeof(user_2k_key), user_2k_key);
#else
                status = SOPC_SecureConnectionConfig_AddUserX509FromPaths((*secureConnConfigArray)[i], "X509",
                                                                          USER_CERT_PATH, USER_KEY_PATH, true);
#endif
            }
        }
    }

    return status;
}

static bool SOPC_GetClientUser1Password(char** outUserName, char** outPassword)
{
    const char* user1 = "user1";
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

static SOPC_ReturnStatus Client_LoadClientConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    /* Retrieve XML configuration file path from environment variables TEST_CLIENT_XML_CONFIG,
     *
     * In case of success returns the file path otherwise load default configuration.
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    const char* xml_client_config_path = getenv("TEST_CLIENT_XML_CONFIG");

    if (NULL != xml_client_config_path)
    {
#ifdef WITH_EXPAT
        status = SOPC_HelperConfigClient_ConfigureFromXML(xml_client_config_path, NULL, nbSecConnCfgs,
                                                          secureConnConfigArray);
#else
        printf(
            "Error: an XML client configuration file path provided whereas XML library not available (Expat).\n"
            "Do not define environment variables TEST_CLIENT_XML_CONFIG.\n"
            "Or compile with XML library available.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
#endif
    }

#ifndef WITH_STATIC_SECURITY_DATA
    // Set callback necessary to retrieve client key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
#endif
    // TODO: move to !WITH_STATIC_SECURITY_DATA when X509 certs are configured statically
    // Set callback necessary to retrieve user key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetUserKeyPasswordCallback(&SOPC_TestHelper_AskPassWithContext_FromEnv);
    }
    // Set callback necessary to retrieve user password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetUserNamePasswordCallback(&SOPC_GetClientUser1Password);
    }

    if (SOPC_STATUS_OK == status && NULL == xml_client_config_path)
    {
        status = Client_SetDefaultConfiguration(nbSecConnCfgs, secureConnConfigArray);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit: Client configured\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit: Client configuration failed\n");
    }

    return status;
}

int main(void)
{
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 200;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 10000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_SecureConnection_Config** secureConnConfigArray = NULL;
    size_t nbSecConnCfgs = 0;

    SOPC_ClientConnection* secureConnections[3] = {NULL};

    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;

    SOPC_ReturnStatus status = Client_Initialize();

    if (SOPC_STATUS_OK == status)
    {
        status = Client_LoadClientConfiguration(&nbSecConnCfgs, &secureConnConfigArray);
    }

    // Set asynchronous response callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigClient_SetServiceAsyncResponse(SOPC_Client_AsyncRespCb);
    }

    // Set an address space for test purpose only to check test result valid (not expected in a client)
    SOPC_AddressSpace* address_space = SOPC_Embedded_AddressSpace_Load();
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

    /* Asynchronous request to get endpoints using reverse connection */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SecureConnection_Config* reverseSecureConnConfig = SOPC_HelperConfigClient_GetConfigFromId("3");
        if (NULL == reverseSecureConnConfig)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ClientHelper_DiscoveryServiceAsync(reverseSecureConnConfig, getGetEndpoints_message(), 1);
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
        for (size_t i = 0; SOPC_STATUS_OK == status && i < nbSecConnCfgs; i++)
        {
            status =
                SOPC_ClientHelper_Connect(secureConnConfigArray[i], SOPC_Client_ConnEventCb, &secureConnections[i]);
        }
    }

    /* Read values on 1st connection */
    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit: Activated 3 sessions on 3 SCs: OK\n");
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        status = SOPC_ClientHelper_ServiceAsync(secureConnections[0], getReadRequest_message(), 1);
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

    /* Write values on 2nd connection */
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
        status = SOPC_ClientHelper_ServiceAsync(secureConnections[1], pWriteReqSent, 1);
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

    /* Re-read values to check previous write effect */
    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends another ReadRequest, to verify that the AddS has changed */
        /* The callback will call the verification */
        // msg freed when sent
        // Use 2 as read request context
        status = SOPC_ClientHelper_ServiceAsync(secureConnections[0], getReadRequest_verif_message(), 2);
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
            status = SOPC_Encodeable_Create(&OpcUa_CreateSubscriptionRequest_EncodeableType, (void**) &createSubReq);
            SOPC_ASSERT(SOPC_STATUS_OK == status);

            createSubReq->MaxNotificationsPerPublish = 0;
            createSubReq->Priority = 0;
            createSubReq->PublishingEnabled = true;
            createSubReq->RequestedLifetimeCount = 3;
            createSubReq->RequestedMaxKeepAliveCount = 1;
            createSubReq->RequestedPublishingInterval = 1000;

            // Reset expected result
            test_results_set_service_result(false);

            status = SOPC_ClientHelper_ServiceAsync(secureConnections[0], createSubReq, OpcUa_BadServiceUnsupported);

            printf(">>Test_Client_Toolkit: create subscription sending\n");
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
    }

    /* Close the connections */
    for (size_t i = 0; i < sizeof(secureConnections) / sizeof(secureConnections[0]); i++)
    {
        if (NULL != secureConnections[i])
        {
            SOPC_ReturnStatus discoStatus = SOPC_ClientHelper_Disconnect(&secureConnections[i]);
            SOPC_ASSERT(SOPC_STATUS_OK == discoStatus);
        }
    }

    cptReadResps = 0;

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
