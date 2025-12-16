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

#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "util_discovery_services.h"

SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
const SOPC_String tcpUaTransportProfileURI = {
    .Length = 65,
    .DoNotClear = true,
    .Data = (SOPC_Byte*) "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary",
};

const OpcUa_UserTokenPolicy anonymousUserTokenPolicy = {
    .PolicyId = SOPC_STRING("anonymous"),
    .TokenType = OpcUa_UserTokenType_Anonymous,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING_NULL,
};

const OpcUa_UserTokenPolicy userNameUserTokenPolicy = {
    .PolicyId = SOPC_STRING("username"),
    .TokenType = OpcUa_UserTokenType_UserName,
    .IssuedTokenType = SOPC_STRING_NULL,
    .IssuerEndpointUrl = SOPC_STRING_NULL,
    .SecurityPolicyUri = SOPC_STRING_NULL,
};
SOPC_GCC_DIAGNOSTIC_RESTORE

static void SOPC_SetServerCertificate(SOPC_SerializedCertificate* serverCertificate, SOPC_ByteString* serverCert)
{
    SOPC_ASSERT(NULL != serverCertificate);
    uint32_t tmpLength = 0;
    SOPC_ReturnStatus status =
        SOPC_ByteString_CopyFromBytes(serverCert, serverCertificate->data, (int32_t) serverCertificate->length);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_ASSERT(tmpLength <= INT32_MAX);
    serverCert->Length = (int32_t) serverCertificate->length;
}

static void SOPC_SetServerApplicationDescription(SOPC_Endpoint_Config* sopcEndpointConfig,
                                                 char** preferredLocales,
                                                 OpcUa_ApplicationDescription* appDesc)
{
    int32_t idx = 0;
    OpcUa_ApplicationDescription* desc = &sopcEndpointConfig->serverConfigPtr->serverDescription;
    SOPC_ASSERT(desc->ApplicationType != OpcUa_ApplicationType_Client);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (preferredLocales != NULL)
    {
        status =
            SOPC_LocalizedText_GetPreferredLocale(&appDesc->ApplicationName, preferredLocales, &desc->ApplicationName);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set Application in application description of response");
        }
    }

    appDesc->ApplicationType = desc->ApplicationType;

    status = SOPC_String_AttachFrom(&appDesc->ApplicationUri, &desc->ApplicationUri);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set Application URI in application description of response");
    }

    if (desc->DiscoveryProfileUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&appDesc->DiscoveryProfileUri, &desc->DiscoveryProfileUri);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set DiscoveryProfileURI in application description of response");
        }
    }

    if (desc->NoOfDiscoveryUrls > 0)
    {
        appDesc->DiscoveryUrls = SOPC_Calloc((size_t) desc->NoOfDiscoveryUrls, sizeof(SOPC_String));
    }
    else
    {
        appDesc->DiscoveryUrls = SOPC_Calloc(1, sizeof(SOPC_String));
    }
    if (appDesc->DiscoveryUrls != NULL)
    {
        if (desc->NoOfDiscoveryUrls > 0)
        {
            appDesc->NoOfDiscoveryUrls = desc->NoOfDiscoveryUrls;
            status = SOPC_STATUS_OK;
            // Copy the discovery URLs provided by configuration
            for (idx = 0; SOPC_STATUS_OK == status && idx < desc->NoOfDiscoveryUrls; idx++)
            {
                status = SOPC_String_AttachFrom(&appDesc->DiscoveryUrls[idx], &desc->DiscoveryUrls[idx]);
            }
        }
        else
        {
            appDesc->NoOfDiscoveryUrls = 1;
            // If not provided by configuration: we set the endpoint URL as default discovery URL since it is mandatory
            status = SOPC_String_AttachFromCstring(&appDesc->DiscoveryUrls[0], sopcEndpointConfig->endpointURL);
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set DiscoveryUrl(s) in application description of response");
        }
    }

    if (desc->GatewayServerUri.Length > 0)
    {
        status = SOPC_String_AttachFrom(&appDesc->GatewayServerUri, &desc->GatewayServerUri);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set GatewayServerURI in application description of response");
        }
    }

    status = SOPC_String_AttachFrom(&appDesc->ProductUri, &desc->ProductUri);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set Product URI in application description of response");
    }
}

static void copyUserIdTokens(SOPC_SecurityPolicy* currentSecurityPolicy,
                             OpcUa_EndpointDescription* newEndPointDescription)
{
    // Set userIdentityTokens
    newEndPointDescription->UserIdentityTokens =
        SOPC_Calloc(currentSecurityPolicy->nbOfUserTokenPolicies, sizeof(OpcUa_UserTokenPolicy));
    if (NULL != newEndPointDescription->UserIdentityTokens)
    {
        newEndPointDescription->NoOfUserIdentityTokens = (int32_t) currentSecurityPolicy->nbOfUserTokenPolicies;
        for (int32_t userIdTokenIdx = 0; userIdTokenIdx < newEndPointDescription->NoOfUserIdentityTokens;
             userIdTokenIdx++)
        {
            OpcUa_UserTokenPolicy* srcUserTokenPolicy = &currentSecurityPolicy->userTokenPolicies[userIdTokenIdx];
            OpcUa_UserTokenPolicy* destUserTokenPolicy = &newEndPointDescription->UserIdentityTokens[userIdTokenIdx];
            OpcUa_UserTokenPolicy_Initialize(destUserTokenPolicy);
            // Note: ignoring return status since only cases of failure are empty string as source of data
            SOPC_UNUSED_RESULT(
                SOPC_String_AttachFrom(&destUserTokenPolicy->IssuedTokenType, &srcUserTokenPolicy->IssuedTokenType));
            SOPC_UNUSED_RESULT(SOPC_String_AttachFrom(&destUserTokenPolicy->IssuerEndpointUrl,
                                                      &srcUserTokenPolicy->IssuerEndpointUrl));
            SOPC_UNUSED_RESULT(SOPC_String_AttachFrom(&destUserTokenPolicy->PolicyId, &srcUserTokenPolicy->PolicyId));
            SOPC_UNUSED_RESULT(SOPC_String_AttachFrom(&destUserTokenPolicy->SecurityPolicyUri,
                                                      &srcUserTokenPolicy->SecurityPolicyUri));

            destUserTokenPolicy->TokenType = srcUserTokenPolicy->TokenType;
        }
    }
}

/* This function computes the security level */
/* see §7.10 Part4 - Value 0 is for not recommended endPoint.
    Others values corresponds to more secured endPoints.*/
static SOPC_Byte getSecurityLevel(OpcUa_MessageSecurityMode SecurityMode, SOPC_String* securityPolicy)
{
    const SOPC_SecurityPolicy_Config* pProfileCfg = SOPC_CryptoProfile_Get(SOPC_String_GetRawCString(securityPolicy));
    if (NULL == pProfileCfg)
    {
        return 0;
    }
    const SOPC_CryptoProfile* secPolicy = pProfileCfg->profile(pProfileCfg->uri);
    if (NULL == secPolicy)
    {
        // Unknown security policy, return unrecommended endpoint value
        return 0;
    }

    SOPC_Byte secuPolicyWeight = pProfileCfg->secuPolicyWeight;

    switch (SecurityMode)
    {
    case OpcUa_MessageSecurityMode_SignAndEncrypt:
        return (SOPC_Byte)(2 * secuPolicyWeight);
    case OpcUa_MessageSecurityMode_Sign:
        return secuPolicyWeight;
    default:
        return 0;
    }
}

static void SOPC_SetCommonFields(OpcUa_EndpointDescription* endointDescription,
                                 SOPC_String* configEndPointURL,
                                 SOPC_String* securityPolicy)
{
    // Set endpointUrl
    endointDescription->EndpointUrl = *configEndPointURL;

    /* Note: current server only support anonymous user authentication */

    // Set transport profile URI
    endointDescription->TransportProfileUri = tcpUaTransportProfileURI;

    // Set securityPolicyUri
    SOPC_ReturnStatus status = SOPC_String_AttachFrom(&endointDescription->SecurityPolicyUri, securityPolicy);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Failed to set security policy in application description of response");
    }
}

constants_statuscodes_bs__t_StatusCode_i SOPC_Discovery_GetEndPointsDescriptions(
    const constants__t_endpoint_config_idx_i endpoint_config_idx,
    bool isCreateSessionResponse,
    SOPC_String* requestEndpointUrl,
    int32_t nbOfLocaleIds,
    SOPC_String* localeIdArray,
    uint32_t* nbOfEndpointDescriptions,
    OpcUa_EndpointDescription** endpointDescriptions)
{
    constants_statuscodes_bs__t_StatusCode_i serviceResult = constants_statuscodes_bs__e_sc_bad_invalid_argument;

    SOPC_Endpoint_Config* sopcEndpointConfig = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_String configEndPointURL;
    uint8_t nbSecuConfigs = 0;
    uint32_t nbEndpointDescription = 0;
    SOPC_SecurityPolicy* tabSecurityPolicy = NULL;
    OpcUa_EndpointDescription* currentConfig_EndpointDescription = NULL;
    bool userTokenServerCertNeeded = false;
    SOPC_SerializedCertificate* serverCertificate = NULL;

    SOPC_String_Initialize(&configEndPointURL);

    sopcEndpointConfig = SOPC_ToolkitServer_GetEndpointConfig(endpoint_config_idx);

    if (NULL != sopcEndpointConfig)
    {
        status = SOPC_String_AttachFromCstring(&configEndPointURL, sopcEndpointConfig->endpointURL);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Failed to set endpoint URL value in application description of response");
        }

        if (sopcEndpointConfig->serverConfigPtr->serverKeyCertPair != NULL)
        {
            status = SOPC_KeyCertPair_GetSerializedCertCopy(sopcEndpointConfig->serverConfigPtr->serverKeyCertPair,
                                                            &serverCertificate);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "Failed to retrieve server certificate data");
            }
        }

        /* Note: comparison with requested URL is not necessary since we have to return a default URL in any case */
        SOPC_UNUSED_ARG(requestEndpointUrl);
        serviceResult = constants_statuscodes_bs__e_sc_ok;
    }
    else
    {
        serviceResult = constants_statuscodes_bs__e_sc_bad_unexpected_error;
    }

    if (constants_statuscodes_bs__e_sc_ok == serviceResult)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        char** preferredLocales = (char**) SOPC_String_GetRawCStringArray(nbOfLocaleIds, localeIdArray);
        SOPC_GCC_DIAGNOSTIC_RESTORE

        nbSecuConfigs = sopcEndpointConfig->nbSecuConfigs;
        tabSecurityPolicy = sopcEndpointConfig->secuConfigurations;

        /* if false == isCreateSessionResponse
         * see part 4 §5.6.2.2: it is recommended that Servers only include the endpointUrl,
         * securityMode, securityPolicyUri, userIdentityTokens, transportProfileUri
         * and securityLevel with all other parameters set to null. */

        // TODO: this code section can probably be optimized
        currentConfig_EndpointDescription = SOPC_Calloc(3u * nbSecuConfigs, sizeof(OpcUa_EndpointDescription));
        nbEndpointDescription = 0;

        if (currentConfig_EndpointDescription != NULL)
        {
            for (int iSecuConfig = 0; iSecuConfig < nbSecuConfigs; iSecuConfig++)
            {
                SOPC_SecurityPolicy currentSecurityPolicy = tabSecurityPolicy[iSecuConfig];
                uint16_t securityModes = currentSecurityPolicy.securityModes;

                if (0 == currentSecurityPolicy.nbOfUserTokenPolicies)
                {
                    /*
                     *  GetEndPointsDescriptions shall not return endpoint descriptions with empty UserTokenPolicies.
                     *  This kind of endpoint configuration is only used to declare implicit discovery endpoint
                     *  and shall not be returned by this service since only the endpoint description allowing sessions
                     * activation are allowed.
                     *
                     */
                    continue;
                }

                // Add an EndpointDescription per security mode
                if ((SOPC_SECURITY_MODE_NONE_MASK & securityModes) != 0)
                {
                    OpcUa_EndpointDescription* newEndPointDescription =
                        &currentConfig_EndpointDescription[nbEndpointDescription];
                    OpcUa_EndpointDescription_Initialize(newEndPointDescription);

                    SOPC_SetCommonFields(newEndPointDescription, &configEndPointURL,
                                         &currentSecurityPolicy.securityPolicy);

                    // Set securityMode
                    newEndPointDescription->SecurityMode = OpcUa_MessageSecurityMode_None;

                    // Set userIdentityTokens
                    copyUserIdTokens(&currentSecurityPolicy, newEndPointDescription);

                    // Set securityLevel
                    newEndPointDescription->SecurityLevel =
                        getSecurityLevel(newEndPointDescription->SecurityMode, &currentSecurityPolicy.securityPolicy);

                    OpcUa_ApplicationDescription_Initialize(&newEndPointDescription->Server);
                    if (!isCreateSessionResponse)
                    {
                        for (uint8_t userTokenIndex = 0;
                             !userTokenServerCertNeeded && userTokenIndex < currentSecurityPolicy.nbOfUserTokenPolicies;
                             userTokenIndex++)
                        {
                            if (currentSecurityPolicy.userTokenPolicies[userTokenIndex].SecurityPolicyUri.Length > 0)
                            {
                                // If SecurityPolicyURI != None, this token should need encryption or server certificate
                                userTokenServerCertNeeded =
                                    (0 != strcmp(SOPC_SecurityPolicy_None_URI,
                                                 SOPC_String_GetRawCString(
                                                     &currentSecurityPolicy.userTokenPolicies[userTokenIndex]
                                                          .SecurityPolicyUri)));
                            }
                        }
                        // Set serverCertificate in case it is needed for a user token policy
                        if (userTokenServerCertNeeded)
                        {
                            if (serverCertificate != NULL)
                            {
                                SOPC_SetServerCertificate(serverCertificate,
                                                          &newEndPointDescription->ServerCertificate);
                            }
                            else
                            {
                                SOPC_Logger_TraceError(
                                    SOPC_LOG_MODULE_CLIENTSERVER,
                                    "Server certificate is NULL while it is needed for user token encryption");
                            }
                        }
                        // Set ApplicationDescription
                        SOPC_SetServerApplicationDescription(sopcEndpointConfig, preferredLocales,
                                                             &newEndPointDescription->Server);
                    }
                    else
                    {
                        // Set Server.ApplicationUri only (see mantis #3578 + part 4 v1.04 RC)
                        status = SOPC_String_AttachFrom(
                            &newEndPointDescription->Server.ApplicationUri,
                            &sopcEndpointConfig->serverConfigPtr->serverDescription.ApplicationUri);

                        if (SOPC_STATUS_OK != status)
                        {
                            SOPC_Logger_TraceWarning(
                                SOPC_LOG_MODULE_CLIENTSERVER,
                                "Failed to set application URI value in application description of response");
                        }
                    }

                    nbEndpointDescription++;
                }

                if ((SOPC_SECURITY_MODE_SIGN_MASK & securityModes) != 0)
                {
                    OpcUa_EndpointDescription* newEndPointDescription =
                        &currentConfig_EndpointDescription[nbEndpointDescription];
                    OpcUa_EndpointDescription_Initialize(newEndPointDescription);

                    SOPC_SetCommonFields(newEndPointDescription, &configEndPointURL,
                                         &currentSecurityPolicy.securityPolicy);

                    // Set securityMode
                    newEndPointDescription->SecurityMode = OpcUa_MessageSecurityMode_Sign;

                    // Set userIdentityTokens
                    copyUserIdTokens(&currentSecurityPolicy, newEndPointDescription);

                    // Set securityLevel
                    newEndPointDescription->SecurityLevel =
                        getSecurityLevel(newEndPointDescription->SecurityMode, &currentSecurityPolicy.securityPolicy);
                    ;

                    OpcUa_ApplicationDescription_Initialize(&newEndPointDescription->Server);
                    if (!isCreateSessionResponse)
                    {
                        // Set serverCertificate
                        SOPC_SetServerCertificate(serverCertificate, &newEndPointDescription->ServerCertificate);
                        // Set ApplicationDescription
                        SOPC_SetServerApplicationDescription(sopcEndpointConfig, preferredLocales,
                                                             &newEndPointDescription->Server);
                    }
                    else
                    {
                        // Set Server.ApplicationUri only (see mantis #3578 + part 4 v1.04 RC)
                        status = SOPC_String_AttachFrom(
                            &newEndPointDescription->Server.ApplicationUri,
                            &sopcEndpointConfig->serverConfigPtr->serverDescription.ApplicationUri);
                        if (SOPC_STATUS_OK != status)
                        {
                            SOPC_Logger_TraceWarning(
                                SOPC_LOG_MODULE_CLIENTSERVER,
                                "Failed to set application URI value in application description of response");
                        }
                    }

                    nbEndpointDescription++;
                }

                if ((SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK & securityModes) != 0)
                {
                    OpcUa_EndpointDescription* newEndPointDescription =
                        &currentConfig_EndpointDescription[nbEndpointDescription];
                    OpcUa_EndpointDescription_Initialize(newEndPointDescription);

                    SOPC_SetCommonFields(newEndPointDescription, &configEndPointURL,
                                         &currentSecurityPolicy.securityPolicy);

                    // Set securityMode
                    newEndPointDescription->SecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;

                    // Set userIdentityTokens
                    copyUserIdTokens(&currentSecurityPolicy, newEndPointDescription);

                    // Set securityLevel
                    newEndPointDescription->SecurityLevel =
                        getSecurityLevel(newEndPointDescription->SecurityMode, &currentSecurityPolicy.securityPolicy);

                    OpcUa_ApplicationDescription_Initialize(&newEndPointDescription->Server);
                    if (!isCreateSessionResponse)
                    {
                        // Set serverCertificate
                        SOPC_SetServerCertificate(serverCertificate, &newEndPointDescription->ServerCertificate);
                        // Set ApplicationDescription
                        SOPC_SetServerApplicationDescription(sopcEndpointConfig, preferredLocales,
                                                             &newEndPointDescription->Server);
                    }
                    else
                    {
                        // Set Server.ApplicationUri only (see mantis #3578 + part 4 v1.04 RC)
                        status = SOPC_String_AttachFrom(
                            &newEndPointDescription->Server.ApplicationUri,
                            &sopcEndpointConfig->serverConfigPtr->serverDescription.ApplicationUri);
                        if (SOPC_STATUS_OK != status)
                        {
                            SOPC_Logger_TraceWarning(
                                SOPC_LOG_MODULE_CLIENTSERVER,
                                "Failed to set application URI value in application description of response");
                        }
                    }

                    nbEndpointDescription++;
                }
            }
        }
        else
        {
            serviceResult = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        }

        OpcUa_EndpointDescription* final_OpcUa_EndpointDescription = NULL;
        if (nbEndpointDescription > 0)
        {
            final_OpcUa_EndpointDescription =
                SOPC_Calloc((size_t) nbEndpointDescription, sizeof(OpcUa_EndpointDescription));
            if (final_OpcUa_EndpointDescription != NULL)
            {
                for (uint32_t i = 0; i < nbEndpointDescription; i++)
                {
                    final_OpcUa_EndpointDescription[i] = currentConfig_EndpointDescription[i];
                }
            }
            else
            {
                serviceResult = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
        }
        *nbOfEndpointDescriptions = nbEndpointDescription;
        *endpointDescriptions = final_OpcUa_EndpointDescription;

        if (currentConfig_EndpointDescription != NULL)
        {
            SOPC_Free(currentConfig_EndpointDescription);
        }

        SOPC_Free(preferredLocales);
    }

    SOPC_KeyManager_SerializedCertificate_Delete(serverCertificate);

    return serviceResult;
}

bool SOPC_Discovery_ContainsBinaryProfileURI(uint32_t nbOfProfileURI, SOPC_String* profileUris)
{
    bool result = false;
    uint32_t i = 0;
    for (i = 0; i < nbOfProfileURI && false == result; i++)
    {
        result = SOPC_String_Equal(&tcpUaTransportProfileURI, &profileUris[i]);
    }
    return result;
}
