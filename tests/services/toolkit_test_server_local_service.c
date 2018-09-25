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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_pki_stack.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"

#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "embedded/loader.h"
#include "test_results.h"
#include "testlib_read_response.h"
#include "testlib_write.h"
#include "util_variant.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"

static int endpointClosed = false;
static int getEndpointsReceived = false;

static uint32_t cptReadResps = 0;

static void Test_ComEvent_FctServer(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    (void) idOrStatus;
    (void) param;
    (void) appContext;
    SOPC_EncodeableType* encType = NULL;

    if (event == SE_CLOSED_ENDPOINT)
    {
        printf("<Test_Server_Local_Service: closed endpoint event: OK\n");
        SOPC_Atomic_Int_Add(&endpointClosed, 1);
    }
    else if (event == SE_LOCAL_SERVICE_RESPONSE)
    {
        encType = *(SOPC_EncodeableType**) param;
        if (encType == &OpcUa_ReadResponse_EncodeableType)
        {
            printf("<Test_Server_Local_Service: received local service ReadResponse \n");
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
            printf("<Test_Server_Local_Service: received local service  WriteResponse \n");
            OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) param;
            test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
        }
        else if (encType == &OpcUa_GetEndpointsResponse_EncodeableType)
        {
            // Check context value is same as one provided with request
            assert(1 == appContext);
            printf("<Test_Server_Local_Service: received GetEndpointsResponse \n");
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
            SOPC_Atomic_Int_Add(&getEndpointsReceived, validEndpoints ? 1 : 0);
        }
    }
    else
    {
        printf("<Test_Server_Local_Service: unexpected endpoint event %d : NOK\n", event);
    }
}

static void Test_AddressSpaceNotif_Fct(SOPC_App_AddSpace_Event event, void* opParam, SOPC_StatusCode opStatus)
{
    // No notification shall be received when using local services
    (void) event;
    (void) opParam;
    (void) opStatus;
    assert(false);
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

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;

    uint32_t epConfigIdx = 0;
    SOPC_Endpoint_Config epConfig;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 50;
    // Loop timeout in milliseconds
    uint32_t loopTimeout = 5000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    // Secu policy configuration:
    static SOPC_SerializedCertificate* serverCertificate = NULL;
    static SOPC_SerializedAsymmetricKey* serverKey = NULL;
    static SOPC_SerializedCertificate* authCertificate = NULL;
    static SOPC_PKIProvider* pkiProvider = NULL;

    SOPC_AddressSpace* address_space = SOPC_Embedded_AddressSpace_Load();

    SOPC_SecurityPolicy secuConfig[1];
    SOPC_String_Initialize(&secuConfig[0].securityPolicy);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&secuConfig[0].securityPolicy, SOPC_SecurityPolicy_Basic256Sha256_URI);
        secuConfig[0].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
    }

    if (SOPC_STATUS_OK == status)
    {
        // Init unique endpoint structure
        epConfig.endpointURL = ENDPOINT_URL;

        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile("./server_public/server_2k_cert.der",
                                                                      &serverCertificate);
        epConfig.serverCertificate = serverCertificate;
    }

    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile("./server_private/server_2k_key.pem", &serverKey);
        epConfig.serverKey = serverKey;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile("./trusted/cacert.der", &authCertificate);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_Create(authCertificate, NULL, &pkiProvider);
        epConfig.pki = pkiProvider;
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Local_Service: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: Certificates and key loaded\n");
    }

    epConfig.secuConfigurations = secuConfig;
    epConfig.nbSecuConfigs = 1;

    // Application description configuration
    OpcUa_ApplicationDescription_Initialize(&epConfig.serverDescription);
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ApplicationUri, APPLICATION_URI);
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ProductUri, PRODUCT_URI);
    epConfig.serverDescription.ApplicationType = OpcUa_ApplicationType_Server;
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ApplicationName.Text, "INGOPCS toolkit server example");

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;
    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_UserAuthentication_CreateManager_AllowAll();
        authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
        if (NULL == authenticationManager || NULL == authorizationManager)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            printf("<Test_Server_Toolkit: Failed to create the user manager\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        epConfig.authenticationManager = authenticationManager;
        epConfig.authorizationManager = authorizationManager;
    }

    // Init stack configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Test_ComEvent_FctServer);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed initializing\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: initialized\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG);
    }

    // Define server address space
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(address_space);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed to configure the @ space\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: @ space configured\n");
        }
    }

    // Define address space modification notification callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceNotifCb(&Test_AddressSpaceNotif_Fct);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the @ space modification notification callback \n");
        }
        else
        {
            printf("<Test_Server_Toolkit: @ space modification notification callback configured\n");
        }
    }

    // Add endpoint description configuration
    if (SOPC_STATUS_OK == status)
    {
        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(&epConfig);
        if (epConfigIdx != 0)
        {
            status = SOPC_Toolkit_Configured();
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed to configure the endpoint\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: Endpoint configured\n");
        }
    }

    // Asynchronous request to open the endpoint
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Service: Opening endpoint... \n");
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
    }

    /* Asynchronous request to get endpoints */
    if (SOPC_STATUS_OK == status)
    {
        // Use 1 as getEndpoints request context
        SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, getGetEndpoints_message(), 1);
        printf("<Test_Server_Local_Service: Get endpoints local request: OK\n");
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
        printf("<Test_Server_Local_Service: GetEndpoints Response received: NOK\n");
        status = SOPC_STATUS_NOK;
    }
    else
    {
        printf("<Test_Client_Toolkit: GetEndpoints Response received: OK\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, getReadRequest_message(), 1);
        printf("<Test_Server_Local_Service: local read request service\n");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);

        // Create WriteRequest to be sent (deallocated by toolkit) */
        pWriteReqSent = tlibw_new_WriteRequest();

        // Create same WriteRequest to check results on response reception */
        pWriteReqCopy = tlibw_new_WriteRequest();
        test_results_set_WriteRequest(pWriteReqCopy);

        // Use 1 as write request context
        SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, pWriteReqSent, 1);

        printf("<Test_Server_Local_Service: local write request sending\n");
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

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends another ReadRequest, to verify that the AddS has changed */
        /* The callback will call the verification */
        // msg freed when sent
        // Use 2 as read request context
        SOPC_ToolkitServer_AsyncLocalServiceRequest(epConfigIdx, getReadRequest_verif_message(), 2);

        printf("<Test_Server_Local_Service: local read request sending\n");
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

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReqCopy);

    // Asynchronous request to close the endpoint
    SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx);

    // Wait until endpoint is closed
    loopCpt = 0;
    loopTimeout = 1000;
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&endpointClosed) == 0 &&
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

    // Clear the toolkit configuration and stop toolkit threads
    SOPC_Toolkit_Clear();

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Service: final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: final result NOK with status = '%d'\n", status);
    }

    // Deallocate locally allocated data

    SOPC_String_Clear(&secuConfig[0].securityPolicy);

    SOPC_KeyManager_SerializedCertificate_Delete(serverCertificate);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(serverKey);
    SOPC_KeyManager_SerializedCertificate_Delete(authCertificate);
    SOPC_PKIProvider_Free(&pkiProvider);
    SOPC_AddressSpace_Delete(address_space);

    SOPC_UserAuthentication_FreeManager(&authenticationManager);
    SOPC_UserAuthorization_FreeManager(&authorizationManager);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
