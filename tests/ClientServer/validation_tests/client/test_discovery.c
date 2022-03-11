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

#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"

#include "config.h"
#include "state_machine.h"
#include "test_suite_client.h"
#include "wait_machines.h"

static StateMachine_Machine* g_pSM = NULL;
static int32_t atomicValidatingResult = 0;

/* Event handlers of the Discovery */
static void EventDispatcher_ValidateGetEndpoints(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

START_TEST(test_getEndpoints)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_discovery_getEndpoints_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert(SOPC_Toolkit_Initialize(EventDispatcher_ValidateGetEndpoints) == SOPC_STATUS_OK);
    g_pSM = StateMachine_Create();
    ck_assert(NULL != g_pSM);
    ck_assert(StateMachine_ConfigureMachine(g_pSM, false, false) == SOPC_STATUS_OK);

    ck_assert(StateMachine_StartDiscovery(g_pSM) == SOPC_STATUS_OK);
    wait_for_machine(&atomicValidatingResult, g_pSM);

    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);
}
END_TEST

void EventDispatcher_ValidateGetEndpoints(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;
    OpcUa_GetEndpointsResponse* pResp = NULL;
    OpcUa_EndpointDescription* pEndp = NULL;
    int32_t i = 0;
    bool bNoneChecked = false;
    bool bB256Checked = false;
    bool bB256S256Checked = false;
    bool bInconsistentPolicyMode = false;
    SOPC_Byte iSecLevelNone = 0;
    SOPC_Byte iSecLevelB256 = 0;
    SOPC_Byte iSecLevelB256S256 = 0;
    SOPC_ByteString* pBufCert = NULL;
    SOPC_CertificateList* pCert = NULL;

    // Set result is still validating since machine state will change on next instruction
    SOPC_Atomic_Int_Set(&atomicValidatingResult, 1);
    ck_assert(StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx));
    switch (event)
    {
    case SE_RCV_DISCOVERY_RESPONSE:
        /* Testing the response is, in fact, a test of the server */
        pResp = (OpcUa_GetEndpointsResponse*) pParam;
        /* Check the presence of the None and Basic256 sec policy (free opc ua does not support B256S256 */
        for (i = 0; i < pResp->NoOfEndpoints; ++i)
        {
            pEndp = &pResp->Endpoints[i];
            /* As we asked for a GetEndpoints on ENDPOINT_URL, it should only return endpoints with that URL */
            /* TODO: freeopcua translates the given hostname to an IP, so it is not possible to check that */
            /* ck_assert(strncmp(SOPC_String_GetRawCString(&pEndp->EndpointUrl), ENDPOINT_URL, strlen(ENDPOINT_URL)
             * + 1)
             * == 0); */
            /* Check that SecPol None <=> SecMode None */
            bInconsistentPolicyMode = false;
            bInconsistentPolicyMode =
                strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_None_URI,
                        strlen(SOPC_SecurityPolicy_None_URI) + 1) == 0;
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
                bB256Checked = true;
                iSecLevelB256 = pEndp->SecurityLevel;
                pBufCert = &pEndp->ServerCertificate;
                ck_assert(SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
                                                                         &pCert) == SOPC_STATUS_OK);
                SOPC_KeyManager_Certificate_Free(pCert);
                pCert = NULL;
                pBufCert = NULL;
            }

            if (strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_Basic256Sha256_URI,
                        strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0 &&
                pEndp->ServerCertificate.Length > 0)
            {
                bB256S256Checked = true;
                iSecLevelB256S256 = pEndp->SecurityLevel;
                pBufCert = &pEndp->ServerCertificate;
                ck_assert(SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
                                                                         &pCert) == SOPC_STATUS_OK);
                SOPC_KeyManager_Certificate_Free(pCert);
                pCert = NULL;
                pBufCert = NULL;
            }
        }

        /* Does not check that a security policy is not described multiple times */
        ck_assert(bNoneChecked && (bB256Checked || bB256S256Checked));
        if (bB256Checked)
        {
            /* Freeopcua always use 0 as SecurityLevel... */
            ck_assert(iSecLevelB256 >= iSecLevelNone);
        }
        if (bB256S256Checked)
        {
            /* Freeopcua always use 0 as SecurityLevel... */
            ck_assert(iSecLevelB256S256 >= iSecLevelNone);
        }
        break;
    default:
        /* TODO: Unhandle "not connected" error" */
        ck_assert_msg(false, "Unexpected event");
        break;
    }

    SOPC_Atomic_Int_Set(&atomicValidatingResult, 0);
}

/* Event handlers of the Discovery */
static void EventDispatcher_ValidateRegisterServer(SOPC_App_Com_Event event,
                                                   uint32_t arg,
                                                   void* pParam,
                                                   uintptr_t smCtx);

START_TEST(test_registerServer)
{
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_discovery_registerServer_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert(SOPC_Toolkit_Initialize(EventDispatcher_ValidateRegisterServer) == SOPC_STATUS_OK);
    g_pSM = StateMachine_Create();
    ck_assert(NULL != g_pSM);
    ck_assert(StateMachine_ConfigureMachine(g_pSM, false, false) == SOPC_STATUS_OK);

    ck_assert(StateMachine_StartRegisterServer(g_pSM) == SOPC_STATUS_OK);
    wait_for_machine(&atomicValidatingResult, g_pSM);

    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);
}
END_TEST

static void EventDispatcher_ValidateRegisterServer(SOPC_App_Com_Event event,
                                                   uint32_t arg,
                                                   void* pParam,
                                                   uintptr_t smCtx)
{
    uintptr_t appCtx = 0;
    OpcUa_RegisterServerResponse* pResp = NULL;

    // Set result is still validating since machine state will change on next instruction
    SOPC_Atomic_Int_Set(&atomicValidatingResult, 1);
    ck_assert(StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx));
    switch (event)
    {
    case SE_RCV_DISCOVERY_RESPONSE:
        /* Testing the response is, in fact, a test of the server */
        pResp = (OpcUa_RegisterServerResponse*) pParam;
        ck_assert((pResp->ResponseHeader.ServiceResult & SOPC_GoodStatusOppositeMask) == 0);
        break;
    default:
        /* TODO: Unhandle "not connected" error" */
        ck_assert_msg(false, "Unexpected event");
        break;
    }
    SOPC_Atomic_Int_Set(&atomicValidatingResult, 0);
}

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
