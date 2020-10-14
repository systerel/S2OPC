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

#include "libs2opc_server_config_custom.h"
#include "libs2opc_server_config_internal.h"

#include "sopc_builtintypes.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config.h"

// Utility function to create a NULL terminated string array from content copied of an array with known size
static char** copy_char_array_into_new_NULL_terminated_array(size_t nbElts, char** src)
{
    // Array length + NULL terminator
    char** sArray = SOPC_Calloc(nbElts + 1, sizeof(char*));
    if (NULL == sArray)
    {
        return NULL;
    }
    bool ok = true;
    for (size_t i = 0; ok && i < nbElts; i++)
    {
        sArray[i] = SOPC_strdup(src[i]);
        ok &= NULL != sArray[i];
    }
    if (!ok)
    {
        for (size_t i = 0; i < nbElts; i++)
        {
            SOPC_Free(sArray[i]);
        }
        SOPC_Free(sArray);
        sArray = NULL;
    }

    return sArray;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetNamespaces(size_t nbNamespaces, char** namespaces)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.config.serverConfig.namespaces)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (0 == nbNamespaces || NULL == namespaces)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    sopc_helper_config.config.serverConfig.namespaces =
        copy_char_array_into_new_NULL_terminated_array(nbNamespaces, namespaces);

    if (NULL == sopc_helper_config.config.serverConfig.namespaces)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetLocaleIds(size_t nbLocales, char** localeIds)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.config.serverConfig.localeIds)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (nbLocales > 0 && NULL == localeIds)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_helper_config.config.serverConfig.localeIds =
        copy_char_array_into_new_NULL_terminated_array(nbLocales, localeIds);

    if (NULL == sopc_helper_config.config.serverConfig.localeIds)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

// Add an application name into server description with given locale or no locale
static SOPC_ReturnStatus SOPC_HelperInternal_AddApplicationNameLocale_NoCheck(const char* appName,
                                                                              const char* appNameLocale)
{
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
        if (NULL == sopc_helper_config.config.serverConfig.localeIds)
        {
            // Ensure we consider at least no locales are defined instead of invalid parameters
            locales = emptyLocales;
        }
        else
        {
            locales = sopc_helper_config.config.serverConfig.localeIds;
        }
        status = SOPC_LocalizedText_AddOrSetLocale(
            &sopc_helper_config.config.serverConfig.serverDescription.ApplicationName, locales, &addAppName);
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
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == applicationUri || NULL == productUri || NULL == defaultAppName || 0 == strlen(defaultAppName))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    OpcUa_ApplicationDescription* appDesc = &sopc_helper_config.config.serverConfig.serverDescription;
    appDesc->ApplicationType = applicationType;

    SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&appDesc->ApplicationUri, applicationUri);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&appDesc->ProductUri, productUri);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperInternal_AddApplicationNameLocale_NoCheck(defaultAppName, defaultAppNameLocale);
    }

    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_AddApplicationNameLocale(const char* additionalAppName,
                                                                   const char* additionalAppNameLocale)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == additionalAppName || 0 == strlen(additionalAppName) || NULL == additionalAppNameLocale ||
        0 == strlen(additionalAppNameLocale))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_HelperInternal_AddApplicationNameLocale_NoCheck(additionalAppName, additionalAppNameLocale);
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetPKIprovider(SOPC_PKIProvider* pki)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.config.serverConfig.pki)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pki)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_helper_config.config.serverConfig.pki = pki;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetCertificateFromPath(const char* serverCertPath, const char* serverKeyPath)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.config.serverConfig.serverCertificate ||
        NULL != sopc_helper_config.config.serverConfig.serverKey)
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

    sopc_helper_config.config.serverConfig.serverCertificate = serverCert;
    sopc_helper_config.config.serverConfig.serverKey = serverKey;

    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetCertificateFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* serverCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* serverPrivateKey)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.config.serverConfig.serverCertificate ||
        NULL != sopc_helper_config.config.serverConfig.serverKey)
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

    sopc_helper_config.config.serverConfig.serverCertificate = serverCert;
    sopc_helper_config.config.serverConfig.serverKey = serverKey;

    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_AddEndpoint(SOPC_Endpoint_Config* endpointConfig)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == endpointConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_Endpoint_Config* epArray = sopc_helper_config.config.serverConfig.endpoints;
    uint8_t nbElts = sopc_helper_config.config.serverConfig.nbEndpoints;

    for (uint8_t i = 0; i < nbElts; i++)
    {
        SOPC_Endpoint_Config* ep = &epArray[i];
        int res = SOPC_strcmp_ignore_case(ep->endpointURL, endpointConfig->endpointURL);
        if (0 == res)
        {
            // Endpoint URL already set
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Error adding an endpoint with an already configured endpoint URL %s",
                                   ep->endpointURL);
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (UINT8_MAX == nbElts)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Use of realloc for each AddEndpoint, possible to optimize or limit to a maximum pre-allocated
    SOPC_Endpoint_Config* newEpArray = SOPC_Realloc(epArray, (size_t) nbElts * sizeof(SOPC_Endpoint_Config),
                                                    ((size_t) nbElts + 1) * sizeof(SOPC_Endpoint_Config));
    if (NULL == newEpArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    newEpArray[nbElts] = *endpointConfig;

    sopc_helper_config.config.serverConfig.endpoints = newEpArray;
    sopc_helper_config.config.serverConfig.nbEndpoints = (uint8_t)(nbElts + 1);

    return SOPC_STATUS_OK;
}

SOPC_Endpoint_Config SOPC_EndpointConfig_Create(const char* url, bool hasDiscovery)
{
    assert(NULL != url);
    SOPC_Endpoint_Config newEp;
    memset(&newEp, 0, sizeof(newEp));
    newEp.endpointURL = SOPC_strdup(url);
    assert(NULL != newEp.endpointURL);
    newEp.hasDiscoveryEndpoint = hasDiscovery;
    newEp.serverConfigPtr = &sopc_helper_config.config.serverConfig;
    return newEp;
}

SOPC_SecurityPolicy* SOPC_EndpointConfig_AddSecurityPolicy(SOPC_Endpoint_Config* destEndpoint,
                                                           SOPC_SecurityPolicy_URI uri)
{
    if (NULL == destEndpoint || destEndpoint->nbSecuConfigs == SOPC_MAX_SECU_POLICIES_CFG)
    {
        return NULL;
    }
    SOPC_SecurityPolicy* sp = &destEndpoint->secuConfigurations[destEndpoint->nbSecuConfigs];
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
    default:
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&sp->securityPolicy, sUri);
    assert(SOPC_STATUS_OK == status);
    destEndpoint->nbSecuConfigs++;
    return sp;
}

SOPC_ReturnStatus SOPC_SecurityPolicy_SetSecurityModes(SOPC_SecurityPolicy* destSecuPolicy, SOPC_SecurityModeMask modes)
{
    if (NULL == destSecuPolicy ||
        0 == (modes & (SOPC_SecurityModeMask_None | SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt)))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    destSecuPolicy->securityModes = (uint16_t) modes;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_SecurityPolicy_AddUserTokenPolicy(SOPC_SecurityPolicy* destSecuPolicy,
                                                         const SOPC_UserTokenPolicy* userTokenPolicy)
{
    if (NULL == destSecuPolicy)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (destSecuPolicy->nbOfUserTokenPolicies == SOPC_MAX_SECU_POLICIES_CFG)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    // Log a warning when used with security mode none + user name without encryption
    if (0 != (destSecuPolicy->securityModes & SOPC_SecurityModeMask_None) &&
        OpcUa_UserTokenType_UserName == userTokenPolicy->TokenType &&
        0 == strcmp(SOPC_SecurityPolicy_None_URI, SOPC_String_GetRawCString(&userTokenPolicy->SecurityPolicyUri)))
    {
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Adding UserName token policy with security mode None for secure channel and user token. "
            "It is strongly discouraged (password will be transfered without encryption)");
    }

    OpcUa_UserTokenPolicy* utp = &destSecuPolicy->userTokenPolicies[destSecuPolicy->nbOfUserTokenPolicies];
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

    destSecuPolicy->nbOfUserTokenPolicies++;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetAddressSpace(SOPC_AddressSpace* addressSpaceConfig)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
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
        sopc_helper_config.server.addressSpace = addressSpaceConfig;
    }
    return status;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserAuthenticationManager(
    SOPC_UserAuthentication_Manager* authenticationMgr)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.server.authenticationManager)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == authenticationMgr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    sopc_helper_config.server.authenticationManager = authenticationMgr;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetUserAuthorizationManager(SOPC_UserAuthorization_Manager* authorizationMgr)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock() || NULL != sopc_helper_config.server.authorizationManager)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == authorizationMgr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    sopc_helper_config.server.authorizationManager = authorizationMgr;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HelperConfigServer_SetSwManufacturer(const char* manufacturerName)
{
    if (!SOPC_HelperConfig_IsInitAndUnlock())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == manufacturerName)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    sopc_helper_config.server.manufacturerName = manufacturerName;

    return SOPC_STATUS_OK;
}
