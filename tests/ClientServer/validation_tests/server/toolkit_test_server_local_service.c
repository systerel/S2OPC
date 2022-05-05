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
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#include "test_results.h"
#include "testlib_read_response.h"
#include "testlib_write.h"
#include "util_variant.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

static int32_t endpointClosed = false;

static uint32_t cptReadResps = 0;

static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&endpointClosed, true);
}

static void SOPC_LocalServiceAsyncRespCallback(SOPC_EncodeableType* encType, void* response, uintptr_t appContext)
{
    if (encType == &OpcUa_ReadResponse_EncodeableType)
    {
        printf("<Test_Server_Local_Service: received local service ReadResponse \n");
        OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) response;
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
            test_results_set_service_result(tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
        }
    }
    else if (encType == &OpcUa_WriteResponse_EncodeableType)
    {
        // Check context value is same as one provided with request
        assert(1 == appContext);
        printf("<Test_Server_Local_Service: received local service  WriteResponse \n");
        OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) response;
        test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
    }
}

static bool checkGetEndpointsResponse(OpcUa_GetEndpointsResponse* getEndpointsResp)
{
    printf("<Test_Server_Local_Service: received GetEndpointsResponse \n");
    SOPC_String endpointUrl;
    SOPC_String_Initialize(&endpointUrl);
    SOPC_ReturnStatus testStatus = SOPC_String_AttachFromCstring(&endpointUrl, DEFAULT_ENDPOINT_URL);
    bool validEndpoints = true;

    if (testStatus != SOPC_STATUS_OK || getEndpointsResp->NoOfEndpoints <= 0)
    {
        // At least one endpoint shall be described with the correct endpoint URL
        validEndpoints = false;
    }
    for (int32_t idx = 0; idx < getEndpointsResp->NoOfEndpoints && validEndpoints != false; idx++)
    {
        validEndpoints = SOPC_String_Equal(&getEndpointsResp->Endpoints[idx].EndpointUrl, &endpointUrl);
    }

    OpcUa_GetEndpointsResponse_Clear(getEndpointsResp);
    SOPC_Free(getEndpointsResp);

    return validEndpoints;
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
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;

    // Configure log
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_local_service_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Local_Service: Failed initializing\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: initialized\n");
    }

    const uint32_t sleepTimeout = 50;
    // Loop timeout in milliseconds
    uint32_t loopTimeout = 5000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    // Secu policy configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Endpoint_Config* ep = SOPC_HelperConfigServer_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
        SOPC_SecurityPolicy* sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);

        if (NULL == ep || NULL == sp)
        {
            SOPC_Free(ep);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_SignAndEncrypt);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
        }
    }

    // Server certificates configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetKeyCertPairFromPath("./server_public/server_2k_cert.der",
                                                                "./server_private/server_2k_key.pem");
    }

    // Set PKI configuration
    if (SOPC_STATUS_OK == status)
    {
        char* lPathsTrustedRoots[] = {"./trusted/cacert.der", NULL};
        char* lPathsTrustedLinks[] = {NULL};
        char* lPathsUntrustedRoots[] = {NULL};
        char* lPathsUntrustedLinks[] = {NULL};
        char* lPathsIssuedCerts[] = {NULL};
        char* lPathsCRL[] = {"./revoked/cacrl.der", NULL};
        SOPC_PKIProvider* pkiProvider = NULL;
        status =
            SOPC_PKIProviderStack_CreateFromPaths(lPathsTrustedRoots, lPathsTrustedLinks, lPathsUntrustedRoots,
                                                  lPathsUntrustedLinks, lPathsIssuedCerts, lPathsCRL, &pkiProvider);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_HelperConfigServer_SetPKIprovider(pkiProvider);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Local_Service: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: Certificates and key loaded\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set namespaces
        char* namespaces[] = {DEFAULT_APPLICATION_URI};
        status = SOPC_HelperConfigServer_SetNamespaces(1, namespaces);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed setting namespaces\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", NULL,
                                                                   OpcUa_ApplicationType_Server);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed setting application description \n");
        }
    }

    // Address space configuration
    SOPC_AddressSpace* address_space = NULL;
    if (SOPC_STATUS_OK == status)
    {
        address_space = SOPC_Embedded_AddressSpace_Load();
        status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetAddressSpace(address_space);
    }

    // Configure the local service asynchronous response callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(SOPC_LocalServiceAsyncRespCallback);
    }

    // Asynchronous request to start server
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
    }

    /*
     * LOCAL SERVICE: GetEndpoints
     */

    /* Synchronous request to get endpoints */
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Service: Server started\n");

        // Use 1 as getEndpoints request context
        OpcUa_GetEndpointsResponse* resp = NULL;
        status =
            SOPC_ServerHelper_LocalServiceSync(SOPC_GetEndpointsRequest_Create(DEFAULT_ENDPOINT_URL), (void**) &resp);

        if (SOPC_STATUS_OK == status)
        {
            bool res = checkGetEndpointsResponse(resp);
            status = (res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
        }

        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: Get endpoints local service synchronous call: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: Get endpoints local  service synchronous call: NOK\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_RegisterServer2Response* resp = NULL;
        status = SOPC_ServerHelper_LocalServiceSync(SOPC_RegisterServer2Request_CreateFromServerConfiguration(),
                                                    (void**) &resp);

        if (SOPC_STATUS_OK == status && 0 == (resp->ResponseHeader.ServiceResult & SOPC_GoodStatusOppositeMask))
        {
            status = resp->NoOfConfigurationResults > 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            for (int32_t i = 0; SOPC_STATUS_OK == status && i < resp->NoOfConfigurationResults; i++)
            {
                if (0 != (resp->ConfigurationResults[i] & SOPC_GoodStatusOppositeMask))
                {
                    // Status is not good, configuration failed
                    status = SOPC_STATUS_NOK;
                }
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }

        OpcUa_RegisterServer2Response_Clear(resp);
        SOPC_Free(resp);

        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: RegisterServer2 local service synchronous call: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: RegisterServer2 local  service synchronous call: NOK\n");
        }
    }

    /*
     * LOCAL SERVICE: Read initial node attributes values
     */

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        status = SOPC_ServerHelper_LocalServiceAsync(getReadRequest_message(), 1);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: NOK\n");
        }
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    /*
     * LOCAL SERVICE: Write node values
     */

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);

        // Create WriteRequest to be sent (deallocated by toolkit) */
        pWriteReqSent = tlibw_new_WriteRequest(address_space);

        // Create same WriteRequest to check results on response reception */
        pWriteReqCopy = tlibw_new_WriteRequest(address_space);
        test_results_set_WriteRequest(pWriteReqCopy);

        // Use 1 as write request context
        status = SOPC_ServerHelper_LocalServiceAsync(pWriteReqSent, 1);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local write asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local write asynchronous request: NOK\n");
        }
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

    /*
     * LOCAL SERVICE: Read node values that were written and check values are modified as requested
     */

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends another ReadRequest, to verify that the AddS has changed */
        /* The callback will call the verification */
        // msg freed when sent
        // Use 2 as read request context
        status = SOPC_ServerHelper_LocalServiceAsync(getReadRequest_verif_message(), 2);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: NOK\n");
        }
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

    // Asynchronous request to stop the server
    SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();

    // Wait until endpoint is closed
    loopCpt = 0;
    loopTimeout = 1000;
    while (SOPC_STATUS_OK == stopStatus && SOPC_Atomic_Int_Get(&endpointClosed) == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        stopStatus = SOPC_STATUS_TIMEOUT;
    }

    // Clear the toolkit configuration and stop toolkit threads
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus)
    {
        printf("<Test_Server_Local_Service: final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: final result NOK with status = '%d' and stopStatus = '%d'\n", status,
               stopStatus);
    }

    return (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus) ? 0 : 1;
}
