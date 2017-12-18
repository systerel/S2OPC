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

/**
 * \file sopc_toolkit_config.h
 *
 * \brief This module shall be used to initialize, configure and clear/terminate the toolkit execution.
 *
 * It is in charge to initialize each event based layer (sockets, secure channels, services) which will
 * start the associated threads.
 * It is also necessary to configure the endpoint description configuration and address space of a toolkit server
 * instance, or the endpoint connection configuration of a toolkit client instance.
 *
 */

#ifndef SOPC_TOOLKIT_CONFIG_H_
#define SOPC_TOOLKIT_CONFIG_H_

#include "sopc_user_app_itf.h"

#include "sopc_builtintypes.h"
#include "sopc_namespace_table.h"
#include "sopc_types.h"

/**
 *  \brief  Initialize the toolkit configuration, libraries and threads
 *
 *  \param pAppFct  Pointer to applicative code function in charge of toolkit communication events
 *
 *  \return SOPC_STATUS_OK if initialization succeeded,
 *  SOPC_STATUS_INVALID_PARAMETERS if \p pAppFct == NULL or
 *  SOPC_STATUS_INVALID_STATE if toolkit already initialized and
 *  SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct);

/**
 *  \brief  Define toolkit configuration as configured and lock its state until toolkit clear operation
 *
 *  \return SOPC_STATUS_OK if initialization succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already
 *  configured
 */
SOPC_ReturnStatus SOPC_Toolkit_Configured(void);

/**
 *  \brief  Clear the stack configuration
 */
void SOPC_Toolkit_Clear(void);

/**
 *  \brief Set the given Address Space for the current toolkit server
 *  (SOPC_ToolkitServer_Initialize required, !SOPC_Toolkit_Configured required).
 *  Note: only one address space can be set, further call will be refused.
 *
 *  \param addressSpace  The address space definition
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized, already
 *  configured or address space is already set, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);

/**
 *  \brief Record the given secure channel configuration in returned index
 *  (SOPC_ToolkitClient_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \return secure channel configuration index if configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);

/**
 *  \brief Record the given secure channel configuration in given index
 *  (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *  Note: it is forbidden to have 2 configurations with same endpointURL
 *
 *  \return endpoint configuration index configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* config);

/**
 *  \brief Set the given namespace table configuration (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured
 * required)
 *
 *  \param nsTable  The new namespace table to be used by the stack
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already
 *  configured, SOPC_STATUS_INVALID_PARAMETER if \p nsTable == NULL,
 *  SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_SetNamespaceUris(SOPC_NamespaceTable* nsTable);

/**
 *  \brief Add the given encodeable types to the configuration (SOPC_Toolkit_Initialize required,
 * !SOPC_Toolkit_Configured required)
 *
 *  \param encTypesTable  The encodeable types to add to the encodeable types configuration (NULL terminated)
 *  \param nbTypes        Number of encodeable types provided in the table
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already
 *  configured, SOPC_STATUS_INVALID_PARAMETER if \p encTypesTable == NULL,
 *  SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable, uint32_t nbTypes);

#endif /* SOPC_TOOLKIT_CONFIG_H_ */
