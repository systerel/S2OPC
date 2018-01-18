/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"

#include <assert.h>
#include <stdbool.h>

#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_secure_channels_api.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "stub_sc_sopc_services_api.h"

static SOPC_SecureChannel_Config* scConfigSingleton = NULL;
static SOPC_Endpoint_Config* epConfigSingleton = NULL;

SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct)
{
    (void) pAppFct;

    SOPC_Helper_EndiannessCfg_Initialize();

    SOPC_EventTimer_Initialize();
    SOPC_Sockets_Initialize();
    SOPC_SecureChannels_Initialize();
    SOPC_Services_Initialize();
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Toolkit_Configured()
{
    return SOPC_STATUS_OK;
}

void SOPC_Toolkit_ClearScConfigElt(SOPC_SecureChannel_Config* scConfig)
{
    if (scConfig != NULL && scConfig->isClientSc == false)
    {
        // In case of server it is an internally created config
        // => only client certificate was specifically allocated

        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_KeyManager_Certificate_Free((SOPC_Certificate*) scConfig->crt_cli);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        free(scConfig);
    }
}

void SOPC_Toolkit_Clear()
{
    SOPC_Sockets_Clear();
    SOPC_EventTimer_Clear();
    SOPC_SecureChannels_Clear();
    SOPC_Services_Clear();
    SOPC_Toolkit_ClearScConfigElt(scConfigSingleton);
    scConfigSingleton = NULL;
    epConfigSingleton = NULL;
}

uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig)
{
    uint32_t result = 0;
    if (NULL == scConfigSingleton)
    {
        result = 1;
        scConfigSingleton = scConfig;
    }
    return result;
}

SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx)
{
    SOPC_SecureChannel_Config* result = NULL;
    if (scConfigIdx == 1)
    {
        result = scConfigSingleton;
    }
    return result;
}

uint32_t SOPC_ToolkitServer_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig)
{
    uint32_t result = 0;
    if (NULL == scConfigSingleton)
    {
        result = 10;
        scConfigSingleton = scConfig;
    }
    return result;
}

bool SOPC_ToolkitServer_RemoveSecureChannelConfig(uint32_t serverScConfigIdx)
{
    bool result = false;
    if (serverScConfigIdx == 10)
    {
        scConfigSingleton = NULL;
        result = true;
    }
    return result;
}

SOPC_SecureChannel_Config* SOPC_ToolkitServer_GetSecureChannelConfig(uint32_t serverScConfigIdx)
{
    SOPC_SecureChannel_Config* result = NULL;
    if (serverScConfigIdx == 10)
    {
        result = scConfigSingleton;
    }
    return result;
}

uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* epConfig)
{
    uint32_t result = 0;
    if (NULL == epConfigSingleton)
    {
        result = 2;
        epConfigSingleton = epConfig;
    }
    return result;
}

SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx)
{
    SOPC_Endpoint_Config* result = NULL;
    if (epConfigIdx == 2)
    {
        result = epConfigSingleton;
    }
    return result;
}

SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes()
{
    // No additional types: return static known types
    return SOPC_KnownEncodeableTypes;
}

SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces()
{
    return NULL;
}
