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

#include "sopc_user_app_itf.h"

#include <assert.h>
#include <string.h>

#include "app_cb_call_context_internal.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki.h"

/** Anonymous user security policy supported configuration */
const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_Anonymous = {
    .TokenType = OpcUa_UserTokenType_Anonymous,
    .PolicyId = {9, true, (SOPC_Byte*) "anonymous"},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
};

/** Username security policy is supported and configured with security policy None.
 * With this security policy, the password will never be encrypted and this policy
 * shall not be used on unsecured or unencrypted secure channels. */
const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_NoneSecurityPolicy = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {8, true, (SOPC_Byte*) "username"},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {sizeof(SOPC_SecurityPolicy_None_URI) - 1, true, (SOPC_Byte*) SOPC_SecurityPolicy_None_URI},
    /* None security policy shall be used only when
   secure channel security policy is non-None and with encryption since password will be non-encrypted */
};

/** Username security policy is supported and configured with Secure Channel security policy.
 *  With this security policy, the password will be encrypted with secure channel security policy algorithm
 *  (None security policy will lead to unencrypted password).
 */
const OpcUa_UserTokenPolicy SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy = {
    .TokenType = OpcUa_UserTokenType_UserName,
    .PolicyId = {8, true, (SOPC_Byte*) "username"},
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
    /* Default security policy shall be used only when
   secure channel security policy is non-None since password will be non-encrypted */
};

static void SOPC_EndpointConfig_Clear(SOPC_Endpoint_Config* epConfig, bool freeCstringsFlag)
{
    if (freeCstringsFlag)
    {
        SOPC_Free(epConfig->endpointURL);
    }

    for (int i = 0; i < epConfig->nbSecuConfigs && i < SOPC_MAX_SECU_POLICIES_CFG; i++)
    {
        SOPC_String_Clear(&epConfig->secuConfigurations[i].securityPolicy);
        for (int j = 0; j < epConfig->secuConfigurations[i].nbOfUserTokenPolicies && j < SOPC_MAX_SECU_POLICIES_CFG;
             j++)
        {
            OpcUa_UserTokenPolicy_Clear(&epConfig->secuConfigurations[i].userTokenPolicies[j]);
        }
    }

    SOPC_UserAuthentication_FreeManager(&epConfig->authenticationManager);
    SOPC_UserAuthorization_FreeManager(&epConfig->authorizationManager);
}

static void SOPC_ServerConfig_Initialize(SOPC_Server_Config* config)
{
    memset(config, 0, sizeof(*config));
    OpcUa_ApplicationDescription_Initialize(&config->serverDescription);
}

static void SOPC_ClientConfig_Initialize(SOPC_Client_Config* config)
{
    memset(config, 0, sizeof(*config));
    OpcUa_ApplicationDescription_Initialize(&config->clientDescription);
}

void SOPC_S2OPC_Config_Initialize(SOPC_S2OPC_Config* config)
{
    SOPC_ServerConfig_Initialize(&config->serverConfig);
    SOPC_ClientConfig_Initialize(&config->clientConfig);
}

static void SOPC_ServerConfig_Clear(SOPC_Server_Config* config)
{
    assert(NULL != config);
    if (config->freeCstringsFlag)
    {
        for (int i = 0; NULL != config->namespaces && NULL != config->namespaces[i]; i++)
        {
            SOPC_Free(config->namespaces[i]);
        }
        SOPC_Free(config->namespaces);

        for (int i = 0; NULL != config->localeIds && NULL != config->localeIds[i]; i++)
        {
            SOPC_Free(config->localeIds[i]);
        }
        SOPC_Free(config->localeIds);

        for (int i = 0; NULL != config->trustedRootIssuersList && NULL != config->trustedRootIssuersList[i]; i++)
        {
            SOPC_Free(config->trustedRootIssuersList[i]);
        }
        SOPC_Free(config->trustedRootIssuersList);

        for (int i = 0;
             NULL != config->trustedIntermediateIssuersList && NULL != config->trustedIntermediateIssuersList[i]; i++)
        {
            SOPC_Free(config->trustedIntermediateIssuersList[i]);
        }
        SOPC_Free(config->trustedIntermediateIssuersList);

        for (int i = 0; NULL != config->issuedCertificatesList && NULL != config->issuedCertificatesList[i]; i++)
        {
            SOPC_Free(config->issuedCertificatesList[i]);
        }
        SOPC_Free(config->issuedCertificatesList);

        for (int i = 0; NULL != config->untrustedRootIssuersList && NULL != config->untrustedRootIssuersList[i]; i++)
        {
            SOPC_Free(config->untrustedRootIssuersList[i]);
        }
        SOPC_Free(config->untrustedRootIssuersList);

        for (int i = 0;
             NULL != config->untrustedIntermediateIssuersList && NULL != config->untrustedIntermediateIssuersList[i];
             i++)
        {
            SOPC_Free(config->untrustedIntermediateIssuersList[i]);
        }
        SOPC_Free(config->untrustedIntermediateIssuersList);

        for (int i = 0;
             NULL != config->certificateRevocationPathList && NULL != config->certificateRevocationPathList[i]; i++)
        {
            SOPC_Free(config->certificateRevocationPathList[i]);
        }
        SOPC_Free(config->certificateRevocationPathList);
    }

    OpcUa_ApplicationDescription_Clear(&config->serverDescription);
    if (config->freeCstringsFlag)
    {
        SOPC_Free(config->serverCertPath);
        SOPC_Free(config->serverKeyPath);
    }
    for (int i = 0; i < config->nbEndpoints; i++)
    {
        SOPC_EndpointConfig_Clear(&config->endpoints[i], config->freeCstringsFlag);
    }
    SOPC_Free(config->endpoints);

    SOPC_KeyManager_SerializedCertificate_Delete(config->serverCertificate);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(config->serverKey);
    SOPC_PKIProvider_Free(&config->pki);
    SOPC_MethodCallManager_Free(config->mcm);

    memset(config, 0, sizeof(*config));
}

static void SOPC_ClientConfig_Clear(SOPC_Client_Config* config)
{
    assert(NULL != config);
    OpcUa_ApplicationDescription_Clear(&config->clientDescription);
    if (config->freeCstringsFlag)
    {
        for (int i = 0; NULL != config->clientLocaleIds && NULL != config->clientLocaleIds[i]; i++)
        {
            SOPC_Free(config->clientLocaleIds[i]);
        }
        SOPC_Free(config->clientLocaleIds);
    }
    memset(config, 0, sizeof(*config));
}

void SOPC_S2OPC_Config_Clear(SOPC_S2OPC_Config* config)
{
    SOPC_ServerConfig_Clear(&config->serverConfig);
    SOPC_ClientConfig_Clear(&config->clientConfig);
}

const SOPC_User* SOPC_CallContext_GetUser(const SOPC_CallContext* callContextPtr)
{
    return callContextPtr->user;
}

OpcUa_MessageSecurityMode SOPC_CallContext_GetSecurityMode(const SOPC_CallContext* callContextPtr)
{
    return callContextPtr->msgSecurityMode;
}

const char* SOPC_CallContext_GetSecurityPolicy(const SOPC_CallContext* callContextPtr)
{
    return callContextPtr->secuPolicyUri;
}

uint32_t SOPC_CallContext_GetEndpointConfigIdx(const SOPC_CallContext* callContextPtr)
{
    return callContextPtr->endpointConfigIdx;
}
