/*
 *  Copyright (C) 2017 Systerel and others.
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

#include <stdbool.h>

#include "sopc_helper_endianess_cfg.h"
#include "sopc_secure_channels_api.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "stub_sc_sopc_services_api.h"

static SOPC_SLinkedList* scConfigs = NULL;
static SOPC_SLinkedList* epConfigs = NULL;
static uint32_t scConfigIdxMax = 0;
static uint32_t epConfigIdxMax = 0;

SOPC_StatusCode SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct)
{
    (void) pAppFct;

    SOPC_Helper_EndianessCfg_Initialize();

    scConfigs = SOPC_SLinkedList_Create(0);

    if (NULL != scConfigs)
    {
        epConfigs = SOPC_SLinkedList_Create(0);
    }

    if (NULL != epConfigs)
    {
        SOPC_Sockets_Initialize();
        SOPC_SecureChannels_Initialize();
        SOPC_Services_Initialize();
    }

    if (NULL == epConfigs)
    {
        SOPC_Toolkit_Clear();
        return STATUS_NOK;
    }
    else
    {
        return STATUS_OK;
    }
}

SOPC_StatusCode SOPC_Toolkit_Configured()
{
    return STATUS_OK;
}

void SOPC_Toolkit_ClearScConfigElt(uint32_t id, void* val)
{
    (void) (id);
    SOPC_SecureChannel_Config* scConfig = val;
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

// Deallocate fields allocated on server side only and free all the SC configs
static void SOPC_Toolkit_ClearScConfigs(void)
{
    SOPC_SLinkedList_Apply(scConfigs, SOPC_Toolkit_ClearScConfigElt);
    SOPC_SLinkedList_Delete(scConfigs);
    scConfigs = NULL;
}

void SOPC_Toolkit_Clear()
{
    SOPC_Sockets_Clear();
    SOPC_SecureChannels_Clear();
    SOPC_Services_Clear();
    SOPC_Toolkit_ClearScConfigs();
    SOPC_SLinkedList_Delete(epConfigs);
    epConfigs = NULL;
}

static SOPC_StatusCode SOPC_IntToolkitConfig_AddConfig(SOPC_SLinkedList* configList, uint32_t idx, void* config)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    void* res = SOPC_SLinkedList_FindFromId(configList, idx);
    if (NULL == res && NULL != config)
    { // Idx is unique
        if (config == SOPC_SLinkedList_Prepend(configList, idx, config))
        {
            status = STATUS_OK;
        }
        else
        {
            status = STATUS_NOK;
        }
    }
    return status;
}

uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig)
{
    uint32_t result = 0;
    SOPC_StatusCode status;
    if (NULL != scConfig)
    {
        scConfigIdxMax++;
        status = SOPC_IntToolkitConfig_AddConfig(scConfigs, scConfigIdxMax, (void*) scConfig);
        if (STATUS_OK == status)
        {
            result = scConfigIdxMax;
        }
        else
        {
            scConfigIdxMax--;
        }
    }
    return result;
}

SOPC_SecureChannel_Config* SOPC_Toolkit_GetSecureChannelConfig(uint32_t scConfigIdx)
{
    return (SOPC_SecureChannel_Config*) SOPC_SLinkedList_FindFromId(scConfigs, scConfigIdx);
    ;
}

uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* epConfig)
{
    uint32_t result = 0;
    if (NULL != epConfig)
    {
        epConfigIdxMax++;
        if (STATUS_OK == SOPC_IntToolkitConfig_AddConfig(epConfigs, epConfigIdxMax, (void*) epConfig))
        {
            result = epConfigIdxMax;
        }
        else
        {
            epConfigIdxMax--;
        }
    }
    return result;
}

SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx)
{
    return (SOPC_Endpoint_Config*) SOPC_SLinkedList_FindFromId(epConfigs, epConfigIdx);
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
