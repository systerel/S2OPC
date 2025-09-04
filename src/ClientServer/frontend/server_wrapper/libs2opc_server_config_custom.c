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

#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_common_internal.h"
#include "libs2opc_server_config_custom.h"
#include "libs2opc_server_internal.h"

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_toolkit_config.h"

#include "opcua_identifiers.h"

static const SOPC_NodeId namespaceArray_nid = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server_NamespaceArray);
static const SOPC_String opcua_uri = SOPC_STRING("http://opcfoundation.org/UA/");

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetLocaleIds(size_t nbLocales, const char** localeIds)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
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

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetApplicationDescription(const char* applicationUri,
                                                                    const char* productUri,
                                                                    const char* defaultAppName,
                                                                    const char* defaultAppNameLocale,
                                                                    OpcUa_ApplicationType applicationType)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
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

SOPC_ReturnStatus SOPC_ServerConfigHelper_AddApplicationNameLocale(const char* additionalAppName,
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
    SOPC_ASSERT(NULL != pConfig);

    return SOPC_Internal_AddApplicationNameLocale(&pConfig->serverConfig.serverDescription,
                                                  pConfig->serverConfig.localeIds, additionalAppName,
                                                  additionalAppNameLocale);
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetPKIprovider(SOPC_PKIProvider* pki)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.pki)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pki)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status =
        SOPC_PKIProvider_SetUpdateCb(pki, &SOPC_ServerInternal_PKIProviderUpdateCb, (uintptr_t) NULL);
    if (SOPC_STATUS_OK == status)
    {
        pConfig->serverConfig.pki = pki;
    }
    return status;
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetKeyCertPairFromPath(const char* serverCertPath,
                                                                 const char* serverKeyPath,
                                                                 bool encrypted)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.serverKeyCertPair)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == serverCertPath || NULL == serverKeyPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_KeyCertPair* serverKeyCertPair = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Retrieve key password if needed */
    char* password = NULL;

    if (encrypted)
    {
        bool res = SOPC_ServerInternal_GetKeyPassword(&password);
        status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyCertPair_CreateFromPaths(serverCertPath, serverKeyPath, password, &serverKeyCertPair);
    }

    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_KeyCertPair_SetUpdateCb(serverKeyCertPair, &SOPC_ServerInternal_KeyCertPairUpdateCb, (uintptr_t) NULL);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }

    if (NULL != password)
    {
        SOPC_Free(password);
    }

    if (SOPC_STATUS_OK == status)
    {
        pConfig->serverConfig.serverKeyCertPair = serverKeyCertPair;
    }

    return status;
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetKeyCertPairFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* serverCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* serverPrivateKey)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.serverKeyCertPair)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (0 == certificateNbBytes || certificateNbBytes > UINT32_MAX || NULL == serverCertificate || 0 == keyNbBytes ||
        keyNbBytes > UINT32_MAX || NULL == serverPrivateKey)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_KeyCertPair* serverKeyCertPair = NULL;
    SOPC_ReturnStatus status = SOPC_KeyCertPair_CreateFromBytes(certificateNbBytes, serverCertificate, keyNbBytes,
                                                                serverPrivateKey, &serverKeyCertPair);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Failed to load server key and certificate from bytes arrays.");
    }
    else
    {
        pConfig->serverConfig.serverKeyCertPair = serverKeyCertPair;

        status =
            SOPC_KeyCertPair_SetUpdateCb(serverKeyCertPair, &SOPC_ServerInternal_KeyCertPairUpdateCb, (uintptr_t) NULL);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }

    return status;
}

SOPC_Endpoint_Config* SOPC_ServerConfigHelper_CreateEndpoint(const char* url, bool hasDiscovery)
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
    SOPC_ASSERT(NULL != pConfig);

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

    const char* sUri = SOPC_SecurityPolicyUriToCstring(uri);
    if (NULL == sUri)
    {
        return NULL;
    }
    if (destEndpoint->nbSecuConfigs > 0)
    {
        for (size_t i = 0; i < (size_t) destEndpoint->nbSecuConfigs; i++)
        {
            const char* spCString = SOPC_String_GetRawCString(&destEndpoint->secuConfigurations[i].securityPolicy);

            if (NULL != spCString && strcmp(spCString, sUri) == 0)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "SOPC_EndpointConfig_AddSecurityConfig: %s security policy URI has already been added", sUri);
                return NULL;
            }
        }
    }

    SOPC_SecurityConfig* sp = &destEndpoint->secuConfigurations[destEndpoint->nbSecuConfigs];
    SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&sp->securityPolicy, sUri);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    SOPC_ASSERT(SOPC_STATUS_OK == status);
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
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Copy(&OpcUa_UserTokenPolicy_EncodeableType, utp, userTokenPolicy);
    if (SOPC_STATUS_OK == status)
    {
        destSecuConfig->nbOfUserTokenPolicies++;
    }
    return status;
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

// Check that NamespaceArray array has at least 2 values (+ type), and precisely:
// - NamespaceArray[0] = OPC UA ("http://opcfoundation.org/UA/")
// - NamespaceArray[1] = ProductURI (= ApplicationURI)
static SOPC_ReturnStatus check_opcua_values_nsArray(SOPC_Variant* nsArray_value)
{
    SOPC_ASSERT(NULL != nsArray_value);

    SOPC_S2OPC_Config* config = SOPC_CommonHelper_GetConfiguration();
    SOPC_ReturnStatus status = (NULL != config) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        // Check: needs at least 2 values.
        status = (2 <= nsArray_value->Value.Array.Length) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

        if (SOPC_STATUS_OK == status)
        {
            // Check: values must be String.
            status = (SOPC_String_Id == nsArray_value->BuiltInTypeId) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

            if (SOPC_STATUS_OK == status)
            {
                int32_t comparison = -1;
                // First value
                const SOPC_String* value0 = &nsArray_value->Value.Array.Content.StringArr[0];
                status = SOPC_String_Compare(&opcua_uri, value0, false, &comparison);
                if (SOPC_STATUS_OK != status || 0 != comparison)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "First value of node 'NamespaceArray': %s is not equal to OPC UA "
                                           "URI ('http://opcfoundation.org/UA/'). "
                                           "Server will stop.",
                                           SOPC_String_GetRawCString(value0));
                    status = SOPC_STATUS_NOK;
                }
                else
                {
                    // Second value
                    const SOPC_String* value1 = &nsArray_value->Value.Array.Content.StringArr[1];
                    status = SOPC_String_Compare(&config->serverConfig.serverDescription.ProductUri, value1, false,
                                                 &comparison);
                    if (SOPC_STATUS_OK != status || 0 != comparison)
                    {
                        SOPC_Logger_TraceError(
                            SOPC_LOG_MODULE_CLIENTSERVER,
                            "Second value of node 'NamespaceArray': %s is not equal to ProductURI (which is "
                            "also ApplicationURI): %s. Server will stop.",
                            SOPC_String_GetRawCString(value1),
                            SOPC_String_GetRawCString(&config->serverConfig.serverDescription.ProductUri));
                        status = SOPC_STATUS_NOK;
                    }
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Node 'NamespaceArray' has invalid value DataType. Must be String. "
                                       "Server will stop.");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Node 'NamespaceArray' has invalid number of values. Needs at least 2."
                                   "Server will stop.");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Unable to retrieve server config. Make sure to have initialized the toolkit."
                               "Server will stop.");
    }

    return status;
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetAddressSpace(SOPC_AddressSpace* addressSpaceConfig)
{
    if (!SOPC_ServerInternal_IsConfiguring())
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == addressSpaceConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
#if S2OPC_EVENT_MANAGEMENT
    SOPC_S2OPC_Config* pEventConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pEventConfig);
    SOPC_EventManager_CreateEventTypes(addressSpaceConfig, &pEventConfig->serverConfig.eventTypes);
#endif
    // Find number of namespaces: it is the size of the array value NamespaceArray
    uint16_t nbOfNS = 0;
    bool foundNode = false;
    SOPC_AddressSpace_Node* namespaceArray =
        SOPC_AddressSpace_Get_Node(addressSpaceConfig, &namespaceArray_nid, &foundNode);
    if (foundNode)
    {
        // Initialize Max numeric id for each namespace
        SOPC_Variant* value = SOPC_AddressSpace_Get_Value(addressSpaceConfig, namespaceArray);
        if (NULL != value && SOPC_VariantArrayType_Array == value->ArrayType)
        {
            nbOfNS = (uint16_t) value->Value.Array.Length;
            status = SOPC_AddressSpace_MaxNsNumId_Initialize(addressSpaceConfig, nbOfNS);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Inconsistency in the number of NS between: value of node NamespaceArray (nb NS = %d) and "
                    "address space (nodes NS index).",
                    nbOfNS);
            }
            else
            {
                // Number of NS in NamespaceArray = nbr of NS in address space.
                // Now check that the two first values are the good ones (OPC UA).
                status = check_opcua_values_nsArray(value);
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Value of mandatory node 'NamespaceArray' is not well defined. "
                                   "Server will stop.");
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Mandatory node 'NamespaceArray' missing in the address space. "
                               "Server will stop.");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(addressSpaceConfig);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Keep address space instance reference for deallocation
        sopc_server_helper_config.addressSpace = addressSpaceConfig;
    }
    return status;
}

SOPC_AddressSpace* SOPC_ServerConfigHelper_GetAddressSpace(void)
{
    if (!SOPC_ServerInternal_IsConfiguring() || NULL == sopc_server_helper_config.addressSpace)
    {
        return NULL;
    }
    return sopc_server_helper_config.addressSpace;
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetUserAuthenticationManager(
    SOPC_UserAuthentication_Manager* authenticationMgr)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.authenticationManager)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == authenticationMgr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pConfig->serverConfig.authenticationManager = authenticationMgr;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetUserAuthorizationManager(SOPC_UserAuthorization_Manager* authorizationMgr)
{
    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_ASSERT(NULL != pConfig);
    if (!SOPC_ServerInternal_IsConfiguring() || NULL != pConfig->serverConfig.authorizationManager)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == authorizationMgr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pConfig->serverConfig.authorizationManager = authorizationMgr;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ServerConfigHelper_SetSoftwareBuildInfo(OpcUa_BuildInfo* buildInfo)
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
        SOPC_EncodeableObject_Create(&OpcUa_BuildInfo_EncodeableType, (void**) &sopc_server_helper_config.buildInfo);

    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_EncodeableObject_Copy(&OpcUa_BuildInfo_EncodeableType, sopc_server_helper_config.buildInfo, buildInfo);
    }

    if (SOPC_STATUS_OK != status)
    {
        OpcUa_BuildInfo_Clear(sopc_server_helper_config.buildInfo);
        SOPC_Free(sopc_server_helper_config.buildInfo);
        sopc_server_helper_config.buildInfo = NULL;
    }

    return status;
}
