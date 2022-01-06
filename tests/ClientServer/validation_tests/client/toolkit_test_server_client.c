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
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"
#include "libs2opc_server_internal.h"

#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_manager.h"

#include "embedded/sopc_addspace_loader.h"

#include "test_results.h"
#include "testlib_read_response.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

#define USERNAME "user"
#define PASSWORD "pass"
#define INVALID_PASSWORD "nopass"

// Define number of read values in read request to force multi chunk use in request and response:
// use max buffer size for 1 chunk and encoded size of a ReadValueId / DataValue which is 18 bytes in this test
#define NB_READ_VALUES ((SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE / 18) + 1)

static char* default_trusted_root_issuers[] = {"trusted/cacert.der", /* Demo CA */
                                               NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der", NULL};

static int32_t endpointClosed = false;
static int32_t sessionClosed = false;
static int32_t sessionFailure = false;
static uint32_t session = 0;

static const char* node_id_str = "ns=1;i=1012";
static const uint64_t write_value = 12;

// test statuses: 0 - not done, > 0 - OK, < 0 - NOK
static int32_t read_test_status = 0;
static int32_t write_test_status = 0;

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
    (void) status;
    SOPC_Atomic_Int_Set(&endpointClosed, true);
}

/*
 * Client events callback
 */
static void Test_ComEvent_FctClient(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    /* avoid unused parameter compiler warning */
    (void) idOrStatus;
    (void) appContext;
    printf(">>Callback: received event %d\n", event);

    if (SE_RCV_SESSION_RESPONSE == event)
    {
        if (NULL != param)
        {
            SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) param;
            if (encType == &OpcUa_ReadResponse_EncodeableType)
            {
                printf(">>Test_Client_Toolkit: received ReadResponse \n");
                int32_t test_status = 1;
                OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) param;
                // perform verifications
                if (readResp->NoOfResults != NB_READ_VALUES)
                {
                    test_status = -1;
                }
                else
                {
                    /* Verify Read Result */
                    for (size_t i = 0; 1 == test_status && i < NB_READ_VALUES; i++)
                    {
                        SOPC_Variant* read_value = &readResp->Results[i].Value;

                        if (read_value->BuiltInTypeId != SOPC_UInt64_Id ||
                            read_value->ArrayType != SOPC_VariantArrayType_SingleValue ||
                            read_value->Value.Uint64 != write_value)
                        {
                            test_status = -1;
                        }
                    }
                }
                SOPC_Atomic_Int_Set(&read_test_status, test_status);
            }
            else if (encType == &OpcUa_WriteResponse_EncodeableType)
            {
                printf(">>Test_Client_Toolkit: received WriteResponse \n");

                int32_t test_status = 1;
                OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) param;

                if (writeResp->NoOfResults != 1 || SOPC_STATUS_OK != writeResp->Results[0])
                {
                    test_status = -1;
                }
                SOPC_Atomic_Int_Set(&write_test_status, test_status);
            }
            else if (encType == &OpcUa_ServiceFault_EncodeableType)
            {
                printf(">>Test_Client_Toolkit: received ServiceFault \n");
            }
            else
            {
                printf(">>Client: Received unexpected response\n");
            }
        }
    }
    else if (SE_ACTIVATED_SESSION == event)
    {
        printf(">>Client: ActivatedSession received\n");
        SOPC_Atomic_Int_Set((int32_t*) &session, (int32_t) idOrStatus);
    }
    else if (SE_SESSION_ACTIVATION_FAILURE == event)
    {
        printf(">>Client: Activation failure\n");
        SOPC_Atomic_Int_Set(&sessionFailure, true);
    }
    else if (SE_CLOSED_SESSION == event || SE_SESSION_ACTIVATION_FAILURE == event)
    {
        printf(">>Client: Session closed\n");
        SOPC_Atomic_Int_Set(&sessionClosed, true);
    }
    else
    {
        printf("<Test_Server_Client: unexpected endpoint event %d : NOK\n", event);
    }
}

/*---------------------------------------------------------------------------
 *                          Client initialization
 *---------------------------------------------------------------------------*/

// Avoid const qualifier in SOPC_SecureChannel_Config
SOPC_PKIProvider* clientPki = NULL;

static SOPC_SecureChannel_Config client_sc_config = {.isClientSc = true,
                                                     .clientConfigPtr = NULL,
                                                     .expectedEndpoints = NULL,
                                                     .serverUri = NULL,
                                                     .url = DEFAULT_ENDPOINT_URL,
                                                     .crt_cli = NULL,
                                                     .key_priv_cli = NULL,
                                                     .crt_srv = NULL,
                                                     .pki = NULL,
                                                     .reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI,
                                                     .requestedLifetime = 20000,
                                                     .msgSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt};

static SOPC_SecureChannel_Config client_user_sc_config = {.isClientSc = true,
                                                          .clientConfigPtr = NULL,
                                                          .expectedEndpoints = NULL,
                                                          .serverUri = NULL,
                                                          .url = DEFAULT_ENDPOINT_URL,
                                                          .crt_cli = NULL,
                                                          .key_priv_cli = NULL,
                                                          .crt_srv = NULL,
                                                          .pki = NULL,
                                                          .reqSecuPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI,
                                                          .requestedLifetime = 20000,
                                                          .msgSecurityMode = OpcUa_MessageSecurityMode_Sign};

static SOPC_Client_Config clientConfig = {.freeCstringsFlag = false, .clientLocaleIds = (char*[]){"fr-FR", NULL}};

static OpcUa_GetEndpointsResponse* expectedEndpoints = NULL;

static SOPC_ReturnStatus client_create_configuration(bool username, uint32_t* client_channel_config_idx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t channel_config_idx = 0;

    SOPC_SerializedCertificate* crt_cli = NULL;
    SOPC_SerializedCertificate* crt_srv = NULL;
    SOPC_SerializedAsymmetricKey* key_priv_cli = NULL;
    SOPC_PKIProvider* pki = NULL;

    SOPC_SecureChannel_Config* scConfig = NULL;
    if (username)
    {
        scConfig = &client_user_sc_config;
    }
    else
    {
        scConfig = &client_sc_config;
    }

    // Set client description
    OpcUa_ApplicationDescription_Initialize(&clientConfig.clientDescription);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&clientConfig.clientDescription.ApplicationName.defaultText,
                                               "S2OPC toolkit client example");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&clientConfig.clientDescription.ProductUri, DEFAULT_PRODUCT_URI);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&clientConfig.clientDescription.ApplicationUri, DEFAULT_APPLICATION_URI);
    }
    if (SOPC_STATUS_OK == status)
    {
        clientConfig.clientDescription.ApplicationType = OpcUa_ApplicationType_Client;
    }
    if (SOPC_STATUS_OK == status)
    {
        scConfig->clientConfigPtr = &clientConfig;
    }

    // Set expected endpoints
    if (NULL == expectedEndpoints)
    {
        // Retrieve endpoints from server
        OpcUa_GetEndpointsRequest* request = SOPC_GetEndpointsRequest_Create(DEFAULT_ENDPOINT_URL);
        if (NULL != request)
        {
            status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &expectedEndpoints);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK == status)
        {
            scConfig->expectedEndpoints = expectedEndpoints;
        }
    }

    /* load certificates and key */
    if (SOPC_STATUS_OK == status)
    {
        const char* client_cert_location = "./client_public/client_2k_cert.der";
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(client_cert_location, &crt_cli);
        if (SOPC_STATUS_OK == status)
        {
            scConfig->crt_cli = crt_cli;
            printf(">>Client: Client certificate loaded\n");
        }
        else
        {
            printf(">>Client: Failed to load client certificate\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        const char* server_cert_location = "./server_public/server_2k_cert.der";
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(server_cert_location, &crt_srv);
        if (SOPC_STATUS_OK == status)
        {
            scConfig->crt_srv = crt_srv;
            printf(">>Client: Server certificate loaded\n");
        }
        else
        {
            printf(">>Client: Failed to load server certificate\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        const char* client_key_priv_location = "./client_private/client_2k_key.pem";
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(client_key_priv_location, &key_priv_cli);
        if (SOPC_STATUS_OK == status)
        {
            scConfig->key_priv_cli = key_priv_cli;
            printf(">>Client: Client private key loaded\n");
        }
        else
        {
            printf(">>Client: Failed to load client private key\n");
        }
    }

    /* create PKI */

    if (SOPC_STATUS_OK == status)
    {
        char* lPathsTrustedLinks[] = {NULL};
        char* lPathsUntrustedRoots[] = {NULL};
        char* lPathsUntrustedLinks[] = {NULL};
        char* lPathsIssuedCerts[] = {NULL};
        status = SOPC_PKIProviderStack_CreateFromPaths(default_trusted_root_issuers, lPathsTrustedLinks,
                                                       lPathsUntrustedRoots, lPathsUntrustedLinks, lPathsIssuedCerts,
                                                       default_revoked_certs, &pki);
        if (SOPC_STATUS_OK == status)
        {
            scConfig->pki = pki;
            clientPki = pki;
            printf(">>Client: PKI created\n");
        }
        else
        {
            printf(">>Client: Failed to create PKI\n");
        }
    }

    /* add secure channel config */
    if (SOPC_STATUS_OK == status)
    {
        channel_config_idx = SOPC_ToolkitClient_AddSecureChannelConfig(scConfig);
        if (0 != channel_config_idx)
        {
            *client_channel_config_idx = channel_config_idx;
        }
        else
        {
            status = SOPC_STATUS_NOK;
            printf(">>Client: Failed to add secure channel configuration\n");
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(crt_cli);
        SOPC_KeyManager_SerializedCertificate_Delete(crt_srv);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(key_priv_cli);
        SOPC_PKIProvider_Free(&pki);
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                          client tests
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus client_send_write_req_test(uint32_t session_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_WriteRequest* write_request = NULL;
    SOPC_NodeId* node_id_ptr = NULL;
    OpcUa_WriteValue* node_to_write = NULL;

    /* create the read request */

    write_request = SOPC_Calloc(1, sizeof(OpcUa_WriteRequest));

    if (NULL != write_request)
    {
        status = SOPC_STATUS_OK;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_WriteRequest_Initialize(write_request);

        node_to_write = NULL;

        node_to_write = SOPC_Calloc(1, sizeof(OpcUa_WriteValue));
        if (NULL == node_to_write)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            OpcUa_WriteValue_Initialize(node_to_write);

            node_id_ptr = SOPC_NodeId_FromCString(node_id_str, (int32_t) strlen(node_id_str));
            if (NULL == node_id_ptr)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_NodeId_Copy(&node_to_write->NodeId, node_id_ptr);
                node_to_write->AttributeId = 13;
                SOPC_DataValue_Initialize(&node_to_write->Value);
                node_to_write->Value.Value.BuiltInTypeId = SOPC_UInt64_Id;
                node_to_write->Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
                node_to_write->Value.Value.Value.Uint64 = write_value;

                write_request->NoOfNodesToWrite = 1;
                write_request->NodesToWrite = node_to_write;
            }

            SOPC_Free(node_id_ptr);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(node_to_write);
        SOPC_Free(write_request);
    }

    /* send the request */
    if (SOPC_STATUS_OK == status)
    {
        printf("Sending the Write request!\n");
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session_id, write_request, 1);
    }

    /* wait for the response verification (done in toolkit callback) */
    const int32_t write_count_limit = 5;
    int32_t write_count = 0;

    while (SOPC_Atomic_Int_Get(&write_test_status) == 0 && write_count < write_count_limit)
    {
        printf("Waiting for write request to be done\n");
        write_count++;
        SOPC_Sleep(sleepTimeout);
    }

    ck_assert_int_gt(SOPC_Atomic_Int_Get(&write_test_status), 0);

    return status;
}

static SOPC_ReturnStatus client_send_read_req_test(uint32_t session_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_ReadRequest* read_request = NULL;

    /* create the read request */
    read_request = SOPC_Calloc(1, sizeof(OpcUa_ReadRequest));
    if (NULL != read_request)
    {
        status = SOPC_STATUS_OK;
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_ReadRequest_Initialize(read_request);

        OpcUa_ReadValueId* nodes_to_read = NULL;

        // We make a read of NB_READ_VALUES values to force multi chunks use
        nodes_to_read = SOPC_Calloc(NB_READ_VALUES, sizeof(OpcUa_ReadValueId));

        if (NULL == nodes_to_read)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            SOPC_Free(read_request);
        }
        else
        {
            SOPC_NodeId* node_id_ptr = SOPC_NodeId_FromCString(node_id_str, (int32_t) strlen(node_id_str));
            if (NULL == node_id_ptr)
            {
                SOPC_Free(read_request);
                SOPC_Free(nodes_to_read);
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            for (size_t i = 0; SOPC_STATUS_OK == status && i < NB_READ_VALUES; i++)
            {
                OpcUa_ReadValueId* node_to_read = &nodes_to_read[i];
                OpcUa_ReadValueId_Initialize(node_to_read);
                node_to_read->AttributeId = 13;
                status = SOPC_NodeId_Copy(&node_to_read->NodeId, node_id_ptr);
            }
            if (SOPC_STATUS_OK == status)
            {
                read_request->NoOfNodesToRead = NB_READ_VALUES;
                read_request->NodesToRead = nodes_to_read;
            }
            SOPC_Free(node_id_ptr);
        }
    }

    /* send the request */
    if (SOPC_STATUS_OK == status)
    {
        printf("Sending the Read request!\n");
        SOPC_ToolkitClient_AsyncSendRequestOnSession(session_id, read_request, 1);
    }

    /* wait for the response verification (done in toolkit callback) */
    const int32_t read_count_limit = 5;
    int32_t read_count = 0;

    while (SOPC_Atomic_Int_Get(&read_test_status) == 0 && read_count < read_count_limit)
    {
        printf("Waiting for read request to be done\n");
        read_count++;
        SOPC_Sleep(sleepTimeout);
    }

    ck_assert_int_gt(SOPC_Atomic_Int_Get(&read_test_status), 0);

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------
 * Application description and endpoint configuration:
 *---------------------------------------------------*/

static SOPC_ReturnStatus authentication_fct(SOPC_UserAuthentication_Manager* authn,
                                            const SOPC_ExtensionObject* token,
                                            SOPC_UserAuthentication_Status* authenticated)
{
    /* avoid unused parameter compiler warning */
    (void) (authn);

    if (NULL == token || NULL == authenticated)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    if (SOPC_ExtObjBodyEncoding_Object != token->Encoding)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        if (strcmp(SOPC_String_GetRawCString(username), USERNAME) == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen(PASSWORD) && memcmp(pwd->Data, PASSWORD, strlen(PASSWORD)) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions authentication_uactt_functions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) SOPC_Free,
    .pFuncValidateUserIdentity = authentication_fct};

static const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_B256 = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {12, true, (SOPC_Byte*) "usernameB256"},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {sizeof(SECURITY_POLICY_BASIC256) - 1, true, (SOPC_Byte*) SECURITY_POLICY_BASIC256},
    /* Default security policy shall be used only when
   secure channel security policy is non-None since password will be non-encrypted */
};

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

    SOPC_ReturnStatus status =
        SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_B256);
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
        char* namespaces[] = {DEFAULT_APPLICATION_URI};
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

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
    }
    if (NULL == authenticationManager)
    {
        SOPC_UserAuthentication_FreeManager(&authenticationManager);
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set a user authentication function that complies with UACTT tests expectations */
        authenticationManager->pFunctions = &authentication_uactt_functions;
        SOPC_HelperConfigServer_SetUserAuthenticationManager(authenticationManager);
    }

    sopc_helper_config.server.configuredSecondsTillShutdown = 0;

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

static void tests_server_client_fct(bool username, bool user_specific_encrypt)
{
    SOPC_Atomic_Int_Set(&endpointClosed, false);
    SOPC_Atomic_Int_Set(&sessionClosed, false);
    SOPC_Atomic_Int_Set(&sessionFailure, false);
    SOPC_Atomic_Int_Set((int32_t*) &session, 0);
    const char* policyId = NULL;
    if (username)
    {
        if (user_specific_encrypt)
        {
            policyId = "usernameB256";
        }
        else
        {
            policyId = "username";
        }
    }
    else
    {
        policyId = "anonymous";
    }

    uint32_t client_channel_config_idx = 0;

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

    /*
     * Define callback for low-level client events
     */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CommonHelper_SetClientComEvent(Test_ComEvent_FctClient);
    }

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

    /* Create client configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = client_create_configuration(username, &client_channel_config_idx);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Successfully created configuration\n");
        }
        else
        {
            printf(">>Client: Failed to create configuration\n");
        }
    }

    /* Connect client to server */
    if (SOPC_STATUS_OK == status)
    {
        if (username)
        {
            // First attempt with wrong password
            status = SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
                client_channel_config_idx, NULL, (uintptr_t) NULL, policyId, USERNAME,
                (const uint8_t*) INVALID_PASSWORD, (int32_t) strlen(INVALID_PASSWORD));

            /* verify session activation failure */
            int32_t count = 0;
            const int32_t count_limit = 5;
            while (SOPC_Atomic_Int_Get(&sessionFailure) == 0 && count < count_limit)
            {
                SOPC_Sleep(sleepTimeout);
                count++;
            }
            ck_assert_int_lt(count, count_limit);

            // Second attempt with correct password
            status = SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(
                client_channel_config_idx, NULL, (uintptr_t) NULL, policyId, USERNAME, (const uint8_t*) PASSWORD,
                (int32_t) strlen(PASSWORD));
        }
        else
        {
            status = SOPC_ToolkitClient_AsyncActivateSession_Anonymous(client_channel_config_idx, NULL,
                                                                       (uintptr_t) NULL, policyId);
        }
    }

    /* verify session activation */
    int32_t count = 0;
    const int32_t count_limit = 5;
    while (SOPC_Atomic_Int_Get((int32_t*) &session) == 0 && count < count_limit)
    {
        SOPC_Sleep(sleepTimeout);
        count++;
    }

    ck_assert_int_lt(count, count_limit);

    /* send a write request */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_write_req_test(session);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Test Write Request Success\n");
        }
        else
        {
            printf(">>Client: Test Write Request Failed\n");
        }
    }

    /* send a read request */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_read_req_test(session);
        if (SOPC_STATUS_OK == status)
        {
            printf(">>Client: Test Read Request Success\n");
        }
        else
        {
            printf(">>Client: Test Read Request Failed\n");
        }
    }

    printf(">>Client: Closing session\n");
    /* client request to close the connection */
    while (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&sessionClosed) == 0)
    {
        SOPC_ToolkitClient_AsyncCloseSession((uint32_t) SOPC_Atomic_Int_Get((int32_t*) &session));
        SOPC_Sleep(sleepTimeout);
    }

    printf(">>Client: Stopping server\n");
    /* Asynchronous request to close the endpoint */
    SOPC_ReturnStatus stopStatus = SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        stopStatus = SOPC_ServerHelper_StopServer();
    }

    /* Wait until endpoint is closed or stop server signal */
    while (SOPC_STATUS_OK == stopStatus && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    /* Clear the server library (stop all library threads) */
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();
    SOPC_PKIProvider_Free(&clientPki);

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Client final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Client final result: NOK with status = '%d'\n", status);
    }
}

START_TEST(test_server_client_anonymous)
{
    tests_server_client_fct(false, false);
}
END_TEST

START_TEST(test_server_client_username)
{
    tests_server_client_fct(true, false);
}
END_TEST

START_TEST(test_server_client_username_specific_encrypt)
{
    tests_server_client_fct(true, true);
}
END_TEST

static Suite* tests_make_suite_server_client(void)
{
    Suite* s = NULL;
    TCase* tc_server_client = NULL;

    s = suite_create("Server/Client");

    tc_server_client = tcase_create("Main test");
    tcase_add_test(tc_server_client, test_server_client_anonymous);
    tcase_add_test(tc_server_client, test_server_client_username);
    tcase_add_test(tc_server_client, test_server_client_username_specific_encrypt);
    OpcUa_GetEndpointsResponse_Clear(expectedEndpoints);
    SOPC_Free(expectedEndpoints);
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
