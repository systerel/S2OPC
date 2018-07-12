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

#include "sopc_time.h"
#include "sopc_toolkit_config.h"

#include "config.h"
#include "state_machine.h"
#include "wait_machines.h"

static StateMachine_Machine* g_pSM = NULL;

/* Event handlers of the Discovery */
static void EventDispatcher_ValidateDiscovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

START_TEST(test_discovery)
{
    ck_assert(SOPC_Toolkit_Initialize(EventDispatcher_ValidateDiscovery) == SOPC_STATUS_OK);
    g_pSM = StateMachine_Create();
    ck_assert(NULL != g_pSM);
    ck_assert(StateMachine_ConfigureMachine(g_pSM) == SOPC_STATUS_OK);
    ck_assert(SOPC_Toolkit_Configured() == SOPC_STATUS_OK);

    ck_assert(StateMachine_StartDiscovery(g_pSM) == SOPC_STATUS_OK);
    wait_for_machines(1, g_pSM);

    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);
}
END_TEST

void EventDispatcher_ValidateDiscovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
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
    SOPC_Certificate* pCert = NULL;

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
            /* ck_assert(strncmp(SOPC_String_GetRawCString(&pEndp->EndpointUrl), ENDPOINT_URL, strlen(ENDPOINT_URL) + 1)
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
                ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
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
                ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
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
}

Suite* client_suite_make_discovery(void)
{
    Suite* s = NULL;
    TCase* tc_discovery = NULL;

    s = suite_create("Client discovery");
    tc_discovery = tcase_create("Without session");

    suite_add_tcase(s, tc_discovery);
    tcase_add_test(tc_discovery, test_discovery);

    return s;
}
