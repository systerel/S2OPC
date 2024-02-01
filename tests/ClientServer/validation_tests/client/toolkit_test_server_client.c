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
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_new_client.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#ifdef WITH_EXPAT
#include "xml_expat/sopc_uanodeset_loader.h"
#endif

#include "opcua_identifiers.h"

#include "embedded/sopc_addspace_loader.h"

#include "test_results.h"
#include "testlib_read_response.h"

#define SOPC_PKI_PATH "./S2OPC_Demo_PKI"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"

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

#define TEST_SERVER_XML_ADDRESS_SPACE "TEST_SERVER_XML_ADDRESS_SPACE"

// Define number of read values in read request to force multi chunk use in request and response:
// use max buffer size for 1 chunk and encoded size of a ReadValueId / DataValue which is 18 bytes in this test
#define NB_READ_VALUES ((SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE / 18) + 1)

static int32_t endpointClosed = false;

static const char* node_id_str = "ns=1;i=1012";
static const uint64_t write_value = 12;

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 500;

#define SHUTDOWN_PHASE_IN_SECONDS 5

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

/**
 * Callback to retrieve username and password for session activation
 */
static bool SOPC_GetClientUserKeyPassword(const SOPC_SecureConnection_Config* secConnConfig,
                                          const char* cert1Sha1,
                                          char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    bool res = SOPC_TestHelper_AskPassWithContext_FromEnv(cert1Sha1, outPassword);
    return res;
}

/*---------------------------------------------------------------------------
 *                          Client initialization
 *---------------------------------------------------------------------------*/

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

static OpcUa_GetEndpointsResponse* expectedEndpoints = NULL;

static SOPC_ReturnStatus client_create_configuration(SOPC_SecureConnection_Config** outSecureConnConfig)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    status = SOPC_ClientConfigHelper_SetPreferredLocaleIds(2, (const char*[]){"fr-FR", "en-US"});

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
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
        return status;
    }

    status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed to configure the client key password callback\n");
    }

    status = SOPC_ClientConfigHelper_SetUserKeyPasswordCallback(&SOPC_GetClientUserKeyPassword);
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Client: Failed to configure the user key password callback\n");
    }

    /* Load client certificate and key from files */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(CLI_CERT_PATH, CLI_KEY_PATH, true);
    }
    /* Create the PKI (Public Key Infrastructure) provider */
    SOPC_PKIProvider* pkiProvider = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromStore(SOPC_PKI_PATH, &pkiProvider);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf(">>Test_Client: Failed to create PKI\n");
    }
    else
    {
        printf(">>Test_Client: PKI created\n");
    }
    /* connect to the endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    if (NULL != secureConnConfig)
    {
        status = SOPC_SecureConnectionConfig_SetExpectedEndpointsDescription(secureConnConfig, expectedEndpoints);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetServerCertificateFromPath(secureConnConfig, SRV_CERT_PATH);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_SetUserX509FromPaths(
            secureConnConfig, SOPC_UserTokenPolicy_X509Basic256Sha256_ID, USER_CERT_PATH, USER_KEY_PATH, true);
    }
    if (SOPC_STATUS_OK == status)
    {
        *outSecureConnConfig = secureConnConfig;
    }
    return status;
}

/*---------------------------------------------------------------------------
 *                          client tests
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus client_send_write_test(SOPC_ClientConnection* secureConnection)
{
    /* create the write request */

    SOPC_DataValue* writeValue = SOPC_Calloc(1, sizeof(*writeValue));

    if (writeValue == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_DataValue_Initialize(writeValue);

    writeValue->Value.BuiltInTypeId = SOPC_UInt64_Id;
    writeValue->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    writeValue->Value.Value.Uint64 = write_value;

    // Create a write request to write the given node value
    OpcUa_WriteRequest* writeRequest = NULL;
    OpcUa_WriteResponse* writeResponse = NULL;

    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    writeRequest = SOPC_WriteRequest_Create(1);
    if (NULL != writeRequest)
    {
        status = SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, 0, node_id_str, SOPC_AttributeId_Value, NULL,
                                                            writeValue);
    }
    SOPC_DataValue_Clear(writeValue);
    SOPC_Free(writeValue);
    writeValue = NULL;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, writeRequest, (void**) &writeResponse);
    }

    if (SOPC_STATUS_OK == status && (writeResponse->ResponseHeader.ServiceResult & SOPC_GoodStatusOppositeMask) != 0 &&
        1 == writeResponse->NoOfResults && (writeResponse->Results[0] & SOPC_GoodStatusOppositeMask) != 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (NULL != writeResponse)
    {
        SOPC_EncodeableObject_Delete(writeResponse->encodeableType, (void**) &writeResponse);
    }
    return status;
}

static SOPC_ReturnStatus client_send_read_req_test(SOPC_ClientConnection* secureConnection)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;

    OpcUa_ReadResponse* readResponse = NULL;
    OpcUa_ReadRequest* readRequest = SOPC_ReadRequest_Create(NB_READ_VALUES, OpcUa_TimestampsToReturn_Both);
    if (NULL != readRequest)
    {
        status = SOPC_STATUS_OK;
        for (size_t i = 0; SOPC_STATUS_OK == status && i < NB_READ_VALUES; i++)
        {
            status =
                SOPC_ReadRequest_SetReadValueFromStrings(readRequest, i, node_id_str, SOPC_AttributeId_Value, NULL);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, readRequest, (void**) &readResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(readResponse->ResponseHeader.ServiceResult) &&
            NB_READ_VALUES == readResponse->NoOfResults)
        {
            /* Verify read results */
            for (size_t i = 0; SOPC_STATUS_OK == status && i < NB_READ_VALUES; i++)
            {
                SOPC_Variant* resultValue = &readResponse->Results[i].Value;

                if (resultValue->BuiltInTypeId != SOPC_UInt64_Id ||
                    resultValue->ArrayType != SOPC_VariantArrayType_SingleValue ||
                    resultValue->Value.Uint64 != write_value)
                {
                    status = SOPC_STATUS_NOK;
                }
            }
        }
        else
        {
            printf("Read failed with status: 0x%08" PRIX32 "\n", readResponse->ResponseHeader.ServiceResult);

            status = SOPC_STATUS_NOK;
        }
    }

    if (NULL != readResponse)
    {
        SOPC_EncodeableObject_Delete(readResponse->encodeableType, (void**) &readResponse);
    }

    return status;
}

#ifdef WITH_EXPAT
#if 0 != S2OPC_NODE_MANAGEMENT
static SOPC_ReturnStatus client_send_add_nodes_req_test(SOPC_ClientConnection* secureConnection)
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
        status = SOPC_ClientHelperNew_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus = SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != addNodesResp);
        if (!SOPC_IsGoodStatus(addNodesResp->ResponseHeader.ServiceResult) || addNodesResp->NoOfResults != 1 || addNodesResp->Results[0].StatusCode ||
            !SOPC_NodeId_Equal(&reqNodeId.NodeId, &addNodesResp->Results[0].AddedNodeId))
        {
            status = SOPC_STATUS_NOK;
        }

        SOPC_ReturnStatus delStatus = SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
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

    return space;
}
#endif

static SOPC_ReturnStatus Server_SetServerConfiguration(void)
{
    /* Load server endpoints configuration
     * use an embedded default demo server configuration.
     */

    SOPC_Endpoint_Config* ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
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
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed to configure the server key user password callback\n");
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromPath("./server_public/server_2k_cert.der",
                                                                "./server_private/encrypted_server_2k_key.pem", true);
    }

    // Set PKI configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_PKIProvider* pkiProvider = NULL;
        status = SOPC_PKIProvider_CreateFromStore(SOPC_PKI_PATH, &pkiProvider);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
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
        status = SOPC_ServerConfigHelper_SetNamespaces(1, namespaces);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed setting namespaces\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", NULL,
                                                                   OpcUa_ApplicationType_Server);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Client: Failed setting application description \n");
        }
    }

    // Address space configuration
    SOPC_AddressSpace* address_space = NULL;
    if (SOPC_STATUS_OK == status)
    {
        const char* xml_address_space_config_path = getenv(TEST_SERVER_XML_ADDRESS_SPACE);

        if (NULL != xml_address_space_config_path)
        {
#ifdef WITH_EXPAT
            address_space = SOPC_LoadAddressSpaceConfigFromFile(xml_address_space_config_path);
#else
            printf(
                "Error: an XML address space configuration file path provided whereas XML library not available "
                "(Expat).\n"
                "Do not define environment variables TEST_SERVER_XML_ADDRESS_SPACE .\n"
                "Or compile with XML library available.\n");
            status = SOPC_STATUS_INVALID_PARAMETERS;
#endif
        }
        else
        {
            address_space = SOPC_Embedded_AddressSpace_Load();
            status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
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
        status = SOPC_ServerConfigHelper_Initialize();
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
    SOPC_SecureConnection_Config* secConnConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = client_create_configuration(&secConnConfig);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Successfully created configuration\n");
        }
        else
        {
            printf(">>Client: Failed to create configuration\n");
        }
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Connect client to server */
    SOPC_ClientConnection* connection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_Connect(secConnConfig, &SOPC_Client_ConnEventCb, &connection);
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Run a write service test */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_write_test(connection);
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
        status = client_send_read_req_test(connection);
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
        status = client_send_add_nodes_req_test(connection);
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
        status = SOPC_ClientHelperNew_Disconnect(&connection);
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Clear client wrapper layer*/
    SOPC_ClientConfigHelper_Clear();

    /* Asynchronous request to close the endpoint */
    SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();

    /* Wait until endpoint is closed or stop server signal */
    while (SOPC_STATUS_OK == stopStatus && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    /* Clear the server wrapper layer */
    SOPC_ServerConfigHelper_Clear();

    /* Clear the client/server toolkit library (stop all library threads) */
    SOPC_CommonHelper_Clear();

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
