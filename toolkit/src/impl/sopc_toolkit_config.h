/**
 *  \file sopc_toolkit_config.h
 *
 *  \brief Stack initialization and configuration
 */
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

#ifndef SOPC_TOOLKIT_CONFIG_H_
#define SOPC_TOOLKIT_CONFIG_H_

#include "sopc_builtintypes.h"
#include "sopc_namespace_table.h"
#include "sopc_event_dispatcher_manager.h"
#include "sopc_types.h"
#include "key_manager.h"

extern SOPC_EventDispatcherManager* servicesEventDispatcherMgr;

typedef struct SOPC_SecureChannel_Config {
    const char*               url;
    const Certificate*        crt_cli;
    const AsymmetricKey*      key_priv_cli;
    const Certificate*        crt_srv;
    const PKIProvider*        pki;
    const char*               reqSecuPolicyUri;
    int32_t                   requestedLifetime;
    OpcUa_MessageSecurityMode msgSecurityMode;
} SOPC_SecureChannel_Config;

typedef struct SOPC_Endpoint_Config{
    char*          endpointURL;
    Certificate*   serverCertificate;
    AsymmetricKey* serverKey;
    PKIProvider*   pki;
} SOPC_Endpoint_Config;

/**
 *  \brief  Initialize the toolkit configuration, libraries and threads
 *
 *  \return STATUS_OK if initialization succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Toolkit_Initialize();

/**
 *  \brief  Define toolkit configuration as configured and lock its state until toolkit clear operation
 *
 *  \return STATUS_OK if initialization succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Toolkit_Configured();

/**
 *  \brief  Clear the stack configuration
 */
void SOPC_Toolkit_Clear();

/**
 *  \brief Record the given secure channel configuration in given index
 *  (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \param scConfigIdx  The (unique) index for the given configuration
 *  \param scConfig     The secure channel configuration to be set at given index
 *
 *  \return STATUS_OK if configuration succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_ToolkitConfig_AddSecureChannelConfig(uint32_t                   scConfigIdx,
                                                          SOPC_SecureChannel_Config* scConfig);


/**
 *  \brief Return the secure channel configuration for the given index or null if not defined. (SOPC_Toolkit_Configured required)
 *
 *  \param scConfigIdx  The secure channel configuration index requested
 *
 *  \return Secure channel configuration at given index
 */
SOPC_SecureChannel_Config* SOPC_ToolkitConfig_GetSecureChannelConfig(uint32_t scConfigIdx);

/**
 *  \brief Return the endpoint configuration for the given index or null if not defined. (SOPC_Toolkit_Configured required)
 *
 *  \param epConfigIdx  The endpoint configuration index requested
 *
 *  \return Endpoint configuration at given index
 */
SOPC_Endpoint_Config* SOPC_ToolkitConfig_GetEndpointConfig(uint32_t epConfigIdx);

/**
 *  \brief Record the given secure channel configuration in given index
 *  (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \param epConfigIdx     The (unique) index for the given configuration
 *  \param endpointConfig  The endpoint configuration to be set at given index
 *
 *  \return STATUS_OK if configuration succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_ToolkitConfig_AddEndpointConfig(uint32_t              idx,
                                                     SOPC_Endpoint_Config* config);

/**
 *  \brief Set the given namespace table configuration (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \param nsTable  The new namespace table to be used by the stack
 *
 *  \return STATUS_OK if configuration succeeded, STATUS_NOK otherwise (NULL pointer or invalid state)
 */
SOPC_StatusCode SOPC_ToolkitConfig_SetNamespaceUris(SOPC_NamespaceTable* nsTable);

/**
 *  \brief Return the encodeable types table configuration used by the stack (SOPC_Toolkit_Configured required)
 *
 *  \return Encodeable types table (terminated by NULL value)
 */
SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes();


/**
 *  \brief Add the given encodeable types to the configuration (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \param encTypesTable  The encodeable types to add to the encodeable types configuration (NULL terminated)
 *  \param nbTypes        Number of encodeable types provided in the table
 *
 *  \return STATUS_OK if configuration succeeded, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable,
                                            uint32_t              nbTypes);

/**
 *  \brief Return the namespace table configuration  (SOPC_Toolkit_Configured required)
 *
 *  \return Namespace table
 */
SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces();

#endif /* SOPC_TOOLKIT_CONFIG_H_ */
