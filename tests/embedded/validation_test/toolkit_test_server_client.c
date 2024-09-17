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

#include <stdlib.h>
#include <string.h>

// Server wrapper
#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_client.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "opcua_identifiers.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_common_constants.h"
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "embedded/sopc_addspace_loader.h"
#include "static_security_data.h"

#include "../unit_test/unit_test_include.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI_2 "urn:S2OPC:localhost_2"
#define DEFAULT_APPLICATION_NAME "Test_Client_S2OPC"

/* Define application namespaces: ns=1 and ns=2 */
static const char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI, DEFAULT_PRODUCT_URI_2};
static const char* default_locale_ids[] = {"en-US", "fr-FR"};

#define TEST_SERVER_XML_ADDRESS_SPACE "TEST_SERVER_XML_ADDRESS_SPACE"

static int32_t endpointClosed = false;

static const char* node_id_str = "ns=1;s=PubInt16";
static const uint16_t write_value = 12;

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 500;

#define SHUTDOWN_PHASE_IN_SECONDS 5

// generated address space.
extern const bool sopc_embedded_is_const_addspace;
extern SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[];
extern const uint32_t SOPC_Embedded_AddressSpace_nNodes;
extern const uint32_t SOPC_Embedded_VariableVariant_nb;
extern SOPC_Variant SOPC_Embedded_VariableVariant[];

/*---------------------------------------------------------------------------
 *                          Common Utilities
 *---------------------------------------------------------------------------*/

static void log_UserCallback(const char* timestampUtc,
                             const char* category,
                             const SOPC_Log_Level level,
                             const char* const line)
{
    SOPC_UNUSED_ARG(timestampUtc);
    SOPC_UNUSED_ARG(category);
    SOPC_UNUSED_ARG(level);
    if (NULL != line)
    {
        PRINT("%s\r\n", line);
    }
}

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

/*---------------------------------------------------------------------------
 *                          Client initialization
 *---------------------------------------------------------------------------*/

static SOPC_SecureConnection_Config* client_create_configuration(void)
{
    char* epURL = NULL;

    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();

    if (SOPC_STATUS_OK == status)
    {
        epURL = SOPC_strdup(DEFAULT_ENDPOINT_URL);
    }

    /* configure the connection */
    SOPC_SecureConnection_Config* configuration = SOPC_ClientConfigHelper_CreateSecureConnection(
        "CLI_Client", epURL, OpcUa_MessageSecurityMode_None, SOPC_SecurityPolicy_None);
    return configuration;
}

/***************************************************/
// Callback for unexpected connection events
static void client_ConnectionEventCallback(SOPC_ClientConnection* config,
                                           SOPC_ClientConnectionEvent event,
                                           SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    PRINT("UNEXPECTED CONNECTION EVENT %d with status 0x%08" PRIX32 "\n", event, status);
}

/*---------------------------------------------------------------------------
 *                          client tests
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus client_send_write_test(SOPC_ClientConnection* secureConnection)
{
    // Create a write request to write the given node value
    OpcUa_WriteRequest* writeRequest = NULL;
    OpcUa_WriteResponse* writeResponse = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_DataValue* writeDataValue = NULL;

    writeDataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
    SOPC_ASSERT(NULL != writeDataValue);
    SOPC_DataValue_Initialize(writeDataValue);
    writeDataValue->Value.BuiltInTypeId = SOPC_Int16_Id;
    writeDataValue->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    writeDataValue->Value.Value.Uint16 = write_value;

    if (SOPC_STATUS_OK == status)
    {
        writeRequest = SOPC_WriteRequest_Create(1);
        if (NULL != writeRequest)
        {
            status = SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, 0, node_id_str, SOPC_AttributeId_Value,
                                                                NULL, writeDataValue);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (NULL != writeDataValue)
    {
        SOPC_DataValue_Clear(writeDataValue);
        SOPC_Free(writeDataValue);
        writeDataValue = NULL;
    }

    // Call the write service
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, writeRequest, (void**) &writeResponse);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(writeResponse->ResponseHeader.ServiceResult))
        {
            if (1 == writeResponse->NoOfResults && SOPC_IsGoodStatus(writeResponse->Results[0]))
            {
                PRINT("Write of value %d succeeded\n", write_value);
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            PRINT("Write service failed with status: 0x%08" PRIX32 "\n", writeResponse->ResponseHeader.ServiceResult);

            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != writeResponse)
    {
        OpcUa_WriteResponse_Clear(writeResponse);
    }
    return status;
}

static SOPC_ReturnStatus client_send_read_req_test(SOPC_ClientConnection* secureConnection)
{
    OpcUa_ReadRequest* readRequest = NULL;
    OpcUa_ReadResponse* readResponse = NULL;
    const uint8_t nbReadValue = 1;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    readRequest = SOPC_ReadRequest_Create(nbReadValue, OpcUa_TimestampsToReturn_Both);
    if (NULL != readRequest)
    {
        status = SOPC_ReadRequest_SetReadValueFromStrings(readRequest, nbReadValue - 1, node_id_str,
                                                          SOPC_AttributeId_Value, NULL);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, readRequest, (void**) &readResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(readResponse->ResponseHeader.ServiceResult) && nbReadValue == readResponse->NoOfResults)
        {
            if (write_value == readResponse->Results->Value.Value.Uint16)
            {
                PRINT("<Test_Client_Toolkit: Read Value succeed\n");
            }
            else
            {
                PRINT("<Test_Client_Toolkit: Read Value Failed\n");
            }
        }
        else
        {
            PRINT("Read failed with status: 0x%08" PRIX32 "\n", readResponse->ResponseHeader.ServiceResult);

            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != readResponse)
    {
        OpcUa_ReadResponse_Clear(readResponse);
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*
 *  Configure the cryptographic parameters of the endpoint:
 * - Server certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Server_SetDefaultCryptographicConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_CertificateList* ca_cert = NULL;
    SOPC_CRLList* crl = NULL;
    SOPC_PKIProvider* pkiProvider = NULL;

    /* Load client/server certificates and server key from C source files (no filesystem needed) */
    status = SOPC_HelperConfigServer_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                             sizeof(server_2k_key), server_2k_key);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &ca_cert);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &crl);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(ca_cert, crl, NULL, NULL, &pkiProvider);
    }

    SOPC_KeyManager_Certificate_Free(ca_cert);
    SOPC_KeyManager_CRL_Free(crl);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
    }

    if (SOPC_STATUS_OK != status)
    {
        PRINT("<Test_Server_Toolkit: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        PRINT("<Test_Server_Toolkit: Certificates and key loaded\n");
    }

    return status;
}

/*
 * Default server configuration loader (without XML configuration)
 */
static SOPC_ReturnStatus Server_SetDefaultConfiguration(void)
{
    // Set namespaces
    SOPC_ReturnStatus status = SOPC_ServerConfigHelper_SetNamespaces(1, default_app_namespace_uris);

    // Set locale ids
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetLocaleIds(2, default_locale_ids);
    }

    // Set application description of server to be returned by discovery services (GetEndpoints, FindServers)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", "en-US",
                                                                   OpcUa_ApplicationType_Server);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_AddApplicationNameLocale("S2OPC toolkit: exemple de serveur", "fr-FR");
    }

    /*
     * Create new endpoint in server
     */
    SOPC_Endpoint_Config* ep = NULL;
    if (SOPC_STATUS_OK == status)
    {
        ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
        status = NULL == ep ? SOPC_STATUS_OUT_OF_MEMORY : status;
    }

    /*
     * Define the certificates, security policies, security modes and user token policies supported by endpoint
     */
    SOPC_SecurityPolicy* sp;
    if (SOPC_STATUS_OK == status)
    {
        /*
         * 1st Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
         */
        sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);
        if (NULL == sp)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(
                sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
            }
            if (SOPC_STATUS_OK == status)
            {
                status =
                    SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
            }
        }

        /*
         * 2nd Security policy is Basic256 with anonymous and username (non encrypted) authentication allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256);
            if (NULL == sp)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_SecurityConfig_SetSecurityModes(
                    sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(
                        sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
                }
            }
        }

        /*
         * 4th Security policy is Aes128-Sha256-RsaOaep with anonymous and username (non encrypted) authentication
         * allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Aes128Sha256RsaOaep);
            if (NULL == sp)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_SecurityConfig_SetSecurityModes(
                    sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(
                        sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
                }
            }
        }

        /*
         * 5th Security policy is Aes256-Sha256-RsaPss with anonymous and username (non encrypted) authentication
         * allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Aes256Sha256RsaPss);
            if (NULL == sp)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_SecurityConfig_SetSecurityModes(
                    sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(
                        sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
                }
            }
        }
    }

    /*
     * 3rd Security policy is None with anonymous and username (non encrypted) authentication allowed
     * (for tests only, otherwise users on unsecure channel shall be forbidden
     *  and only discovery endpoint activated on a secured channel configuration)
     */
    if (SOPC_STATUS_OK == status)
    {
        sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
        if (NULL == sp)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_None);

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(
                    sp, &SOPC_UserTokenPolicy_Anonymous); /* Necessary for tests only */
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(
                    sp,
                    &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy); /* Necessary for UACTT tests only */
            }
        }
    }

    /**
     * Define server certificate and PKI provider
     */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultCryptographicConfig();
    }

    return status;
}

/*------------------------------
 * Address space configuration :
 *------------------------------*/

static SOPC_ReturnStatus Server_SetDefaultAddressSpace(void)
{
    /* Load embedded default server address space:
     * Use the embedded address space (already defined as C code) loader.
     * The address space C structure shall have been generated prior to compilation.
     * This should be done using the script ./scripts/generate-s2opc-address-space.py
     */

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    // Address space must be constant
    if (sopc_embedded_is_const_addspace)
    {
        SOPC_AddressSpace* addSpace =
            SOPC_AddressSpace_CreateReadOnlyNodes(SOPC_Embedded_AddressSpace_nNodes, SOPC_Embedded_AddressSpace_Nodes,
                                                  SOPC_Embedded_VariableVariant_nb, SOPC_Embedded_VariableVariant);
        status = (NULL != addSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerConfigHelper_SetAddressSpace(addSpace);
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        PRINT("<Test_Server_Toolkit: Failed to configure the @ space\n");
    }
    else
    {
        PRINT("<Test_Server_Toolkit: @ space configured\n");
    }

    return status;
}

static SOPC_ReturnStatus Server_SetServerConfiguration(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = Server_SetDefaultConfiguration();

    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultAddressSpace();
    }

    // Note: user manager are AllowAll by default

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

void suite_test_server_client(int* index)
{
    PRINT("\nTEST %d: validation server client\n", *index);

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    PRINT("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
          build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
          build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    PRINT("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
          build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
          build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize the server library (start library threads) */

    // Get default log config and set the custom path
    SOPC_Log_Configuration log_config = SOPC_Common_GetDefaultLogConfiguration();
    log_config.logLevel = SOPC_LOG_LEVEL_WARNING;
    log_config.logSystem = SOPC_LOG_SYSTEM_USER;
    log_config.logSysConfig.userSystemLogConfig.doLog = (SOPC_Log_UserDoLog*) &log_UserCallback;

    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&log_config);
    PRINT("status = %d\n", status);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        PRINT("<Test_Server_Client: Failed initializing\n");
    }
    else
    {
        PRINT("<Test_Server_Client: initialized\n");
    }
    SOPC_ASSERT(SOPC_STATUS_OK == status);

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
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    /* Start server / Finalize configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);

        if (SOPC_STATUS_OK != status)
        {
            PRINT("<Test_Server_Client: Failed to configure the endpoint\n");
        }
        else
        {
            PRINT("<Test_Server_Client: Endpoint configured\n");
        }
    }
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    /* Create client configuration */
    SOPC_SecureConnection_Config* clientConfiguration = NULL;
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        clientConfiguration = client_create_configuration();
        if (NULL == clientConfiguration)
        {
            status = SOPC_STATUS_NOK;
            PRINT(">>Client: Failed to create configuration\n");
        }
        else
        {
            PRINT(">>Client: Successfully created configuration\n");
        }
    }

    // Connect Client to server
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(clientConfiguration, client_ConnectionEventCallback, &secureConnection);
    }

    /* Run a write service test */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_write_test(secureConnection);
        if (SOPC_STATUS_OK == status)
        {
            PRINT(">>Client: Test Write Success\n");
        }
        else
        {
            PRINT(">>Client: Test Write Failed\n");
        }
    }
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    /* Run a read service test */
    if (SOPC_STATUS_OK == status)
    {
        status = client_send_read_req_test(secureConnection);
        if (SOPC_STATUS_OK == status)
        {
            PRINT(">>Client: Test Read Success\n");
        }
        else
        {
            PRINT(">>Client: Test Read Failed\n");
        }
    }
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    /* client request to close the connection */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Disconnect(&secureConnection);
    }

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
    SOPC_HelperConfigServer_Clear();

    /* Clear the client/server toolkit library (stop all library threads) */
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK == status)
    {
        PRINT("<Test_Server_Client final result: OK\n");
    }
    else
    {
        PRINT("<Test_Server_Client final result: NOK with status = '%d'\n", status);
    }
    PRINT("Test 1: ok\n");
    *index += 1;
}
