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
#include <stdbool.h>

#include "test_suite_client.h"

#include "libs2opc_client_config_custom.h"
#include "libs2opc_client.h"
#include "libs2opc_request_builder.h"
#include "sopc_encodeabletype.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_None
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_None

#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define GATEWAY_SERVER_URI ""

/* Event handlers of the Discovery */
static void ValidateGetEndpointsResponse(OpcUa_GetEndpointsResponse* pResp);

START_TEST(test_getEndpoints)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_discovery_getEndpoints_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "discovery", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);

    OpcUa_GetEndpointsRequest* getEpReq = SOPC_GetEndpointsRequest_Create(DEFAULT_ENDPOINT_URL);
    ck_assert_ptr_nonnull(getEpReq);
    OpcUa_GetEndpointsResponse* getEpResp = NULL;

    status = SOPC_ClientHelperNew_DiscoveryServiceSync(secureConnConfig, getEpReq, (void**) &getEpResp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ValidateGetEndpointsResponse(getEpResp);

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
}
END_TEST

void ValidateGetEndpointsResponse(OpcUa_GetEndpointsResponse* pResp)
{
    ck_assert(&OpcUa_GetEndpointsResponse_EncodeableType == pResp->encodeableType);

    OpcUa_EndpointDescription* pEndp = NULL;
    int32_t i = 0;
    bool bNoneChecked = false;
    bool bSecuChecked = false;
    bool bInconsistentPolicyMode = false;
    SOPC_Byte iSecLevelNone = 0;
    SOPC_Byte iSecLevelWithSecu = 0;
    SOPC_ByteString* pBufCert = NULL;
    SOPC_CertificateList* pCert = NULL;

    /* Check the presence of the None and Basic256 sec policy (free opc ua does not support B256S256 */
    for (i = 0; i < pResp->NoOfEndpoints; ++i)
    {
        pEndp = &pResp->Endpoints[i];
        pBufCert = &pEndp->ServerCertificate;
        /* As we asked for a GetEndpoints on ENDPOINT_URL, it should only return endpoints with that URL */
        /* TODO: freeopcua translates the given hostname to an IP, so it is not possible to check that */
        /* ck_assert(strncmp(SOPC_String_GetRawCString(&pEndp->EndpointUrl), ENDPOINT_URL, strlen(ENDPOINT_URL)
         * + 1)
         * == 0); */
        /* Check that SecPol None <=> SecMode None */
        bInconsistentPolicyMode = false;
        bInconsistentPolicyMode = strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri),
                                          SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0;
        bInconsistentPolicyMode ^= OpcUa_MessageSecurityMode_None == pEndp->SecurityMode;
        ck_assert(!bInconsistentPolicyMode);

        /* If it is None, there is nothing more to check. */
        if (strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_None_URI,
                    strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0)
        {
            bNoneChecked = true;
            iSecLevelNone = pEndp->SecurityLevel;
        }

        /* Check the received certificate: it shall be present */
        if (strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_Basic256_URI,
                    strlen(SOPC_SecurityPolicy_Basic256_URI) + 1) == 0 &&
            pEndp->ServerCertificate.Length > 0)
        {
            bSecuChecked = true;
            iSecLevelWithSecu = pEndp->SecurityLevel;
            ck_assert(SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
                                                                     &pCert) == SOPC_STATUS_OK);
            SOPC_KeyManager_Certificate_Free(pCert);
            pCert = NULL;
        }

        if (strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_Basic256Sha256_URI,
                    strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0 &&
            pEndp->ServerCertificate.Length > 0)
        {
            bSecuChecked = true;
            iSecLevelWithSecu = pEndp->SecurityLevel;
            ck_assert(SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
                                                                     &pCert) == SOPC_STATUS_OK);
            SOPC_KeyManager_Certificate_Free(pCert);
            pCert = NULL;
        }
    }

    /* Does not check that a security policy is not described multiple times */
    ck_assert(bNoneChecked && bSecuChecked);
    /* Freeopcua always use 0 as SecurityLevel... */
    ck_assert(iSecLevelWithSecu >= iSecLevelNone);

    SOPC_EncodeableObject_Delete(pResp->encodeableType, (void**) &pResp);
}

START_TEST(test_registerServer)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_discovery_registerServer_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "discovery", DEFAULT_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);

    OpcUa_RegisterServerRequest* pReq = NULL;
    status = SOPC_EncodeableObject_Create(&OpcUa_RegisterServerRequest_EncodeableType, (void**) &pReq);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_LocalizedText* serverName = SOPC_Calloc(1, sizeof(SOPC_LocalizedText));
    ck_assert_ptr_nonnull(serverName);
    SOPC_String* discoveryURL = SOPC_String_Create();
    ck_assert_ptr_nonnull(discoveryURL);

    OpcUa_RegisteredServer* pServ = &pReq->Server;

    bool fillRequest =
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->ServerUri, DEFAULT_APPLICATION_URI)) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->ProductUri, DEFAULT_PRODUCT_URI)) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->GatewayServerUri, GATEWAY_SERVER_URI)) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&pServ->SemaphoreFilePath, "")) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&serverName->defaultLocale, "Locale")) &&
        (SOPC_STATUS_OK == SOPC_String_AttachFromCstring(&serverName->defaultText, "Text")) &&
        (SOPC_STATUS_OK == SOPC_String_InitializeFromCString(discoveryURL, "opc.tcp://test"));
    ck_assert(fillRequest);
    pServ->NoOfServerNames = 1;
    pServ->ServerNames = serverName;
    pServ->NoOfDiscoveryUrls = 1;
    pServ->DiscoveryUrls = discoveryURL;
    pServ->IsOnline = true;
    pServ->ServerType = OpcUa_ApplicationType_Server;

    OpcUa_RegisterServerResponse* pResp = NULL;

    status = SOPC_ClientHelperNew_DiscoveryServiceSync(secureConnConfig, (void*) pReq, (void**) &pResp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert((pResp->ResponseHeader.ServiceResult & SOPC_GoodStatusOppositeMask) == 0);
    SOPC_EncodeableObject_Delete(pResp->encodeableType, (void**) &pResp);

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
}
END_TEST

Suite* client_suite_make_discovery(void)
{
    Suite* s = NULL;
    TCase* tc_getEndpoints = NULL;
    TCase* tc_registerServer = NULL;

    s = suite_create("Client discovery");
    tc_getEndpoints = tcase_create("GetEndpoints");
    suite_add_tcase(s, tc_getEndpoints);
    tcase_add_test(tc_getEndpoints, test_getEndpoints);

    tc_registerServer = tcase_create("RegisterServer");
    suite_add_tcase(s, tc_registerServer);
    tcase_add_test(tc_registerServer, test_registerServer);

    return s;
}
