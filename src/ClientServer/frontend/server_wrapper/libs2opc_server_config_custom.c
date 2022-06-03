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
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"
#include "libs2opc_server_config_custom.h"
#include "libs2opc_server_internal.h"

#include "sopc_builtintypes.h"
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config.h"

SOPC_ReturnStatus SOPC_HelperConfigServer_SetNamespaces(size_t nbNamespaces, char** namespaces)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.namespaces)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (0 == nbNamespaces || NULL == namespaces)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pConfig->serverConfig.namespaces = SOPC_CommonHelper_Copy_Char_Array(nbNamespaces, namespaces);

    if (NULL == pConfig->serverConfig.namespaces)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetLocaleIds(size_t nbLocales, char** localeIds)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.localeIds)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (nbLocales > 0 && NULL == localeIds)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    pConfig->serverConfig.localeIds = SOPC_CommonHelper_Copy_Char_Array(nbLocales, localeIds);

    if (NULL == pConfig->serverConfig.localeIds)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

/**
 * \brief Define an application name into application description with given locale or no locale.
 *        The locales supported by the application shall be provided and application name is checked to be compliant.
 *
 *  \param appDesc        The Server application description
 *  \param appLocaleIds   The Server application Locales supported (checked regarding application locales)
 *  \param appName        The Server application name to define
 *  \param appNameLocale  (optional) The locale of the Server application name provided
 *
 * \return SOPC_STATUS_OK in case of success
 */
static SOPC_ReturnStatus SOPC_Internal_AddApplicationNameLocale(OpcUa_ApplicationDescription* appDesc,
                                                                char** appLocaleIds,
                                                                const char* appName,
                                                                const char* appNameLocale)
{
    if (NULL == appDesc || NULL == appName)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_LocalizedText addAppName;
    SOPC_LocalizedText_Initialize(&addAppName);
    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&addAppName.defaultText, appName);
    if (SOPC_STATUS_OK == status && NULL != appNameLocale)
    {
        status = SOPC_String_CopyFromCString(&addAppName.defaultLocale, appNameLocale);
    }
    if (SOPC_STATUS_OK == status)
    {
        char* emptyLocales[1] = {NULL};
        char** locales = NULL;
        if (NULL == appLocaleIds)
        {
            // Ensure we consider at least no locales are defined instead of invalid parameters
            locales = emptyLocales;
        }
        else
        {
            locales = appLocaleIds;
        }
        status = SOPC_LocalizedText_AddOrSetLocale(&appDesc->ApplicationName, locales, &addAppName);
    }
    SOPC_LocalizedText_Clear(&addAppName);

    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetApplicationDescription(const char* applicationUri,
                                                                    const char* productUri,
                                                                    const char* defaultAppName,
                                                                    const char* defaultAppNameLocale,
                                                                    OpcUa_ApplicationType applicationType)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || pConfig->serverConfig.serverDescription.ApplicationUri.Length > 0 ||
        pConfig->serverConfig.serverDescription.ProductUri.Length > 0 ||
        pConfig->serverConfig.serverDescription.ApplicationName.defaultText.Length > 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == applicationUri || NULL == productUri || NULL == defaultAppName || 0 == strlen(defaultAppName))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_ApplicationDescription* appDesc = &pConfig->serverConfig.serverDescription;
    appDesc->ApplicationType = applicationType;

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&appDesc->ApplicationUri, applicationUri);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&appDesc->ProductUri, productUri);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Internal_AddApplicationNameLocale(&pConfig->serverConfig.serverDescription,
                                                        pConfig->serverConfig.localeIds, defaultAppName,
                                                        defaultAppNameLocale);
    }

    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_AddApplicationNameLocale(const char* additionalAppName,
                                                                   const char* additionalAppNameLocale)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == additionalAppName || 0 == strlen(additionalAppName) || NULL == additionalAppNameLocale ||
        0 == strlen(additionalAppNameLocale))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);

    return SOPC_Internal_AddApplicationNameLocale(&pConfig->serverConfig.serverDescription,
                                                  pConfig->serverConfig.localeIds, additionalAppName,
                                                  additionalAppNameLocale);
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetPKIprovider(SOPC_PKIProvider* pki)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.pki)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pki)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    pConfig->serverConfig.pki = pki;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetKeyCertPairFromPath(const char* serverCertPath, const char* serverKeyPath)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.serverCertificate ||
        NULL != pConfig->serverConfig.serverKey)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == serverCertPath || NULL == serverKeyPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SerializedCertificate* serverCert = NULL;
    SOPC_SerializedAsymmetricKey* serverKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(serverCertPath, &serverCert);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(serverKeyPath, &serverKey);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to load server key from path %s\n",
                                   serverKeyPath);
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to load server certificate from path %s\n",
                               serverCertPath);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(serverCert);
        serverCert = NULL;
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(serverKey);
        serverKey = NULL;
    }

    pConfig->serverConfig.serverCertificate = serverCert;
    pConfig->serverConfig.serverKey = serverKey;

    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetKeyCertPairFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* serverCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* serverPrivateKey)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.serverCertificate ||
        NULL != pConfig->serverConfig.serverKey)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (0 == certificateNbBytes || certificateNbBytes > UINT32_MAX || NULL == serverCertificate || 0 == keyNbBytes ||
        keyNbBytes > UINT32_MAX || NULL == serverPrivateKey)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SerializedCertificate* serverCert = NULL;
    SOPC_SerializedAsymmetricKey* serverKey = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(
        serverCertificate, (uint32_t) certificateNbBytes, &serverCert);
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(serverPrivateKey, (uint32_t) keyNbBytes, &serverKey);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to load server key from bytes array\n");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to load server certificate from bytes array\n");
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(serverCert);
        serverCert = NULL;
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(serverKey);
        serverKey = NULL;
    }

    pConfig->serverConfig.serverCertificate = serverCert;
    pConfig->serverConfig.serverKey = serverKey;

    return status;
}

SOPC_Endpoint_Config* SOPC_HelperConfigServer_CreateEndpoint(const char* url, bool hasDiscovery)
{
    if (NULL == url || !SOPC_ServerInternal_IsConfiguring() ||
        sopc_server_helper_config.nbEndpoints >= SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
    {
        return NULL;
    }
    for (uint8_t i = 0; i < sopc_server_helper_config.nbEndpoints; i++)
    {
        SOPC_Endpoint_Config* ep = sopc_server_helper_config.endpoints[i];
        int res = SOPC_strcmp_ignore_case(ep->endpointURL, url);
        if (0 == res)
        {
            // Endpoint URL already set
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Error adding an endpoint with an already configured endpoint URL %s",
                                   ep->endpointURL);
            return NULL;
        }
    }
    SOPC_Endpoint_Config* newEp = SOPC_Calloc(1, sizeof(SOPC_Endpoint_Config));
    if (NULL == newEp)
    {
        return NULL;
    }

    newEp->endpointURL = SOPC_strdup(url);
    if (NULL == newEp->endpointURL)
    {
        SOPC_Free(newEp);
        return NULL;
    }

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    assert(NULL != pConfig);

    newEp->hasDiscoveryEndpoint = hasDiscovery;
    newEp->serverConfigPtr = &pConfig->serverConfig;
    sopc_server_helper_config.endpoints[sopc_server_helper_config.nbEndpoints] = newEp;
    sopc_server_helper_config.nbEndpoints++;

    return newEp;
}

SOPC_SecurityConfig* SOPC_EndpointConfig_AddSecurityConfig(SOPC_Endpoint_Config* destEndpoint,
                                                           SOPC_SecurityPolicy_URI uri)
{
    if (NULL == destEndpoint || destEndpoint->nbSecuConfigs == SOPC_MAX_SECU_POLICIES_CFG)
    {
        return NULL;
    }
    SOPC_SecurityConfig* sp = &destEndpoint->secuConfigurations[destEndpoint->nbSecuConfigs];
    char* sUri = NULL;
    switch (uri)
    {
    case SOPC_SecurityPolicy_None:
        sUri = SOPC_SecurityPolicy_None_URI;
        break;
    case SOPC_SecurityPolicy_Basic256:
        sUri = SOPC_SecurityPolicy_Basic256_URI;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256:
        sUri = SOPC_SecurityPolicy_Basic256Sha256_URI;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep:
        sUri = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI;
        break;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss:
        sUri = SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI;
        break;
    default:
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&sp->securityPolicy, sUri);
    assert(SOPC_STATUS_OK == status);
    destEndpoint->nbSecuConfigs++;
    return sp;
}

SOPC_ReturnStatus SOPC_SecurityConfig_SetSecurityModes(SOPC_SecurityConfig* destSecuConfig, SOPC_SecurityModeMask modes)
{
    if (NULL == destSecuConfig ||
        0 == (modes & (SOPC_SecurityModeMask_None | SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt)))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    destSecuConfig->securityModes = (uint16_t) modes;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_SecurityConfig_AddUserTokenPolicy(SOPC_SecurityConfig* destSecuConfig,
                                                         const SOPC_UserTokenPolicy* userTokenPolicy)
{
    if (NULL == destSecuConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (destSecuConfig->nbOfUserTokenPolicies == SOPC_MAX_SECU_POLICIES_CFG)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    // Log a warning when used with security mode none + user name without encryption
    if (0 != (destSecuConfig->securityModes & SOPC_SecurityModeMask_None) &&
        OpcUa_UserTokenType_UserName == userTokenPolicy->TokenType &&
        0 == strcmp(SOPC_SecurityPolicy_None_URI, SOPC_String_GetRawCString(&userTokenPolicy->SecurityPolicyUri)))
    {
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Adding UserName token policy with security mode None for secure channel and user token. "
            "It is strongly discouraged (password will be transfered without encryption)");
    }

    OpcUa_UserTokenPolicy* utp = &destSecuConfig->userTokenPolicies[destSecuConfig->nbOfUserTokenPolicies];
    OpcUa_UserTokenPolicy_Initialize(utp);
    SOPC_ReturnStatus status = SOPC_String_Copy(&utp->IssuedTokenType, &userTokenPolicy->IssuedTokenType);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    status = SOPC_String_Copy(&utp->IssuerEndpointUrl, &userTokenPolicy->IssuerEndpointUrl);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    status = SOPC_String_Copy(&utp->PolicyId, &userTokenPolicy->PolicyId);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    status = SOPC_String_Copy(&utp->SecurityPolicyUri, &userTokenPolicy->SecurityPolicyUri);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    utp->TokenType = userTokenPolicy->TokenType;

    destSecuConfig->nbOfUserTokenPolicies++;
    return SOPC_STATUS_OK;
}

bool SOPC_EndpointConfig_AddClientToConnect(SOPC_Endpoint_Config* destEndpoint,
                                            const char* clientAppURI,
                                            const char* clientEndpointURL)
{
    // Note: clientAppURI ignored for now since no verification is implemented
    if (NULL == destEndpoint || NULL == clientEndpointURL || !SOPC_ServerInternal_IsConfiguring() ||
        destEndpoint->nbClientsToConnect >= SOPC_MAX_REVERSE_CLIENT_CONNECTIONS)
    {
        return false;
    }
    SOPC_Server_ClientToConnect* clientToConnect = &destEndpoint->clientsToConnect[destEndpoint->nbClientsToConnect];
    clientToConnect->clientApplicationURI = SOPC_strdup(clientAppURI);
    clientToConnect->clientEndpointURL = SOPC_strdup(clientEndpointURL);
    if (NULL == clientToConnect->clientEndpointURL)
    {
        SOPC_Free(clientToConnect->clientApplicationURI);
        clientToConnect->clientApplicationURI = NULL;
        return false;
    }

    destEndpoint->nbClientsToConnect++;
    return true;
}

bool SOPC_EndpointConfig_StopListening(SOPC_Endpoint_Config* destEndpoint)
{
    if (NULL == destEndpoint || !SOPC_ServerInternal_IsConfiguring() || destEndpoint->nbClientsToConnect <= 0)
    {
        return false;
    }
    destEndpoint->noListening = true;
    return true;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetAddressSpace(SOPC_AddressSpace* addressSpaceConfig)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == addressSpaceConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_ToolkitServer_SetAddressSpaceConfig(addressSpaceConfig);
    if (SOPC_STATUS_OK == status)
    {
        // Keep address space instance reference for deallocation
        sopc_server_helper_config.addressSpace = addressSpaceConfig;
    }
    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserAuthenticationManager(
    SOPC_UserAuthentication_Manager* authenticationMgr)
{
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != sopc_server_helper_config.authenticationManager)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == authenticationMgr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    sopc_server_helper_config.authenticationManager = authenticationMgr;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserAuthorizationManager(SOPC_UserAuthorization_Manager* authorizationMgr)
{
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != sopc_server_helper_config.authorizationManager)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == authorizationMgr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    sopc_server_helper_config.authorizationManager = authorizationMgr;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetSoftwareBuildInfo(OpcUa_BuildInfo* buildInfo)
{
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != sopc_server_helper_config.buildInfo)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == buildInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status =
        SOPC_Encodeable_Create(&OpcUa_BuildInfo_EncodeableType, (void**) &sopc_server_helper_config.buildInfo);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&sopc_server_helper_config.buildInfo->ProductUri, &buildInfo->ProductUri);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&sopc_server_helper_config.buildInfo->ManufacturerName, &buildInfo->ManufacturerName);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&sopc_server_helper_config.buildInfo->ProductName, &buildInfo->ProductName);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&sopc_server_helper_config.buildInfo->SoftwareVersion, &buildInfo->SoftwareVersion);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&sopc_server_helper_config.buildInfo->BuildNumber, &buildInfo->BuildNumber);
    }

    sopc_server_helper_config.buildInfo->BuildDate = buildInfo->BuildDate;

    if (SOPC_STATUS_OK != status)
    {
        OpcUa_BuildInfo_Clear(sopc_server_helper_config.buildInfo);
        SOPC_Free(sopc_server_helper_config.buildInfo);
        sopc_server_helper_config.buildInfo = NULL;
    }

    return status;
}
