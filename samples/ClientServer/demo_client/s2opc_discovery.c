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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_crypto_provider.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
static StateMachine_Machine* g_pSM = NULL;

/* Event handler of the Discovery */
static void EventDispatcher_Discovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

static void PrintEndpoints(OpcUa_GetEndpointsResponse* pResp);

static const char* const usage[] = {
    "s2opc_discovery [options]",
    NULL,
};

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    struct argparse_option options[] = {OPT_HELP(),      CONN_OPTIONS[0], CONN_OPTIONS[1], CONN_OPTIONS[2],
                                        CONN_OPTIONS[3], CONN_OPTIONS[4], CONN_OPTIONS[5], CONN_OPTIONS[6],
                                        CONN_OPTIONS[7], CONN_OPTIONS[8], CONN_OPTIONS[9], CONN_OPTIONS[10],
                                        OPT_END()};
    struct argparse argparse;

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, "\nS2OPC discovery demo: get endpoints from a server", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    printf("S2OPC discovery demo.\n");
    /* Initialize SOPC_Common */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_discovery_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_Common_Initialize(logConfiguration);
    }
    /* Init */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(EventDispatcher_Discovery);
        g_pSM = StateMachine_Create();
    }
    if (SOPC_STATUS_OK == status && NULL == g_pSM)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Configuration, which include Secure Channel configuration. */
    if (SOPC_STATUS_OK == status)
    {
        status = StateMachine_ConfigureMachine(g_pSM, !NONE, ENCRYPT);
    }

    /* Starts Discovery */
    if (SOPC_STATUS_OK == status)
    {
        status = StateMachine_StartDiscovery(g_pSM);
    }

    /* Active wait */
    while (SOPC_STATUS_OK == status && !StateMachine_IsIdle(g_pSM) && iWait * SLEEP_LENGTH <= SC_LIFETIME)
    {
        iWait += 1;
        SOPC_Sleep(SLEEP_LENGTH);
    }

    /* Finish it */
    SOPC_Toolkit_Clear();
    StateMachine_Delete(&g_pSM);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}

static void EventDispatcher_Discovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
{
    uintptr_t appCtx = 0;

    if (StateMachine_EventDispatcher(g_pSM, &appCtx, event, arg, pParam, smCtx))
    {
        switch (event)
        {
        case SE_RCV_DISCOVERY_RESPONSE:
            PrintEndpoints((OpcUa_GetEndpointsResponse*) pParam);
            break;
        default:
            break;
        }
    }
    else
    {
        printf("# Error: Received event %i not processed by the machine.\n", event);
        g_pSM->state = stError;
    }
}

static void PrintEndpoints(OpcUa_GetEndpointsResponse* pResp)
{
    int32_t i = 0;
    int32_t j = 0;
    uint32_t k = 0;
    OpcUa_EndpointDescription* pEndp = NULL;
    OpcUa_UserTokenPolicy* pPol = NULL;
    SOPC_ByteString* pBufCert = NULL;
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_CertificateList* pCert = NULL;
    uint32_t lenThmb = 0;
    char* pThmb = NULL;
    int nbCharThmb = 0;

    if (SOPC_GoodGenericStatus != pResp->ResponseHeader.ServiceResult)
    {
        printf("# Error: GetEndpoints failed with status code %i.\n", pResp->ResponseHeader.ServiceResult);
    }
    else
    {
        printf("%" PRIi32 " endpoints:\n", pResp->NoOfEndpoints);
    }

    for (i = 0; i < pResp->NoOfEndpoints; ++i)
    {
        pEndp = &pResp->Endpoints[i];
        printf("- Endpoint %s\n", SOPC_String_GetRawCString(&pEndp->EndpointUrl));
        printf("  Security policy and mode: %s ", SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri));
        switch (pEndp->SecurityMode)
        {
        default:
        case OpcUa_MessageSecurityMode_Invalid:
            printf("OpcUa_MessageSecurityMode_Invalid");
            break;
        case OpcUa_MessageSecurityMode_None:
            printf("OpcUa_MessageSecurityMode_None");
            break;
        case OpcUa_MessageSecurityMode_Sign:
            printf("OpcUa_MessageSecurityMode_Sign");
            break;
        case OpcUa_MessageSecurityMode_SignAndEncrypt:
            printf("OpcUa_MessageSecurityMode_SignAndEncrypt");
            break;
        }
        printf("\n  ApplicationDescription:\n");
        printf("    ApplicationUri: %s\n    ProductUri: %s\n", SOPC_String_GetRawCString(&pEndp->Server.ApplicationUri),
               SOPC_String_GetRawCString(&pEndp->Server.ProductUri));

        printf("  CertificateThumbprint: ");
        pBufCert = &pEndp->ServerCertificate;
        if (pBufCert->Length <= 0)
        {
            printf("<null>\n");
        }
        else
        {
            pProvider = SOPC_CryptoProvider_Create(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri));
            if (NULL == pProvider)
            {
                printf("<error creating crypto provider>\n");
            }
            else
            {
                if (SOPC_KeyManager_Certificate_CreateOrAddFromDER(pBufCert->Data, (uint32_t) pBufCert->Length,
                                                                   &pCert) != SOPC_STATUS_OK)
                {
                    printf("<error creating certificate>\n");
                }
                else
                {
                    if (SOPC_CryptoProvider_CertificateGetLength_Thumbprint(pProvider, &lenThmb) != SOPC_STATUS_OK)
                    {
                        printf("<error calculating certificate thumbprint length>\n");
                    }
                    else
                    {
                        pThmb = SOPC_Calloc(lenThmb + 1, sizeof(char));
                        if (NULL == pThmb || SOPC_KeyManager_Certificate_GetThumbprint(
                                                 pProvider, pCert, (uint8_t*) pThmb, lenThmb) != SOPC_STATUS_OK)
                        {
                            printf("<failed to compute certificate thumbprint>\n");
                        }
                        else
                        {
                            for (k = 0; k < lenThmb; ++k)
                            {
                                nbCharThmb = printf("%02" SCNx8, pThmb[k]);
                                assert(2 == nbCharThmb);

                                if (k < lenThmb - 1)
                                {
                                    printf(":");
                                }
                            }
                            printf("\n");
                            SOPC_Free(pThmb);
                            pThmb = NULL;
                        }
                    }
                    SOPC_KeyManager_Certificate_Free(pCert);
                    pCert = NULL;
                }
                SOPC_CryptoProvider_Free(pProvider);
                pProvider = NULL;
            }
        }

        printf("  UserIdentityTokens:\n");
        for (j = 0; j < pEndp->NoOfUserIdentityTokens; ++j)
        {
            pPol = &pEndp->UserIdentityTokens[j];
            printf("  - PolicyId %s\n", SOPC_String_GetRawCString(&pPol->PolicyId));
            switch (pPol->TokenType)
            {
            case OpcUa_UserTokenType_Anonymous:
                printf("    OpcUa_UserTokenType_Anonymous\n");
                break;
            case OpcUa_UserTokenType_UserName:
                printf("    OpcUa_UserTokenType_UserName\n");
                break;
            case OpcUa_UserTokenType_Certificate:
                printf("    OpcUa_UserTokenType_Certificate\n");
                break;
            case OpcUa_UserTokenType_IssuedToken:
                printf("    OpcUa_UserTokenType_IssuedToken\n");
                printf("    IssuedTokenType %s\n", SOPC_String_GetRawCString(&pPol->IssuedTokenType));
                printf("    IssuerEndpointUrl %s\n", SOPC_String_GetRawCString(&pPol->IssuerEndpointUrl));
                break;
            case OpcUa_UserTokenType_Kerberos:
                printf("    OpcUa_UserTokenType_Kerberos\n");
                break;
            default:
                printf("    Invalid/Unrecognized\n");
                break;
            }
            printf("    SecurityPolicyUri ");
            if (pPol->SecurityPolicyUri.Length <= 0)
            {
                printf("<empty, endpoint policy is %s>\n", SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri));
                if (strncmp(SOPC_String_GetRawCString(&pEndp->SecurityPolicyUri), SOPC_SecurityPolicy_None_URI,
                            strlen(SOPC_SecurityPolicy_None_URI)) == 0)
                {
                    printf("      WARNING: secrets in user identity token will not be encrypted\n");
                }
            }
            else
            {
                printf("%s", SOPC_String_GetRawCString(&pPol->SecurityPolicyUri));
                if (strncmp(SOPC_String_GetRawCString(&pPol->SecurityPolicyUri), SOPC_SecurityPolicy_None_URI,
                            strlen(SOPC_SecurityPolicy_None_URI)) == 0)
                {
                    printf("      WARNING: secrets in user identity token will not be encrypted\n");
                }
            }
        }
    }
}
