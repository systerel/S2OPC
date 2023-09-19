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

#include <string.h>

#include "app_cb_call_context_internal.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

static void SOPC_EndpointConfig_Clear(SOPC_Endpoint_Config* epConfig, bool freeCstringsFlag)
{
    if (freeCstringsFlag)
    {
        SOPC_Free(epConfig->endpointURL);

        for (int i = 0; i < epConfig->nbClientsToConnect && i < SOPC_MAX_REVERSE_CLIENT_CONNECTIONS; i++)
        {
            SOPC_Free(epConfig->clientsToConnect[i].clientApplicationURI);
            SOPC_Free(epConfig->clientsToConnect[i].clientEndpointURL);
        }
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

void SOPC_ServerConfig_Initialize(SOPC_Server_Config* config)
{
    memset(config, 0, sizeof(*config));
    OpcUa_ApplicationDescription_Initialize(&config->serverDescription);
}

void SOPC_ClientConfig_Initialize(SOPC_Client_Config* config)
{
    memset(config, 0, sizeof(*config));
    OpcUa_ApplicationDescription_Initialize(&config->clientDescription);
    config->clientDescription.ApplicationType = OpcUa_ApplicationType_Client;
}

void SOPC_S2OPC_Config_Initialize(SOPC_S2OPC_Config* config)
{
    SOPC_ServerConfig_Initialize(&config->serverConfig);
    SOPC_ClientConfig_Initialize(&config->clientConfig);
}

void SOPC_ServerConfig_Clear(SOPC_Server_Config* config)
{
    SOPC_ASSERT(NULL != config);
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
    }

    OpcUa_ApplicationDescription_Clear(&config->serverDescription);
    if (config->freeCstringsFlag)
    {
        SOPC_Free(config->serverCertPath);
        SOPC_Free(config->serverKeyPath);
        SOPC_Free(config->serverPkiPath);
    }
    for (int i = 0; i < config->nbEndpoints; i++)
    {
        SOPC_EndpointConfig_Clear(&config->endpoints[i], config->freeCstringsFlag);
    }
    SOPC_Free(config->endpoints);

    SOPC_KeyCertPair_Delete(&config->serverKeyCertPair);
    SOPC_PKIProvider_Free(&config->pki);
    SOPC_MethodCallManager_Free(config->mcm);

    memset(config, 0, sizeof(*config));
}

static void SOPC_SecureChannelConfig_Clear(SOPC_SecureChannel_Config* scConfig)
{
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    OpcUa_GetEndpointsResponse_Clear((void*) scConfig->expectedEndpoints);
    SOPC_Free((void*) scConfig->expectedEndpoints);
    scConfig->expectedEndpoints = NULL;
    SOPC_Free((void*) scConfig->serverUri);
    scConfig->serverUri = NULL;
    SOPC_Free((void*) scConfig->url);
    SOPC_Free((void*) scConfig->reqSecuPolicyUri);
    scConfig->reqSecuPolicyUri = NULL;
    scConfig->url = NULL;

    SOPC_KeyManager_SerializedCertificate_Delete((SOPC_SerializedCertificate*) scConfig->peerAppCert);
    scConfig->peerAppCert = NULL;
    SOPC_GCC_DIAGNOSTIC_RESTORE
    scConfig->clientConfigPtr = NULL;

    return;
}

void SOPC_ClientConfig_Clear(SOPC_Client_Config* config)
{
    SOPC_ASSERT(NULL != config);
    OpcUa_ApplicationDescription_Clear(&config->clientDescription);
    if (config->freeCstringsFlag)
    {
        for (int i = 0; NULL != config->clientLocaleIds && NULL != config->clientLocaleIds[i]; i++)
        {
            SOPC_Free(config->clientLocaleIds[i]);
        }
        SOPC_Free(config->clientLocaleIds);

        if (config->freeCstringsFlag && NULL != config->configFromPaths)
        {
            SOPC_Client_ConfigFromPaths* pathsConfig = config->configFromPaths;

            SOPC_Free(pathsConfig->clientCertPath);
            SOPC_Free(pathsConfig->clientKeyPath);
            SOPC_Free(pathsConfig->clientPkiPath);

            SOPC_Free(config->configFromPaths);
            config->configFromPaths = NULL;
        }
    }

    SOPC_KeyCertPair_Delete(&config->clientKeyCertPair);
    config->clientKeyCertPair = NULL;
    SOPC_PKIProvider_Free(&config->clientPKI);
    config->clientPKI = NULL;

    for (uint16_t i = 0; i < config->nbSecureConnections; i++)
    {
        SOPC_SecureConnection_Config* secConnConfig = config->secureConnections[i];
        SOPC_ASSERT(NULL != secConnConfig);
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_Free((void*) secConnConfig->userDefinedId);
        SOPC_Free((void*) secConnConfig->reverseURL);
        SOPC_Free((void*) secConnConfig->sessionConfig.userPolicyId);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        SOPC_SecureChannelConfig_Clear(&secConnConfig->scConfig);
        SOPC_Free(secConnConfig->serverCertPath);
        if (config->freeCstringsFlag && OpcUa_UserTokenType_UserName == secConnConfig->sessionConfig.userTokenType)
        {
            SOPC_Free(secConnConfig->sessionConfig.userToken.userName.userName);
            SOPC_Free(secConnConfig->sessionConfig.userToken.userName.userPwd);
        }
        else if (OpcUa_UserTokenType_Certificate == secConnConfig->sessionConfig.userTokenType)
        {
            if (secConnConfig->sessionConfig.userToken.userX509.isConfigFromPathNeeded &&
                NULL != secConnConfig->sessionConfig.userToken.userX509.configFromPaths)
            {
                if (config->freeCstringsFlag)
                {
                    SOPC_Free(secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userCertPath);
                    SOPC_Free(secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyPath);
                }
                SOPC_Free(secConnConfig->sessionConfig.userToken.userX509.configFromPaths);
                secConnConfig->sessionConfig.userToken.userX509.configFromPaths = NULL;
            }
            SOPC_KeyManager_SerializedCertificate_Delete(secConnConfig->sessionConfig.userToken.userX509.certX509);
            SOPC_KeyManager_SerializedAsymmetricKey_Delete(secConnConfig->sessionConfig.userToken.userX509.keyX509);
        }

        SOPC_Free(secConnConfig);
        config->secureConnections[i] = NULL;
    }
    for (uint16_t i = 0; i < config->nbReverseEndpointURLs; i++)
    {
        SOPC_Free(config->reverseEndpointURLs[i]);
        config->reverseEndpointURLs[i] = NULL;
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

SOPC_AddressSpaceAccess* SOPC_CallContext_GetAddressSpaceAccess(const SOPC_CallContext* callContextPtr)
{
    return callContextPtr->addressSpaceForMethodCall;
}
