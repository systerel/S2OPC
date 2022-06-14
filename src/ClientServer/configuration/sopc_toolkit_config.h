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

#include "sopc_address_space.h"
#include "sopc_user_app_itf.h"

#include "sopc_builtintypes.h"
#include "sopc_common_build_info.h"
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
 *  \warning it is only necessary for the server configuration finalization
 *           and should not be set before server configuration ended.
 *
 *  \return SOPC_STATUS_OK if initialization succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized or already
 *  configured,
 *  SOPC_STATUS_INVALID_PARAMETERS if server configuration is defined but no address space is set
 */
SOPC_ReturnStatus SOPC_ToolkitServer_Configured(void);

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
 *  \brief Set the given Address Space modification notification callback
 *  for the current toolkit server (SOPC_ToolkitServer_Initialize required, !SOPC_Toolkit_Configured required).
 *  Note: only one callback can be set, further call will be refused.
 *
 *  \param pAddSpaceNotifFct  The address space notification event callback definition
 *
 *  \return SOPC_STATUS_OK if configuration succeeded,
 *  SOPC_STATUS_INVALID_STATE if toolkit is not initialized, already
 *  configured or address space is already set, SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_ToolkitServer_SetAddressSpaceNotifCb(SOPC_AddressSpaceNotif_Fct* pAddSpaceNotifFct);

/**
 * \brief Index type for client secure channel / endpoint connection configuration
 */
typedef uint32_t SOPC_EndpointConnectionConfigIdx;

/**
 *  \brief Record the given secure channel configuration for endpoint connection in returned index
 *  (SOPC_ToolkitClient_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \return secure channel configuration index if configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
SOPC_EndpointConnectionConfigIdx SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);

/**
 * \brief Index type for server endpoint configuration
 */
typedef uint32_t SOPC_EndpointConfigIdx;

/**
 *  \brief Record the given endpoint configuration in given index
 *  (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  Note1: it is forbidden to have 2 configurations with same endpointURL
 *  Note2: if security policy None is not activated, less than SOPC_MAX_SECU_POLICIES_CFG
 *         shall be defined to allow discovery services access to be added.
 *
 *  \return endpoint configuration index configuration succeeded,
 *  0 if toolkit is not initialized, already configured or parameters provided are incorrect
 */
SOPC_EndpointConfigIdx SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* config);

/**
 * \brief Get Toolkit build information
 *
 *
 * \return          Toolkit build information
 */
SOPC_Toolkit_Build_Info SOPC_ToolkitConfig_GetBuildInfo(void);

/**
 *  \brief  Close all Secure Channels established as client
 *          and clear associated configurations added by ::SOPC_ToolkitClient_AddSecureChannelConfig.
 *          All previous index returned by ::SOPC_ToolkitClient_AddSecureChannelConfig
 *          are invalid after calling this function.
 *
 *  \note Any client active session should be closed prior to calling this function.
 *
 *  \note It should be used to ensure configuration previously added with
 *        ::SOPC_ToolkitClient_AddSecureChannelConfig are not used anymore by toolkit.
 *        Other alternative to ensure that is call to ::SOPC_Toolkit_Clear.
 */
void SOPC_ToolkitClient_ClearAllSCs(void);

#endif /* SOPC_TOOLKIT_CONFIG_H_ */
