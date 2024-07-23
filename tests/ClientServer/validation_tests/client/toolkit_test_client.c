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
#include "libs2opc_request_builder.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encodeabletype.h"
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
// PKI path
#define SOPC_PKI_PATH "./S2OPC_Demo_PKI"
#endif // WITH_STATIC_SECURITY_DATA

// User certificate path
#define USER_CERT_PATH "./user_public/user_2k_cert.der"
// User key path
#define USER_KEY_PATH "./user_private/encrypted_user_2k_key.pem"

// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 200;
// Loop timeout in milliseconds
static const uint32_t loopTimeout = 10000;

static int32_t getEndpointsReceived = 0;

static uint32_t cptReadResps = 0;

#if S2OPC_NANO_PROFILE
#define TEST_SUB_SERVICE_SUPPORTED false
#else
#define TEST_SUB_SERVICE_SUPPORTED true
#endif

static const char* monitoredNodeIds[3] = {"ns=1;i=1012", "i=2258", "ns=1;s=Boolean_001"};
static const uintptr_t monitoredItemIndexes[3] = {0, 1, 2};
static const unsigned int monitoredItemExpNotifs[3] = {1, 2, 1};
static unsigned int monitoredItemNotifs[3] = {0, 0, 0};

static SOPC_ReturnStatus wait_service_response(void)
{
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

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

    return status;
}

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

static void SOPC_Client_SubscriptionNotification_Cb(const SOPC_ClientHelper_Subscription* subscription,
                                                    SOPC_StatusCode status,
                                                    SOPC_EncodeableType* notificationType,
                                                    uint32_t nbNotifElts,
                                                    const void* notification,
                                                    uintptr_t* monitoredItemCtxArray)
{
    uintptr_t userCtx = SOPC_ClientHelperNew_Subscription_GetUserParam(subscription);
    SOPC_ASSERT(12 == userCtx);

    if (SOPC_IsGoodStatus(status) && &OpcUa_DataChangeNotification_EncodeableType == notificationType)
    {
        test_results_set_service_result(true);
        const OpcUa_DataChangeNotification* notifs = (const OpcUa_DataChangeNotification*) notification;
        for (uint32_t i = 0; i < nbNotifElts; i++)
        {
            printf("Value change notification for node %s:\n",
                   (const char*) monitoredNodeIds[monitoredItemCtxArray[i]]);
            SOPC_Variant_Print(&notifs->MonitoredItems[i].Value.Value);
            printf("\n");
            monitoredItemNotifs[monitoredItemCtxArray[i]]++;
        }
        bool expectedResult = true;
        for (size_t i = 0; i < sizeof(monitoredItemExpNotifs) / sizeof(monitoredItemExpNotifs[0]); i++)
        {
            if (monitoredItemNotifs[i] != monitoredItemExpNotifs[i])
            {
                expectedResult = false;
            }
        }
        test_results_set_service_result(expectedResult);
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
    status = SOPC_EncodeableObject_Create(&OpcUa_GetEndpointsRequest_EncodeableType, (void**) &getEndpointReq);
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
        status = SOPC_ClientConfigHelper_Initialize();
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
        SOPC_CertificateList* static_cacert = NULL;
        SOPC_CRLList* static_crl = NULL;

        /* Load client certificates and key from C source files (no filesystem needed) */
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromBytes(sizeof(client_2k_cert), client_2k_cert,
                                                                 sizeof(client_2k_key), client_2k_key);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &static_cacert);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &static_crl);
        }

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateFromList(static_cacert, static_crl, NULL, NULL, &pkiProvider);
        }
        SOPC_KeyManager_Certificate_Free(static_cacert);
        SOPC_KeyManager_CRL_Free(static_crl);
#else // WITH_STATIC_SECURITY_DATA == false

        /* Load client certificate and key from files */
        status = SOPC_ClientConfigHelper_SetKeyCertPairFromPath(CLI_CERT_PATH, CLI_KEY_PATH, true);

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateFromStore(SOPC_PKI_PATH, &pkiProvider);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Test_Client: Failed to create PKI\n");
        }
        else
        {
            printf(">>Test_Client: PKI created\n");
        }

#endif

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ClientConfigHelper_SetPKIprovider(pkiProvider);
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
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetPreferredLocaleIds(
        (sizeof(preferred_locale_ids) / sizeof(preferred_locale_ids[0]) - 1), preferred_locale_ids);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetApplicationDescription(APPLICATION_URI, APPLICATION_URI, APPLICATION_NAME,
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
        SOPC_SecureConnection_Config* secureConnConfig1 = SOPC_ClientConfigHelper_CreateSecureConnection(
            "1", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
        SOPC_SecureConnection_Config* secureConnConfig2 = SOPC_ClientConfigHelper_CreateSecureConnection(
            "2", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
        SOPC_SecureConnection_Config* secureConnConfig3 = SOPC_ClientConfigHelper_CreateSecureConnection(
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
        status = SOPC_ClientConfigHelper_GetSecureConnectionConfigs(nbSecConnCfgs, secureConnConfigArray);
    }

    // Load server certificate
    if (MSG_SECURITY_MODE != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        for (size_t i = 0; SOPC_STATUS_OK == status && i < *nbSecConnCfgs; i++)
        {
#ifdef WITH_STATIC_SECURITY_DATA
            status = SOPC_SecureConnectionConfig_SetServerCertificateFromBytes((*secureConnConfigArray)[i],
                                                                               sizeof(server_2k_cert), server_2k_cert);
#else
            status =
                SOPC_SecureConnectionConfig_SetServerCertificateFromPath((*secureConnConfigArray)[i], SRV_CERT_PATH);
#endif

            // Set username  as authentication mode for second connection
            if (i == 1)
            {
                status = SOPC_SecureConnectionConfig_SetUserName((*secureConnConfigArray)[i], "username_Basic256Sha256",
                                                                 NULL, NULL);
            }
            // Set X509 as authentication mode for third connection
            else if (i == 2)
            {
#ifdef WITH_STATIC_SECURITY_DATA
                status = SOPC_SecureConnectionConfig_SetUserX509FromBytes((*secureConnConfigArray)[i], "X509",
                                                                          sizeof(user_2k_cert), user_2k_cert,
                                                                          sizeof(user_2k_key), user_2k_key);
#else
                status = SOPC_SecureConnectionConfig_SetUserX509FromPaths((*secureConnConfigArray)[i], "X509",
                                                                          USER_CERT_PATH, USER_KEY_PATH, true);
#endif
            }
        }
    }

    return status;
}

#ifndef WITH_STATIC_SECURITY_DATA

static bool SOPC_GetClientUserKeyPassword(const SOPC_SecureConnection_Config* secConnConfig,
                                          const char* cert1Sha1,
                                          char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    bool res = SOPC_TestHelper_AskPassWithContext_FromEnv(cert1Sha1, outPassword);
    return res;
}
#endif // WITH_STATIC_SECURITY_DATA

static bool SOPC_GetClientUser1Password(const SOPC_SecureConnection_Config* secConnConfig,
                                        char** outUserName,
                                        char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
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
    const char* xml_client_config_path = NULL;

#ifndef WITH_STATIC_SECURITY_DATA
    xml_client_config_path = getenv("TEST_CLIENT_XML_CONFIG");

    if (NULL != xml_client_config_path)
    {
#ifdef WITH_EXPAT
        status = SOPC_ClientConfigHelper_ConfigureFromXML(xml_client_config_path, NULL, nbSecConnCfgs,
                                                          secureConnConfigArray);
#else
        printf(
            "Error: an XML client configuration file path provided whereas XML library not available (Expat).\n"
            "Do not define environment variables TEST_CLIENT_XML_CONFIG.\n"
            "Or compile with XML library available.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
#endif // WITH_EXPAT
    }

    // Set callback necessary to retrieve client key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
    // Set callback necessary to retrieve user key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserKeyPasswordCallback(&SOPC_GetClientUserKeyPassword);
    }
#endif // WITH_STATIC_SECURITY_DATA

    // Set callback necessary to retrieve user password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&SOPC_GetClientUser1Password);
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

static SOPC_ReturnStatus test_non_reg_issue_1428_create_MI(SOPC_ClientHelper_Subscription* subscription)
{
    SOPC_ASSERT(NULL != subscription);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_CreateMonitoredItemsRequest* createMonItReq =
        SOPC_CreateMonitoredItemsRequest_Create(0, 1, OpcUa_TimestampsToReturn_Both);
    if (NULL == createMonItReq)
    {
        status = SOPC_STATUS_NOK;
    }
    // 1st MI (with NodeId + some params with deadband filter)
    if (SOPC_STATUS_OK == status)
    {
        SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(monitoredNodeIds[0], (int32_t) strlen(monitoredNodeIds[0]));
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(createMonItReq, 0, nodeId, SOPC_AttributeId_Value,
                                                                     NULL);
        SOPC_NodeId_Clear(nodeId);
        SOPC_Free(nodeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Create an invalid/unknown filter type
        SOPC_ExtensionObject* extObj = SOPC_Calloc(1, sizeof(*extObj));
        SOPC_ASSERT(NULL != extObj);
        extObj->Encoding = SOPC_ExtObjBodyEncoding_ByteString;
        extObj->TypeId.NodeId.Namespace = 42;
        extObj->TypeId.NodeId.IdentifierType = SOPC_IdentifierType_Numeric;
        extObj->TypeId.NodeId.Data.Numeric = 1000;
        status =
            SOPC_String_AttachFromCstring((SOPC_String*) &extObj->Body.Bstring, "<<<I am unique kind of filter !>>>");
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
                createMonItReq, 0, OpcUa_MonitoringMode_Reporting, 0, -1, extObj, 1, true);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_ExtensionObject_Clear(extObj);
            SOPC_Free(extObj);
        }
    }

    // Create the monitored items
    OpcUa_CreateMonitoredItemsResponse createMonItResp;
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_CreateMonitoredItemsResponse_Initialize(&createMonItResp);
        status = SOPC_ClientHelperNew_Subscription_CreateMonitoredItems(
            subscription, createMonItReq, (const uintptr_t*) monitoredItemIndexes, &createMonItResp);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (!SOPC_IsGoodStatus(createMonItResp.ResponseHeader.ServiceResult) || 1 != createMonItResp.NoOfResults ||
            OpcUa_BadMonitoredItemFilterInvalid != createMonItResp.Results[0].StatusCode)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        SOPC_EncodeableObject_Delete(&OpcUa_CreateMonitoredItemsRequest_EncodeableType, (void**) &createMonItReq);
    }

    OpcUa_CreateMonitoredItemsResponse_Clear(&createMonItResp);

    return status;
}

static SOPC_ReturnStatus test_subscription(SOPC_ClientConnection* connection)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_CreateSubscriptionRequest* createSubReq = SOPC_CreateSubscriptionRequest_Create(500, 6, 2, 1000, true, 0);
    if (TEST_SUB_SERVICE_SUPPORTED)
    {
        /* In case the subscription service shall not be supported, check service response is unsupported service*/
        SOPC_ClientHelper_Subscription* subscription = SOPC_ClientHelperNew_CreateSubscription(
            connection, createSubReq, SOPC_Client_SubscriptionNotification_Cb, 12);
        SOPC_ASSERT(NULL != subscription);
        OpcUa_CreateMonitoredItemsRequest* createMonItReq =
            SOPC_CreateMonitoredItemsRequest_Create(0, 3, OpcUa_TimestampsToReturn_Both);
        if (NULL == createMonItReq)
        {
            status = SOPC_STATUS_NOK;
        }
        // 1st MI (with NodeId + some params with deadband filter)
        if (SOPC_STATUS_OK == status)
        {
            SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(monitoredNodeIds[0], (int32_t) strlen(monitoredNodeIds[0]));
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(createMonItReq, 0, nodeId,
                                                                         SOPC_AttributeId_Value, NULL);
            SOPC_NodeId_Clear(nodeId);
            SOPC_Free(nodeId);
        }
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ExtensionObject* extObj = SOPC_MonitoredItem_DataChangeFilter(
                OpcUa_DataChangeTrigger_Status, OpcUa_DeadbandType_Absolute, (double) 1000);
            SOPC_ASSERT(NULL != extObj);
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
                createMonItReq, 0, OpcUa_MonitoringMode_Reporting, 0, -1, extObj, 1, true);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_ExtensionObject_Clear(extObj);
                SOPC_Free(extObj);
            }
        }
        // 2nd MI (with NodeId  as C string + some params without deadband filter)
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemIdFromStrings(
                createMonItReq, 1, monitoredNodeIds[1], SOPC_AttributeId_Value, NULL);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
                createMonItReq, 1, OpcUa_MonitoringMode_Reporting, 0, 0, NULL, 10, true);
        }
        // 3rd MI (with NodeId  as C string + default params)
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemIdFromStrings(
                createMonItReq, 2, monitoredNodeIds[2], SOPC_AttributeId_Value, NULL);
        }

        // Create the monitored items
        OpcUa_CreateMonitoredItemsResponse createMonItResp;
        if (SOPC_STATUS_OK == status)
        {
            OpcUa_CreateMonitoredItemsResponse_Initialize(&createMonItResp);
            status = SOPC_ClientHelperNew_Subscription_CreateMonitoredItems(
                subscription, createMonItReq, (const uintptr_t*) monitoredItemIndexes, &createMonItResp);
        }

        // Check CreateMonitoredItems response and prepare delete monitored items request
        OpcUa_DeleteMonitoredItemsRequest* delMonItReq = NULL;
        OpcUa_DeleteMonitoredItemsResponse delMonItResp;
        OpcUa_DeleteMonitoredItemsResponse_Initialize(&delMonItResp);
        bool deleteMonitoredItems = false;
        if (SOPC_STATUS_OK == status)
        {
            // Check CreateMonitoredItems response
            if (SOPC_IsGoodStatus(createMonItResp.ResponseHeader.ServiceResult) && createMonItResp.NoOfResults == 3)
            {
                uint32_t queueSize = 1;
                delMonItReq = SOPC_DeleteMonitoredItemsRequest_Create(0, 3, NULL);
                for (int32_t i = 0; i < createMonItResp.NoOfResults; i++)
                {
                    if (!SOPC_IsGoodStatus(createMonItResp.Results[i].StatusCode))
                    {
                        status = SOPC_STATUS_NOK;
                    }
                    else
                    {
                        // Check revised queue size are the one requested (1, 10 and 100)
                        if (createMonItResp.Results[i].RevisedQueueSize != queueSize)
                        {
                            status = SOPC_STATUS_NOK;
                        }
                        queueSize *= 10;
                        SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId(delMonItReq, (size_t) i,
                                                                            createMonItResp.Results[i].MonitoredItemId);
                    }
                }
                deleteMonitoredItems = true;
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            // Reset expected result
            test_results_set_service_result(false);

            /* Wait until expected notifications are received */
            status = wait_service_response();
        }

        SOPC_ReturnStatus delMIstatus = SOPC_STATUS_OK;
        if (deleteMonitoredItems)
        {
            delMIstatus =
                SOPC_ClientHelperNew_Subscription_DeleteMonitoredItems(subscription, delMonItReq, &delMonItResp);
            OpcUa_CreateMonitoredItemsResponse_Clear(&createMonItResp);

            // Check DeleteMonitoredItems response
            if (SOPC_STATUS_OK == delMIstatus)
            {
                if (SOPC_IsGoodStatus(delMonItResp.ResponseHeader.ServiceResult) && delMonItResp.NoOfResults == 3)
                {
                    for (int32_t i = 0; i < delMonItResp.NoOfResults; i++)
                    {
                        if (!SOPC_IsGoodStatus(delMonItResp.Results[i]))
                        {
                            delMIstatus = SOPC_STATUS_NOK;
                        }
                    }
                }
                else
                {
                    delMIstatus = SOPC_STATUS_NOK;
                }
            }
            OpcUa_DeleteMonitoredItemsResponse_Clear(&delMonItResp);
        }

        /* NON REGRESSSION TEST FOR ISSUE #1428: server fail due to buffer overflow without fix */
        if (SOPC_STATUS_OK == status)
        {
            status = test_non_reg_issue_1428_create_MI(subscription);
        }

        SOPC_ReturnStatus delSubStatus = SOPC_ClientHelperNew_DeleteSubscription(&subscription);

        if (SOPC_STATUS_OK == status)
        {
            if (SOPC_STATUS_OK != delMIstatus)
            {
                status = delMIstatus;
            }
            else if (SOPC_STATUS_OK != delSubStatus)
            {
                status = delSubStatus;
            }
        }
    }
    else
    {
        // Note: we do not test the dedicated subscription API
        // because for now the behavior will be to close the connection

        // Reset expected result
        test_results_set_service_result(false);

        status = SOPC_ClientHelperNew_ServiceAsync(connection, createSubReq, OpcUa_BadServiceUnsupported);

        printf(">>Test_Client_Toolkit: create subscription sending\n");

        /* Wait until service response is received */
        if (SOPC_STATUS_OK == status)
        {
            status = wait_service_response();
        }
    }
    return status;
}

int main(void)
{
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
        status = SOPC_ClientConfigHelper_SetServiceAsyncResponse(SOPC_Client_AsyncRespCb);
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
        SOPC_SecureConnection_Config* reverseSecureConnConfig = SOPC_ClientConfigHelper_GetConfigFromId("3");
        if (NULL == reverseSecureConnConfig)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ClientHelperNew_DiscoveryServiceAsync(reverseSecureConnConfig, getGetEndpoints_message(), 1);
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
    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&getEndpointsReceived) == 0)
    {
        printf(">>Test_Client_Toolkit: GetEndpoints Response received: NOK\n");
        status = SOPC_STATUS_NOK;
    }
    else if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit: GetEndpoints Response received: OK\n");
    }

    /* Create the 3 connections */
    if (SOPC_STATUS_OK == status)
    {
        for (size_t i = 0; SOPC_STATUS_OK == status && i < nbSecConnCfgs; i++)
        {
            status =
                SOPC_ClientHelperNew_Connect(secureConnConfigArray[i], SOPC_Client_ConnEventCb, &secureConnections[i]);
        }
    }

    /* Read values on 1st connection */
    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit: Activated 3 sessions on 3 SCs: OK\n");
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        status = SOPC_ClientHelperNew_ServiceAsync(secureConnections[0], getReadRequest_message(), 1);
        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    if (SOPC_STATUS_OK == status)
    {
        status = wait_service_response();
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
        status = SOPC_ClientHelperNew_ServiceAsync(secureConnections[1], pWriteReqSent, 1);
        printf(">>Test_Client_Toolkit: write request sending\n");
    }

    /* Wait until service response is received */
    if (SOPC_STATUS_OK == status)
    {
        status = wait_service_response();
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
        status = SOPC_ClientHelperNew_ServiceAsync(secureConnections[0], getReadRequest_verif_message(), 2);
        printf(">>Test_Client_Toolkit: read request sending\n");
    }

    /* Wait until service response is received */
    if (SOPC_STATUS_OK == status)
    {
        status = wait_service_response();
    }

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReqCopy);

    if (SOPC_STATUS_OK == status)
    {
        status = test_subscription(secureConnections[0]);
    }

    /* Close the connections */
    for (size_t i = 0; i < sizeof(secureConnections) / sizeof(secureConnections[0]); i++)
    {
        if (NULL != secureConnections[i])
        {
            SOPC_ReturnStatus discoStatus = SOPC_ClientHelperNew_Disconnect(&secureConnections[i]);
            SOPC_ASSERT(SOPC_STATUS_OK == discoStatus);
        }
    }

    cptReadResps = 0;

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
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
