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

#include "sopc_services_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_helper_endianness_cfg.h"
#include "sopc_secure_channels_api.h"
#include "sopc_sockets_api.h"

#include "sopc_encodeable.h"
#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_time.h"

#include "util_b2c.h"

/* Check IEEE-754 compliance */
#include "sopc_ieee_check.h"

static struct
{
    uint8_t initDone;
    uint8_t locked;
    Mutex mut;
    SOPC_SecureChannel_Config* scConfigs[SOPC_MAX_SECURE_CONNECTIONS + 1];
    SOPC_SecureChannel_Config* serverScConfigs[SOPC_MAX_SECURE_CONNECTIONS + 1];
    SOPC_Endpoint_Config* epConfigs[SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1]; // index 0 reserved
    uint32_t scConfigIdxMax;
    uint32_t serverScLastConfigIdx;
    uint32_t epConfigIdxMax;

    /* OPC UA namespace and encodeable types */
    SOPC_NamespaceTable* nsTable;
    SOPC_EncodeableType** encTypesTable;
    uint32_t nbEncTypesTable;

    /* Log configuration */
    const char* logDirPath;
    uint32_t logMaxBytes;
    uint16_t logMaxFiles;
    SOPC_Log_Level logLevel;

} tConfig = {.initDone = false,
             .locked = false,
             .scConfigIdxMax = 0,
             .serverScLastConfigIdx = 0,
             .epConfigIdxMax = 0,
             .nsTable = NULL,
             .encTypesTable = NULL,
             .nbEncTypesTable = 0,
             .logDirPath = "",
             .logMaxBytes = 1048576, // 1 MB
             .logMaxFiles = 50,
             .logLevel = SOPC_LOG_LEVEL_ERROR};

SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL != pAppFct)
    {
        status = SOPC_STATUS_NOT_SUPPORTED;
    }

    if (false == SOPC_IEEE_Check())
    {
        status = SOPC_STATUS_NOK;
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

        SOPC_Helper_EndiannessCfg_Initialize();
        SOPC_Namespace_Initialize(tConfig.nsTable);

        if (SIZE_MAX / (SOPC_MAX_SECURE_CONNECTIONS + 1) < sizeof(SOPC_SecureChannel_Config*) ||
            SIZE_MAX / (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1) < sizeof(SOPC_Endpoint_Config*))
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            memset(tConfig.scConfigs, 0, (SOPC_MAX_SECURE_CONNECTIONS + 1) * sizeof(SOPC_SecureChannel_Config*));
            memset(tConfig.serverScConfigs, 0, (SOPC_MAX_SECURE_CONNECTIONS + 1) * sizeof(SOPC_SecureChannel_Config*));
            memset(tConfig.epConfigs, 0,
                   (SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS + 1) * sizeof(SOPC_Endpoint_Config*));
            SOPC_EventTimer_Initialize();
            SOPC_Sockets_Initialize();
            SOPC_SecureChannels_Initialize();
            SOPC_Services_Initialize();
        }

        Mutex_Unlock(&tConfig.mut);
    }

    return status;
}

SOPC_ReturnStatus SOPC_Toolkit_Configured()
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (false == tConfig.locked)
        {
            // Check an address space is defined in case a endpoint configuration exists
            if (tConfig.epConfigIdxMax == 0 || tConfig.epConfigIdxMax > 0)
            {
                tConfig.locked = true;
                SOPC_Services_ToolkitConfigured();
                SOPC_Logger_Initialize(tConfig.logDirPath, tConfig.logMaxBytes, tConfig.logMaxFiles);
                SOPC_Logger_SetTraceLogLevel(tConfig.logLevel);
                status = SOPC_STATUS_OK;
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
        assert(false == scConfig->isClientSc);
        // In case of server it is an internally created config
        // => only client certificate was specifically allocated
        // Exceptional case: configuration added internally and shall be freed on clear call
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_KeyManager_Certificate_Free((SOPC_Certificate*) scConfig->crt_cli);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        free(scConfig);
        tConfig.serverScConfigs[serverScConfigIdxWithoutOffset] = NULL;
    }
}

// Deallocate fields allocated on server side only and free all the SC configs
static void SOPC_Toolkit_ClearServerScConfigs_WithoutLock(void)
{
    // Index 0 reserved for indet, index = MAX valid
    for (uint32_t i = 1; i <= SOPC_MAX_SECURE_CONNECTIONS; i++)
    {
        SOPC_ToolkitServer_ClearScConfig_WithoutLock(i);
    }
}

void SOPC_Toolkit_Clear()
{
    if (tConfig.initDone != false)
    {
        // Services are in charge to gracefully close all connections.
        // It must be done before stopping the services
        SOPC_Services_PreClear();

        SOPC_Sockets_Clear();
        SOPC_EventTimer_Clear();
        SOPC_SecureChannels_Clear();
        SOPC_Services_Clear();

        Mutex_Lock(&tConfig.mut);
        if (tConfig.encTypesTable != NULL)
        {
            free(tConfig.encTypesTable);
        }
        tConfig.nsTable = NULL;
        tConfig.encTypesTable = NULL;
        tConfig.nbEncTypesTable = 0;

        SOPC_Toolkit_ClearServerScConfigs_WithoutLock();
        SOPC_Logger_Clear();
        tConfig.locked = false;
        tConfig.initDone = false;
        Mutex_Unlock(&tConfig.mut);
        Mutex_Clear(&tConfig.mut);
    }
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
            if (tConfig.scConfigIdxMax < SOPC_MAX_SECURE_CONNECTIONS)
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
    if (scConfigIdx > 0 && scConfigIdx <= SOPC_MAX_SECURE_CONNECTIONS)
    {
        if (tConfig.initDone != false)
        {
            Mutex_Lock(&tConfig.mut);
            if (tConfig.locked != false)
            {
                res = tConfig.scConfigs[scConfigIdx];
            }
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
                if (lastScIdx < SOPC_MAX_SECURE_CONNECTIONS)
                {
                    lastScIdx++; // Minimum used == 1 && Maximum used == MAX + 1
                    if (NULL == tConfig.serverScConfigs[lastScIdx])
                    {
                        tConfig.serverScLastConfigIdx = lastScIdx;
                        tConfig.serverScConfigs[lastScIdx] = scConfig;
                        idxWithServerOffset =
                            SOPC_MAX_SECURE_CONNECTIONS + lastScIdx; // disjoint with SC config indexes for client
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

uint32_t SOPC_ToolkitServer_TranslateSecureChannelConfigIdxOffset(uint32_t serverScConfigIdx)
{
    uint32_t res = 0;
    if (serverScConfigIdx > SOPC_MAX_SECURE_CONNECTIONS &&
        serverScConfigIdx <= 2 * SOPC_MAX_SECURE_CONNECTIONS) // disjoint with SC config indexes for client
    {
        res = serverScConfigIdx - SOPC_MAX_SECURE_CONNECTIONS;
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

SOPC_ReturnStatus SOPC_ToolkitConfig_SetNamespaceUris(SOPC_NamespaceTable* nsTable)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (false == tConfig.locked)
        {
            status = SOPC_STATUS_OK;
            tConfig.nsTable = nsTable;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

static uint32_t GetKnownEncodeableTypesLength(void)
{
    uint32_t result = 0;
    for (result = 0; SOPC_KnownEncodeableTypes[result] != NULL; result++)
        ;
    return result + 1;
}

SOPC_ReturnStatus SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable, uint32_t nbTypes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (false == tConfig.locked)
        {
            uint32_t idx = 0;
            uint32_t nbKnownTypes = 0;
            SOPC_EncodeableType** additionalTypes = NULL;

            status = SOPC_STATUS_INVALID_PARAMETERS;
            if (encTypesTable != NULL && nbTypes > 0)
            {
                status = SOPC_STATUS_OK;
                if (NULL == tConfig.encTypesTable)
                {
                    // known types to be added
                    nbKnownTypes = GetKnownEncodeableTypesLength();
                    // +1 for null value termination
                    if (((uint64_t) nbKnownTypes + nbTypes + 1) <= SIZE_MAX / sizeof(SOPC_EncodeableType*))
                    {
                        tConfig.encTypesTable =
                            malloc(sizeof(SOPC_EncodeableType*) * (size_t)(nbKnownTypes + nbTypes + 1));
                    } // else NULL due to previous condition
                    if (NULL == tConfig.encTypesTable ||
                        tConfig.encTypesTable != memcpy(tConfig.encTypesTable, SOPC_KnownEncodeableTypes,
                                                        nbKnownTypes * sizeof(SOPC_EncodeableType*)))
                    {
                        tConfig.encTypesTable = NULL;
                    }
                    else
                    {
                        additionalTypes = tConfig.encTypesTable;
                        tConfig.nbEncTypesTable = nbKnownTypes;
                    }
                }
                else
                {
                    if ((uint64_t) tConfig.nbEncTypesTable + nbTypes + 1 <= SIZE_MAX / sizeof(SOPC_EncodeableType*))
                    {
                        // +1 for null value termination
                        additionalTypes =
                            realloc(tConfig.encTypesTable,
                                    sizeof(SOPC_EncodeableType*) * (size_t) tConfig.nbEncTypesTable + nbTypes + 1);
                    }
                    else
                    {
                        additionalTypes = NULL;
                    }
                }

                if (additionalTypes != NULL)
                {
                    tConfig.encTypesTable = additionalTypes;

                    for (idx = 0; idx < nbTypes; idx++)
                    {
                        tConfig.encTypesTable[tConfig.nbEncTypesTable + idx] = encTypesTable[idx];
                    }
                    tConfig.nbEncTypesTable += nbTypes;
                    // NULL terminated table
                }
                else
                {
                    status = SOPC_STATUS_NOK;
                }
            }
            return status;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}

SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes()
{
    SOPC_EncodeableType** res = NULL;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.locked != false)
        {
            if (tConfig.encTypesTable != NULL && tConfig.nbEncTypesTable > 0)
            {
                // Additional types are present: contains known types + additional
                res = tConfig.encTypesTable;
            }
            else
            {
                // No additional types: return static known types
                res = SOPC_KnownEncodeableTypes;
            }
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces()
{
    SOPC_NamespaceTable* res = NULL;
    if (tConfig.initDone != false)
    {
        Mutex_Lock(&tConfig.mut);
        if (tConfig.locked != false)
        {
            res = tConfig.nsTable;
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return res;
}

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace)
{
    (void) addressSpace;
    // No services implemented
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_AddressSpaceNotif_Fct* pAddSpaceNotifFct)
{
    (void) pAddSpaceNotifFct;
    // No services implemented
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_ToolkitConfig_SetLogPath(const char* logDirPath, uint32_t maxBytes, uint16_t maxFiles)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (logDirPath != NULL && maxBytes > 100 && maxFiles > 0)
    {
        status = SOPC_STATUS_INVALID_STATE;
        if (tConfig.initDone != false)
        {
            Mutex_Lock(&tConfig.mut);
            if (false == tConfig.locked)
            {
                tConfig.logDirPath = logDirPath;
                tConfig.logMaxBytes = maxBytes;
                tConfig.logMaxFiles = maxFiles;
                status = SOPC_STATUS_OK;
            }
            Mutex_Unlock(&tConfig.mut);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_ToolkitConfig_SetLogLevel(SOPC_Toolkit_Log_Level level)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    SOPC_Log_Level result = SOPC_LOG_LEVEL_ERROR;
    if (tConfig.initDone != false)
    {
        status = SOPC_STATUS_OK;
        Mutex_Lock(&tConfig.mut);
        switch (level)
        {
        case SOPC_TOOLKIT_LOG_LEVEL_ERROR:
            result = SOPC_LOG_LEVEL_ERROR;
            break;
        case SOPC_TOOLKIT_LOG_LEVEL_WARNING:
            result = SOPC_LOG_LEVEL_WARNING;
            break;
        case SOPC_TOOLKIT_LOG_LEVEL_INFO:
            result = SOPC_LOG_LEVEL_INFO;
            break;
        case SOPC_TOOLKIT_LOG_LEVEL_DEBUG:
            result = SOPC_LOG_LEVEL_DEBUG;
            break;
        default:
            result = SOPC_LOG_LEVEL_DEBUG;
        }
        if (false == tConfig.locked)
        {
            // Only record level for init
            tConfig.logLevel = result;
        }
        else
        {
            // Change the log level
            SOPC_Logger_SetTraceLogLevel(result);
        }
        Mutex_Unlock(&tConfig.mut);
    }
    return status;
}
