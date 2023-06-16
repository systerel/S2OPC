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

#include <check.h>
#include <stdlib.h>
#include <string.h>

// Server wrapper
#include "libs2opc_client_cmds.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_encodeable.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#ifdef WITH_EXPAT
#include "xml_expat/sopc_uanodeset_loader.h"
#endif

#include "opcua_identifiers.h"

#include "embedded/sopc_addspace_loader.h"

#include "test_results.h"
#include "testlib_read_response.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"

#define TEST_SERVER_XML_ADDRESS_SPACE "TEST_SERVER_XML_ADDRESS_SPACE"

// Define number of read values in read request to force multi chunk use in request and response:
// use max buffer size for 1 chunk and encoded size of a ReadValueId / DataValue which is 18 bytes in this test
#define NB_READ_VALUES ((SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE / 18) + 1)

static char* default_trusted_root_issuers[] = {"trusted/cacert.der", /* Demo CA */
                                               NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der", NULL};

static int32_t connectionClosed = false;
static int32_t endpointClosed = false;

static const char* node_id_str = "ns=1;i=1012";
static const uint64_t write_value = 12;

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 500;

#define SHUTDOWN_PHASE_IN_SECONDS 5

static SOPC_AddressSpace* addressSpace = NULL;

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/*
 * Server stop callback
 */
static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&endpointClosed, true);
}

static void disconnect_callback(const uint32_t c_id)
{
    SOPC_UNUSED_ARG(c_id);
    SOPC_Atomic_Int_Set(&connectionClosed, true);
}

/*---------------------------------------------------------------------------
 *                          Client initialization
 *---------------------------------------------------------------------------*/

static OpcUa_GetEndpointsResponse* expectedEndpoints = NULL;

static int32_t client_create_configuration(void)
{
    int32_t res = SOPC_ClientHelper_Initialize(disconnect_callback);
    if (res < 0)
    {
        return res;
    }

    SOPC_ReturnStatus status = SOPC_ClientHelper_SetLocaleIds(2, (const char*[]){"fr-FR", "en-US"});

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                             DEFAULT_APPLICATION_NAME, "fr-FR",
                                                             OpcUa_ApplicationType_Client);
    }

    // Retrieve endpoints from server
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_GetEndpointsRequest* request = SOPC_GetEndpointsRequest_Create(DEFAULT_ENDPOINT_URL);
        if (NULL != request)
        {
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &expectedEndpoints);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        return -100;
    }

    SOPC_ClientHelper_Security security = {
        .security_policy = SOPC_SecurityPolicy_Basic256Sha256_URI,
        .security_mode = OpcUa_MessageSecurityMode_SignAndEncrypt,
        .path_cert_auth = "./trusted/cacert.der",
        .path_crl = "./revoked/cacrl.der",
        .path_cert_srv = "./server_public/server_2k_cert.der",
        .path_cert_cli = "./client_public/client_2k_cert.der",
        .path_key_cli = "./client_private/encrypted_client_2k_key.pem",
        .policyId = SOPC_UserTokenPolicy_X509Basic256Sha256_ID,
        .username = NULL,
        .password = NULL,
        .path_cert_x509_token = "./user_public/user_2k_cert.der",
        .path_key_x509_token = "./user_private/encrypted_user_2k_key.pem",
        .key_x509_token_encrypted = true,
    };

    SOPC_ClientHelper_EndpointConnection endpoint = {
        .endpointUrl = DEFAULT_ENDPOINT_URL,
        .serverUri = NULL,
        .reverseConnectionConfigId = 0,
    };

    status = SOPC_HelperConfigClient_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed to configure the client key password callback\n");
    }

    status = SOPC_HelperConfigClient_SetUserKeyPasswordCallback(&SOPC_TestHelper_AskPassWithContext_FromEnv);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed to configure the user key password callback\n");
    }

    /* connect to the endpoint */
    return SOPC_ClientHelper_CreateConfiguration(&endpoint, &security, expectedEndpoints);
}

/*---------------------------------------------------------------------------
 *                          client tests
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus client_send_write_test(int32_t connectionId)
{
    /* create the write request */

    SOPC_ClientHelper_WriteValue writeValue;
    writeValue.nodeId = node_id_str;

    writeValue.indexRange = NULL;
    writeValue.value = SOPC_Calloc(1, sizeof(*writeValue.value));

    if (writeValue.value == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_DataValue_Initialize(writeValue.value);

    writeValue.value->Value.BuiltInTypeId = SOPC_UInt64_Id;
    writeValue.value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    writeValue.value->Value.Value.Uint64 = write_value;

    SOPC_StatusCode writeResult;
    int32_t writeRes = SOPC_ClientHelper_Write(connectionId, &writeValue, 1, &writeResult);

    SOPC_ReturnStatus status = 0 == writeRes ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status && (writeResult & SOPC_GoodStatusOppositeMask) != 0)
    {
        status = SOPC_STATUS_NOK;
    }

    SOPC_Free(writeValue.value);

    return status;
}

static SOPC_ReturnStatus client_send_read_req_test(int32_t connectionId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SOPC_ClientHelper_ReadValue* readValues = SOPC_Calloc(NB_READ_VALUES, sizeof(*readValues));
    SOPC_DataValue* resultValues = SOPC_Calloc(NB_READ_VALUES, sizeof(*resultValues));

    if (NULL != readValues && NULL != resultValues)
    {
        status = SOPC_STATUS_OK;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (size_t i = 0; SOPC_STATUS_OK == status && i < NB_READ_VALUES; i++)
    {
        SOPC_DataValue_Initialize(&resultValues[i]);
        SOPC_ClientHelper_ReadValue* readValue = &readValues[i];
        readValue->attributeId = 13;
        readValue->nodeId = node_id_str;
        readValue->indexRange = NULL;
    }

    /* Send the request and retrieve result */
    if (SOPC_STATUS_OK == status)
    {
        int32_t result = SOPC_ClientHelper_Read(connectionId, readValues, NB_READ_VALUES, resultValues);
        status = 0 == result ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    /* Verify read results */
    for (size_t i = 0; SOPC_STATUS_OK == status && i < NB_READ_VALUES; i++)
    {
        SOPC_Variant* resultValue = &resultValues[i].Value;

        if (resultValue->BuiltInTypeId != SOPC_UInt64_Id ||
            resultValue->ArrayType != SOPC_VariantArrayType_SingleValue || resultValue->Value.Uint64 != write_value)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (NULL != resultValues)
    {
        for (size_t i = 0; i < NB_READ_VALUES; i++)
        {
            SOPC_DataValue_Clear(&resultValues[i]);
        }
    }
    SOPC_Free(readValues);
    SOPC_Free(resultValues);

    return status;
}

#ifdef WITH_EXPAT
#if 0 != S2OPC_NODE_MANAGEMENT
static SOPC_ReturnStatus client_send_add_nodes_req_test(int32_t connectionId)
{
    // Note: address space need to be defined dynamically using

    const char* xml_address_space_config_path = getenv(TEST_SERVER_XML_ADDRESS_SPACE);
    if(NULL == xml_address_space_config_path){
        printf("ERROR: "TEST_SERVER_XML_ADDRESS_SPACE" env variable shall be set\n");
    }
    ck_assert_ptr_nonnull(xml_address_space_config_path);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;
    SOPC_ExpandedNodeId parentNodeId;
    SOPC_ExpandedNodeId_Initialize(&parentNodeId);
    SOPC_NodeId referenceTypeId;
    SOPC_NodeId_Initialize(&referenceTypeId);
    SOPC_ExpandedNodeId reqNodeId;
    SOPC_ExpandedNodeId_Initialize(&reqNodeId);
    SOPC_QualifiedName browseName;
    SOPC_QualifiedName_Initialize(&browseName);
    SOPC_ExpandedNodeId typeDefinition;
    SOPC_ExpandedNodeId_Initialize(&typeDefinition);

    // Parent node is "Objects" node
    parentNodeId.NodeId.Data.Numeric = OpcUaId_ObjectsFolder;
    // Reference type "Organizes" node
    referenceTypeId.Data.Numeric = OpcUaId_Organizes;
    // Type definition is BaseDataVariable (i=63)
    typeDefinition.NodeId.Data.Numeric = OpcUaId_BaseDataVariableType;

    // NodeId request
    reqNodeId.NodeId.Namespace = 1;
    reqNodeId.NodeId.IdentifierType = SOPC_IdentifierType_String;
    status = SOPC_String_AttachFromCstring(&reqNodeId.NodeId.Data.String, "NewNodeId42");
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    // BrowseName
    browseName.NamespaceIndex = 1;
    status = SOPC_String_AttachFromCstring(&browseName.Name, "NewAddedNode42");
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    addNodesReq = SOPC_AddNodesRequest_Create(1);

    if (NULL == addNodesReq)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_AddNodeRequest_SetVariableAttributes(addNodesReq, 0, &parentNodeId, &referenceTypeId, &reqNodeId,
                                                       &browseName, &typeDefinition, NULL, NULL, NULL, NULL, NULL, NULL,
                                                       NULL, 0, NULL, NULL, NULL, NULL, NULL);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_GenericService(connectionId, (void*) addNodesReq, (void**) &addNodesResp);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != addNodesResp);
        if (!SOPC_IsGoodStatus(addNodesResp->ResponseHeader.ServiceResult) || addNodesResp->NoOfResults != 1 || addNodesResp->Results[0].StatusCode ||
            !SOPC_NodeId_Equal(&reqNodeId.NodeId, &addNodesResp->Results[0].AddedNodeId))
        {
            status = SOPC_STATUS_NOK;
        }

        SOPC_ReturnStatus delStatus = SOPC_Encodeable_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }
    else
    {
        SOPC_ReturnStatus delStatus = SOPC_Encodeable_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    return status;
}
#endif
#endif

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------
 * Application description and endpoint configuration:
 *---------------------------------------------------*/
#ifdef WITH_EXPAT
static SOPC_AddressSpace* SOPC_LoadAddressSpaceConfigFromFile(const char* filename)
{
    FILE* fd = fopen(filename, "r");
    if (NULL == fd)
    {
        return NULL;
    }
    SOPC_AddressSpace* space = SOPC_UANodeSet_Parse(fd);
    fclose(fd);

    SOPC_ReturnStatus status = SOPC_ToolkitServer_SetAddressSpaceConfig(space);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_AddressSpace_Delete(space);
        space = NULL;
    }
    // Keep pointer for deallocation
    addressSpace = space;

    return space;
}
#endif

static SOPC_ReturnStatus Server_SetServerConfiguration(void)
{
    /* Load server endpoints configuration
     * use an embedded default demo server configuration.
     */

    SOPC_Endpoint_Config* ep = SOPC_HelperConfigServer_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
    SOPC_SecurityPolicy* sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);

    if (NULL == ep || NULL == sp)
    {
        SOPC_Free(ep);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_SignAndEncrypt);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_X509_Basic256Sha256SecurityPolicy);
    }

    // Server certificates configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
        pConfig->serverConfig.serverKeyEncrypted = true;
        status = SOPC_HelperConfigServer_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed to configure the server key user password callback\n");
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetKeyCertPairFromPath("./server_public/server_2k_cert.der",
                                                                "./server_private/encrypted_server_2k_key.pem", true);
    }

    // Set PKI configuration
    if (SOPC_STATUS_OK == status)
    {
        char* lPathsTrustedLinks[] = {NULL};
        char* lPathsUntrustedRoots[] = {NULL};
        char* lPathsUntrustedLinks[] = {NULL};
        char* lPathsIssuedCerts[] = {NULL};
        SOPC_PKIProvider* pkiProvider = NULL;
        status = SOPC_PKIProviderStack_CreateFromPaths(default_trusted_root_issuers, lPathsTrustedLinks,
                                                       lPathsUntrustedRoots, lPathsUntrustedLinks, lPathsIssuedCerts,
                                                       default_revoked_certs, &pkiProvider);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_HelperConfigServer_SetPKIprovider(pkiProvider);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        printf("<Test_Server_Client: Certificates and key loaded\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set namespaces
        const char* namespaces[] = {DEFAULT_APPLICATION_URI};
        status = SOPC_HelperConfigServer_SetNamespaces(1, namespaces);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed setting namespaces\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", NULL,
                                                                   OpcUa_ApplicationType_Server);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed setting application description \n");
        }
    }

    // Address space configuration

    const char* xml_address_space_config_path = getenv(TEST_SERVER_XML_ADDRESS_SPACE);
    SOPC_AddressSpace* address_space = NULL;

    if (NULL != xml_address_space_config_path)
    {
#ifdef WITH_EXPAT
        address_space = SOPC_LoadAddressSpaceConfigFromFile(xml_address_space_config_path);
#else
        printf(
            "Error: an XML address space configuration file path provided whereas XML library not available (Expat).\n"
            "Do not define environment variables TEST_SERVER_XML_ADDRESS_SPACE .\n"
            "Or compile with XML library available.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
#endif
    }
    else
    {
        if (SOPC_STATUS_OK == status)
        {
            address_space = SOPC_Embedded_AddressSpace_Load();
            status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_HelperConfigServer_SetAddressSpace(address_space);
        }
    }

    // Note: user manager are AllowAll by default

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

START_TEST(test_server_client)
{
    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize the server library (start library threads) */

    // Get default log config and set the custom path
    SOPC_Log_Configuration log_config = SOPC_Common_GetDefaultLogConfiguration();
    log_config.logLevel = SOPC_LOG_LEVEL_DEBUG;
    log_config.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_client_logs/";
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&log_config);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed initializing\n");
    }
    else
    {
        printf("<Test_Server_Client: initialized\n");
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Configuration of:
       - Server endpoints configuration:
         - Enpoint URL,
         - Security endpoint properties,
         - Cryptographic parameters,
         - Application description
       - Server address space initial content
       - User authentication and authorization management
    */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetServerConfiguration();
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Start server / Finalize configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed to configure the endpoint\n");
        }
        else
        {
            printf("<Test_Server_Client: Endpoint configured\n");
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Create client configuration */
    int32_t clientCfgId = -1;
    if (SOPC_STATUS_OK == status)
    {
        clientCfgId = client_create_configuration();
        if (clientCfgId > 0)
        {
            printf(">>Client: Successfully created configuration\n");
        }
        else
        {
            status = SOPC_STATUS_NOK;
            printf(">>Client: Failed to create configuration\n");
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Connect client to server */
    int32_t connectionId = 0;
    if (SOPC_STATUS_OK == status)
    {
        connectionId = SOPC_ClientHelper_CreateConnection(clientCfgId);

        if (connectionId <= 0)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Run a write service test */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_write_test(connectionId);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Test Write Success\n");
        }
        else
        {
            printf(">>Client: Test Write Failed\n");
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Run a read service test */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_read_req_test(connectionId);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Test Read Success\n");
        }
        else
        {
            printf(">>Client: Test Read Failed\n");
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

#ifdef WITH_EXPAT
#if 0 != S2OPC_NODE_MANAGEMENT
    /* Run an add nodes service test */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_add_nodes_req_test(connectionId);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Test AddNodes Success\n");
        }
        else
        {
            printf(">>Client: Test AddNodes Failed\n");
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);
#endif
#endif

    /* client request to close the connection */
    if (SOPC_STATUS_OK == status)
    {
        int32_t disconnectResult = SOPC_ClientHelper_Disconnect(connectionId);
        if (0 == disconnectResult && SOPC_Atomic_Int_Get(&connectionClosed) != 0)
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Clear client wrapper layer*/
    SOPC_ClientHelper_Finalize();

    /* Asynchronous request to close the endpoint */
    SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();

    /* Wait until endpoint is closed or stop server signal */
    while (SOPC_STATUS_OK == stopStatus && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    /* Clear the server wrapper layer */
    SOPC_HelperConfigServer_Clear();

    /* Clear the client/server toolkit library (stop all library threads) */
    SOPC_CommonHelper_Clear();

    if (NULL != addressSpace)
    {
        SOPC_AddressSpace_Delete(addressSpace);
        addressSpace = NULL;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Client final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Client final result: NOK with status = '%d'\n", status);
    }
}
END_TEST

static Suite* tests_make_suite_server_client(void)
{
    Suite* s = NULL;
    TCase* tc_server_client = NULL;

    s = suite_create("Server/Client");

    tc_server_client = tcase_create("Main test");
    tcase_add_test(tc_server_client, test_server_client);
    tcase_set_timeout(tc_server_client, 0);
    suite_add_tcase(s, tc_server_client);

    return s;
}

int main(void)
{
    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_make_suite_server_client());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
