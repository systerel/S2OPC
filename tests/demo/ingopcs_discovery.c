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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_crypto_provider.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#include "config.h"
#include "ingopcs_browse.h"
#include "state_machine.h"

/* The state machine which handles async events.
 * It is shared between the main thread and the Toolkit event thread.
 * It should be protected by a Mutex.
 */
StateMachine_Machine* g_pSM = NULL;

/* Event handler of the Discovery */
void EventDispatcher_Discovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx);

void PrintEndpoints(OpcUa_GetEndpointsResponse* pResp);

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t iWait = 0;

    printf("INGOPCS discovery demo.\n");
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
        status = StateMachine_ConfigureMachine(g_pSM);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Configured();
        if (SOPC_STATUS_OK == status)
        {
            printf("# Info: Toolkit configuration done.\n");
            printf("# Info: Sending GetEndpoints.\n");
        }
        else
        {
            printf("# Error: Toolkit configuration failed.\n");
        }
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

    return status;
}

void EventDispatcher_Discovery(SOPC_App_Com_Event event, uint32_t arg, void* pParam, uintptr_t smCtx)
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

void PrintEndpoints(OpcUa_GetEndpointsResponse* pResp)
{
    int32_t i = 0;
    int32_t j = 0;
    uint32_t k = 0;
    OpcUa_EndpointDescription* pEndp = NULL;
    OpcUa_UserTokenPolicy* pPol = NULL;
    SOPC_ByteString* pBufCert = NULL;
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_Certificate* pCert = NULL;
    uint32_t lenThmb = 0;
    char* pThmb = NULL;

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
                if (SOPC_KeyManager_Certificate_CreateFromDER(pBufCert->Data, pBufCert->Length, &pCert) !=
                    SOPC_STATUS_OK)
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
                        pThmb = calloc(lenThmb + 1, sizeof(char));
                        if (NULL == pThmb || SOPC_KeyManager_Certificate_GetThumbprint(
                                                 pProvider, pCert, (uint8_t*) pThmb, lenThmb) != SOPC_STATUS_OK)
                        {
                            printf("<failed to compute certificate thumbprint>\n");
                        }
                        else
                        {
                            for (k = 0; k < lenThmb; ++k)
                            {
                                printf("%02" SCNx8, pThmb[k]);
                                if (k < lenThmb - 1)
                                {
                                    printf(":");
                                }
                            }
                            printf("\n");
                            free(pThmb);
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
                break;
            case OpcUa_UserTokenType_Kerberos:
                printf("    OpcUa_UserTokenType_Kerberos\n");
                break;
            default:
                printf("    Invalid/Unrecognized\n");
                break;
            }
        }
    }
}
