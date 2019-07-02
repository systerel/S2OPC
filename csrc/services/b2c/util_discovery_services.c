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

#include "constants.h"
#include "sopc_crypto_profiles.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "util_discovery_services.h"

const SOPC_String tcpUaTransportProfileURI = {
    .Length = 65,
    .DoNotClear = true,
    .Data = (SOPC_Byte*) "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary",
};

const OpcUa_UserTokenPolicy anonymousUserTokenPolicy = {
    .PolicyId = {9, true, (SOPC_Byte*) "anonymous"},
    .TokenType = OpcUa_UserTokenType_Anonymous,
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
};

const OpcUa_UserTokenPolicy userNameUserTokenPolicy = {
    .PolicyId = {9, true, (SOPC_Byte*) "username"},
    .TokenType = OpcUa_UserTokenType_UserName,
    .IssuedTokenType = {0, true, NULL},
    .IssuerEndpointUrl = {0, true, NULL},
    .SecurityPolicyUri = {0, true, NULL},
};

static void SOPC_SetServerCertificate(SOPC_Endpoint_Config* sopcEndpointConfig, OpcUa_EndpointDescription* epDesc)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t tmpLength = 0;

    if (sopcEndpointConfig->serverCertificate == NULL)
    {
        return;
    }

    assert(sopcEndpointConfig->serverCertificate->length <= INT32_MAX);
    status = SOPC_ByteString_CopyFromBytes((SOPC_ByteString*) &epDesc->ServerCertificate,
                                           sopcEndpointConfig->serverCertificate->data,
                                           (int32_t) sopcEndpointConfig->serverCertificate->length);
    assert(SOPC_STATUS_OK == status);
    assert(tmpLength <= INT32_MAX);
    epDesc->ServerCertificate.Length = (int32_t) sopcEndpointConfig->serverCertificate->length;
}

static void SOPC_SetServerApplicationDescription(SOPC_Endpoint_Config* sopcEndpointConfig,
                                                 OpcUa_ApplicationDescription* appDesc)
{
    int32_t idx = 0;
    SOPC_String_AttachFrom(&appDesc->ApplicationUri, &sopcEndpointConfig->serverDescription.ApplicationUri);
    SOPC_String_AttachFrom(&appDesc->ProductUri, &sopcEndpointConfig->serverDescription.ProductUri);
    SOPC_LocalizedText_Copy(&appDesc->ApplicationName, &sopcEndpointConfig->serverDescription.ApplicationName);
    assert(sopcEndpointConfig->serverDescription.ApplicationType != OpcUa_ApplicationType_Client);
    appDesc->ApplicationType = sopcEndpointConfig->serverDescription.ApplicationType;
    SOPC_String_AttachFrom(&appDesc->GatewayServerUri, &sopcEndpointConfig->serverDescription.GatewayServerUri);
    SOPC_String_AttachFrom(&appDesc->DiscoveryProfileUri, &sopcEndpointConfig->serverDescription.DiscoveryProfileUri);

    if (sopcEndpointConfig->serverDescription.NoOfDiscoveryUrls > 0)
    {
        appDesc->DiscoveryUrls =
            SOPC_Calloc((size_t) sopcEndpointConfig->serverDescription.NoOfDiscoveryUrls, sizeof(SOPC_String));
    }
    else
    {
        appDesc->DiscoveryUrls = SOPC_Calloc(1, sizeof(SOPC_String));
    }
    if (appDesc->DiscoveryUrls != NULL)
    {
        if (sopcEndpointConfig->serverDescription.NoOfDiscoveryUrls > 0)
        {
            appDesc->NoOfDiscoveryUrls = sopcEndpointConfig->serverDescription.NoOfDiscoveryUrls;
            // Copy the discovery URLs provided by configuration
            for (idx = 0; idx < sopcEndpointConfig->serverDescription.NoOfDiscoveryUrls; idx++)
            {
                SOPC_String_AttachFrom(&appDesc->DiscoveryUrls[idx],
                                       &sopcEndpointConfig->serverDescription.DiscoveryUrls[idx]);
            }
        }
        else
        {
            appDesc->NoOfDiscoveryUrls = 1;
            // If not provided by configuration: we set the endpoint URL as default discovery URL since it is mandatory
            SOPC_String_AttachFromCstring(&appDesc->DiscoveryUrls[0], sopcEndpointConfig->endpointURL);
        }
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
            SOPC_String_AttachFrom(&destUserTokenPolicy->IssuedTokenType, &srcUserTokenPolicy->IssuedTokenType);
            SOPC_String_AttachFrom(&destUserTokenPolicy->IssuerEndpointUrl, &srcUserTokenPolicy->IssuerEndpointUrl);
            SOPC_String_AttachFrom(&destUserTokenPolicy->PolicyId, &srcUserTokenPolicy->PolicyId);
            SOPC_String_AttachFrom(&destUserTokenPolicy->SecurityPolicyUri, &srcUserTokenPolicy->SecurityPolicyUri);
            destUserTokenPolicy->TokenType = srcUserTokenPolicy->TokenType;
        }
    }
}

/* This function computes the security level */
/* see §7.10 Part4 - Value 0 is for not recommended endPoint.
    Others values corresponds to more secured endPoints.*/
static SOPC_Byte getSecurityLevel(OpcUa_MessageSecurityMode SecurityMode, SOPC_String* securityPolicy)
{
    const SOPC_CryptoProfile* secPolicy = SOPC_CryptoProfile_Get(SOPC_String_GetRawCString(securityPolicy));
    SOPC_Byte secuPolicyWeight = 0;

    switch (secPolicy->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        secuPolicyWeight = 2;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        secuPolicyWeight = 1;
        break;
    default:
        secuPolicyWeight = 0;
    }

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

constants_statuscodes_bs__t_StatusCode_i SOPC_Discovery_GetEndPointsDescriptions(
    const constants__t_endpoint_config_idx_i endpoint_config_idx,
    bool isCreateSessionResponse,
    SOPC_String* requestEndpointUrl,
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

    SOPC_String_Initialize(&configEndPointURL);

    sopcEndpointConfig = SOPC_ToolkitServer_GetEndpointConfig(endpoint_config_idx);

    if (NULL != sopcEndpointConfig)
    {
        status = SOPC_String_AttachFromCstring(&configEndPointURL, sopcEndpointConfig->endpointURL);
        assert(SOPC_STATUS_OK == status);

        /* Note: comparison with requested URL is not necessary since we have to return a default URL in any case */
        (void) requestEndpointUrl;
        serviceResult = constants_statuscodes_bs__e_sc_ok;
    }
    else
    {
        serviceResult = constants_statuscodes_bs__e_sc_bad_unexpected_error;
    }

    if (constants_statuscodes_bs__e_sc_ok == serviceResult)
    {
        nbSecuConfigs = sopcEndpointConfig->nbSecuConfigs;
        tabSecurityPolicy = sopcEndpointConfig->secuConfigurations;

        /* if false == isCreateSessionResponse
         * see part 4 §5.6.2.2: it is recommended that Servers only include the endpointUrl,
         * securityMode, securityPolicyUri, userIdentityTokens, transportProfileUri
         * and securityLevel with all other parameters set to null. */

        // TODO: this code section can probably be optimized
        currentConfig_EndpointDescription = SOPC_Calloc(3u * nbSecuConfigs, sizeof(OpcUa_EndpointDescription));
        nbEndpointDescription = 0;
        OpcUa_EndpointDescription newEndPointDescription;
        OpcUa_EndpointDescription_Initialize(&newEndPointDescription);

        if (currentConfig_EndpointDescription != NULL)
        {
            // Set endpointUrl
            newEndPointDescription.EndpointUrl = configEndPointURL;

            /* Note: current server only support anonymous user authentication */

            // Set transport profile URI
            newEndPointDescription.TransportProfileUri = tcpUaTransportProfileURI;

            for (int iSecuConfig = 0; iSecuConfig < nbSecuConfigs; iSecuConfig++)
            {
                SOPC_SecurityPolicy currentSecurityPolicy = tabSecurityPolicy[iSecuConfig];
                uint16_t securityModes = currentSecurityPolicy.securityModes;

                // Set securityPolicyUri
                SOPC_String_AttachFrom(&newEndPointDescription.SecurityPolicyUri,
                                       &currentSecurityPolicy.securityPolicy);

                // Add an EndpointDescription per security mode
                if ((SOPC_SECURITY_MODE_NONE_MASK & securityModes) != 0)
                {
                    // Set securityMode
                    newEndPointDescription.SecurityMode = OpcUa_MessageSecurityMode_None;

                    // Set userIdentityTokens
                    copyUserIdTokens(&currentSecurityPolicy, &newEndPointDescription);

                    // Set securityLevel
                    newEndPointDescription.SecurityLevel =
                        getSecurityLevel(newEndPointDescription.SecurityMode, &currentSecurityPolicy.securityPolicy);

                    OpcUa_ApplicationDescription_Initialize(&newEndPointDescription.Server);
                    if (false == isCreateSessionResponse)
                    {
                        // Set ApplicationDescription
                        SOPC_SetServerApplicationDescription(sopcEndpointConfig, &newEndPointDescription.Server);
                    }
                    else
                    {
                        // Set Server.ApplicationUri only (see mantis #3578 + part 4 v1.04 RC)
                        SOPC_String_AttachFrom(&newEndPointDescription.Server.ApplicationUri,
                                               &sopcEndpointConfig->serverDescription.ApplicationUri);
                    }

                    currentConfig_EndpointDescription[nbEndpointDescription] = newEndPointDescription;
                    nbEndpointDescription++;
                }

                if ((SOPC_SECURITY_MODE_SIGN_MASK & securityModes) != 0)
                {
                    // Set securityMode
                    newEndPointDescription.SecurityMode = OpcUa_MessageSecurityMode_Sign;

                    // Set userIdentityTokens
                    copyUserIdTokens(&currentSecurityPolicy, &newEndPointDescription);

                    // Set securityLevel
                    newEndPointDescription.SecurityLevel =
                        getSecurityLevel(newEndPointDescription.SecurityMode, &currentSecurityPolicy.securityPolicy);
                    ;

                    OpcUa_ApplicationDescription_Initialize(&newEndPointDescription.Server);
                    if (false == isCreateSessionResponse)
                    {
                        // Set serverCertificate
                        SOPC_SetServerCertificate(sopcEndpointConfig, &newEndPointDescription);
                        // Set ApplicationDescription
                        SOPC_SetServerApplicationDescription(sopcEndpointConfig, &newEndPointDescription.Server);
                    }
                    else
                    {
                        // Set Server.ApplicationUri only (see mantis #3578 + part 4 v1.04 RC)
                        SOPC_String_AttachFrom(&newEndPointDescription.Server.ApplicationUri,
                                               &sopcEndpointConfig->serverDescription.ApplicationUri);
                    }

                    currentConfig_EndpointDescription[nbEndpointDescription] = newEndPointDescription;
                    nbEndpointDescription++;
                }

                if ((SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK & securityModes) != 0)
                {
                    // Set securityMode
                    newEndPointDescription.SecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;

                    // Set userIdentityTokens
                    copyUserIdTokens(&currentSecurityPolicy, &newEndPointDescription);

                    // Set securityLevel
                    newEndPointDescription.SecurityLevel =
                        getSecurityLevel(newEndPointDescription.SecurityMode, &currentSecurityPolicy.securityPolicy);

                    OpcUa_ApplicationDescription_Initialize(&newEndPointDescription.Server);
                    if (false == isCreateSessionResponse)
                    {
                        // Set serverCertificate
                        SOPC_SetServerCertificate(sopcEndpointConfig, &newEndPointDescription);
                        // Set ApplicationDescription
                        SOPC_SetServerApplicationDescription(sopcEndpointConfig, &newEndPointDescription.Server);
                    }
                    else
                    {
                        // Set Server.ApplicationUri only (see mantis #3578 + part 4 v1.04 RC)
                        SOPC_String_AttachFrom(&newEndPointDescription.Server.ApplicationUri,
                                               &sopcEndpointConfig->serverDescription.ApplicationUri);
                    }

                    currentConfig_EndpointDescription[nbEndpointDescription] = newEndPointDescription;
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
    }

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
