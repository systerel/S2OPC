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

#include "libs2opc_client_config_custom.h"

#include "libs2opc_client_internal.h"
#include "libs2opc_common_internal.h"
#include "sopc_assert.h"
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include <string.h>

#define SOPC_DEFAULT_REQ_LIFETIME_MS 3600000

SOPC_ReturnStatus SOPC_ClientConfigHelper_SetPreferredLocaleIds(size_t nbLocales, const char** localeIds)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == localeIds)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (NULL == pConfig->clientConfig.clientLocaleIds)
    {
        pConfig->clientConfig.clientLocaleIds = SOPC_CommonHelper_Copy_Char_Array(nbLocales, localeIds);
        if (NULL != pConfig->clientConfig.clientLocaleIds)
        {
            pConfig->clientConfig.freeCstringsFlag = true;
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientConfigHelper_SetApplicationDescription(const char* applicationUri,
                                                                    const char* productUri,
                                                                    const char* defaultAppName,
                                                                    const char* defaultAppNameLocale,
                                                                    OpcUa_ApplicationType applicationType)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == applicationUri || NULL == productUri || NULL == defaultAppName ||
        (OpcUa_ApplicationType_Client != applicationType && OpcUa_ApplicationType_ClientAndServer != applicationType))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    status = SOPC_String_CopyFromCString(&pConfig->clientConfig.clientDescription.ApplicationUri, applicationUri);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&pConfig->clientConfig.clientDescription.ProductUri, productUri);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&pConfig->clientConfig.clientDescription.ApplicationName.defaultText,
                                             defaultAppName);
    }
    if (SOPC_STATUS_OK == status && NULL != defaultAppNameLocale)
    {
        status = SOPC_String_CopyFromCString(&pConfig->clientConfig.clientDescription.ApplicationName.defaultLocale,
                                             defaultAppNameLocale);
    }
    if (SOPC_STATUS_OK == status)
    {
        pConfig->clientConfig.clientDescription.ApplicationType = applicationType;
        pConfig->clientConfig.freeCstringsFlag = true;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientConfigHelper_SetPKIprovider(SOPC_PKIProvider* pki)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == pki)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (NULL == pConfig->clientConfig.clientPKI)
    {
        pConfig->clientConfig.clientPKI = pki;
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientConfigHelper_SetKeyCertPairFromPath(const char* clientCertPath,
                                                                 const char* clientKeyPath,
                                                                 bool encrypted)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == clientCertPath || NULL == clientKeyPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (NULL == pConfig->clientConfig.clientKey &&
        (NULL == pConfig->clientConfig.configFromPaths || NULL == pConfig->clientConfig.configFromPaths->clientKeyPath))
    {
        if (NULL == pConfig->clientConfig.configFromPaths)
        {
            pConfig->clientConfig.configFromPaths = SOPC_Calloc(1, sizeof(*pConfig->clientConfig.configFromPaths));
            if (NULL == pConfig->clientConfig.configFromPaths)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        char* certPath = NULL;
        char* keyPath = NULL;
        if (SOPC_STATUS_OK == status)
        {
            certPath = SOPC_strdup(clientCertPath);
            keyPath = SOPC_strdup(clientKeyPath);
            if (NULL == certPath || NULL == keyPath)
            {
                SOPC_Free(certPath);
                SOPC_Free(keyPath);
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            pConfig->clientConfig.configFromPaths->clientCertPath = certPath;
            pConfig->clientConfig.configFromPaths->clientKeyPath = keyPath;
            pConfig->clientConfig.configFromPaths->clientKeyEncrypted = encrypted;
            pConfig->clientConfig.isConfigFromPathsNeeded = true;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientConfigHelper_SetKeyCertPairFromBytes(size_t certificateNbBytes,
                                                                  const unsigned char* clientCertificate,
                                                                  size_t keyNbBytes,
                                                                  const unsigned char* clientPrivateKey)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == clientCertificate || 0 == certificateNbBytes || NULL == clientPrivateKey || 0 == keyNbBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (NULL == pConfig->clientConfig.clientKey)
    {
        SOPC_SerializedCertificate* cert = NULL;
        SOPC_SerializedAsymmetricKey* key = NULL;

        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(clientPrivateKey, (uint32_t) keyNbBytes, &key);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(clientCertificate,
                                                                         (uint32_t) certificateNbBytes, &cert);
        }

        if (SOPC_STATUS_OK == status)
        {
            pConfig->clientConfig.clientKey = key;
            pConfig->clientConfig.clientCertificate = cert;
        }
        else
        {
            SOPC_KeyManager_SerializedAsymmetricKey_Delete(key);
            SOPC_KeyManager_SerializedCertificate_Delete(cert);
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_SecureConnection_Config* SOPC_ClientConfigHelper_CreateSecureConnection(const char* userDefinedId,
                                                                             const char* endpointUrl,
                                                                             OpcUa_MessageSecurityMode secuMode,
                                                                             SOPC_SecurityPolicy_URI secuPolicy)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return NULL;
    }
    if (NULL == userDefinedId || NULL == endpointUrl || OpcUa_MessageSecurityMode_Invalid >= secuMode ||
        secuMode >= OpcUa_MessageSecurityMode_SizeOf || SOPC_SecurityPolicy_None > secuPolicy ||
        secuPolicy > SOPC_SecurityPolicy_Aes256Sha256RsaPss ||
        (OpcUa_MessageSecurityMode_None != secuMode && SOPC_SecurityPolicy_None == secuPolicy) ||
        (OpcUa_MessageSecurityMode_None == secuMode && SOPC_SecurityPolicy_None != secuPolicy))
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();
    SOPC_SecureConnection_Config* secConnConfig = NULL;

    if (pConfig->clientConfig.nbSecureConnections < SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG)
    {
        secConnConfig = SOPC_Calloc(1, sizeof(*secConnConfig));
        if (NULL != secConnConfig)
        {
            if (SOPC_SecurityPolicy_None != secuPolicy && NULL == pConfig->clientConfig.clientKey &&
                (!pConfig->clientConfig.isConfigFromPathsNeeded ||
                 NULL == pConfig->clientConfig.configFromPaths->clientKeyPath))
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "Attempt to create a secure connection (!= None mode) without client key/certificate configured.");
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Call SOPC_ClientConfigHelper_SetKeyCertPair* function prior to "
                                       "SOPC_ClientConfigHelper_CreateSecureConnection.");
                status = SOPC_STATUS_INVALID_STATE;
            }
            if (SOPC_STATUS_OK == status)
            {
                secConnConfig->scConfig.requestedLifetime = SOPC_DEFAULT_REQ_LIFETIME_MS;
                // Empty user policy id per default (anonymous without policy)
                secConnConfig->sessionConfig.userPolicyId = SOPC_strdup("");
                status =
                    (NULL != secConnConfig->sessionConfig.userPolicyId ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
            }
            if (SOPC_STATUS_OK == status)
            {
                char* secuPolicyUri = SOPC_strdup(SOPC_SecurityPolicyUriToCstring(secuPolicy));
                char* userDefinedIdCopy = SOPC_strdup(userDefinedId);
                char* endpointUrlCopy = SOPC_strdup(endpointUrl);
                if (NULL != secuPolicyUri && NULL != userDefinedIdCopy && NULL != endpointUrlCopy)
                {
                    secConnConfig->scConfig.reqSecuPolicyUri = secuPolicyUri;
                    secConnConfig->userDefinedId = userDefinedIdCopy;
                    secConnConfig->scConfig.url = endpointUrlCopy;
                    secConnConfig->scConfig.msgSecurityMode = secuMode;
                    secConnConfig->scConfig.isClientSc = true;
                    secConnConfig->scConfig.clientConfigPtr = &pConfig->clientConfig;
                    secConnConfig->secureConnectionIdx = pConfig->clientConfig.nbSecureConnections;
                    pConfig->clientConfig.secureConnections[pConfig->clientConfig.nbSecureConnections] = secConnConfig;
                    pConfig->clientConfig.nbSecureConnections++;
                }
                else
                {
                    SOPC_Free(secuPolicyUri);
                    SOPC_Free(userDefinedIdCopy);
                    SOPC_Free(endpointUrlCopy);
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Free(secConnConfig);
            secConnConfig = NULL;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Too many secure connection configured (>= SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG).");
        status = SOPC_STATUS_NOT_SUPPORTED;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return secConnConfig;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetExpectedEndpointsDescription(
    SOPC_SecureConnection_Config* secConnConfig,
    const OpcUa_GetEndpointsResponse* getEndpointsResponse)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == getEndpointsResponse)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!secConnConfig->finalized && NULL == secConnConfig->scConfig.expectedEndpoints)
    {
        OpcUa_GetEndpointsResponse* respCopy = NULL;
        status = SOPC_Encodeable_Create(&OpcUa_GetEndpointsResponse_EncodeableType, (void**) &respCopy);
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_EncodeableObject_Copy(&OpcUa_GetEndpointsResponse_EncodeableType, respCopy, getEndpointsResponse);
        }
        if (SOPC_STATUS_OK == status)
        {
            secConnConfig->scConfig.expectedEndpoints = respCopy;
        }
        else
        {
            SOPC_ReturnStatus deleteStatus =
                SOPC_Encodeable_Delete(&OpcUa_GetEndpointsResponse_EncodeableType, (void**) &respCopy);
            SOPC_UNUSED_RESULT(deleteStatus);
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetReverseConnection(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* clientReverseEndpointUrl)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == clientReverseEndpointUrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (!secConnConfig->finalized && NULL == secConnConfig->reverseURL)
    {
        char* rEpURLcopy = SOPC_strdup(clientReverseEndpointUrl);
        status = (rEpURLcopy != NULL ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
        if (SOPC_STATUS_OK == status)
        {
            // Find if this reverse EP already exists in client application
            bool found = false;
            for (uint16_t i = 0; !found && i < pConfig->clientConfig.nbReverseEndpointURLs; i++)
            {
                found = (0 == strcmp(clientReverseEndpointUrl, pConfig->clientConfig.reverseEndpointURLs[i]));
            }
            // Add new reverse EP to client application if it not already defined
            if (!found)
            {
                // Note: it is not be possible to create more secure connections and thus more reverse EPs through API
                SOPC_ASSERT(pConfig->clientConfig.nbReverseEndpointURLs < SOPC_MAX_CLIENT_SECURE_CONNECTIONS_CONFIG);
                char* rEpURLcopyBis = SOPC_strdup(clientReverseEndpointUrl);
                if (NULL != rEpURLcopyBis)
                {
                    pConfig->clientConfig.reverseEndpointURLs[pConfig->clientConfig.nbReverseEndpointURLs] =
                        rEpURLcopyBis;
                    pConfig->clientConfig.nbReverseEndpointURLs++;
                }
                else
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                    SOPC_Free(rEpURLcopy);
                    rEpURLcopy = NULL;
                }
            }
        }
        // Sets the reverse endpoint URL in the secure connection config
        if (SOPC_STATUS_OK == status)
        {
            secConnConfig->reverseURL = rEpURLcopy;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetReqLifetime(SOPC_SecureConnection_Config* secConnConfig,
                                                             uint32_t reqLifetime)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!secConnConfig->finalized)
    {
        secConnConfig->scConfig.requestedLifetime = reqLifetime;
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetServerCertificateFromPath(SOPC_SecureConnection_Config* secConnConfig,
                                                                           const char* serverCertPath)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == serverCertPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!secConnConfig->finalized && NULL == secConnConfig->scConfig.peerAppCert &&
        NULL == secConnConfig->serverCertPath)
    {
        secConnConfig->serverCertPath = SOPC_strdup(serverCertPath);
        if (NULL != secConnConfig->serverCertPath)
        {
            secConnConfig->isServerCertFromPathNeeded = true;
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetServerCertificateFromBytes(SOPC_SecureConnection_Config* secConnConfig,
                                                                            size_t certificateNbBytes,
                                                                            const unsigned char* serverCertificate)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == serverCertificate || 0 == certificateNbBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!secConnConfig->finalized && NULL == secConnConfig->scConfig.peerAppCert &&
        !secConnConfig->isServerCertFromPathNeeded)
    {
        SOPC_SerializedCertificate* srvCert = NULL;
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(serverCertificate, (uint32_t) certificateNbBytes,
                                                                     &srvCert);

        if (SOPC_STATUS_OK == status)
        {
            secConnConfig->scConfig.peerAppCert = srvCert;
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetAnonymous(SOPC_SecureConnection_Config* secConnConfig,
                                                           const char* userPolicyId)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == userPolicyId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (OpcUa_UserTokenType_Anonymous != secConnConfig->sessionConfig.userTokenType ||
        (NULL != secConnConfig->sessionConfig.userPolicyId &&
         0 != SOPC_strncmp_ignore_case("", secConnConfig->sessionConfig.userPolicyId, 1)))
    {
        // User token already defined
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        char* userPolicyIdCopy = SOPC_strdup(userPolicyId);
        if (NULL == secConnConfig->sessionConfig.userPolicyId)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Free((char*) secConnConfig->sessionConfig.userPolicyId);
            SOPC_GCC_DIAGNOSTIC_RESTORE
            secConnConfig->sessionConfig.userPolicyId = userPolicyIdCopy;
        }
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetUserName(SOPC_SecureConnection_Config* secConnConfig,
                                                          const char* userPolicyId,
                                                          const char* userName,
                                                          const char* password)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == userPolicyId || (NULL == userName && NULL != password) ||
        (NULL != userName && NULL == password))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (OpcUa_UserTokenType_Anonymous != secConnConfig->sessionConfig.userTokenType)
    {
        // User token already defined
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        char* userPolicyIdCopy = SOPC_strdup(userPolicyId);
        char* userNameCopy = SOPC_strdup(userName);
        char* passwordCopy = SOPC_strdup(password);
        if (NULL == userPolicyIdCopy || (NULL == userNameCopy && NULL != userName) ||
            (NULL == passwordCopy && NULL != password))
        {
            SOPC_Free(userPolicyIdCopy);
            SOPC_Free(userNameCopy);
            SOPC_Free(passwordCopy);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Free((char*) secConnConfig->sessionConfig.userPolicyId);
            SOPC_GCC_DIAGNOSTIC_RESTORE
            secConnConfig->sessionConfig.userPolicyId = userPolicyIdCopy;
            secConnConfig->sessionConfig.userToken.userName.userName = userNameCopy;
            secConnConfig->sessionConfig.userToken.userName.userPwd = passwordCopy;
            secConnConfig->sessionConfig.userTokenType = OpcUa_UserTokenType_UserName;
        }
    }
    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetUserX509FromPaths(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* userPolicyId,
                                                                   const char* userCertPath,
                                                                   const char* userKeyPath,
                                                                   bool encrypted)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == secConnConfig || NULL == userCertPath || NULL == userKeyPath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (OpcUa_UserTokenType_Anonymous != secConnConfig->sessionConfig.userTokenType)
    {
        // User token already defined
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        secConnConfig->sessionConfig.userToken.userX509.configFromPaths =
            SOPC_Calloc(1, sizeof(*secConnConfig->sessionConfig.userToken.userX509.configFromPaths));
        char* userPolicyIdCopy = SOPC_strdup(userPolicyId);
        char* userCertPathCopy = SOPC_strdup(userCertPath);
        char* userKeyPathCopy = SOPC_strdup(userKeyPath);
        if (NULL == secConnConfig->sessionConfig.userToken.userX509.configFromPaths || NULL == userPolicyIdCopy ||
            NULL == userCertPathCopy || NULL == userKeyPathCopy)
        {
            SOPC_Free(secConnConfig->sessionConfig.userToken.userX509.configFromPaths);
            secConnConfig->sessionConfig.userToken.userX509.configFromPaths = NULL;
            SOPC_Free(userPolicyIdCopy);
            SOPC_Free(userCertPathCopy);
            SOPC_Free(userKeyPathCopy);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Free((char*) secConnConfig->sessionConfig.userPolicyId);
            SOPC_GCC_DIAGNOSTIC_RESTORE
            secConnConfig->sessionConfig.userPolicyId = userPolicyIdCopy;
            secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userCertPath = userCertPathCopy;
            secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyPath = userKeyPathCopy;
            secConnConfig->sessionConfig.userToken.userX509.configFromPaths->userKeyEncrypted = encrypted;
            secConnConfig->sessionConfig.userToken.userX509.isConfigFromPathNeeded = true;
            secConnConfig->sessionConfig.userTokenType = OpcUa_UserTokenType_Certificate;
        }
    }
    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}

SOPC_ReturnStatus SOPC_SecureConnectionConfig_SetUserX509FromBytes(SOPC_SecureConnection_Config* secConnConfig,
                                                                   const char* userPolicyId,
                                                                   size_t certificateNbBytes,
                                                                   const unsigned char* userCertificate,
                                                                   size_t keyNbBytes,
                                                                   const unsigned char* userPrivateKey)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == userCertificate || 0 == certificateNbBytes || NULL == userPrivateKey || 0 == keyNbBytes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (OpcUa_UserTokenType_Anonymous != secConnConfig->sessionConfig.userTokenType)
    {
        // User token already defined
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SerializedCertificate* cert = NULL;
        SOPC_SerializedAsymmetricKey* key = NULL;
        char* userPolicyIdCopy = SOPC_strdup(userPolicyId);

        if (NULL == userPolicyIdCopy)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status =
                SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(userPrivateKey, (uint32_t) keyNbBytes, &key);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(userCertificate, (uint32_t) certificateNbBytes,
                                                                         &cert);
        }

        if (SOPC_STATUS_OK == status)
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Free((char*) secConnConfig->sessionConfig.userPolicyId);
            SOPC_GCC_DIAGNOSTIC_RESTORE
            secConnConfig->sessionConfig.userPolicyId = userPolicyIdCopy;
            secConnConfig->sessionConfig.userToken.userX509.certX509 = cert;
            secConnConfig->sessionConfig.userToken.userX509.keyX509 = key;
            secConnConfig->sessionConfig.userTokenType = OpcUa_UserTokenType_Certificate;
        }
        else
        {
            SOPC_Free(userPolicyIdCopy);
            SOPC_KeyManager_SerializedAsymmetricKey_Delete(key);
            SOPC_KeyManager_SerializedCertificate_Delete(cert);
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientConfigHelper_GetSecureConnectionConfigs(size_t* nbScConfigs,
                                                                     SOPC_SecureConnection_Config*** scConfigArray)
{
    if (!SOPC_ClientInternal_IsInitialized())
    {
        // Client wrapper not initialized
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == nbScConfigs || NULL == scConfigArray)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus mutStatus = Mutex_Lock(&sopc_client_helper_config.configMutex);

    SOPC_S2OPC_Config* pConfig = SOPC_CommonHelper_GetConfiguration();

    if (0 == pConfig->clientConfig.nbSecureConnections)
    {
        *nbScConfigs = 0;
        *scConfigArray = NULL;
    }
    else
    {
        *nbScConfigs = pConfig->clientConfig.nbSecureConnections;
        *scConfigArray = pConfig->clientConfig.secureConnections;
    }

    mutStatus = Mutex_Unlock(&sopc_client_helper_config.configMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return status;
}
