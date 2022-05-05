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

#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "sopc_common.h"
#include "sopc_encodeable.h"
#include "sopc_event_timer_manager.h"
#include "sopc_filesystem.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_secure_channels_api.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_constants.h"

#include "util_b2c.h"

static struct
{
    uint8_t initDone;
    uint8_t locked;
    Mutex mut;
    SOPC_SecureChannel_Config* scConfigs[SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1];
    SOPC_SecureChannel_Config* serverScConfigs[SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1];
    SOPC_Endpoint_Config* epConfigs[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1]; // index 0 reserved
    uint32_t scConfigIdxMax;
    uint32_t serverScLastConfigIdx;
    uint32_t epConfigIdxMax;
} // Any change in values below shall be also done in SOPC_Toolkit_Clear
tConfig = {.initDone = false, .locked = false, .scConfigIdxMax = 0, .serverScLastConfigIdx = 0, .epConfigIdxMax = 0};

SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL != pAppFct)
    {
        status = SOPC_STATUS_NOT_SUPPORTED;
    }

    if (SOPC_STATUS_OK == status && !SOPC_Common_IsInitialized())
    {
        /* Initialize with default log configuration */
        SOPC_Log_Configuration defaultLogConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        status = SOPC_Common_Initialize(defaultLogConfiguration);
    }

    if (SOPC_STATUS_OK == status && false != tConfig.initDone)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status && false == tConfig.initDone)
    {
        Mutex_Initialization(&tConfig.mut);
        Mutex_Lock(&tConfig.mut);
        tConfig.initDone = true;

        if (SIZE_MAX / (SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1) < sizeof(SOPC_SecureChannel_Config*) ||
            SIZE_MAX / (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1) < sizeof(SOPC_Endpoint_Config*))
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            memset(tConfig.scConfigs, 0,
                   (SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1) * sizeof(SOPC_SecureChannel_Config*));
            memset(tConfig.serverScConfigs, 0,
                   (SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED + 1) * sizeof(SOPC_SecureChannel_Config*));
            memset(tConfig.epConfigs, 0,
                   (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1) * sizeof(SOPC_Endpoint_Config*));
            SOPC_EventTimer_Initialize();
            SOPC_Sockets_Initialize();
            SOPC_SecureChannels_Initialize(SOPC_Sockets_SetEventHandler);
        }

        Mutex_Unlock(&tConfig.mut);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ToolkitServer_Configured(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (false == tConfig.locked)
        {
            tConfig.locked = true;
            status = SOPC_STATUS_OK;
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
        assert(false == scConfig->isClientSc);
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
    if (tConfig.initDone != false)
    {
        SOPC_Sockets_Clear();
        SOPC_EventTimer_Clear();
        SOPC_SecureChannels_Clear();

        Mutex_Lock(&tConfig.mut);

        SOPC_Toolkit_ClearServerScConfigs_WithoutLock();
        SOPC_Logger_Clear();
        // Reset values to init value
        tConfig.initDone = false;
        tConfig.locked = false;
        tConfig.scConfigIdxMax = 0;
        tConfig.serverScLastConfigIdx = 0;
        tConfig.epConfigIdxMax = 0;
        Mutex_Unlock(&tConfig.mut);
        Mutex_Clear(&tConfig.mut);
    }
    SOPC_Common_Clear();
}

uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig)
{
    uint32_t result = 0;
    if (NULL != scConfig)
    {
        // TODO: check all parameters of scConfig (requested lifetime >= MIN, etc)
        if (tConfig.initDone != false)
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
    }
    return result;
}

SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx)
{
    SOPC_SecureChannel_Config* res = NULL;
    if (scConfigIdx > 0 && scConfigIdx <= SOPC_MAX_SECURE_CONNECTIONS_PLUS_BUFFERED)
    {
        if (tConfig.initDone != false)
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
    uint32_t lastScIdx = 0;
    uint32_t idxWithServerOffset = 0;
    if (NULL != scConfig)
    {
        // TODO: check all parameters of scConfig (requested lifetime >= MIN, etc)
        if (tConfig.initDone != false)
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
    if (idxWithoutOffset != 0 && tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.locked != false)
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
    if (idxWithoutOffset != 0 && tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.locked != false)
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

uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* epConfig)
{
    uint32_t result = 0;
    if (NULL != epConfig)
    {
        // TODO: check all parameters of epConfig: certificate presence w.r.t. secu policy, app desc (Uris are valid
        // w.r.t. part 6), etc.
        if (tConfig.initDone != false)
        {
            Mutex_Lock(&tConfig.mut);
            if (false == tConfig.locked)
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
    }
    return result;
}

SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx)
{
    SOPC_Endpoint_Config* res = NULL;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.locked != false)
        {
            res = tConfig.epConfigs[epConfigIdx];
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace)
{
    SOPC_UNUSED_ARG(addressSpace);

    // No services implemented
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_AddressSpaceNotif_Fct* pAddSpaceNotifFct)
{
    SOPC_UNUSED_ARG(pAddSpaceNotifFct);

    // No services implemented
    return SOPC_STATUS_NOT_SUPPORTED;
}
