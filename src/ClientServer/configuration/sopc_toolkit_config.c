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
#include <stdio.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_common.h"
#include "sopc_encodeable.h"
#include "sopc_enum_types.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_build_info.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include "address_space_impl.h"
#include "util_b2c.h"

/* Check IEEE-754 compliance */
#include "sopc_ieee_check.h"

static struct
{
    uint8_t initDone;
    uint8_t serverConfigLocked;
    Mutex mut;
    SOPC_SecureChannel_Config* scConfigs[SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1];
    SOPC_SecureChannel_Config* serverScConfigs[SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1];
    SOPC_Endpoint_Config* epConfigs[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1]; // index 0 reserved
    uint32_t scConfigIdxMax;
    uint32_t serverScLastConfigIdx;
    uint32_t epConfigIdxMax;
} // Any change in values below shall be also done in SOPC_Toolkit_Clear
tConfig = {.initDone = false,
           .serverConfigLocked = false,
           .scConfigIdxMax = 0,
           .serverScLastConfigIdx = 0,
           .epConfigIdxMax = 0};

SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pAppFct)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status && !SOPC_Common_IsInitialized())
    {
        /* Initialize with default log configuration */
        SOPC_Log_Configuration defaultLogConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        status = SOPC_Common_Initialize(defaultLogConfiguration);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (tConfig.initDone)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        else
        {
            Mutex_Initialization(&tConfig.mut);
            Mutex_Lock(&tConfig.mut);
            tConfig.initDone = true;

            appEventCallback = pAppFct;

            // Ensure constants cannot be modified later
            SOPC_Common_SetEncodingConstants(SOPC_Common_GetDefaultEncodingConstants());
            SOPC_Helper_EndiannessCfg_Initialize();

            if (SOPC_STATUS_OK == status)
            {
                memset(tConfig.scConfigs, 0, sizeof(tConfig.scConfigs));
                memset(tConfig.serverScConfigs, 0, sizeof(tConfig.serverScConfigs));
                memset(tConfig.epConfigs, 0, sizeof(tConfig.epConfigs));
                SOPC_App_Initialize();
                SOPC_EventTimer_Initialize();
                SOPC_Sockets_Initialize();
                SOPC_SecureChannels_Initialize(SOPC_Sockets_SetEventHandler);
                SOPC_Services_Initialize(SOPC_SecureChannels_SetEventHandler);

                SOPC_Toolkit_Build_Info toolkitBuildInfo = SOPC_ToolkitConfig_GetBuildInfo();

                /* set log level to INFO for version logging, then restore it */
                SOPC_Log_Level level = SOPC_Logger_GetTraceLogLevel();
                SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_INFO);
                SOPC_Logger_TraceInfo(
                    SOPC_LOG_MODULE_CLIENTSERVER, "Common library DATE='%s' VERSION='%s' SIGNATURE='%s' DOCKER='%s'",
                    toolkitBuildInfo.commonBuildInfo.buildBuildDate, toolkitBuildInfo.commonBuildInfo.buildVersion,
                    toolkitBuildInfo.commonBuildInfo.buildSrcCommit, toolkitBuildInfo.commonBuildInfo.buildDockerId);
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                      "Client/Server toolkit library DATE='%s' VERSION='%s' SIGNATURE='%s' DOCKER='%s'",
                                      toolkitBuildInfo.clientServerBuildInfo.buildBuildDate,
                                      toolkitBuildInfo.clientServerBuildInfo.buildVersion,
                                      toolkitBuildInfo.clientServerBuildInfo.buildSrcCommit,
                                      toolkitBuildInfo.clientServerBuildInfo.buildDockerId);
                SOPC_Logger_SetTraceLogLevel(level);
            }

            Mutex_Unlock(&tConfig.mut);
        }
    }

    return status;
}

static SOPC_ReturnStatus SOPC_SecurityCheck_UserCredentialsEncrypted(const SOPC_SecurityPolicy* pSecurityPolicy,
                                                                     const OpcUa_UserTokenPolicy* pUserTokenPolicies)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_String securityPolicyNoneURI;
    SOPC_String_Initialize(&securityPolicyNoneURI);
    status = SOPC_String_AttachFromCstring(&securityPolicyNoneURI, SOPC_SecurityPolicy_None_URI);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_NOK;
    }

    // Check if SecurityPolicy "security mode" is "None" AND if "UserToken security policy" is "empty" (default)
    if (0 != (pSecurityPolicy->securityModes & SOPC_SECURITY_MODE_NONE_MASK) &&
        pUserTokenPolicies->SecurityPolicyUri.Length <= 0)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Security Check UserCredentials: Failed. Combination not allowed : SecurityPolicy "
                               "security mode is None and UserToken security policy is empty.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check if SecurityPolicy "security mode" is "None or Sign" AND if "UserToken security policy" is "None"
    else if (0 != (pSecurityPolicy->securityModes & (SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_NONE_MASK)) &&
             true == SOPC_String_Equal(&pUserTokenPolicies->SecurityPolicyUri, &securityPolicyNoneURI))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Security Check UserCredentials: Failed. Combination not allowed : SecurityPolicy "
                               "security mode is None or Sign and UserToken security policy is None.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return status;
}

static SOPC_ReturnStatus SOPC_ToolkitServer_SecurityCheck(void)
{
    SOPC_Endpoint_Config* pEpConfig;
    SOPC_SecurityPolicy* pSecurityPolicy;
    OpcUa_UserTokenPolicy* pUserTokenPolicies;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ReturnStatus statusSecurityCheck = SOPC_STATUS_OK;
    SOPC_String securityPolicyNoneURI;

    SOPC_String_Initialize(&securityPolicyNoneURI);
    status = SOPC_String_AttachFromCstring(&securityPolicyNoneURI, SOPC_SecurityPolicy_None_URI);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_NOK;
    }

    for (uint32_t nbEpConfigIndex = 1; nbEpConfigIndex <= tConfig.epConfigIdxMax; nbEpConfigIndex++)
    {
        pEpConfig = tConfig.epConfigs[nbEpConfigIndex];

        for (uint8_t nbSecuIndex = 0; nbSecuIndex < pEpConfig->nbSecuConfigs; nbSecuIndex++)
        {
            pSecurityPolicy = &pEpConfig->secuConfigurations[nbSecuIndex];

            for (uint8_t nbTokenIndex = 0; nbTokenIndex < pSecurityPolicy->nbOfUserTokenPolicies; nbTokenIndex++)
            {
                pUserTokenPolicies = &pSecurityPolicy->userTokenPolicies[nbTokenIndex];

                if (OpcUa_UserTokenType_Anonymous != pUserTokenPolicies->TokenType)
                {
                    status = SOPC_SecurityCheck_UserCredentialsEncrypted(pSecurityPolicy, pUserTokenPolicies);
                    if (SOPC_STATUS_OK != status)
                    {
                        statusSecurityCheck = status;
                        status = SOPC_STATUS_OK;
                    }
                }
            }

            /* Check if SecurityPolicy "security policy URI" is different from "None" AND if SecurityPolicy "security
            mode" is "None" */
            if (false == SOPC_String_Equal(&pSecurityPolicy->securityPolicy, &securityPolicyNoneURI) &&
                0 != (pSecurityPolicy->securityModes & SOPC_SECURITY_MODE_NONE_MASK))
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Security Check: Failed. Combination not allowed : SecurityPolicy security "
                                       "policy URI is different from None and SecurityPolicy security mode is None.\n");
                statusSecurityCheck = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
    }
    return statusSecurityCheck;
}

SOPC_ReturnStatus SOPC_ToolkitServer_Configured(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (tConfig.initDone)
    {
        Mutex_Lock(&tConfig.mut);
        if (!tConfig.serverConfigLocked)
        {
            // Check an address space is defined in case a endpoint configuration exists
            if (tConfig.epConfigIdxMax > 0 && sopc_addressSpace_configured)
            {
                tConfig.serverConfigLocked = true;
                SOPC_AddressSpace_Check_Configured();
                status = SOPC_ToolkitServer_SecurityCheck();
            }
            else
            {
                // No address space defined whereas a server configuration exists
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

static void SOPC_ToolkitServer_ClearScConfig_WithoutLock(uint32_t serverScConfigIdxWithoutOffset)
{
    SOPC_SecureChannel_Config* scConfig = tConfig.serverScConfigs[serverScConfigIdxWithoutOffset];
    if (scConfig != NULL)
    {
        assert(!scConfig->isClientSc);
        // In case of server it is an internally created config
        // => only client certificate was specifically allocated
        // Exceptional case: configuration added internally and shall be freed on clear call
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_KeyManager_SerializedCertificate_Delete((SOPC_SerializedCertificate*) scConfig->crt_cli);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        SOPC_Free(scConfig);
        tConfig.serverScConfigs[serverScConfigIdxWithoutOffset] = NULL;
    }
}

// Deallocate fields allocated on server side only and free all the SC configs
static void SOPC_Toolkit_ClearServerScConfigs_WithoutLock(void)
{
    // Index 0 reserved for indet, index = MAX valid
    for (uint32_t i = 1; i <= SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED; i++)
    {
        SOPC_ToolkitServer_ClearScConfig_WithoutLock(i);
    }
}

void SOPC_Toolkit_Clear(void)
{
    if (tConfig.initDone)
    {
        // Services are in charge to gracefully close all connections.
        // It must be done before stopping the services
        SOPC_Services_CloseAllSCs(false);

        SOPC_Sockets_Clear();
        SOPC_EventTimer_Clear();
        SOPC_SecureChannels_Clear();
        SOPC_Services_Clear();
        SOPC_App_Clear();

        Mutex_Lock(&tConfig.mut);

        SOPC_Toolkit_ClearServerScConfigs_WithoutLock();
        appEventCallback = NULL;
        appAddressSpaceNotificationCallback = NULL;
        address_space_bs__nodes = NULL;
        sopc_addressSpace_configured = false;
        // Reset values to init value
        tConfig.initDone = false;
        tConfig.serverConfigLocked = false;
        tConfig.scConfigIdxMax = 0;
        tConfig.serverScLastConfigIdx = 0;
        tConfig.epConfigIdxMax = 0;
        Mutex_Unlock(&tConfig.mut);
        Mutex_Clear(&tConfig.mut);
    }
    SOPC_Common_Clear();
}

static bool SOPC_Internal_CheckClientSecureChannelConfig(const SOPC_SecureChannel_Config* scConfig)
{
    bool result = true;
    if (!scConfig->isClientSc)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "AddSecureChannelConfig check: isClientSc flag not set");
        result = false;
    }
    if (NULL == scConfig->url)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddSecureChannelConfig check: server endpoint URL not set");
        result = false;
    }
    if (NULL == scConfig->reqSecuPolicyUri)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddSecureChannelConfig check: Security Policy URI not set");
        result = false;
    }
    if (scConfig->msgSecurityMode <= OpcUa_MessageSecurityMode_Invalid ||
        scConfig->msgSecurityMode >= OpcUa_MessageSecurityMode_SizeOf)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "AddSecureChannelConfig check: Security Mode not set");
        result = false;
    }
    if (scConfig->requestedLifetime < SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "AddSecureChannelConfig check: requested lifetime is less than minimum defined: %" PRIu32
                               " < %" PRIu32,
                               scConfig->requestedLifetime, (uint32_t) SOPC_MINIMUM_SECURE_CONNECTION_LIFETIME);
        result = false;
    }
    if ((NULL != scConfig->reqSecuPolicyUri &&
         (0 != strcmp(scConfig->reqSecuPolicyUri, SOPC_SecurityPolicy_None_URI))) ||
        scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None)
    {
        if (NULL == scConfig->pki)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "AddSecureChannelConfig check: PKI is not defined but is required due to Security policy / mode");
            result = false;
        }
        if (NULL == scConfig->crt_cli || NULL == scConfig->key_priv_cli)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AddSecureChannelConfig check: Client certificate or key is not defined but is "
                                   "required due to Security policy / mode");
            result = false;
        }
        if (NULL == scConfig->crt_srv)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "AddSecureChannelConfig check: Server certificate is not defined but is required "
                                   "due to Security policy / mode");
            result = false;
        }
    }
    return result;
}

uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig)
{
    assert(NULL != scConfig);
    uint32_t result = 0;

    if (tConfig.initDone && SOPC_Internal_CheckClientSecureChannelConfig(scConfig))
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.scConfigIdxMax < SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED)
        {
            tConfig.scConfigIdxMax++; // Minimum used == 1 && Maximum used == MAX + 1
            assert(NULL == tConfig.scConfigs[tConfig.scConfigIdxMax]);
            tConfig.scConfigs[tConfig.scConfigIdxMax] = scConfig;
            result = tConfig.scConfigIdxMax;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return result;
}

SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx)
{
    SOPC_SecureChannel_Config* res = NULL;
    if (scConfigIdx > 0 && scConfigIdx <= SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED)
    {
        if (tConfig.initDone)
        {
            Mutex_Lock(&tConfig.mut);
            res = tConfig.scConfigs[scConfigIdx];
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return res;
}

uint32_t SOPC_ToolkitServer_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig)
{
    assert(NULL != scConfig);

    uint32_t lastScIdx = 0;
    uint32_t idxWithServerOffset = 0;

    // TODO: check all parameters of scConfig (requested lifetime >= MIN, etc)
    if (tConfig.initDone)
    {
        Mutex_Lock(&tConfig.mut);
        lastScIdx = tConfig.serverScLastConfigIdx;
        do
        {
            if (lastScIdx < SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED)
            {
                lastScIdx++; // Minimum used == 1 && Maximum used == MAX + 1
                if (NULL == tConfig.serverScConfigs[lastScIdx])
                {
                    tConfig.serverScLastConfigIdx = lastScIdx;
                    tConfig.serverScConfigs[lastScIdx] = scConfig;
                    idxWithServerOffset = SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED +
                                          lastScIdx; // disjoint with SC config indexes for client
                }
            }
            else
            {
                lastScIdx = 0; // lastScIdx++ <=> lastScIdx = 1 will be tested next time
            }
        } while (0 == idxWithServerOffset && lastScIdx != tConfig.serverScLastConfigIdx);
        Mutex_Unlock(&tConfig.mut);
    }
    return idxWithServerOffset;
}

static uint32_t SOPC_ToolkitServer_TranslateSecureChannelConfigIdxOffset(uint32_t serverScConfigIdx)
{
    uint32_t res = 0;
    if (serverScConfigIdx > SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED &&
        serverScConfigIdx <=
            2 * SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED) // disjoint with SC config indexes for client
    {
        res = serverScConfigIdx - SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED;
    }
    return res;
}

SOPC_SecureChannel_Config* SOPC_ToolkitServer_GetSecureChannelConfig(uint32_t serverScConfigIdx)
{
    SOPC_SecureChannel_Config* res = NULL;
    uint32_t idxWithoutOffset = SOPC_ToolkitServer_TranslateSecureChannelConfigIdxOffset(serverScConfigIdx);
    if (idxWithoutOffset != 0 && tConfig.initDone)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.serverConfigLocked)
        {
            res = tConfig.serverScConfigs[idxWithoutOffset];
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

bool SOPC_ToolkitServer_RemoveSecureChannelConfig(uint32_t serverScConfigIdx)
{
    bool res = false;
    uint32_t idxWithoutOffset = SOPC_ToolkitServer_TranslateSecureChannelConfigIdxOffset(serverScConfigIdx);
    if (idxWithoutOffset != 0 && tConfig.initDone)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.serverConfigLocked)
        {
            if (tConfig.serverScConfigs[idxWithoutOffset] != NULL)
            {
                res = true;
                SOPC_ToolkitServer_ClearScConfig_WithoutLock(idxWithoutOffset);
            }
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

static bool SOPC_ToolkitServer_AddEndpointConfig_HasOrAddDiscoveryEndpoint(SOPC_Endpoint_Config* epConfig)
{
    assert(epConfig->nbSecuConfigs <= SOPC_MAX_SECU_POLICIES_CFG);
    int res = 0;
    bool hasNoneSecurityConfig = false;
    for (uint8_t i = 0; i < epConfig->nbSecuConfigs && !hasNoneSecurityConfig; i++)
    {
        res = strcmp(SOPC_SecurityPolicy_None_URI,
                     SOPC_String_GetRawCString(&epConfig->secuConfigurations[i].securityPolicy));
        hasNoneSecurityConfig = (0 == res);
    }

    if (!hasNoneSecurityConfig)
    {
        if (epConfig->nbSecuConfigs < SOPC_MAX_SECU_POLICIES_CFG)
        {
            SOPC_SecurityPolicy* secuPolicy = &epConfig->secuConfigurations[epConfig->nbSecuConfigs];
            // No user token policy defined to forbid any session to be activated on discovery endpoint only
            secuPolicy->nbOfUserTokenPolicies = 0;
            secuPolicy->securityModes = SOPC_SECURITY_MODE_NONE_MASK;
            SOPC_String_Initialize(&secuPolicy->securityPolicy);
            SOPC_ReturnStatus status =
                SOPC_String_AttachFromCstring(&secuPolicy->securityPolicy, SOPC_SecurityPolicy_None_URI);
            if (SOPC_STATUS_OK == status)
            {
                // Implicit discovery endpoint added
                epConfig->nbSecuConfigs++;
                hasNoneSecurityConfig = true;
            }
        } // else: no remaining config to add a discovery endpoint configuration
    }

    return hasNoneSecurityConfig;
}

uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* epConfig)
{
    uint32_t result = 0;
    assert(NULL != epConfig);
    assert(NULL != epConfig->serverConfigPtr);

    if (epConfig->nbSecuConfigs > SOPC_MAX_SECU_POLICIES_CFG)
    {
        return result;
    }

    if (epConfig->hasDiscoveryEndpoint && !SOPC_ToolkitServer_AddEndpointConfig_HasOrAddDiscoveryEndpoint(epConfig))
    {
        return result;
    }

    // TODO: check all parameters of epConfig: certificate presence w.r.t. secu policy, app desc (Uris are valid
    // w.r.t. part 6), etc.
    if (tConfig.initDone)
    {
        Mutex_Lock(&tConfig.mut);
        if (!tConfig.serverConfigLocked)
        {
            if (tConfig.epConfigIdxMax < SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS)
            {
                tConfig.epConfigIdxMax++;
                assert(NULL == tConfig.epConfigs[tConfig.epConfigIdxMax]);
                tConfig.epConfigs[tConfig.epConfigIdxMax] = epConfig;
                result = tConfig.epConfigIdxMax;
            }
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return result;
}

SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx)
{
    SOPC_Endpoint_Config* res = NULL;
    if (tConfig.initDone)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.serverConfigLocked)
        {
            res = tConfig.epConfigs[epConfigIdx];
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

static void SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace)
{
    assert(NULL != addressSpace);
    address_space_bs__nodes = addressSpace;
    sopc_addressSpace_configured = true;
}

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (addressSpace != NULL)
    {
        status = SOPC_STATUS_INVALID_STATE;
        if (tConfig.initDone)
        {
            Mutex_Lock(&tConfig.mut);
            if (!tConfig.serverConfigLocked && !sopc_addressSpace_configured)
            {
                status = SOPC_STATUS_OK;
                SOPC_Internal_ToolkitServer_SetAddressSpaceConfig(addressSpace);
            }
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_AddressSpaceNotif_Fct* pAddSpaceNotifFct)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (pAddSpaceNotifFct != NULL)
    {
        status = SOPC_STATUS_INVALID_STATE;
        if (tConfig.initDone)
        {
            Mutex_Lock(&tConfig.mut);
            if (!tConfig.serverConfigLocked && appAddressSpaceNotificationCallback == NULL)
            {
                status = SOPC_STATUS_OK;
                appAddressSpaceNotificationCallback = pAddSpaceNotifFct;
            }
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return status;
}

SOPC_Toolkit_Build_Info SOPC_ToolkitConfig_GetBuildInfo(void)
{
    return (SOPC_Toolkit_Build_Info){SOPC_Common_GetBuildInfo(), SOPC_ClientServer_GetBuildInfo()};
}

void SOPC_ToolkitClient_ClearAllSCs(void)
{
    if (!tConfig.initDone)
    {
        return;
    }
    SOPC_Services_CloseAllSCs(true);
    Mutex_Lock(&tConfig.mut);
    memset(tConfig.scConfigs, 0, sizeof(tConfig.scConfigs));
    tConfig.scConfigIdxMax = 0;
    Mutex_Unlock(&tConfig.mut);
}
