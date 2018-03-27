/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <check.h>
//#include <inttypes.h>
#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <stdarg.h>

//#include "sopc_builtintypes.h"
//#include "sopc_crypto_provider.h"
#include "sopc_time.h"
//#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
//#include "sopc_types.h"

#include "config.h"
#include "state_machine.h"

StateMachine_Machine* g_pSM = NULL;

/* Global helper */
void wait_for_machine(int count, ...)
{
    va_list args;
    int i = 0;
    StateMachine_Machine* pSM = NULL;
    uint32_t iWait = 0;

    va_start(args, count);
    for (i = 0; i < count; ++i)
    {
        pSM = va_arg(args, StateMachine_Machine*);
        while (NULL != pSM && !StateMachine_IsIdle(g_pSM) && iWait * SLEEP_LENGTH <= SC_LIFETIME)
        {
            iWait += 1;
            SOPC_Sleep(SLEEP_LENGTH);
        }
    }
}

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
    wait_for_machine(1, g_pSM);

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
                        strlen(SOPC_SecurityPolicy_Basic256_URI) + 1) == 0)
            {
                bB256Checked = true;
                iSecLevelB256 = pEndp->SecurityLevel;
                pBufCert = &pEndp->ServerCertificate;
                ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(pBufCert->Data, pBufCert->Length, &pCert) ==
                          SOPC_STATUS_OK);
                SOPC_KeyManager_Certificate_Free(pCert);
                pCert = NULL;
                pBufCert = NULL;
            }

            if (strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_Basic256Sha256_URI,
                        strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0)
            {
                bB256S256Checked = true;
                iSecLevelB256S256 = pEndp->SecurityLevel;
                pBufCert = &pEndp->ServerCertificate;
                ck_assert(SOPC_KeyManager_Certificate_CreateFromDER(pBufCert->Data, pBufCert->Length, &pCert) ==
                          SOPC_STATUS_OK);
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
